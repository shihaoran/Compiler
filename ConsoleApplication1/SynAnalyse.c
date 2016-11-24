#include "LexAnalyse.h"
#include "SynAnalyse.h"
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
			return 0;
		}
		sym = getsym();
		if (sym != ASSIGNSYM)
		{
			error(DECLARATION_HAVE_NO_EQL);
			return 0;
		}
		sym = getsym();
		integer();
		while (sym == COMMASYM)
		{
			sym = getsym();
			if (sym != IDSYM)
			{
				error(DECLARATION_SHOULD_HAVE_A_ID);
				return 0;
			}
			sym = getsym();
			if (sym != ASSIGNSYM)
			{
				error(DECLARATION_HAVE_NO_EQL);
				return 0;
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
			return 0;
		}
		sym = getsym();
		if (sym != ASSIGNSYM)
		{
			error(DECLARATION_HAVE_NO_EQL);
			return 0;
		}
		sym = getsym();
		if (sym != CHSYM)
		{
			error(WRONG_ASSIGN_SYNTAX);
			return 0;
		}
		sym = getsym();
		while (sym == COMMASYM)
		{
			sym = getsym();
			if (sym != IDSYM)
			{
				error(DECLARATION_SHOULD_HAVE_A_ID);
				return 0;
			}
			sym = getsym();
			if (sym != ASSIGNSYM)
			{
				error(DECLARATION_HAVE_NO_EQL);
				return 0;
			}
			sym = getsym();
			if (sym != CHSYM)
			{
				error(WRONG_ASSIGN_SYNTAX);
				return 0;
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
	if (!head())
	{
		error(WRONG_HEAD);
		return 0;
	}
	if (sym == LMPARENSYM)
	{
		sym = getsym();
		if (sym != NUMSYM)
		{
			error(ARRAY_SUBVALUE_SHOULD_BE_INTEGER);
			return 0;
		}
		sym = getsym();
		if (sym != RMPARENSYM)
		{
			error(PARENT_DISMATCH);
			return 0;
		}
		sym = getsym();
	}
	if (!var_defination_backend())
	{
		return 0;
	}
	return 1;
}
int var_defination_backend()
{
	if (sym == SEMICOLONSYM)
	{
		return 1;
	}
	else if(sym!=COMMASYM)//如果是分号（空）或者逗号
	{
		error(ERROR_VARIABLEDELARTION);
		return 0;
	}
	while (sym == COMMASYM)
	{
		sym = getsym();
		if (sym != IDSYM)
		{
			error(ERROR_VARIABLEDELARTION);
			return 0;
		}
		sym = getsym();
		if (sym == LMPARENSYM)//数组
		{
			sym = getsym();
			if (sym != NUMSYM)
			{
				error(ARRAY_SUBVALUE_SHOULD_BE_INTEGER);
				return 0;
			}
			sym = getsym();
			if (sym != RMPARENSYM)
			{
				error(PARENT_DISMATCH);
				return 0;
			}
			sym = getsym();
		}
	}
	return 1;
}
int var_declaration()
{
	while (sym == INTSYM||sym==CHARSYM)
	{
		var_defination();
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);//TODO:退出
			return 0;
		}
		else sym = getsym();
	}
	return 1;
}
int integer()
{
	if (sym == PLUSSYM || sym == MINUSSYM)
	{
		sym = getsym();
	}
	else if (sym == ZEROSYM)
	{
		sym = getsym();
		return 1;
	}
	if (sym != NUMSYM)
	{
		error(WRONG_FORMAT_INTEGER);
		return 0;
	}
	sym = getsym();
	return 1;
}
int head()
{
	if (sym == INTSYM || sym == CHARSYM)
	{
		sym = getsym();
		if (sym == IDSYM)
		{
			sym = getsym();
			return 1;
		}
		else
		{
			error(WRONG_HEAD);
			return 0;
		}
	}
	else
	{
		error(WRONG_HEAD);
		return 0;
	}
}
int value_parameter_table()
{
	if (sym == RPARENSYM)
	{
		return 1;
	}
	else
	{
		if (!expression())
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		while (sym==COMMASYM)
		{
			sym = getsym();
			if (!expression())
			{
				error(ERROR_PARAMETER);
				return 0;
			}
		}
		return 1;
	}
}
int parameter_table()
{
	if (sym == INTSYM || sym == CHARSYM)
	{
		sym = getsym();
		if (sym != IDSYM)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		sym = getsym();
		while (sym == COMMASYM)//处理逗号
		{
			sym = getsym();
			if (sym != INTSYM && sym != CHARSYM)
			{
				error(ERROR_PARAMETER);
				return 0;
			}
			sym = getsym();
			if (sym != IDSYM)
			{
				error(ERROR_PARAMETER);
				return 0;
			}
			sym = getsym();
		}
		if (sym != RPARENSYM)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		return 1;
	}
	else if(sym == RPARENSYM)//为空
	{
		return 1;
	}
	else
	{
		error(ERROR_PARAMETER);
		return 0;
	}
}
int void_func_defination()
{
	if (sym != IDSYM)
	{
		error(MISSING_IDENTIFIER);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(ERROR_PARAMETER);
		return 0;
	}
	sym = getsym();
	if (!parameter_table())
	{
		return 0;
	}
	if(sym!=RPARENSYM)
	{
		error(ERROR_PARAMETER);
		return 0;
	}
	sym = getsym();
	if (sym != LBPARENSYM)
	{
		error(MISSING_STRUCTURER_IN_FUNC_DEFINE);
		return 0;
	}
	sym = getsym();
	if (!compound_statement())
	{
		return 0;
	}
	if (sym != RBPARENSYM)
	{
		error(BRACE_DISMATCH);
		return 0;
	}
	sym = getsym();
	return 1;
}
int return_func_defination()
{
	if (!head())
		return 0;
	if (!return_func_defination_backend())
		return 0;
	return 1;
}
int return_func_defination_backend()
{
	if (sym != LPARENSYM)
	{
		error(ERROR_PARAMETER);
		return 0;
	}
	sym = getsym();
	parameter_table();
	if (sym != RPARENSYM)
	{
		error(ERROR_PARAMETER);
		return 0;
	}
	sym = getsym();
	if (sym != LBPARENSYM)
	{
		error(MISSING_STRUCTURER_IN_FUNC_DEFINE);
		return 0;
	}
	sym = getsym();
	compound_statement();
	if(sym!=RBPARENSYM)
	{
		error(BRACE_DISMATCH);
		return 0;
	}
	sym = getsym();
	return 1;
}
int main_func()//主函数,void main前面判断过了
{
	if (sym != LPARENSYM)
	{
		error(WRONG_HEAD);
		return 0;
	}
	sym = getsym();
	if (sym != RPARENSYM)
	{
		error(WRONG_HEAD);
		return 0;
	}
	sym = getsym();
	if (sym != LBPARENSYM)
	{
		error(MISSING_STRUCTURER_IN_FUNC_DEFINE);
		return 0;
	}	
	sym = getsym();
	compound_statement();
	if (sym != RBPARENSYM)
	{
		error(BRACE_DISMATCH);
		return 0;
	}
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
		if (!if_statement())
		{
			return 0;
		}
		return 1;
	}
	else if (sym == WHILESYM)
	{
		if (!while_statement())
		{
			return 0;
		}
		return 1;
	}
	else if (sym == LBPARENSYM)//TODO：检查一下
	{
		sym = getsym();
		while (statement()) {}//因为可能为空，不应报错
		if (sym != RBPARENSYM)
		{
			error(BRACE_DISMATCH);
			return 0;
		}
		sym = getsym();
		return 1;
	}
	else if (sym == IDSYM)
	{
		sym = getsym();
		//TODO:有无返回值完全相同 这里不写函数
		//函数调用语句
		if (sym == LPARENSYM)
		{
			if (!parameter_table())
			{
				return 0;
			}
			if (sym != RPARENSYM)
			{
				error(PARENT_DISMATCH);
				return 0;
			}
			sym = getsym();
			if (sym != SEMICOLONSYM)
			{
				error(MISSING_SEMICOLON);
				return 0;
			}
		}
		else if (sym == ASSIGNSYM)//赋值语句1
		{
			sym = getsym();
			if (!expression())
			{
				return 0;
			}
			if (sym != SEMICOLONSYM)
			{
				error(MISSING_SEMICOLON);
				return 0;
			}
		}
		else if (sym == LMPARENSYM)//赋值语句数组
		{
			sym = getsym();
			if (!expression())
			{
				return 0;
			}
			if (sym != RMPARENSYM)
			{
				error(PARENT_DISMATCH);
				return 0;
			}
			sym = getsym();
			if (sym != ASSIGNSYM)
			{
				error(WRONG_ASSIGN_SYNTAX);
				return 0;
			}
			sym = getsym();
			if (!expression())
			{
				return 0;
			}
			if (sym != SEMICOLONSYM)
			{
				error(MISSING_SEMICOLON);
				return 0;
			}
		}
		else
		{
			error(ERROR_IN_STATEMENT);
			return 0;
		}
		sym = getsym();
		return 1;
	}
	else if (sym == SCANFSYM)
	{
		if (!scanf_statement())
		{
			return 0;
		}
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
			return 0;
		}
		sym = getsym();
		return 1;
	}
	else if (sym == PRINTFSYM)
	{
		if (!printf_statement())
		{
			return 0;
		}
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
			return 0;
		}
		sym = getsym();
		return 1;
	}
	else if (sym == SWITCHSYM)
	{
		if (!switch_statement())
		{
			return 0;
		}
		return 1;
	}
	else if (sym == RETURNSYM)
	{
		if (!switch_statement())
		{
			return 0;
		}
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
			return 0;
		}
		sym = getsym();
		return 1;
	}
	else if (sym == SEMICOLONSYM)
	{
		sym = getsym();
		return 1;
	}
	else
	{
		error(WRONG_STATEMENT);
		return 0;
	}
}
int if_statement()
{
	if (sym != IFSYM)
	{
		error(ERROR_IN_IF);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(ERROR_IN_IF);
		return 0;
	}
	sym = getsym();
	if (!condition_statement())
	{
		return 0;
	}
	if (sym != RPARENSYM)
	{
		error(ERROR_IN_IF);
		return 0;
	}
	sym = getsym();
	if (!statement())
	{
		return 0;
	}
	if (sym == ELSESYM)
	{
		if (!statement())
		{
			return 0;
		}
	}
	return 1;
}
int while_statement()
{
	if (sym != WHILESYM)
	{
		error(ERROR_IN_WHILE);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(ERROR_IN_WHILE);
		return 0;
	}
	sym = getsym();
	if (!condition_statement())
	{
		return 0;
	}
	if (sym != RPARENSYM)
	{
		error(ERROR_IN_WHILE);
		return 0;
	}
	sym = getsym();
	if (!statement())
	{
		return 0;
	}
	return 1;
}
int condition_statement()
{
	if (!expression())
	{
		return 0;
	}
	if (sym == BIGTHSYM||sym == SMALLTHSYM||
		sym == NOTBTHSYM||sym == NOTSTHSYM||
		sym == EQLSYM||sym == NOTESYM)
	{
		sym = getsym();
		if (!expression())
		{
			error(WRONG_EXPRESSION);
		}
	}
	return 1;
}
int scanf_statement()
{
	if (sym != SCANFSYM)
	{
		error(ERROR_IN_SCANF);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(ERROR_IN_SCANF);
		return 0;
	}
	sym = getsym();
	if(sym!=IDSYM)
	{
		error(ERROR_IN_SCANF);
		return 0;
	}
	sym = getsym();
	while (sym == COMMASYM)
	{
		sym = getsym();
		if (sym != IDSYM)
		{
			error(ERROR_IN_SCANF);
			return 0;
		}
		sym = getsym();
	}
	if(sym!=RPARENSYM)
	{
		error(ERROR_IN_SCANF);
		return 0;
	}
	sym = getsym();
	return 1;
}
int printf_statement()
{
	if (sym != PRINTFSYM)
	{
		error(ERROR_IN_PRINTF);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(ERROR_IN_PRINTF);
		return 0;
	}
	sym = getsym();
	if (sym == STRSYM)
	{
		sym = getsym();
		if (sym == COMMASYM)//字符串 表达式
		{
			sym = getsym();
			if (!expression())
			{
				error(ERROR_IN_PRINTF);//想想是不是要保留
				return 0;
			}
		}
		else if (sym == RPARENSYM) {}//仅字符串
		else
		{
			error(ERROR_IN_PRINTF);
			return 0;
		}
	}
	else//仅表达式
	{
		if (!expression())
		{
			error(ERROR_IN_PRINTF);//想想是不是要保留
			return 0;
		}
	}
	if (sym != RPARENSYM)
	{
		error(ERROR_IN_PRINTF);
		return 0;
	}
	sym = getsym();
	return 1;
}
int switch_statement()
{
	if (sym != SWITCHSYM)
	{
		error(ERROR_IN_SWITCH);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(ERROR_IN_SWITCH);
		return 0;
	}
	sym = getsym();
	if (!expression())
	{
		return 0;
	}
	if (sym != RPARENSYM)
	{
		error(PARENT_DISMATCH);
		return 0;
	}
	sym = getsym();
	if (sym != LBPARENSYM)
	{
		error(ERROR_IN_SWITCH);
		return 0;
	}
	sym = getsym();
	if (!switch_table())
	{
		return 0;
	}
	if (sym == DEFAULTSYM)//有缺省
	{
		if (!default_statement())
		{
			return 0;
		}
	}
	if (sym != RBPARENSYM)
	{
		error(ERROR_IN_SWITCH);
		return 0;
	}
	sym = getsym();
	return 1;
}
int switch_table()
{
	if (sym != CASESYM)
	{
		error(ERROR_IN_SWITCHTABLE);
		return 0;
	}
	sym = getsym();//下面判断一个常量
	if (sym == CHSYM)
	{
		sym = getsym();
	}
	else if (integer()) {}
	else
	{
		error(ERROR_IN_SWITCHTABLE);
		return 0;
	}
	if (sym != COLONSYM)
	{
		error(ERROR_IN_SWITCHTABLE);
		return 0;
	}
	sym = getsym();
	if (!statement())
	{
		return 0;
	}
	while (sym == CASESYM)
	{
		sym = getsym();//下面判断一个常量
		if (sym == CHSYM)
		{
			sym = getsym();
		}
		else if (integer()) {}
		else
		{
			error(ERROR_IN_SWITCHTABLE);
			return 0;
		}
		if (sym != COLONSYM)
		{
			error(ERROR_IN_SWITCHTABLE);
			return 0;
		}
		sym = getsym();
		if (!statement())
		{
			return 0;
		}
	}
	return 1;
}
int default_statement()
{
	if (sym != DEFAULTSYM)
	{
		error(ERROR_IN_DEFAULT);
		return 0;
	}
	sym = getsym();
	if (sym != COLONSYM)
	{
		error(ERROR_IN_DEFAULT);
		return 0;
	}
	if (!statement())
	{
		return 0;
	}
	return 1;
}
int return_statement()
{
	if (sym != RETURNSYM)
	{
		error(ERROR_IN_RETURN);
		return 0;
	}
	sym = getsym();
	if (sym == LPARENSYM)
	{
		sym = getsym();
		if (!expression())
		{
			return 0;
		}
		if (sym != RPARENSYM)
		{
			error(PARENT_DISMATCH);
			return 0;
		}
	}//TODO:统一结束处理
	sym = getsym();
	return 1;
}
int expression()//表达式
{
	if (sym == PLUSSYM || sym == MINUSSYM)
	{
		sym = getsym();
	}
	if (!item())
	{
		return 0;
	}
	while (sym == PLUSSYM || sym == MINUSSYM)
	{
		sym = getsym();
		if (!item())
		{
			return 0;
		}
	}
	return 1;
}
int item()//项
{
	if (!factor())
	{
		return 0;
	}
	while (sym == TIMESSYM || sym == DIVSYM)
	{
		sym = getsym();
		if (!factor())
		{
			return 0;
		}
	}
	return 1;
}
int factor()//因子
{
	if (sym == IDSYM)//标识符，数组，有返回值函数调用
	{
		sym = getsym();
		if (sym == LMPARENSYM)//数组
		{
			sym = getsym();
			if (!expression())
			{
				return 0;
			}
			if (sym != RMPARENSYM)
			{
				error(PARENT_DISMATCH);
				return 0;
			}
			sym = getsym();
		}
		else if (sym == LPARENSYM)//有返回值函数调用
		{
			sym = getsym();
			if (!value_parameter_table())
			{
				return 0;
			}
			if (sym != RPARENSYM)
			{
				error(PARENT_DISMATCH);
				return 0;
			}
			sym = getsym();
		}
		return 1;
	}
	else if (sym == PLUSSYM || sym == MINUSSYM || sym == NUMSYM || sym == ZEROSYM)
	{
		if (!integer())
		{
			return 0;
		}
		return 1;
	}
	else if (sym == CHSYM)
	{
		sym = getsym();
		return 1;
	}
	else if (sym == LPARENSYM)
	{
		sym = getsym();
		if (!expression())
		{
			return 0;
		}
		if (sym != RPARENSYM)
		{
			error(PARENT_DISMATCH);
			return 0;
		}
		sym = getsym();
		return 1;
	}
	else
	{
		error(ERROR_IN_EXPRESSION);
		return 0;
	}
}
int program()
{
	int main_cnt = 0;
	int var_flag = 0;//在主程序中的函数定义后不应该有变量说明
	sym = getsym();
	if (sym == CONSTSYM)//常量说明
	{
		const_declaration();
	}
	while (sym == INTSYM || sym == CHARSYM || sym == VOIDSYM)//TODO:需要循环
	{
		if (main_cnt != 0)
		{
			error(TOO_MANY_CODE_AFTER_MAIN_FUNCTION);
			return 0;
		}
		if (sym == VOIDSYM)
		{
			sym = getsym();
			if (sym == MAINSYM)
			{
				sym = getsym();
				main_func();//从(开始进入
				main_cnt++;//不退出继续
				continue;
			}
			else if (sym == IDSYM)
			{
				void_func_defination();//从标识符开始进入
				var_flag = 1;
				continue;
			}
			else
			{
				error(WRONG_HEAD);
				return 0;
			}
		}
		else//int char
		{
			head();
			if (sym == LMPARENSYM)//数组
			{
				if (var_flag == 1)
				{
					error(VAR_DECLARATION_AFTER_FUNC);
					return 0;
				}
				sym = getsym();
				if (sym != NUMSYM)
				{
					error(ARRAY_SUBVALUE_SHOULD_BE_INTEGER);
					return 0;
				}
				sym = getsym();
				if (sym != RMPARENSYM)
				{
					error(PARENT_DISMATCH);
					return 0;
				}
				sym = getsym();
				if (sym == SEMICOLONSYM)
				{
					//TODO：这里打印
					sym = getsym();
					continue;
				}
				else if (sym == COMMASYM)
				{
					var_defination_backend();//说明一行有多个定义
					if (sym == SEMICOLONSYM)
					{
						//TODO：这里打印
						sym = getsym();
						continue;
					}
				}
			}
			else if (sym == COMMASYM)//变量定义后部
			{
				if (var_flag == 1)
				{
					error(VAR_DECLARATION_AFTER_FUNC);
					return 0;
				}
				var_defination_backend();//说明一行有多个定义
				if (sym == SEMICOLONSYM)
				{
					//TODO：这里打印
					sym = getsym();
					continue;
				}
			}
			else if (sym == SEMICOLONSYM)
			{
				if (var_flag == 1)
				{
					error(VAR_DECLARATION_AFTER_FUNC);
					return 0;
				}
				//TODO：这里打印
				sym = getsym();
				continue;
			}
			else if (sym == LPARENSYM)//说明是有返回值函数定义
			{
				return_func_defination_backend();
				var_flag = 1;
			}
			else
			{
				error(MISSING_SEMICOLON);
				return 0;
			}
		}
	}
	if (main_cnt == 0)
		error(MISSING_MAIN_FUNC);
	if (sym != EOF)
	{
		error(UNFINISHED_PROGRAM);
		return 0;
	}
	return 1;
}





int main()
{
	int result,i=0;
	init();
	program();
	scanf("%d", &result);
}