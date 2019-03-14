#pragma once
#include "lexical_analyse.h"

//编译开始
void compileBegin();

//＜程序＞    :: = ［＜常量说明＞］［＜变量说明＞］ [<函数定义>]＜主函数＞
void procedure();

//＜主函数＞    ::= void main‘(’‘)’‘{’＜复合语句＞‘}’
int mainfunction();

//＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
int constDeclare();

//＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}| char＜标识符＞＝＜字符＞{ ,＜标识符＞＝＜字符＞ }
int constDefine();

//＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞
int headDefine();

//＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}
int varDeclare();

//＜变量定义＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’){,(＜标识符＞|＜标识符＞‘[’＜无符号整数＞‘]’ )}  //＜无符号整数＞表示数组元素的个数，其值需大于0
int varDefine();

//<函数定义>	::= {＜有返回值函数定义＞ | ＜无返回值函数定义＞}
int functionDefine();

//＜有返回值函数定义＞  ::=  ＜声明头部＞‘(’＜参数表＞‘)’ ‘{’＜复合语句＞‘}’|＜声明头部＞‘{’＜复合语句＞‘}’  //第一种选择为有参数的情况，第二种选择为无参数的情况
int functionHasRetDefine();

//＜无返回值函数定义＞  ::= void＜标识符＞(’＜参数表＞‘)’‘{’＜复合语句＞‘}’| void＜标识符＞{’＜复合语句＞‘}’//第一种选择为有参数的情况，第二种选择为无参数的情况
int functionNoRetDefine();

//＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞
int comStatement();

//＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}
int paraTable();

//＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}  //[+|-]只作用于第一个<项>
int expression();

//＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}
int item();

//＜因子＞    ::= ＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’|‘(’＜表达式＞‘)’｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
int factor();

//＜语句＞    ::= ＜条件语句＞｜＜循环语句＞| ‘{’＜语句列＞‘}’| ＜有返回值函数调用语句＞; | ＜无返回值函数调用语句＞; ｜＜赋值语句＞; ｜＜读语句＞; ｜＜写语句＞; ｜＜空＞; | ＜返回语句＞;
int sentence();

//＜条件语句＞::= if ‘(’＜条件＞‘)’＜语句＞[else＜语句＞]
int sentenceIf();

//＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞｜＜表达式＞ //表达式为0条件为假，否则为真
int condition(char *lable);

//<无符号整数>  ::= ＜数字＞｛＜数字＞｝
int intUnsigned();

//＜整数＞        ::= ［＋｜－］＜无符号整数＞
int intCol();

//<while循环语句>	::= while ‘(’＜条件＞‘)’＜语句＞
int sentenceWhile();

//<for循环语句>		::= for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
int sentenceFor();

//＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}
int valueOfparaTable();

//＜读语句＞    ::=  scanf ‘(’＜标识符＞{,＜标识符＞}‘)’
int sentenceScanf();

//＜写语句＞    ::= printf ‘(’ ＜字符串＞,＜表达式＞ ‘)’| printf ‘(’＜字符串＞ ‘)’| printf ‘(’＜表达式＞‘)’
int sentencePrintf();

//＜返回语句＞   ::=  return[‘(’＜表达式＞‘)’]  
int sentenceReturn();

void genmidCodeFile();

void genopmidCodeFile();


