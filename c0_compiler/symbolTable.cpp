#include "symbolTable.h"
#include "error.h"

symbolTable symTable;
char idname[MAX_WORD_CHAR];
int value;
int address;
itemtype symbolType;
int paraNum;
int size;

int isconstvalue;
int isarrayvalue;
int factortype;			//�����жϵ�ǰ���ӡ�scanf���롢printf�������ʲô���͵�ֵ

void init_symbolTable()
{
	symTable.index = 0;
	symTable.totalPro = 0;
	symTable.indexOfpro[0] = 0;
}

void insertTab(char *idname, int value, int address, itemtype symboltype, int paraNum, int size)
{
	if (symTable.index > MAX_FILE_WORDS) {
		error(29, linecount);
		return;
	}
	pushTab(idname, value, address, symboltype, paraNum, size);
}

void pushTab(char *idname, int value, int address, itemtype symboltype, int paraNum, int size)
{
	if (symboltype != Function) {
		if (!isAbleinsert(idname)) {
			return;
		}
	}
	else {
		int i;
		for (i = 0; i < symTable.index; i++) {
			if (strcmp(symTable.symolArray[symTable.indexOfpro[i]].idname, idname) == 0) {
				error(3, linecount);
				return;
			}
		}
		if (i == symTable.index) {										//û���ҵ������Բ���
			symTable.indexOfpro[symTable.totalPro] = symTable.index;
			symTable.totalPro++;										//����+1
		}
	}

	strcpy(symTable.symolArray[symTable.index].idname, idname);
	symTable.symolArray[symTable.index].address = address;
	symTable.symolArray[symTable.index].paraNum = paraNum;
	symTable.symolArray[symTable.index].size = size;
	symTable.symolArray[symTable.index].symbolType = symboltype;
	symTable.symolArray[symTable.index].value = value;
	symTable.index++;
}

void insertPara(int paraNum)
{
	symTable.symolArray[symTable.indexOfpro[symTable.totalPro - 1]].paraNum = paraNum;
}

int lookupTab(char *idname, int isfunc)
{
	isconstvalue = false;
	isarrayvalue = false;
	if (isfunc) {							//��ǰ��ʶ���Ǻ���
		int i;
		for (i = 0; i < symTable.totalPro; i++) {
			if (strcmp(symTable.symolArray[symTable.indexOfpro[i]].idname, idname) == 0) {
				break;						//�ҵ��˸ú���
			}
		}
		if (i == symTable.totalPro) {			//û���ҵ�����δ����
			return -1;
		}
		if (symTable.symolArray[symTable.indexOfpro[i]].paraNum != paraNum) {			//�����������Ƿ���ͬ
			error(29, linecount);
			return -2;
		}
		factortype = symTable.symolArray[symTable.indexOfpro[i]].value;
		return 1;
	}
	else {
		int i;
		for (i = symTable.indexOfpro[symTable.totalPro - 1]; i < symTable.index; i++) {	//���ھֲ���������
			if (strcmp(symTable.symolArray[i].idname, idname) == 0) {
				break;
			}
		}
		if (i == symTable.index) {				//�ֲ���û�У���ȫ�����ҵ�
			int globaltail = symTable.indexOfpro[0];
			for (i = 0; i < globaltail; i++) {
				if (strcmp(symTable.symolArray[i].idname, idname) == 0) {
					break;
				}
			}
			if (i == globaltail) {
				error(2, linecount);
				return 0;
			}
			if (symTable.symolArray[i].symbolType == 0) {			//����
				factortype = INTSY;
				isconstvalue = true;
				return symTable.symolArray[i].value;
			}
			else if (symTable.symolArray[i].symbolType == 1) {			//�ַ�����
				factortype = CHARSY;
				isconstvalue = true;
				return symTable.symolArray[i].value;
			}
			else if (symTable.symolArray[i].symbolType == 2) {
				factortype = symTable.symolArray[i].value;
				if (symTable.symolArray[i].size != 0) {				//����
					isarrayvalue = true;
					return symTable.symolArray[i].address;
				}
				else {
					if (i < symTable.indexOfpro[0]) {
						return -9876;
					}
					else {
						return symTable.symolArray[i].address;
					}
				}
			}
			else if (symTable.symolArray[i].symbolType == 4) {
				factortype = symTable.symolArray[i].value;
				return -1;
			}
			else {
				return -2;
			}
		}
		else {
			if (symTable.symolArray[i].symbolType == 0) {
				factortype = INTSY;
				isconstvalue = true;
				return symTable.symolArray[i].value;
			}
			else if (symTable.symolArray[i].symbolType == 1) {
				factortype = CHARSY;
				isconstvalue = true;
				return symTable.symolArray[i].value;
			}
			else if (symTable.symolArray[i].symbolType == 2) {
				factortype = symTable.symolArray[i].value;
				if (symTable.symolArray[i].size != 0) {
					isarrayvalue = true;
				}
				return symTable.symolArray[i].address;
			}
			else if (symTable.symolArray[i].symbolType == 4) {
				factortype = symTable.symolArray[i].value;
				return -1;
			}
			else {
				return -2;
			}
		}
	}
}

int getArraylenth(char *idname)
{
	int i;
	for (i = symTable.indexOfpro[symTable.totalPro - 1]; i < symTable.index; i++) {
		if (strcmp(symTable.symolArray[i].idname, idname) == 0) {
			return symTable.symolArray[i].size;
		}
	}
	if (i == symTable.index) {		//��ȫ�ֱ������ҵ�
		i = 0;
		for (; i < symTable.indexOfpro[1]; i++) {
			if (strcmp(symTable.symolArray[i].idname, idname) == 0) {
				return symTable.symolArray[i].size;
			}
		}
	}
	return -1;
}

int isAbleinsert(char *idname)
{
	int i = symTable.indexOfpro[symTable.totalPro - 1];
	for (; i < symTable.index; i++) {
		if (strcmp(symTable.symolArray[i].idname, idname) == 0) {
			error(3, linecount);
			return 0;
		}
	}
	return 1;
}

int getFuncparatype(char *funcname, int paranumber)
{
	int i;
	for (i = 0; i < symTable.totalPro; i++) {
		if (strcmp(symTable.symolArray[symTable.indexOfpro[i]].idname, funcname) == 0) {
			break;						//�ҵ��˸ú���
		}
	}
	return symTable.symolArray[symTable.indexOfpro[i] + paranumber].value;
}

int funcHasbracket(char *funcname)
{
	int i;
	for (i = 0; i < symTable.totalPro; i++) {
		if (strcmp(symTable.symolArray[symTable.indexOfpro[i]].idname, funcname) == 0) {
			break;						//�ҵ��˸ú���
		}
	}

	factortype = symTable.symolArray[symTable.indexOfpro[i]].value;

	if (symTable.symolArray[symTable.indexOfpro[i]].paraNum != 0) {
		return 1;
	}
	return 0;
}

int Allisnum(char *string)
{
	for (int i = 0; i < strlen(string); i++) {
		if (string[i] >= 48 && string[i] <= 57) {
			continue;
		}
		else {
			return false;
		}
	}
	return true;
}

int isChar(char *string)
{
	int i = 2;
	if (string[0] >= ' ' && string[0] <= '~') {
		if (string[1] == '\0') {
			while (i < strlen(string)) {
				if (string[i] == ' ') {
					return 1;
				}
			}
		}
	}
	return 0;
}


