#include "tomips.h"
#include "symbolTable.h"

STRINGANDLABLE stringAndlable[MAX_STRING_NUM];
int stringnum = 0;

TEMPVAR tempvar[MAX_TEMP_VAR];			//临时变量
int tempvarcount = 0;

GLOBALCONST globalpara[MAX_GLOBAL];		//全局的变量
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

void genMips()			
{
	genData();
	genText();
}

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

void genData()
{
	FILE *ffp = fopen(mipsCodefileName, "w+");
	fputs(".data", ffp);
	fprintf(ffp, "\n");
	int i;
	int arrayspace = 0;
	for (i = 0; i < codenum_count; i++) {
		if (strcmp(midcode[i].op, Func) == 0) {
			beginnum = i;
			break;
		}
		if (strcmp(midcode[i].op, Var) == 0) {						//全局var定义
			fprintf(ffp, "\t");
			fputs(midcode[i].rst, ffp);
			fprintf(ffp, ":.space 4");
			fprintf(ffp, "\n");
			strcpy(globalpara[globalcount].globalname, midcode[i].rst);
			strcpy(globalpara[globalcount].value, Space);
			globalcount++;
		}
	}
	for (i = 0; i < codenum_count; i++) {
		if (strcmp(midcode[i].op, Func) == 0) {
			break;
		}
		if (strcmp(midcode[i].op, Array) == 0) {					//数组元素，申请.space
			arrayspace = atoi(midcode[i].var2) * 4;
			fprintf(ffp, "\t");
			fputs(midcode[i].rst, ffp); fprintf(ffp, ":.space ");
			char int_to_str[100];
			itoa(arrayspace, int_to_str, 10);
			fputs(int_to_str, ffp);
			fprintf(ffp, "\n");
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
				fprintf(ffp, "\t");
				fputs(strlable, ffp);
				fprintf(ffp, ":.asciiz ");
				fprintf(ffp, "\"");
				fputs(midcode[i].var1, ffp);
				fprintf(ffp, "\"\n");
				strcpy(stringAndlable[stringnum].string, midcode[i].var1);
				strcpy(stringAndlable[stringnum].lable, strlable);
				stringnum++;
			}
		}
	}
	fclose(ffp);
}

void genText()
{
	FILE *fp = fopen(mipsCodefileName, "a+");

	fputs(".text\n", fp);
	fprintf(fp, "\n");
	fputs("j\tmain\n", fp);

	int i;
	for (i = beginnum; i < codenum_count; i++) {											//全局变量在Data段已经处理
		FOURYUANCODE item = midcode[i];
		if (strcmp(item.op, Var) == 0) {													//变量定义,加入局部变量表
			if (strcmp(item.var1, Int) == 0 || strcmp(item.var1, Char) == 0) {
				strcpy(localvar[localvarcount].varname, item.rst);
				localvar[localvarcount].addr = localoffset;
				fputs("addiu\t$sp, $sp, -4\n", fp);
				localoffset = localoffset + 4;
				localvarcount++;
			}
		}
		else if (strcmp(item.op, Array) == 0) {
			if (strcmp(item.var1, Int) == 0 || strcmp(item.var1, Char) == 0) {
				int arrayspace = atoi(item.var2) * 4;
				char int_to_str[10];
				itoa(arrayspace, int_to_str, 10);
				strcpy(localvar[localvarcount].varname, item.rst);
				localvar[localvarcount].addr = localoffset;
				fputs("addiu\t$sp, $sp, -", fp);
				fputs(int_to_str, fp);
				fprintf(fp, "\n");
				localoffset = localoffset + arrayspace;
				localvarcount++;
			}
		}
		else if (strcmp(item.op, Lab) == 0) {
			fputs(item.rst, fp);
			fprintf(fp, ":\n");
		}
		else if (strcmp(item.op, Func) == 0) {												//函数定义
			funcparanum = 0;

			fputs(item.rst, fp);
			fprintf(fp, ":\n");
			for (int j = i + 1; j < codenum_count; j++) {
				if(strcmp(midcode[j].op, Para) == 0) {
					funcparanum++;
				}
				else {
					break;
				}
			}
			for (int j = i + 1; j < codenum_count; j++) {
				if (strcmp(midcode[j].op, Para) == 0) {
					continue;
				}
				else if (strcmp(midcode[j].op, Var) == 0) {
					localparaoffset = localparaoffset + 4;
				}
				else if (strcmp(midcode[j].op, Array) == 0) {
					int arrayspace = atoi(midcode[j].var2) * 4;;
					localparaoffset = localparaoffset + arrayspace;
				}
				else if(strcmp(midcode[j].op, Const) == 0){
					continue;
				}
				else {
					break;
				}
			}

			if (strcmp(item.rst, "main") != 0) {
				fputs("addiu\t$sp, $sp, -4\n", fp);
				fputs("sw\t$ra, 0($sp)\n", fp);
				if (funcparanum != 0) {
					fputs("li\t$t0, ", fp);
					char int_to_str[10];
					itoa(funcparanum * 4, int_to_str, 10);
					fputs(int_to_str, fp);
					fprintf(fp, "\n");
					fputs("subu\t$sp, $sp, $t0\n", fp);
				}
			}
			else {
				fputs("li\t$fp, ", fp);
				char int_to_str[10];
				itoa(tempvaraddrBegin, int_to_str, 10);
				fputs(int_to_str, fp);
				fprintf(fp, "\n");
			}

		}
		else if (strcmp(item.op, Funcend) == 0) {											//函数定义结束，临时变量，局部变量清零
			localvarcount = 0;
			localoffset = 0;
			localparaoffset = 0;

			tempvarcount = 0;
			tempoffset = 0;
		}
		else if (strcmp(item.op, Para) == 0) {												//参数定义
			switch (funcdefpara) {
				case 0:
					fputs("sw\t$a0, 0($sp)\n", fp);
					break;
				case 1:
					fputs("sw\t$a1, 4($sp)\n", fp);
					break;
				case 2:
					fputs("sw\t$a2, 8($sp)\n", fp);
					break;
				case 3:
					fputs("sw\t$a3, 12($sp)\n", fp);
					break;
				default:
					break;
			}
			strcpy(localvar[localvarcount].varname, item.rst);
			localvar[localvarcount].addr = localparaoffset;
			localparaoffset = localparaoffset + 4;
			localvarcount++;
			funcdefpara++;
			if (strcmp(midcode[i + 1].op, Para) != 0) {
				funcdefpara = 0;
			}
		}
		else if (strcmp(item.op, ParaCall) == 0) {											//值参表
			int addr,flag = 0;
			if (item.rst[0] == 'V' && item.rst[1] == 'A' && item.rst[2] == 'R' && item.rst[3] == '_') {
				addr = gettempaddr(item.rst);
				flag = 1;
			}
			else {
				addr = getlocalvarAddr(item.rst);
			}
			char int_to_str1[10];
			itoa(funccallpara, int_to_str1, 10);
			if (funccallpara < 4) {
				if (flag == 1) {
					char int_to_str2[20];
					itoa(addr, int_to_str2, 10);
					fputs("lw\t$a", fp);
					fputs(int_to_str1, fp);
					fputs(", ", fp);
					fputs(int_to_str2, fp);
					fprintf(fp, "($fp)\n");
				}
				else {
					if (addr == -1) {
						if (Allisnum(item.rst) || item.rst[0] == '-') {
							fputs("li\t$a", fp);
							fputs(int_to_str1, fp);
							fputs(", ", fp);
							fputs(item.rst, fp);
							fprintf(fp, "\n");
						}
						else {
							fputs("la\t$t0, ", fp);
							fputs(item.rst, fp);
							fprintf(fp, "\n");
							fputs("lw\t$a", fp);
							fputs(int_to_str1, fp);
							fputs(", 0($t0)\n", fp);
						}
					}
					else {
						fputs("lw\t$a", fp);
						fputs(int_to_str1, fp);
						fputs(", ", fp);
						char int_to_str2[20];
						itoa(addr, int_to_str2, 10);
						fputs(int_to_str2, fp);
						fputs("($sp)\n", fp);
					}
				}
			}
			else {
				//得到该函数的参数个数，当参数超过4个时，用于确定接下来的参数存储的位置
				int getparanum = 0;
				for (int k = i + 1; k < codenum_count; k++) {
					if (strcmp(midcode[k].op, ParaCall) == 0) {
						continue;
					}
					if (strcmp(midcode[k].op, Call) == 0) {
						for (int i = 0; i < symTable.index; i++) {
							if (strcmp(symTable.symolArray[i].idname, midcode[k].var1) == 0) {
								if (symTable.symolArray[i].symbolType == Function) {
									getparanum = symTable.symolArray[i].paraNum;
									break;
								}
							}
						}
					}
					break;
				}
				if (flag == 1) {
					char int_to_str2[20];
					itoa(addr, int_to_str2, 10);
					fputs("lw\t$t9, ", fp);
					fputs(int_to_str2, fp);
					fprintf(fp, "($fp)\n");
				}
				else {
					if (addr == -1) {
						if (Allisnum(item.rst) || item.rst[0] == '-') {
							fputs("li\t$t9, ", fp);
							fputs(item.rst, fp);
							fprintf(fp, "\n");
						}
						else {
							fputs("la\t$t0, ", fp);
							fputs(item.rst, fp);
							fprintf(fp, "\n");
							fputs("lw\t$t9, 0($t0)\n", fp);
						}
					}
					else {
						fputs("lw\t$t9, ", fp);
						char int_to_str2[20];
						itoa(addr, int_to_str2, 10);
						fputs(int_to_str2, fp);
						fputs("($sp)\n", fp);
					}
				}
				fputs("sw\t$t9, -", fp);
				char int_to_str[10];
				itoa((getparanum - funccallpara) * 4 + 8, int_to_str, 10);
				fputs(int_to_str, fp);
				fprintf(fp, "($sp)\n");
			}
			funccallpara++;
		}
		else if (strcmp(item.op, Call) == 0) {												//函数调用
			char int_to_str[20];
			int addr;

			fputs("addiu\t$sp, $sp, -4\n", fp);
			fputs("sw\t$fp, 0($sp)\n", fp);
			fputs("addiu\t$fp, $fp, ", fp);
			itoa(tempoffset, int_to_str, 10);
			fputs(int_to_str, fp);
			fprintf(fp, "\n");

			fputs("jal\t", fp);
			fputs(item.var1, fp);
			fprintf(fp, "\n");
			if (strcmp(item.rst, Space) != 0) {
				inserttempvar(item.rst);
				addr = gettempaddr(item.rst);
				fputs("lw\t$t0, ", fp);
				itoa(returnvaluespace, int_to_str, 10);
				fputs(int_to_str, fp);
				fprintf(fp, "\n");
				itoa(addr, int_to_str, 10);
				fputs("sw\t$t0, ", fp);
				fputs(int_to_str, fp);
				fprintf(fp, "($fp)\n");
			}
			funccallpara = 0;
		}
		else if (strcmp(item.op, Assign) == 0) {											//赋值语句
			int addr_a, addr_b, addr_i;	
			if (strcmp(item.var2, Space) == 0) {		// a = b型
				if (Allisnum(item.var1) || item.var1[0] == '-') {	// b为立即数或字符
					fputs("li\t$t0, ", fp);
					fputs(item.var1, fp);
					fprintf(fp, "\n");
				}
				else if (isChar(item.var1)) {
					fputs("li\t$t0, '", fp);
					fputs(item.var1, fp);
					fprintf(fp, "'\n");
				}
				else {
					if (item.var1[0] == 'V' && item.var1[1] == 'A' && item.var1[2] == 'R' && item.var1[3] == '_') {	//为临时变量
						addr_b = gettempaddr(item.var1);
						char int_to_str[20];
						itoa(addr_b, int_to_str, 10);
						fputs("lw\t$t0, ", fp);	
						fputs(int_to_str, fp);
						fprintf(fp, "($fp)\n");
					}
					else {
						addr_b = getlocalvarAddr(item.var1);
						if (addr_b == -1) {							//未找到时在Data中取
							fputs("la\t$t0, ", fp);
							fputs(item.var1, fp);
							fprintf(fp, "\n");
							fputs("lw\t$t0, 0($t0)\n", fp);		
						}
						else {
							fputs("lw\t$t0, ", fp);
							char int_to_str[20];
							itoa(addr_b, int_to_str, 10);
							fputs(int_to_str, fp);
							fprintf(fp, "($sp)\n");
						}
					}
				}
				if (item.rst[0] == 'V' && item.rst[1] == 'A' && item.rst[2] == 'R' && item.rst[3] == '_') {
					inserttempvar(item.rst);
					addr_a = gettempaddr(item.rst);
					char int_to_str[20];
					itoa(addr_a, int_to_str, 10);
					fputs("sw\t$t0, ", fp);
					fputs(int_to_str, fp);
					fprintf(fp, "($fp)\n");
				}
				else {
					addr_a = getlocalvarAddr(item.rst);
					if (addr_a == -1) {
						fputs("la\t$t1, ", fp);
						fputs(item.rst, fp);
						fprintf(fp, "\n");
						fputs("sw\t$t0, 0($t1)\n", fp);
					}
					else {
						fputs("sw\t$t0, ", fp);
						char int_to_str[20];
						itoa(addr_a, int_to_str, 10);
						fputs(int_to_str, fp);
						fprintf(fp, "($sp)\n");
					}
				}
			}
			else {															//a = b[i]数组取值
				if (Allisnum(item.var2)) {
					fputs("li\t$t1, ", fp);
					fputs(item.var2, fp);
					fprintf(fp, "\n");
					fputs("li\t$t2, 4\n", fp);
					fputs("mult\t$t1, $t2\n", fp);
					fputs("mflo\t$t1\n", fp);
				}
				else {
					if (item.var2[0] == 'V' && item.var2[1] == 'A' && item.var2[2] == 'R' && item.var2[3] == '_') {
						addr_i = gettempaddr(item.var2);
						fputs("lw\t$t1, ", fp);
						char int_to_str[20];
						itoa(addr_i, int_to_str, 10);
						fputs(int_to_str, fp);
						fprintf(fp, "($fp)\n");
						fputs("li $t2, 4\n", fp);
						fputs("mult\t$t1, $t2\n", fp);
						fputs("mflo\t$t1\n", fp);
					}
					else {
						addr_i = getlocalvarAddr(item.var2);
						if (addr_i == -1) {
							fputs("la\t$t1, ", fp);
							fputs(item.var2, fp);
							fprintf(fp, "\n");
							fputs("lw\t$t1,0($t1)\n", fp);
							fputs("li $t2, 4\n", fp);
							fputs("mult\t$t1, $t2\n", fp);
							fputs("mflo\t$t1\n", fp);
						}
						else {
							char int_to_str[20];
							itoa(addr_i, int_to_str, 10);
							fputs("lw\t$t1, ", fp);
							fputs(int_to_str, fp);
							fprintf(fp, "($sp)\n");
							fputs("li $t2, 4\n", fp);
							fputs("mult\t$t1, $t2\n", fp);
							fputs("mflo\t$t1\n", fp);
						}
					}
				}
				addr_b = getlocalvarAddr(item.var1);
				if (addr_b == -1) {
					fputs("la\t$t0, ", fp);
					fputs(item.var1, fp);
					fprintf(fp, "\n");
					fputs("addu\t$t0, $t0, $t1\n", fp);
					fputs("lw\t$t0, 0($t0)\n", fp);
				}
				else {
					fputs("li\t$t0, ", fp);
					char int_to_str[20];
					itoa(addr_b, int_to_str, 10);
					fputs(int_to_str, fp);
					fprintf(fp, "\n");
					fputs("addu\t$t0, $t0, $t1\n", fp);
					fputs("addu\t$t0, $t0, $sp\n", fp);
					fputs("lw\t$t0, 0($t0)\n", fp);
				}
				if (item.rst[0] == 'V' && item.rst[1] == 'A' && item.rst[2] == 'R' && item.rst[3] == '_') {
					inserttempvar(item.rst);
					addr_a = gettempaddr(item.rst);
					char int_to_str[20];
					itoa(addr_a, int_to_str, 10);
					fputs("sw\t$t0, ", fp);
					fputs(int_to_str, fp);
					fprintf(fp, "($fp)\n");
				}
				else {
					addr_a = getlocalvarAddr(item.rst);
					if (addr_a == -1) {
						fputs("la\t$t1, ", fp);
						fputs(item.rst, fp);
						fprintf(fp, "\n");
						fputs("sw\t$t0, 0($t1)\n", fp);
					}
					else {
						fputs("sw\t$t0, ", fp);
						char int_to_str[20];
						itoa(addr_a, int_to_str, 10);
						fputs(int_to_str, fp);
						fprintf(fp, "($sp)\n");
					}
				}
			}
		}
		else if (strcmp(item.op, Assarr) == 0) {											//数组赋值 a[i] = b
			int addr_a, addr_b, addr_i;
			if (Allisnum(item.var1) || item.var1[0] == '-') {
				fputs("li\t$t0, ", fp);
				fputs(item.var1, fp);
				fprintf(fp, "\n");
			}
			else if (isChar(item.var1)) {
				fputs("li\t$t0, '", fp);
				fputs(item.var1, fp);
				fprintf(fp, "'\n");
			}
			else {
				if (item.var1[0] == 'V' && item.var1[1] == 'A' && item.var1[2] == 'R' && item.var1[3] == '_') {	//为临时变量
					addr_b = gettempaddr(item.var1);
					char int_to_str[20];
					itoa(addr_b, int_to_str, 10);
					fputs("lw\t$t0, ", fp);
					fputs(int_to_str, fp);
					fprintf(fp, "($fp)\n");
				}
				else {
					addr_b = getlocalvarAddr(item.var1);
					if (addr_b == -1) {
						fputs("la\t$t0, ", fp);
						fputs(item.var1, fp);
						fprintf(fp, "\n");
						fputs("lw\t$t0, 0($t0)\n", fp);
					}
					else {
						fputs("lw\t$t0, ", fp);
						char int_to_str[20];
						itoa(addr_b, int_to_str, 10);
						fputs(int_to_str, fp);
						fprintf(fp, "($sp)\n");
					}
				}
			}
			if (Allisnum(item.var2)) {
				fputs("li\t$t1, ", fp);
				fputs(item.var2, fp);
				fprintf(fp, "\n");
				fputs("li $t2, 4\n", fp);
				fputs("mult\t$t1, $t2\n", fp);
				fputs("mflo\t$t1\n", fp);
			}
			else {
				if (item.var2[0] == 'V' && item.var2[1] == 'A' && item.var2[2] == 'R' && item.var2[3] == '_') {
					addr_i = gettempaddr(item.var2);
					fputs("lw\t$t1, ", fp);
					char int_to_str[20];
					itoa(addr_i, int_to_str, 10);
					fputs(int_to_str, fp);
					fprintf(fp, "($fp)\n");
					fputs("li $t2, 4\n", fp);
					fputs("mult\t$t1, $t2\n", fp);
					fputs("mflo\t$t1\n", fp);
				}
				else {
					addr_i = getlocalvarAddr(item.var2);
					if (addr_i == -1) {
						fputs("la\t$t1, ", fp);
						fputs(item.var2, fp);
						fprintf(fp, "\n");
						fputs("lw\t$t1,0($t1)\n", fp);
						fputs("li $t2, 4\n", fp);
						fputs("mult\t$t1, $t2\n", fp);
						fputs("mflo\t$t1\n", fp);
					}
					else {
						char int_to_str[20];
						itoa(addr_i, int_to_str, 10);
						fputs("lw\t$t1, ", fp);
						fputs(int_to_str, fp);
						fprintf(fp, "($sp)\n");
						fputs("li $t2, 4\n", fp);
						fputs("mult\t$t1, $t2\n", fp);
						fputs("mflo\t$t1\n", fp);
					}
				}
			}
			addr_a = getlocalvarAddr(item.rst);
			if (addr_a == -1) {
				fputs("la\t$t2, ", fp);
				fputs(item.rst, fp);
				fprintf(fp, "\n");
				fputs("addu\t$t1, $t1, $t2\n", fp);
				fputs("sw\t$t0, 0($t1)\n", fp);
			}
			else {
				fputs("li\t$t2, ", fp);
				char int_to_str[20];
				itoa(addr_a, int_to_str, 10);
				fputs(int_to_str, fp);
				fprintf(fp, "\n");
				fputs("addu\t$t1, $t1, $t2\n", fp);
				fputs("addu\t$t1, $t1, $sp\n", fp);
				fputs("sw\t$t0, 0($t1)\n", fp);
			}
		}
		else if (strcmp(item.op, Jmp) == 0) {												//无条件跳转
			fprintf(fp, "j\t");
			fputs(item.rst, fp);
			fprintf(fp, "\n");
		}																					//其他6种跳转语句
		else if ((strcmp(item.op, Beq) == 0) || (strcmp(item.op, Bgez) == 0) || (strcmp(item.op, Bgtz) == 0) || (strcmp(item.op, Bltz) == 0) || (strcmp(item.op, Blez) == 0) || (strcmp(item.op, Bne) == 0)) {
			int addr;
			if (item.var1[0] == 'V' && item.var1[1] == 'A' && item.var1[2] == 'R' && item.var1[3] == '_') {
				addr = gettempaddr(item.var1);
				char int_to_str[20];
				itoa(addr, int_to_str, 10);
				fputs("lw\t$t0, ", fp);
				fputs(int_to_str, fp);
				fprintf(fp, "($fp)\n");
			}
			else if (Allisnum(item.var1)) {
				fputs("li\t$t0, ", fp);
				fputs(item.var1, fp);
				fprintf(fp, "\n");
			}
			else {
				addr = getlocalvarAddr(item.var1);
				if (addr == -1) {
					fputs("la\t$t0, ", fp);
					fputs(item.var1, fp);
					fprintf(fp, "\n");
					fputs("lw\t$t0, 0($t0)\n", fp);
				}
				else {
					char int_to_str[20];
					itoa(addr, int_to_str, 10);
					fputs("lw\t$t0, ", fp);
					fputs(int_to_str, fp);
					fprintf(fp, "($sp)\n");
				}
			}
			if (strcmp(item.op, Beq) == 0) {
				fputs("beq\t$t0, $0, ", fp);
				fputs(item.rst, fp);
				fprintf(fp, "\n");
			}
			else if (strcmp(item.op, Bne) == 0) {
				fputs("bne\t$t0, $0, ", fp);
				fputs(item.rst, fp);
				fprintf(fp, "\n");
			}
			else if (strcmp(item.op, Bgez) == 0) {
				fputs("bgez\t$t0, ", fp);
				fputs(item.rst, fp);
				fprintf(fp, "\n");
			}
			else if (strcmp(item.op, Bgtz) == 0) {
				fputs("bgtz\t$t0, ", fp);
				fputs(item.rst, fp);
				fprintf(fp, "\n");
			}
			else if (strcmp(item.op, Blez) == 0) {
				fputs("blez\t$t0, ", fp);
				fputs(item.rst, fp);
				fprintf(fp, "\n");
			}
			else if (strcmp(item.op, Bltz) == 0) {
				fputs("bltz\t$t0, ", fp);
				fputs(item.rst, fp);
				fprintf(fp, "\n");
			}
		}
		else if ((strcmp(item.op, Add) == 0) || (strcmp(item.op, Sub) == 0) || (strcmp(item.op, Mul) == 0) || (strcmp(item.op, Div) == 0)) {
			int addr1, addr2, addr3;										//运算表达式，逐个项分析
			if (Allisnum(item.var1) || (item.var1[0] == '-')) {
				fputs("li\t$t0, ", fp);
				fputs(item.var1, fp);
				fprintf(fp, "\n");
			}
			else {
				if (item.var1[0] == 'V' && item.var1[1] == 'A' && item.var1[2] == 'R' && item.var1[3] == '_') {
					addr1 = gettempaddr(item.var1);
					fputs("lw\t$t0, ", fp);
					char int_to_str[20];
					itoa(addr1, int_to_str, 10);
					fputs(int_to_str, fp);
					fprintf(fp, "($fp)\n");
				}
				else {
					addr1 = getlocalvarAddr(item.var1);
					if (addr1 == -1) {
						fputs("la\t$t0, ", fp);
						fputs(item.var1, fp);
						fprintf(fp, "\n");
						fputs("lw\t$t0, 0($t0)\n", fp);
					}
					else {
						char int_to_str[20];
						itoa(addr1, int_to_str, 10);
						fputs("lw\t$t0, ", fp);
						fputs(int_to_str, fp);
						fputs("($sp)\n", fp);
					}
				}
			}
			if (Allisnum(item.var2) || (item.var2[0] == '-')) {
				fputs("li\t$t1, ", fp);
				fputs(item.var2, fp);
				fprintf(fp, "\n");
			}
			else {
				if (item.var2[0] == 'V' && item.var2[1] == 'A' && item.var2[2] == 'R' && item.var2[3] == '_') {
					addr2 = gettempaddr(item.var2);
					fputs("lw\t$t1, ", fp);
					char int_to_str[20];
					itoa(addr2, int_to_str, 10);
					fputs(int_to_str, fp);
					fprintf(fp, "($fp)\n");
				}
				else {
					addr2 = getlocalvarAddr(item.var2);
					if (addr2 == -1) {
						fputs("la\t$t1, ", fp);
						fputs(item.var2, fp);
						fprintf(fp, "\n");
						fputs("lw\t$t1, 0($t1)\n", fp);
					}
					else {
						char int_to_str[20];
						itoa(addr2, int_to_str, 10);
						fputs("lw\t$t1, ", fp);
						fputs(int_to_str, fp);
						fputs("($sp)\n", fp);
					}
				}
			}
			if (strcmp(item.op, Add) == 0) {
				fputs("addu\t$t0, $t0, $t1\n", fp);
			}
			else if (strcmp(item.op, Sub) == 0) {
				fputs("subu\t$t0, $t0, $t1\n", fp);
			}
			else if (strcmp(item.op, Mul) == 0) {
				fputs("mult\t$t0, $t1\n", fp);
				fputs("mflo\t$t0\n", fp);
			}
			else if (strcmp(item.op, Div) == 0) {
				fputs("div\t$t0, $t1\n", fp);
				fputs("mflo\t$t0\n", fp);
			}
			if (item.rst[0] == 'V' && item.rst[1] == 'A' && item.rst[2] == 'R' && item.rst[3] == '_') {		//运算过程中命名的临时变量
				inserttempvar(item.rst);
				addr3 = gettempaddr(item.rst);
				char int_to_str[20];
				itoa(addr3, int_to_str, 10);
				fputs("sw\t$t0, ", fp);
				fputs(int_to_str, fp);
				fprintf(fp, "($fp)\n");
			}
			else {
				addr3 = getlocalvarAddr(item.rst);
				if (addr3 == -1) {
					fputs("la\t$t1, ", fp);
					fputs(item.rst, fp);
					fprintf(fp, "\n");
					fputs("sw\t$t0, 0($t1)\n", fp);
				}
				else {
					char int_to_str[20];
					itoa(addr3, int_to_str, 10);
					fputs("sw\t$t0, ", fp);
					fputs(int_to_str, fp);
					fprintf(fp, "($sp)\n");
				}
			}
		}
		else if (strcmp(item.op, Scanf) == 0) {									 //读语句
			if (strcmp(item.var1, Int) == 0) {
				fputs("li\t$v0, 5\n",fp);
			}
			else if (strcmp(item.var1, Char) == 0) {
				fputs("li\t$v0, 12\n", fp);
			}
			fputs("syscall\n", fp);
			int addr = getlocalvarAddr(item.rst);
			if (addr == -1) {
				fputs("la\t$t0, ", fp);
				fputs(item.rst, fp);
				fprintf(fp, "\n");
				fputs("sw\t$v0, 0($t0)\n", fp);
			}
			else {
				fputs("sw\t$v0, ", fp);
				char int_to_str[20];
				itoa(addr, int_to_str, 10);
				fputs(int_to_str, fp);
				fprintf(fp, "($sp)\n");
			}
		}
		else if (strcmp(item.op, Printf) == 0) {								//写语句
			int addr2;
			char stringlable[10];
			if (strcmp(item.var1, None) != 0) {
				fputs("la\t$a0, ", fp);
				for (int i = 0; i < stringnum; i++) {
					if (strcmp(item.var1, stringAndlable[i].string) == 0) {
						strcpy(stringlable, stringAndlable[i].lable);
						break;
					}
				}
				fputs(stringlable, fp);
				fprintf(fp, "\n");
				fputs("li\t$v0, 4\n", fp);
				fputs("syscall\n", fp);
			}
			if (strcmp(item.var2, Space) != 0) {
				if (isChar(item.var2)) {									//输出字符
					fputs("li\t$a0, ", fp);
					fputs(item.var2, fp);
					fprintf(fp, "\n");
					fputs("li\t$v0, 11\n", fp);
					fputs("syscall\n", fp);
				}
				else if (Allisnum(item.var2) || item.var2[0] == '-') {		//输出整数
					fputs("li\t$a0, ", fp);
					fputs(item.var2, fp);
					fprintf(fp, "\n");
					if (strcmp(item.rst, "int") == 0) {
						fputs("li\t$v0, 1\n", fp);
						fputs("syscall\n", fp);
					}
					else {
						fputs("li\t$v0, 11\n", fp);
						fputs("syscall\n", fp);
					}
				}															//输出表达式
				else if(item.var2[0] == 'V' && item.var2[1] == 'A' && item.var2[2] == 'R' && item.var2[3] == '_'){
					addr2 = gettempaddr(item.var2);					//在临时变量表中找该变量
					fputs("lw\t$a0, ", fp);
					char int_to_str[20];
					itoa(addr2, int_to_str, 10);
					fputs(int_to_str, fp);
					fprintf(fp, "($fp)\n");
					if (strcmp(item.rst, "int") == 0) {
						fputs("li\t$v0, 1\n", fp);
						fputs("syscall\n", fp);
					}
					else {
						fputs("li\t$v0, 11\n", fp);
						fputs("syscall\n", fp);
					}
				}
				else {														//输出某个变量
					addr2 = getlocalvarAddr(item.var2);
					if (addr2 == -1) {
						fputs("la\t$t0 ", fp);
						fputs(item.var2, fp);
						fprintf(fp, "\n");
						fputs("lw\t$a0, 0($t0)\n", fp);
					}
					else {
						char int_to_str[20];
						itoa(addr2, int_to_str, 10);
						fputs("lw\t$a0, ", fp);
						fputs(int_to_str, fp);
						fprintf(fp, "($sp)\n");
					}
					if (strcmp(item.rst, "int") == 0) {
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
		else if (strcmp(item.op, Ret) == 0) {										//返回语句
			if (strcmp(item.rst, Space) != 0) {
				if (Allisnum(item.rst) || isChar(item.rst)) {
					fputs("li\t$v1, ", fp);
					fputs(item.rst, fp);
					fprintf(fp, "\n");
					fputs("sw\t$v1, ", fp);
					char int_to_str[20];
					itoa(returnvaluespace, int_to_str, 10);
					fputs(int_to_str, fp);
					fprintf(fp, "\n");
				}
				else {
					int addr;
					if (item.rst[0] == 'V' && item.rst[1] == 'A' && item.rst[2] == 'R' && item.rst[3] == '_') {
						addr = gettempaddr(item.rst);
						fputs("lw\t$v1, ", fp);
						char int_to_str[20];
						itoa(addr, int_to_str, 10);
						fputs(int_to_str, fp);
						fprintf(fp, "($fp)\n");
						fputs("sw\t$v1, ", fp);
						itoa(returnvaluespace, int_to_str, 10);
						fputs(int_to_str, fp);
						fprintf(fp, "\n");
					}
					else {
						addr = getlocalvarAddr(item.rst);
						if (addr == -1) {
							fputs("la\t$t0, ", fp);
							fputs(item.rst, fp);
							fprintf(fp, "\n");
							fputs("lw\t$v1, 0($t0)\n", fp);
							char int_to_str[20];
							fputs("sw\t$v1, ", fp);
							itoa(returnvaluespace, int_to_str, 10);
							fputs(int_to_str, fp);
							fprintf(fp, "\n");
						}
						else {
							char int_to_str[20];
							itoa(addr, int_to_str, 10);
							fputs("lw\t$v1, ", fp);
							fputs(int_to_str, fp);
							fprintf(fp, "($sp)\n");
							fputs("sw\t$v1, ", fp);
							itoa(returnvaluespace, int_to_str, 10);
							fputs(int_to_str, fp);
							fprintf(fp, "\n");
						}
					}
				}
			}
			char int_to_str[20];
			fputs("addiu\t$sp, $sp, ", fp);
			itoa(localparaoffset, int_to_str, 10);
			fputs(int_to_str, fp);
			fprintf(fp, "\n");
			fputs("lw\t$ra, 0($sp)\n", fp);
			fputs("addiu\t$sp, $sp, 4\n", fp);

			fputs("lw\t$fp, 0($sp)\n", fp);
			fputs("addiu\t$sp, $sp, 4\n", fp);

			fputs("jr\t$ra\n",fp);
			
			funccallpara = 0;
		}
		else if (strcmp(item.op, Exit) == 0) {										//程序退出
			fputs("li\t$v0, 10\n", fp);
			fputs("syscall\n", fp);
		}
		else {
			continue;
		}
	}
	fclose(fp);
}