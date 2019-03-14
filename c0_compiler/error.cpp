#include "error.h"

int errornum = 1;

const char * errorCode[34] = {
	"Open file failed.",
	"viod main error.",
	"Undefined identifier.",
	"Repeated defination.",
	"Lack of small bracket.",
	"Lack of middle bracket.",
	"Lack of big bracket.",
	"Lack of comma.",
	"Lack of semicolon.",
	"Illegal char.",
	"Lack of assign symbol.",
	"Lack of function name.",
	"Lack of identifier.",
	"Unmatched singal quotes.",
	"Unmatched double quotes.",
	"Unmatched parament type.",
	"Illegal assign.",
	"Illegal defination",
	"Illegal parament",
	"Illegal operator in For",
	"Illegal operator in Condition",
	"Middle code is the largest number.",
	"Scanf error,you scaned an element of an array.",
	"Return error.",
	"Value of a return function missed.",
	"Relation operation error.",
	"Out of array bound.",
	"Error number after op.",
	"Factor error.",
	"Parament number error",
	"Condition factor type error",
	"Illegal char.",
	"Having brackets while calling a function without parament.",
	"Having brackets while defining a function without parament"
};

void error(int errorcode, int line)
{
	printf("Error_%d:  line:%d\n", errornum++, line);
	printf("======= %s\n", errorCode[errorcode]);
	printf("\n");
}