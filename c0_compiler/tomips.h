#pragma once
#include "valueDefine.h"
#include "midCode.h"

typedef struct {			//ÿ��string���������ݶζ�Ӧ�ı�ǩ
	char string[100];
	char lable[10];
}STRINGANDLABLE;

typedef struct {			//��ʱ����
	char tempname[MAX_WORD_CHAR];
	int addr;
}TEMPVAR;

typedef struct {			//ȫ�ֱ���
	char globalname[MAX_WORD_CHAR];
	char value[MAX_WORD_CHAR];
}GLOBALCONST;

typedef struct {			//�ֲ�����
	char varname[MAX_WORD_CHAR];
	int addr;
}LOCALVAR;

extern STRINGANDLABLE stringAndlable[MAX_STRING_NUM];
extern TEMPVAR tempvar[MAX_TEMP_VAR];
extern GLOBALCONST globalpara[MAX_GLOBAL];
extern LOCALVAR	localvar[MAX_VAR];

void insertlocalvar(char *varname);
int getlocalvarAddr(char *varname);
void inserttempvar(char *varname);
int gettempaddr(char *varname);

void genData();
void genText();
void genMips();


