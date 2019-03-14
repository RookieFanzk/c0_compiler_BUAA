#pragma once
#include "valueDefine.h"

typedef struct {
	char op[10];
	char var1[100];
	char var2[100];
	char rst[100];
}FOURYUANCODE;

extern FOURYUANCODE	midcode[MAX_MID_CODE];

extern int codenum_count;				
extern int lablenum_count;				
extern int varnum_count;
extern int strnum_count;

void genMidcode(char *op, char *var1, char *var2, char *rst);

void genMidcode(char *op, char *var1, int var2, char *rst);			//对带有常量的赋值或运算语句

char *genLable();

char *genVarname();

char *genStrlable();

void constMerge();			//常量合并，暂时只处理了一种简单情况如 a = 1 + 2；当表达式右边不止两项时，还未处理

