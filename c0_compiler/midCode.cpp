#include "midCode.h"
#include "error.h"
#include "valueDefine.h"
#include "symbolTable.h"

FOURYUANCODE	midcode[MAX_MID_CODE];
int codenum_count = 0;				//中间代码条数
int lablenum_count = 0;				//标签计数，同时用于生成标签
int varnum_count = 0;				//变量名计数，同时用于生成变量名，主要是运算的中间值
int strnum_count = 0;				//字符串数

void genMidcode(char *op, char *var1, char *var2, char *rst)
{
	if (codenum_count < MAX_MID_CODE) {
		
		strcpy(midcode[codenum_count].op, op);
		strcpy(midcode[codenum_count].var1, var1);
		strcpy(midcode[codenum_count].var2, var2);
		strcpy(midcode[codenum_count].rst, rst);
		codenum_count++;
		
		//printf("%s\t%s\t%s\t%s\n", op, var1, var2, rst);
	}
	else {
		error(21, linecount);
		return;
	}
}

void genMidcode(char *op, char *var1, int var2, char *rst)
{
	if (codenum_count < MAX_MID_CODE) {
		char int_to_str[MAX_WORD_CHAR];
		itoa(var2, int_to_str, 10);
		strcpy(midcode[codenum_count].op, op);
		strcpy(midcode[codenum_count].var1, var1);
		strcpy(midcode[codenum_count].var2, int_to_str);
		strcpy(midcode[codenum_count].rst, rst);
		codenum_count++;

		//printf("%s\t%s\t%s\t%s\n", op, var1, int_to_str, rst);
	}
	else {
		error(21, linecount);
		return;
	}
}

char *genLable()
{
	char lable[10] = { '\0' };				
	sprintf(lable, "LABLE_%d", lablenum_count);
	lablenum_count++;
	return lable;
}

char *genVarname()
{
	char varname[10] = { '\0' };
	sprintf(varname, "VAR_%d", varnum_count);
	varnum_count++;
	return varname;
}

char *genStrlable()
{
	char strlable[10] = { '\0' };
	sprintf(strlable, "STRING_%d", strnum_count);
	strnum_count++;
	return strlable;
}

void constMerge()
{
	int i;
	FOURYUANCODE item;
	for (i = 0; i < codenum_count; i++) {
		item = midcode[i];
		if (strcmp(item.op, Add) == 0 || strcmp(item.op, Sub) == 0 || strcmp(item.op, Mul) == 0 || strcmp(item.op, Div) == 0) {
			if (Allisnum(item.var1) && Allisnum(item.var2)) {
				int temp = 0;
				if (strcmp(item.op, Add) == 0) {
					temp = atoi(item.var1) + atoi(item.var2);
				}
				else if (strcmp(item.op, Sub) == 0) {
					temp = atoi(item.var1) - atoi(item.var2);
				}
				else if (strcmp(item.op, Mul) == 0) {
					temp = atoi(item.var1) * atoi(item.var2);
				}
				else if (strcmp(item.op, Div) == 0) {
					if (atoi(item.var2) != 0) {
						temp = atoi(item.var1) / atoi(item.var2);
					}
				}
				char int_to_str[10];
				itoa(temp, int_to_str, 10);
				if (strcmp(midcode[i + 1].op, Assign) == 0 && strcmp(midcode[i + 1].var1, item.rst) == 0 && strcmp(midcode[i + 1].var2, Space) == 0) {
					strcpy(midcode[i].op, Assign);
					strcpy(midcode[i].var1, int_to_str);
					strcpy(midcode[i].var2, Space);
					strcpy(midcode[i].rst, midcode[i + 1].rst);
					strcpy(midcode[i+1].op, Space);
					strcpy(midcode[i+1].var1, Space);
					strcpy(midcode[i+1].var2, Space);
					strcpy(midcode[i+1].rst, Space);

				}
				else {
					strcpy(midcode[i].op, Assign);
					strcpy(midcode[i].var1, int_to_str);
					strcpy(midcode[i].var2, Space);
				}
			}
		}
	}
}

