#include "syntax_analyse.h"

int main()
{
	char filepath[MAX_FILEPATH_NAME];
	printf("Please input the path of the txt file:\n");
	scanf("%s", filepath);
	readfile(filepath);

	compileBegin();

	printf("FINISH!\n");
	return 0;
}
