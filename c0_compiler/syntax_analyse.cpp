#include "syntax_analyse.h"
#include "error.h"
#include "symbolTable.h"
#include "midCode.h"
#include "tomips.h"
#include "optimize.h"

int numberValue = 0;
int opcoderecord = 1;				//�������¼,��+��Ϊ1����-��Ϊ0
int typerecord = -1;				//��¼����int\char��
char errorid[MAX_WORD_CHAR];		//��¼�����ʶ��
char currentfunc[MAX_WORD_CHAR];	//��ǰ�����������ں�����¼����
char currentItem[MAX_WORD_CHAR];	//��¼��ǰ���ʽ���

int isVoidfunc = false;				//�з���ֵ���޷���ֵ����
int funcHasret = false;				//������Ϊ�з���ֵ����ȴȱ�ٷ���ֵ������

int isVoidMain = false;				//Ϊ������
int inForloop = false;				//Forѭ���У�����Ϊһ�������ı��ʽ�����Խӷֺ�
int inIf = false;					//��If����У���������Ϊ�������ʽ������������
int inWhile = false;				//While���ͬ��

int var_function_error = false;		//�ڱ�����������ʱ�����ŵ�һ�������п���Ϊ����ֵ�������Դ�����¼��ͻ
int error_word[3];

char funcrecord[MAX_WORD_CHAR];		//���ú���ʱ��¼���������Ա�ƥ���������;

void compileBegin()
{
	init_symbolTable();			//�ȳ�ʼ�����ű�
	nextsym();					//Ԥ��һ��symbol
	procedure();				//��ʼ������������
	genmidCodeFile();			//����м����
	genopmidCodeFile();			//����Ż�����м����
	genMips();					//����mips����
}

//������    :: = �ۣ�����˵�����ݣۣ�����˵������{ ���������壾 }����������
void procedure()
{
	constDeclare();

	varDeclare();

	functionDefine();

	mainfunction();
}

//����������    ::= void main��(����)����{����������䣾��}��
int mainfunction()
{
	Word word1;
	word1 = getsymWord();
	if (word1.current_sy != MAINSY) {
		error(1,linecount);		
		return 0;
	}
	isVoidMain = true;
	strcpy(idname, word1.current_str);
	strcpy(currentfunc, idname);
	insertTab(idname, 0, 0, Function, 0, 0);
	genMidcode(Func, Space, Space, currentfunc);
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != LSBRACKET) {
		error(4,linecount);	
		sethasAsym();
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != RSBRACKET) {
		error(4, linecount);
		sethasAsym();
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != LBBRACKET) {
		error(6, linecount);
		sethasAsym();
	}
	nextsym();
	comStatement();

	if (strcmp(midcode[codenum_count - 1].op, Exit) != 0) {
		genMidcode(Exit, Space, Space, Space);
	}

	word1 = getsymWord();
	if (word1.current_sy != RBBRACKET) {
		error(6, linecount);
	}
	genMidcode(Funcend, Space, Space, Main);
	return 1;
}


//������˵���� ::=  const���������壾;{ const���������壾;}
int constDeclare()
{
	Word word1;
	word1 = getsymWord();

	if (word1.current_sy != CONSTSY) {
		return 0;
	}

	nextsym();
	constDefine();

	word1 = getsymWord();
	if (word1.current_sy != SEMICOLON) {
		error(8, linecount);
		sethasAsym();
	}
	
	while (1) {
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != CONSTSY)
			break;
		nextsym();
		constDefine();
		word1 = getsymWord();
		if (word1.current_sy != SEMICOLON) {
			error(8, linecount);
			sethasAsym();
		}
	}
	return 1;
}

//���������壾   ::=   int����ʶ��������������{,����ʶ��������������}| char����ʶ���������ַ���{ ,����ʶ���������ַ��� }
int constDefine()
{
	Word word1;
	word1 = getsymWord();

	if (word1.current_sy == INTSY) {
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != IDEN) {
			error(12, linecount);
			skip(';');
			return 0;
		}
		strcpy(idname, word1.current_str);
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != ASSIGN) {
			error(10, linecount);
			skip(';');
			return 0;
		}
		nextsym();
		intCol();
		value = numberValue;
		address++;
		symbolType = ConstantInt;
		paraNum = -1;
		size = 0;
		insertTab(idname, value, address, symbolType, paraNum, size);

		genMidcode(Const, Int, value, idname);

		while (1) {
			word1 = getsymWord();
			if (word1.current_sy != COMMA)
				break;
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy != IDEN) {
				error(12, linecount);
				skip(';');
				return 0;
			}
			strcpy(idname, word1.current_str);
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy != ASSIGN) {
				error(10, linecount);
				skip(';');
				return 0;
			}
			nextsym();
			word1 = getsymWord();
			intCol();
			value = numberValue;
			address++;
			symbolType = ConstantInt;
			paraNum = -1;
			size = 0;
			insertTab(idname, value, address, symbolType, paraNum, size);

			genMidcode(Const, Int, value, idname);
		}

	}
	else if (word1.current_sy == CHARSY) {
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != IDEN) {
			error(12, linecount);
			skip(';');
			return 0;
		}
		strcpy(idname, word1.current_str);
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != ASSIGN) {
			error(10, linecount);
			skip(';');
			return 0;
		}
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != CHARCOL) {
			error(16, linecount);
			skip(';');
			return 0;
		}
		value = word1.current_str[0];
		address++;
		symbolType = ConstantChar;
		paraNum = -1;
		size = 0;
		insertTab(idname, value, address, symbolType, paraNum, size);

		genMidcode(Const, Char, value, idname);

		nextsym();
		while (1) {
			word1 = getsymWord();
			if (word1.current_sy != COMMA)
				break;
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy != IDEN) {
				error(12, linecount);
				skip(';');
				return 0;
			}
			strcpy(idname, word1.current_str);
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy != ASSIGN) {
				error(10, linecount);
				skip(';');
				return 0;
			}
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy != CHARCOL) {
				error(16, linecount);
				skip(';');
				return 0;
			}
			value = word1.current_str[0];
			address++;
			symbolType = ConstantChar;
			paraNum = -1;
			size = 0;
			insertTab(idname, value, address, symbolType, paraNum, size);

			genMidcode(Const, Char, value, idname);
			nextsym();
		}
	}
	else {
		skip(';');
		error(17, linecount);
		return 0;
	}
	return 1;
}

//������ͷ����   ::=  int����ʶ���� |char����ʶ����
int headDefine()
{
	Word word1;
	if (var_function_error) {
		word1.current_sy = (symboltype)error_word[0];
		if (word1.current_sy == INTSY || word1.current_sy == CHARSY) {
			value = word1.current_sy;
			word1.current_sy = (symboltype)error_word[1];
			if (word1.current_sy != IDEN) {
				error(12, linecount);
				nextsym();
				return 0;
			}
			strcpy(idname, errorid);
		}
	}
	else {
		word1 = getsymWord();
		if (word1.current_sy == INTSY || word1.current_sy == CHARSY) {
			value = word1.current_sy;
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy == MAINSY) {
				error(1, linecount);
				nextsym();
				return 0;
			}
			if (word1.current_sy != IDEN) {
				error(12, linecount);
				nextsym();
				return 0;
			}
			strcpy(idname, word1.current_str);
		}
		else {
			return 0;
		}
	}
	address = 0;
	symbolType = Function;
	paraNum = 0;
	size = 0;
	insertTab(idname, value, address, symbolType, paraNum, size);
	strcpy(currentfunc, idname);

	nextsym();
	return 1;
}

//������˵����  ::= ���������壾;{���������壾;}
int varDeclare()
{
	Word word1;
	word1 = getsymWord();

	if (!varDefine()) {
		return 0;
	}
	word1 = getsymWord();
	if (word1.current_sy != SEMICOLON) {
		error(8, linecount);
		sethasAsym();
	}
	while (1) {
		nextsym();
		word1 = getsymWord();
		if (!varDefine()) {
			break;
		}
		word1 = getsymWord();
		if (word1.current_sy != SEMICOLON) {
			error(8, linecount);
			sethasAsym();
		}
	}
	return 1;
}

//���������壾  ::= �����ͱ�ʶ����(����ʶ����|����ʶ������[�����޷�����������]��){,(����ʶ����|����ʶ������[�����޷�����������]�� )}  //���޷�����������ʾ����Ԫ�صĸ�������ֵ�����0
int varDefine()
{
	Word word1;
	word1 = getsymWord();
	error_word[2] = {0};
	if (word1.current_sy != INTSY && word1.current_sy != CHARSY) {
		return 0;
	}
	typerecord = word1.current_sy;
	error_word[0] = word1.current_sy;
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy == MAINSY) {
		error(1, linecount);
		return 0;
	}
	if (word1.current_sy != IDEN) {
		error(12, linecount);
		skip(';');
		return 0;
	}
	error_word[1] = word1.current_sy;
	strcpy(idname, word1.current_str);
	strcpy(errorid, word1.current_str);
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy == LSBRACKET || word1.current_sy == LBBRACKET) {
		error_word[2] = word1.current_sy;
		var_function_error = true;
		return 0;
	}
	if (word1.current_sy == LMBRACKET) {
		nextsym();
		intUnsigned();
		size = numberValue;

		word1 = getsymWord();
		if (word1.current_sy != RMBRACKET) {
			error(5, linecount);
			sethasAsym();
		}
		else {
			value = typerecord;
			address = address + size;
			symbolType = Variable;
			paraNum = -1;
			insertTab(idname, value, address, symbolType, paraNum, size);
			if (typerecord == INTSY) {
				genMidcode(Array, Int, size, idname);
			}
			else if (typerecord == CHARSY) {
				genMidcode(Array, Char, size, idname);
			}
			nextsym();
		}
	}
	else {
		value = typerecord;
		address++;
		symbolType = Variable;
		paraNum = -1;
		size = 0;
		insertTab(idname, value, address, symbolType, paraNum, size);
		if (typerecord == INTSY) {
			genMidcode(Var, Int, Space, idname);
		}
		else if (typerecord == CHARSY) {
			genMidcode(Var, Char, Space, idname);
		}
	}
	while (1) {
		word1 = getsymWord();
		if (word1.current_sy != COMMA) {
			break;
		}
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != IDEN) {
			error(12, linecount);
			skip(';');
			return 0;
		}
		strcpy(idname, word1.current_str);
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy == LMBRACKET) {
			nextsym();
			intUnsigned();
			size = numberValue;
			word1 = getsymWord();
			if (word1.current_sy != RMBRACKET) {
				error(5, linecount);
				sethasAsym();
			}
			else {
				value = typerecord;
				address = address + size;
				symbolType = Variable;
				paraNum = -1;
				insertTab(idname, value, address, symbolType, paraNum, size);
				if (typerecord == INTSY) {
					genMidcode(Array, Int, size, idname);
				}
				else if (typerecord == CHARSY) {
					genMidcode(Array, Char, size, idname);
				}
				nextsym();
			}
		}
		else {
			value = typerecord;
			address++;
			symbolType = Variable;
			paraNum = -1;
			size = 0;
			insertTab(idname, value, address, symbolType, paraNum, size);
			if (typerecord == INTSY) {
				genMidcode(Var, Int, Space, idname);
			}
			else if (typerecord == CHARSY) {
				genMidcode(Var, Char, Space, idname);
			}
		}
	}
	return 1;
}

//<��������>	::= {���з���ֵ�������壾 | ���޷���ֵ�������壾}
int functionDefine()
{
	Word word1;
	if (var_function_error) {
		word1.current_sy = (symboltype)error_word[0];
		if (word1.current_sy == INTSY || word1.current_sy == CHARSY) {
			functionHasRetDefine();
		}
		else if (word1.current_sy == VOIDSY) {
			functionNoRetDefine();
		}
		else {
			return 0;
		}
	}
	else {
		word1 = getsymWord();
		if (word1.current_sy == INTSY || word1.current_sy == CHARSY) {
			functionHasRetDefine();
		}
		else if (word1.current_sy == VOIDSY) {
			functionNoRetDefine();
		}
		else {
			return 0;
		}
	}
	while (1) {
		varnum_count = 0;
		word1 = getsymWord();
		if (word1.current_sy == INTSY || word1.current_sy == CHARSY) {
			functionHasRetDefine();
		}
		else if (word1.current_sy == VOIDSY) {
			functionNoRetDefine();
		}
		else {
			break;
		}
	}
	return 1;
}

//���з���ֵ�������壾  ::=  ������ͷ������(������������)�� ��{����������䣾��}��|������ͷ������{����������䣾��}��  //��һ��ѡ��Ϊ�в�����������ڶ���ѡ��Ϊ�޲��������
int functionHasRetDefine()
{
	Word word1;
	funcHasret = false;

	int rettype = 0;
	word1 = getsymWord();
	if (var_function_error) {
		int flag = 0;
		headDefine();
		typerecord = error_word[0];
		rettype = error_word[0];
		if (typerecord == INTSY) {
			genMidcode(Func, Int, Space, currentfunc);
		}
		else if (typerecord == CHARSY) {
			genMidcode(Func, Char, Space, currentfunc);
		}
		word1.current_sy = (symboltype)error_word[2];
		var_function_error = false;
		if (word1.current_sy == LSBRACKET) {
			flag = 1;
			paraTable();
			if (paraNum == 0) {
				error(33, linecount);
			}
			word1 = getsymWord();
			if (word1.current_sy != RSBRACKET) {
				error(4, linecount);
			}
			else {
				nextsym();
				word1 = getsymWord();
			}
		}
		if (word1.current_sy == LBBRACKET) {
			if (flag == 1) {
				nextsym();
				flag = 0;
			}
		}
		else {
			error(6, linecount);
		}
		comStatement();
		word1 = getsymWord();
		if (word1.current_sy != RBBRACKET) {
			error(6, linecount);
		}
		else {
			nextsym();
		}
		if (!funcHasret) {
			error(24, linecount);
		}
		if (rettype != factortype) {
			error(23, linecount);
		}
		genMidcode(Funcend, Space, Space, currentfunc);
		funcHasret = false;
		return 1;
	}
	else {
		word1 = getsymWord();
		typerecord = word1.current_sy;
		rettype = word1.current_sy;
		headDefine();
		if (typerecord == INTSY) {
			genMidcode(Func, Int, Space, currentfunc);
		}
		else if (typerecord == CHARSY) {
			genMidcode(Func, Char, Space, currentfunc);
		}
		word1 = getsymWord();
		if (word1.current_sy == LSBRACKET) {
			nextsym();
			paraTable();
			if (paraNum == 0) {
				error(33, linecount);
			}
			word1 = getsymWord();
			if (word1.current_sy != RSBRACKET) {
				error(4, linecount);
			}
			else {
				nextsym();
			}
		}
	}
	word1 = getsymWord();
	if (word1.current_sy == LBBRACKET) {
		nextsym();
	}
	else {
		error(6, linecount);
	}
	comStatement();
	word1 = getsymWord();
	if (word1.current_sy != RBBRACKET) {
		error(6, linecount);
		sethasAsym();
	}
	else {
		nextsym();
	}

	if (!funcHasret) {
		error(24, linecount);
	}
	if (rettype != factortype) {
		error(23, linecount);
	}
	genMidcode(Funcend, Space, Space, currentfunc);
	funcHasret = false;
	return 1;
}

//���޷���ֵ�������壾  ::= void����ʶ����(������������)����{����������䣾��}��| void����ʶ����{����������䣾��}��//��һ��ѡ��Ϊ�в�����������ڶ���ѡ��Ϊ�޲��������
int functionNoRetDefine()
{
	Word word1;
	word1 = getsymWord();
	if (word1.current_sy != VOIDSY) {
		return 0;
	}
	isVoidfunc = true;
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy == MAINSY) {
		return 0;
	}
	if (word1.current_sy != IDEN) {
		error(12, linecount);
	}
	strcpy(idname, word1.current_str);
	value = 0;
	address = 0;
	symbolType = Function;
	paraNum = 0;
	size = 0;
	insertTab(idname, value, address, symbolType, paraNum, size);
	strcpy(currentfunc, idname);
	genMidcode(Func, Void, Space, currentfunc);
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy == LSBRACKET) {
		nextsym();
		paraTable();
		if (paraNum == 0) {
			error(33, linecount);
		}
		word1 = getsymWord();
		if (word1.current_sy != RSBRACKET) {
			error(4, linecount);
		}
		else {
			nextsym();
		}
	}
	word1 = getsymWord();
	if (word1.current_sy == LBBRACKET) {
		nextsym();
	}
	else {
		error(6, linecount);
	}
	comStatement();
	word1 = getsymWord();
	if (word1.current_sy != RBBRACKET) {
		error(6, linecount);
	}
	else {
		nextsym();
	}
	if (strcmp(midcode[codenum_count - 1].op, Ret) != 0) {
		genMidcode(Ret, Space, Space, Space);
	}
	genMidcode(Funcend, Space, Space, currentfunc);
	isVoidfunc = false;
	return 1;
}

//��������䣾   ::=  �ۣ�����˵�����ݣۣ�����˵�����ݣ�����У�
int comStatement()
{
	constDeclare();
	varDeclare();
	while (1) {
		if (!sentence())
			break;
	}
	return 1;
}

//��������    ::=  �����ͱ�ʶ��������ʶ����{,�����ͱ�ʶ��������ʶ����}
int paraTable()
{
	Word word1;
	word1 = getsymWord();
	if (word1.current_sy == RSBRACKET) {
		return 0;
	}
	else if (word1.current_sy != INTSY && word1.current_sy != CHARSY) {
		error(18, linecount);
		skip(')');
		return 0;
	}
	typerecord = word1.current_sy;
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != IDEN) {
		error(12, linecount);
		skip(')');
		return 0;
	}
	strcpy(idname, word1.current_str);
	value = typerecord;
	address++;
	symbolType = Parament;
	paraNum++;
	size = 0;
	insertTab(idname, value, address, symbolType, paraNum, size);
	if (typerecord == INTSY) {
		genMidcode(Para, Int, Space, idname);
	}
	else if (typerecord == CHARSY) {
		genMidcode(Para, Char, Space, idname);
	}
	nextsym();
	while (1) {
		word1 = getsymWord();
		if (word1.current_sy != COMMA) {
			break;
		}
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != INTSY && word1.current_sy != CHARSY) {
			error(18, linecount);
			skip(')');
			return 0;
		}
		typerecord = word1.current_sy;
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != IDEN) {
			error(12, linecount);
			skip(')');
			return 0;
		}
		strcpy(idname, word1.current_str);
		value = typerecord;
		address++;
		symbolType = Parament;
		paraNum++;
		size = 0;
		insertTab(idname, value, address, symbolType, paraNum, size);
		if (typerecord == INTSY) {
			genMidcode(Para, Int, Space, idname);
		}
		else if (typerecord == CHARSY) {
			genMidcode(Para, Char, Space, idname);
		}
		nextsym();
	}
	insertPara(paraNum);
	return 1;
}

/*
	���ʽ����������ߵĴ���˼·������������м�������ֻ�ȥ����ÿһ��С���ֱ��ʽ
*/

//�����ʽ��    ::= �ۣ������ݣ��{���ӷ�����������}  //[+|-]ֻ�����ڵ�һ��<��>
int expression()					
{
	Word word1;
	factortype = -1;
	char result1[MAX_WORD_CHAR];		//���ʽ�ж������ڴ���м�������
	char result2[MAX_WORD_CHAR];
	char result3[MAX_WORD_CHAR];		//3���ڴ��ÿ����������ս��
	strcpy(result1, Space);				//ÿ�δ�����ʽ�ȳ�ʼ��Ϊ��
	strcpy(result2, Space);
	strcpy(result3, Space);

	word1 = getsymWord();
	if (word1.current_sy == PLUS || word1.current_sy == MINUS) {
		if (word1.current_sy == PLUS) {
			opcoderecord = 1;
		}
		else if (word1.current_sy == MINUS) {
			opcoderecord = 0;
		}
		nextsym();
		item();
		if (opcoderecord == 1) {
			strcpy(result3, currentItem);
		}
		else if (opcoderecord == 0) {			//Ϊ���Ż�����һ�θ�����
			strcpy(result1, currentItem);
			strcpy(result3, genVarname());		//����һ���м����������Ϊ���
			genMidcode(Sub, Zero, result1, result3);
		}
		opcoderecord = 1;
		factortype = INTSY;
		while (1) {
			word1 = getsymWord();
			if (!(word1.current_sy == PLUS || word1.current_sy == MINUS)) {
				break;
			}
			if (word1.current_sy == PLUS) {
				nextsym();
				item();
				strcpy(result1, result3);
				strcpy(result2, currentItem);
				strcpy(result3, genVarname());
				genMidcode(Add, result1, result2, result3);
			}
			else if (word1.current_sy == MINUS) {
				nextsym();
				item();
				strcpy(result1, result3);
				strcpy(result2, currentItem);
				strcpy(result3, genVarname());
				genMidcode(Sub, result1, result2, result3);
			}
			factortype = INTSY;
		}
		strcpy(currentItem, result3);	//ÿ�����ʽ���ɿ���һ������浱ǰ���ʽ�����ս����������Ҫ�õ����ʽ����ʱ��Ϊ����ֵʹ��
	}
	else {
		item();
		strcpy(result3, currentItem);
		while (1) {
			word1 = getsymWord();
			if (!(word1.current_sy == PLUS || word1.current_sy == MINUS)) {
				break;
			}
			if (word1.current_sy == PLUS) {
				nextsym();
				item();
				strcpy(result1, result3);
				strcpy(result2, currentItem);
				strcpy(result3, genVarname());
				genMidcode(Add, result1, result2, result3);
			}
			else if (word1.current_sy == MINUS) {
				nextsym();
				item();
				strcpy(result1, result3);
				strcpy(result2, currentItem);
				strcpy(result3, genVarname());
				genMidcode(Sub, result1, result2, result3);
			}
			factortype = INTSY;
		}
		strcpy(currentItem, result3);
	}
	return 1;
}

//���     ::= �����ӣ�{���˷�������������ӣ�}
int item()
{
	Word word1;
	char result1[MAX_WORD_CHAR];			//Ҳ�ǿ������м������
	char result2[MAX_WORD_CHAR];
	char result3[MAX_WORD_CHAR];		
	strcpy(result1, Space);				
	strcpy(result2, Space);
	strcpy(result3, Space);

	factor();
	strcpy(result3, currentItem);			//�ȼ�¼factor��������ս��
	while (1) {
		word1 = getsymWord();
		if (!(word1.current_sy == TIMES || word1.current_sy == DIV)) {
			break;
		}
		if (word1.current_sy == TIMES) {
			nextsym();
			strcpy(result1, result3);
			factor();
			strcpy(result2, currentItem);
			strcpy(result3, genVarname());
			genMidcode(Mul, result1, result2, result3);
		}
		else if (word1.current_sy == DIV) {
			nextsym();
			strcpy(result1, result3);
			factor();
			strcpy(result2, currentItem);
			strcpy(result3, genVarname());
			genMidcode(Div, result1, result2, result3);
		}
	}
	strcpy(currentItem, result3);
	return 1;
}

//�����ӣ�    ::= ����ʶ����������ʶ������[�������ʽ����]��|��(�������ʽ����)������������|���ַ��������з���ֵ����������䣾
/*
	����factorʱ�����ֺ��������Լ�����˳��ǳ���Ҫ�����ڵ�һ��ûд�ã�debug�þ�(TVT)
*/
int factor()
{										
	Word word1;
	char result3[MAX_WORD_CHAR];						 //��Ȼ�����в��漰�����㣬Ϊ��ͳһ����Ȼ��result3��¼���
	char identifierName[MAX_WORD_CHAR];
	strcpy(result3, Space);								
	int checkTab;
	word1 = getsymWord();
	if (word1.current_sy == IDEN) {
		strcpy(idname, word1.current_str);
		strcpy(identifierName, word1.current_str);		 //�����Ǻ������ã��ȼ�¼

		nextsym();
		word1 = getsymWord();
		if (word1.current_sy == LMBRACKET) {			 //������,��Ҫ�ж��Ƿ�Խ��,���ʽ������������һ�����ӱ��ʽʱ��
			nextsym();									 //��Ҫ�ȼ�����ֵ�ſ��жϣ��˴��Ȳ�����
			lookupTab(idname, false);
			int temp = factortype;			//�����п�����char���ͣ������±�ʱ���ȼ�¼����
			expression();
			factortype = temp;
			strcpy(result3, currentItem);
			if (Allisnum(result3)) {
				int arrayLenth = getArraylenth(identifierName);
				if (arrayLenth != -1 && atoi(result3) >= arrayLenth) {
					error(26, linecount);
				}
			}
			word1 = getsymWord();
			if (word1.current_sy != RMBRACKET) {
				error(5, linecount);
			}
			strcpy(result3, genVarname());
			genMidcode(Assign, identifierName, currentItem, result3);
			strcpy(currentItem, result3);
			nextsym();
			return 1;
		}
		else if (word1.current_sy == LSBRACKET) {			//�в�����������
			strcpy(funcrecord, idname);
			int bracket_flag = funcHasbracket(identifierName);
			if (bracket_flag == 0) {
				error(32, linecount);
				skip(';');
				return 0;
			}
			nextsym();
			valueOfparaTable();
			checkTab = lookupTab(identifierName, true);
			if (checkTab == -1) {
				error(2, linecount);
				skip(';');
				return 0;
			}
			word1 = getsymWord();
			if (word1.current_sy != RSBRACKET) {
				error(16, linecount);
			}
			nextsym();
			strcpy(result3, genVarname());
			genMidcode(Call, identifierName, Space, result3);
			strcpy(currentItem, result3);
			return 1;
		}
		else{												//�޲����������ã��Ѿ�����һ�����ʣ���ס��ҪԤ��
			paraNum = 0;
			checkTab = lookupTab(identifierName, true);
			if (checkTab == 1) {
				strcpy(result3, genVarname());
				genMidcode(Call, identifierName, Space, result3);
				strcpy(currentItem, result3);
				return 1;
			}
			checkTab = lookupTab(identifierName, false);		//˵��ֻ�Ǳ�ʶ��
			if (isconstvalue == true) {
				isconstvalue = false;
				char int_to_str[MAX_WORD_CHAR];
				itoa(checkTab, int_to_str, 10);
				strcpy(currentItem, int_to_str);
			}
			else {
				if (checkTab == -9876) {
					char tempvar[MAX_WORD_CHAR];
					strcpy(tempvar, genVarname());
					genMidcode(Assign, idname, Space, tempvar);
					strcpy(currentItem, tempvar);
				}
				else {
					strcpy(currentItem, idname);
				}
			}
			return 1;
		}
	}
	else if (word1.current_sy == LSBRACKET) {
		nextsym();
		expression();
		word1 = getsymWord();
		if (word1.current_sy != RSBRACKET) {
			error(4, linecount);
		}
		factortype = INTSY;
		nextsym();
		return 1;
	}
	else if (word1.current_sy == UNSIGNEDINT || word1.current_sy == PLUS || word1.current_sy == MINUS) {
		factortype = INTSY;
		if (word1.current_sy == UNSIGNEDINT) {
			strcpy(currentItem, word1.current_str);
			nextsym();
		}
		else {
			int flag = -1;
			if (word1.current_sy == PLUS) {
				flag = 1;
			}
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy != UNSIGNEDINT) {
				error(27, linecount);
				skip(';');
				return 0;
			}
			int numvalue = atoi(word1.current_str);
			numvalue = numvalue * flag;
			char int_to_str[MAX_WORD_CHAR];
			itoa(numvalue, int_to_str, 10);
			strcpy(currentItem, int_to_str);
			nextsym();
		}
		return 1;
	}
	else if (word1.current_sy == CHARCOL) {
		factortype = CHARSY;
		char int_to_str[MAX_WORD_CHAR];
		itoa(word1.current_str[0], int_to_str, 10);
		strcpy(currentItem, int_to_str);
		nextsym();
		return 1;
	}
	else {
		error(28, linecount);
		skip(';');
		return 0;
	}
}
/*
	�ʼ���Ϊ����ֵ��䡢�����������Ҳ�����ú���д���������ڵ����з��֣����ڱ�ʶ���ĳ�ͻ��������������������������еĹ�ϵʮ�ָ��ӣ�
	������factor�Ĵ����У�Ҳ���漰���뺯���������ĳ�ͻ���������Ծ�ֱ��д�뵽���������棬���������Ĵ��ݾͻ��ԵñȽϼ򵥡�
*/

//����䣾    ::= ��������䣾����ѭ����䣾| ��{��������У���}��| ���з���ֵ����������䣾; | ���޷���ֵ����������䣾; ������ֵ��䣾; ��������䣾; ����д��䣾; �����գ�; | ��������䣾;
int sentence()
{
	Word word1;
	word1 = getsymWord();
	if (word1.current_sy == IFSY) {
		return sentenceIf();
	}
	else if (word1.current_sy == FORSY) {
		return sentenceFor();
	}
	else if (word1.current_sy == WHILESY) {
		return sentenceWhile();
	}
	else if (word1.current_sy == LBBRACKET) {
		nextsym();
		while (1) {
			if (!sentence())
				break;
		}
		word1 = getsymWord();
		if (word1.current_sy != RBBRACKET) {
			error(6, linecount);
			sethasAsym();
		}
		nextsym();
		return 1;
	}
	else if (word1.current_sy == SCANFSY) {
		sentenceScanf();
		word1 = getsymWord();
		if (word1.current_sy != SEMICOLON) {
			error(8, linecount);
			sethasAsym();
		}
		nextsym();
		return 1;
	}
	else if (word1.current_sy == PRINTFSY) {
		sentencePrintf();
		word1 = getsymWord();
		if (word1.current_sy != SEMICOLON) {
			error(8, linecount);
			sethasAsym();
		}
		nextsym();
		return 1;
	}
	else if (word1.current_sy == RETURNSY) {
		sentenceReturn();
		word1 = getsymWord();
		if (word1.current_sy != SEMICOLON) {
			error(8, linecount);
			sethasAsym();
		}
		nextsym();
		return 1;
	}
	else if (word1.current_sy == IDEN) {	//��ֵ����뺯�����ó�ͻ����
		char result1[MAX_WORD_CHAR];		//��ű��ʽֵ
		char result2[MAX_WORD_CHAR];		//��������±�
		char result3[MAX_WORD_CHAR];
		char identifierName[MAX_WORD_CHAR];	//��ֵ��䡢�������������ڳ�ͻ���ȱ����ͻ����
		int checkTab;

		strcpy(idname, word1.current_str);
		strcpy(identifierName, word1.current_str);
		strcpy(result3, word1.current_str);
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy == ASSIGN) {
			checkTab = lookupTab(idname, false);
			int	typematch = factortype;
			if (isconstvalue == true) {		//�Գ�����ֵ������
				isconstvalue = false;
				error(16,linecount);
				skip(';');
				nextsym();
				return 1;
			}
			nextsym();
			expression();
			if (typematch != factortype) {
				error(16, linecount);
			}
			strcpy(result1, currentItem);
			word1 = getsymWord();
			if (word1.current_sy != SEMICOLON) {
				error(8, linecount);
				sethasAsym();
			}
			genMidcode(Assign, result1, Space, result3);
			nextsym();
			return 1;
		}
		else if (word1.current_sy == LMBRACKET) {
			checkTab = lookupTab(idname, false);
			int	typematch = factortype;
			nextsym();
			expression();
			strcpy(result2, currentItem);
			if (Allisnum(result2)) {
				int arrayLenth = getArraylenth(identifierName);
				if (arrayLenth != -1 && atoi(result2) >= arrayLenth) {
					error(26, linecount);
				}
			}
			word1 = getsymWord();
			if (word1.current_sy != RMBRACKET) {
				error(5, linecount);
				sethasAsym();
			}
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy == ASSIGN) {
				nextsym();
				expression();
				if (typematch != factortype) {
					error(16, linecount);
				}
				strcpy(result1, currentItem);
				word1 = getsymWord();
				if (word1.current_sy != SEMICOLON) {
					error(8, linecount);
					sethasAsym();
				}
				genMidcode(Assarr, result1, result2, result3);
			}
			nextsym();
			return 1;
		}
		else if (word1.current_sy == SEMICOLON) {				//�޲�����������
			paraNum = 0;										//�����ķ�ԭ�򣬺�������͵����޲���ʱ��Ҫ���ţ�û�о���ֵ��������
																//�ֶ�������������0����ȻparaNumֵ�����
			checkTab = lookupTab(identifierName, true);
			if (checkTab == -1) {
				error(2, linecount);
				skip(';');
				nextsym();
				return 1;
			}
			nextsym();
			genMidcode(Call, identifierName, Space, Space);
			return 1;
		}
		else if (word1.current_sy == LSBRACKET) {				//�в�����������
			strcpy(funcrecord, identifierName);
			int bracket_flag = funcHasbracket(identifierName);
			if (bracket_flag == 0) {
				error(32, linecount);
				skip(';');
				nextsym();
				return 1;
			}
			nextsym();
			valueOfparaTable();
			checkTab = lookupTab(identifierName, true);
			if (checkTab == -1) {
				error(2, linecount);
				skip(';');
				nextsym();
				return 1;
			}
			word1 = getsymWord();
			if (word1.current_sy != RSBRACKET) {
				error(16, linecount);
				sethasAsym();
			}
			nextsym();
			word1 = getsymWord();
			if (word1.current_sy != SEMICOLON) {
				error(8, linecount);
				sethasAsym();
			}
			nextsym();
			genMidcode(Call, identifierName, Space, Space);
			return 1;
		}
		else {
			error(16, linecount);
			skip(';');
			nextsym();
			return 1;
		}
	}
	else if (word1.current_sy == SEMICOLON) {		//�����
		if (file_finish == true) {
			return 0;
		}
		nextsym();
		return 1;
	}
	else {
		return 0;
	}
}

//��������䣾::= if ��(������������)������䣾[else����䣾]
int sentenceIf()
{
	Word word1;
	char lable1[MAX_WORD_CHAR];				//����if��else��ǩ��������ת
	char lable2[MAX_WORD_CHAR];

	word1 = getsymWord();
	if (word1.current_sy != IFSY) {
		return 0;
	}
	strcpy(lable1, genLable());
	strcpy(lable2, genLable());

	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != LSBRACKET) {
		error(4, linecount);
		sethasAsym();
	}
	nextsym();
	inIf = true;
	condition(lable1);
	inIf = false;
	word1 = getsymWord();
	if (word1.current_sy != RSBRACKET) {
		error(4, linecount);
	}
	else {
		nextsym();
	}
	sentence();
	genMidcode(Jmp, Space, Space, lable2);
	genMidcode(Lab, Space, Space, lable1);			//��һ����ǩ����else֮ǰ��if�������������ǣ���תִ��else
	word1 = getsymWord();
	if (word1.current_sy == ELSESY) {
		nextsym();
		sentence();
	}
	genMidcode(Lab, Space, Space, lable2);			//�ڶ�����ǩ����else֮��if����������ʱ��ֱ������else
	return 1;
}

//��������    ::=  �����ʽ������ϵ������������ʽ���������ʽ�� //���ʽΪ0����Ϊ�٣�����Ϊ��
int condition(char *lable)
{
	Word word1;
	char exprrst1[MAX_WORD_CHAR];
	char exprrst2[MAX_WORD_CHAR];
	char midname[MAX_WORD_CHAR];
	if (expression()) {
		if (factortype == CHARSY) {
			error(30, linecount);
		}
		strcpy(exprrst1, currentItem);
		word1 = getsymWord();
		if (word1.current_sy >= (symboltype)18 && word1.current_sy <= (symboltype)23) {
			typerecord = word1.current_sy;
			nextsym();
			expression();
			if (factortype == CHARSY) {
				error(30, linecount);
			}
			strcpy(exprrst2, currentItem);
		}
		else if (word1.current_sy == SEMICOLON) {				//��for�У�������ֻ��һ�����ʽ���������ʱ�ֺ�
			if (inForloop) {
				genMidcode(Beq, exprrst1, Zero, lable);
			}
			return 1;
		}
		else if (word1.current_sy == RSBRACKET) {
			if (inIf) {
				genMidcode(Beq, exprrst1, Zero, lable);
			}
			if (inWhile) {
				genMidcode(Beq, exprrst1, Zero, lable);
			}
			return 1;
		}
		if (typerecord == LESSEQ) {
			strcpy(midname, genVarname());
			genMidcode(Sub, exprrst1, exprrst2, midname);
			genMidcode(Bgtz, midname, Space, lable);
		}
		else if (typerecord == LESS) {
			strcpy(midname, genVarname());
			genMidcode(Sub, exprrst1, exprrst2, midname);
			genMidcode(Bgez, midname, Space, lable);
		}
		else if (typerecord == MOREEQ) {
			strcpy(midname, genVarname());
			genMidcode(Sub, exprrst1, exprrst2, midname);
			genMidcode(Bltz, midname, Space, lable);
		}
		else if (typerecord == MORE) {
			strcpy(midname, genVarname());
			genMidcode(Sub, exprrst1, exprrst2, midname);
			genMidcode(Blez, midname, Space, lable);
		}
		else if (typerecord == NOTEQ) {
			strcpy(midname, genVarname());
			genMidcode(Sub, exprrst1, exprrst2, midname);
			genMidcode(Beq, midname, Zero, lable);
		}
		else if (typerecord == EQUAL) {
			strcpy(midname, genVarname());
			genMidcode(Sub, exprrst1, exprrst2, midname);
			genMidcode(Bne, midname, Zero, lable);
		}
		else {
			error(25, linecount);
			skip(')');
			return 0;
		}
	}
	else {
		error(20, linecount);
		skip(')');
		return 0;
	}
	return 1;
}

//<�޷�������>  ::= �����֣��������֣���
int intUnsigned()
{
	Word word1;
	word1 = getsymWord();
	if (word1.current_sy != UNSIGNEDINT)
		return 0;
	if (opcoderecord == 1) {
		numberValue = atoi(word1.current_str);
	}
	else {
		numberValue = 0 - atoi(word1.current_str);
	}
	opcoderecord = 1;
	nextsym();
	return 1;
}

//��������        ::= �ۣ������ݣ��޷���������
int intCol()
{
	Word word1;
	opcoderecord = 1;
	word1 = getsymWord();
	if (word1.current_sy == PLUS || word1.current_sy == MINUS) {
		if (word1.current_sy == PLUS) {
			opcoderecord = 1;
		}
		else {
			opcoderecord = 0;
		}
		nextsym();
		return intUnsigned();
	}
	else {
		return intUnsigned();
	}
}

//<whileѭ�����>	::= while ��(������������)������䣾
int sentenceWhile()
{
	Word word1;
	char lable1[MAX_WORD_CHAR];		//����2����ǩ������ת
	char lable2[MAX_WORD_CHAR];
	word1 = getsymWord();
	if (word1.current_sy != WHILESY) {
		return 0;
	}
	strcpy(lable1, genLable());
	strcpy(lable2, genLable());
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != LSBRACKET) {
		error(4, linecount);
		sethasAsym();
	}

	genMidcode(Lab, Space, Space, lable1);

	nextsym();
	word1 = getsymWord();
	inWhile = true;
	condition(lable2);
	inWhile = false;
	word1 = getsymWord();
	if (word1.current_sy != RSBRACKET) {
		error(4, linecount);
	}
	else {
		nextsym();
	}
	sentence();
	genMidcode(Jmp, Space, Space, lable1);
	genMidcode(Lab, Space, Space, lable2);
	return 1;
}

//<forѭ�����>		::= for'('����ʶ�����������ʽ��;��������;����ʶ����������ʶ����(+|-)��������')'����䣾
int sentenceFor()
{
	Word word1;
	char lable1[MAX_WORD_CHAR];		//����2����ǩ������ת
	char lable2[MAX_WORD_CHAR];
	char lable3[MAX_WORD_CHAR];

	char idname1[MAX_WORD_CHAR];	//���漰��3����ʶ����
	char idname2[MAX_WORD_CHAR];
	char idname3[MAX_WORD_CHAR];

	char op[5];						//�������
	char op1[MAX_WORD_CHAR];		//�������
	char op2[MAX_WORD_CHAR];

	int checkTab;					//����ű���

	word1 = getsymWord();
	if (word1.current_sy != FORSY) {
		return 0;
	}
	strcpy(lable1, genLable());
	strcpy(lable2, genLable());
	strcpy(lable3, genLable());
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != LSBRACKET) {
		error(4, linecount);
		sethasAsym();
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != IDEN) {
		error(12, linecount);
	}
	strcpy(idname1, word1.current_str);
	checkTab = lookupTab(idname1, false);
	if (checkTab == -1) {
		error(2, linecount);
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != ASSIGN) {
		error(10, linecount);
	}
	nextsym();

	expression();
	genMidcode(Assign, currentItem, Space, idname1);
	word1 = getsymWord();
	if (word1.current_sy != SEMICOLON) {
		error(8, linecount);
	}
	else {
		nextsym();
	}

	genMidcode(Jmp, Space, Space, lable2);		//��һ�ν���for��ֱ����ת��ѭ����ִ��

	genMidcode(Lab, Space, Space, lable1);		//��һ����ǩ���������ж�֮ǰ
	inForloop = true;
	condition(lable3);
	inForloop = false;

	word1 = getsymWord();
	if (word1.current_sy != SEMICOLON) {
		error(8, linecount);
	}
	else {
		nextsym();
	}
	word1 = getsymWord();
	if (word1.current_sy != IDEN) {
		error(12, linecount);
	}
	strcpy(idname2, word1.current_str);
	checkTab = lookupTab(idname2, false);
	if (checkTab == -1) {
		error(2, linecount);
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != ASSIGN) {
		error(10, linecount);
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != IDEN) {
		error(12, linecount);
	}
	strcpy(idname3, word1.current_str);
	checkTab = lookupTab(idname3, false);
	if (checkTab == -1) {
		error(2, linecount);
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != PLUS && word1.current_sy != MINUS) {
		error(19,linecount);
	}
	if (word1.current_sy == PLUS) {
		strcpy(op, "+");
	}
	else if (word1.current_sy == MINUS) {
		strcpy(op, "-");
	}
	else {
		strcpy(op, Space);
	}
	nextsym();
	word1 = getsymWord();
	strcpy(op1, word1.current_str);
	intUnsigned();
	word1 = getsymWord();
	if (word1.current_sy != RSBRACKET) {
		error(4, linecount);
	}
	else {
		nextsym();
	}

	strcpy(op2, genVarname());

	genMidcode(Lab, Space, Space, lable2);

	sentence();

	genMidcode(op, idname3, op1, op2);			//�벽�������㣬��ֵ������ʶ��
	genMidcode(Assign, op2, Space, idname2);

	genMidcode(Jmp, Space, Space, lable1);		//��ת�������ж�
	genMidcode(Lab, Space, Space, lable3);		//��������ǩ������ѭ��
	return 1;
}

//��ֵ������   ::= �����ʽ��{,�����ʽ��}
int valueOfparaTable()
{
	Word word1;
	char numofpara[100][MAX_WORD_CHAR] = {0};
	int paranumber = 0;
	char temp_funcrecord[MAX_WORD_CHAR];
	strcpy(temp_funcrecord, funcrecord);
	expression();
	while (1) {
		strcpy(numofpara[paranumber],currentItem);
		paranumber++;
		if (factortype != getFuncparatype(temp_funcrecord, paranumber)) {
			error(15,linecount);
		}
		word1 = getsymWord();
		if (word1.current_sy != COMMA) {
			break;
		}
		nextsym();
		expression();
	}
	char temp[MAX_WORD_CHAR];
	for (int i = 0; i < 100; i++) {
		strcpy(temp, numofpara[i]);
		if (strcmp(temp, "") == 0) {
			break;
		}
		genMidcode(ParaCall, Space, Space, temp);
	}
	paraNum = paranumber;
	return 1;
}

//������䣾    ::=  scanf ��(������ʶ����{,����ʶ����}��)��
int sentenceScanf()
{
	Word word1;
	int checkTab;
	word1 = getsymWord();
	if (word1.current_sy != SCANFSY) {
		return 0;
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != LSBRACKET) {
		error(4, linecount);
		sethasAsym();
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != IDEN) {
		error(12, linecount);
	}
	strcpy(idname, word1.current_str);
	checkTab = lookupTab(idname, false);
	if (isarrayvalue) {					//���ܶ�������Ԫ��
		error(22, linecount);
		isarrayvalue = false;
	}
	genMidcode(Scanf, factortype == INTSY ? Int : Char, Space, idname);
	nextsym();
	while (1) {
		word1 = getsymWord();
		if (word1.current_sy != COMMA) {
			break;
		}
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy != IDEN) {
			error(12, linecount);
			nextsym();
		}
		else {
			strcpy(idname, word1.current_str);
			checkTab = lookupTab(idname, false);
			if (isarrayvalue) {					//���ܶ�������Ԫ��
				error(22, linecount);
				isarrayvalue = false;
			}
			genMidcode(Scanf, factortype == INTSY ? Int : Char, Space, idname);
			nextsym();
		}
	}
	if (word1.current_sy != RSBRACKET) {
		error(4, linecount);
		sethasAsym();
	}
	else {
		nextsym();
	}
	return 1;
}

//��д��䣾    ::= printf ��(�� ���ַ�����,�����ʽ�� ��)��| printf ��(�����ַ����� ��)��| printf ��(�������ʽ����)��
int sentencePrintf()
{
	Word word1;
	char string1[MAX_WORD_CHAR];
	char exprrst[MAX_WORD_CHAR];
	word1 = getsymWord();
	if (word1.current_sy != PRINTFSY) {
		return 0;
	}
	strcpy(string1, None);
	strcpy(exprrst, Space);
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != LSBRACKET) {
		error(4, linecount);
		sethasAsym();
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy == STRING) {
		strcpy(string1, word1.current_str);
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy == COMMA) {
			nextsym();
			expression();
			strcpy(exprrst, currentItem);
			word1 = getsymWord();
			if (word1.current_sy != RSBRACKET) {
				error(4, linecount);
				sethasAsym();
			}
		}
		else if (word1.current_sy != RSBRACKET) {
			error(4, linecount);
			sethasAsym();
		}
	}
	else {
		expression();
		strcpy(exprrst, currentItem);
		word1 = getsymWord();
		if (word1.current_sy != RSBRACKET) {
			error(4, linecount);
			sethasAsym();
		}
	}
	nextsym();
	if (factortype == INTSY) {
		genMidcode(Printf, string1, exprrst, Int);
	}
	else {
		genMidcode(Printf, string1, exprrst, Char);
	}

	return 1;
}

//��������䣾   ::=  return[��(�������ʽ����)��] 
int sentenceReturn()
{
	Word word1;
	char exprrst[MAX_WORD_CHAR];
	word1 = getsymWord();
	if (word1.current_sy != RETURNSY) {
		return 0;
	}
	nextsym();
	word1 = getsymWord();
	if (word1.current_sy != SEMICOLON && isVoidfunc) {
		error(23, linecount);
		skip(';');
		return 0;
	}
	if (word1.current_sy == LSBRACKET) {
		funcHasret = true;
		nextsym();
		expression();
		strcpy(exprrst, currentItem);
		word1 = getsymWord();
		if (word1.current_sy != RSBRACKET) {
			error(4, linecount);
			sethasAsym();
		}
		if (isVoidMain) {
			genMidcode(Exit, Space, Space, Space);
		}
		else {
			genMidcode(Ret, Space, Space, exprrst);
		}
	}
	else if (word1.current_sy == SEMICOLON) {
		if (isVoidMain) {
			genMidcode(Exit, Space, Space, Space);
			return 1;
		}
		else {
			genMidcode(Ret, Space, Space, Space);
			sethasAsym();
		}
	}
	else {
		sethasAsym();
	}
	nextsym();
	return 1;
}

void genmidCodeFile()
{
	constMerge();
	FILE *fp = fopen(midCodefileName, "w+");
	int i;
	for (i = 0; i < codenum_count; i++) {
		if (strcmp(midcode[i].op, Space) == 0) {
			continue;
		}
		fputs(midcode[i].op, fp);
		fprintf(fp, "\t");
		fputs(midcode[i].var1, fp);
		fprintf(fp, "\t");
		fputs(midcode[i].var2, fp);
		fprintf(fp, "\t");
		fputs(midcode[i].rst, fp);
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void genopmidCodeFile()
{
	optimizeMidcode();
	FILE *fp = fopen(opmidCodefileName, "w+");
	int i;
	for (i = 0; i < codenum_count; i++) {
		if (strcmp(opmidcode[i].op, Space) == 0) {
			continue;
		}
		fputs(opmidcode[i].op, fp);
		fprintf(fp, "\t");
		fputs(opmidcode[i].var1, fp);
		fprintf(fp, "\t");
		fputs(opmidcode[i].var2, fp);
		fprintf(fp, "\t");
		fputs(opmidcode[i].rst, fp);
		fprintf(fp, "\t");
		if (opmidcode[i].isblockbegin == 1) {
			fputs("1\n", fp);
		}
		else {
			fputs("0\n", fp);
		}
	}
	fclose(fp);
}

