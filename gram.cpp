#define _CRT_SECURE_NO_WARNINGS
#include "lib.h"
#include "lexical.h"
#include "table.h"
#include "quad.h"
#include"register.h"
using namespace std;
extern FILE* in, * out;
extern int line;
extern int glbPos;
//extern FILE* out;
char store[5000];
int symbol;
int point = 0;
char dic[37][15] = { "","IDENFR","IFTK","MINU","ASSIGN","INTCON","ELSETK","MULT","SEMICN","CHARCON","DOTK","DIV","COMMA","STRCON","WHILETK","LSS","LPARENT","CONSTTK","FORTK","LEQ","RPARENT","INTTK","SCANFTK","GRE","LBRACK","CHARTK","PRINTFTK","GEQ","RBRACK","VOIDTK","RETURNTK","EQL","LBRACE","MAINTK","PLUS","NEQ","RBRACE" };
char voidTale[100][50] = {};  //有返回的函数表
int voidL = 0;
char nvoidTable[100][50] = {};  //无返回的函数表
int nvoidL = 0;
//int voidFlag = 0;  //void Function flag=1，return function flag=2
int returnCount = 0;
int layer = 0;
char curFunction[100];  //  当前定义函数
stack<string> callFunction;
char curCall[100]; //当前被调用的函数
int expressionInt = 0;// 判断表达式类型
int varSymbol = 0;//刚开始进入varExplain的标志
int label;
char labelTag[20];
int glbInt;
int stringAcc;
char glbString1[100];
int pcbPos;
char stringname[20];
int callLayer = 0;    //函数调用层数，主要针对参数表
int loopLayer = 0;
void labelProduce() {
	char another[18];
	sprintf(another, "%d", label); 
	label++;
	int i;
	labelTag[0] = 'L';
	labelTag[1] = '_';
	for (i = 0; another[i] != '\0'; i++) {
		labelTag[i + 2] = another[i];
	}
	labelTag[i + 2] = '\0';
}

int relation() {
	if (symbol == LSS || symbol == LEQ || symbol == GRE || symbol == GEQ || symbol == NEQ || symbol == EQL) {
		return symbol;
	}
	return 0;
}

char* getCurCall() {
	if (callFunction.empty()) {
		printf("!!!No Current Function!!!\n");
		strcpy(curCall, "");
	}
	else {
		string cur = callFunction.top();
		strcpy(curCall, cur.c_str());
	}
	return curCall;
}
char* genStringName() {
	char behind[19];
	sprintf(behind, "%d", stringAcc);
	stringAcc++;
	stringname[0] = '_';
	int i;
	for (i = 0; behind[i] != '\0'; i++) {
		stringname[i + 1] = behind[i];
	}
	stringname[i + 1] = '\0';
	return stringname;
}
void printLexi(Lexical l) {
	return;
	/*
	if (l.type) {
		printf("%s %s\n", dic[l.type], l.item);
		fprintf(out, "%s %s\n", dic[l.type], l.item);
	}*/
}
void printSeq(const char* str) {
	return;
	//puts(str);
	//fprintf(out, str);
}
void getStore(int count) {
	return;
	/*
	if (count > getLexiSize())
		count = getLexiSize();
	while (count > 0) {
		Lexical item = getLexi();
		if (item.type != 0) {
			printLexi(item);
		}
		count--;
	}*/
}

void clearStore() {
	store[0] = '\0';
	point = 0;
}

int getItem() {
	int type = getsym();
	store[point++] = symbol;
	return type;
}

void add2nvoid(char* str) {
	strcpy(nvoidTable[nvoidL], str);
	nvoidL++;
}

void add2void(char* str) {
	strcpy(voidTale[voidL], str);
	voidL++;
}

int nonInteger() {   //＜无符号整数＞  ::= ＜非零数字＞｛＜数字＞｝| 0 ,进入前读好一串数字，没有读下一个
	if (symbol == INTCON) {
		glbInt = getGlbInt();
		getStore(getLexiSize());
		printSeq("<无符号整数>\n");
		return 1;
	}
	
	return 0;
}

int Integer() {   //＜整数＞        ::= ［＋｜－］＜无符号整数＞ //进入前读好一串数字，没有读下一个
	int negflag = 0;
	if (symbol == PLUS || symbol == MINU) {
		if (symbol == MINU) negflag = 1;
		symbol = getsym();
	}
	if (symbol != INTCON)
		return 0;
	nonInteger();
	char reg1[20];
	strcpy(reg1, getNewReg(layer));
	if (negflag) {
		createQuadInt(-1*glbInt, reg1);
	}
	else {
		createQuadInt(glbInt, reg1);
	}
	putQuadRef(reg1);
	getStore(getLexiSize());
	printSeq("<整数>\n");
	return 1;
}
//＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝"
int String() {   //进入前读好一串
	if (symbol == STRCON) {
		getStore(getLexiSize());
		printSeq("<字符串>\n");
		strcpy(glbString1, getGlbString());
		symbol = getsym();
		return 1;
	}
	return 0;
}
//＜常量定义＞::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}| char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
//=后面只能是整型或字符型常量   //困惑
int constDefine() {  //读到下一个
	idenfr ide;
	if ((symbol != INTTK) && (symbol != CHARTK)) return 0;
	else if (symbol == INTTK) {
		symbol = getsym();
		if (symbol != IDENFR) error('a');
		char name1[50];
		strcpy(name1, getWord());
		char type1[10];
		strcpy(type1, "int");
		ide = findIdenfrCur(name1);
		if (strcmp(ide.name, " ") != 0) error('b');      //名字重定义
		else { 
			addidenfr(name1, type1, 1);
			if (layer > 0) {
				addVar(name1);
				createQuadProcVar(name1);
				addIdenfrUse(name1, 1);
			}
		}
		symbol = getsym();
		//assert(symbol == ASSIGN);
		symbol = getsym();
		if (!Integer()) error('o');
		char src[20];
		strcpy(src, popQuadRef());
		
		createQuad(OperatorNum::ASS, 1, src, src, name1);
		getStore(getLexiSize());  //clear Queue 防止非逗号乱入
		symbol = getsym();
		if (symbol != COMMA) {
			printSeq("<常量定义>\n");
			return 1;
		}
		while (symbol == COMMA) {
			symbol = getsym();
			if (symbol != IDENFR) error('a');
			char name[50];
			strcpy(name, getWord());
			char type[10];
			strcpy(type, "int");
			ide = findIdenfrCur(name);
			if (strcmp(ide.name, " ") != 0) error('b');    //名字重定义
			else { 
				addidenfr(name, type, 1);
				if (layer > 0) {
					addVar(name);
					createQuadProcVar(name);
					addIdenfrUse(name, 1);
				}
			}
			symbol = getsym();
			//assert(symbol == ASSIGN);
			symbol = getsym();
			if (!Integer()) error('o');//Integer中将前面缓存的输出
			char src1[20];
			strcpy(src1, popQuadRef());
			
			createQuad(OperatorNum::ASS, 1, src1, src1, name);
			symbol = getsym();
		}
		printSeq("<常量定义>\n");
		return 1;
	}
	else {
		symbol = getsym();
		if (symbol != IDENFR) error('a');
		char name1[50];
		strcpy(name1, getWord());
		char type1[10];
		strcpy(type1, "char");
		ide = findIdenfrCur(name1);
		if (strcmp(ide.name, " ") != 0) error('b');    //名字重定义
		else { 
			addidenfr(name1, type1, 1); 
			if (layer > 0)
			{
				addVar(name1);
				createQuadProcVar(name1);
				addIdenfrUse(name1, 1);
			}
		}
		symbol = getsym();
		//assert(symbol == ASSIGN);
		symbol = getsym();
		if (symbol != CHARCON) error('o');
		char src1[20];
		strcpy(src1, getNewReg(layer));
		createQuadChar(getGlbCh(), src1);
		createQuad(OperatorNum::ASS, 1, src1, src1, name1);
		
		getStore(getLexiSize());
		symbol = getsym();
		if (symbol != COMMA) {
			printSeq("<常量定义>\n");
			return 1;
		}
		while (1) {
			symbol = getsym();
			if (symbol != IDENFR) error('a');
			char name[50];
			strcpy(name, getWord());
			char type[10];
			strcpy(type, "char");
			ide = findIdenfrCur(name);
			if (strcmp(ide.name, " ") != 0) error('b');    //名字重定义
			else {
				addidenfr(name, type, 1);
				if (layer > 0) {
					addVar(name);
					createQuadProcVar(name);
					addIdenfrUse(name, 1);
				}
			}
			symbol = getsym();
			//assert(symbol == ASSIGN);
			symbol = getsym();
			if (symbol != CHARCON) error('o');
			char src[20];
			strcpy(src, getNewReg(layer));
			createQuadChar(getGlbCh(), src);
			createQuad(OperatorNum::ASS, 1, src, src, name);
			
			getStore(getLexiSize());
			symbol = getsym();
			if (symbol != COMMA) {
				printSeq("<常量定义>\n");
				return 1;
			}
		}
	}
	return 0;
}
//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}     //未打印
int constExplain() {
	if (symbol != CONSTTK) return 0;
	symbol = getsym();
	if (constDefine()) {
		if (symbol != SEMICN) {
			error('k');
			while (symbol == CONSTTK) {
				symbol = getsym();
				constDefine();
				if (symbol != SEMICN) error('k');
				else symbol = getsym();
			}
		}
		else {
			symbol = getsym();
			while (symbol == CONSTTK) {
				symbol = getsym();
				constDefine();
				if (symbol != SEMICN) error('k');
				else symbol = getsym();
			}
		}
		printSeq("<常量说明>\n");
		return 1;
	}
	return 0;
}
//变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']'){,(＜标识符＞|＜标识符＞'['＜无符号整数＞']' )}
// 从标识符后一位开始 可能是 ;[
int varDefine() {  //区别和有返回函数
	idenfr ide;
	int intflag = 0;
	if ((symbol != CHARTK) && (symbol != INTTK)) return 0;
	if (symbol == INTTK)
		intflag = 1;
	getStore(getLexiSize() - 1);//print 前面的分号，不print int和char标志
	symbol = getsym();
	char name[50];
	strcpy(name, getWord());
	if (symbol == IDENFR) {
		symbol = getsym();
		if (symbol == LPARENT) {  //识别出返回函数
			createGlbEnd();
			getStore(getLexiSize() - 3);
			printSeq("<变量说明>\n");//把前面变量定义都打印出来
			getStore(getLexiSize() - 1);
			printSeq("<声明头部>\n");
			strcpy(curFunction, name);
			createQuadFuncBegin(name);
			createQuadLabel(name);
			idenfr idf = findIdenfrCur(name);
			if (strcmp(idf.name, " ") != 0) error('b');
			Function f = getFunction(name);
			if (strcmp(f.name, " ") != 0) error('b');   //名字重定义
			else {
				if (intflag) initFunction(name, INTCON);
				else initFunction(name, CHARCON);
			}
			add2nvoid(name);
			return 0;
		}
		else if (symbol == LBRACK) {
			char type[10];
			if (intflag)
				strcpy(type, "lisint");
			else
				strcpy(type, "lischar");
			ide = findIdenfrCur(name);
			if (strcmp(ide.name, " ") != 0) error('b');
			else addidenfr(name, type, 0);
			getStore(getLexiSize());
			symbol = getsym();
			if (!nonInteger()) {
				error('i');      //不是整型表达式or不符合词法？？？
			}
			if (layer == 0) {
				fprintf(out, "%s: .space %d\n", name, glbInt * 4);
				glbPos += glbInt;
				addGlbArr(name);
			}
			else {
				addArray(curFunction, name, pcbPos);
				pcbPos += glbInt;
			}
			symbol = getsym();
			if (symbol != RBRACK) {
				error('m');
				getStore(getLexiSize() - 1);
			}
			else {
				getStore(getLexiSize());
				symbol = getsym();
			}
		}
		else {
			char type[10];
			if (intflag)
				strcpy(type, "int");
			else
				strcpy(type, "char");
			ide = findIdenfrCur(name);
			if (strcmp(ide.name, " ") != 0) error('b');
			else {
				addidenfr(name, type, 0);
				if (layer > 0) {
					addVar(name);
					createQuadProcVar(name);
				}
				else createQuadGlbVar(name);
			}
		}
	}
	while (symbol == COMMA) {
		getStore(getLexiSize() - 1);
		symbol = getsym();
		//assert(symbol == IDENFR);
		char name[50];
		strcpy(name, getWord());
		if (symbol == IDENFR) {
			symbol = getsym();
			if (symbol == LPARENT) {
				createGlbEnd();
				getStore(getLexiSize() - 3);
				printSeq("<变量说明>\n");//把前面变量定义都打印出来
				getStore(getLexiSize() - 1);
				printSeq("<声明头部>\n");
				strcpy(curFunction, name);
				createQuadFuncBegin(name);
				createQuadLabel(name);
				idenfr idf = findIdenfrCur(name);
				if (strcmp(idf.name, " ") != 0) error('b');
				Function f = getFunction(name);
				if (strcmp(f.name, " ") != 0) error('b');   //名字重定义
				else {
					if (intflag) initFunction(name, INTCON);
					else initFunction(name, CHARCON);
				}
				add2nvoid(name);
				return 0;
			}
			if (symbol == LBRACK) {
				char type[10];
				if (intflag)
					strcpy(type, "lisint");
				else
					strcpy(type, "lischar");
				ide = findIdenfrCur(name);
				if (strcmp(ide.name, " ") != 0) error('b');
				else addidenfr(name, type, 0);
				getStore(getLexiSize());
				symbol = getsym();
				if (!nonInteger()) error('i');
				if (layer == 0) {
					fprintf(out, "%s: .space %d\n", name, glbInt * 4);
					glbPos += glbInt;
					addGlbArr(name);
				}
				else {
					addArray(curFunction, name, pcbPos);
					pcbPos += glbInt;
				}
				symbol = getsym();
				if (symbol != RBRACK) {
					error('m');
					getStore(getLexiSize() - 1);
				}
				else {
					getStore(getLexiSize());
					symbol = getsym();
				}
			}
			else {
				char type[10];
				if (intflag)
					strcpy(type, "int");
				else
					strcpy(type, "char");
				ide = findIdenfrCur(name);
				if (strcmp(ide.name, " ") != 0) error('b');
				else {
					addidenfr(name, type, 0);
					if (layer > 0)
					{
						addVar(name);
						createQuadProcVar(name);
					}
					else createQuadGlbVar(name);
				}
			}
		}
		else {
			error('a');
		}
	}
	getStore(getLexiSize() - 1);
	printSeq("<变量定义>\n");
	//if (symbol != SEMICN) error('k');
	return 1;
}
// 过滤掉类型标志符,标志符,直接以下一位进入,如果是(则是返回函数
int varExplain() { //＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
	if (symbol == LBRACK) {   //第一个已经加到符号表了
		getStore(getLexiSize());
		symbol = getsym();
		if (!nonInteger()) error('i');
		if (layer == 0) {
			fprintf(out, " %d\n", glbInt * 4);
			glbPos += glbInt;
		}
		else {
			
			pcbPos += glbInt;
		}
		symbol = getsym();
		if (symbol != RBRACK) {
			error('m');
			getStore(getLexiSize() - 1);
		}
		else {
			getStore(getLexiSize());
			symbol = getsym();
		}
	}
	else {
		getStore(getLexiSize() - 1);//int a;
	}
	if (symbol == COMMA) {
		while (symbol == COMMA) {
			symbol = getsym();
			if (symbol != IDENFR) error('a');
			char name[50];
			strcpy(name, getWord());
			idenfr idf = findIdenfrCur(name);
			symbol = getsym();
			if (symbol == LBRACK) {
				char type[10];
				if (varSymbol == 1) {
					strcpy(type, "lisint");
				}
				else strcpy(type, "lischar");
				if (strcmp(idf.name, " ") != 0) error('b');
				else addidenfr(name, type, 0);
				getStore(getLexiSize());
				symbol = getsym();
				if (!nonInteger()) error('i');
				if (layer == 0) {
					fprintf(out, "%s: .space %d\n", name, glbInt * 4);
					glbPos += glbInt;
					addGlbArr(name);
				}
				else {
					addArray(curFunction, name, pcbPos);
					pcbPos += glbInt;
				}
				symbol = getsym();
				if (symbol != RBRACK) {
					error('m');
					getStore(getLexiSize() - 1);
				}
				else {
					getStore(getLexiSize());
					symbol = getsym();
				}
			}
			else {
				char type[10];
				if (varSymbol) strcpy(type, "int");
				else strcpy(type, "char");
				if (strcmp(idf.name, " ") != 0) error('b');
				else {
					addidenfr(name, type, 0);
					if (layer > 0)
					{
						addVar(name);
						createQuadProcVar(name);
					}
					else createQuadGlbVar(name);
				}
				if (symbol == SEMICN) {
					getStore(getLexiSize() - 1);
					break;
				}
			}
		}
	}
	printSeq("<变量定义>\n");
	if (symbol != SEMICN) error('k');
	else {
		getStore(getLexiSize());//;
		symbol = getsym();
	}
	while (symbol == INTTK || symbol == CHARTK) {
		if (!varDefine()) {
			return 0;
		}
		getStore(getLexiSize() - 1);
		if (symbol != SEMICN) error('k');
		else symbol = getsym();
	}
	getStore(getLexiSize() - 1);
	printSeq("<变量说明>\n");  //void function or 其他语句
	return 1;
}

void parameterTable() {  ///＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞ 
	//assert((symbol == INTTK) || (symbol == CHARTK));
	//参数表声明类型也要加到符号表！！此时已经newTable完成
	int isInt = 0;
	int count = 0;
	if (symbol == INTTK) {
		addParam(curFunction, INTCON);
		isInt = 1;
	}
	else addParam(curFunction, CHARCON);
	symbol = getsym();
	if (symbol != IDENFR) error('a');
	char name[50];
	strcpy(name, getWord());
	idenfr ide = findIdenfrCur(name);
	if (strcmp(ide.name, " ") != 0) error('b');      //名字重定义
	else {
		addVar(name);
		createQuadProcParaVar(name);
		createQuadPara(name, curFunction);
		char type[10];
		if (isInt) {
			strcpy(type, "int");
			isInt = 0;
		}
		else strcpy(type, "char");
		addidenfr(name, type, 0);
		addIdenfrUse(name, 1);
	}
	symbol = getsym();
	while (symbol == COMMA) {
		count++;
		symbol = getsym();
			//assert((symbol == INTTK) || (symbol == CHARTK));
		if (symbol == INTTK) {
			addParam(curFunction, INTCON);
			isInt = 1;
		}
		else addParam(curFunction, CHARCON);
		symbol = getsym();
		if (symbol != IDENFR) error('a');
		char name[50];
		strcpy(name, getWord());
		idenfr ide = findIdenfrCur(name);
		if (strcmp(ide.name, " ") != 0) error('b');      //名字重定义
		else {
			addVar(name);
			createQuadProcParaVar(name);
			createQuadPara(name, curFunction);
			char type[10];
			if (isInt) {
				strcpy(type, "int");
				isInt = 0;
			}
			else strcpy(type, "char");
			addidenfr(name, type, 0);
			if (count<=3)
				addIdenfrUse(name, 1);
		}
		symbol = getsym();
	}
	getStore(getLexiSize() - 1);
	printSeq("<参数表>\n");
}

void step() {
	nonInteger();
	char reg1[20];
	strcpy(reg1, getNewReg(layer));
	createQuadInt(getGlbInt(), reg1);
	putQuadRef(reg1);
	printSeq("<步长>\n");
	symbol = getsym();
}
//＜因子＞::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞:＜标识符＞'('＜值参数表＞')'
int factor() {
	int flag = 0;
	if (symbol == IDENFR) {
		char name[50];
		strcpy(name, getWord());      //查表判断标志符是否定义过，查找标识符类型
		symbol = getsym();
		if (symbol == LBRACK) {     //lis型
			int old = 0;
			idenfr id = findIdenfr(name);
			if (strcmp(id.name, " ") == 0) {
				error('c');
			}//未定义
			else {
				if (strcmp(id.type, "lisint") == 0)    //如果不是list型？？？
					expressionInt = 1;	
			}
			old = expressionInt;
			symbol = getsym();
			int type = expression();
			expressionInt = old;
			if (type == CHARCON) error('i');    //数组下标只能是整型表达式
			if (symbol != RBRACK) error('m');
			//出现在表达式中的lis元素都是取值
			else symbol = getsym();
			char inside[20];
			char reg[20];
			strcpy(inside, popQuadRef());
			strcpy(reg,getNewReg(layer));  
			//数组名统一在？？？？？数组也不能全部放在fp中，
			//不同函数中数组重名
			//死亡！！！
			createQuad(OperatorNum::ASTK, 2, reg, inside, name);  //取数组值，保存在寄存器中
			putQuadRef(reg);
		}
		else if (symbol == LPARENT) { //当前symbol指向( 与assign区分  
			string sName = name;
			callFunction.push(sName);

			//strcpy(curCall, name);   //函数未定义问题已经在valueTable中解决
			//表达式中时无返回语句？
			//Function func = getFunction(name);
			returnCall();           //expressionInt  判断在valueTable中进行
			
		}
		else {
			idenfr id = findIdenfr(name);
			
			if (strcmp(id.name, " ") == 0) {
				error('c');
			}//未定义
			else {
				if (strcmp(id.type, "int") == 0)    //如果是list型？？？
					expressionInt = 1;
				idenfr idf = findIdenfrCur(name);
				addIdenfrUse(name, 1 + loopLayer * 10);
				if (strcmp(idf.name, " ") == 0)    //是全局变量，为了防止改变，赋给另一个临时变量
				{
					flag = 1;
					/*char reg1[20];
					strcpy(reg1, getNewReg(layer));
					createQuad(OperatorNum::ASS, 1, name, name, reg1);   //保住左值
					putQuadRef(reg1);*/
				}
			}
			//if (flag == 0) {
				putQuadRef(name);                   //加入ref表
			//}
		}
	}
	else if (symbol == LPARENT) {    //括号中一定转为int型
		int old;
		expressionInt = 1;       //此时表达式的值已经在quadList栈顶
		old = expressionInt;
		symbol = getsym();
		if (expression() == INTCON) {
			expressionInt = 1;
		}
		expressionInt = old;
		if (symbol != RPARENT) error('l');
		else symbol = getsym();
	}
	else if (symbol == CHARCON) {    //
		symbol = getsym();
		char reg[20];
		strcpy(reg, getNewReg(layer));
		createQuadChar(getGlbCh(), reg);
		putQuadRef(reg);
	}
	else if (symbol == INTCON || symbol == PLUS || symbol == MINU) {
		expressionInt = 1;
		Integer();
		symbol = getsym();
	}
	else error('a');
	getStore(getLexiSize() - 1);
	printSeq("<因子>\n");
	return flag;
}

int term() {  //＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}
	int glb;
	glb = factor();                              //没有出现×÷就不需要生成四元式
	while (symbol == MULT || symbol == DIV) {   //字符型一旦参与运算则转换成整型，包括小括号括起来的字符型，也算参与了运算
		int multflag = 0;
		if (symbol == MULT)
			multflag = 1;
		char src1[20];
		char reg1[20];
		strcpy(src1, popQuadRef());
		if (glb) {
			//char reg1[20];
			strcpy(reg1, getNewReg(layer));
			createQuad(OperatorNum::ASS, 1, src1, src1, reg1);
		}
		expressionInt = 1;
		symbol = getsym();
		factor();
		char src2[20];
		strcpy(src2, popQuadRef());
		char dst[20];
		strcpy(dst, getNewReg(layer));
		if (multflag) {
			if (glb)
			{
				createQuad(OperatorNum::MULTI, 2, reg1, src2, dst);
				glb = 0;
			}
			else
			    createQuad(OperatorNum::MULTI, 2, src1, src2, dst);
		}
		else {
			if (glb)
			{
				createQuad(OperatorNum::DIVI, 2, reg1, src2, dst);
				glb = 0;
			}
		    else
			    createQuad(OperatorNum::DIVI, 2,src1, src2, dst);
		}
		putQuadRef(dst);
	}
	getStore(getLexiSize() - 1);
	printSeq("<项>\n");
	return glb;
}

int expression() {  //＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}   // 返回INTCON或CHARCON
	//expressionInt = 0;
	int negFlag = 0;
	int glb;
	if (symbol == PLUS || symbol == MINU) {     //字符型一旦参与运算则转换成整型，包括小括号括起来的字符型，也算参与了运算
		expressionInt = 1;
		if (symbol == MINU) negFlag = 1;
		symbol = getsym();
	}
	glb = term();
	if (negFlag) {
		char reg1[20];
		strcpy(reg1, popQuadRef());
		createQuad(OperatorNum::NEG, 1, reg1, reg1, reg1);
		putQuadRef(reg1);
	}
	while (symbol == PLUS || symbol == MINU) {
		int addFlag = (symbol == PLUS);
		expressionInt = 1;
		char src1[20];
		char reg1[20];
		strcpy(src1, popQuadRef());
		if (glb) {
			//char reg1[20];
			strcpy(reg1, getNewReg(layer));
			createQuad(OperatorNum::ASS, 1, src1, src1, reg1);   //保住左值
		}
		symbol = getsym();
		term();
		char src2[20];
		strcpy(src2, popQuadRef());
		char dst[20];
		strcpy(dst, getNewReg(layer));
		if (addFlag) {
			if (glb) {
				createQuad(OperatorNum::ADD, 2, reg1, src2, dst);
				glb = 0;
			}
			else {
				createQuad(OperatorNum::ADD, 2, src1, src2, dst);
			}
		}
		else {
			if (glb) {
				createQuad(OperatorNum::SUB, 2, reg1, src2, dst);
				glb = 0;
			}
			else {
				createQuad(OperatorNum::SUB, 2, src1, src2, dst);
			}
		}
		putQuadRef(dst);
	}
	getStore(getLexiSize() - 1);
	printSeq("<表达式>\n");
	if (expressionInt) {
		expressionInt = 0;
		return INTCON;
	}
	else return CHARCON;
}

int condition(int mark) {  //＜表达式＞＜关系运算符＞＜表达式＞|<表达式>  表达式必须时整型
	int type;
	type = expression();
	if (type == CHARCON) error('f');  //条件判断中出现不合法的类型
	char src1[20];
	strcpy(src1, popQuadRef());
	//labelProduce();
	int reType = relation();
	if (reType) {
		symbol = getsym();
		type = expression();
		if (type == CHARCON) error('f');
		char src2[20];
		strcpy(src2, popQuadRef());
		if (mark == 1) {
			if (reType == LSS) createQuad(OperatorNum::BGE, 2, src1, src2, labelTag);
			if (reType == LEQ) createQuad(OperatorNum::BG, 2, src1, src2, labelTag);
			if (reType == GRE) createQuad(OperatorNum::LSE, 2, src1, src2, labelTag);
			if (reType == GEQ) createQuad(OperatorNum::LS, 2, src1, src2, labelTag);
			if (reType == NEQ) createQuad(OperatorNum::EQUAL, 2, src1, src2, labelTag);
			if (reType == EQL) createQuad(OperatorNum::NEQUAL, 2, src1, src2, labelTag);
		}
		else {
			if (reType == LSS) createQuad(OperatorNum::LS, 2, src1, src2, labelTag);
			if (reType == LEQ) createQuad(OperatorNum::LSE, 2, src1, src2, labelTag);
			if (reType == GRE) createQuad(OperatorNum::BG, 2, src1, src2, labelTag);
			if (reType == GEQ) createQuad(OperatorNum::BGE, 2, src1, src2, labelTag);
			if (reType == NEQ) createQuad(OperatorNum::NEQUAL, 2, src1, src2, labelTag);
			if (reType == EQL) createQuad(OperatorNum::EQUAL, 2, src1, src2, labelTag);
		}
	}
	else {                        //和0比较
		if (mark == 1) {
			createQuad(OperatorNum::BZ, 1, src1, src1, labelTag);
		}
		else {
			createQuad(OperatorNum::BNZ, 1, src1, src1, labelTag);
		}
	}
	getStore(getLexiSize() - 1);
	printSeq("<条件>\n");
	return 1;
}

int conditionStatement() {  // if '('＜条件＞')'＜语句＞［else＜语句＞］
	if (symbol != IFTK) return 0;
	symbol = getsym();                        //if (!条件） bne,src1,src2,label1
	//assert(symbol == LPARENT);              //if 语句段 //goto Label2
	symbol = getsym();                        //Label1:
	labelProduce();                          //[else 语句段]
	condition(1);                             //
	if (symbol != RPARENT) error('l');       //Label2:
    else symbol = getsym();                  //
	char labelOld[20];                        //Label1
	strcpy(labelOld, labelTag);
	labelProduce();                      
	char label2[20];
	strcpy(label2, labelTag);
	statement();  
	createQuadGoto(label2);             //goto Label2
	createQuadLabel(labelOld);         //Label1:
	if (symbol == ELSETK) {
		symbol = getsym();
		statement();
	}
	createQuadLabel(label2);            //Label2 声明
	getStore(getLexiSize() - 1);
	printSeq("<条件语句>\n");
	return 1;
}
// while '('＜条件＞')'＜语句＞| 
//do＜语句＞while '('＜条件＞')' |for'('＜标识符＞＝＜表达式＞;
//＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞      

int loopStatement() {
	if (symbol != WHILETK && symbol != DOTK && symbol != FORTK) return 0;
	if (symbol == WHILETK) {                              //Label0:
		symbol = getsym();                                 //（条件） Bne src1，src2，Label1
		//assert(symbol == LPARENT);                      
		getStore(getLexiSize());                         
		symbol = getsym();                                //语句段
		labelProduce();      //produce Label0            //goto Label0
		createQuadLabel(labelTag);                      //Label1:
		char Label0[20];
		strcpy(Label0, labelTag); 
		labelProduce();           //produce Label1
		loopLayer++;
		condition(1);                                     
		char Label1[20];
		strcpy(Label1, labelTag);
		if (symbol != RPARENT) error('l');             
		else symbol = getsym();
		statement();
		loopLayer--;
		createQuadGoto(Label0);                  //goto Label0
		createQuadLabel(Label1);
	}
	else if (symbol == DOTK) {                     //Label1:
		getStore(getLexiSize());                   //语句段
		symbol = getsym();                         //bne src1,src2,label1
		labelProduce();
		createQuadLabel(labelTag);
		char labelNow[20];
		strcpy(labelNow, labelTag);
		loopLayer++;
		statement();
		strcpy(labelTag, labelNow);
		if (symbol != WHILETK) error('n');
		else symbol = getsym();
		//assert(symbol = LPARENT);
		getStore(getLexiSize());
		symbol = getsym();
		condition(0);
		loopLayer--;
		if (symbol != RPARENT) {
			error('l');
			getStore(getLexiSize() - 1);
		}
		else {
			getStore(getLexiSize());
			symbol = getsym();
		}
	}
	else if (symbol == FORTK) {                   //你是魔鬼吧
		symbol = getsym();                       //
		//assert(symbol == LPARENT);
		symbol = getsym();
		if (symbol != IDENFR) error('a');
		char name[50];
		strcpy(name, getWord());
		int checkType = 0;
		idenfr idf = findIdenfr(name);
		if (strcmp(idf.name, " ") == 0) {
			error('c');
		}//未定义符号
		else {
			if (idf.isConst == 1) error('j');          //不能改变常量的值
			checkType = 1;
			addIdenfrUse(name, 1);
		}
		symbol = getsym();
		//assert(symbol == ASSIGN);
		symbol = getsym();
		int asType = expression();
		char src1[20];
		strcpy(src1, popQuadRef());
		createQuad(OperatorNum::ASS, 1, src1, src1, name);     //idenfr=expression
		if (checkType) {
			if (strcmp(idf.type, "int") == 0) {
				if (asType != INTCON)
					error('e');                //类型不符合
			}
			else {
				if (strcmp(idf.type, "char") == 0) {
					if (asType != CHARCON) error('e');   //类型不符合
				}
			}
		}
		if (symbol != SEMICN) error('k');
		else symbol = getsym();
		labelProduce();                     //Laabel0:
		createQuadLabel(labelTag);
		char label0[20];
		strcpy(label0, labelTag);
		labelProduce();             //produce Label1
		loopLayer++;
		condition(1);                //bne src1,src2,label1
		char label1[20];
		strcpy(label1, labelTag);
		//for'('＜标识符＞＝＜表达式＞;
		//＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞   
		if (symbol != SEMICN) error('k');
		else symbol = getsym();
		char name1[20];
		if (symbol != IDENFR) error('a');
		strcpy(name1, getWord());          //标志符1
		idf = findIdenfr(name1);
		if (strcmp(idf.name, " ") == 0) {
			error('c');
		}//未定义符号
		else {
			if (idf.isConst == 1) error('j');          //不能改变常量的值
			addIdenfrUse(name1, 1 + loopLayer * 10);
		}
;		symbol = getsym();
		//assert(symbol == ASSIGN);
		symbol = getsym();
		if (symbol != IDENFR) error('a');
		strcpy(name, getWord());     //new Name
		idf = findIdenfr(name);
		if (strcmp(idf.name, " ") == 0) {
			error('c');
		}//未定义符号
		else {
			addIdenfrUse(name, 1 + loopLayer * 10);
		}
		symbol = getsym();
		int minu = (symbol == MINU);
		//assert(symbol == MINU || symbol == PLUS);
		symbol = getsym();
		step();
		char stepCount[20];
		strcpy(stepCount, popQuadRef());     //在Integer中压入
		if (symbol != RPARENT) error('l');
		else symbol = getsym();
		statement();                         //语句段
		loopLayer--;
		if (minu)
			createQuad(OperatorNum::SUB, 2, name, stepCount, name1);
		else
			createQuad(OperatorNum::ADD, 2, name, stepCount, name1);
		createQuadGoto(label0);
		createQuadLabel(label1);
	}
	getStore(getLexiSize() - 1);
	printSeq("<循环语句>\n");
	return 1;
}

int readStatement() { //scanf '('＜标识符＞{,＜标识符＞}')'
	//assert(symbol == LPARENT);
	symbol = getsym();
	if (symbol != IDENFR) error('a');
	char name[50];
	strcpy(name, getWord());
	//createQuadScanf(name);
	idenfr idf = findIdenfr(name);
	if (strcmp(idf.name, " ") == 0)
	{
		error('c');
	}//未定义符号
	else {
		if (idf.isConst == 1) error('j');          //不能改变常量的值
		addIdenfrUse(name, 1 + loopLayer * 10);
	}
	int isInt = 0;
	if ((strcmp(idf.type, "int") == 0) || (strcmp(idf.type, "lisint") == 0))
		isInt = 1;
	createQuadScanf(name, isInt);
	symbol = getsym();
	while (symbol == COMMA) {
		symbol = getsym();
		if (symbol != IDENFR) error('a');
		strcpy(name, getWord());
		//createQuadScanf(name);
		idenfr idf = findIdenfr(name);
		isInt = 0;
		if (strcmp(idf.name, " ") == 0)
		{
			error('c');
		}//未定义符号
		else {
			if (idf.isConst == 1) error('j');          //不能改变常量的值
			addIdenfrUse(name, 1 + loopLayer * 10);
		}
		if ((strcmp(idf.type, "int") == 0) || (strcmp(idf.type, "lisint") == 0))
			isInt = 1;
		createQuadScanf(name, isInt);
		symbol = getsym();
	}
	if (symbol != RPARENT) {
		error('l');
		getStore(getLexiSize() - 1);
	}
	else {
		getStore(getLexiSize());
		symbol = getsym();
	}
	printSeq("<读语句>\n");
	return 1;
}
//＜写语句＞  printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
int writeStatement() {
	//assert(symbol == LPARENT);
	int wrong = 0;
	symbol = getsym();
	if (symbol == STRCON) {
		String();
		char strName[20];
		strcpy(strName, genStringName());
		putString(strName, glbString1);
		//fprintf(out, "%s:.asciiz\"%s\"\n", strName, glbString1);
		createQuadPrintf(strName, -1);
		if (symbol == COMMA) {
			symbol = getsym();
			char name[50];
			strcpy(name, getWord());
			int expType=expression();
			char ref[20];
			strcpy(ref, popQuadRef());
			if (expType == INTCON) {
				createQuadPrintf(ref, 1);
			}
			else {
				createQuadPrintf(ref,0);
			}
		}
		if (symbol != RPARENT) {
			error('l');
			wrong = 1;
		}
	}
	else {
		int expType = expression();
		char ref[20];
		strcpy(ref, popQuadRef());
		if (expType == INTCON)
			createQuadPrintf(ref, 1);
		else
			createQuadPrintf(ref, 0);
		if (symbol != RPARENT) {
			error('l');
			wrong = 1;
		}
	}
	if (wrong) {
		getStore(getLexiSize() - 1);
	}
	else {
		getStore(getLexiSize());
		symbol = getsym();
	}
	createQuadPrintfEnd();
	printSeq("<写语句>\n");
	return 1;
}
// return['('＜表达式＞')']   
int returnStatement() { //当前symbol指向return后
	returnCount++;
	Function func = getFunction(curFunction);
	if (symbol == LPARENT) {
		//returnCount++;
		//if (voidFlag == 1) error('g');    //无返回值的函数存在不匹配的return语句
		symbol = getsym();
		int exptype = expression();
		char expReturn[20];
		strcpy(expReturn, popQuadRef());
		createQuadReturnValue(expReturn);
		if (func.type == VOIDTK) error('g');//无返回值的函数存在不匹配的return语句
		else if (func.type != exptype) error('h');//有返回值的函数存在不匹配的return语句
		if (symbol != RPARENT) error('l');
		else symbol = getsym();
	}
	else {
		if (func.type != VOIDTK)//有返回值的函数存在不匹配的return语句
			error('h');
	}
	if (strcmp(curFunction,"main")!=0)
		createQuadReturn();
	getStore(getLexiSize() - 1);
	printSeq("<返回语句>\n");
	return 1;
}
//＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
int assignStatement(char* nameRef) { //当前指向assign 或者中括号   //只需要生成四元式，结果不需要加到refList中
	int assType;
	if (symbol == ASSIGN) {
		symbol = getsym();
		assType = expression();
		char src1[20];
		strcpy(src1, popQuadRef());
		createQuad(OperatorNum::ASS, 1, src1, src1, nameRef);
	}
	else {
		//assert(symbol == LBRACK);
		symbol = getsym();
		int type = expression();
		if (type == CHARCON) error('i');    //数组下标只能是整型表达式
		if (symbol != RBRACK) error('m');
		else symbol = getsym();
		char src2[20];     //inside               // 
		strcpy(src2, popQuadRef());
		//assert(symbol == ASSIGN);
		symbol = getsym();
		assType = expression();
		char src1[20];                            //偏移
		strcpy(src1, popQuadRef());
		createQuad(OperatorNum::ASAR, 2, src1, src2, nameRef);
	}
	printSeq("<赋值语句>\n");
	return assType;
}


//a(当前symbol指向( ＜标识符＞'('＜值参数表＞')'
int returnCall() {
	symbol = getsym();
	valueTable();      //PARA 结束
	if (symbol != RPARENT) {
		error('l');
		getStore(getLexiSize() - 1);
	}
	else {
		getStore(getLexiSize());
		symbol = getsym();
	}
	createQuadJump(getCurCall());
	if (callFunction.empty()) {
		printf("!!!!!WHY ARE YOU EMPTY???\n");
	}
	else {
		callFunction.pop();
	}
	printSeq("<有返回值函数调用语句>\n");
	char reg1[20];
	strcpy(reg1, getNewReg(layer));
	putQuadRef(reg1);
	createQuadFuncCall(reg1);
	return 1;
}
//＜标识符＞'('＜值参数表＞')' 当前symbol指向(
int nreturnCall() {
	symbol = getsym();
	valueTable();
	if (symbol != RPARENT) {
		error('l');
		getStore(getLexiSize() - 1);
	}
	else {
		getStore(getLexiSize());
		symbol = getsym();
	}
	createQuadJump(getCurCall());
	if (callFunction.empty()) {
		printf("!!!!!WHY ARE YOU EMPTY???\n");
	}
	else {
		callFunction.pop();
	}
	symbol = getsym();
	printSeq("<无返回值函数调用语句>\n");
	return 1;
}

int valueTable() {//＜表达式＞{,＜表达式＞}｜＜空＞
	int exptype;
	int functype;
	int count = 0;
	int count_func = 0;
	int func_param_count = 0;
	int flag = 0;
	int old = 0;
	int inside = 0;
	callLayer++;
	if (callLayer>1)
		inside = 1;
	Function cur_func = getFunction(getCurCall());
	if (strcmp(cur_func.name, " ") == 0)
	{
		error('c');                    //函数未定义
		flag = 1;
	}
	else {
		func_param_count = cur_func.parCount;
		if (cur_func.type == INTCON)     //expression中returnCall
			old = 1;
	}
	if (symbol == RPARENT) {
		getStore(getLexiSize() - 1);
		printSeq("<值参数表>\n");
		if (old == 1)
			expressionInt = old;
		if (!flag) {                   //函数未定义则忽略
			if (count != func_param_count) {
				error('d');
			}//函数参数个数不匹配
		}
		return 1;
	}
	else {
		exptype = expression();
		char popName[20];
		strcpy(popName, popQuadRef());
		createQuadPush(popName,getCurCall(),count,callLayer);
		if (count < func_param_count) {
			if (cur_func.table[count_func] != exptype)
				error('e');
			count_func++;
		}
		count++;
		while (symbol == COMMA) {
			symbol = getsym();
			exptype = expression();
			strcpy(popName, popQuadRef());
			createQuadPush(popName,getCurCall(),count,1);
			if (count < func_param_count) {   //函数未定义则count和func_param_count 都是0
				if (cur_func.table[count_func] != exptype)
					error('e');   //函数参数类型不匹配
				count_func++;
			}
			count++;
		}
		if (!flag) {                   //函数未定义则忽略
			if (count != func_param_count) {
				error('d');
			}//函数参数个数不匹配
		}
		if (old == 1) {
			expressionInt = old;
		}
		getStore(getLexiSize() - 1);
		printSeq("<值参数表>\n");
		callLayer--;
		return 1;
	}
}
//＜条件语句＞｜＜循环语句＞| '{'＜语句列＞'}'| ＜有返回值函数调用语句＞; 
// ＜无返回值函数调用语句＞; ｜＜赋值语句＞; ｜＜读语句＞; ｜＜写语句＞; ｜＜空＞; | ＜返回语句＞;
int statement() {
	int flag = 0;
	if (symbol == RBRACE) return false;
	if (symbol == SEMICN) {  //＜空＞; 
		getStore(getLexiSize());
		printSeq("<语句>\n");
		symbol = getsym();
		return 1;
	}
	else if (symbol == LBRACE) {
		symbol = getsym();
		if (symbol == RBRACE) {
			getStore(getLexiSize() - 1);
			printSeq("<语句列>\n");
			getStore(getLexiSize());
			printSeq("<语句>\n");
			symbol = getsym();
			return 1;
		}
		statementSequence();
		//assert(symbol == RBRACE);
		symbol = getsym();
		flag = 1;
	}
	else if (symbol == IFTK) {
		conditionStatement();
		flag = 1;
	}
	else if (symbol == WHILETK || symbol == DOTK || symbol == FORTK) {
		loopStatement();
		flag = 1;
	}
	else if (symbol == SCANFTK) {
		flag = 1;
		symbol = getsym();
		readStatement();
		if (symbol != SEMICN) error('k');
		else symbol = getsym();
	}
	else if (symbol == PRINTFTK) {
		flag = 1;
		symbol = getsym();
		writeStatement();
		if (symbol != SEMICN) error('k');
		else symbol = getsym();
	}
	else if (symbol == RETURNTK) {
		flag = 1;
		symbol = getsym();
		returnStatement();
		if (symbol != SEMICN) error('k');
		else symbol = getsym();
	}
	else if (symbol == IDENFR) {//赋值=or[ 调用(
		flag = 1;
		char name[50];
		strcpy(name, getWord());
		symbol = getsym();
		int checkType = 0;
		if ((symbol == ASSIGN) || (symbol == LBRACK)) {
			idenfr ide = findIdenfr(name);
			if (strcmp(ide.name, " ") == 0) {
				error('c');
			}//未定义
			else {
				if (ide.isConst == 1) error('j');         //不能改变常量的值
				checkType = 1;
				if (symbol == ASSIGN)
					addIdenfrUse(name, 1 + loopLayer * 10);
			}
			int assType = assignStatement(ide.name);
			if (checkType) {
				if (strcmp(ide.type, "int") == 0 || strcmp(ide.type, "lisint") == 0) {
					if (assType != INTCON) error('e');        //赋值语句类型不匹配
				}
				else {
					if (assType != CHARCON) error('e');
				}
			}
			if (symbol != SEMICN) error('k');
			else symbol = getsym();
		}
		else {  //函数调用,判断是return还是nonreturn
			//symbol == LPARENT);
			string sName = name;
			callFunction.push(sName);
			int found = 0;
			int i = 0;
			for (i = 0; i < nvoidL; i++) {
				if (found == 1) {
					break;
				}
				if (strcmp(nvoidTable[i], name) == 0) {
					found = 1;
					returnCall();
				}
			} if (found == 0) { nreturnCall(); }
			if (symbol != SEMICN) error('k');
			else symbol = getsym();
		}
	}
	if (flag) {
		getStore(getLexiSize() - 1);
		printSeq("<语句>\n");
		return 1;
	}
	return 0;
}

int statementSequence() { //｛＜语句＞｝
	while (statement()) {
		;
	}
	printSeq("<语句列>\n");
	return 1;
}
//＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
int combineStatement() {
	if (symbol == CONSTTK) {
		constExplain();
	}
	if (symbol == INTTK || symbol == CHARTK) {
		int intFlag = (symbol == INTTK);
		symbol = getsym();
		if (symbol != IDENFR) error('a');
		char name[50];
		strcpy(name, getWord());
		symbol = getsym();
		char type[10];
		if (intFlag) {
			varSymbol = 1;
			if (symbol == LBRACK) {
				strcpy(type, "lisint");
				if (layer == 0) {
					fprintf(out, "%s: .space", name);
					addGlbArr(name);
				}
				else
					addArray(curFunction, name, pcbPos);
			}
			else strcpy(type, "int");
		}
		else {
			if (symbol == LBRACK) {
				strcpy(type, "lischar");
				if (layer == 0)
				{
					fprintf(out, "%s: .space", name);
					addGlbArr(name);
				}
				else
					addArray(curFunction, name, pcbPos);
			}
			else strcpy(type, "char");
		}
		idenfr idf = findIdenfrCur(name);
		if (strcmp(idf.name, " ") != 0) error('b');
		else {
			addidenfr(name, type, 0);
			//addVar(name);
			if ((strcmp(type, "char") == 0) || (strcmp(type, "int") == 0))           //arr不加入
				if (layer > 0)
				{
					addVar(name);
					createQuadProcVar(name);
				}
		}
		varExplain();
		varSymbol = 0;

	}
	statementSequence();
	printSeq("<复合语句>\n");
	return 1;
}
// ＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'  
int returnFunction() {  //int a( symbol指向(,已打印声明头部,(未打印
	//getStore();
	layer++;
	pcbPos = 0;
	loopLayer = 0;
	newTable(layer);
	newTotalTable(curFunction);
	//voidFlag = 2;
	returnCount = 0;
	symbol = getsym();
	if (symbol == RPARENT) {
		createQuadParaEnd();
		getStore(getLexiSize() - 1);
		printSeq("<参数表>\n");
		getStore(getLexiSize());
		symbol = getsym();
	}
	else {
		parameterTable();
		createQuadParaEnd();
		if (symbol != RPARENT) error('l');
		else symbol = getsym();
	}
	//assert(symbol == LBRACE);
	symbol = getsym();
	combineStatement();
	//assert(symbol == RBRACE);
	getStore(getLexiSize());
	printSeq("<有返回值函数定义>\n");
	if (returnCount == 0) error('h');//有返回值的函数缺少return语句
	createQuadReturn();
	createQuadFuncEnd();
	//voidFlag = 0;
	sortVar(curFunction);
	delTable();
	popTotalTable(pcbPos);
	layer--;
	symbol = getsym();
	return 1;
}
//＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
int nreturnFunction() { // void c(,当前symbol指向括号
	layer++;
	pcbPos = 0;
	loopLayer = 0;
	newTable(layer);
	newTotalTable(curFunction);
	//voidFlag = 1;
	symbol = getsym();
	if (symbol == RPARENT) {
		createQuadParaEnd();
		getStore(getLexiSize() - 1);
		printSeq("<参数表>\n");
		getStore(getLexiSize());
		symbol = getsym();
	}
	else {
		parameterTable();
		createQuadParaEnd();
		if (symbol != RPARENT) error('l');
		else symbol = getsym();
	}
	//assert(symbol == LBRACE);
	symbol = getsym();
	combineStatement();
	//assert(symbol == RBRACE);
	getStore(getLexiSize());
	printSeq("<无返回值函数定义>\n");
	//voidFlag = 0;
    sortVar(curFunction);
	delTable();
	popTotalTable(pcbPos);
	createQuadReturn();
	createQuadFuncEnd();
	layer--;
	symbol = getsym();
	return 1;
}
//＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
void mainFunction() {
	pcbPos = 0;
	layer++;
	loopLayer = 0;
	newTable(layer);
	newTotalTable(curFunction);
	//assert(symbol == LPARENT);
	symbol = getsym();
	if (symbol != RPARENT) error('l');
	else symbol = getsym();
	//assert(symbol == LBRACE);
	getStore(getLexiSize());
	symbol = getsym();
	combineStatement();
	//assert(symbol == RBRACE);
	getStore(getLexiSize());
	printSeq("<主函数>\n");
	sortVar(curFunction);
	delTable();
	popTotalTable(pcbPos);
	createQuadFuncEnd();
	layer--;
}
//＜程序＞    ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
void programme() {   //全局变量，常量说明，变量说明默认初始化是0(内存中直接就是，不需要赋值了hiahiahiahia)，只需要把内存位置标好
	symbol = getsym();  //声明数组需要预留出来位置，防止fp冲掉数组
	int jmain = 0;
	newTable(layer);
	fprintf(out, ".data\n");
	if (symbol == CONSTTK) {
		constExplain();
		getStore(getLexiSize());
	}
	if (symbol == INTTK || symbol == CHARTK) {  
		int intflag = (symbol == INTTK);
		symbol = getsym(); //标识符
		char name[50];
		strcpy(name, getWord());
		getStore(getLexiSize());
		if (symbol == IDENFR) {
			symbol = getsym();
			if (symbol == LPARENT) {//返回函数
				createGlbEnd();
				jmain = 1;
				add2nvoid(name);
				printSeq("<声明头部>\n");
				getStore(getLexiSize());
				strcpy(curFunction, name);
				createQuadFuncBegin(name);
				createQuadLabel(name);
				idenfr idf = findIdenfrCur(name);
				if (strcmp(idf.name, " ") != 0) error('b');   //全局变量与函数重名，此时函数的table还没加
				Function f = getFunction(name);
				if (strcmp(f.name, " ") != 0) error('b');
				else {
					if (intflag) initFunction(name, INTCON);
					else initFunction(name, CHARCON);
				}
				returnFunction();
			}
			else {  //,[;
				char type[10];
				if (intflag) {
					varSymbol = 1;
					if (symbol == LBRACK) {
						strcpy(type, "lisint");
						if (layer == 0)
						{
							fprintf(out, "%s: .space", name);
							addGlbArr(name);
						}
						else
							addArray(curFunction, name, pcbPos);
					}
					else strcpy(type, "int");
				}
				else {
					if (symbol == LBRACK) {
						strcpy(type, "lischar");
						if (layer == 0)
						{
							fprintf(out, "%s: .space", name);
							addGlbArr(name);
						}
						else
							addArray(curFunction, name, pcbPos);
					}
					else strcpy(type, "char");
				}
				idenfr idf = findIdenfrCur(name);
				if (strcmp(idf.name, " ") != 0) error('b');
				else {
					addidenfr(name, type, 0);
					if ((strcmp(type, "int") == 0) || (strcmp(type, "char") == 0)) {
						if (layer > 0)
						{
							addVar(name);
							createQuadProcVar(name);
						}
						else createQuadGlbVar(name);
				}

				}
				if (!varExplain()) {
					jmain = 1;
					returnFunction();
				}
				varSymbol = 0;
			}
		}
	}
	if (jmain == 0)
		createGlbEnd();
	while (symbol == INTTK || symbol == CHARTK || symbol == VOIDTK) {
		if (symbol == INTTK || symbol == CHARTK) {
			int intflag = (symbol == INTTK);
			char name[50];
			symbol = getsym();
			strcpy(name, getWord());
			getStore(getLexiSize());
			printSeq("<声明头部>\n");
			createQuadFuncBegin(name);
			createQuadLabel(name);
			strcpy(curFunction, name);
			add2nvoid(name);
			symbol = getsym();//(
			idenfr idf = findIdenfrCur(name);
			if (strcmp(idf.name, " ") != 0) error('b');
			Function f = getFunction(name);
			if (strcmp(f.name, " ") != 0) error('b');
			else {
				if (intflag) initFunction(name, INTCON);
				else initFunction(name, CHARCON);
			}
			returnFunction();
		}
		else {
			getStore(getLexiSize());
			symbol = getsym();
			char name[50];
			strcpy(name, getWord());
			//Lexical l = getLexi();
			createQuadFuncBegin(name);
			createQuadLabel(name);
			if (strcmp(name, "main") == 0) {
				idenfr idf = findIdenfrCur(name);
				if (strcmp(idf.name, " ") != 0) error('b');
				Function f = getFunction(name);
				if (strcmp(f.name, " ") != 0) error('b');
				else initFunction(name, VOIDTK);
				//printLexi(name);
				getStore(getLexiSize());
				symbol = getsym();
				//mainFunction();
				strcpy(curFunction, name);
				break;
			}
			else {
				add2void(name);
				getStore(getLexiSize());
				idenfr idf = findIdenfrCur(name);
				if (strcmp(idf.name, " ") != 0) error('b');
				Function f = getFunction(name);
				if (strcmp(f.name, " ") != 0) error('b');
				else initFunction(name, VOIDTK);
				symbol = getsym();
				strcpy(curFunction, name);
				nreturnFunction();
			}
		}
	}
	mainFunction();
	printSeq("<程序>\n");
	glbPos += getString();
	fprintf(out, ".text\n");
}



