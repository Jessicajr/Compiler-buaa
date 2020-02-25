#pragma once
#include "lib.h"

enum OperatorNum {    //所有常量值都先通过EQUALi赋值给var，再进行
		ASS,      //         done            赋值,1           
		ASSi,     //         done            常数赋值,1 
		ASAR,     //         done            在gram.cpp中完成.data后的一串声明数组元素赋值,2 
		ASTK,    //          done            数组元素取值,2 
		NEG,     //          done            取负,1   
		SUB,     //2         done
		ADD,     //2         done
		MULTI,   //2         done
		DIVI,    //2         done 
		EQUAL,  //==,2       done
		LS,     //<,2        done
		LSE,    //<=,2       done
		BG,     //>,2        done
		BGE,    //>=,2       done
		NEQUAL,  //!=,2      done
		LABEL,    //声明     done
		GOTO,    //j         done
		BNZ,     //          done             满足跳转  
		BZ,      //
		PUSH,    //          done             调用函数参数压栈或保存在寄存器（值参数表）
		PARA,   //           done             函数定义中将a0-a3,栈中的值取到临时寄存器中
		JR,     //           done             函数返回
		JAL,    //           done             函数调用   
		RET,    //           done             函数返回值 
		SCANF, //            done
		PRITNF, //           done
		FUNCBEGIN,   //      done             函数开始声明，区别一个基本块，更新寄存器表(不在mips中体现）
		FUNCEND,    //       done            （不在mips中体现）
		GLBEND,    //                          全局变量声明结束，需要j Main
		PRINTEND,
		GLBVAR,   //全局var型变量需要保存到glbvarList中
		PROCVAR,   //函数中变量声明以及参数声明要预留出来位置，否则会被全局变量覆盖
		PARAVAR,   //函数参数变量和函数中声明的变量要区别，主要在第五个参数以后的位置问题
		PARAEND,  //标志一个函数 参数表输入结束
		FUNCCALL,   //函数调用，需要一个move $t,$v1的动作
		DEFAULT  //
};

typedef struct Operator{
	OperatorNum op;  //操作符
	int num;     //有效操作数0或1或2
}Operator;

typedef struct quadItem {
	Operator op;
	char source1[100];
	char source2[100];
	int intSource;
	//char charSource;
	char dst[100];
	int pushNum;
	int callLayer;
}quadItem;

typedef struct ASCIIZ {
	char name[20];
	char String1[100];

}ASCIIZ;
void createQuad(OperatorNum op, int validRef, char* source1, char* source2, char* dst);
void createQuadLabel(char* labelTag);
void createQuadInt(int source, char* dst);
void createQuadChar(char source, char* dst);
void createQuadGoto(char* labelTag);
void createQuadPara(char* varName, char* funcName);
void createQuadPush(char* refName, char* funcName,int num,int callLayer);
void createQuadJump(char* dst);
void createQuadReturnValue(char* name);
void createQuadScanf(char* dst,int isInt);
void createQuadPrintf(char* dst, int isInt);
void createQuadReturn();      
void createQuadFuncBegin(char* funcName);    //创建新的寄存器表
void createQuadFuncEnd();  //可以扔掉寄存器表
void createGlbEnd();      //全局变量声明结束，j main
void createQuadPrintfEnd();  
void createQuadParaEnd();
void createQuadGlbVar(char* name);
void createQuadProcVar(char* name);
void createQuadProcParaVar(char* name);
quadItem getQuad();
void putQuadRef(char* name);
char* popQuadRef();
char* getNewReg(int layer);
bool isEnd(quadItem item);
int getString();
void putString(char* strName, char* strCont);
void createQuadFuncCall(char* dst);
