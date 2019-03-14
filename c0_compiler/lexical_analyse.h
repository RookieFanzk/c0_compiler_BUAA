#pragma once
#include "valueDefine.h"

extern const char * keywords_array[KEY_NUM];
extern const char * symbolwords_array[SYMBOL_NUM];

struct Word {								//单词结构体
	char current_str[MAX_WORD_CHAR];
	symboltype current_sy;
};

extern char filecontents[MAX_FILE_CHAR];	//存储文件的全部字符

extern int file_length;						//文件长度
extern int fileindex;						//当前行指针
extern int linecount;						//当前行数	

extern char current_char;					//当前读入的字符
extern Word word;							//当前单词
extern int hasAsym;							//判断是否已经有了一个单词

extern int file_finish;

void readfile(char *filepath);

void tolowercase(char string[]);			//标识符全部转化为小写

int reserver(char string[]);				//查保留字表

extern void retract();

extern Word getsymWord();

void nextsym();

void skip(char endskip);

extern void sethasAsym();

