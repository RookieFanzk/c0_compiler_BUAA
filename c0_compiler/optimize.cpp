#include "optimize.h"

OpMidCode opmidcode[MAX_MID_CODE];

BasicBlock blockflow[MAX_BLOCK_NUM];
int blocknum_count = 0;

FlowDiagram flow[MAX_FLOW_NUM];
int flow_index = 0;

DagNode dagnode[MAX_DAGNODE];
int nodenum = 0;

NodeTable nodetab[MAX_DAGNODE];
int nodetab_index = 0;

ConflictMap conflictmap[100];
int conflict_index = 0;

FOURYUANCODE newmidcode[MAX_MID_CODE];
int newmidcodenum = 0;

int global_judge(char *varname)
{
	int globaltail = symTable.indexOfpro[0];
	for (int i = 0; i < globaltail; i++) {
		if (strcmp(symTable.symolArray[i].idname, varname) == 0) {
			return 1;
		}
	}
	return 0;
}

int temp_judge(char *varname)
{
	if (varname[0] == 'V' && varname[1] == 'A' && varname[2] == 'R' && varname[3] == '_') {
		return 1;
	}
	return 0;
}

OpMidCode toOpmidcode(FOURYUANCODE item, OpMidCode opitem, int flag)
{
	strcpy(opitem.op, item.op);
	strcpy(opitem.var1, item.var1);
	strcpy(opitem.var2, item.var2);
	strcpy(opitem.rst, item.rst);
	opitem.isblockbegin = flag;
	return opitem;
}

void doorsentence()
{
	constMerge();
	int i;
	FOURYUANCODE item;
	for (i = 0; i < codenum_count; i++) {
		item = midcode[i];
		if (i == 0) {
			opmidcode[i] = toOpmidcode(item, opmidcode[i], true);
		}
		else if (strcmp(item.op, Func) == 0) {
			opmidcode[i] = toOpmidcode(item, opmidcode[i], true);
		}
		else if (strcmp(item.op, Lab) == 0) {
			opmidcode[i] = toOpmidcode(item, opmidcode[i], true);
		}
		/*else if (strcmp(item.op, Scanf) == 0) {
			opmidcode[i] = toOpmidcode(item, opmidcode[i], true);
		}
		else if (strcmp(item.op, Printf) == 0) {
			opmidcode[i] = toOpmidcode(item, opmidcode[i], true);
		}
		else if (strcmp(item.op, Call) == 0) {
			opmidcode[i] = toOpmidcode(item, opmidcode[i], true);
		}*/
		else {
			if (strcmp(midcode[i - 1].op, Jmp) == 0 || strcmp(midcode[i - 1].op, Bgez) == 0 || strcmp(midcode[i - 1].op, Bgtz) == 0 || strcmp(midcode[i - 1].op, Blez) == 0 || strcmp(midcode[i - 1].op, Bltz) == 0 || strcmp(midcode[i - 1].op, Beq) == 0 || strcmp(midcode[i - 1].op, Bne) == 0) {
				opmidcode[i] = toOpmidcode(item, opmidcode[i], true);
			}
			/*else if (strcmp(midcode[i - 1].op, Ret) == 0 || strcmp(midcode[i - 1].op, Exit) == 0) {
				if (strcmp(item.op, Funcend) != 0) {
					opmidcode[i] = toOpmidcode(item, opmidcode[i], true);
				}
			}*/
			else {
				opmidcode[i] = toOpmidcode(item, opmidcode[i], false);
			}
		}
	}
}

void devideBlock()
{
	doorsentence();
	int i = 0;
	OpMidCode opitem;
	while(i < codenum_count) {
		opitem = opmidcode[i];
		if (opitem.isblockbegin == 1) {
			int codenum = 0;
			int k;
			blockflow[blocknum_count].blockcode[codenum] = opitem;
			codenum++;
			for (k = i + 1; k < codenum_count; k++) {
				opitem = opmidcode[k];
				if (opitem.isblockbegin == 0) {
					blockflow[blocknum_count].blockcode[codenum] = opitem;
					codenum++;
				}
				else {
					blockflow[blocknum_count].blocknum = blocknum_count;
					blockflow[blocknum_count].codenum = codenum;
					blocknum_count++;
					break;
				}
			}
			if (k == codenum_count) {
				blockflow[blocknum_count].blocknum = blocknum_count;
				blockflow[blocknum_count].codenum = codenum;
				blocknum_count++;
			}
			i = k;
		}
	}
}

int search_tempvar(int blocknum, char *tempvar)
{
	int i;
	for (i = 0; i < blockflow[blocknum].tempnum; i++) {
		if (strcmp(blockflow[blocknum].tempvarReg[i].tempvar, tempvar) == 0) {
			return 1;
		}
	}
	return 0;
}

void allocate_temp_reg()
{
	int i;
	for (i = 0; i < blocknum_count; i++) {
		for (int j = 0; j < blockflow[i].codenum; j++) {
			OpMidCode opitem = blockflow[i].blockcode[j];
			if (opitem.rst[0] == 'V' && opitem.rst[1] == 'A' && opitem.rst[2] == 'R' && opitem.rst[3] == '_') {
				if (!search_tempvar(i, opitem.rst)) {
					if (blockflow[i].tempnum <= 9) {
						strcpy(blockflow[i].tempvarReg[blockflow[i].tempnum].tempvar, opitem.rst);
						strcpy(blockflow[i].tempvarReg[blockflow[i].tempnum].reg, tempreg[blockflow[i].tempnum]);
						blockflow[i].tempnum++;
					}
					else {
						int m = -1, n = -1;
						for (int k = 0; k < blockflow[i].tempnum; k++) {
							if (strcmp(blockflow[i].tempvarReg[k].tempvar, opitem.var1) == 0) {
								m = blockflow[i].tempvarReg[k].reg[2] - '0';
							}
							if (strcmp(blockflow[i].tempvarReg[k].tempvar, opitem.var2) == 0) {
								n = blockflow[i].tempvarReg[k].reg[2] - '0';
							}
						}
						for (int x = 0; x < 10; x++) {
							if (x != m && x != n) {
								char reg[5] = "$t";
								char a[2];
								itoa(x, a, 10);
								strcat(reg, a);
								strcpy(blockflow[i].tempvarReg[blockflow[i].tempnum].tempvar, opitem.rst);
								strcpy(blockflow[i].tempvarReg[blockflow[i].tempnum].reg, reg);
								blockflow[i].tempnum++;
								break;
							}
						}
					}
				}
			}
		}
	}
}

void init_Flow()
{
	int i;
	for (i = 0; i < MAX_FLOW_NUM; i++) {
		flow[i].blocknum = 0;
		for (int j = 0; j < MAX_BLOCK_NUM; j++) {
			blockflow[flow[i].block[j]].defnum = 0;
			blockflow[flow[i].block[j]].usenum = 0;
			blockflow[flow[i].block[j]].innum = 0;
			blockflow[flow[i].block[j]].outnum = 0;
		}
	}
}

void genFlow()
{
	init_Flow();
	int i;
	for (i = 0; i < blocknum_count; i++) {
		//����ȫ�ֳ������������飬������һ����ͼ,����ֻ��һ��������
		if((i == 0) && (strcmp(blockflow[i].blockcode[0].op, Func) != 0)) {
			flow[flow_index].block[0] = 0;
			flow[flow_index].blocknum = 1;
			flow_index++;
		}
		//Ϊÿ����������һ����ͼ
		if (strcmp(blockflow[i].blockcode[0].op, Func) == 0) {
			flow[flow_index].block[0] = i++;
			flow[flow_index].blocknum++;
			while ((i < blocknum_count) && (strcmp(blockflow[i].blockcode[0].op, Func) != 0)) {
				flow[flow_index].block[flow[flow_index].blocknum] = i++;
				flow[flow_index].blocknum++;
			}
			flow_index++;
			i--;
		}
	}
}

void get_nextblock()
{
	int i,j;
	//ǰ������̻������ϵֻ��ÿ����ͼ�н���
	//iΪ��ͼ������jΪ��ͼ�л���������
	//nextblock[2]�����У��±�0�洢��̿�Ϊ��һ������������1�洢��תĿ�������
	for (i = 0; i < flow_index; i++) {
		for (j = 0; j < flow[i].blocknum; j++) {
			//��λ��ÿ������������һ���м���롣��Ҫͨ���������һ�������һ�����������ӻ�����
			int tailcode = blockflow[flow[i].block[j]].codenum - 1;
			//�Ȱ����п��nextblock[1]����ͼ�����һ���������nextblock[0]��ʼ��Ϊ-1
			blockflow[flow[i].block[j]].nextblock[1] = -1;
			if(j == flow[i].blocknum-1)
				blockflow[flow[i].block[j]].nextblock[0] = -1;
			//����һ��������ʼ��־�������ǿպ��������߽���һ��������䣬��̼�Ϊ��һ��������
			if (strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Func) == 0) {
				blockflow[flow[i].block[j]].nextblock[0] = flow[i].block[j] + 1;
			}
			//��������תָ����ֻ����תĿ��
			else if (strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Jmp) == 0) {
				blockflow[flow[i].block[j]].nextblock[0] = -1;
				int findflag = false;
				for (int k = 0; k < blocknum_count; k++) {
					for (int m = 0; m < blockflow[k].codenum; m++) {
						if (strcmp(blockflow[k].blockcode[m].op, Lab) == 0 && strcmp(blockflow[k].blockcode[m].rst, blockflow[flow[i].block[j]].blockcode[tailcode].rst) == 0) {
							blockflow[flow[i].block[j]].nextblock[1] = k;
							findflag = true;
							break;
						}
					}
					if (findflag == true) {
						break;
					}
				}
			}
			//������תָ����Ϊ��һ�����������תĿ��
			else if (strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Beq) == 0 || strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Bne) == 0 || strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Bgez) == 0 || strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Bgtz) == 0 || strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Blez) == 0 || strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Bltz) == 0) {
				blockflow[flow[i].block[j]].nextblock[0] = flow[i].block[j] + 1;
				int findflag = false;
				for (int k = 0; k < blocknum_count; k++) {
					for (int m = 0; m < blockflow[k].codenum; m++) {
						if (strcmp(blockflow[k].blockcode[m].op, Lab) == 0 && strcmp(blockflow[k].blockcode[m].rst, blockflow[flow[i].block[j]].blockcode[tailcode].rst) == 0) {
							blockflow[flow[i].block[j]].nextblock[1] = k;
							findflag = true;
							break;
						}
					}
					if (findflag == true) {
						break;
					}
				}
			}
			//���һ�������������һ��������return��exit��˵���ú������ж��return
			//else if (strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Ret) == 0 || strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Exit) == 0) {
			//	blockflow[flow[i].block[j]].nextblock[0] = j + 1;
			//}
			//Ϊ����������־���޺�̿�
			else if (strcmp(blockflow[flow[i].block[j]].blockcode[tailcode].op, Funcend) == 0) {
				blockflow[flow[i].block[j]].nextblock[0] = -1;
			}
			else {
				blockflow[flow[i].block[j]].nextblock[0] = flow[i].block[j] + 1;
			}
		}
	}
}
	
int defjudge(int blocknum, char *varname)
{
	int i;
	//������Ԫʽ����ƣ�ĳһ�����Ϊ��
	if (strcmp(varname, Space) == 0) {
		return 1;
	}
	for (i = 0; i < blockflow[blocknum].defnum; i++) {
		if (strcmp(blockflow[blocknum].defvar[i], varname) == 0) {
			return 1;
		}
	}
	return 0;
}

int usejudge(int blocknum, char *varname)
{
	int i;
	if (strcmp(varname, Space) == 0) {
		return 1;
	}
	for (i = 0; i < blockflow[blocknum].usenum; i++) {
		if (strcmp(blockflow[blocknum].usevar[i], varname) == 0) {
			return 1;
		}
	}
	return 0;
}

//����Ϊ����������ͼ�л�����������flagΪdef��use��־���õ�ÿһ���������def,use
void getBdef_use(char *varname, int blocknum, int flag)
{	
	//def��������ʹ��
	if (flag == 1) {
		if (usejudge(blocknum, varname) == 0) {
			if (defjudge(blocknum, varname) == 0) {
				strcpy(blockflow[blocknum].defvar[blockflow[blocknum].defnum], varname);
				blockflow[blocknum].defnum++;
			}
		}
	}
	//useʹ�����ڶ���
	else {
		if (defjudge(blocknum, varname) == 0) {
			if (usejudge(blocknum, varname) == 0) {
				strcpy(blockflow[blocknum].usevar[blockflow[blocknum].usenum], varname);
				blockflow[blocknum].usenum++;
			}
		}
	}
}

//Ϊÿ����ͼ��ÿ�������齨��def,use,����Ϊ��ͼ����
void getFdef_use(int f)
{
	for (int i = 0; i < flow[f].blocknum; i++) {
		//��ͼ��ÿ�����������Ϊ�������
		int blocknum = flow[f].block[i];
		for (int j = 0; j < blockflow[blocknum].codenum; j++) {
			if (strcmp(blockflow[blocknum].blockcode[j].op, Para) == 0) {
				getBdef_use(blockflow[blocknum].blockcode[j].rst, blocknum, 0);
			}
			else if (strcmp(blockflow[blocknum].blockcode[j].op, ParaCall) == 0) {
				if (!global_judge(blockflow[blocknum].blockcode[j].rst) && !temp_judge(blockflow[blocknum].blockcode[j].rst) && !(Allisnum(blockflow[blocknum].blockcode[j].rst) || blockflow[blocknum].blockcode[j].rst[0] == '-')) {
					getBdef_use(blockflow[blocknum].blockcode[j].rst, blocknum, 0);
				}
			}
			else if (strcmp(blockflow[blocknum].blockcode[j].op, Scanf) == 0) {
				if (!global_judge(blockflow[blocknum].blockcode[j].rst) && !temp_judge(blockflow[blocknum].blockcode[j].rst)) {
					getBdef_use(blockflow[blocknum].blockcode[j].rst, blocknum, 0);
				}
			}
			else if (strcmp(blockflow[blocknum].blockcode[j].op, Printf) == 0) {
				if (!global_judge(blockflow[blocknum].blockcode[j].var2) && !temp_judge(blockflow[blocknum].blockcode[j].var2) && !(Allisnum(blockflow[blocknum].blockcode[j].var2) || blockflow[blocknum].blockcode[j].var2[0] == '-')) {
					getBdef_use(blockflow[blocknum].blockcode[j].var2, blocknum, 0);
				}
			}
			else if (strcmp(blockflow[blocknum].blockcode[j].op, Ret) == 0) {
				if (!global_judge(blockflow[blocknum].blockcode[j].rst) && !temp_judge(blockflow[blocknum].blockcode[j].rst) && !(Allisnum(blockflow[blocknum].blockcode[j].rst) || blockflow[blocknum].blockcode[j].rst[0] == '-')) {
					getBdef_use(blockflow[blocknum].blockcode[j].rst, blocknum, 0);
				}
			}
			else if (strcmp(blockflow[blocknum].blockcode[j].op, Add) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Sub) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Mul) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Div) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Assign) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Assarr) == 0) {
				if (strcmp(blockflow[blocknum].blockcode[j].op, Assign) == 0) {
					if (strcmp(blockflow[blocknum].blockcode[j].var2, Space) == 0) {
						if (!global_judge(blockflow[blocknum].blockcode[j].var1) && !temp_judge(blockflow[blocknum].blockcode[j].var1) && !(Allisnum(blockflow[blocknum].blockcode[j].var1) || blockflow[blocknum].blockcode[j].var1[0] == '-')) {
							getBdef_use(blockflow[blocknum].blockcode[j].var1, blocknum, 0);
						}
						if (!global_judge(blockflow[blocknum].blockcode[j].rst) && !temp_judge(blockflow[blocknum].blockcode[j].rst)) {
							getBdef_use(blockflow[blocknum].blockcode[j].rst, blocknum, 1);
						}
					}
					else {
						if (!global_judge(blockflow[blocknum].blockcode[j].var2) && !temp_judge(blockflow[blocknum].blockcode[j].var2) && !(Allisnum(blockflow[blocknum].blockcode[j].var2) || blockflow[blocknum].blockcode[j].var2[0] == '-')) {
							getBdef_use(blockflow[blocknum].blockcode[j].var2, blocknum, 0);
						}
						if (!global_judge(blockflow[blocknum].blockcode[j].rst) && !temp_judge(blockflow[blocknum].blockcode[j].rst)) {
							getBdef_use(blockflow[blocknum].blockcode[j].rst, blocknum, 1);
						}
					}
				}
				else if (strcmp(blockflow[blocknum].blockcode[j].op, Assarr) == 0) {
					if (!global_judge(blockflow[blocknum].blockcode[j].var1) && !temp_judge(blockflow[blocknum].blockcode[j].var1) && !(Allisnum(blockflow[blocknum].blockcode[j].var1) || blockflow[blocknum].blockcode[j].var1[0] == '-')) {
						getBdef_use(blockflow[blocknum].blockcode[j].var1, blocknum, 0);
					}
					if (!global_judge(blockflow[blocknum].blockcode[j].var2) && !temp_judge(blockflow[blocknum].blockcode[j].var2) && !(Allisnum(blockflow[blocknum].blockcode[j].var2) || blockflow[blocknum].blockcode[j].var2[0] == '-')) {
						getBdef_use(blockflow[blocknum].blockcode[j].var2, blocknum, 0);
					}
				}
				else {
					if (!global_judge(blockflow[blocknum].blockcode[j].var1) && !temp_judge(blockflow[blocknum].blockcode[j].var1) && !(Allisnum(blockflow[blocknum].blockcode[j].var1) || blockflow[blocknum].blockcode[j].var1[0] == '-')) {
						getBdef_use(blockflow[blocknum].blockcode[j].var1, blocknum, 0);
					}
					if (!global_judge(blockflow[blocknum].blockcode[j].var2) && !temp_judge(blockflow[blocknum].blockcode[j].var2) && !(Allisnum(blockflow[blocknum].blockcode[j].var2) || blockflow[blocknum].blockcode[j].var2[0] == '-')) {
						getBdef_use(blockflow[blocknum].blockcode[j].var2, blocknum, 0);
					}
					if (!global_judge(blockflow[blocknum].blockcode[j].rst) && !temp_judge(blockflow[blocknum].blockcode[j].rst) && !(Allisnum(blockflow[blocknum].blockcode[j].rst) || blockflow[blocknum].blockcode[j].rst[0] == '-')) {
						getBdef_use(blockflow[blocknum].blockcode[j].rst, blocknum, 1);
					}
				}
			}
			else if (strcmp(blockflow[blocknum].blockcode[j].op, Beq) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Bne) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Bgez) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Bgtz) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Blez) == 0 || strcmp(blockflow[blocknum].blockcode[j].op, Bltz) == 0) {
				if (!global_judge(blockflow[blocknum].blockcode[j].var1) && !temp_judge(blockflow[blocknum].blockcode[j].var1) && !(Allisnum(blockflow[blocknum].blockcode[j].var1) || blockflow[blocknum].blockcode[j].var1[0] == '-')) {
					getBdef_use(blockflow[blocknum].blockcode[j].var1, blocknum, 0);
				}
			}
		}
	}
}

int search_in(int blocknum, char *varname)
{
	int i;
	if (strcmp(varname, Space) == 0) {
		return 1;
	}
	for (i = 0; i < blockflow[blocknum].innum; i++) {
		if (strcmp(blockflow[blocknum].invar[i], varname) == 0) {
			return 1;
		}
	}
	return 0;
}

int search_out(int blocknum, char *varname)
{
	int i;
	if (strcmp(varname, Space) == 0) {
		return 1;
	}
	for (i = 0; i < blockflow[blocknum].outnum; i++) {
		if (strcmp(blockflow[blocknum].outvar[i], varname) == 0) {
			return 1;
		}
	}
	return 0;
}

void getinout(int f)
{
	int times;
	for (times = 0; times < 1000; times++) {
		int flag = 1;
		for (int i = 0; i < flow[f].blocknum; i++) {
			int blocknum = flow[f].block[i];
			int lastInnum = blockflow[blocknum].innum;
			//out
			for (int j = 0; j <= 1; j++) {
				if (blockflow[blocknum].nextblock[j] != -1) {
					for (int k = 0; k < blockflow[blockflow[blocknum].nextblock[j]].innum; k++) {
						if (!search_out(blocknum, blockflow[blockflow[blocknum].nextblock[j]].invar[k])) {
							strcpy(blockflow[blocknum].outvar[blockflow[blocknum].outnum], blockflow[blockflow[blocknum].nextblock[j]].invar[k]);
							blockflow[blocknum].outnum++;
						}
					}
				}
			}
			//in = use and (out - def)
			//use
			for (int k = 0; k < blockflow[blocknum].usenum; k++) {
				if (!search_in(blocknum, blockflow[blocknum].usevar[k])) {
					strcpy(blockflow[blocknum].invar[blockflow[blocknum].innum], blockflow[blocknum].usevar[k]);
					blockflow[blocknum].innum++;
				}
			}
			//out - def
			for (int k = 0; k < blockflow[blocknum].outnum; k++) {
				if (!search_in(blocknum, blockflow[blocknum].outvar[k])) {
					if (!defjudge(blocknum, blockflow[blocknum].outvar[k])) {	//����out�еı�������def��
						strcpy(blockflow[blocknum].invar[blockflow[blocknum].innum], blockflow[blocknum].outvar[k]);
						blockflow[blocknum].innum++;
					}
				}
			}
			if (lastInnum != blockflow[blocknum].innum) {
				flag = 0;
			}
		}
		if (flag == 1) {
			break;
		}
	}
}

//�ж��Ƿ��Ѿ�������varname�ĳ�ͻͼ
int search_confmapvar(int f, char *varname)
{
	int i;
	for (i = 0; i < flow[f].conflictnum; i++) {
		if (strcmp(flow[f].conflictmap[i].name, varname) == 0) {
			return i;
		}
	}
	return -1;
}

//�ж�var2�Ƿ��Ѿ���var1�ĳ�ͻͼ��
int search_confmap(int f, char *var1 , char *var2)
{
	int i;
	for (i = 0; i < flow[f].conflictnum; i++) {
		if (strcmp(flow[f].conflictmap[i].name, var1) == 0) {
			for (int j = 0; j < flow[f].conflictmap[i].confvarnum; j++) {
				if (strcmp(flow[f].conflictmap[i].confVar[j], var2) == 0) {
					return 1;
				}
			}
		}
	}
	return 0;
}

void genConflictmap(int f)
{
	for (int j = 0; j < flow[f].blocknum; j++) {
		for (int k = 0; k < blockflow[flow[f].block[j]].innum; k++) {
			int flag = search_confmapvar(f, blockflow[flow[f].block[j]].invar[k]);
			if (flag == -1) {
				strcpy(flow[f].conflictmap[flow[f].conflictnum].name, blockflow[flow[f].block[j]].invar[k]);
				flag = flow[f].conflictnum;
				flow[f].conflictnum++;
			}
			for (int m = 0; m < blockflow[flow[f].block[j]].innum; m++) {
				if (!search_confmap(f, flow[f].conflictmap[flag].name, blockflow[flow[f].block[j]].invar[m])) {
					if (strcmp(flow[f].conflictmap[flag].name, blockflow[flow[f].block[j]].invar[m]) != 0) {
						strcpy(flow[f].conflictmap[flag].confVar[flow[f].conflictmap[flag].confvarnum], blockflow[flow[f].block[j]].invar[m]);
						flow[f].conflictmap[flag].confvarnum++;
						flow[f].conflictmap[flag].edgenum++;
					}
				}
			}
			for (int m = 0; m < blockflow[flow[f].block[j]].outnum; m++) {
				if (!search_confmap(f, flow[f].conflictmap[flag].name, blockflow[flow[f].block[j]].outvar[m])) {
					if (strcmp(flow[f].conflictmap[flag].name, blockflow[flow[f].block[j]].outvar[m]) != 0) {
						strcpy(flow[f].conflictmap[flag].confVar[flow[f].conflictmap[flag].confvarnum], blockflow[flow[f].block[j]].outvar[m]);
						flow[f].conflictmap[flag].confvarnum++;
						flow[f].conflictmap[flag].edgenum++;
					}
				}
			}
		}
	}
}

int getnextnode(int f)
{
	int i;
	for (i = 0; i < flow[f].conflictnum; i++) {
		if (flow[f].conflictmap[i].edgenum < 8 && flow[f].conflictmap[i].isAllocated != 1) {
			return i;
		}
	}
	return -1;
}

//�õ�������Ϊvarname�ĳ�ͻͼ�±�
int getvarConfnum(int f, char *varname)
{
	int i;
	for (i = 0; i < flow[f].conflictnum; i++) {
		if (strcmp(flow[f].conflictmap[i].name, varname) == 0) {
			return i;
		}
	}
	return -1;
}

void allocate_reg(int f)
{
	int nodestack[100];
	int stackindex = 0;
	int node;
	int temp = flow[f].conflictnum;	//��ͻͼ��ʣ��ڵ���
	while (temp > 0) {
		node = getnextnode(f);
		while (node != -1) {
			flow[f].conflictmap[node].isAllocated = 1;
			flow[f].conflictmap[node].flag = 1;
			nodestack[stackindex++] = node;
			temp--;
			for (int i = 0; i < flow[f].conflictnum; i++) {
				for (int j = 0; j < flow[f].conflictmap[i].confvarnum; j++) {
					if (strcmp(flow[f].conflictmap[node].name, flow[f].conflictmap[i].confVar[j]) == 0) {
						flow[f].conflictmap[i].edgenum--;
					}
				}
			}
			node = getnextnode(f);
		}
		if (temp == 0) {
			break;
		}
		//���Է�����Ѿ������ˣ������������ӱߴ��ڵ���8�Ľڵ�
		for (int i = 0; i < flow[f].conflictnum; i++) {
			if (flow[f].conflictmap[i].confvarnum >= 8 && flow[f].conflictmap[i].isAllocated != 1) {
				flow[f].conflictmap[i].isAllocated = 1;
				flow[f].conflictmap[i].flag = 0;
				nodestack[stackindex++] = i;
				node = i;
				temp--;
				break;
			}
		}
		for (int i = 0; i < flow[f].conflictnum; i++) {
			for (int j = 0; j < flow[f].conflictmap[i].confvarnum; j++) {
				if (strcmp(flow[f].conflictmap[node].name, flow[f].conflictmap[i].confVar[j]) == 0) {
					flow[f].conflictmap[i].edgenum--;
				}
			}
		}
	}
	if (flow[f].conflictnum != 0) {
		for (int i = stackindex - 1; i >= 0; i--) {
			int colornode = -1;
			int color[8] = { 1,2,3,4,5,6,7,8 };
			if (flow[f].conflictmap[nodestack[i]].flag == 1) {
				for (int j = 0; j < flow[f].conflictmap[nodestack[i]].confvarnum; j++) {
					int index = getvarConfnum(f, flow[f].conflictmap[nodestack[i]].confVar[j]);
					for (int k = 0; k < 8; k++) {
						if (flow[f].conflictmap[index].color == color[k]) {
							color[k] = -1;
						}
					}
				}
				for (int k = 0; k < 8; k++) {
					if (color[k] != -1) {
						flow[f].conflictmap[nodestack[i]].color = color[k];
						char a[2];
						itoa(color[k] - 1, a, 10);
						strcpy(flow[f].conflictmap[nodestack[i]].reg, "$s");
						strcat(flow[f].conflictmap[nodestack[i]].reg, a);
						break;
					}
				}
			}
			else {
				strcpy(flow[f].conflictmap[nodestack[i]].reg, None);
			}
		}
		for (int i = 0; i < flow[f].conflictnum; i++) {
			if (flow[f].conflictmap[i].isAllocated != 1) {
				strcpy(flow[f].conflictmap[i].reg, None);
			}
		}
	}
}

void global_reg_allocate()
{
	int i;
	for (i = 0; i < flow_index; i++) {
		getFdef_use(i);
		getinout(i);
		genConflictmap(i);
		allocate_reg(i);
		int k = 0;
	}
}

void optimizeMidcode()
{
	devideBlock();
	allocate_temp_reg();
	genFlow();
	get_nextblock();
	global_reg_allocate();
}


