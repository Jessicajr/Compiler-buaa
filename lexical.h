#pragma once
//标识符
#define IDENFR 1
//if
#define IFTK 2
//-
#define MINU 3
//=
#define ASSIGN 4
//整型常量
#define INTCON 5
//else
#define ELSETK 6
//*
#define MULT 7
//;
#define SEMICN 8
//字符常量
#define CHARCON 9
//do
#define DOTK 10
///
#define DIV 11
//,
#define COMMA 12
//字符串
#define STRCON 13
//while
#define WHILETK 14
//<
#define LSS 15
//	(
#define LPARENT 16
//const
#define CONSTTK 17
//for
#define FORTK 18
//<=
#define LEQ 19
//)
#define RPARENT 20
//int
#define INTTK 21
//scanf
#define SCANFTK 22
//>
#define GRE 23
//[
#define LBRACK 24
//char
#define CHARTK 25
//printf
#define PRINTFTK 26
//>=
#define GEQ 27
//]
#define RBRACK 28
//void
#define VOIDTK 29
//return
#define RETURNTK 30
//==
#define EQL 31
//{
#define LBRACE 32
//main
#define MAINTK 33
//+
#define PLUS 34
//!=
#define NEQ 35
//}
#define RBRACE 36
#define BOTTOM 9999
typedef struct Lexical {
	char item[100];
	int type;
}Lexical;

Lexical getLexi();
int getLexiSize();
int getsym();
char* getWord();
void error(char ch);
void programme();
int expression();
int returnCall();
int statement();
int valueTable();
int statementSequence();
char getGlbCh();
int getGlbInt();
char* getGlbString();