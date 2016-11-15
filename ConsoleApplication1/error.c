#include<stdio.h>
#include"error.h"
extern int linenum;
int errnum = 0;
int error(int errortype)
{
	errnum++;
	printf("Error %d:C%d ", errnum, errortype);
	switch (errortype)
	{
	case UNKNOWN_SYMBOL:
		printf( "unknown symbol in line"); break;
	case STRING_TOO_LONG:
		printf( "string is too long"); break;
	case WRONG_FORMAT_REAL_NUMBER:
		printf( "wrong format real number"); break;
	case WRONG_FORMAT_INTEGER:
		printf( "wrong format integer"); break;
	case UNFINISHED_STRING:
		printf( "\'\"\'dismatch"); break;
	case UNFINISHED_PROGRAM:
		printf( "the program is not finished"); break;
	case MISSING_SEMICN:
		printf( "sentence misses semicn"); break;
	case DECLARATION_IS_NOT_START_WITH_TYPE:
		printf( "declartion is not start with type"); break;
	case DECLARATION_HAVE_NO_EQL:
		printf( "wrong format of constant declartion"); break;
	case DECLARATION_SHOULD_HAVE_A_ID:
		printf( "declartion should have a id"); break;
	case WRONG_ASSIGN_SYNTAX:
		printf( "should not use assign symbol"); break;
	case WRONG_EXPRESSION:
		printf( "wrong expression"); break;
	case ARRAY_OVERFLOW:
		printf( "array overflow"); break;
	case ARRAY_SUBVALUE_SHOULD_BE_INTEGER:
		printf( "array subvalue should be an integer"); break;
	case USING_AN_ICON_WITHOUT_DECLARTION:
		printf( "non-declartion variable"); break;
	case ERROR_VARIABLEDELARTION:
		printf( "error variable decleartion"); break;
	case ERROR_DATA_TYPE:
		printf( "error data type"); break;
	case ERROR_FUNCTIONDELARATION:
		printf( "error function declartion"); break;
	case RE_DECLARATION:
		printf( "re-declartion"); break;
	case ERROR_SENTENCE:
		printf( "error format sentence"); break;
	case ERROR_RETURN_TYPE:
		printf( "error return type"); break;
	case PARENT_DISMATCH:
		printf( "parent dismatch"); break;
	case ERROR_FOR_SENTENCE:
		printf( "for sentence error"); break;
	case ERROR_PROGRAM_STRUCTURE:
		printf( "error program structure"); break;
	case ERROR_IN_WRITE_SENTENCE:
		printf( "error in write sentence"); break;
	case ERROR_IN_READ_SENTENCE:
		printf( "error int read sentence"); break;
	case MISSING_WHILE:
		printf( "miss while condition"); break;
	case MISSING_SENTENCE:
		printf( "miss sentence"); break;
	}
	printf( " in line %d\n", linenum + 1);
	return errortype;
};