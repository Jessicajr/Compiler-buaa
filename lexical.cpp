#define _CRT_SECURE_NO_WARNINGS
#include"lib.h"
#include"lexical.h"
#include "table.h"
using namespace std;
char token[500];
int line = 1;
FILE* in, * out;
char* Start = token;
char* End = token;
queue<Lexical> BUFFER;   //输出队列
char cache[50];
char glbCh;
int glbInt1;
char glbString[100];
//int spaceFlag = 0;
//int spaceCount;

void clearToken() {
	int i;
	for (i = 0; i <=(End-Start); i++) {
		token[i] = '\0';
	}
	End = Start;
}

void retract(int step) {
	int i = 0;
	for (i = 0; i < step; i++) {
		fseek(in, -1, SEEK_CUR);
	}
}

char* get() {
	return Start;
}
char* getWord() {
	return cache;
}

void clearCache() {
	int i;
	for (i = 0; i < 50; i++) {
		cache[i] = '\0';
	}
}
void error(char ch) {
	/*if (ch == 'a') {
		;
	}
	if (ch == 'k') {
		printf("%d %c\n", line-1, ch);
		fprintf(out, "%d %c\n", line-1, ch);
		//spaceFlag = 0;
	}
	else {
		printf("%d %c\n", line, ch);
		fprintf(out, "%d %c\n", line, ch);
	}*/
}


void catToken(char ch) {
	*End = ch;
	End++;
}

int isAdd(char ch) {
	if (ch == '+' || ch == '-') {
		return 1;
	}
	return 0;
}

int isMulti(char ch) {
	if (ch == '*' || ch == '/') {
		return 1;
	}
	return 0;
}
int isSpace(char ch) {
	if (ch == '\n') {
		line++;
	}
	if (ch == ' ' || ch == '\n' || ch == '\t'|| ch=='\r') {
		return 1;
	}
	return 0;
}

int isLetter(char ch) { //＜字母＞   ::= ＿｜a｜．．．｜z｜A｜．．．｜Z
	if ((ch == '_') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
		return 1;
	}
	return 0;
}

int isDigit(char ch) {  //＜数字＞ ::= ０｜＜非零数字＞
	if (ch >= '0' && ch <= '9') {
		return 1;
	}
	return 0;
}

int legalChar(char ch) {
	if (isAdd(ch) || isMulti(ch) || isDigit(ch) || isLetter(ch))
		return 1;
	return 0;
}
int getLexiSize() {
	return BUFFER.size();
}

int isRelation(char ch) {//＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==
	catToken(ch);
	if (ch == '<') {
		ch = fgetc(in);
		if (ch == '=') {
			catToken(ch);
			return LEQ;
		}
		else {
			if (ch == EOF)
				return BOTTOM;
			retract(1);
			return LSS;

		}
	}
	else if (ch == '>') {
		ch = fgetc(in);
		if (ch == '=') {
			catToken(ch);
			return GEQ;
		}
		else {
			if (ch == EOF)
				return BOTTOM;
			retract(1);
			return GRE;
		}
	}
	else if (ch == '=') {
		ch = fgetc(in);
		if (ch == '=') {
			catToken(ch);
			return EQL;
		}
		else {
			if (ch == EOF)
				return BOTTOM;
			retract(1);
			return ASSIGN;
		}
	}
	else if (ch == '!') {
		ch = fgetc(in);
		if (ch == '=') {
			catToken(ch);
			return NEQ;
		}
		else {
			error('a');
			return -1;
		}
	}
	return 0;
}

int isReserver(char* str) {   //＜类型标识符＞      ::=  int | char
	if (strcmp(str, "const") == 0) {
		return CONSTTK;
	}
	else if (strcmp(str, "int") == 0) {
		return INTTK;
	}
	else if (strcmp(str, "char") == 0) {
		return CHARTK;
	}
	else if (strcmp(str, "void") == 0) {
		return VOIDTK;
	}
	else if (strcmp(str, "main") == 0) {
		return MAINTK;
	}
	else if (strcmp(str, "if") == 0) {
		return IFTK;
	}
	else if (strcmp(str, "else") == 0) {
		return ELSETK;
	}
	else if (strcmp(str, "do") == 0) {
		return DOTK;
	}
	else if (strcmp(str, "while") == 0) {
		return WHILETK;
	}
	else if (strcmp(str, "for") == 0) {
		return FORTK;
	}
	else if (strcmp(str, "scanf") == 0) {
		return SCANFTK;
	}
	else if (strcmp(str, "printf") == 0) {
		return PRINTFTK;
	}
	else if (strcmp(str, "return") == 0) {
		return RETURNTK;
	}
	return 0;
}

int isSign(char ch) {  //＜标识符＞    ::=  ＜字母＞｛＜字母＞｜＜数字＞｝
	char* tok;
	//strcpy(symbol,"sign");
	int judge;
	while (isLetter(ch) || isDigit(ch)) {
		catToken(ch);
		ch = fgetc(in);
	}
	retract(1);
	tok = get();
	strcpy(cache, tok);
	judge = isReserver(tok);
	//区分保留字
  //     printf("IDENFR %s\n",tok);
	if (!judge) return IDENFR;
	return judge;
}



int getsymType() {
	char ch;
	Lexical l;
	if ((ch = fgetc(in)) != EOF) {
		while (isSpace(ch)) {
			//spaceCount++;
			ch = fgetc(in);
		}
		if (isLetter(ch)) {     //＜标识符＞    ::=  ＜字母＞｛＜字母＞｜＜数字＞｝
			return isSign(ch);
		}
		else if (isDigit(ch))  //整型常量   ＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝| 0
		{
			int count = 0;
			char oldch;
			while (isDigit(ch)) {
				catToken(ch);
				oldch = ch;
				ch = fgetc(in);
				if (count == 0 && oldch == '0' && isDigit(ch)) error('a');   //0｛＜数字＞｝非法
				count++;
			}
			if (isLetter(ch)) {
				while (isLetter(ch) || isDigit(ch)) {
					catToken(ch);
					ch = fgetc(in);
				}
				retract(1);
				error('a');
				return IDENFR;
			}

			retract(1);
			return INTCON;
		}

		else if (ch == '\"') {   //字符串常量
			//char old = ch;
			ch = fgetc(in);
			while (ch != '\"') {
				if (ch == '\\') 
					catToken('\\');
				catToken(ch);
				//old = ch;
				ch = fgetc(in);
			}
			return(STRCON);
		}
		else if (ch == '\'') {  //字符型常量 '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞'
			ch = fgetc(in);
			if (!legalChar(ch)) error('a');
			glbCh = ch;
			ch = fgetc(in);
			if (ch != '\'') {     // 未找到对应的单引号
				error('a');
				retract(1);
			}
			return CHARCON;
		}
		else if (ch == '<' || ch == '>' || ch == '=' || ch == '!') return(isRelation(ch));
		else if (ch == '+') {
			catToken(ch);
			return PLUS;
		}
		else if (ch == '-') {
			catToken(ch);
			return MINU;
		}
		else if (ch == '*') {
			catToken(ch);
			return MULT;
		}
		else if (ch == '/') {
			catToken(ch);
			return DIV;
		}
		else if (ch == ';') {
			catToken(ch);
			return SEMICN;
		}
		else if (ch == ',') {
			catToken(ch);
			return COMMA;
		}
		else if (ch == '(') {
			catToken(ch);
			return LPARENT;
		}
		else if (ch == ')') {
			catToken(ch);
			return RPARENT;
		}
		else if (ch == '[') {
			catToken(ch);
			return LBRACK;
		}
		else if (ch == ']') {
			catToken(ch);
			return RBRACK;
		}
		else if (ch == '{') {
			catToken(ch);
			return LBRACE;
		}
		else if (ch == '}') {
			catToken(ch);
			return RBRACE;
		}
		else {
			error('a');
			return -1;
		}
		//else error();
	}
	return BOTTOM;
}

int getsym() {
	int type;
	char* tok;
	Lexical l;
	type = getsymType();
	while (type == -1)
		type = getsymType();
	tok = get();
	strcpy(l.item, tok);
	l.type = type;
	BUFFER.push(l);
	if (type == INTCON) 
		sscanf(tok, "%d", &glbInt1);
	if (type == STRCON)
		strcpy(glbString, tok);
	clearToken();
	return type;
}
char* getGlbString() {
	return glbString;
}
Lexical getLexi() {
	if (BUFFER.empty()) {
		Lexical l;
		l.type = 0;
		return l;
	}
	Lexical l = BUFFER.front();
	BUFFER.pop();
	return l;
}

char getGlbCh() {
	return glbCh;
}

int getGlbInt() {
	return glbInt1;
}

