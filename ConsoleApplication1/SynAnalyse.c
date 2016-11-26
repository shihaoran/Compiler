#include "SynAnalyse.h"
#include "error.h"
#include "LexAnalyse.h"

/*=================语法分析部分=====================*/
int sym;//全局符号
extern int line;//当前行
extern char id[MAX_ID_LEN];//最后读入的标识符
/*======================END=========================*/

/*=================四元式生成部分=====================*/
int search_sym(char *name)//寻找当前标识符在符号表中的位置
{
	int i;
	for (i = sym_ptr; i >= 0; i--)
	{
		if (!strcmp(name, sym_table[i].name))
		{
			return i;
		}
	}
	return -1;
}

void emit(int op, char* op1, char* op2, char* opr)//生成四元式，加入四元式数组中,局部常量不加标识符用以区分全局局部
{
	quat_table[quat_ptr].op = op;
	strcpy(quat_table[quat_ptr].op1, op1);
	strcpy(quat_table[quat_ptr].op2, op2);
	strcpy(quat_table[quat_ptr].opr, opr);
	quat_ptr++;
}

void add_sym(char *name, int type, int value_type, int int_v, char char_v, char* str_v)
{
	strcpy(sym_table[sym_ptr].name, name);
	sym_table[sym_ptr].type = type;
	sym_table[sym_ptr].value_type = value_type;
	if (type == TYPE_CONST)
	{
		switch (value_type)
		{
		case TYPE_VALUE_INT: 
			sym_table[sym_ptr].int_value = int_v; 
			break;
		case TYPE_VALUE_CHAR: 
			sym_table[sym_ptr].char_value = char_v;
			break;
		case TYPE_VALUE_STR: 
			sym_table[sym_ptr].str_value, str_v;
			break;
		}
	}
	else if (type == TYPE_ARRAY)
		sym_table[sym_ptr].int_value = int_v;
	else
		sym_table[sym_ptr].int_value = 0;
	sym_ptr++;
}
int add_tmp(char *name, int type)
{
	sprintf(name, "$%d", sym_ptr-tmp_ptr);
	add_sym(name, TYPE_TMP, type, 0, 0, NULL);
	return sym_ptr - 1;
}
int gen_op(char *name, int i,int type,int array_i, int array_i_type)//0为符号表指针，1为int立即数, 2为char立即数, 3为数组（在下层，此处不会返回3）
{
	char tmp[MAX_OP_LEN];
	char tmp1[MAX_OP_LEN];
	int tmp_type;
	if (type == 0)
	{
		if (i < 0)
		{
			error(OP_OUT_OF_RANGE);
			return 4;
		}
		else if (i < para_ptr)//如果是全局变量
		{
			sprintf(name, "%s", sym_table[i].name);
		}
		else if (i < local_ptr)//如果是参数
		{
			sprintf(name, "@%d", i-para_ptr);
		}
		else if (i < tmp_ptr)//如果是局部变量
		{
			sprintf(name, "%%%d", i - local_ptr);
		}
		else //如果是临时变量
		{
			sprintf(name, "$%d", i - tmp_ptr);
		}
		return sym_table[i].value_type;
	}
	else if (type == 1 || type == 2)
	{
		sprintf(name, "%d", i);
		return type;
	}
	else if (type == 3)//数组
	{
		if (i < para_ptr)
		{
			strcpy(name,sym_table[i].name);
		}
		else
		{
			sprintf(name, "%%%d", i-local_ptr);
		}
		if (array_i_type)//0为符号表指针，1为整数
		{
			sprintf(tmp, "[%d]", i);
			strcat(name, tmp);
		}
		else//如果是符号表指针，生成相应的变量号
		{
			tmp_type=gen_op(tmp, array_i, 0, 0, 0);//TODO：如果是char?
			sprintf(tmp1, "[%s]", tmp);
			strcat(name, tmp1);
		}
		return sym_table[i].value_type;
	}
	else
	{
		error(OP_OUT_OF_RANGE);
		return 4;
	}
}

/*======================END=========================*/

int const_defination()//常量定义
{
	int position;
	char temp_str[MAX_OP_LEN];
	if (sym == INTSYM)
	{
		sym = getsym();
		if (sym != IDSYM)
		{
			error(DECLARATION_SHOULD_HAVE_A_ID);
			return 0;
		}
		position = search_sym(id);
		if ((position >= para_ptr&&in_func) || (position >= 0 && !in_func))
		{
			error(DUPLICATE_DEFINE_IDENTIFIER);
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
		add_sym(id, TYPE_CONST, TYPE_VALUE_INT, num_sign, 0, NULL);
		sprintf(temp_str, "%d", num_sign);
		if(in_func)
			emit(CONST, "INT", "", temp_str);
		else
			emit(CONST, "INT", id, temp_str);
		while (sym == COMMASYM)
		{
			sym = getsym();
			if (sym != IDSYM)
			{
				error(DECLARATION_SHOULD_HAVE_A_ID);
				return 0;
			}
			position = search_sym(id);
			if ((position >= para_ptr&&in_func) || (position >= 0 && !in_func))
			{
				error(DUPLICATE_DEFINE_IDENTIFIER);
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
			add_sym(id, TYPE_CONST, TYPE_VALUE_INT, num_sign, 0, NULL);
			sprintf(temp_str, "%d", num_sign);
			if (in_func)
				emit(CONST, "INT", "", temp_str);
			else
				emit(CONST, "INT", id, temp_str);
		}
		printf("Line:%d --This is a const_defination_statement!\n", line+1);
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
		position = search_sym(id);
		if ((position >= para_ptr&&in_func) || (position >= 0 && !in_func))
		{
			error(DUPLICATE_DEFINE_IDENTIFIER);
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
		add_sym(id, TYPE_CONST, TYPE_VALUE_CHAR, 0, c, NULL);
		sprintf(temp_str, "%d", c);
		if (in_func)
			emit(CONST, "CHAR", "", temp_str);
		else
			emit(CONST, "CHAR", id, temp_str);
		sym = getsym();
		while (sym == COMMASYM)
		{
			sym = getsym();
			if (sym != IDSYM)
			{
				error(DECLARATION_SHOULD_HAVE_A_ID);
				return 0;
			}
			position = search_sym(id);
			if ((position >= para_ptr&&in_func) || (position >= 0 && !in_func))
			{
				error(DUPLICATE_DEFINE_IDENTIFIER);
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
			add_sym(id, TYPE_CONST, TYPE_VALUE_CHAR, 0, c, NULL);
			sprintf(temp_str, "%d", c);
			if (in_func)
				emit(CONST, "CHAR", "", temp_str);
			else
				emit(CONST, "CHAR", id, temp_str);
			sym = getsym();
		}
		printf("Line:%d --This is a const_defination_statement!\n", line+1);
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
	int position;
	char temp_str[MAX_OP_LEN];
	int head_type;
	head_type = head();
	if (head_type==0)
	{
		error(WRONG_HEAD);
		return 0;
	}
	position = search_sym(id);
	if ((position >= para_ptr&&in_func) || (position >= 0 && !in_func))
	{
		error(DUPLICATE_DEFINE_IDENTIFIER);
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
		sprintf(temp_str, "%d", num);
		sym = getsym();
		if (sym != RMPARENSYM)
		{
			error(PARENT_DISMATCH);
			return 0;
		}
		if (head_type == 1)
		{
			add_sym(id, TYPE_ARRAY, TYPE_VALUE_INT, num, 0, NULL);
			if (in_func)
				emit(VAR, "INT", "", temp_str);
			else
				emit(VAR, "INT", id, temp_str);
		}
		else
		{
			add_sym(id, TYPE_ARRAY, TYPE_VALUE_CHAR, num, 0, NULL);
			if (in_func)
				emit(VAR, "CHAR", "", temp_str);
			else
				emit(VAR, "CHAR", id, temp_str);
		}
		sym = getsym();
	}
	else
	{
		if (head_type == 1)
		{
			add_sym(id, TYPE_VAR, TYPE_VALUE_INT, 0, 0, NULL);
			if (in_func)
				emit(VAR, "INT", "", "");
			else
				emit(VAR, "INT", id, "");
		}
		else
		{
			add_sym(id, TYPE_VAR, TYPE_VALUE_CHAR, 0, 0, NULL);
			if (in_func)
				emit(VAR, "CHAR", "", "");
			else
				emit(VAR, "CHAR", id, "");
		}
	}
	if (!var_defination_backend(head_type))
	{
		return 0;
	}
	return 1;
}
int var_defination_backend(int head_type)
{
	int position;
	char temp_str[MAX_OP_LEN];
	if (sym == SEMICOLONSYM)
	{
		printf("Line:%d --This is a var_defination_statement!\n", line+1);
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
		position = search_sym(id);
		if ((position >= para_ptr&&in_func) || (position >= 0 && !in_func))
		{
			error(DUPLICATE_DEFINE_IDENTIFIER);
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
			sprintf(temp_str, "%d", num);
			sym = getsym();
			if (sym != RMPARENSYM)
			{
				error(PARENT_DISMATCH);
				return 0;
			}
			if (head_type == 1)
			{
				add_sym(id, TYPE_ARRAY, TYPE_VALUE_INT, num, 0, NULL);
				if (in_func)
					emit(VAR, "INT", "", temp_str);
				else
					emit(VAR, "INT", id, temp_str);
			}
			else
			{
				add_sym(id, TYPE_ARRAY, TYPE_VALUE_CHAR, num, 0, NULL);
				if (in_func)
					emit(VAR, "CHAR", "", temp_str);
				else
					emit(VAR, "CHAR", id, temp_str);
			}
			sym = getsym();
		}
		else
		{
			if (head_type == 1)
			{
				add_sym(id, TYPE_VAR, TYPE_VALUE_INT, 0, 0, NULL);
				if (in_func)
					emit(VAR, "INT", "", "");
				else
					emit(VAR, "INT", id, "");
			}
			else
			{
				add_sym(id, TYPE_VAR, TYPE_VALUE_CHAR, 0, 0, NULL);
				if (in_func)
					emit(VAR, "CHAR", "", "");
				else
					emit(VAR, "CHAR", id, "");
			}
		}
	}
	printf("Line:%d --This is a var_defination_statement!\n", line+1);
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
	int flag = 0;
	if (sym == PLUSSYM)
	{
		sym = getsym();
	}
	else if (sym == MINUSSYM)
	{
		flag = 1;
		sym = getsym();
	}
	if (sym == NUMSYM)
	{
		sym = getsym();
		if (flag)//说明有负号
		{
			num_sign = -num;
		}
		else
		{
			num_sign = num;
		}
		return 1;
	}
	else if (sym == ZEROSYM)
	{
		sym = getsym();
		num_sign = num;
		return 1;
	}
	else
	{
		error(WRONG_FORMAT_INTEGER);
		return 0;
	}
}
int head()//0为错误，1为int,2为char
{
	if (sym == INTSYM)
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
	else if (sym == CHARSYM)
	{
		sym = getsym();
		if (sym == IDSYM)
		{
			sym = getsym();
			return 2;
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
int call_func(int i)
{
	if (sym_table[i].type != TYPE_FUNC)
	{
		error(THIS_IS_NOT_FUNC);
		return -1;
	}
	if (!value_parameter_table(i))
	{
		return 4;
	}
	emit(CALL, sym_table[i].name, "", "");
	if (sym != RPARENSYM)
	{
		error(PARENT_DISMATCH);
		return 4;
	}
	sym = getsym();
	return sym_table[i].value_type;
}
int value_parameter_table(int i)
{
	int now_ptr;//当前参数指针
	int stack_ptr = 0;//栈指针
	int j;//表达式返回值
	int type;//表达式返回类型
	char para_stack[MAX_PARA_LEN][MAX_OP_LEN];
	char op_1[MAX_OP_LEN];
	if (sym == RPARENSYM)
	{
		if (sym_table[i + 1].type == TYPE_PARA)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		return 1;
	}
	else
	{
		now_ptr = i + 1;
		if (sym_table[now_ptr].type != TYPE_PARA)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		j = expression(&type);
		if (type == 4 || j == -1)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		if ((type == 0 && sym_table[j].value_type == sym_table[now_ptr].value_type) ||
			((type == 1 || type == 2) && type == sym_table[now_ptr].value_type))
		{
			gen_op(op_1, j, type, 0, 0);
			strcpy(para_stack[stack_ptr], op_1);
		}
		else
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		now_ptr++;
		while (sym==COMMASYM)
		{
			sym = getsym();
			if (sym_table[now_ptr].type != TYPE_PARA)
			{
				error(ERROR_PARAMETER);
				return 0;
			}
			j = expression(&type);
			if (type == 4 || j == -1)
			{
				error(ERROR_PARAMETER);
				return 0;
			}
			if ((type == 0 && sym_table[j].value_type == sym_table[now_ptr].value_type) ||
				((type == 1 || type == 2) && type == sym_table[now_ptr].value_type))
			{
				gen_op(op_1, j, type, 0, 0);
				strcpy(para_stack[stack_ptr++], op_1);
			}
			else
			{
				error(ERROR_PARAMETER);
				return 0;
			}
			now_ptr++;
		}
		if (sym_table[now_ptr].type == TYPE_PARA)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		stack_ptr--;
		for(stack_ptr;stack_ptr>=0;stack_ptr--)
			emit(PARA, para_stack[stack_ptr], "", "");
		return 1;
	}
}
int parameter_table()
{
	int position;
	int para_type;
	if (sym == INTSYM || sym == CHARSYM)
	{
		if (sym == INTSYM)
			para_type = 1;
		else
			para_type = 2;
		sym = getsym();
		if (sym != IDSYM)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		position = search_sym(id);
		if (position >= para_ptr) 
		{
			error(DUPLICATE_DEFINE_IDENTIFIER);
			return 0;
		}
		add_sym(id, TYPE_PARA, para_type, 0, 0, NULL);
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
			position = search_sym(id);
			if (position >= para_ptr)
			{
				error(DUPLICATE_DEFINE_IDENTIFIER);
				return 0;
			}
			add_sym(id, TYPE_PARA, para_type, 0, 0, NULL);
			sym = getsym();
		}
		if (sym != RPARENSYM)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		local_ptr = sym_ptr;
		return 1;
	}
	else if(sym == RPARENSYM)//为空
	{
		return 1;
		local_ptr = sym_ptr;
	}
	else
	{
		error(ERROR_PARAMETER);
		return 0;
	}
}
int void_func_defination()
{
	int position;
	if (sym != IDSYM)
	{
		error(MISSING_IDENTIFIER);
		return 0;
	}
	position = search_sym(id);
	if (position >= 0)
	{
		error(DUPLICATE_DEFINE_IDENTIFIER);
		return 0;
	}
	add_sym(id, TYPE_FUNC, TYPE_VALUE_INIT, 0, 0, NULL);
	emit(FUNC, id, "", "");
	para_ptr = sym_ptr;
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
	printf("Line:%d --This is a void_function_defination_statement!\n", line + 1);
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
int return_func_defination(int head_type)
{
	if (!head())
		return 0;
	if (!return_func_defination_backend(head_type))
		return 0;
	return 1;
}
int return_func_defination_backend(int head_type)
{
	int position;
	in_func = 1;
	if (sym != LPARENSYM)
	{
		error(ERROR_PARAMETER);
		return 0;
	}
	position = search_sym(id);
	if (position >= 0 )
	{
		error(DUPLICATE_DEFINE_IDENTIFIER);
		return 0;
	}
	add_sym(id, TYPE_FUNC, head_type, 0, 0, NULL);
	emit(FUNC, id, "", "");
	para_ptr = sym_ptr;
	sym = getsym();
	parameter_table();
	if (sym != RPARENSYM)
	{
		error(ERROR_PARAMETER);
		return 0;
	}
	printf("Line:%d --This is a return_function_defination_statement!\n", line + 1);
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
	printf("Line:%d --This is a main_function_defination_statement!\n", line + 1);
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
	if (sym == RBPARENSYM)
	{
		return 1;
	}
	while (statement()) 
	{
		if (sym == RBPARENSYM)
		{
			break;
		}
	}
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
		printf("Line:%d --This is an if_statement!\n", line+1);
		return 1;
	}
	else if (sym == WHILESYM)
	{
		if (!while_statement())
		{
			return 0;
		}
		printf("Line:%d --This is a while_statement!\n", line+1);
		return 1;
	}
	else if (sym == LBPARENSYM)//TODO：检查一下
	{
		sym = getsym();
		while (statement()) 
		{
			if (sym == RBPARENSYM)
				break;
		}//因为可能为空，不应报错
		if (sym != RBPARENSYM)
		{
			error(BRACE_DISMATCH);
			return 0;
		}
		printf("Line:%d --This is a statement list!\n", line+1);
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
			if (sym != SEMICOLONSYM)
			{
				error(MISSING_SEMICOLON);
				return 0;
			}
			printf("Line:%d --This is a function_call_statement!\n", line+1);
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
			printf("Line:%d --This is an assign_statement!\n", line+1);
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
			printf("Line:%d --This is an assign_statement!\n", line+1);
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
		printf("Line:%d --This is a scanf_statement!\n", line+1);
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
		printf("Line:%d --This is a printf_statement!\n", line+1);
		sym = getsym();
		return 1;
	}
	else if (sym == SWITCHSYM)
	{
		if (!switch_statement())
		{
			return 0;
		}
		printf("Line:%d --This is a switch_statement!\n", line+1);
		return 1;
	}
	else if (sym == RETURNSYM)
	{
		if (!return_statement())
		{
			return 0;
		}
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
			return 0;
		}
		printf("Line:%d --This is a return_statement!\n", line+1);
		sym = getsym();
		return 1;
	}
	else if (sym == SEMICOLONSYM)
	{
		printf("Line:%d --This is an empty_statement!\n", line+1);
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
		sym = getsym();
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
	sym = getsym();
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
		sym = getsym();
	}//TODO:统一结束处理
	return 1;
}
int expression(int *type)//表达式,传入的指针表示返回的数的类型，0为符号表指针，1为int立即数,2为char立即数,3为数组（在下层，此处不会返回3），4为报错此时返回-1
{
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	int i,array_i,array_i_type;
	int tmp_type1,tmp_type2;
	int first;//保存最开始的符号
	int flag=0;//是否只有一项
	int op;//多项时当前是正负号
	if (sym == PLUSSYM || sym == MINUSSYM)//为了能够直接 
	{
		first = (sym == PLUSSYM) ? 0 : 1;
		sym = getsym();
	}
	i=item(type,&array_i,&array_i_type);
	if (*type == 4||i==-1)
	{
		*type = 4;
		return -1;
	}
	else if (*type == 3)//说明是数组
	{
		*type = gen_op(op_1, i, *type, array_i, array_i_type);
		i = add_tmp(op_r, *type);
		emit(MOV, op_1, "",op_r);
		*type = 0;
	}
	while (sym == PLUSSYM || sym == MINUSSYM)
	{
		flag = 1;
		op = (sym == PLUSSYM) ? ADD : SUB;
		if (first)//开始有一个负号，结束后把结果放到op1中
		{
			if (*type == 1 || *type == 2) //如果是立即数,取负存入临时变量
			{
				tmp_type1 = gen_op(op_2,-i,*type,array_i,array_i_type);
				i = add_tmp(op_1, tmp_type1);
				emit(MOV, op_2, "", op_1);
			}
			else//如果不是，是指针，新增临时变量取反
			{
				tmp_type1 = gen_op(op_2, i, *type, array_i, array_i_type);
				i = add_tmp(op_1, tmp_type1);
				emit(NEG, op_2, "", op_1);
			}
			*type = 0;
			first = 0;
		}
		else
		{
			tmp_type1 = gen_op(op_1, i, *type, array_i, array_i_type);
		}
		sym = getsym();
		i = item(type, &array_i, &array_i_type);
		if (*type == 4)
		{
			return -1;
		}
		tmp_type2 = gen_op(op_2, i, *type, array_i, array_i_type);
		if (tmp_type1 == 2 && tmp_type2 == 2)//如果两项中有一项为int，都转换为int
		{
			i = add_tmp(op_r, 2);
		}
		else
		{
			i = add_tmp(op_r, 1);
		}
		emit(op, op_1, op_2, op_r);
		*type = 0;
	}
	if (!flag)//如果只有一项
	{
		if ((*type == 1 || *type == 2)&&first)//如果是立即数,直接返回负值
		{
			i = -i;
		}
		else
		{
			*type = gen_op(op_1, i, *type, array_i, array_i_type);
			i = add_tmp(op_r, *type);
			emit(NEG, op_1, "", op_r);//如果不是，是指针，新增临时变量取反
		}
	}
	return i;
}
int item(int *type,int *array_i,int *array_i_type)//项
{
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	int tmp_type1, tmp_type2;
	int i;
	int op;//当前是乘除号
	i = factor(type, array_i, array_i_type);
	if (*type == 4 || i == -1)
	{
		*type = 4;
		return -1;
	}
	while (sym == TIMESSYM || sym == DIVSYM)
	{
		tmp_type1 = gen_op(op_1, i, *type, array_i, array_i_type);
		op = (sym == TIMESSYM) ? MUL : DIV;
		sym = getsym();
		i = factor(type, array_i, array_i_type);
		if (*type == 4 || i == -1)
		{
			*type = 4;
			return -1;
		}
		tmp_type2 = gen_op(op_2, i, *type, array_i, array_i_type);
		if (tmp_type1 == 2 && tmp_type2 == 2)//如果两项中有一项为int，都转换为int
		{
			i = add_tmp(op_r, 2);
		}
		else
		{
			i = add_tmp(op_r, 1);
		}
		emit(op, op_1, op_2, op_r);
		*type = 0;
	}
	return i;
}
int factor(int *type, int *array_i, int *array_i_type)//因子
{
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	int i;
	*type = 0;//清空，其实只用上传
	if (sym == IDSYM)//标识符，数组，有返回值函数调用
	{
		i = search_sym(id);
		if (i < 0)
		{
			error(NONE_DEFINE_IDENTIFIER);
			*type = 4;
			return -1;
		}
		sym = getsym();
		if (sym == LMPARENSYM)//数组
		{
			if (sym_table[i].type != TYPE_ARRAY)
			{
				error(THIS_IS_NOT_ARRAY);
				*type = 4;
				return -1;
			}
			sym = getsym();
			*array_i = expression(type);
			if (*type == 4 || *array_i == -1)
			{
				*type = 4;
				return -1;
			}
			*array_i_type = *type;
			*type = 3;
			gen_op(op_1, i, *type, array_i, array_i_type);
			i = add_tmp(op_r, sym_table[i].value_type);
			emit(MOV, op_1, "", op_r);
			*type = 0;
			if (sym != RMPARENSYM)
			{
				error(PARENT_DISMATCH);
				*type = 4;
				return -1;
			}
			sym = getsym();
		}
		else if (sym == LPARENSYM)//有返回值函数调用
		{
			sym = getsym();
			i = call_func(i);
			if (i==4)
			{
				*type = 4;
				return -1;
			}
			i = add_tmp(op_r, i);
			emit(MOV, "!eax", "", op_r);
		}
		else//普通变量常量参数
		{
			if (sym_table[i].type != TYPE_CONST&&sym_table[i].type != TYPE_VAR&&sym_table[i].type != TYPE_PARA)
			{
				error(IDENTIFIER_TYPE_DISMATCH);
				*type = 4;
				return -1;
			}
		}
	}
	else if (sym == PLUSSYM || sym == MINUSSYM || sym == NUMSYM || sym == ZEROSYM)
	{
		if (!integer())
		{
			*type = 4;
			return -1;
		}
		i = num_sign;
		*type = 1;
	}
	else if (sym == CHSYM)
	{
		sym = getsym();
		i = c;
		*type = 2;
	}
	else if (sym == LPARENSYM)
	{
		sym = getsym();
		i = expression(type);
		if (*type==4||i==-1)
		{
			*type = 4;
			return -1;
		}
		if (sym != RPARENSYM)
		{
			error(PARENT_DISMATCH);
			*type = 4;
			return -1;
		}
		sym = getsym();
	}
	else
	{
		error(ERROR_IN_EXPRESSION);
		*type = 4;
		return -1;
	}
	return i;
}
int program()
{
	int main_cnt = 0;
	int var_flag = 0;//在主程序中的函数定义后不应该有变量说明
	int head_type;
	int position;
	char temp_str[MAX_OP_LEN];
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
			head_type=head();
			if (head_type == 0)
			{
				error(WRONG_HEAD);
				return 0;
			}
			position = search_sym(id);
			if ((position >= para_ptr&&in_func) || (position >= 0 && !in_func))
			{
				error(DUPLICATE_DEFINE_IDENTIFIER);
				return 0;
			}
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
				sprintf(temp_str, "%d", num);
				sym = getsym();
				if (sym != RMPARENSYM)
				{
					error(PARENT_DISMATCH);
					return 0;
				}
				if (head_type == 1)
				{
					add_sym(id, TYPE_ARRAY, TYPE_VALUE_INT, num, 0, NULL);
					if (in_func)
						emit(VAR, "INT", "", temp_str);
					else
						emit(VAR, "INT", id, temp_str);
				}
				else
				{
					add_sym(id, TYPE_ARRAY, TYPE_VALUE_CHAR, num, 0, NULL);
					if (in_func)
						emit(VAR, "CHAR", "", temp_str);
					else
						emit(VAR, "CHAR", id, temp_str);
				}
				sym = getsym();
				if (sym == SEMICOLONSYM)
				{
					sym = getsym();
					continue;
				}
				else if (sym == COMMASYM)
				{
					var_defination_backend(head_type);//说明一行有多个定义
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
				if (head_type == 1)
				{
					add_sym(id, TYPE_VAR, TYPE_VALUE_INT, 0, 0, NULL);
					if (in_func)
						emit(VAR, "INT", "", "");
					else
						emit(VAR, "INT", id, "");
				}
				else
				{
					add_sym(id, TYPE_VAR, TYPE_VALUE_CHAR, 0, 0, NULL);
					if (in_func)
						emit(VAR, "CHAR", "", "");
					else
						emit(VAR, "CHAR", id, "");
				}
				var_defination_backend(head_type);//说明一行有多个定义
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
				if (head_type == 1)
				{
					add_sym(id, TYPE_VAR, TYPE_VALUE_INT, 0, 0, NULL);
					if (in_func)
						emit(VAR, "INT", "", "");
					else
						emit(VAR, "INT", id, "");
				}
				else
				{
					add_sym(id, TYPE_VAR, TYPE_VALUE_CHAR, 0, 0, NULL);
					if (in_func)
						emit(VAR, "CHAR", "", "");
					else
						emit(VAR, "CHAR", id, "");
				}
				printf("Line:%d --This is a var_defination_statement!\n", line + 1);
				sym = getsym();
				continue;
			}
			else if (sym == LPARENSYM)//说明是有返回值函数定义
			{
				return_func_defination_backend(head_type);
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


void print_quat()
{
	int i;
	for (i = 0; i < quat_ptr; i++)
	{
		printf("%s   %s   %s   %s\n", quat_op_name[quat_table[i].op], quat_table[i].op1, quat_table[i].op2, quat_table[i].opr);
	}
}


int main()
{
	int result,i=0;
	init();
	program();
	print_quat();
	scanf("%d", &result);
}