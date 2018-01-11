%option noyywrap

%{
    extern "C"
    {
        int yylex(void);
    }

    void yyerror(const char*);

    #include <cstdlib>
    #include <string>
    #include <iostream>
    #include <cln/cln.h>
    #include "bison.tab.h"

    static void update_loc(){
        static int curr_line = 1;
        static int curr_col  = 1;

        yylloc.first_line   = curr_line;
        yylloc.first_column = curr_col;

        char * s; 
        for(s = yytext; *s != '\0'; s++) {
            if (*s == '\n') {
            curr_line++;
            curr_col = 1;
            } else {
                curr_col++;
            }
        }

        yylloc.last_line   = curr_line;
        yylloc.last_column = curr_col;
    }

    #define YY_USER_ACTION update_loc();

    Position* createPos(YYLTYPE yylpos);
%}

%x COMMENT

%%
[ \t\n\r]                       ;
<INITIAL>"("                    { BEGIN(COMMENT); }
<COMMENT>")"                    { BEGIN(INITIAL); }
<COMMENT>.|[\n\t]               ;

<INITIAL>"BEGIN"                { return T_BEGIN; }
<INITIAL>"END"                  { return T_END; }
<INITIAL>"VAR"                  { return T_VAR; }

<INITIAL>":="                   { return T_ASSIGN; }
<INITIAL>"READ"                 { return T_READ; }
<INITIAL>"WRITE"                { return T_WRITE; }
<INITIAL>"IF"                   { return T_IF; }
<INITIAL>"THEN"                 { return T_THEN; }
<INITIAL>"ENDIF"                { return T_ENDIF; }
<INITIAL>"ELSE"                 { return T_ELSE; }
<INITIAL>"WHILE"                { return T_WHILE; }
<INITIAL>"DO"                   { return T_DO; }
<INITIAL>"ENDWHILE"             { return T_ENDWHILE; }
<INITIAL>"FOR"                  { return T_FOR; }
<INITIAL>"FROM"                 { return T_FROM; }
<INITIAL>"TO"                   { return T_TO; }
<INITIAL>"DOWNTO"               { return T_DOWNTO; }
<INITIAL>"ENDFOR"               { return T_ENDFOR; }

<INITIAL>"+"                    { return T_PLUS; }
<INITIAL>"-"                    { return T_MINUS; }
<INITIAL>"*"                    { return T_MULT; }
<INITIAL>"/"                    { return T_DIV; }
<INITIAL>"%"                    { return T_MOD; }

<INITIAL>"="                    { return T_EQ; }
<INITIAL>"<>"                   { return T_NEQ; }
<INITIAL>"<"                    { return T_LT; }
<INITIAL>">"                    { return T_GT; }
<INITIAL>"<="                   { return T_LEQ; }
<INITIAL>">="                   { return T_GEQ; }

<INITIAL>("0"|[1-9][0-9]*) 		{ yylval.num = new cl_I(yytext); return T_NUM; }
<INITIAL>[_a-z]+                { yylval.str = new string(yytext); return T_PIDIDENTIFIER; }

<INITIAL>"["                    { return T_LEFT_ARR_BRACE; }
<INITIAL>"]"                    { return T_RIGHT_ARR_BRACE; }

<INITIAL>";"                    ;

.                               { poserror(createPos(yylloc), "bad sign"); }
%%