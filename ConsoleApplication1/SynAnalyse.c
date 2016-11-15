#include "LexAnalyse.h"
#include "symnum.h"
#include "error.h"
int sym;
int const_defination()//常量定义
{
	if (sym == INTSYM)
	{
		sym = getsym();
		if (sym != IDSYM)
		{
			error(DECLARATION_SHOULD_HAVE_A_ID);
		}
		sym = getsym();
		if (sym != ASSIGNSYM)
		{
			error(DECLARATION_HAVE_NO_EQL);
		}
		sym = getsym();
		integer();
		while (sym == COMMASYM)
		{
			sym = getsym();
			if (sym != IDSYM)
			{
				error(DECLARATION_SHOULD_HAVE_A_ID);
			}
			sym = getsym();
			if (sym != ASSIGNSYM)
			{
				error(DECLARATION_HAVE_NO_EQL);
			}
			sym = getsym();
			integer();
		}
		return 1;
	}
	else if (sym==CHARSYM)
	{
		sym = getsym();
		if (sym != IDSYM)
		{
			error(DECLARATION_SHOULD_HAVE_A_ID);
		}
		sym = getsym();
		if (sym != ASSIGNSYM)
		{
			error(DECLARATION_HAVE_NO_EQL);
		}
		sym = getsym();
		if (sym != CHSYM)
		{
			error(WRONG_ASSIGN_SYNTAX);
		}
		sym = getsym();
		while (sym == COMMASYM)
		{
			sym = getsym();
			if (sym != IDSYM)
			{
				error(DECLARATION_SHOULD_HAVE_A_ID);
			}
			sym = getsym();
			if (sym != ASSIGNSYM)
			{
				error(DECLARATION_HAVE_NO_EQL);
			}
			sym = getsym();
			if (sym != CHSYM)
			{
				error(WRONG_ASSIGN_SYNTAX);
			}
			sym = getsym();
		}
		return 1;
	}
	else
	{
		error(DECLARATION_IS_NOT_START_WITH_TYPE);
		return 0;
	}
}
int const_declaration()
{
	while (sym == CONSTSYM)
	{
		sym = getsym();
		const_defination();
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);//TODO:退出
			return 0;
		}
		else sym = getsym();
	}
	return 1;
}
int var_defination()
{

}
int var_declaration()
{

}
int integer()
{
	if (sym == PLUSSYM || sym == MINUSSYM)
	{
		sym = getsym();
		if (sym != NUMSYM)
		{
			error(WRONG_FORMAT_INTEGER);
			return 0;
		}
		sym = getsym();

	}
	else if (sym == ZEROSYM)
	{
		sym = getsym();
		return 1;
	}
	else
	{
		error(WRONG_FORMAT_INTEGER);
		return 0;
	}
}
int void_func_defination()
{

}
int main_func()//主函数
{
	if (sym != LPARENSYM)
		error(WRONG_HEAD);
	sym = getsym();
	if (sym != RPARENSYM)
		error(WRONG_HEAD);
	sym = getsym();
	if (sym != LBPARENSYM)
		error(PARENT_DISMATCH);
	sym = getsym();
	compound_statement();
	if (sym != RBPARENSYM)
		error(PARENT_DISMATCH);
	sym = getsym();
	return 1;
}
int compound_statement()//复合语句
{
	if (sym == CONSTSYM)
	{
		const_declaration();
	}
	if (sym == INTSYM || sym == CHARSYM)
	{
		var_declaration();
	}
	while (statement()) {}
	if (sym != RBPARENSYM)
	{
		error(WRONG_STATEMENT);
		return 0;
	}
	return 1;
}
int statement()
{
	if (sym == IFSYM)
	{

	}
	else if (sym == WHILESYM)
	{

	}
	else if (sym == LBPARENSYM)
	{

	}
	else if (sym == IDSYM)
	{
		//TODO:有无返回值完全相同 这里不写函数
	}
	else if (sym == SCANFSYM)
	{

	}
	else if (sym == PRINTFSYM)
	{

	}
	else if (sym == SWITCHSYM)
	{

	}
	else if (sym == RETURNSYM)
	{

	}
	else if (sym == SEMICOLONSYM)
	{

	}
	else
	{
		error(WRONG_STATEMENT);
		return 0;
	}
}
int program()
{
	sym = getsym();
	if (sym == CONSTSYM)//常量说明
	{
		const_declaration();
	}
	if (sym == INTSYM || sym == CHARSYM || sym == VOIDSYM)//TODO:需要循环
	{
		if (sym == VOIDSYM)
		{
			sym = getsym();
			if (sym == MAINSYM)
			{
				sym = getsym();
				main_func();//从(开始进入
			}
			else if (sym == IDSYM)
			{
				void_func_defination();
			}
			else
			{
				error(WRONG_HEAD);
			}
		}
		else
		{
			error(MISSING_IDENTIFIER);
		}
		var_declaration(&sym);;
		while (sym == SEMICOLONSYM)
			sym = getsym();
	}
	if (sym == VOIDSYM)
	{

	}
	else
	{
		error(WRONG_HEAD);
	}
}





int main()
{
	int result,i=0;
	printf("jell%d  ",line);
	init();
	result = getsym();
	while (result != EOF)
	{
		printresult(result, i);
		i++;
		result = getsym();
	}
	scanf("%s", &result);
}