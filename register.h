#pragma once
#include "lib.h"
enum Reg {
	t0, t1, t2, t3, t4, t5, t6, t7,
	s0, s1, s2, s3, s4, s5, s6, s7
};
enum pushReg {
	a0,a1,a2,a3
};
typedef struct Position {
	bool reg;           //判断是寄存器中还是内存
	int memCount;       //内存中的位置
	Reg regNum;         //寄存器的标号
	char pS[3];         //fp or gp or sp
}Position;
typedef struct pushPosition {
	bool reg;
	pushReg regNum;
	int memCount;
}pushPosition;

void createProTable(char* processName);
void delProTable();
Position getVarPos(char* name,int mark);
void storeReg();
pushPosition paraVar(char*name,int num);
void mipsCode();
void addVarPos(char* name);
void addParaPos(char* name);
void restoreReg();