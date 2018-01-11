CC=g++ -std=c++11

all: compiler

compiler: bison.tab.c lex.yy.c
	$(CC) 	bison.tab.c \
		lex.yy.c \
		Expression.cpp \
		-l cln -o compiler

lex.yy.c: compiler.l
	flex compiler.l

bison.tab.c: compiler.y
	bison -d -o bison.tab.c compiler.y

clean:
	rm lex.yy.c bison.tab.c bison.tab.h
