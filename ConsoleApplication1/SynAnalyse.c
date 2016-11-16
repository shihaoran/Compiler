#include "LexAnalyse.h"
#include "symnum.h"
#include "error.h"
int sym;
int const_defination()//��������
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
			error(MISSING_SEMICOLON);//TODO:�˳�
			return 0;
		}
		else sym = getsym();
	}
	return 1;
}
int var_defination()
{

}
int var_defination_backend()
{

}
int var_declaration()
{
	while (sym == INTSYM||sym==CHARSYM)
	{
		var_defination();
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);//TODO:�˳�
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
int parameter_table()
{
	if (sym == INTSYM || sym == CHARSYM)
	{
		sym = getsym();
		if (sym == IDSYM)
		{
			sym = getsym();
			while (sym == COMMASYM)//������
			{
				sym = getsym();
				if (sym == INTSYM || sym == CHARSYM)
				{
					sym = getsym();
					if (sym == IDSYM)
					{
						sym = getsym();
						continue;
					}
					else
					{
						error(ERROR_PARAMETER);
						return 0;
					}
				}
				else
				{
					error(ERROR_PARAMETER);
					return 0;
				}
			}
			if (sym == RPARENSYM)
			{
				return 1;
			}
			else
			{
				error(ERROR_PARAMETER);
				return 0;
			}
		}
		else
		{
			error(ERROR_PARAMETER);
			return 0;
		}
	}
	else if(sym == RPARENSYM)//Ϊ��
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

}
int void_func_defination()
{

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
int main_func()//������,void mainǰ���жϹ���
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
int compound_statement()//�������
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
		//TODO:���޷���ֵ��ȫ��ͬ ���ﲻд����
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
	int main_cnt = 0;
	sym = getsym();
	if (sym == CONSTSYM)//����˵��
	{
		const_declaration();
	}
	while (sym == INTSYM || sym == CHARSYM || sym == VOIDSYM)//TODO:��Ҫѭ��
	{
		if (sym == VOIDSYM)
		{
			sym = getsym();
			if (sym == MAINSYM)
			{
				sym = getsym();
				main_func();//��(��ʼ����
				main_cnt++;
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
		else//int char
		{
			head();
			if (sym == LMPARENSYM)//����
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
				if (sym == SEMICOLONSYM)
				{
					//TODO�������ӡ
					sym = getsym();
					continue;
				}
				else if (sym == COMMASYM)
				{
					var_defination_backend();//˵��һ���ж������
					if (sym == SEMICOLONSYM)
					{
						//TODO�������ӡ
						sym = getsym();
						continue;
					}
				}
			}
			else if (sym == COMMASYM)//���������
			{
				var_defination_backend();//˵��һ���ж������
				if (sym == SEMICOLONSYM)
				{
					//TODO�������ӡ
					sym = getsym();
					continue;
				}
			}
			else if (sym == SEMICOLONSYM)
			{
				//TODO�������ӡ
				sym = getsym();
				continue;
			}
			else if (sym == LPARENSYM)//˵�����з���ֵ��������
			{
				return_func_defination_backend();
			}
			else
			{
				error(MISSING_SEMICOLON);
				return 0;
			}
		}
		if (main_cnt == 0)
			error(MISSING_MAIN_FUNC);
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