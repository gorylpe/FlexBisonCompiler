/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_BISON_TAB_H_INCLUDED
# define YY_YY_BISON_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 1 "compiler.y" /* yacc.c:1909  */

    #include "ProgramFlags.h"
    #include "cmdParts/Number.h"
    #include "cmdParts/Identifier.h"
    #include "cmdParts/Expression.h"
    #include "cmdParts/Value.h"
    #include "cmdParts/Condition.h"
    #include "cmds/Command.h"
    #include "cmds/Assignment.h"
    #include "cmds/For.h"
    #include "cmds/If.h"
    #include "cmds/IfElse.h"
    #include "cmds/Read.h"
    #include "cmds/While.h"
    #include "cmds/Write.h"

#line 61 "bison.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_BAD_SIGN = 258,
    T_VAR = 259,
    T_BEGIN = 260,
    T_END = 261,
    T_ASSIGN = 262,
    T_READ = 263,
    T_WRITE = 264,
    T_IF = 265,
    T_THEN = 266,
    T_ENDIF = 267,
    T_ELSE = 268,
    T_WHILE = 269,
    T_DO = 270,
    T_ENDWHILE = 271,
    T_FOR = 272,
    T_FROM = 273,
    T_TO = 274,
    T_DOWNTO = 275,
    T_ENDFOR = 276,
    T_PLUS = 277,
    T_MINUS = 278,
    T_MULT = 279,
    T_DIV = 280,
    T_MOD = 281,
    T_EQ = 282,
    T_NEQ = 283,
    T_LT = 284,
    T_GT = 285,
    T_LEQ = 286,
    T_GEQ = 287,
    T_NUM = 288,
    T_PIDIDENTIFIER = 289,
    T_LEFT_ARR_BRACE = 290,
    T_RIGHT_ARR_BRACE = 291
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 67 "compiler.y" /* yacc.c:1909  */

    std::string* str;
    cln::cl_I*   num;
    Number*      number;
    Identifier*  ident;
    Expression*  expr;
    Value*       value;
    Command*     cmd;
    Condition*   cond;

#line 121 "bison.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_BISON_TAB_H_INCLUDED  */
