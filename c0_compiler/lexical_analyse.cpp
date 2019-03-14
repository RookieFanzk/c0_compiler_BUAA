#include "lexical_analyse.h"
#include "error.h"

const char * keywords_array[KEY_NUM] = {
	"const", "int", "char", "void", "main", "if", "else", "while", "for", "scanf", "printf", "return"
};

const char * symbolwords_array[SYMBOL_NUM] = {
	"CONSTSY", "INTSY", "CHARSY", "VOIDSY", "MAINSY", "IFSY", "ELSESY", "WHILESY", "FORSY", "SCANFSY", "PRINTFSY", "RETURNSY",
	"PLUS", "MINUS", "TIMES", "DIV", "CHARCOL", "UNSIGNEDINT",
	"LESS", "LESSEQ", "MORE", "MOREEQ", "NOTEQ", "EQUAL","ASSIGN",
	"LSBRACKET", "RSBRACKET", "LMBRACKET", "RMBRACKET", "LBBRACKET", "RBBRACKET",
	"COMMA", "SEMICOLON", "COLON", "BACKSLASH",
	"IDEN","STRING"
};

char filecontents[MAX_FILE_CHAR];		//存储文件的全部字符

int file_length = 0;					//文件长度
int fileindex = -1;						//当前行指针
int linecount = 1;						//当前行数	

char current_char;						//当前读入的字符
Word word;								//当前单词
int hasAsym = false;					//判断是否已经有了一个单词

int file_finish = false;

void readfile(char *filepath);

void tolowercase(char string[]);		//标识符全部转化为小写

int reserver(char string[]);			//查保留字表

void readfile(char *filepath)
{
	FILE *fp;
	if ((fp = fopen(filepath, "r")) == NULL) {
		error(0,0);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);			//定位到文件尾部，以得到文件大小
	file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	int i = 0;
	char ch;
	while (!feof(fp)) {				//读取文件的全部内容，存在filecontents中
		ch = fgetc(fp);
		if (ch == EOF)
			break;
		filecontents[i] = ch;
		i++;
	}

	fclose(fp);
}

char nextch()
{
	if (fileindex >= file_length)
		return EOF;
	
	return filecontents[++fileindex];							//逐个读取字符
}

void tolowercase(char string[])	
{
	int i = 0;
	for (i = 0; i < strlen(string); i++) {
		if (string[i] > 64 && string[i] < 91) {
			string[i] = string[i] + 32;
		}
	}
}

int reserver(char string[])									//查保留字表
{
	int i = 0;
	for (i = 0; i < KEY_NUM; i++) {
		if (strcmp(string, keywords_array[i]) == 0) {
			return i;										//找到了返回下表
		}
	}

	return -1;												//没找到返回-1
}

void nextsym()
{
	if (hasAsym) {
		hasAsym = false;
		return;
	}

	if (fileindex > file_length) {
		return;
	}

	int word_index = 0;
	char wordArray[1000] = {'\0'};
	current_char = nextch();
	while (current_char == ' ' || current_char == '\n' || current_char == '\t') {
		if (current_char == '\n')
			linecount++;
		current_char = nextch();
	}

	if (current_char == '\0' || current_char == EOF) {
		file_finish = true;
		return;
	}

	if (current_char == '_' || isalpha(current_char)) {
		while (current_char == '_' || isalpha(current_char) || isalnum(current_char)) {
			if (word_index < MAX_WORD_CHAR) {
				wordArray[word_index] = current_char;
				word_index++;
				current_char = nextch();
			}
		}
		tolowercase(wordArray);
		if (current_char != EOF)
			retract();
		int Areserver = reserver(wordArray);
		if (Areserver >= 0) {
			word.current_sy = (symboltype)Areserver;
		}
		else {
			word.current_sy = IDEN;
		}
		strcpy(word.current_str, wordArray);
	}
	else if (isalnum(current_char)) {
		while (isalnum(current_char)) {
			wordArray[word_index] = current_char;
			word_index++;
			current_char = nextch();
		}
		if (current_char != EOF)
			retract();
		word.current_sy = UNSIGNEDINT;
		char int_to_str[20];
		itoa(atoi(wordArray), int_to_str, 10);
		strcpy(word.current_str, int_to_str);
	}
	else if (current_char == '+') {
		wordArray[word_index] = current_char;
		word.current_sy = PLUS;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '-') {
		wordArray[word_index] = current_char;
		word.current_sy = MINUS;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '*') {
		wordArray[word_index] = current_char;
		word.current_sy = TIMES;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '/') {
		wordArray[word_index] = current_char;
		word.current_sy = DIV;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '<') {
		wordArray[word_index] = current_char;
		word_index++;
		current_char = nextch();
		if (current_char == '=') {
			wordArray[word_index] = current_char;
			word.current_sy = LESSEQ;
		}
		else {
			if (current_char != EOF)
				retract();
			word.current_sy = LESS;
		}
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '>') {
		wordArray[word_index] = current_char;
		word_index++;
		current_char = nextch();
		if (current_char == '=') {
			wordArray[word_index] = current_char;
			word.current_sy = MOREEQ;
		}
		else {
			if (current_char != EOF)
				retract();
			word.current_sy = MORE;
		}
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '!') {
		wordArray[word_index] = current_char;
		word_index++;
		current_char = nextch();
		if (current_char == '=') {
			wordArray[word_index] = current_char;
			word.current_sy = NOTEQ;
		}
		else {
			if (current_char != EOF)
				retract();
			error(9,linecount);
			nextsym();
			return;
		}
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '=') {
		wordArray[word_index] = current_char;
		word_index++;
		current_char = nextch();
		if (current_char == '=') {
			wordArray[word_index] = current_char;
			word.current_sy = EQUAL;
		}
		else {
			if (current_char != EOF)
				retract();
			word.current_sy = ASSIGN;
		}
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '(') {
		wordArray[word_index] = current_char;
		word.current_sy = LSBRACKET;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '(') {
		wordArray[word_index] = current_char;
		word.current_sy = LSBRACKET;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == ')') {
		wordArray[word_index] = current_char;
		word.current_sy = RSBRACKET;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '[') {
		wordArray[word_index] = current_char;
		word.current_sy = LMBRACKET;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == ']') {
		wordArray[word_index] = current_char;
		word.current_sy = RMBRACKET;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '{') {
		wordArray[word_index] = current_char;
		word.current_sy = LBBRACKET;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '}') {
		wordArray[word_index] = current_char;
		word.current_sy = RBBRACKET;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == ',') {
		wordArray[word_index] = current_char;
		word.current_sy = COMMA;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == ';') {
		wordArray[word_index] = current_char;
		word.current_sy = SEMICOLON;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == ':') {
		wordArray[word_index] = current_char;
		word.current_sy = COLON;
		strcpy(word.current_str, wordArray);
	}
	else if (current_char == '\'') {
		current_char = nextch();
		if (current_char == '_' || current_char == '+' || current_char == '-' || current_char == '*' || current_char == '/' || 
			(current_char >= 'a' && current_char <= 'z') || 
			(current_char >= 'A' && current_char <= 'Z') || (current_char >= '0' && current_char <= '9')) {
			wordArray[word_index] = current_char;
			current_char = nextch();
			if (current_char == '\'') {
				word.current_sy = CHARCOL;
				strcpy(word.current_str, wordArray);
			}
			else {
				error(13, linecount);
				retract();
				nextsym();
				return;
			}
		}
		else {
			error(31, linecount);
			return;
		}
	}
	else if (current_char == '"') {
		current_char = nextch();
		while (current_char == 32 || current_char == 33 || (current_char >= 35 && current_char <= 126)) {
			if (current_char == '\\') {
				wordArray[word_index] = current_char;
				word_index++;
				wordArray[word_index] = '\\';
				word_index++;
			}
			else {
				wordArray[word_index] = current_char;
				word_index++;
			}
			current_char = nextch();
		}
		if (current_char == '"') {
			word.current_sy = STRING;
			strcpy(word.current_str, wordArray);
		}
		else {
			error(14, linecount);
			retract();
			nextsym();
			return;
		}
	}
	else {
		error(31, linecount);
		nextsym();
	}

	//printf("%s\n", word.current_str);
}

void skip(char endskip)
{
	Word skipsym;
	skipsym = getsymWord();
	while (skipsym.current_str[0] != endskip) {
		nextsym();
		skipsym = getsymWord();
	}
}

void sethasAsym()
{
	hasAsym = true;
}

void retract()
{
	fileindex--;
}

Word getsymWord()
{
	return word;
}
