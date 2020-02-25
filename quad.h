#pragma once
#include "lib.h"

enum OperatorNum {    //���г���ֵ����ͨ��EQUALi��ֵ��var���ٽ���
		ASS,      //         done            ��ֵ,1           
		ASSi,     //         done            ������ֵ,1 
		ASAR,     //         done            ��gram.cpp�����.data���һ����������Ԫ�ظ�ֵ,2 
		ASTK,    //          done            ����Ԫ��ȡֵ,2 
		NEG,     //          done            ȡ��,1   
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
		LABEL,    //����     done
		GOTO,    //j         done
		BNZ,     //          done             ������ת  
		BZ,      //
		PUSH,    //          done             ���ú�������ѹջ�򱣴��ڼĴ�����ֵ������
		PARA,   //           done             ���������н�a0-a3,ջ�е�ֵȡ����ʱ�Ĵ�����
		JR,     //           done             ��������
		JAL,    //           done             ��������   
		RET,    //           done             ��������ֵ 
		SCANF, //            done
		PRITNF, //           done
		FUNCBEGIN,   //      done             ������ʼ����������һ�������飬���¼Ĵ�����(����mips�����֣�
		FUNCEND,    //       done            ������mips�����֣�
		GLBEND,    //                          ȫ�ֱ���������������Ҫj Main
		PRINTEND,
		GLBVAR,   //ȫ��var�ͱ�����Ҫ���浽glbvarList��
		PROCVAR,   //�����б��������Լ���������ҪԤ������λ�ã�����ᱻȫ�ֱ�������
		PARAVAR,   //�������������ͺ����������ı���Ҫ������Ҫ�ڵ���������Ժ��λ������
		PARAEND,  //��־һ������ �������������
		FUNCCALL,   //�������ã���Ҫһ��move $t,$v1�Ķ���
		DEFAULT  //
};

typedef struct Operator{
	OperatorNum op;  //������
	int num;     //��Ч������0��1��2
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
void createQuadFuncBegin(char* funcName);    //�����µļĴ�����
void createQuadFuncEnd();  //�����ӵ��Ĵ�����
void createGlbEnd();      //ȫ�ֱ�������������j main
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
