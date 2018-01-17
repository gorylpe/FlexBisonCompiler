%code requires{
    #include "Number.h"
    #include "Identifier.h"    
    #include "Expression.h"    
    #include "Value.h"
    #include "Command.h"
    #include "Condition.h"
}
%{
extern "C"
{
    int yylex(void);
}
#include "Utils.h"

#include <exception>
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <stack>
#include "Program.h"
#include "Number.h"
#include "Identifier.h"    
#include "Expression.h"    
#include "Value.h"
#include "Command.h"
#include "Condition.h"
#include "Position.h"

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

command: identifier T_ASSIGN expression                                     { $$ = new Assignment($1, $3); }
    | T_IF condition T_THEN commands T_ENDIF                                { $$ = new If($2, blockStack.top()); blockStack.pop(); }
    | T_IF condition T_THEN commands T_ELSE commands T_ENDIF                { auto block1 = blockStack.top(); blockStack.pop();
                                                                              auto block2 = blockStack.top(); blockStack.pop();
                                                                              $$ = new IfElse($2, block2, block1); }
    | T_FOR T_PIDIDENTIFIER T_FROM value T_TO value T_DO commands T_ENDFOR  { $$ = new For(createPos(@2), *$2, $4, $6, blockStack.top(), true); blockStack.pop(); }
    | T_FOR T_PIDIDENTIFIER T_FROM value T_DOWNTO value T_DO commands T_ENDFOR  { $$ = new For(createPos(@2), *$2, $4, $6, blockStack.top(), false); blockStack.pop(); }
    | T_WHILE condition T_DO commands T_ENDWHILE                            { $$ = new While($2, blockStack.top()); blockStack.pop(); }
    | T_READ identifier                                                     { $$ = new Read($2); }
    | T_WRITE value                                                         { $$ = new Write($2); }
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

int main(void) {
    cerr << "Compilation started" << endl;
    cerr.flush();
    try {
        yyparse();
        cerr << "Parsed" << endl;
        cout << program.generateCode();
    } catch(const std::exception&) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
