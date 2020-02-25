#pragma once
#include "lib.h"
using namespace std;
typedef struct idenfr {
	char name[100];             // δ�ҵ�ʱ����һ��nameΪ" " ��
	char type[10];             //int,char,lisint,lischar
	int isConst;              //
	int useCount;               //ʹ�ô���
}idenfr;


typedef struct Function {
	char name[100];
	int type;               //INTCON,CHARCON,VOIDTK
	int table[20];        //INTCON,CHARCON,��֧�ֲ�����������
	int parCount;        //�������±�
}Function;


//typedef unordered_map<string, idenfr> Table;
bool isConst(idenfr id);
char* symbolType(idenfr id);
idenfr findIdenfrCur(char* name);
void delTable();
idenfr  findIdenfr(char* name);
void addidenfr(char* name, char* type, int isConst);
void newTable(int layerCount);
Function getFunction(char* name);
void addParam(char* name, int paraType);
void initFunction(char* name, int type);
void addVar(char* varName);
void popTotalTable(int ArraySize);
int getVarNum(char* funcName);
void newTotalTable(char* funcName);
void addArray(char* funcName, char* array, int pos);
int getArrayEnd(char* funcName);
int getVarNum(char* funcName);
int ArrayPos(char* funcName, char* arrayName);
void addGlbArr(char* name);
bool findGlbArr(char* name);
int findArrayinFunc(char* funcName, char* arrayName);
void addIdenfrUse(char* name, int add);
void sortVar(char* funcName);
vector<string> getMostVarUse(char* funcName);