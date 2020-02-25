#define _CRT_SECURE_NO_WARNINGS
#include "register.h"
#include "quad.h"
#include "table.h"
using namespace std;
extern FILE* in, * out;
char tempRegDic[16][3] = { "t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5","s6","s7" };
char paraRegDic[4][3] = { "a0","a1","a2","a3" };

int glbPos = 0;         //全局访存位置,所有process累计，防止函数调用内存冲掉
int processFlag = 0;     // 区别全局变量，遇到一个函数begin标志即改变
int glbSp = 0;
int gbPos = 0;
typedef struct processTable {
	unordered_map<string, Position> varList;     //存放过程变量与位置的对应关系
	unordered_map<int, int> reg2mem;
	//vector<string> regList;         //记录临时寄存器保存的变量，用于寄存器导入内存查找变量名对应的位置
	//vector<string> paraList;                 //函数参数表
	unordered_map<int, int> regCount;          //对于变量分配的寄存器，要统计是否赋值
	vector<int> useRegList;
	queue<int> optRegQueue;            //临时变量用完就删
	vector<string> mostUseVar;         //局部变量使用次数最多优先得到寄存器
	int varCount;
	int paraCount;                   //参数统计
	int memCount;                           //压入sp的变量相对于该过程sp初始位置
	int back;                         
}processTable;
stack<vector<int>> varPosCount;  //过程用到寄存器队列，退回时取最后一个vector+1即可，先reg后mem
stack <processTable> var2posList;       //每个函数定义时创建新的
unordered_map<string, Position> glbvarList;   //存放全局变量位置
void createProTable(char* processName) {
	int i;
	processFlag++;
	processTable pt;
	pt.varCount = 0;
	pt.memCount = getArrayEnd(processName);
	pt.varList = unordered_map<string, Position>();
	pt.reg2mem = unordered_map<int, int>();
	//pt.regList = vector<string>();
	pt.useRegList = vector<int>();
	pt.optRegQueue = queue<int>();
	pt.regCount = unordered_map<int, int>();
	pt.paraCount = 0;
	for (i = 0; i < 16; i++) {
		pt.optRegQueue.push(i);
	}
	pt.mostUseVar = getMostVarUse(processName);
	pt.back = getVarNum(processName);
	var2posList.push(pt);
}

void delProTable() {
	var2posList.pop();
}

bool findVar(char* name) {
	string sName = name;
	processTable pcT = var2posList.top();
	if (pcT.varList.find(sName) == pcT.varList.end())
		return false;
	return true;
}

pushPosition paraVar(char* name,int num) {       //函数先声明后调用，在函数声明的时候构建参数表,参数传递统一压栈
	string sName = name;                         //如果需要压如内存，直接记录在varList中
	processTable& pcT = var2posList.top();
	pushPosition ppos;
	Position pos;
	if (num < 4)  {                       //因为要转移到通用寄存器，所以不需要计入表
		ppos.reg = true;
		ppos.regNum = pushReg(num);
	}
	else {                                               //在定义时已经在内存中找到位置
		if (pcT.varList.find(sName) == pcT.varList.end())
			printf("can't find var:%s\n", name);
		else {
			ppos.reg = false;
			ppos.memCount = pcT.varList[sName].memCount;
		}
	}
	return ppos;
}


Position getVarPos(char* name,int mark) {
	/*if (strcmp(name, "#RET") == 0)    //返回函数
	{
		Position pos;
		pos.reg = true;
		pos.regNum = Reg::v1;
		return pos;
	}*/
	string sName = name;
	if (processFlag == 0) {  //全局变量，全部保存在内存中,从gp开始
		unordered_map<string, Position>::const_iterator got1 = glbvarList.find(sName);
		if (got1 != glbvarList.end())
			return got1->second;
		Position pos;
		pos.reg = false;
		pos.memCount = gbPos;
		gbPos++;
		strcpy(pos.pS, "gp");
		glbvarList.insert(make_pair(sName, pos));
		return pos;
	}
	processTable& pcT = var2posList.top();
	unordered_map<string, Position>::const_iterator got = pcT.varList.find(sName);
	if (got != pcT.varList.end()) {                             //全局变量重定义
		Position pos = got->second;
		if (mark &&pos.reg && name[0] <= '9' && name[0] >= '0') {         //使用临时变量，用完寄存器就收回
			int recycleNum = pos.regNum;
			vector<int>::iterator it;
			for (it = pcT.useRegList.begin(); it != pcT.useRegList.end();) {
				if (*it == recycleNum) {
					it = pcT.useRegList.erase(it);      //删除以后已经指向下个元素
				}
				else {
					++it;
				}
			}
			pcT.optRegQueue.push(recycleNum);
		}
		else if (pos.reg && (name[0] > '9' || name[0] < '0')) {
			int num = pos.regNum;
			if (pcT.regCount.find(num) == pcT.regCount.end()) {
				printf("!!!!NOT FOUND\n");
			}
			else {
				pcT.regCount[num]++;
			}
		}
		return pos;
	}
	else {
		// 检查是否是全局变量
		unordered_map<string, Position>::const_iterator got1 = glbvarList.find(sName);
		if (got1 != glbvarList.end())
			return got1->second;                                //是全局变量，返回全局变量在内存中的位置
		//不是全局变量
		pcT.varCount++;
		/*int tempRegUser = pcT.regList.size();
		if (pcT.varCount < 16) {
			pcT.regList.push_back(sName);   //涉及到使用临时寄存器的变量才需要转移
			Position pos;
			pos.reg = true;
			pos.regNum = Reg(tempRegUser);
			pcT.varList.insert(make_pair(sName, pos));
			return pos;
		}*/
		if (!pcT.optRegQueue.empty()) {
			Position pos;
			pos.reg = true;
			int num = pcT.optRegQueue.front();
			pos.regNum = Reg(num);
			pcT.optRegQueue.pop();
			pcT.useRegList.push_back(num);
			pcT.varList.insert(make_pair(sName, pos));
			return pos;
		}
		else {
			Position pos;
			pos.reg = false;
			pos.memCount = -1*pcT.memCount;
			pcT.memCount++;
			strcpy(pos.pS, "sp");
			pcT.varList.insert(make_pair(sName, pos));
			return pos;
		}
	}
}

void addVarPos(char* name) {
	string sName = name;
	if (processFlag == 0) {  //全局变量，全部保存在内存中,从gp开始
		unordered_map<string, Position>::const_iterator got1 = glbvarList.find(sName);
		if (got1 != glbvarList.end())
			return;
		Position pos;
		pos.reg = false;
		pos.memCount = gbPos;
		gbPos++;
		strcpy(pos.pS, "gp");
		glbvarList.insert(make_pair(sName, pos));
	}
	else {
		
		processTable& pcT = var2posList.top();
		unordered_map<string, Position>::const_iterator got = pcT.varList.find(sName);
		if (got != pcT.varList.end()) {
			printf("!!!!!!!redefine var%s\n!!!!!!", name);
			return;
		}
		//pcT.varCount++;
		//int tempRegUser = pcT.regList.size();
		/*if (pcT.varCount < 5) {
			pcT.regList.push_back(sName);   //涉及到使用临时寄存器的变量才需要转移
			Position pos;
			pos.reg = true;
			pos.regNum = Reg(tempRegUser);
			pcT.varList.insert(make_pair(sName, pos));
		}*/
		vector<string>::iterator it = find(pcT.mostUseVar.begin(), pcT.mostUseVar.end(), sName);
		int pos;
		if (it == pcT.mostUseVar.end())
			pos = -1;
		else
			pos = distance(pcT.mostUseVar.begin(), it);
		pcT.varCount++;
		if(!pcT.optRegQueue.empty() && pos < 3 && pos!=-1){
			Position pos;
			pos.reg = true;
			int num = pcT.optRegQueue.front();
			pos.regNum = Reg(num);
			pcT.optRegQueue.pop();
			pcT.useRegList.push_back(num);
			pcT.varList.insert(make_pair(sName, pos));
			pcT.regCount.insert(make_pair(num, 0));
		}
		else {
			Position pos;
			pos.reg = false;
			pos.memCount = -1 * pcT.memCount;
			pcT.memCount++;
			strcpy(pos.pS, "sp");
			pcT.varList.insert(make_pair(sName, pos));
		}
	}
}

void addParaPos(char* name) {
	string sName = name;
	processTable& pcT = var2posList.top();
	unordered_map<string, Position>::const_iterator got = pcT.varList.find(sName);
	if (got != pcT.varList.end()) {
		printf("!!!!!!!redefine var%s\n!!!!!!", name);
		return;
	}
	vector<string>::iterator it = find(pcT.mostUseVar.begin(), pcT.mostUseVar.end(), sName);
	int pos;
	if (it == pcT.mostUseVar.end())
		pos = -1;
	else
		pos = distance(pcT.mostUseVar.begin(), it);
	pcT.varCount++;
	if (pos < 5 && pos!=-1) {       //前4个传参保存在寄存器中
		if (!pcT.optRegQueue.empty()) {
			Position pos;
			pos.reg = true;
			int num = pcT.optRegQueue.front();
			pos.regNum = Reg(num);
			pcT.optRegQueue.pop();
			pcT.useRegList.push_back(num);
			if (pcT.varCount >= 5) {
				pos.memCount = -1*pcT.back;
				pcT.back -= 1;
			}
			pcT.varList.insert(make_pair(sName, pos));
			pcT.regCount.insert(make_pair(num, 0));
		}
		else {
			printf("!!!!!WTF?!!!\n");
		}
	}
	else {
		Position pos;
		pos.reg = false;
		pos.memCount = -1 * pcT.memCount;
		pcT.memCount++;
		strcpy(pos.pS, "sp");
		pcT.varList.insert(make_pair(sName, pos));
	}

}

void storeReg() {                           //寄存器值进入内存，仅在codeGeneration中调用，因此可以写
	processTable& pcT = var2posList.top();
	int remNum = 16;
	int i;
	if (pcT.useRegList.size() < 16)
		remNum = pcT.useRegList.size();
	for (i = 0; i < remNum; i++) {
		int regNum = pcT.useRegList[i];
		if (pcT.regCount.find(regNum) != pcT.regCount.end()) {     //分配给变量的寄存器还未初始化过
			if (pcT.regCount[regNum] == 0)
				continue;
		}
		if (pcT.reg2mem.find(regNum) == pcT.reg2mem.end()) {   //一个临时寄存器固定存放到sp的一个位置
			int curMem = -1 * pcT.memCount;
			pcT.memCount++;
			pcT.reg2mem.insert(make_pair(regNum, curMem));
		}
		int mem = pcT.reg2mem[regNum];
		fprintf(out, "sw $%s,%d($sp)\n", tempRegDic[regNum], mem * 4); //寄存器的值写入内存！！！
	}
	/*for (i = 0; i < remNum; i++) {
		if (pcT.reg2mem.find(i) == pcT.reg2mem.end()) {   //一个临时寄存器固定存放到sp的一个位置
			int curMem = -1 * pcT.memCount;
			pcT.memCount++;
			pcT.reg2mem.insert(make_pair(i, curMem));
		}
		int mem = pcT.reg2mem[i];
		fprintf(out, "sw $%s,%d($sp)\n", tempRegDic[i], mem * 4); //寄存器的值写入内存！！！
	}*/
	
}

void restoreReg() {
	processTable& pcT = var2posList.top();
	/*int restoreNum = pcT.reg2mem.size();
	int i;
	for (i = 0; i < restoreNum; i++) {
		int memPos = pcT.reg2mem[i] * 4;
		fprintf(out, "lw $%s,%d($sp)\n", tempRegDic[i], memPos);
	}*/
	int restoreNum = pcT.useRegList.size();
	int i;
	for (i = 0; i < restoreNum; i++) {
		int regNum = pcT.useRegList[i];
		if (pcT.regCount.find(regNum) != pcT.regCount.end()) {     //分配给变量的寄存器还未初始化过
			if (pcT.regCount[regNum] == 0)
				continue;
		}
		if (pcT.reg2mem.find(regNum) == pcT.reg2mem.end()) {
			printf("!!!!!!NOT PUT IN MEM!!!!\n");
			return;
		}
		int memPos = pcT.reg2mem[regNum] * 4;
		fprintf(out, "lw $%s,%d($sp)\n", tempRegDic[regNum], memPos);
	}

}