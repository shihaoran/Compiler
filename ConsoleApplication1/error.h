#pragma once

#ifndef ERROR_H
#define ERROR_H
enum errorname
{
	NO_ERROR = 0,
	UNKNOWN_SYMBOL = 1,
	STRING_TOO_LONG,
	WRONG_FORMAT_INTEGER,
	UNFINISHED_STRING,
	UNFINISHED_PROGRAM,//
	MISSING_IDENTIFIER,//
	MISSING_STRUCTURER_IN_FUNC_DEFINE,//
	DECLARATION_IS_NOT_START_WITH_TYPE,//
	MISSING_MAIN_FUNC,//
	DECLARATION_HAVE_NO_EQL,//
	DECLARATION_SHOULD_HAVE_A_ID,//
	WRONG_ASSIGN_SYNTAX,//
	WRONG_EXPRESSION,//
	WRONG_HEAD,//函数定义头部错误
	VAR_DECLARATION_AFTER_FUNC,//
	WRONG_STATEMENT,//
	MISSING_SEMICOLON,//
	ARRAY_OVERFLOW,
	ARRAY_SUBVALUE_SHOULD_BE_INTEGER,//
	USING_AN_ICON_WITHOUT_DECLARTION,
	ERROR_VARIABLEDELARTION,
	ERROR_PARAMETER,//
	ERROR_DATA_TYPE,
	ERROR_FUNCTIONDELARATION,
	RE_DECLARATION,
	ERROR_SENTENCE,
	ERROR_PROGRAM_STRUCTURE,
	ERROR_RETURN_TYPE,
	PARENT_DISMATCH,//
	BRACE_DISMATCH,//
	ERROR_FOR_SENTENCE,
	ERROR_IN_IF,//
	ERROR_IN_WHILE,//
	ERROR_IN_STATEMENT,//
	ERROR_IN_SCANF,//
	ERROR_IN_PRINTF,//
	ERROR_IN_SWITCH,//
	ERROR_IN_SWITCHTABLE,//
	ERROR_IN_DEFAULT,//
	ERROR_IN_RETURN,//
	ERROR_IN_EXPRESSION,//
	MISSING_WHILE,
	ERROR_CONDITION,
	TOO_MANY_CODE_AFTER_MAIN_FUNCTION,
	MISSING_SENTENCE,
	//语法分析错误
	DUPLICATE_DEFINE_IDENTIFIER,
	OP_OUT_OF_RANGE
};
int error(int errortype);
#endif
