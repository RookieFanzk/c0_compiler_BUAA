#include "tomips.h"
#include "symbolTable.h"
#include "optimize.h"

STRINGANDLABLE stringAndlable[MAX_STRING_NUM];
int stringnum = 0;

TEMPVAR tempvar[MAX_TEMP_VAR];			//临时变量
int tempvarcount = 0;

GLOBALCONST globalpara[MAX_GLOBAL];		//全局的常量、变量
int globalcount = 0;

LOCALVAR localvar[MAX_VAR];				//局部变量
int localvarcount = 0;
int localoffset = 0;
int localparaoffset = 0;

int funcparanum = 0;
int funccallpara = 0;
int funcdefpara = 0;

unsigned int returnvaluespace = 0x10015000;
unsigned int tempvaraddrBegin = 0x10015004;				//临时变量存放区起始地址
int tempoffset = 0;										//临时变量相对于当前fp的偏移

int beginnum;

void insertlocalvar(char *varname)
{
	strcpy(localvar[localvarcount].varname, varname);
	localvar[localvarcount].addr = localoffset;
	localoffset = localoffset + 4;
	localvarcount++;
}

int getlocalvarAddr(char *varname)								//得到局部变量地址
{
	for (int i = localvarcount - 1; i >= 0; i--) {
		if (strcmp(localvar[i].varname, varname) == 0) {
			return localvar[i].addr;
		}
	}
	return -1;
}

void inserttempvar(char *varname)								//临时变量插入临时变量区
{
	strcpy(tempvar[tempvarcount].tempname, varname);
	tempvar[tempvarcount].addr = tempoffset;
	tempoffset = tempoffset + 4;
	tempvarcount++;
}

int gettempaddr(char *varname)									//临时变量寻址
{
	for (int i = tempvarcount - 1; i >= 0; i--) {
		if (strcmp(tempvar[i].tempname, varname) == 0) {
			return tempvar[i].addr;
		}
		if (strcmp(tempvar[i].tempname, "VAR_0") == 0) {
			return -1;
		}
	}
	return -1;
}

char *gettempreg(int blocknum, char *varname)
{
	int i;
	for (i = 0; i < blockflow[blocknum].tempnum; i++) {
		if (strcmp(blockflow[blocknum].tempvarReg[i].tempvar, varname) == 0) {
			return blockflow[blocknum].tempvarReg[i].reg;
		}
	}
	return None;
}

char *getvarreg(int f, char *varname)
{
	int i;
	for (i = 0; i < flow[f].conflictnum; i++) {
		if (strcmp(flow[f].conflictmap[i].name, varname) == 0) {
			return flow[f].conflictmap[i].reg;
		}
	}
	return None;
}

char *int_to_str(int i)
{
	char inttostr[20];
	itoa(i, inttostr, 10);
	return inttostr;
}

void backwrite(int blocknum, char *varname, FILE *fp)
{
	int i;
	for (i = 0; i < blockflow[blocknum].tempnum; i++) {
		if (strcmp(blockflow[blocknum].tempvarReg[i].tempvar, varname) == 0) {
			if (i > 9) {
				for (int j = i - 1; j >= 0; j--) {
					if (strcmp(blockflow[blocknum].tempvarReg[j].reg, blockflow[blocknum].tempvarReg[i].reg) == 0) {
						int addr = gettempaddr(blockflow[blocknum].tempvarReg[j].tempvar);
						fputs("sw\t", fp);
						fputs(blockflow[blocknum].tempvarReg[i].reg, fp);
						fputs(", ", fp);
						fputs(int_to_str(addr), fp);
						fputs("($fp)\n", fp);
						break;
					}
				}
			}
		}
	}
}

void genMips()
{
	genData();
	genText();
}

void genData()
{
	FILE *fp = fopen(mipsCodefileName, "w+");
	fputs(".data", fp);
	fprintf(fp, "\n");
	int i;
	int arrayspace = 0;
	for (i = 0; i < codenum_count; i++) {
		if (strcmp(midcode[i].op, Func) == 0) {
			beginnum = i;
			break;
		}
		if (strcmp(midcode[i].op, Var) == 0) {						//全局var定义
			fprintf(fp, "\t");
			fputs(midcode[i].rst, fp);
			fprintf(fp, ":.space 4");
			fprintf(fp, "\n");
			strcpy(globalpara[globalcount].globalname, midcode[i].rst);
			strcpy(globalpara[globalcount].value, Space);
			globalcount++;
		}
	}
	for (i = 0; i < codenum_count; i++) {
		if (strcmp(midcode[i].op, Func) == 0) {
			break;
		}
		if (strcmp(midcode[i].op, Array) == 0) {					//全局数组元素，申请.space
			arrayspace = atoi(midcode[i].var2) * 4;
			fprintf(fp, "\t");
			fputs(midcode[i].rst, fp); fprintf(fp, ":.space ");
			char int_to_str[100];
			itoa(arrayspace, int_to_str, 10);
			fputs(int_to_str, fp);
			fprintf(fp, "\n");
			strcpy(globalpara[globalcount].globalname, midcode[i].rst);
			strcpy(globalpara[globalcount].value, midcode[i].var2);
			globalcount++;
		}
	}
	for (i = 0; i < codenum_count; i++) {							//字符串，.ascii
		if (strcmp(midcode[i].op, Printf) == 0) {
			char strlable[10];
			if (strcmp(midcode[i].var1, None) != 0) {
				strcpy(strlable, genStrlable());
				fprintf(fp, "\t");
				fputs(strlable, fp);
				fprintf(fp, ":.asciiz ");
				fprintf(fp, "\"");
				fputs(midcode[i].var1, fp);
				fprintf(fp, "\"\n");
				strcpy(stringAndlable[stringnum].string, midcode[i].var1);
				strcpy(stringAndlable[stringnum].lable, strlable);
				stringnum++;
			}
		}
	}
	fclose(fp);
}

void genText()
{
	FILE *fp = fopen(mipsCodefileName, "a+");

	fputs(".text\n", fp);
	fprintf(fp, "\n");
	fputs("j\tmain\n", fp);
	beginnum = (beginnum == 0) ? 0 : 1;
	for (int i = beginnum; i < flow_index; i++) {
		for (int j = 0; j < flow[i].blocknum; j++) {
			for (int k = 0; k < blockflow[flow[i].block[j]].codenum; k++) {
				OpMidCode opitem = blockflow[flow[i].block[j]].blockcode[k];
				if (strcmp(opitem.op, Var) == 0) {
					if (strcmp(opitem.var1, Int) == 0 || strcmp(opitem.var1, Char) == 0) {
						strcpy(localvar[localvarcount].varname, opitem.rst);
						localvar[localvarcount].addr = localoffset;
						fputs("addiu\t$sp, $sp, -4\n", fp);
						localoffset = localoffset + 4;
						localvarcount++;
					}
				}
				else if (strcmp(opitem.op, Array) == 0) {
					if (strcmp(opitem.var1, Int) == 0 || strcmp(opitem.var1, Char) == 0) {
						int arrayspace = atoi(opitem.var2) * 4;
						strcpy(localvar[localvarcount].varname, opitem.rst);
						localvar[localvarcount].addr = localoffset;
						fputs("addiu\t$sp, $sp, -", fp);
						fputs(int_to_str(arrayspace), fp);
						fprintf(fp, "\n");
						localoffset = localoffset + arrayspace;
						localvarcount++;
					}
				}
				else if (strcmp(opitem.op, Lab) == 0) {
					fputs(opitem.rst, fp);
					fputs(":\n", fp);
				}
				else if (strcmp(opitem.op, Func) == 0) {
					funcparanum = 0;
					fputs(opitem.rst, fp);
					fputs(":\n", fp);
					for (int m = k + 1; m < blockflow[flow[i].block[j]].codenum; m++) {
						if (strcmp(blockflow[flow[i].block[j]].blockcode[m].op, Para) == 0) {
							funcparanum++;
						}
						else {
							break;
						}
					}
					for (int m = k + 1; m < blockflow[flow[i].block[j]].codenum; m++) {
						if (strcmp(blockflow[flow[i].block[j]].blockcode[m].op, Para) == 0) {
							continue;
						}
						else if (strcmp(blockflow[flow[i].block[j]].blockcode[m].op, Var) == 0) {
							localparaoffset = localparaoffset + 4;
						}
						else if (strcmp(blockflow[flow[i].block[j]].blockcode[m].op, Array) == 0) {
							int arrayspace = atoi(blockflow[flow[i].block[j]].blockcode[m].var2) * 4;;
							localparaoffset = localparaoffset + arrayspace;
						}
						else if (strcmp(blockflow[flow[i].block[j]].blockcode[m].op, Const) == 0) {
							continue;
						}
						else {
							break;
						}
					}
					if (strcmp(opitem.rst, "main") != 0) {
						fputs("addiu\t$sp, $sp, -4\n", fp);
						fputs("sw\t$ra, 0($sp)\n", fp);
						if (funcparanum != 0) {
							fputs("subiu\t$sp, $sp, ", fp);
							fputs(int_to_str(funcparanum * 4), fp);
							fputs("\n", fp);
						}
					}
					else {
						fputs("li\t$fp, ", fp);
						fputs(int_to_str(tempvaraddrBegin), fp);
						fputs("\n", fp);
					}
				}
				else if (strcmp(opitem.op, Funcend) == 0) {											
					localvarcount = 0;
					localoffset = 0;
					localparaoffset = 0;

					tempvarcount = 0;
					tempoffset = 0;
				}
				else if (strcmp(opitem.op, Para) == 0) {
					switch (funcdefpara) {
						case 0:
							if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
								fputs("move\t", fp);
								fputs(getvarreg(i, opitem.rst), fp);
								fputs(", $a0\n", fp);
							}
							else {
								fputs("sw\t$a0, 0($sp)\n", fp);
							}
							break;
						case 1:
							if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
								fputs("move\t", fp);
								fputs(getvarreg(i, opitem.rst), fp);
								fputs(", $a1\n", fp);
							}
							else {
								fputs("sw\t$a1, 4($sp)\n", fp);
							}
							break;
						case 2:
							if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
								fputs("move\t", fp);
								fputs(getvarreg(i, opitem.rst), fp);
								fputs(", $a2\n", fp);
							}
							else {
								fputs("sw\t$a2, 8($sp)\n", fp);
							}
							break;
						case 3:
							if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
								fputs("move\t", fp);
								fputs(getvarreg(i, opitem.rst), fp);
								fputs(", $a3\n", fp);
							}
							else {
								fputs("sw\t$a3, 12($sp)\n", fp);
							}
							break;
						default:
							break;
					}

					strcpy(localvar[localvarcount].varname, opitem.rst);
					localvar[localvarcount].addr = localparaoffset;
					if ((funcdefpara >= 4) && (strcmp(getvarreg(i, opitem.rst), None) != 0)) {
						fputs("lw\t", fp);
						fputs(getvarreg(i, opitem.rst), fp);
						fputs(", ", fp);
						fputs(int_to_str(localparaoffset), fp);
						fputs("($sp)\n", fp);
					}
					localparaoffset = localparaoffset + 4;
					localvarcount++;
					funcdefpara++;
					if (strcmp(blockflow[flow[i].block[j]].blockcode[k + 1].op, Para) != 0) {
						funcdefpara = 0;
					}
				}
				else if (strcmp(opitem.op, ParaCall) == 0) {
					int addr, flag = 0;
					if (opitem.rst[0] == 'V' && opitem.rst[1] == 'A' && opitem.rst[2] == 'R' && opitem.rst[3] == '_') {
						addr = gettempaddr(opitem.rst);
						flag = 1;	//临时变量标志
					}
					else {
						addr = getlocalvarAddr(opitem.rst);
					}
					if (funccallpara < 4) {
						if (flag == 1) {
							fputs("move\t$a", fp);
							fputs(int_to_str(funccallpara), fp);
							fputs(", ", fp);
							fputs(gettempreg(flow[i].block[j], opitem.rst), fp);
							fputs("\n", fp);
						}
						else {
							if (addr == -1) {
								if (Allisnum(opitem.rst) || opitem.rst[0] == '-') {
									fputs("li\t$a", fp);
									fputs(int_to_str(funccallpara), fp);
									fputs(", ", fp);
									fputs(opitem.rst, fp);
									fputs("\n", fp);
								}
								else {
									fputs("la\t$v0, ", fp);
									fputs(opitem.rst, fp);
									fputs("\n", fp);
									fputs("lw\t$a", fp);
									fputs(int_to_str(funccallpara), fp);
									fputs(", 0($v0)", fp);
								}
							}
							else {
								if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
									fputs("move\t$a", fp);
									fputs(int_to_str(funccallpara), fp);
									fputs(", ", fp);
									fputs(getvarreg(i, opitem.rst), fp);
									fputs("\n", fp);
								}
								else {
									fputs("lw\t$a", fp);
									fputs(int_to_str(funccallpara), fp);
									fputs(", ", fp);
									fputs(int_to_str(addr), fp);
									fputs("($sp)\n", fp);
								}
							}
						}
					}
					else {
						int getparanum = 0;
						for (int m = k + 1; m < blockflow[flow[i].block[j]].codenum; m++) {
							if (strcmp(blockflow[flow[i].block[j]].blockcode[m].op, ParaCall) == 0) {
								continue;
							}
							if (strcmp(blockflow[flow[i].block[j]].blockcode[m].op, Call) == 0) {
								for (int n = 0; n < symTable.index; n++) {
									if (strcmp(symTable.symolArray[n].idname, blockflow[flow[i].block[j]].blockcode[m].var1) == 0) {
										if (symTable.symolArray[n].symbolType == Function) {
											getparanum = symTable.symolArray[n].paraNum;
											break;
										}
									}
								}
							}
							break;
						}
						if (flag == 1) {
							fputs("sw\t", fp);
							fputs(gettempreg(flow[i].block[j], opitem.rst), fp);
							fputs(", -", fp);
							fputs(int_to_str((getparanum - funccallpara) * 4 + 8), fp);
							fputs("($sp)\n", fp);
						}
						else {
							if (addr == -1) {
								if (Allisnum(opitem.rst) || opitem.rst[0] == '-') {
									fputs("li\t$v0, ", fp);
									fputs(opitem.rst, fp);
									fputs("\n", fp);
								}
								else {
									fputs("la\t$v0, ", fp);
									fputs(opitem.rst, fp);
									fputs("\n", fp);
									fputs("lw\t$v0, 0($v0)\n", fp);
								}
								fputs("sw\t$v0, -", fp);
								fputs(int_to_str((getparanum - funccallpara) * 4 + 8), fp);
								fputs("($sp)\n", fp);
							}
							else {
								if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
									fputs("sw\t", fp);
									fputs(getvarreg(i, opitem.rst), fp);
									fputs(", -", fp);
									fputs(int_to_str((getparanum - funccallpara) * 4 + 8), fp);
									fputs("($sp)\n", fp);
								}
								else {
									fputs("lw\t$v0, ", fp);
									fputs(int_to_str(addr), fp);
									fputs("($sp)\n", fp);
									fputs("sw\t$v0, -", fp);
									fputs(int_to_str((getparanum - funccallpara) * 4 + 8), fp);
									fputs("($sp)\n", fp);
								}
							}
						}
					}
					funccallpara++;
				}
				else if (strcmp(opitem.op, Call) == 0) {
					char tempvar[20][MAX_WORD_CHAR];
					int tempindex = 0;
					int becallvar = 0;
					for (int m = 0; m < flow_index; m++) {
						if (strcmp(blockflow[flow[m].block[0]].blockcode[0].rst, opitem.var1) == 0) {
							becallvar = flow[m].conflictnum;
						}
					}
					for (int m = 0; m < flow[i].conflictnum; m++) {
						if ((flow[i].conflictmap[m].reg[2] - '0') < becallvar) {
							fputs("sw\t", fp);
							fputs(flow[i].conflictmap[m].reg, fp);
							fputs(", ", fp);
							fputs(int_to_str(getlocalvarAddr(flow[i].conflictmap[m].name)), fp);
							fputs("($sp)\n", fp);
						}
					}
		
					for (int m = k - 1; m >= 0; m--) {
						int flag = 0;
						if (blockflow[flow[i].block[j]].blockcode[m].rst[0] == 'V' && blockflow[flow[i].block[j]].blockcode[m].rst[1] == 'A' && blockflow[flow[i].block[j]].blockcode[m].rst[2] == 'R' && blockflow[flow[i].block[j]].blockcode[m].rst[3] == '_' && strcmp(blockflow[flow[i].block[j]].blockcode[m].op,ParaCall) != 0) {
							for (int n = m; n < k; n++) {
								if ((strcmp(blockflow[flow[i].block[j]].blockcode[n].var1, blockflow[flow[i].block[j]].blockcode[m].rst) == 0 || strcmp(blockflow[flow[i].block[j]].blockcode[n].var2, blockflow[flow[i].block[j]].blockcode[m].rst) == 0) || (strcmp(blockflow[flow[i].block[j]].blockcode[n].op, ParaCall) == 0 && strcmp(blockflow[flow[i].block[j]].blockcode[n].rst, blockflow[flow[i].block[j]].blockcode[m].rst) == 0)) {
									flag = 1;
									break;
								}
							}
							if (flag == 0) {
								fputs("sw\t", fp);
								fputs(gettempreg(flow[i].block[j], blockflow[flow[i].block[j]].blockcode[m].rst), fp);
								fputs(", ", fp);
								fputs(int_to_str(gettempaddr(blockflow[flow[i].block[j]].blockcode[m].rst)), fp);
								fputs("($fp)\n", fp);
								strcpy(tempvar[tempindex++], blockflow[flow[i].block[j]].blockcode[m].rst);
							}
						}
					}

					fputs("addiu\t$sp, $sp, -4\n", fp);
					fputs("sw\t$fp, 0($sp)\n", fp);
					fputs("addiu\t$fp, $fp, ", fp);
					fputs(int_to_str(tempoffset), fp);
					fputs("\n", fp);

					fputs("jal\t", fp);
					fputs(opitem.var1, fp);
					fputs("\n", fp);

					for (int m = 0; m < flow[i].conflictnum; m++) {
						if ((flow[i].conflictmap[m].reg[2] - '0') < becallvar) {
							fputs("lw\t", fp);
							fputs(flow[i].conflictmap[m].reg, fp);
							fputs(", ", fp);
							fputs(int_to_str(getlocalvarAddr(flow[i].conflictmap[m].name)), fp);
							fputs("($sp)\n", fp);
						}
					}

					if (tempindex != 0) {
						for (int m = 0; m < tempindex; m++) {
							fputs("lw\t", fp);
							fputs(gettempreg(flow[i].block[j], tempvar[m]), fp);
							fputs(", ", fp);
							fputs(int_to_str(gettempaddr(tempvar[m])), fp);
							fputs("($fp)\n", fp);
						}
					}
					
					if (strcmp(opitem.rst, Space) != 0) {
						inserttempvar(opitem.rst);
						backwrite(flow[i].block[j], opitem.rst, fp);
						fputs("lw\t", fp);
						fputs(gettempreg(flow[i].block[j], opitem.rst), fp);
						fputs(", ", fp);
						fputs(int_to_str(returnvaluespace), fp);
						fputs("\n", fp);
					}
					funccallpara = 0;
				}
				else if (strcmp(opitem.op, Assign) == 0) {
					int addr_a, addr_b, addr_i;
					if (strcmp(opitem.var2, Space) == 0) {	// = b   a
						if (Allisnum(opitem.var1) || opitem.var1[0] == '-' || isChar(opitem.var1)) {	// b为立即数或字符
							if (opitem.rst[0] == 'V' && opitem.rst[1] == 'A' && opitem.rst[2] == 'R' && opitem.rst[3] == '_') {
								inserttempvar(opitem.rst);
								backwrite(flow[i].block[j], opitem.rst, fp);
								fputs("li\t", fp);
								fputs(gettempreg(flow[i].block[j], opitem.rst), fp);
								fputs(", ", fp);
								if (isChar(opitem.var1)) {
									fputs("'", fp);
									fputs(opitem.var1, fp);
									fputs("'\n", fp);
								}
								else {
									fputs(opitem.var1, fp);
									fputs("\n", fp);
								}
							}
							else {
								addr_a = getlocalvarAddr(opitem.rst);
								if (addr_a == -1) {
									fputs("li\t$v0, ", fp);
									if (isChar(opitem.var1)) {
										fputs("'", fp);
										fputs(opitem.var1, fp);
										fputs("'\n", fp);
									}
									else {
										fputs(opitem.var1, fp);
										fputs("\n", fp);
									}
									fputs("la\t$v1, ", fp);
									fputs(opitem.rst, fp);
									fputs("\n", fp);
									fputs("sw\t$v0, 0($v1)\n", fp);
								}
								else {
									if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
										fputs("li\t", fp);
										fputs(getvarreg(i, opitem.rst), fp);
										fputs(", ", fp);
										if (isChar(opitem.var1)) {
											fputs("'", fp);
											fputs(opitem.var1, fp);
											fputs("'\n", fp);
										}
										else {
											fputs(opitem.var1, fp);
											fputs("\n", fp);
										}
									}
									else {
										fputs("li\t$v0, ", fp);
										if (isChar(opitem.var1)) {
											fputs("'", fp);
											fputs(opitem.var1, fp);
											fputs("'\n", fp);
										}
										else {
											fputs(opitem.var1, fp);
											fputs("\n", fp);
										}
										fputs("sw\t$v0, ", fp);
										fputs(int_to_str(addr_a), fp);
										fputs("($sp)\n", fp);
									}
								}
							}
						}
						else if (opitem.var1[0] == 'V' && opitem.var1[1] == 'A' && opitem.var1[2] == 'R' && opitem.var1[3] == '_') {
							addr_a = getlocalvarAddr(opitem.rst);
							if (addr_a == -1) {
								fputs("la\t$v0, ", fp);
								fputs(opitem.rst, fp);
								fputs("\n", fp);
								fputs("sw\t", fp);
								fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
								fputs(", 0($v0)\n", fp);
							}
							else {
								if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
									fputs("move\t", fp);
									fputs(getvarreg(i, opitem.rst), fp);
									fputs(", ", fp);
									fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
									fputs("\n", fp);
								}
								else {
									fputs("sw\t", fp);
									fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
									fputs(", ", fp);
									fputs(int_to_str(addr_a), fp);
									fputs("($sp)\n", fp);
								}
							}
						}
						else {
							addr_b = getlocalvarAddr(opitem.var1);
							if (addr_b == -1) {
								fputs("la\t$v0, ", fp);
								fputs(opitem.var1, fp);
								fputs("\n", fp);
								fputs("lw\t$v0, 0($v0)\n", fp);
							}
							else {
								if (strcmp(getvarreg(i, opitem.var1), None) != 0) {
									fputs("move\t$v0, ", fp);
									fputs(getvarreg(i, opitem.var1), fp);
									fputs("\n", fp);
								}
								else {
									fputs("lw\t$v0, ", fp);
									fputs(int_to_str(addr_b), fp);
									fputs("($sp)\n", fp);
								}
							}
							if (opitem.rst[0] == 'V' && opitem.rst[1] == 'A' && opitem.rst[2] == 'R' && opitem.rst[3] == '_') {
								inserttempvar(opitem.rst);
								backwrite(flow[i].block[j], opitem.rst, fp);
								fputs("move\t", fp);
								fputs(gettempreg(flow[i].block[j], opitem.rst), fp);
								fputs(", $v0\n", fp);
							}
							else {
								addr_a = getlocalvarAddr(opitem.rst);
								if (addr_a == -1) {
									fputs("la\t$v1, ", fp);
									fputs(opitem.rst, fp);
									fputs("\n", fp);
									fputs("sw\t$v0, 0($v1)\n", fp);
								}
								else {
									if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
										fputs("move\t", fp);
										fputs(getvarreg(i, opitem.rst), fp);
										fputs(", $v0\n", fp);
									}
									else {
										fputs("sw\t$v0, ", fp);
										fputs(int_to_str(addr_a), fp);
										fputs("($sp)\n", fp);
									}
								}
							}
						}
					}
					else {			// = b i a  a = b[i]
						if (Allisnum(opitem.var2)) {
							fputs("li\t$v0, ", fp);
							fputs(int_to_str(atoi(opitem.var2) * 4), fp);
							fputs("\n", fp);
						}
						else if(opitem.var2[0] == 'V' && opitem.var2[1] == 'A' && opitem.var2[2] == 'R' && opitem.var2[3] == '_'){
							fputs("li\t$v0, 4\n", fp);
							fputs("mult\t$v0, ",fp);
							fputs(gettempreg(flow[i].block[j], opitem.var2), fp);
							fputs("\n", fp);
							fputs("mflo\t$v0\n", fp);
						}
						else {
							addr_i = getlocalvarAddr(opitem.var2);
							if (addr_i == -1) {
								fputs("la\t$v0, ", fp);
								fputs(opitem.var2, fp);
								fputs("\n", fp);
								fputs("lw\t$v0, 0($v0)\n", fp);
								fputs("li\t$v1, 4\n", fp);
								fputs("mult\t$v0, $v1\n", fp);
								fputs("mflo\t$v0\n", fp);
							}
							else {
								if (strcmp(getvarreg(i, opitem.var2), None) != 0) {
									fputs("li\t$v0, 4\n", fp);
									fputs("mult\t$v0, ", fp);
									fputs(getvarreg(i, opitem.var2), fp);
									fputs("\n", fp);
									fputs("mflo\t$v0\n", fp);
								}
								else {
									fputs("lw\t$v0, ", fp);
									fputs(int_to_str(addr_i), fp);
									fputs("($sp)\n", fp);
									fputs("li\t$v1, 4\n", fp);
									fputs("mult\t$v0, $v1\n", fp);
									fputs("mflo\t$v0\n", fp);
								}
							}
						}
						addr_b = getlocalvarAddr(opitem.var1);
						if (addr_b == -1) {
							fputs("la\t$v1, ", fp);
							fputs(opitem.var1, fp);
							fputs("\n", fp);
							fputs("addu\t$v0, $v0, $v1\n", fp);
							fputs("lw\t$v0, 0($v0)\n", fp);
						}
						else {
							fputs("li\t$v1, ", fp);
							fputs(int_to_str(addr_b), fp);
							fputs("\n", fp);
							fputs("addu\t$v1, $v1, $v0\n", fp);
							fputs("addu\t$v1, $v1, $sp\n", fp);
							fputs("lw\t$v0, 0($v1)\n", fp);
						}
						if (opitem.rst[0] == 'V' && opitem.rst[1] == 'A' && opitem.rst[2] == 'R' && opitem.rst[3] == '_') {
							inserttempvar(opitem.rst);
							backwrite(flow[i].block[j], opitem.rst, fp);
							fputs("move\t", fp);
							fputs(gettempreg(flow[i].block[j], opitem.rst), fp);
							fputs(", $v0\n", fp);
						}
						else {
							addr_a = getlocalvarAddr(opitem.rst);
							if (addr_a == -1) {
								fputs("la\t$v1, ", fp);
								fputs(opitem.rst, fp);
								fputs("\n", fp);
								fputs("sw\t$v0, 0($v1)\n", fp);
							}
							else {
								if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
									fputs("move\t", fp);
									fputs(getvarreg(i, opitem.rst), fp);
									fputs(", $v0\n", fp);
								}
								else {
									fputs("sw\t$v0, ", fp);
									fputs(int_to_str(addr_a), fp);
									fputs("($sp)\n", fp);
								}
							}
						}
					}
				}
				else if (strcmp(opitem.op, Assarr) == 0) {		// []= b i a   a[i] = b
					int addr_a, addr_b, addr_i;
					if (Allisnum(opitem.var1) || opitem.var1[0] == '-') {
						fputs("li\t$v0, ", fp);
						fputs(opitem.var1, fp);
						fputs("\n", fp);
					}
					else if (isChar(opitem.var1)) {
						fputs("li\t$v0, '", fp);
						fputs(opitem.var1, fp);
						fprintf(fp, "'\n");
					}
					else if(opitem.var1[0] == 'V' && opitem.var1[1] == 'A' && opitem.var1[2] == 'R' && opitem.var1[3] == '_'){
						fputs("move\t$v0, ", fp);
						fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
						fputs("\n", fp);
					}
					else {
						addr_b = getlocalvarAddr(opitem.var1);
						if (addr_b == -1) {
							fputs("la\t$v0, ", fp);
							fputs(opitem.var1, fp);
							fputs("\n", fp);
							fputs("lw\t$v0, 0($v0)\n", fp);
						}
						else {
							if (strcmp(getvarreg(i, opitem.var1), None) != 0) {
								fputs("move\t$v0, ", fp);
								fputs(getvarreg(i, opitem.var1), fp);
								fputs("\n", fp);
							}
							else {
								fputs("lw\t$v0, ", fp);
								fputs(int_to_str(addr_b), fp);
								fputs("($sp)\n", fp);
							}
						}
					}
					if (Allisnum(opitem.var2)) {
						fputs("li\t$v1, ", fp);
						fputs(int_to_str(atoi(opitem.var2) * 4), fp);
						fputs("\n", fp);
					}
					else if(opitem.var2[0] == 'V' && opitem.var2[1] == 'A' && opitem.var2[2] == 'R' && opitem.var2[3] == '_'){
						fputs("li\t$v1, 4\n", fp);
						fputs("mult\t$v1, ", fp);
						fputs(gettempreg(flow[i].block[j], opitem.var2), fp);
						fputs("\n", fp);
						fputs("mflo\t$v1\n", fp);
					}
					else {
						addr_i = getlocalvarAddr(opitem.var2);
						if (addr_i == -1) {
							fputs("la\t$v1, ", fp);
							fputs(opitem.var2, fp);
							fputs("\n", fp);
							fputs("lw\t$v1, 0($v1)\n", fp);
							fputs("sll\t$v1, $v1, 2\n", fp);
						}
						else {
							if (strcmp(getvarreg(i, opitem.var2), None) != 0) {
								fputs("move\t$v1, ", fp);
								fputs(getvarreg(i, opitem.var2), fp);
								fputs("\n", fp);
								fputs("sll\t$v1, $v1, 2\n", fp);
							}
							else {
								fputs("lw\t$v1, ", fp);
								fputs(int_to_str(addr_i), fp);
								fputs("($sp)\n", fp);
								fputs("sll\t$v1, $v1, 2\n", fp);
							}
						}
					}
					addr_a = getlocalvarAddr(opitem.rst);
					if (addr_a == -1) {
						fputs("la\t$gp, ", fp);
						fputs(opitem.rst, fp);
						fputs("\n", fp);
						fputs("addu\t$v1, $v1, $gp\n", fp);
						fputs("sw\t$v0, 0($v1)\n", fp);
					}
					else {
						fputs("li\t$gp, ", fp);
						fputs(int_to_str(addr_a), fp);
						fputs("\n", fp);
						fputs("addu\t$v1, $v1, $gp\n", fp);
						fputs("addu\t$v1, $v1, $sp\n", fp);
						fputs("sw\t$v0, 0($v1)\n", fp);
					}
				}
				else if (strcmp(opitem.op, Jmp) == 0) {
					fputs("j\t", fp);
					fputs(opitem.rst, fp);
					fputs("\n", fp);
				}
				else if ((strcmp(opitem.op, Beq) == 0) || (strcmp(opitem.op, Bgez) == 0) || (strcmp(opitem.op, Bgtz) == 0) || (strcmp(opitem.op, Bltz) == 0) || (strcmp(opitem.op, Blez) == 0) || (strcmp(opitem.op, Bne) == 0)) {
					if (opitem.var1[0] == 'V' && opitem.var1[1] == 'A' && opitem.var1[2] == 'R' && opitem.var1[3] == '_') {
						if (strcmp(opitem.op, Beq) == 0) {
							fputs("beq\t", fp);
							fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
							fputs(", 0, ", fp);
							fputs(opitem.rst, fp);
							fputs("\n", fp);
						}
						else if (strcmp(opitem.op, Bne) == 0) {
							fputs("bne\t", fp);
							fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
							fputs(", 0, ", fp);
							fputs(opitem.rst, fp);
							fputs("\n", fp);
						}
						else if (strcmp(opitem.op, Bgez) == 0) {
							fputs("bgez\t", fp);
							fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
							fputs(", ", fp);
							fputs(opitem.rst, fp);
							fputs("\n", fp);
						}
						else if (strcmp(opitem.op, Bgtz) == 0) {
							fputs("bgtz\t", fp);
							fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
							fputs(", ", fp);
							fputs(opitem.rst, fp);
							fputs("\n", fp);
						}
						else if (strcmp(opitem.op, Blez) == 0) {
							fputs("blez\t", fp);
							fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
							fputs(", ", fp);
							fputs(opitem.rst, fp);
							fputs("\n", fp);
						}
						else if (strcmp(opitem.op, Bltz) == 0) {
							fputs("bltz\t", fp);
							fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
							fputs(", ", fp);
							fputs(opitem.rst, fp);
							fputs("\n", fp);
						}
					}
					else if(Allisnum(opitem.var1)){
						fputs("li\t$v0, ", fp);
						fputs(opitem.var1, fp);
						fprintf(fp, "\n");
						if (strcmp(opitem.op, Beq) == 0) {
							fputs("beq\t$v0, $0, ", fp);
							fputs(opitem.rst, fp);
							fprintf(fp, "\n");
						}
					}
					else {
						int addr = getlocalvarAddr(opitem.var1);
						if (addr == -1) {
							
						}
					}
				}
				else if ((strcmp(opitem.op, Add) == 0) || (strcmp(opitem.op, Sub) == 0) || (strcmp(opitem.op, Mul) == 0) || (strcmp(opitem.op, Div) == 0)) {
					int addr1, addr2, addr3;
					int rstindex = -1;
					for (int m = 0; m < blockflow[flow[i].block[j]].tempnum; m++) {
						if (strcmp(blockflow[flow[i].block[j]].tempvarReg[m].tempvar, opitem.rst) == 0) {
							rstindex = m;
						}
					}
					if (Allisnum(opitem.var1) || opitem.var1[0] == '-') {
						fputs("li\t$v0, ", fp);
						fputs(opitem.var1, fp);
						fputs("\n", fp);
					}
					else if(opitem.var1[0] == 'V' && opitem.var1[1] == 'A' && opitem.var1[2] == 'R' && opitem.var1[3] == '_'){
						for (int m = rstindex - 1; m >= 0; m--) {
							if (strcmp(gettempreg(flow[i].block[j], opitem.var1), blockflow[flow[i].block[j]].tempvarReg[m].reg) == 0) {
								if (strcmp(opitem.var1, blockflow[flow[i].block[j]].tempvarReg[m].tempvar) != 0) {
									fputs("lw\t$v0, ", fp);
									fputs(int_to_str(gettempaddr(opitem.var1)), fp);
									fputs("($fp)\n", fp);
								}
								else {
									fputs("move\t$v0, ", fp);
									fputs(gettempreg(flow[i].block[j], opitem.var1), fp);
									fputs("\n", fp);
								}
								break;
							}
						}
					}
					else {
						addr1 = getlocalvarAddr(opitem.var1);
						if (addr1 == -1) {
							fputs("la\t$v0, ", fp);
							fputs(opitem.var1, fp);
							fputs("\n", fp);
							fputs("lw\t$v0, 0($v0)\n", fp);
						}
						else {
							if (strcmp(getvarreg(i, opitem.var1), None) != 0) {
								fputs("move\t$v0, ", fp);
								fputs(getvarreg(i, opitem.var1), fp);
								fputs("\n", fp);
							}
							else {
								fputs("lw\t$v0, ", fp);
								fputs(int_to_str(addr1), fp);
								fputs("($sp)\n", fp);
							}
						}
					}
					if (Allisnum(opitem.var2) || opitem.var2[0] == '-') {
						fputs("li\t$v1, ", fp);
						fputs(opitem.var2, fp);
						fputs("\n", fp);
					}
					else if (opitem.var2[0] == 'V' && opitem.var2[1] == 'A' && opitem.var2[2] == 'R' && opitem.var2[3] == '_') {
						for (int m = rstindex - 1; m >= 0; m--) {
							if (strcmp(gettempreg(flow[i].block[j], opitem.var2), blockflow[flow[i].block[j]].tempvarReg[m].reg) == 0) {
								if (strcmp(opitem.var2, blockflow[flow[i].block[j]].tempvarReg[m].tempvar) != 0) {
									fputs("lw\t$v1, ", fp);
									fputs(int_to_str(gettempaddr(opitem.var2)), fp);
									fputs("($fp)\n", fp);
								}
								else {
									fputs("move\t$v1, ", fp);
									fputs(gettempreg(flow[i].block[j], opitem.var2), fp);
									fputs("\n", fp);
								}
								break;
							}
						}
					}
					else {
						addr2 = getlocalvarAddr(opitem.var2);
						if (addr2 == -1) {
							fputs("la\t$v1, ", fp);
							fputs(opitem.var2, fp);
							fputs("\n", fp);
							fputs("lw\t$v1, 0($v1)\n", fp);
						}
						else {
							if (strcmp(getvarreg(i, opitem.var2), None) != 0) {
								fputs("move\t$v1, ", fp);
								fputs(getvarreg(i, opitem.var2), fp);
								fputs("\n", fp);
							}
							else {
								fputs("lw\t$v1, ", fp);
								fputs(int_to_str(addr2), fp);
								fputs("($sp)\n", fp);
							}
						}
					}
					if (strcmp(opitem.op, Add) == 0) {
						fputs("addu\t$v0, $v0, $v1\n", fp);
					}
					else if (strcmp(opitem.op, Sub) == 0) {
						fputs("subu\t$v0, $v0, $v1\n", fp);
					}
					else if (strcmp(opitem.op, Mul) == 0) {
						fputs("mult\t$v0, $v1\n", fp);
						fputs("mflo\t$v0\n", fp);
					}
					else if (strcmp(opitem.op, Div) == 0) {
						fputs("div\t$v0, $v1\n", fp);
						fputs("mflo\t$v0\n", fp);
					}
					inserttempvar(opitem.rst);
					backwrite(flow[i].block[j], opitem.rst, fp);
					fputs("move\t", fp);
					fputs(gettempreg(flow[i].block[j], opitem.rst), fp);
					fputs(", $v0\n",fp);
				}
				else if (strcmp(opitem.op, Scanf) == 0) {
					if (strcmp(opitem.var1, Int) == 0) {
						fputs("li\t$v0, 5\n", fp);
					}
					else if (strcmp(opitem.var1, Char) == 0) {
						fputs("li\t$v0, 12\n", fp);
					}
					fputs("syscall\n", fp);
					int addr = getlocalvarAddr(opitem.rst);
					if (addr == -1) {
						fputs("la\t$v0, ", fp);
						fputs(opitem.rst, fp);
						fputs("\n", fp);
						fputs("sw\t$v0, 0($v0)\n", fp);
					}
					else {
						if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
							fputs("move\t", fp);
							fputs(getvarreg(i, opitem.rst), fp);
							fputs(", $v0\n", fp);
						}
						else {
							fputs("sw\t$v0, ", fp);
							fputs(int_to_str(addr), fp);
							fputs("($sp)\n", fp);
						}
					}
				}
				else if (strcmp(opitem.op, Printf) == 0) {
					int addr2;
					char stringlable[10];
					if (strcmp(opitem.var1, None) != 0) {
						fputs("la\t$a0, ", fp);
						for (int i = 0; i < stringnum; i++) {
							if (strcmp(opitem.var1, stringAndlable[i].string) == 0) {
								strcpy(stringlable, stringAndlable[i].lable);
								break;
							}
						}
						fputs(stringlable, fp);
						fprintf(fp, "\n");
						fputs("li\t$v0, 4\n", fp);
						fputs("syscall\n", fp);
					}
					if (strcmp(opitem.var2, Space) != 0) {
						if (isChar(opitem.var2)) {									
							fputs("li\t$a0, ", fp);
							fputs(opitem.var2, fp);
							fprintf(fp, "\n");
							fputs("li\t$v0, 11\n", fp);
							fputs("syscall\n", fp);
						}
						else if (Allisnum(opitem.var2) || opitem.var2[0] == '-') {		
							fputs("li\t$a0, ", fp);
							fputs(opitem.var2, fp);
							fprintf(fp, "\n");
							if (strcmp(opitem.rst, "int") == 0) {
								fputs("li\t$v0, 1\n", fp);
								fputs("syscall\n", fp);
							}
							else {
								fputs("li\t$v0, 11\n", fp);
								fputs("syscall\n", fp);
							}
						}
						else if (opitem.var2[0] == 'V' && opitem.var2[1] == 'A' && opitem.var2[2] == 'R' && opitem.var2[3] == '_') {
							fputs("move\t$a0, ", fp);
							fputs(gettempreg(flow[i].block[j], opitem.var2), fp);
							fputs("\n", fp);
							if (strcmp(opitem.rst, "int") == 0) {
								fputs("li\t$v0, 1\n", fp);
								fputs("syscall\n", fp);
							}
							else {
								fputs("li\t$v0, 11\n", fp);
								fputs("syscall\n", fp);
							}
						}
						else {
							addr2 = getlocalvarAddr(opitem.var2);
							if (addr2 == -1) {
								fputs("la\t$v0, ", fp);
								fputs(opitem.var2, fp);
								fputs("\n", fp);
								fputs("lw\t$a0, 0($v0)\n", fp);
							}
							else {
								if (strcmp(getvarreg(i, opitem.var2), None) != 0) {
									fputs("move\t$a0, ", fp);
									fputs(getvarreg(i, opitem.var2), fp);
									fputs("\n", fp);
								}
								else {
									fputs("lw\t$a0, ", fp);
									fputs(int_to_str(addr2), fp);
									fputs("($sp)\n", fp);
								}
								if (strcmp(opitem.rst, "int") == 0) {
									fputs("li\t$v0, 1\n", fp);
									fputs("syscall\n", fp);
								}
								else {
									fputs("li\t$v0, 11\n", fp);
									fputs("syscall\n", fp);
								}
							}
						}
					}
				}
				else if (strcmp(opitem.op, Ret) == 0) {
					if (strcmp(opitem.rst, Space) != 0) {
						if (Allisnum(opitem.rst) || isChar(opitem.rst)) {
							fputs("li\t$v0, ", fp);
							fputs(opitem.rst, fp);
							fprintf(fp, "\n");
							fputs("sw\t$v0, ", fp);
							fputs(int_to_str(returnvaluespace), fp);
							fprintf(fp, "\n");
						}
						else {
							int addr;
							if (opitem.rst[0] == 'V' && opitem.rst[1] == 'A' && opitem.rst[2] == 'R' && opitem.rst[3] == '_') {
								fputs("sw\t", fp);
								fputs(gettempreg(flow[i].block[j], opitem.rst), fp);
								fputs(", ", fp);
								fputs(int_to_str(returnvaluespace), fp);
								fprintf(fp, "\n");
							}
							else {
								addr = getlocalvarAddr(opitem.rst);
								if (addr == -1) {
									fputs("la\t$v0, ", fp);
									fputs(opitem.rst, fp);
									fputs("\n", fp);
									fputs("lw\t$v0, 0($v0)\n", fp);
									fputs("sw\t$v0, ", fp);
									fputs(int_to_str(returnvaluespace), fp);
									fprintf(fp, "\n");
								}
								else {
									if (strcmp(getvarreg(i, opitem.rst), None) != 0) {
										fputs("sw\t", fp);
										fputs(getvarreg(i, opitem.rst), fp);
										fputs(", ", fp);
										fputs(int_to_str(returnvaluespace), fp);
										fprintf(fp, "\n");
									}
									else {
										fputs("lw\t$v0, ", fp);
										fputs(int_to_str(addr), fp);
										fputs("($sp)\n", fp);
										fputs("sw\t$v0, ", fp);
										fputs(int_to_str(returnvaluespace), fp);
										fprintf(fp, "\n");
									}
								}
							}
						}
					}
					fputs("addiu\t$sp, $sp, ", fp);
					fputs(int_to_str(localparaoffset), fp);
					fputs("\n", fp);
					fputs("lw\t$ra, 0($sp)\n", fp);
					fputs("addiu\t$sp, $sp, 4\n", fp);

					fputs("lw\t$fp, 0($sp)\n", fp);
					fputs("addiu\t$sp, $sp, 4\n", fp);

					fputs("jr\t$ra\n", fp);

					funccallpara = 0;
				}
				else if (strcmp(opitem.op, Exit) == 0) {										
					fputs("li\t$v0, 10\n", fp);
					fputs("syscall\n", fp);
				}
				else {
					continue;
				}
			}
		}
	}
	fclose(fp);
}