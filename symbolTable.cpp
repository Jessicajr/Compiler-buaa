#define _CRT_SECURE_NO_WARNINGS
#include"lib.h"
#include"lexical.h"
#include "table.h"
using namespace std;
typedef struct Table {              //在进入一个函数的时候建
	int layer;
	unordered_map<string, idenfr> table;
}Table;
typedef struct TotalTable {             //包含中间变量的符号表，用于计算sp偏移位置
	char funcName[30];
	unordered_set<string> varName;
}TotalTable;

typedef struct ProcessBlock {
	int varStart;         //数组定义后的起始位置
	int totalSize;         //数组及变量共同占的位置
}ProcessBlock;

unordered_map<string, unordered_map<string, int>> func2array2start;
stack<TotalTable> totalSymbolTable;
unordered_map<string, ProcessBlock> func2varCount;
stack<Table> symbolTable;
unordered_map<string, Function> funcTable;
unordered_set<string> glbArrLis;
unordered_map<string, vector<string>> func2var;

void addGlbArr(char* name) {
	string sName = name;
	glbArrLis.insert(sName);
}

bool findGlbArr(char* name) {
	string sName = name;
	if (glbArrLis.find(sName) == glbArrLis.end())
		return false;
	return true;
}

int findArrayinFunc(char* funcName,char* arrayName) {
	string sFunc = funcName;
	string sArr = arrayName;
	unordered_map< string, unordered_map<string, int>>::iterator got1 = func2array2start.find(sFunc);
	if (got1 == func2array2start.end())
	{
		printf("!!!!!!!No defind Func%s\n", funcName);
		return -1;
	}
	unordered_map<string, int>::iterator got2 = got1->second.find(sArr);
	if (got2 == got1->second.end())
		return -2;
	return got2->second;
}



bool isConst(idenfr id) {
	return (id.isConst);
}

char* symbolType(idenfr id) {
	return id.type;
}

Table getTable() {              
	Table tab = symbolTable.top();
	return tab;
}

idenfr findIdenfrCur(char *name) {  //在当前过程的符号表中查找，用来解决重定义问题
	string nameS = name;
	Table tab = symbolTable.top();
	if (tab.table.find(nameS) == tab.table.end()) {
		idenfr temp;
		strcpy(temp.name, " ");
		strcpy(temp.type, "");
		temp.isConst = 2;
		return temp;
	}
	else {
		return tab.table[nameS];
	}
}

void addIdenfrUse(char* name, int add) {
	string sName = name;
	Table& tab = symbolTable.top();
	if (tab.table.find(sName) == tab.table.end())   //不是当前定义的变量
		return;
	idenfr& idf = tab.table[sName];
	idf.useCount += add;
}

void delTable() {                        //从函数退出时进行
	symbolTable.pop();
}

void quickSort(vector<idenfr>&vec, int low, int high) {
	int i, j, key;
	i = low;
	j = high;
	if (i < j) {
		key = vec[low].useCount;
		idenfr idf = vec[low];
		while (i != j)
		{
			while (j > i&& vec[j].useCount <= key) {
				--j;
			}if (i < j) {
				vec[i] = vec[j];
				++i;
			}
			while (j > i&& vec[i].useCount > key) {
				++i;
			}if (i < j) {
				vec[j] = vec[i];
				--j;
			}
		}
		vec[i] = idf;
		quickSort(vec, low, i - 1);
		quickSort(vec, i + 1, high);
	}

}

void sortVar(char* funcName) {
	string sName = funcName;
	Table tab = symbolTable.top();
	vector<idenfr> vec = vector<idenfr>();
	for (unordered_map<string, idenfr> ::iterator it = tab.table.begin(); it != tab.table.end(); it++) {
		if ((strcmp((*it).second.type, "int") == 0) || (strcmp((*it).second.type, "char") == 0)) {
			vec.push_back((*it).second);
		}
	}
	vector<idenfr>& vec2 = vec;
	quickSort(vec2, 0, vec.size() - 1);
	vector<string> vec1 = vector<string>();
	for (vector<idenfr>::iterator ite = vec.begin(); ite != vec.end(); ite++) {
		vec1.push_back((*ite).name);
	}
	func2var.insert(make_pair(sName, vec1));
}

vector<string> getMostVarUse(char* funcName) {
	string sName = funcName;
	if (func2var.find(sName) == func2var.end()) {
		printf("!!!!!!!!!!NOT FOUND FUNC:%s\n", funcName);
		return vector<string>();
	}
	else {
		return func2var[sName];
	}
}

void pushTable(Table tab) {
	symbolTable.push(tab);

}


idenfr  findIdenfr(char* name) {
	stack<Table> buffer;
	if (symbolTable.empty()) {
		idenfr temp;
		strcpy(temp.name, " ");
		strcpy(temp.type, "");
		temp.isConst = 2;
		return temp;
	}
	idenfr collect = findIdenfrCur(name);
	while (strcmp(collect.name, " ") == 0) {     //当前过程符号表中未找到
		Table cur = symbolTable.top();
		delTable();
		buffer.push(cur);                   //暂存
		if (symbolTable.empty()) {
			break;
		}
		collect= findIdenfrCur(name);
	}
	while (!buffer.empty()) {             //restore
		Table buffertop = buffer.top();
		buffer.pop();
		pushTable(buffertop);
	}
	return collect;
}

void addidenfr(char *name,char* type,int isConst) {
	string nameS = name;
	idenfr idf;
	strcpy(idf.name, name);
	strcpy(idf.type, type);
	idf.isConst = isConst;
	idf.useCount = 0;
	//Table curTable = getTable();
	//curTable.table.insert(make_pair(nameS, idf));
	//curTable.table[nameS] = idf;
	symbolTable.top().table.insert(make_pair(nameS, idf));
}
void addVar(char* varName) {
	string sName = varName;
	totalSymbolTable.top().varName.insert(sName);
}

void popTotalTable(int ArraySize) {                             //顺便统计一个过程中的总变量数
	TotalTable ttab = totalSymbolTable.top();
	//int varNum = ttab.varName.size()+ArraySize;
	string Sname = ttab.funcName;
	ProcessBlock pcb;
	pcb.varStart = ArraySize;
	pcb.totalSize = ttab.varName.size() + ArraySize;
	func2varCount.insert(make_pair(Sname, pcb));
	totalSymbolTable.pop();
}

int getVarNum(char* funcName) {                //返回函数和数组共同占用的内存
	string sName = funcName;
	if (func2varCount.find(sName) == func2varCount.end()) {
		printf("!!!No define:%s!!!\n", funcName);
		return -1;
	}
	else {
		return func2varCount[sName].totalSize;
	}

}

int getArrayEnd(char* funcName) {              //返回变量sp开始的位置
	string sName = funcName;
	unordered_map<string, ProcessBlock>::iterator got1 = func2varCount.find(sName);
	if (got1 == func2varCount.end()) {
		printf("!!!No define:%s!!!\n", funcName);
		return -1;
	}
	else {
		return (got1->second).varStart;
	}
}

int ArrayPos(char* funcName, char* arrayName) {     
	string sFunc = funcName;
	string sArray = arrayName;
	if (func2array2start.find(sFunc) == func2array2start.end()) {
		printf("!!!!!not define func %s\n", funcName);
		return -1;
	}
	if (func2array2start[sFunc].find(sArray) == func2array2start[sFunc].end()) {
		printf("!!!!!not define array %s in %s\n", arrayName,funcName);
		return -1;
	}
	return func2array2start[sFunc][sArray];
}

void newTable(int layerCount) {                   //在一个过程或者函数刚开始创建
	Table tab;
	tab.layer = layerCount;
	tab.table = unordered_map<string, idenfr>();
	pushTable(tab);
}

void newTotalTable(char* funcName) {
	string sName = funcName;
	func2array2start[sName] = unordered_map<string, int>();
	TotalTable ttab;
	strcpy(ttab.funcName, funcName);
	ttab.varName = unordered_set<string>();
	totalSymbolTable.push(ttab);
}

void addArray(char* funcName, char* array, int pos) {
	string sFunc = funcName;
	string sArray = array;
	func2array2start[sFunc].insert(make_pair(sArray,pos));
}

void initFunction(char* name,int type) {      //函数插入表在进入function()前进行
	Function func;
	string nameS = name;
	strcpy(func.name, name);
	func.type = type;
	func.parCount = 0;
	funcTable.insert(make_pair(nameS, func));
	//funcTable[nameS] = func;
}

void addParam(char* name, int parType) {
	string nameS = name;
	//Function func = funcTable[nameS];
	int count = funcTable[nameS].parCount;
	funcTable[nameS].table[count] = parType;
	funcTable[nameS].parCount++;
	//funcTable.insert(make_pair(nameS, func));
}

Function getFunction(char* name) {
	string nameS = name;
	if (funcTable.find(nameS) == funcTable.end()) {    //notFounr
		Function f;
		strcpy(f.name, " ");
		f.type = -1;
		f.parCount = -1;
		return f;
	}
	Function func = funcTable[nameS];
	return func;
}

