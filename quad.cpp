#define _CRT_SECURE_NO_WARNINGS
#include "quad.h"
#include "lib.h"
#include"table.h"
using namespace std;
extern FILE* in, * out;
queue<quadItem> quadList;
stack<string> quadRefList;   //四元式中ref记录
vector<string> registerList;
queue<ASCIIZ> stringQueue;
int asciizLen;
int regCount;
char regName[20];
char printDic[35][10] = { "ASS","ASSi","ASAR","ASTK","NEG","SUB","ADD","MULTI","DIVI","EQUAL","LS","LSE","BG","BGE","NEQUAL","LABEL","GOTO","BNZ","PUSH","PARA","JR","JAL","RET","SCANF","PRITNF","FUNCBEGIN","FUNCEND","GLBEND","PRINTEND","GLBVAR","PROCVAR","PARAVAR","PARAEND","FUNCCALL","DEFAULT" };
void createQuad(OperatorNum op,int validRef, char* source1, char* source2,char* dst) {   //var op var 构建四元式并将四元式push进队列
	quadItem item;
	Operator ope;
	ope.op = op;
	ope.num = validRef;
	item.op = ope;
	strcpy(item.source1, source1);
	strcpy(item.source2, source2);
	strcpy(item.dst, dst);
	quadList.push(item);
	if (validRef == 2)
		printf("%s %s, %s, %s\n", printDic[op], source1, source2, dst);
	else {
		if (validRef == 1)
			printf("%s %s, %s\n", printDic[op], source1, dst);
		else
			printf("%s %s\n", printDic[op], dst);
	}
}
void createQuadFuncCall(char* dst) {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::FUNCCALL;
	ope.num = -2;
	item.op = ope;
	strcpy(item.dst, dst);
	quadList.push(item);
	printf("FUNCCALL %s\n", dst);
}
void createQuadInt(int source, char* dst) {    //var = intVar
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::ASSi;
	ope.num = 1;
	item.op = ope;
	item.intSource = source;
	strcpy(item.dst, dst);
	quadList.push(item);
	printf("ASSi Int %d, %s\n", source, dst);
}

void createQuadChar(char source, char* dst) {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::ASSi;
	ope.num = 1;
	item.op = ope;
	item.intSource = source;
	strcpy(item.dst, dst);
	quadList.push(item);
	printf("ASSi Char %c, %s\n",  source, dst);
}
void createQuadGlbVar(char* name) {
	quadItem item;
	Operator ope;
	ope.num = -1;
	ope.op = OperatorNum::GLBVAR;
	item.op = ope;
	strcpy(item.dst, name);
	quadList.push(item);
}

void createQuadProcVar(char* name) {
	quadItem item;
	Operator ope;
	ope.num = -1;
	ope.op = OperatorNum::PROCVAR;
	item.op = ope;
	strcpy(item.dst, name);
	quadList.push(item);
}
void createQuadProcParaVar(char* name) {
	quadItem item;
	Operator ope;
	ope.num = -1;
	ope.op = OperatorNum::PARAVAR;
	item.op = ope;
	strcpy(item.dst, name);
	quadList.push(item);
}
void createQuadLabel(char* labelTag) {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::LABEL;
	ope.num = 0;
	strcpy(item.dst, labelTag);
	item.op = ope;
	quadList.push(item);
	printf("LABEL %s\n",  labelTag);
}
void createQuadFuncBegin(char* funcName){
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::FUNCBEGIN;
	ope.num = -1;
	item.op = ope;
	strcpy(item.dst, funcName);
	quadList.push(item);
	printf("######FuncDefStart######\n");
}

void createQuadGoto(char* labelTag) {       // 无条件跳转，一般用于条件判断后面
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::GOTO;
	ope.num = 0;
	strcpy(item.dst, labelTag);
	item.op = ope;
	quadList.push(item);
	printf("GOTO %s\n", labelTag);
}
void createQuadPara(char *varName,char *funcName) {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::PARA;
	ope.num = 1;
	strcpy(item.source1, varName);
	strcpy(item.dst, funcName);
	item.op = ope;
	quadList.push(item);
	printf("PARA %s, %s\n", varName, funcName);
}

void createQuadParaEnd() {
	quadItem item;
	Operator ope;
	ope.num = -2;
	ope.op = OperatorNum::PARAEND;
	item.op = ope;
	quadList.push(item);
}
void createQuadPush(char* refName,char* funcName,int num,int callLayer) {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::PUSH;
	ope.num = 1;
	strcpy(item.source1, refName);
	strcpy(item.dst,funcName);
	item.op = ope;
	item.pushNum = num;
	item.callLayer = callLayer;
	quadList.push(item);
	printf("PUSH %s, %s\n", refName, funcName);
}
void createQuadJump(char* dst) {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::JAL;
	ope.num = 0;
	strcpy(item.dst, dst);
	item.op = ope;
	quadList.push(item);
	printf("JAL %s\n", dst);
}
void createQuadReturn() {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::JR;
	ope.num = -1;
	item.op = ope;
	quadList.push(item);
	printf("JR\n");
}
void createQuadFuncEnd() {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::FUNCEND;
	ope.num = -1;
	item.op = ope;
	quadList.push(item);
	printf("######FuncDefEnd######\n");
}
void createQuadReturnValue(char* name) {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::RET;
	ope.num = 1;
	strcpy(item.source1, name);
	strcpy(item.dst, "#RET");
	item.op = ope;
	quadList.push(item);
	printf("RET %s\n", name);
}

void createQuadScanf(char* dst,int isInt) {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::SCANF;
	ope.num = 0;
	strcpy(item.dst, dst);
	item.op = ope;
	item.intSource = isInt;
	quadList.push(item);
	if (isInt)
		printf("SCANF int %s\n", dst);
	else
		printf("SCANF char %s\n",dst);
}
void createQuadPrintf(char *dst, int isInt) {// printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')'
	quadItem item;      //isInt是-1 表示此处为strCON
	Operator ope;
	ope.op = OperatorNum::PRITNF;
	ope.num = 0;
	strcpy(item.dst, dst);
	item.op = ope;
	item.intSource = isInt;
	quadList.push(item);
	if (isInt == -1)
		printf("PRINT String %s\n",dst);
	if (isInt == 1)
		printf("PRINT Int %s\n", dst);
	if (isInt == 0)
		printf("PRINT Char %s\n", dst);

}
void createQuadPrintfEnd() {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::PRINTEND;
	ope.num = -1;
	item.op = ope;
	quadList.push(item);
}
quadItem getQuad() {
	if (quadList.empty()) {
		quadItem temp;
		Operator ope;
		ope.op = OperatorNum::DEFAULT;
		temp.op = ope;
		return temp;
	}
	else {
		quadItem item = quadList.front();
		quadList.pop();
		return item;
	}
}
void createGlbEnd() {
	quadItem item;
	Operator ope;
	ope.op = OperatorNum::GLBEND;
	ope.num = -1;
	item.op = ope;
	quadList.push(item);
	printf("############StartMain##########\n");

}
void putQuadRef(char* name) {
	string sName = name;
	quadRefList.push(sName);
}

char* popQuadRef() {
	string now = quadRefList.top();   //赋值完毕可以删除
	quadRefList.pop();
	const char* ret = now.c_str();
	char* buf = new char[strlen(ret) + 1];
	strcpy(buf, ret);
	return buf;
}

char* getNewReg(int layer) {    //获得新寄存器并保存在使用寄存器队列中,j将寄存器名字保存到Func
	
	if (registerList.empty())
		regCount = 0;
	else regCount++;
	sprintf(regName, "%d", regCount);
	string sregName = regName;
	registerList.push_back(sregName);
	if (layer>0)
		addVar(regName);
	return regName;
}

bool outFunc(quadItem item) {
	if (item.op.op == OperatorNum::FUNCEND)
		return true;
	return false;
}

bool inFunc(quadItem item) {
	return (item.op.op == OperatorNum::FUNCBEGIN);
}

bool isEnd(quadItem item) {
	if (item.op.op == OperatorNum::DEFAULT)
		return true;
	return false;
}
void putString(char* strName, char* strCont) {
	ASCIIZ asc;
	strcpy(asc.name, strName);
	strcpy(asc.String1, strCont);
	asciizLen += strlen(strCont)+1;
	stringQueue.push(asc);
}
int getString() {
	fprintf(out, "__:.asciiz\"\\n\"\n");
	asciizLen += 2;
	while (!stringQueue.empty()) {
		ASCIIZ asc = stringQueue.front();
		stringQueue.pop();
		fprintf(out, "%s:.asciiz\"%s\"\n", asc.name, asc.String1);
	}
	if ((asciizLen % 4) == 0)
		return asciizLen / 4;
	else
		return (1 + asciizLen / 4);


}