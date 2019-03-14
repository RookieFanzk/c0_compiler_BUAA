#include "syntax_analyse.h"
#include "error.h"
#include "symbolTable.h"
#include "midCode.h"
#include "tomips.h"
#include "optimize.h"

int numberValue = 0;
int opcoderecord = 1;				//运算符记录,‘+’为1，‘-’为0
int typerecord = -1;				//记录区分int\char等
char errorid[MAX_WORD_CHAR];		//记录误读标识符
char currentfunc[MAX_WORD_CHAR];	//当前函数名，用于后续记录参数
char currentItem[MAX_WORD_CHAR];	//记录当前表达式结果

int isVoidfunc = false;				//有返回值和无返回值函数
int funcHasret = false;				//处理定义为有返回值函数却缺少返回值的问题

int isVoidMain = false;				//为主函数
int inForloop = false;				//For循环中，条件为一个单独的表达式，可以接分号
int inIf = false;					//在If语句中，条件可以为单独表达式，后面是括号
int inWhile = false;				//While语句同上

int var_function_error = false;		//在变量声明结束时，接着的一个函数有可能为返回值函数，以此来记录冲突
int error_word[3];

char funcrecord[MAX_WORD_CHAR];		//调用函数时记录函数名，以便匹配参数类型;

void compileBegin()
{
	init_symbolTable();			//先初始化符号表
	nextsym();					//预读一个symbol
	procedure();				//开始分析整个程序
	genmidCodeFile();			//输出中间代码
	genopmidCodeFile();			//输出优化后的中间代码
	genMips();					//生成mips代码
}

//＜程序＞    :: = ［＜常量说明＞］［＜变量说明＞］{ ＜函数定义＞ }＜主函数＞
void procedure()
{
	constDeclare();

	varDeclare();

	functionDefine();

	mainfunction();
}

//＜主函数＞    ::= void main‘(’‘)’‘{’＜复合语句＞‘}’
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


//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
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

//＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}| char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
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

//＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞
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

//＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
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

//＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ )}  //＜无符号整数＞表示数组元素的个数，其值需大于0
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

//<函数定义>	::= {＜有返回值函数定义＞ | ＜无返回值函数定义＞}
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

//＜有返回值函数定义＞  ::=  ＜声明头部＞‘(’＜参数表＞‘)’ ‘{’＜复合语句＞‘}’|＜声明头部＞‘{’＜复合语句＞‘}’  //第一种选择为有参数的情况，第二种选择为无参数的情况
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

//＜无返回值函数定义＞  ::= void＜标识符＞(’＜参数表＞‘)’‘{’＜复合语句＞‘}’| void＜标识符＞{’＜复合语句＞‘}’//第一种选择为有参数的情况，第二种选择为无参数的情况
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

//＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
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

//＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}
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
	表达式、项、因子三者的处理思路相近，用三个中间变量，轮换去处理每一个小部分表达式
*/

//＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}  //[+|-]只作用于第一个<项>
int expression()					
{
	Word word1;
	factortype = -1;
	char result1[MAX_WORD_CHAR];		//表达式有多个项，用于存放中间运算结果
	char result2[MAX_WORD_CHAR];
	char result3[MAX_WORD_CHAR];		//3用于存放每个计算的最终结果
	strcpy(result1, Space);				//每次处理表达式先初始化为空
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
		else if (opcoderecord == 0) {			//为负号会先做一次负运算
			strcpy(result1, currentItem);
			strcpy(result3, genVarname());		//生成一个中间变量名，作为结果
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
		strcpy(currentItem, result3);	//每个表达式都可看作一个项，保存当前表达式的最终结果，后续需要用到表达式处理时作为返回值使用
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

//＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}
int item()
{
	Word word1;
	char result1[MAX_WORD_CHAR];			//也是开三个中间数组存
	char result2[MAX_WORD_CHAR];
	char result3[MAX_WORD_CHAR];		
	strcpy(result1, Space);				
	strcpy(result2, Space);
	strcpy(result3, Space);

	factor();
	strcpy(result3, currentItem);			//先记录factor处理的最终结果
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

//＜因子＞    ::= ＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’|‘(’＜表达式＞‘)’｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
/*
	处理factor时，两种函数调用以及数组顺序非常重要，由于第一次没写好，debug好久(TVT)
*/
int factor()
{										
	Word word1;
	char result3[MAX_WORD_CHAR];						 //虽然因子中不涉及到运算，为了统一，依然用result3记录结果
	char identifierName[MAX_WORD_CHAR];
	strcpy(result3, Space);								
	int checkTab;
	word1 = getsymWord();
	if (word1.current_sy == IDEN) {
		strcpy(idname, word1.current_str);
		strcpy(identifierName, word1.current_str);		 //可能是函数调用，先记录

		nextsym();
		word1 = getsymWord();
		if (word1.current_sy == LMBRACKET) {			 //是数组,需要判断是否越界,表达式不是整数，是一个复杂表达式时，
			nextsym();									 //需要先计算其值才可判断，此处先不考虑
			lookupTab(idname, false);
			int temp = factortype;			//数组有可能是char类型，处理下标时，先记录类型
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
		else if (word1.current_sy == LSBRACKET) {			//有参数函数调用
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
		else{												//无参数函数调用，已经读过一个单词，记住不要预读
			paraNum = 0;
			checkTab = lookupTab(identifierName, true);
			if (checkTab == 1) {
				strcpy(result3, genVarname());
				genMidcode(Call, identifierName, Space, result3);
				strcpy(currentItem, result3);
				return 1;
			}
			checkTab = lookupTab(identifierName, false);		//说明只是标识符
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
	最开始设计为将赋值语句、函数调用语句也单独用函数写出来，但在调试中发现，由于标识符的冲突，导致这个参数在这三个函数中的关系十分复杂，
	而且在factor的处理中，也会涉及到与函数调用语句的冲突，所以索性就直接写入到语句分析里面，这样参数的传递就会显得比较简单。
*/

//＜语句＞    ::= ＜条件语句＞｜＜循环语句＞| ‘{’＜语句列＞‘}’| ＜有返回值函数调用语句＞; | ＜无返回值函数调用语句＞; ｜＜赋值语句＞; ｜＜读语句＞; ｜＜写语句＞; ｜＜空＞; | ＜返回语句＞;
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
	else if (word1.current_sy == IDEN) {	//赋值语句与函数调用冲突部分
		char result1[MAX_WORD_CHAR];		//存放表达式值
		char result2[MAX_WORD_CHAR];		//存放数组下标
		char result3[MAX_WORD_CHAR];
		char identifierName[MAX_WORD_CHAR];	//赋值语句、函数调用语句存在冲突，先保存冲突变量
		int checkTab;

		strcpy(idname, word1.current_str);
		strcpy(identifierName, word1.current_str);
		strcpy(result3, word1.current_str);
		nextsym();
		word1 = getsymWord();
		if (word1.current_sy == ASSIGN) {
			checkTab = lookupTab(idname, false);
			int	typematch = factortype;
			if (isconstvalue == true) {		//对常量赋值，报错
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
		else if (word1.current_sy == SEMICOLON) {				//无参数函数调用
			paraNum = 0;										//由于文法原因，函数定义和调用无参数时不要括号，没有经过值参数表处理，
																//手动将参数个数置0，不然paraNum值会错乱
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
		else if (word1.current_sy == LSBRACKET) {				//有参数函数调用
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
	else if (word1.current_sy == SEMICOLON) {		//空语句
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

//＜条件语句＞::= if ‘(’＜条件＞‘)’＜语句＞[else＜语句＞]
int sentenceIf()
{
	Word word1;
	char lable1[MAX_WORD_CHAR];				//生成if和else标签，用于跳转
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
	genMidcode(Lab, Space, Space, lable1);			//第一个标签放在else之前，if中条件不满足是，跳转执行else
	word1 = getsymWord();
	if (word1.current_sy == ELSESY) {
		nextsym();
		sentence();
	}
	genMidcode(Lab, Space, Space, lable2);			//第二个标签放在else之后，if中条件满足时，直接跳过else
	return 1;
}

//＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞ //表达式为0条件为假，否则为真
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
		else if (word1.current_sy == SEMICOLON) {				//在for中，条件中只有一个表达式，后面可以时分号
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

//<无符号整数>  ::= ＜数字＞｛＜数字＞｝
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

//＜整数＞        ::= ［＋｜－］＜无符号整数＞
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

//<while循环语句>	::= while ‘(’＜条件＞‘)’＜语句＞
int sentenceWhile()
{
	Word word1;
	char lable1[MAX_WORD_CHAR];		//生成2个标签用于跳转
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

//<for循环语句>		::= for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
int sentenceFor()
{
	Word word1;
	char lable1[MAX_WORD_CHAR];		//生成2个标签用于跳转
	char lable2[MAX_WORD_CHAR];
	char lable3[MAX_WORD_CHAR];

	char idname1[MAX_WORD_CHAR];	//共涉及到3个标识符名
	char idname2[MAX_WORD_CHAR];
	char idname3[MAX_WORD_CHAR];

	char op[5];						//存运算符
	char op1[MAX_WORD_CHAR];		//存操作数
	char op2[MAX_WORD_CHAR];

	int checkTab;					//查符号表结果

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

	genMidcode(Jmp, Space, Space, lable2);		//第一次进来for后，直接跳转到循环体执行

	genMidcode(Lab, Space, Space, lable1);		//第一个标签放在条件判断之前
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

	genMidcode(op, idname3, op1, op2);			//与步长作运算，赋值给左侧标识符
	genMidcode(Assign, op2, Space, idname2);

	genMidcode(Jmp, Space, Space, lable1);		//跳转到条件判断
	genMidcode(Lab, Space, Space, lable3);		//第三个标签，结束循环
	return 1;
}

//＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}
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

//＜读语句＞    ::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’
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
	if (isarrayvalue) {					//不能读入数组元素
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
			if (isarrayvalue) {					//不能读入数组元素
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

//＜写语句＞    ::= printf ‘(’ ＜字符串＞,＜表达式＞ ‘)’| printf ‘(’＜字符串＞ ‘)’| printf ‘(’＜表达式＞‘)’
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

//＜返回语句＞   ::=  return[‘(’＜表达式＞‘)’] 
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

