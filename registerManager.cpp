#define _CRT_SECURE_NO_WARNINGS
#include "register.h"
#include "quad.h"
#include "table.h"
using namespace std;
extern FILE* in, * out;
char tempRegDic[16][3] = { "t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5","s6","s7" };
char paraRegDic[4][3] = { "a0","a1","a2","a3" };

int glbPos = 0;         //ȫ�ַô�λ��,����process�ۼƣ���ֹ���������ڴ���
int processFlag = 0;     // ����ȫ�ֱ���������һ������begin��־���ı�
int glbSp = 0;
int gbPos = 0;
typedef struct processTable {
	unordered_map<string, Position> varList;     //��Ź��̱�����λ�õĶ�Ӧ��ϵ
	unordered_map<int, int> reg2mem;
	//vector<string> regList;         //��¼��ʱ�Ĵ�������ı��������ڼĴ��������ڴ���ұ�������Ӧ��λ��
	//vector<string> paraList;                 //����������
	unordered_map<int, int> regCount;          //���ڱ�������ļĴ�����Ҫͳ���Ƿ�ֵ
	vector<int> useRegList;
	queue<int> optRegQueue;            //��ʱ���������ɾ
	vector<string> mostUseVar;         //�ֲ�����ʹ�ô���������ȵõ��Ĵ���
	int varCount;
	int paraCount;                   //����ͳ��
	int memCount;                           //ѹ��sp�ı�������ڸù���sp��ʼλ��
	int back;                         
}processTable;
stack<vector<int>> varPosCount;  //�����õ��Ĵ������У��˻�ʱȡ���һ��vector+1���ɣ���reg��mem
stack <processTable> var2posList;       //ÿ����������ʱ�����µ�
unordered_map<string, Position> glbvarList;   //���ȫ�ֱ���λ��
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

pushPosition paraVar(char* name,int num) {       //��������������ã��ں���������ʱ�򹹽�������,��������ͳһѹջ
	string sName = name;                         //�����Ҫѹ���ڴ棬ֱ�Ӽ�¼��varList��
	processTable& pcT = var2posList.top();
	pushPosition ppos;
	Position pos;
	if (num < 4)  {                       //��ΪҪת�Ƶ�ͨ�üĴ��������Բ���Ҫ�����
		ppos.reg = true;
		ppos.regNum = pushReg(num);
	}
	else {                                               //�ڶ���ʱ�Ѿ����ڴ����ҵ�λ��
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
	/*if (strcmp(name, "#RET") == 0)    //���غ���
	{
		Position pos;
		pos.reg = true;
		pos.regNum = Reg::v1;
		return pos;
	}*/
	string sName = name;
	if (processFlag == 0) {  //ȫ�ֱ�����ȫ���������ڴ���,��gp��ʼ
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
	if (got != pcT.varList.end()) {                             //ȫ�ֱ����ض���
		Position pos = got->second;
		if (mark &&pos.reg && name[0] <= '9' && name[0] >= '0') {         //ʹ����ʱ����������Ĵ������ջ�
			int recycleNum = pos.regNum;
			vector<int>::iterator it;
			for (it = pcT.useRegList.begin(); it != pcT.useRegList.end();) {
				if (*it == recycleNum) {
					it = pcT.useRegList.erase(it);      //ɾ���Ժ��Ѿ�ָ���¸�Ԫ��
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
		// ����Ƿ���ȫ�ֱ���
		unordered_map<string, Position>::const_iterator got1 = glbvarList.find(sName);
		if (got1 != glbvarList.end())
			return got1->second;                                //��ȫ�ֱ���������ȫ�ֱ������ڴ��е�λ��
		//����ȫ�ֱ���
		pcT.varCount++;
		/*int tempRegUser = pcT.regList.size();
		if (pcT.varCount < 16) {
			pcT.regList.push_back(sName);   //�漰��ʹ����ʱ�Ĵ����ı�������Ҫת��
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
	if (processFlag == 0) {  //ȫ�ֱ�����ȫ���������ڴ���,��gp��ʼ
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
			pcT.regList.push_back(sName);   //�漰��ʹ����ʱ�Ĵ����ı�������Ҫת��
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
	if (pos < 5 && pos!=-1) {       //ǰ4�����α����ڼĴ�����
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

void storeReg() {                           //�Ĵ���ֵ�����ڴ棬����codeGeneration�е��ã���˿���д
	processTable& pcT = var2posList.top();
	int remNum = 16;
	int i;
	if (pcT.useRegList.size() < 16)
		remNum = pcT.useRegList.size();
	for (i = 0; i < remNum; i++) {
		int regNum = pcT.useRegList[i];
		if (pcT.regCount.find(regNum) != pcT.regCount.end()) {     //����������ļĴ�����δ��ʼ����
			if (pcT.regCount[regNum] == 0)
				continue;
		}
		if (pcT.reg2mem.find(regNum) == pcT.reg2mem.end()) {   //һ����ʱ�Ĵ����̶���ŵ�sp��һ��λ��
			int curMem = -1 * pcT.memCount;
			pcT.memCount++;
			pcT.reg2mem.insert(make_pair(regNum, curMem));
		}
		int mem = pcT.reg2mem[regNum];
		fprintf(out, "sw $%s,%d($sp)\n", tempRegDic[regNum], mem * 4); //�Ĵ�����ֵд���ڴ棡����
	}
	/*for (i = 0; i < remNum; i++) {
		if (pcT.reg2mem.find(i) == pcT.reg2mem.end()) {   //һ����ʱ�Ĵ����̶���ŵ�sp��һ��λ��
			int curMem = -1 * pcT.memCount;
			pcT.memCount++;
			pcT.reg2mem.insert(make_pair(i, curMem));
		}
		int mem = pcT.reg2mem[i];
		fprintf(out, "sw $%s,%d($sp)\n", tempRegDic[i], mem * 4); //�Ĵ�����ֵд���ڴ棡����
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
		if (pcT.regCount.find(regNum) != pcT.regCount.end()) {     //����������ļĴ�����δ��ʼ����
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