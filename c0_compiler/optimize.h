#pragma once
#include "valueDefine.h"
#include "symbolTable.h"
#include "midcode.h"

typedef struct {
	char op[10];
	char var1[100];
	char var2[100];
	char rst[100];
	int isblockbegin;
}OpMidCode;

typedef struct {
	char tempvar[10];
	char reg[4];
}TempVarReg;

typedef struct {
	int blocknum;
	int codenum;
	OpMidCode blockcode[MAX_BLOCKCODE];
	int nextblock[2];	//最多2个后继块
	char defvar[100][MAX_WORD_CHAR];
	char usevar[100][MAX_WORD_CHAR];
	char invar[100][MAX_WORD_CHAR];
	char outvar[100][MAX_WORD_CHAR];
	int defnum;
	int usenum;
	int innum;
	int outnum;
	TempVarReg tempvarReg[100];
	int tempnum;
}BasicBlock;

typedef struct {
	int index;
	int lchild;
	int rchild;
	int isleaf;
	int inqueue;	
	char nodename[MAX_WORD_CHAR];
}DagNode;

typedef struct {
	int index;
	char nodename[MAX_WORD_CHAR];
}NodeTable;

typedef struct {
	char name[MAX_WORD_CHAR];
	char confVar[100][MAX_WORD_CHAR];
	int confvarnum;
	int isAllocated;
	int edgenum;
	int flag;	//是否分配全局寄存器
	int color;
	char reg[4];
}ConflictMap;

typedef struct {
	int block[MAX_BLOCK_NUM];
	int blocknum;
	ConflictMap conflictmap[100];
	int conflictnum;
}FlowDiagram;


extern OpMidCode opmidcode[MAX_MID_CODE];
extern BasicBlock blockflow[MAX_BLOCK_NUM];
extern FlowDiagram flow[MAX_FLOW_NUM];
extern DagNode dagnode[MAX_DAGNODE];
extern NodeTable nodetab[MAX_DAGNODE];

extern int flow_index;

void devideBlock();
void optimizeMidcode();

