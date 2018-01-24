%code requires{
    #include "ProgramFlags.h"
    #include "cmdParts/Number.h"
    #include "cmdParts/Identifier.h"
    #include "cmdParts/Expression.h"
    #include "cmdParts/Value.h"
    #include "cmdParts/Condition.h"
    #include "cmds/Command.h"
    #include "cmds/CommandsBlock.h"
    #include "cmds/Assignment.h"
    #include "cmds/For.h"
    #include "cmds/If.h"
    #include "cmds/IfElse.h"
    #include "cmds/Read.h"
    #include "cmds/While.h"
    #include "cmds/Write.h"
}
%{
extern "C"
{
    int yylex(void);
}
#include "Utils.h"
#include "ProgramFlags.h"
#include <exception>
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <stack>
#include "Program.h"
#include "Position.h"
#include "cmdParts/Number.h"
#include "cmdParts/Identifier.h"
#include "cmdParts/Expression.h"
#include "cmdParts/Value.h"
#include "cmdParts/Condition.h"
#include "cmds/Command.h"
#include "cmds/CommandsBlock.h"
#include "cmds/Assignment.h"
#include "cmds/For.h"
#include "cmds/If.h"
#include "cmds/IfElse.h"
#include "cmds/Read.h"
#include "cmds/While.h"
#include "cmds/Write.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

struct YYLTYPE;
Position* createPos(YYLTYPE yylpos);

stack<CommandsBlock*> blockStack;

Program program;

void yyerror(const char*);
Command* unrollWhile(Condition* cond, CommandsBlock* block, int numOfUnrolls);
%}

%locations

%union {
    std::string* str;
    cln::cl_I*   num;
    Number*      number;
    Identifier*  ident;
    Expression*  expr;
    Value*       value;
    Command*     cmd;
    Condition*   cond;
}

%token T_BAD_SIGN

%token T_VAR
%token T_BEGIN
%token T_END

%token T_ASSIGN
%token T_READ
%token T_WRITE
%token T_IF
%token T_THEN
%token T_ENDIF
%token T_ELSE
%token T_WHILE
%token T_DO
%token T_ENDWHILE
%token T_FOR
%token T_FROM
%token T_TO
%token T_DOWNTO
%token T_ENDFOR

%token T_PLUS
%token T_MINUS
%token T_MULT
%token T_DIV
%token T_MOD

%token T_EQ
%token T_NEQ
%token T_LT
%token T_GT
%token T_LEQ
%token T_GEQ

%token<num> T_NUM
%token<str> T_PIDIDENTIFIER

%token T_LEFT_ARR_BRACE
%token T_RIGHT_ARR_BRACE

%type<number> number
%type<ident> identifier
%type<expr> expression
%type<cmd> command
%type<value> value
%type<cond> condition

%start program

%%

program: T_VAR vdeclarations T_BEGIN commands T_END {
    program.setBlock(blockStack.top());
    blockStack.pop();
}
;

vdeclarations:
    | vdeclarations T_PIDIDENTIFIER {
        memory.addVariable(createPos(@2), *$2);
    }
    | vdeclarations T_PIDIDENTIFIER T_LEFT_ARR_BRACE T_NUM T_RIGHT_ARR_BRACE {
        memory.addArrayVariable(createPos(@2), createPos(@4), *$2, *$4);
    }
;

commands:                   { blockStack.push(new CommandsBlock()); }
    | commands command      { blockStack.top()->addCommand($2); }
;

command: identifier T_ASSIGN expression                                         { $$ = new Assignment($1, $3); }
    | T_IF condition T_THEN commands T_ENDIF                                    { $$ = new If($2, blockStack.top()); blockStack.pop(); }
    | T_IF condition T_THEN commands T_ELSE commands T_ENDIF                    { auto block1 = blockStack.top(); blockStack.pop();
                                                                                auto block2 = blockStack.top(); blockStack.pop();
                                                                                if(block1->equals(block2)){
                                                                                    $$ = block1;
                                                                                } else {
                                                                                    $$ = new IfElse($2, block2, block1);
                                                                                }}
    | T_FOR T_PIDIDENTIFIER T_FROM value T_TO value T_DO commands T_ENDFOR      { $$ = new For(createPos(@2), *$2, $4, $6, blockStack.top(), true); blockStack.pop(); }
    | T_FOR T_PIDIDENTIFIER T_FROM value T_DOWNTO value T_DO commands T_ENDFOR  { $$ = new For(createPos(@2), *$2, $4, $6, blockStack.top(), false); blockStack.pop(); }
    | T_WHILE condition T_DO commands T_ENDWHILE                                { $$ = unrollWhile($2, blockStack.top(), pflags.getWhileUnrollsNumber()); blockStack.pop();} //while -> if with while inside for checking for first time args
    | T_READ identifier                                                         { $$ = new Read($2); }
    | T_WRITE value                                                             { $$ = new Write($2); }
;

expression: value                   { $$ = new Expression($1); }
    | value T_PLUS value            { $$ = new Expression(Expression::Type::ADDITION, $1, $3); }
    | value T_MINUS value           { $$ = new Expression(Expression::Type::SUBTRACTION, $1, $3); }
    | value T_MULT value            { $$ = new Expression(Expression::Type::MULTIPLICATION, $1, $3); }
    | value T_DIV value             { $$ = new Expression(Expression::Type::DIVISION, $1, $3); }
    | value T_MOD value             { $$ = new Expression(Expression::Type::MODULO, $1, $3); }
;

condition: value T_EQ value     { $$ = new Condition(Condition::Type::EQ, $1, $3); }
    | value T_NEQ value         { $$ = new Condition(Condition::Type::NEQ, $1, $3); }
    | value T_LT value          { $$ = new Condition(Condition::Type::LT, $1, $3); }
    | value T_GT value          { $$ = new Condition(Condition::Type::GT, $1, $3); }
    | value T_LEQ value         { $$ = new Condition(Condition::Type::LEQ, $1, $3); }
    | value T_GEQ value         { $$ = new Condition(Condition::Type::GEQ, $1, $3); }
;

value: number{
        $$ = new Value($1);
    }
    | identifier {
        $$ = new Value($1);
    }
;

number: T_NUM {
        $$ = new Number(createPos(@1), *$1);
    }
;

identifier: T_PIDIDENTIFIER {
        $$ = new Identifier(createPos(@1), *$1);
    }
    | T_PIDIDENTIFIER T_LEFT_ARR_BRACE T_PIDIDENTIFIER T_RIGHT_ARR_BRACE {
        $$ = new Identifier(createPos(@1), *$1, *$3);
    }
    | T_PIDIDENTIFIER T_LEFT_ARR_BRACE T_NUM T_RIGHT_ARR_BRACE { 
        $$ = new Identifier(createPos(@1), *$1, *$3);
    }
;

%%
Position* createPos(YYLTYPE yylpos){
    return new Position(yylpos.first_line, yylpos.first_column, yylpos.last_line, yylpos.last_column);
}

Command* unrollWhile(Condition* cond, CommandsBlock* block, int numOfUnrolls){
    Command* lastCommand = new While(cond->clone(), block->clone());
    for(int i = 0; i < numOfUnrolls; ++i){
        CommandsBlock* clonedBlock = block->clone();
        clonedBlock->addCommand(lastCommand);

        lastCommand = new If(cond->clone(), clonedBlock);
    }

    return lastCommand;
}

void yyerror (const char* s){
    cout << "error - " << string(s) << endl;
    cerr << "error - " << string(s) << endl;
    throw exception();
}

void poserror(Position* pos, const string& s){
    cout << pos->fl << ":" << pos->fc << "-" << pos->ll << ":" << pos->lc << " ";
    cerr << pos->fl << ":" << pos->fc << "-" << pos->ll << ":" << pos->lc << " ";
    yyerror(s.c_str());
}

void showHelp(){
    string help =
    "Usage ./compiler [-h|v|nf|nm|nn|np] [-w n] < input > output 2> log\n"
    "\n"
    "Available params\n"
    "   -h --help             - shows this help\n"
    "   -v --verbose          - prints full log - default(false)\n"
    "   -w --while n          - unroll while loops n times - default(50) - LOWER THIS IF YOU HAVE MULTIPLE NESTED LOOPS\n"
    "   -nf --no-for          - don't unroll for loops - default(false)\n"
    "   -nm --no-mem          - don't optimize variable declarations in memory - default(false)\n"
    "   -nn --no-num          - don't store similar numbers in new identifier if it's profitable - default(false)\n"
    "   -np --no-propagation  - don't propagate values - default(false)\n"
    "   -nc --no-opt-code     - don't show resulted optimized code";

    cerr << help << endl;
}

int main(int argc, char* argv[]) {

    for(int i = 1; i < argc; ++i){
        string arg = argv[i];
        if(arg == "-h" || arg == "--help"){
            showHelp();
            return EXIT_SUCCESS;
        } else if(arg == "-v" || arg == "--verbose"){
            pflags.setVerbose(true);
        } else if(arg == "-w" || arg == "--while"){
            if(i + 1 < argc){
                try{
                    i++;
                    string numStr = argv[i];
                    int num = stoi(numStr);
                    pflags.setWhileUnrollsNumber(num);

                } catch(const std::exception&) {
                    cerr << "Wrong unrolls number" << endl;
                    return EXIT_FAILURE;
                }
            } else {
                cerr << "Needs a while unrolls number" << endl;
                return EXIT_FAILURE;
            }
        } else if(arg == "-nf" || arg == "--no-for"){
            pflags.setUnrollFors(false);
        } else if(arg == "-nm" || arg == "--no-mem"){
            pflags.setMemoryOptimization(false);
        } else if(arg == "-nn" || arg == "--no-num"){
            pflags.setOptimizeSimilarNumbers(false);
        } else if(arg == "-np" || arg == "--no-propagation"){
            pflags.setPropagateValues(false);
        } else if(arg == "-nc" || arg == "--no-opt-code"){
            pflags.setShowOptimizedCode(false);
        } else {
            cerr << "Wrong param" << endl;
            return EXIT_FAILURE;
        }
    }

    try {
        if(pflags.verbose())
            cerr << "---PARSING---" << endl;
        yyparse();
        cout << program.generateCode();
    } catch(const std::exception&) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
