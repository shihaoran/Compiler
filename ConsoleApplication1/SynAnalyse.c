#include "LexAnalyse.h"
#include "symnum.h"
#include "error.h"
int sym;
int const_declaration()
{

}
int var_declaration()
{

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
		error(WRONG_HEAD);
	sym = getsym();
	compound_statement();
}
int compound_statement()//复合语句
{

}

int program()
{
	sym = getsym();
	if (sym == CONSTSYM)//常量说明
	{
		while (sym == CONSTSYM)
		{
			sym = getsym();
			const_declaration();
			if (sym != SEMICOLONSYM)
				error(MISSING_SEMICOLON);
			else sym = getsym();
		}
	}
	if (sym == INTSYM || sym == CHARSYM || sym == VOIDSYM)//TODO:需要循环
	{
		if (sym == VOIDSYM)
		{
			sym = getsym();
			if (sym == MAINSYM)
			{
				sym = getsym();
				main_func();
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