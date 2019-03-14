#pragma once
#include "lexical_analyse.h"

//���뿪ʼ
void compileBegin();

//������    :: = �ۣ�����˵�����ݣۣ�����˵������ [<��������>]����������
void procedure();

//����������    ::= void main��(����)����{����������䣾��}��
int mainfunction();

//������˵���� ::=  const���������壾;{ const���������壾;}
int constDeclare();

//���������壾   ::=   int����ʶ��������������{,����ʶ��������������}| char����ʶ���������ַ���{ ,����ʶ���������ַ��� }
int constDefine();

//������ͷ����   ::=  int����ʶ���� |char����ʶ����
int headDefine();

//������˵����  ::= ���������壾;{���������壾;}
int varDeclare();

//���������壾  ::= �����ͱ�ʶ����(����ʶ����|����ʶ������[�����޷�����������]��){,(����ʶ����|����ʶ������[�����޷�����������]�� )}  //���޷�����������ʾ����Ԫ�صĸ�������ֵ�����0
int varDefine();

//<��������>	::= {���з���ֵ�������壾 | ���޷���ֵ�������壾}
int functionDefine();

//���з���ֵ�������壾  ::=  ������ͷ������(��������������)�� ��{����������䣾��}��|������ͷ������{����������䣾��}��  //��һ��ѡ��Ϊ�в�����������ڶ���ѡ��Ϊ�޲��������
int functionHasRetDefine();

//���޷���ֵ�������壾  ::= void����ʶ����(��������������)����{����������䣾��}��| void����ʶ����{����������䣾��}��//��һ��ѡ��Ϊ�в�����������ڶ���ѡ��Ϊ�޲��������
int functionNoRetDefine();

//��������䣾   ::=  �ۣ�����˵�����ݣۣ�����˵�����ݣ�����У�
int comStatement();

//����������    ::=  �����ͱ�ʶ��������ʶ����{,�����ͱ�ʶ��������ʶ����}
int paraTable();

//������ʽ��    ::= �ۣ������ݣ��{���ӷ�����������}  //[+|-]ֻ�����ڵ�һ��<��>
int expression();

//���     ::= �����ӣ�{���˷�������������ӣ�}
int item();

//�����ӣ�    ::= ����ʶ����������ʶ������[��������ʽ����]��|��(��������ʽ����)������������|���ַ��������з���ֵ����������䣾
int factor();

//����䣾    ::= ��������䣾����ѭ����䣾| ��{��������У���}��| ���з���ֵ����������䣾; | ���޷���ֵ����������䣾; ������ֵ��䣾; ��������䣾; ����д��䣾; �����գ�; | ��������䣾;
int sentence();

//��������䣾::= if ��(������������)������䣾[else����䣾]
int sentenceIf();

//��������    ::=  ������ʽ������ϵ�������������ʽ����������ʽ�� //����ʽΪ0����Ϊ�٣�����Ϊ��
int condition(char *lable);

//<�޷�������>  ::= �����֣��������֣���
int intUnsigned();

//��������        ::= �ۣ������ݣ��޷���������
int intCol();

//<whileѭ�����>	::= while ��(������������)������䣾
int sentenceWhile();

//<forѭ�����>		::= for'('����ʶ������������ʽ��;��������;����ʶ����������ʶ����(+|-)��������')'����䣾
int sentenceFor();

//��ֵ��������   ::= ������ʽ��{,������ʽ��}
int valueOfparaTable();

//������䣾    ::=  scanf ��(������ʶ����{,����ʶ����}��)��
int sentenceScanf();

//��д��䣾    ::= printf ��(�� ���ַ�����,������ʽ�� ��)��| printf ��(�����ַ����� ��)��| printf ��(��������ʽ����)��
int sentencePrintf();

//��������䣾   ::=  return[��(��������ʽ����)��]  
int sentenceReturn();

void genmidCodeFile();

void genopmidCodeFile();

