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

void genMidcode(char *op, char *var1, int var2, char *rst);			//�Դ��г����ĸ�ֵ���������

char *genLable();

char *genVarname();

char *genStrlable();

void constMerge();			//�����ϲ�����ʱֻ������һ�ּ������ a = 1 + 2�������ʽ�ұ߲�ֹ����ʱ����δ����

