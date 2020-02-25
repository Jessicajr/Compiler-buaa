#define _CRT_SECURE_NO_WARNINGS
#include "quad.h"
# include "lib.h"
#include "register.h"
#include "table.h"
extern FILE* in, * out;
using namespace std;
char curFuncName[20];      //当前函数名，用来找对应array的起始位置
int nGlbMark;              //全局数组保存在fp中，用.space写在最前面
int callinPush;
char Dic[16][3] = { "t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5","s6","s7" };
char varDst[4][3] = { "a0","a1","a2","a3" };
queue<pushPosition> curPara;
unordered_map<string, queue<pushPosition>> paraMap;
void genAddorSub(quadItem item) {
	char ope[4];
	strcpy(ope, "sub");
	if (item.op.op == OperatorNum::ADD)
		strcpy(ope, "add");
	Position pos1 = getVarPos(item.source1,1);
	Position pos2 = getVarPos(item.source2,1);
	Position pos3 = getVarPos(item.dst,1);
	if (pos1.reg && pos2.reg) {                       //都在临时寄存器中
		int reg1 = pos1.regNum;
		int reg2 = pos2.regNum;
		if (pos3.reg) {
			int reg3 = pos3.regNum;
			fprintf(out, "%s $%s,$%s,$%s\n", ope,Dic[reg3], Dic[reg1], Dic[reg2]);
		}
		else {
			fprintf(out, "%s $t8,$%s,$%s\n", ope,Dic[reg1], Dic[reg2]);
			int memCount3 = pos3.memCount * 4;
			fprintf(out, "sw $t8,%d($%s)\n",memCount3,pos3.pS);
		}
	}
	else {
		if (pos1.reg) {
			int reg1 = pos1.regNum;
			int memCount2 = pos2.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memCount2,pos2.pS);
			if (pos3.reg) {
				int reg3 = pos3.regNum;
				fprintf(out, "%s $%s,$%s,$t8\n", ope,Dic[reg3],Dic[reg1]);
			}
			else {
				fprintf(out, "%s $t9,$%s,$t8\n", ope,Dic[reg1]);
				int memCount3 = pos3.memCount * 4;
				fprintf(out, "sw $t9,%d($%s)\n", memCount3,pos3.pS);
			}
		}
		else if (pos2.reg) {
			int reg2 = pos2.regNum;
			int memCount1 = pos1.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memCount1,pos1.pS);
			if (pos3.reg) {
				int reg3 = pos3.regNum;
				fprintf(out, "%s $%s,$t8,$%s\n", ope,Dic[reg3], Dic[reg2]);
			}
			else {
				fprintf(out, "%s $t9,$t8,$%s\n", ope,Dic[reg2]);
				int memCount3 = pos3.memCount * 4;
				fprintf(out, "sw $t9,%d($%s)\n", memCount3,pos3.pS);
			}
		}
		else {
			int memCount1 = pos1.memCount * 4;
			int memCount2 = pos2.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memCount1,pos1.pS);
			fprintf(out, "lw $t9,%d($%s)\n", memCount2,pos2.pS);
			if (pos3.reg) {
				int reg3 = pos3.regNum;
				fprintf(out, "%s $%s,$t8,$t9\n", ope, Dic[reg3]);
			}
			else {
				fprintf(out, "%s $t9,$t8,$t9\n", ope);
				int memCount3 = pos3.memCount * 4;
				fprintf(out, "sw $t9,%d($%s)\n", memCount3,pos3.pS);
			}
		}
	}
}
void genMultorDiv(quadItem item) {
	char ope[5];
	if (item.op.op == OperatorNum::MULTI)
		strcpy(ope, "mult");
	else
		strcpy(ope, "div");
	Position pos1 = getVarPos(item.source1,1);
	Position pos2 = getVarPos(item.source2,1);
	Position pos3 = getVarPos(item.dst,1);
	if (pos1.reg && pos2.reg) {                       //都在临时寄存器中
		int reg1 = pos1.regNum;
		int reg2 = pos2.regNum;
		fprintf(out, "%s $%s,$%s\n", ope, Dic[reg1], Dic[reg2]);
		if (pos3.reg) {
			int reg3 = pos3.regNum;
			fprintf(out, "mflo $%s\n",Dic[reg3]);
		}
		else {
			fprintf(out, "mflo $t8\n");
			int memCount3 = pos3.memCount * 4;
			fprintf(out, "sw $t8,%d($%s)\n", memCount3,pos3.pS);
		}
	}
	else {
		if (pos1.reg) {
			int reg1 = pos1.regNum;
			int memCount2 = pos2.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memCount2,pos2.pS);
			fprintf(out, "%s $%s,$t8\n", ope, Dic[reg1]);
			if (pos3.reg) {
				int reg3 = pos3.regNum;
				fprintf(out, "mflo $%s\n", Dic[reg3]);
			}
			else {
				fprintf(out, "mflo $t8\n");
				int memCount3 = pos3.memCount * 4;
				fprintf(out, "sw $t8,%d($%s)\n", memCount3,pos3.pS);
			}
		}
		else if (pos2.reg) {
			int reg2 = pos2.regNum;
			int memCount1 = pos1.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memCount1,pos1.pS);
			fprintf(out, "%s $t8,$%s\n",ope, Dic[reg2]);
			if (pos3.reg) {
				int reg3 = pos3.regNum;
				fprintf(out, "mflo $%s\n", Dic[reg3]);
			}
			else {
				fprintf(out, "mflo $t8\n");
				int memCount3 = pos3.memCount * 4;
				fprintf(out, "sw $t8,%d($%s)\n", memCount3,pos3.pS);
			}
		}
		else {
			int memCount1 = pos1.memCount * 4;
			int memCount2 = pos2.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memCount1,pos1.pS);
			fprintf(out, "lw $t9,%d($%s)\n", memCount2,pos2.pS);
			fprintf(out, "%s $t8,$t9\n", ope);
			if (pos3.reg) {
				int reg3 = pos3.regNum;
				fprintf(out, "mflo $%s\n", Dic[reg3]);
			}
			else {
				fprintf(out, "mflo $t8\n");
				int memCount3 = pos3.memCount * 4;
				fprintf(out, "sw $t8,%d($%s)\n", memCount3,pos3.pS);
			}
		}
	}
}
void genAssorNeg(quadItem item) {
	int mark = 1;
	char ope[5];
	strcpy(ope, "move");
	if (item.op.op == OperatorNum::NEG)
	{
		strcpy(ope, "neg");
		if (strcmp(item.source1 ,item.dst)==0)
			mark = 0;
	}
	Position pos1 = getVarPos(item.source1,mark);
	Position pos3 = getVarPos(item.dst,0);
	if (pos1.reg) {
		int reg1 = pos1.regNum;
		if (pos3.reg) {
			int reg3 = pos3.regNum;
			fprintf(out, "%s $%s,$%s\n", ope, Dic[reg3], Dic[reg1]);
		}
		else {
			fprintf(out, "%s $t8,$%s\n", ope, Dic[reg1]);
			int memCount3 = pos3.memCount * 4;
			fprintf(out, "sw $t8,%d($%s)\n", memCount3,pos3.pS);
		}
	}
	else {
		int memCount1 = pos1.memCount * 4;
		fprintf(out, "lw $t8,%d($%s)\n", memCount1,pos1.pS);
		if (pos3.reg) {
			int reg3 = pos3.regNum;
			fprintf(out, "%s $%s,$t8\n", ope, Dic[reg3]);
		}
		else {
			fprintf(out, "%s $t9,$t8\n", ope);
			int memCount3 = pos3.memCount * 4;
			fprintf(out, "sw $t9,%d($%s)\n", memCount3,pos3.pS);
		}
	}
}
void genAssi(quadItem item) {
	Position pos3 = getVarPos(item.dst,1);
	if (pos3.reg) {
		int reg3 = pos3.regNum;
		fprintf(out, "addi $%s,$zero,%d\n", Dic[reg3], item.intSource);
	}
	else {
		fprintf(out, "addi $t9,$zero,%d\n", item.intSource);
		int memCount3 = pos3.memCount * 4;
		fprintf(out, "sw $t9,%d($%s)\n", memCount3,pos3.pS);
	}
}

void genAS(quadItem item) {                  //glb Array 在最开始声明过
	                                         //其他Array在本函数中定义
	Position posReg = getVarPos(item.source1,1);
	Position posOff = getVarPos(item.source2,1);
	char arrayName[20];
	strcpy(arrayName, item.dst);
	char ope[3];
	strcpy(ope, "lw");
	if (item.op.op == OperatorNum::ASAR)
		strcpy(ope, "sw");
	if (posReg.reg) {
		int regNum1 = posReg.regNum;
		if (posOff.reg) {
			int regNum2 = posOff.regNum;
			fprintf(out, "sll $t9,$%s,2\n", Dic[regNum2]);           //off*4
			int inFunc = findArrayinFunc(curFuncName, arrayName);
			if (inFunc == -1)
				return;
			if (inFunc == -2) {                              //查找全局数组
				if (!findGlbArr(arrayName)) {
					printf("!!!!!not define array:%s\n", arrayName);
					return;
				}
				else {
					fprintf(out, "%s $%s,%s($t9)\n", ope, Dic[regNum1], arrayName);
				}
			}
			else {     //数组定义在当前函数中，inFunc是数组起始位置
				//-(off*4+arrstart*4)($sp)
				inFunc =inFunc*(-4);
				fprintf(out, "sub $sp,$sp,$t9\n");//sp = sp-4*offset
				fprintf(out, "%s $%s,%d($sp)\n", ope, Dic[regNum1], inFunc);
				fprintf(out, "add $sp,$sp,$t9\n");
			}
			
		}
		else {
			int memNum1 = posOff.memCount * 4;
			fprintf(out, "lw $t9,%d($%s)\n", memNum1,posOff.pS);
			fprintf(out, "sll $t9,$t9,2\n");           //ind*4
			int inFunc = findArrayinFunc(curFuncName, arrayName);
			if (inFunc == -1)
				return;
			if (inFunc == -2) {                              //查找全局数组
				if (!findGlbArr(arrayName)) {
					printf("!!!!!not define array:%s\n", arrayName);
					return;
				}
				else {
					fprintf(out, "%s $%s,%s($t9)\n", ope, Dic[regNum1], arrayName);
				}
			}
			else {     //数组定义在当前函数中，inFunc是数组起始位置
				//-(off*4+arrstart*4)($sp)
				inFunc = inFunc * (-4);
				fprintf(out, "sub $sp,$sp,$t9\n");//sp = sp-4*offset
				fprintf(out, "%s $%s,%d($sp)\n", ope, Dic[regNum1], inFunc);
				fprintf(out, "add $sp,$sp,$t9\n");
			}
			
		}
	}
	else {
		int memNum2 = posReg.memCount * 4;
		if (item.op.op == OperatorNum::ASAR) {
			fprintf(out, "lw $t8,%d($%s)\n", memNum2,posReg.pS);
		}
		if (posOff.reg) {
			int regNum2 = posOff.regNum;
			fprintf(out, "sll $t9,$%s,2\n", Dic[regNum2]);           //ind*4
			int inFunc = findArrayinFunc(curFuncName, arrayName);
			if (inFunc == -1)
				return;
			if (inFunc == -2) {                              //查找全局数组
				if (!findGlbArr(arrayName)) {
					printf("!!!!!not define array:%s\n", arrayName);
					return;
				}
				else {
					fprintf(out, "%s $t8,%s($t9)\n", ope, arrayName);
				}
			}
			else {     //数组定义在当前函数中，inFunc是数组起始位置
				//-(off*4+arrstart*4)($sp)
				inFunc = inFunc * (-4);
				fprintf(out, "sub $sp,$sp,$t9\n");//sp = sp-4*offset
				fprintf(out, "%s $t8,%d($sp)\n", ope,inFunc);
				fprintf(out, "add $sp,$sp,$t9\n");
			}
			
		}
		else {
			int memNum1 = posOff.memCount * 4;
			fprintf(out, "lw $t9,%d($%s)\n", memNum1,posOff.pS);
			fprintf(out, "sll $t9,$t9,2\n");           //ind*4
			int inFunc = findArrayinFunc(curFuncName, arrayName);
			if (inFunc == -1)
				return;
			if (inFunc == -2) {                              //查找全局数组
				if (!findGlbArr(arrayName)) {
					printf("!!!!!not define array:%s\n", arrayName);
					return;
				}
				else {
					fprintf(out, "%s $t8,%s($t9)\n", ope, arrayName);
				}
			}
			else {     //数组定义在当前函数中，inFunc是数组起始位置
				//-(off*4+arrstart*4)($sp)
				inFunc = inFunc * (-4);
				fprintf(out, "sub $sp,$sp,$t9\n");//sp = sp-4*offset
				fprintf(out, "%s $t8,%d($sp)\n", ope, inFunc);
				fprintf(out, "add $sp,$sp,$t9\n");
			}
		}
		if (item.op.op == OperatorNum::ASTK) {
			fprintf(out, "sw $t8,%d($%s)\n", memNum2,posReg.pS);
		}
	}
}

void genConditionEQ(quadItem item) {
	char label[20];
	char ope[5];
	strcpy(label, item.dst);
	Position pos1 = getVarPos(item.source1,1);
	Position pos2 = getVarPos(item.source2,1);
	if (item.op.op == OperatorNum::EQUAL)
		strcpy(ope, "beq");
	if (item.op.op == OperatorNum::NEQUAL)
		strcpy(ope, "bne");
	if (pos1.reg) {
		int regNum1 = pos1.regNum;
		if (pos2.reg) {
			int regNum2 = pos2.regNum;
			fprintf(out, "%s $%s,$%s,%s\n", ope, Dic[regNum1], Dic[regNum2], label);
		}
		else {
			int memNum2 = pos2.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memNum2,pos2.pS);
			fprintf(out, "%s $%s,$t8,%s\n", ope, Dic[regNum1], label);
		}
	}
	else {
		int memNum1 = pos1.memCount * 4;
		fprintf(out, "lw $t9,%d($%s)\n", memNum1,pos1.pS);
		if (pos2.reg) {
			int regNum2 = pos2.regNum;
			fprintf(out, "%s $t9,$%s,%s\n", ope, Dic[regNum2], label);
		}
		else {
			int memNum2 = pos2.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memNum2,pos2.pS);
			fprintf(out, "%s $t9,$t8,%s\n", ope, label);
		}
	}
}
void genConditionCal(quadItem item) {
	char label[20];
	char ope[5];
	strcpy(label, item.dst);
	Position pos1 = getVarPos(item.source1,1);
	Position pos2 = getVarPos(item.source2,1);
	if (item.op.op == OperatorNum::BG) 
		strcpy(ope, "bgtz");
	if (item.op.op == OperatorNum::BGE) 
		strcpy(ope, "bgez");
	if (item.op.op == OperatorNum::LS) 
		strcpy(ope, "bltz");
	if (item.op.op == OperatorNum::LSE) 
		strcpy(ope, "blez");
	if (pos1.reg) {
		int regNum1 = pos1.regNum;
		if (pos2.reg) {
			int regNum2 = pos2.regNum;
			fprintf(out, "sub $t8,$%s,$%s\n", Dic[regNum1], Dic[regNum2]);
		}
		else {
			int memNum2 = pos2.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memNum2,pos2.pS);
			fprintf(out, "sub $t8,$%s,$t8\n", Dic[regNum1]);
		}
	}
	else {
		int memNum1 = pos1.memCount * 4;
		fprintf(out, "lw $t9,%d($%s)\n", memNum1,pos1.pS);
		if (pos2.reg) {
			int regNum2 = pos2.regNum;
			fprintf(out, "sub $t8,$t9,$%s\n",Dic[regNum2]);
		}
		else {
			int memNum2 = pos2.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memNum2,pos2.pS);
			fprintf(out, "sub $t8,$t9,$t8\n");
		}
	}
	fprintf(out, "%s $t8,%s\n", ope, label);
}
void genConditionUni(quadItem item) {
	char label[20];
	char ope[4];
	if (item.op.op == OperatorNum::BZ) {
		strcpy(ope, "beq");
	}
	else {
		strcpy(ope, "bne");
	}
	Position pos1 = getVarPos(item.source1,1);
	strcpy(label, item.dst);
	if (pos1.reg) {
		int regNum = pos1.regNum;
		fprintf(out, "%s $%s,$zero,%s\n", ope,Dic[regNum], label);
	}
	else {
		int memCount = pos1.memCount * 4;
		fprintf(out, "lw $t8,%d($%s)\n", memCount,pos1.pS);
		fprintf(out, "%s $t8,$zero,%s\n",ope, label);
	}
}
void genJump(quadItem item) {
	if (item.op.op == OperatorNum::GOTO)
		fprintf(out, "j %s\n", item.dst);
	else {      //函数调用
		int curFuncVarSize = getVarNum(curFuncName) * 4 + 4;    //ra
		int raPos = (curFuncVarSize-4) * -1;
		curFuncVarSize += (16 * (callinPush+1));                                 //a0-a3
		storeReg();                                          //传参数表构建完成，可以保存寄存器现场,sw操作在内部实现
		fprintf(out, "sw $ra,%d($sp)\n", raPos);               //保存返回地址在最底下
		int j;
		int memPos = raPos - 16*(callinPush+1);
		fprintf(out, "subi $sp,$sp,%d\n", curFuncVarSize);   //调用其他函数时,预留出本函数所有变量的位置
		fprintf(out, "jal %s\n", item.dst);
		fprintf(out, "addi $sp,$sp,%d\n", curFuncVarSize);//恢复本函数sp的位置
		fprintf(out, "lw $ra,%d($sp)\n", raPos);           //恢复返回地址
		restoreReg();
		if (callinPush > 0 ) {
			for (j = 3; j >= 0; j--) {                         //a0~a3 保护
				fprintf(out, "lw $%s,%d($sp)\n", varDst[j], memPos);
				memPos += 4;
			}
			callinPush -=1;
		}
	}
}
void genValueReturn(quadItem item) {
	Position pos = getVarPos(item.source1,1);
	if (pos.reg) {
		int regNum = pos.regNum;
		fprintf(out, "move $v1,$%s\n", Dic[regNum]);
	}
	else {
		int memCount = pos.memCount * 4;
		fprintf(out, "lw $v1,%d($%s)\n", memCount,pos.pS);
	}
}
void genPara(quadItem item) {                 //a0移到临时寄存器中,必然是该函数出现的第一批变量
	strcpy(curFuncName, item.dst);             //后取出的
	int curNum = curPara.size();
	pushPosition ppos = paraVar(item.source1,curNum);
	curPara.push(ppos);
	if (ppos.reg) {                                              //a0-a3,转移
		int regNumsrc = ppos.regNum;
		Position pos = getVarPos(item.source1,1);
		if (pos.reg) {
			int regNumdst = pos.regNum;
			fprintf(out, "move $%s,$%s\n", Dic[regNumdst], varDst[regNumsrc]);
		}
		else { //
			int memCountdst = pos.memCount * 4;
			fprintf(out, "move $t9,$%s\n", varDst[regNumsrc]);
			fprintf(out, "sw $t9,%d($%s)\n", memCountdst, pos.pS);
		}
	}
	else {              //ppos 临时放在内存中，实际变量应该放在寄存器中
		Position pos = getVarPos(item.source1, 1);
		if (pos.reg) {
			int memCOunt = ppos.memCount * 4;
			fprintf(out, "lw $t9,%d($sp)\n", memCOunt);
			fprintf(out, "move $%s,$t9\n", Dic[pos.regNum]);
		}
	}
	/*if (pos.reg) {
		int regNumdst = pos.regNum;
		if (ppos.reg) {
			int regNumsrc = ppos.regNum;
			fprintf(out, "move $%s,$%s\n", Dic[regNumdst], varDst[regNumsrc]);
		}
		else {
			int memCount = ppos.memCount * 4;
			fprintf(out, "lw $%s,%d($sp)\n", Dic[regNumdst],memCount);
			//fprintf(out, "move $%s,$t8\n", Dic[regNumdst]);
		}
	}*/
	/*else {
		int memCountdst = pos.memCount * 4;
		if (ppos.reg) {
			int regNumsrc = ppos.regNum;
			fprintf(out, "move $t9,$%s\n", varDst[regNumsrc]);
		}
		else {
			int memCount = ppos.memCount * 4;
			fprintf(out, "lw $t9,%d($sp)\n", memCount);
		}
		fprintf(out, "sw $t9,%d($%s)\n", memCountdst,pos.pS);
	}*/
}
void genFuncCall(quadItem item) {
	Position pos = getVarPos(item.dst,1);
	if (pos.reg) {
		fprintf(out, "move $%s,$v1\n", Dic[pos.regNum]);
	}
	else {
		int memCountdst = pos.memCount * 4;
		fprintf(out, "sw $v1,%d($sp)\n", memCountdst);
	}
}
void genPush(quadItem item) {       //将通用寄存器的值存到a0
	string funcName = item.dst;
	unordered_map<string, queue<pushPosition>>::iterator iter = paraMap.find(funcName);
	if (iter == paraMap.end()) {
		printf("no define func:%s\n",item.dst);
		return;
	}
	else {
		if (iter->second.empty()) {
			printf("var number not meet in func:%s\n", item.dst);
			return;
		}
		queue<pushPosition> temp = paraMap[funcName];
		int i;
		pushPosition ppos;
		if (item.callLayer > 1) {                  //参数表中有函数调用，需要a0-a3压栈
			callinPush = item.callLayer-1;
			int j;
			int curFuncVarSize = getVarNum(curFuncName) * 4 + 4;
			int raPos = (curFuncVarSize - 4) * -1;    //ra
			int memPos = raPos - (16 * callinPush);
			for (j = 0; j < 4; j++) {
				memPos -= 4;
				fprintf(out, "sw $%s,%d($sp)\n", varDst[j], memPos);
			}
		}
		for (i = 0; i <= item.pushNum; i++) {
			ppos = temp.front();
			temp.pop();
		}
		 //这个要是下一个函数sp的位置！！！
		Position pos = getVarPos(item.source1,1);
		if (pos.reg) {
			int regNumSrc = pos.regNum;
			if (ppos.reg) {
				int regNumDst = ppos.regNum;
				fprintf(out, "move $%s,$%s\n", varDst[regNumDst], Dic[regNumSrc]);
			}
			else {
				int memCountDst = ppos.memCount * 4;
				int curFuncVarSize = getVarNum(curFuncName) * 4 + 4;    //ra
				curFuncVarSize += (16 * (callinPush + 1));
				memCountDst -= curFuncVarSize;  //存到跳转过去函数的sp里面呀，sp减操作在后面
				//fprintf(out, "move $t8,$%s\n", Dic[regNumSrc]);
				fprintf(out, "sw $%s,%d($sp)\n", Dic[regNumSrc], memCountDst);
			}
		}
		else {
			int memCountSrc = pos.memCount * 4;
			fprintf(out, "lw $t9,%d($%s)\n", memCountSrc,pos.pS);
			if (ppos.reg) {
				int regNumDst = ppos.regNum;
				fprintf(out, "move $%s,$t9\n", varDst[regNumDst]);
			}
			else {
				int memCountDst = ppos.memCount * 4;
				int curFuncVarSize = getVarNum(curFuncName) * 4 + 4;    //ra
				curFuncVarSize += (16 * (callinPush + 1));
				memCountDst -= curFuncVarSize;  //存到跳转过去函数的sp里面呀，sp减操作在后面
				fprintf(out, "sw $t9,%d($sp)\n", memCountDst);
			}
		}
	}
}
void genScanf(quadItem item) {       //move $dst,$v0
	Position posDst = getVarPos(item.dst,1);
	int op = 12;
	if (item.intSource)
		op = 5;
	fprintf(out, "li $v0,%d\n", op);
	fprintf(out, "syscall\n");
	if (posDst.reg) {
		int regNum = posDst.regNum;
		fprintf(out, "move $%s,$v0\n", Dic[regNum]);
	}
	else {
		int memCount = posDst.memCount * 4;
		fprintf(out, "move $t8,$v0\n");
		fprintf(out, "sw $t8,%d($%s)\n", memCount,posDst.pS);
	}
}
void genPrintf(quadItem item) {
	int op;
	if (item.intSource == -1) {     //是字符串
		fprintf(out, "la $a0,%s\n", item.dst);
		op = 4;
	}
	else {
		Position pos = getVarPos(item.dst,1);
		if (pos.reg) {
			int regNum = pos.regNum;
			fprintf(out, "move $a0,$%s\n", Dic[regNum]);
		}
		else {
			int memCount = pos.memCount * 4;
			fprintf(out, "lw $t8,%d($%s)\n", memCount,pos.pS);
			fprintf(out, "move $a0,$t8\n");
		}
		if (item.intSource == 1)
			op = 1;
		else
			op = 11;
	}
	fprintf(out, "li $v0,%d\n", op);
	fprintf(out, "syscall\n");
}
void mipsCode() {
	quadItem item = getQuad();
	while (!isEnd(item)) {
		if (item.op.op == OperatorNum::PARAEND) {
			string sName = curFuncName;
			paraMap.insert(make_pair(sName, curPara));
			curPara = queue<pushPosition>();                //clear up
		}
		switch (item.op.op) {
		case OperatorNum::PARAEND:
			break;
		case OperatorNum::ADD:
		case OperatorNum::SUB:
			genAddorSub(item);
			break;
		case OperatorNum::MULTI:
		case OperatorNum::DIVI:
			genMultorDiv(item);
			break;
		case OperatorNum::NEG:
		case OperatorNum::ASS:
			genAssorNeg(item);
			break;
		case OperatorNum::ASSi:
			genAssi(item);
			break;
		case OperatorNum::ASAR:
		case OperatorNum::ASTK:
			genAS(item);
			break;
		case OperatorNum::EQUAL:
		case OperatorNum::NEQUAL:
			genConditionEQ(item);
			break;
		case OperatorNum::BG:
		case OperatorNum::BGE:
		case OperatorNum::LS:
		case OperatorNum::LSE:
			genConditionCal(item);
			break;
		case OperatorNum::BNZ:
		case OperatorNum::BZ:
			genConditionUni(item);
			break;
		case OperatorNum::FUNCBEGIN:
			strcpy(curFuncName, item.dst);
			createProTable(curFuncName);
			//strcpy(curFuncName, item.dst);
			break;
		case OperatorNum::FUNCEND:
			delProTable();
			break;
		case OperatorNum::GOTO:
		case OperatorNum::JAL:
			genJump(item);
			break;
		case OperatorNum::JR:
			fprintf(out, "jr $ra\n");
			break;
		case OperatorNum::LABEL:
			fprintf(out, "%s:\n", item.dst);
			break;
		case OperatorNum::RET:
			genValueReturn(item);
			break;
		case OperatorNum::PARA:
			genPara(item);
			break;
		case OperatorNum::PUSH:
			genPush(item);
			break;
		case OperatorNum::SCANF:
			genScanf(item);
			break;
		case OperatorNum::PRITNF:
			genPrintf(item);
			break;
		case OperatorNum::GLBEND:
			//printf("j main\n");
			fprintf(out, "j main\n");
			nGlbMark = 1;
			break;
		case OperatorNum::PRINTEND:
			fprintf(out, "la $a0,__\n");
			fprintf(out, "li $v0,4\n");
			fprintf(out, "syscall\n");
			break;
		case OperatorNum::GLBVAR:
		case OperatorNum::PROCVAR:
			addVarPos(item.dst);
			break;
		case OperatorNum::PARAVAR:
			addParaPos(item.dst);
			break;
		case OperatorNum::FUNCCALL:
			genFuncCall(item);
			break;
		default:
			printf("invalid\n");
		}
		item = getQuad();
	}
}