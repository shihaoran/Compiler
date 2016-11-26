#include<stdio.h>
#include"error.h"
#include"LexAnalyse.h"

extern int line;
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
	case MISSING_IDENTIFIER:
		printf( "missing indentifier"); break;
	case WRONG_FORMAT_INTEGER:
		printf( "wrong format integer"); break;
	case UNFINISHED_STRING:
		printf( "\'\"\'dismatch"); break;
	case UNFINISHED_PROGRAM:
		printf( "the program is not finished"); break;
	case MISSING_STRUCTURER_IN_FUNC_DEFINE:
		printf( "missing structurer in function define"); break;
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
	case MISSING_MAIN_FUNC:
		printf( "missing main function"); break;
	case WRONG_HEAD:
		printf( "wrong head"); break;
	case MISSING_WHILE:
		printf( "miss while condition"); break;
	case MISSING_SENTENCE:
		printf( "miss sentence"); break;
	case VAR_DECLARATION_AFTER_FUNC:
		printf("var declaration after function"); break;
	case WRONG_STATEMENT:
		printf("wrong statement"); break;
	case MISSING_SEMICOLON:
		printf("miss semicolon"); break;
	case ERROR_PARAMETER:
		printf("error parameter"); break;
	case BRACE_DISMATCH:
		printf("brace dismatch"); break;
	case ERROR_IN_IF:
		printf("error in if"); break;
	case ERROR_IN_WHILE:
		printf("error in while"); break;
	case ERROR_IN_STATEMENT:
		printf("error in statement"); break;
	case ERROR_IN_SCANF:
		printf("error in scanf"); break;
	case ERROR_IN_PRINTF:
		printf("error in printf"); break;
	case ERROR_IN_SWITCH:
		printf("error in switch"); break;
	case ERROR_IN_SWITCHTABLE:
		printf("error in switch table"); break;
	case ERROR_IN_DEFAULT:
		printf("error in default"); break;
	case ERROR_IN_RETURN:
		printf("error in return"); break;
	case DUPLICATE_DEFINE_IDENTIFIER:
		printf("duplicate define identifier"); break;
	case NONE_DEFINE_IDENTIFIER:
		printf("none define identifier"); break;
	case THIS_IS_NOT_ARRAY:
		printf("this is not array"); break;
	case THIS_IS_NOT_FUNC:
		printf("this is not function"); break;
	case IDENTIFIER_TYPE_DISMATCH:
		printf("identifier type dismatch"); break;
	case OP_OUT_OF_RANGE:
		printf("op out of range"); break;
	}
	printf( " in line %d\n", line + 1);
	return errortype;
};