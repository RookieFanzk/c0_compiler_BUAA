#pragma once
#include "valueDefine.h"

typedef struct {
	char idname[MAX_WORD_CHAR];		//������
	int value;						//����ֵ��Ϊint��char����ʱΪ��Ӧ��ֵ������ֵ����intΪ1,charΪ2���޷���ֵ����ʱΪ0,Ϊ����ʱ�洢INTSY\CHARSY
	int address;					//���ŵ�ַ
	itemtype symbolType;			//�������� 
	int paraNum;					//��������������������Ǻ�����Ϊ-1
	int size;						//�����С������������Ϊ0
}symbolWord;

typedef struct {
	symbolWord symolArray[MAX_FILE_WORDS];
	int index;						//ջ��ָ��
	int totalPro;					//�ֳ�������
	int indexOfpro[MAX_PRO];		//�ֳ�������
}symbolTable;

extern symbolTable symTable;
extern char idname[MAX_WORD_CHAR];		
extern int value;
extern int address;
extern itemtype symbolType;
extern int paraNum;
extern int size;

extern char funcname[MAX_WORD_CHAR];

extern int isconstvalue;
extern int isarrayvalue;
extern int factortype;

void init_symbolTable();

void insertTab(char *idname, int value, int address, itemtype symboltype, int paraNum, int size);

void pushTab(char *idname, int value, int address, itemtype symboltype, int paraNum, int size);

void insertPara(int paraNum);

int lookupTab(char *idname, int isfunc);

int getArraylenth(char *idname);		//��ʹ������ʱ��Ҫ�õ����鳤�����ж��Ƿ�Խ��

int isAbleinsert(char *idname);			//�жϷ����Ƿ��ܲ�����ű�

int getFuncparatype(char *funcname, int paranumber);	//������������ƥ�亯��

int funcHasbracket(char *funcname); //�����޲���ʱ����������

int Allisnum(char *string);				

int isChar(char *string);


