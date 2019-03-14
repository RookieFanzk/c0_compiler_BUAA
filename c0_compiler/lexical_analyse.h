#pragma once
#include "valueDefine.h"

extern const char * keywords_array[KEY_NUM];
extern const char * symbolwords_array[SYMBOL_NUM];

struct Word {								//���ʽṹ��
	char current_str[MAX_WORD_CHAR];
	symboltype current_sy;
};

extern char filecontents[MAX_FILE_CHAR];	//�洢�ļ���ȫ���ַ�

extern int file_length;						//�ļ�����
extern int fileindex;						//��ǰ��ָ��
extern int linecount;						//��ǰ����	

extern char current_char;					//��ǰ������ַ�
extern Word word;							//��ǰ����
extern int hasAsym;							//�ж��Ƿ��Ѿ�����һ������

extern int file_finish;

void readfile(char *filepath);

void tolowercase(char string[]);			//��ʶ��ȫ��ת��ΪСд

int reserver(char string[]);				//�鱣���ֱ�

extern void retract();

extern Word getsymWord();

void nextsym();

void skip(char endskip);

extern void sethasAsym();

