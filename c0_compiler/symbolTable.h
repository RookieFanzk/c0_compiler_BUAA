#pragma once
#include "valueDefine.h"

typedef struct {
	char idname[MAX_WORD_CHAR];		//符号名
	int value;						//符号值，为int或char常量时为相应的值，返回值函数int为1,char为2，无返回值函数时为0,为变量时存储INTSY\CHARSY
	int address;					//符号地址
	itemtype symbolType;			//符号类型 
	int paraNum;					//函数参数个数，如果不是函数则为-1
	int size;						//数组大小，不是数组则为0
}symbolWord;

typedef struct {
	symbolWord symolArray[MAX_FILE_WORDS];
	int index;						//栈顶指针
	int totalPro;					//分程序总数
	int indexOfpro[MAX_PRO];		//分程序索引
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

int getArraylenth(char *idname);		//在使用数组时需要得到数组长度以判断是否越界

int isAbleinsert(char *idname);			//判断符号是否能插入符号表

int getFuncparatype(char *funcname, int paranumber);	//函数参数类型匹配函数

int funcHasbracket(char *funcname); //函数无参数时不能有括号

int Allisnum(char *string);				

int isChar(char *string);


