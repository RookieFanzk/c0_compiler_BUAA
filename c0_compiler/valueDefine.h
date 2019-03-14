#pragma once
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#pragma warning(disable: 4996)

//通用常量定义
#define MAX_FILEPATH_NAME		100
#define KEY_NUM			12
#define SYMBOL_NUM		37
#define true			1
#define false			0
#define MAX_WORD_CHAR	100
#define MAX_FILE_CHAR	1000000
#define MAX_FILE_WORDS	1000000
#define MAX_PRO			100
#define MAX_MID_CODE	10000
#define MAX_STRING_NUM	100
#define MAX_ARRAY_NUM	100
#define MAX_GLOBAL		100
#define MAX_VAR			100
#define MAX_TEMP_VAR	100
#define MAX_BLOCKCODE	1000
#define MAX_BLOCK_NUM	500
#define MAX_FLOW_NUM	100
#define MAX_DAGNODE		500

enum symboltype {
	CONSTSY, INTSY, CHARSY, VOIDSY, MAINSY, IFSY, ELSESY, WHILESY, FORSY, SCANFSY, PRINTFSY, RETURNSY,
	PLUS, MINUS, TIMES, DIV, CHARCOL, UNSIGNEDINT,
	LESS, LESSEQ, MORE, MOREEQ, NOTEQ, EQUAL, ASSIGN,
	LSBRACKET, RSBRACKET, LMBRACKET, RMBRACKET, LBBRACKET, RBBRACKET,
	COMMA, SEMICOLON, COLON, BACKSLASH,
	IDEN, STRING
};

enum itemtype {
	ConstantInt, ConstantChar, Variable, Function, Parament
};

extern const char* midCodefileName;
extern const char* opmidCodefileName;
extern const char* mipsCodefileName;
extern const char* tempreg[10];

//四元式相关,定义如下
/*
	const int a = 1				const,int,1,a
	int a						var,int, , a
	int a[10]					array,int,10, a
	a = b						=,b, ,a
	a[i] = b					[]=,b,i,a
	a = b[i]					=,b,i,a
	int func()					func,int,,func
	func(int a,,,)				para,int,,,a
	(a,b)//值参表				para_in_call, , ,a
								para_in_call, , ,b
	a = func()					call,func, ,a
	func()						call,func,,
	a = b + c					add,b,c,a
	a = b - c					sub,b,c,a
	a = b * c					mul,b,c,a
	a = b / c					div, b,c,a
	if(!true)					条件中生成跳转指令
	if(true)					jmp,,,lable
	set lable					lab:,,,lable
	return 						ret,,,
	return (a)					ret,,,a
	scanf(a);//a为int			scanf,int,,a
								scanf,char,,a
	printf("a",b)				printf,a,b,int	int为b的类型（如果有b的话）
	printf("a")					printf,a,,int
	printf(b)					printf,,b,int
*/
extern char Space[];
extern char Const[];
extern char Var[];
extern char Array[];
extern char Int[];
extern char Char[];
extern char Void[];
extern char Assign[];
extern char Assarr[];
extern char Func[];
extern char Funcend[];
extern char Para[];
extern char ParaCall[];
extern char Call[];
extern char Beq[];
extern char Bgez[];
extern char Bgtz[];
extern char Blez[];
extern char Bltz[];
extern char Bne[];
extern char Add[];
extern char Sub[];
extern char Mul[];
extern char Div[];
extern char Jmp[];
extern char Lab[];
extern char Ret[];
extern char Scanf[];
extern char Printf[];
extern char Exit[];
extern char Main[];
extern char Zero[];
extern char None[];

