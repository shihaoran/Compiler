#include "SynAnalyse.h"
#include "error.h"
#include "LexAnalyse.h"

/*=================语法分析部分=====================*/
int sym;//全局符号
extern int line;//当前行
extern char id[MAX_ID_LEN];//最后读入的标识符
extern char string[MAX_TOKEN_LEN];//最后读入的字符串
/*======================END=========================*/
FILE *mips;
int gen_mips_ptr=0;
int local_table[MAX_TAB_LEN];
int local_offset = 0;//当前相对坐标偏移量
int func_cnt = 0;//当前是第几个函数，用于return
int para_cnt = 0;//记录函数调用参数数量，call时清零
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
	int i;
	if (quat_ptr == 0)
	{
		for (i = 0; i < MAX_QUAT_LEN; i++)
		{
			quat_table[i].label = -1;
			quat_table[i].is_empty = 0;
		}
	}
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
		if (in_func)
		{
			local_table[sym_ptr - local_ptr] = local_offset;
			local_offset++;
		}
	}
	else if (type == TYPE_ARRAY)
	{
		sym_table[sym_ptr].int_value = int_v;
		if (in_func)
		{
			local_table[sym_ptr - local_ptr] = local_offset;
			local_offset+=int_v;
		}
	}
	else
	{
		sym_table[sym_ptr].int_value = 0;
		if (in_func&&type==TYPE_VAR)
		{
			local_table[sym_ptr - local_ptr] = local_offset;
			local_offset++;
		}
	}
	sym_ptr++;
}
int add_tmp(char *name, int type)
{
	sprintf(name, "$%d", tmp_cnt);
	add_sym(name, TYPE_TMP, type, 0, 0, NULL);
	tmp_cnt++;
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
			sprintf(name, "%%%d", local_table[i - local_ptr]);
		}
		else //如果是临时变量
		{
			sprintf(name, "%s", sym_table[i].name);
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
			name[0] = '&';
			strcat(name,sym_table[i].name);
		}
		else
		{
			sprintf(name, "&%%%d", local_table[i - local_ptr]);
		}
		if (array_i_type)//0为符号表指针，1为整数
		{
			sprintf(tmp, "[%d]", array_i);
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
void handle_error(int *type)//跳读函数type为跳读结束符号集
{
	int flag = 1,i;
	int len = (sizeof(type) / sizeof(flag));
	do
	{
		for (i = 0; i < len; i++)
		{
			if (type[i] == sym)
			{
				flag = 0;
				break;
			}
		}
		if(flag)
			sym = getsym();
	} while (flag);
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
		if (!integer())
		{
			int endsym[] = { SEMICOLONSYM ,COMMASYM};
			handle_error(endsym);
			error_cnt++;
		}
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
			if (!integer())
			{
				int endsym[] = { SEMICOLONSYM ,COMMASYM };
				handle_error(endsym);
				error_cnt++;
			}
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
		if (!const_defination())//进行出错处理
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
		}
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
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
		int endsym[] = { SEMICOLONSYM };
		handle_error(endsym);
		error_cnt++;
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
		if (!var_defination())
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
			return 1;
		}
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
			error(MISSING_IDENTIFIER);
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
			error(MISSING_IDENTIFIER);
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
			error(MISSING_PARAMETER);
			return 0;
		}
		return 1;
	}
	else
	{
		now_ptr = i + 1;
		if (sym_table[now_ptr].type != TYPE_PARA)
		{
			error(UNDEFINE_PARAMETER);
			return 0;
		}
		j = expression(&type);
		if (type == 4 && j == -1)
		{
			int endsym[] = { COMMASYM,RPARENSYM };
			handle_error(endsym);
			error_cnt++;
			error(ERROR_PARAMETER);
		}
		if ((type == 0 && sym_table[j].value_type == sym_table[now_ptr].value_type) ||
			((type == 1 || type == 2) && type == sym_table[now_ptr].value_type))
		{
			gen_op(op_1, j, type, 0, 0);
			strcpy(para_stack[stack_ptr++], op_1);
		}
		else
		{
			error(DISMATCH_PARAMETER);
			return 0;
		}
		now_ptr++;
		while (sym==COMMASYM)
		{
			sym = getsym();
			if (sym_table[now_ptr].type != TYPE_PARA)
			{
				error(UNDEFINE_PARAMETER);
				return 0;
			}
			j = expression(&type);
			if (type == 4 && j == -1)
			{
				int endsym[] = { COMMASYM,RPARENSYM };
				handle_error(endsym);
				error_cnt++;
				error(ERROR_PARAMETER);
			}
			if ((type == 0 && sym_table[j].value_type == sym_table[now_ptr].value_type) ||
				((type == 1 || type == 2) && type == sym_table[now_ptr].value_type))
			{
				gen_op(op_1, j, type, 0, 0);
				strcpy(para_stack[stack_ptr++], op_1);
			}
			else
			{
				error(DISMATCH_PARAMETER);
				return 0;
			}
			now_ptr++;
		}
		if (sym_table[now_ptr].type == TYPE_PARA)
		{
			error(MISSING_PARAMETER);
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
			error(MISSING_IDENTIFIER);
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
				error(DECLARATION_IS_NOT_START_WITH_TYPE);
				return 0;
			}
			if (sym == INTSYM)
				para_type = 1;
			else
				para_type = 2;
			sym = getsym();
			if (sym != IDSYM)
			{
				error(MISSING_IDENTIFIER);
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
			error(PARENT_DISMATCH);
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
		error(DECLARATION_IS_NOT_START_WITH_TYPE);
		return 0;
	}
}
int void_func_defination()
{
	int position;
	int i;
	int func_id_i;
	has_return = 0;
	in_func = 1;
	if (sym != IDSYM)
	{
		error(MISSING_IDENTIFIER);
		return 0;
	}
	position = search_sym(id);
	if (position >= 0)
	{
		error(DUPLICATE_DEFINE_IDENTIFIER);
	}
	add_sym(id, TYPE_FUNC, TYPE_VALUE_INIT, 0, 0, NULL);
	func_id_i = sym_ptr - 1;
	emit(FUNC, id, "", "");
	para_ptr = sym_ptr;
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(ERROR_PARAMETER);
	}
	sym = getsym();
	if (!parameter_table())
	{
		int endsym[]= {RPARENSYM};
		handle_error(endsym);
		error_cnt++;
	}
	if(sym!=RPARENSYM)
	{
		error(ERROR_PARAMETER);
		int endsym[]= { RPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	printf("Line:%d --This is a void_function_defination_statement!\n", line + 1);
	sym = getsym();
	if (sym != LBPARENSYM)
	{
		error(MISSING_STRUCTURER_IN_FUNC_DEFINE);
		int endsym[]= { LBPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	sym = getsym();
	if (!compound_statement())
	{
		int endsym[] = { RBPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	if (has_return)
	{
		error(NRETURN_FUNC_CANT_RETURN);
	}
	if (sym != RBPARENSYM)
	{
		error(MISSING_BRACE);
		int endsym[]= { RBPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	sym = getsym();
	in_func = 0;
	emit(EOFUNC, sym_table[func_id_i].name, "", "");
	for (i = para_ptr; i < sym_ptr; i++)//清空到参数的名字，但是仍然要保留参数，用作参数个数判断
		sym_table[i].name[0] = '\0';
	for (i = 0; i < local_offset; i++)//其实这里不应该到offset，但是找不到局部变量个数了，只能找到更大的值
		local_table[i] = 0;
	local_offset = 0;
	sym_ptr = local_ptr;
	return 1;
}
int return_func_defination(int head_type)
{
	if (!head())
		return 0;
	if (!return_func_defination_backend(head_type))
		return 0;
	if (!has_return)
	{
		error(RUTURN_FUNC_MUST_RETURN);
	}
	return 1;
}
int return_func_defination_backend(int head_type)
{
	int position;
	int func_id_i;//保存当前函数名的符号表地址
	int i;
	in_func = 1;
	has_return = 0;
	if (sym != LPARENSYM)
	{
		error(ERROR_PARAMETER);
	}
	position = search_sym(id);
	if (position >= 0 )
	{
		error(DUPLICATE_DEFINE_IDENTIFIER);
	}
	add_sym(id, TYPE_FUNC, head_type, 0, 0, NULL);
	func_id_i = sym_ptr - 1;
	emit(FUNC, id, "", "");
	para_ptr = sym_ptr;
	sym = getsym();
	if (!parameter_table())
	{
		int endsym[] = { RPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	if (sym != RPARENSYM)
	{
		error(ERROR_PARAMETER);
		int endsym[] = { RPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	printf("Line:%d --This is a return_function_defination_statement!\n", line + 1);
	sym = getsym();
	if (sym != LBPARENSYM)
	{
		error(MISSING_STRUCTURER_IN_FUNC_DEFINE);
	}
	sym = getsym();
	if (!compound_statement())
	{
		int endsym[] = { RBPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	if(sym!=RBPARENSYM)
	{
		error(MISSING_BRACE);
		int endsym[] = { RBPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	sym = getsym();
	in_func = 0;
	emit(EOFUNC, sym_table[func_id_i].name, "", "");
	for (i = para_ptr; i < sym_ptr; i++)//清空到参数的名字，但是仍然要保留参数，用作参数个数判断
		sym_table[i].name[0] = '\0';
	for (i = 0; i < local_offset; i++)//其实这里不应该到offset，但是找不到局部变量个数了，只能找到更大的值
		local_table[i] = 0;
	local_offset = 0;
	sym_ptr = local_ptr;
	return 1;
}
int main_func()//主函数,void main前面判断过了
{
	has_return = 0;
	in_func = 1;
	emit(MAINFUNC, "", "", "");
	if (sym != LPARENSYM)
	{
		error(WRONG_HEAD);
	}
	sym = getsym();
	if (sym != RPARENSYM)
	{
		error(WRONG_HEAD);
		int endsym[] = { RPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	para_ptr = sym_ptr;//置参数起始位置
	printf("Line:%d --This is a main_function_defination_statement!\n", line + 1);
	sym = getsym();
	if (sym != LBPARENSYM)
	{
		error(MISSING_STRUCTURER_IN_FUNC_DEFINE);
	}	
	sym = getsym();
	if (!compound_statement())
	{
		int endsym[] = { RBPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	if (has_return)
	{
		error(NRETURN_FUNC_CANT_RETURN);
	}
	if (sym != RBPARENSYM)
	{
		error(MISSING_BRACE);
		int endsym[] = { RBPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	sym = getsym();
	in_func = 0;
	emit(EOMAINFUNC, "", "", "");
	return 1;
}
int compound_statement()//复合语句
{
	local_ptr = sym_ptr;
	if (sym == CONSTSYM)
	{
		if (!const_declaration())
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
			sym = getsym();
		}
	}
	if (sym == INTSYM || sym == CHARSYM)
	{
		if (!var_declaration())
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
			sym = getsym();
		}
	}
	tmp_ptr = sym_ptr;
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
		error(MISSING_BRACE);
		int endsym[] = { RBPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	emit(NOP, "", "", "");//加几个nop防止label被冲掉
	return 1;
}
int assign_statement(int i)
{
	int now_ptr;
	int type;
	int tmp_type_1, tmp_type_2;//为了判断赋值两侧类型是否相同
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	if (sym_table[i].type != TYPE_VAR&&sym_table[i].type != TYPE_ARRAY&&sym_table[i].type != TYPE_PARA)
	{
		error(ASSIGN_TYPE_ERROR);
		return 0;
	}
	if (sym == LMPARENSYM)//赋值语句数组
	{
		if (sym_table[i].type != TYPE_ARRAY)
		{
			error(IDENTIFIER_TYPE_DISMATCH);
			return 0;
		}
		sym = getsym();
		now_ptr = expression(&type);
		if (now_ptr == -1 && type == 4)
		{
			return 0;
		}
		tmp_type_1 = gen_op(op_1, i, 3, now_ptr, type);
		if (sym != RMPARENSYM)
		{
			error(PARENT_DISMATCH);
			return 0;
		}
		sym = getsym();
	}
	else
	{
		if (sym_table[i].type != TYPE_VAR&&sym_table[i].type != TYPE_PARA)
		{
			error(ASSIGN_TYPE_ERROR);
			return 0;
		}
		type = 0;
		tmp_type_1 = gen_op(op_1, i, type, 0, 0);
	}
	if (sym == ASSIGNSYM)//赋值语句
	{
		sym = getsym();
		now_ptr = expression(&type);//这里可能返回负值
		if (now_ptr==-1&&type==4)
		{
			return 0;
		}
		tmp_type_2 = gen_op(op_2, now_ptr, type, 0, 0);
		if (tmp_type_1 != tmp_type_2)
		{
			error(ASSIGN_DISMATCH);
			return 0;
		}
		emit(MOV, op_2, "", op_1);
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
			return 0;
		}
		printf("Line:%d --This is an assign_statement!\n", line + 1);
	}
	else
	{
		error(ASSIGN_ERROR);
		return 0;
	}
}
int statement()
{
	int i;
	int type;
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
			error(MISSING_BRACE);
			int endsym[] = { RBPARENSYM };
			handle_error(endsym);
			error_cnt++;
		}
		printf("Line:%d --This is a statement list!\n", line+1);
		sym = getsym();
		return 1;
	}
	else if (sym == IDSYM)
	{
		sym = getsym();
		//TODO:有无返回值完全相同 这里不写函数
		i = search_sym(id);
		if (i < 0)
		{
			error(UNDEFINE_IDENTIFIER);
		}
		//函数调用语句
		if (sym == LPARENSYM)
		{
			sym = getsym();
			type=call_func(i);
			if (type==4)
			{
				int endsym[] = { SEMICOLONSYM };
				handle_error(endsym);
				error_cnt++;
			}
			if (sym != SEMICOLONSYM)
			{
				error(MISSING_SEMICOLON);
				int endsym[] = { SEMICOLONSYM };
				handle_error(endsym);
				error_cnt++;
			}
			printf("Line:%d --This is a function_call_statement!\n", line+1);
		}
		else if (sym == ASSIGNSYM|| sym == LMPARENSYM)//赋值语句
		{
			if(!assign_statement(i))
			{
				int endsym[] = { SEMICOLONSYM };
				handle_error(endsym);
				error_cnt++;
			}
		}
		else
		{
			error(ERROR_IN_STATEMENT);
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
		}
		sym = getsym();
		return 1;
	}
	else if (sym == SCANFSYM)
	{
		if (!scanf_statement())
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
		}
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
		}
		printf("Line:%d --This is a scanf_statement!\n", line+1);
		sym = getsym();
		return 1;
	}
	else if (sym == PRINTFSYM)
	{
		if (!printf_statement())
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
		}
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
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
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
		}
		if (sym != SEMICOLONSYM)
		{
			error(MISSING_SEMICOLON);
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
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
	char label_str[MAX_OP_LEN];
	int label_1, label_2;
	if (sym != IFSYM)
	{
		error(ERROR_IN_IF);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(MISSING_BRACKET);
		return 0;
	}
	sym = getsym();
	if (quat_table[quat_ptr].label != -1)//防止两个标签加到一个四元式上
	{
		emit(NOP, "", "", "");
	}
	sprintf(label_str, "LABEL_%d", label_ptr);
	label_1 = label_ptr++;
	if (!condition_statement(label_str))
	{
		return 0;
	}
	if (sym != RPARENSYM)
	{
		error(PARENT_DISMATCH);
		return 0;
	}
	sym = getsym();
	if (!statement())
	{
		return 0;
	}
	if (sym == ELSESYM)
	{
		sprintf(label_str, "LABEL_%d", label_ptr);
		label_2 = label_ptr++;
		emit(JMP, "", "", label_str);
		sym = getsym();
		if (quat_table[quat_ptr].label != -1)//防止两个标签加到一个四元式上
		{
			emit(NOP, "", "", "");
		}
		quat_table[quat_ptr].label = label_1;
		if (!statement())
		{
			return 0;
		}
		if (quat_table[quat_ptr].label != -1)//防止两个标签加到一个四元式上
		{
			emit(NOP, "", "", "");
		}
		quat_table[quat_ptr].label = label_2;
	}
	else
	{
		if (quat_table[quat_ptr].label != -1)//防止两个标签加到一个四元式上
		{
			emit(NOP, "", "", "");
		}
		quat_table[quat_ptr].label = label_1;
	}
	return 1;
}
int while_statement()
{
	char label_str[MAX_OP_LEN];
	int label_1, label_2;
	int quat_tmp_ptr;//保存while指令的四元式指针
	if (sym != WHILESYM)
	{
		error(ERROR_IN_WHILE);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(MISSING_BRACKET);
		return 0;
	}
	sym = getsym();
	if (quat_table[quat_ptr].label != -1)//防止两个标签加到一个四元式上
	{
		emit(NOP, "", "", "");
	}
	sprintf(label_str, "LABEL_%d", label_ptr);
	label_1 = label_ptr++;
	quat_tmp_ptr = quat_ptr;//需要将label放到表达式前面
	if (!condition_statement(label_str))
	{
		return 0;
	}
	if (sym != RPARENSYM)
	{
		error(PARENT_DISMATCH);
		return 0;
	}
	sym = getsym();
	if (!statement())
	{
		return 0;
	}
	sprintf(label_str, "LABEL_%d", label_ptr);
	label_2 = label_ptr++;
	quat_table[quat_tmp_ptr].label = label_2;
	emit(JMP, "", "", label_str);
	if (quat_table[quat_ptr].label != -1)//防止两个标签加到一个四元式上
	{
		emit(NOP, "", "", "");
	}
	quat_table[quat_ptr].label = label_1;
	return 1;
}
int condition_statement(char *label)//在其中生成条件跳转语句，返回该语句的四元式地址
{
	int i;
	int type;
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	int temp_op;
	i = expression(&type);
	if (i == -1 && type == 4)
	{
		int endsym[] = { BIGTHSYM,SMALLTHSYM,NOTBTHSYM,NOTSTHSYM,EQLSYM,NOTESYM };
		handle_error(endsym);
		error_cnt++;
	}
	gen_op(op_1, i, type, 0, 0);
	if (sym == BIGTHSYM||sym == SMALLTHSYM||
		sym == NOTBTHSYM||sym == NOTSTHSYM||
		sym == EQLSYM||sym == NOTESYM)
	{
		temp_op = sym;
		sym = getsym();
		i = expression(&type);
		if (i == -1 && type == 4)
		{
			int endsym[] = { RPARENSYM };
			handle_error(endsym);
			error_cnt++;
		}
		gen_op(op_2, i, type, 0, 0);
		switch (temp_op)
		{
		case BIGTHSYM:emit(JLE, op_1, op_2, label); break;
		case SMALLTHSYM:emit(JGE, op_1, op_2, label); break;
		case NOTBTHSYM:emit(JG, op_1, op_2, label); break;
		case NOTSTHSYM:emit(JL, op_1, op_2, label); break;
		case EQLSYM:emit(JNE, op_1, op_2, label); break;
		case NOTESYM:emit(JE, op_1, op_2, label); break;
		}
	}
	else
	{
		emit(JZ, op_1, "", label);
	}
	return 1;
}
int scanf_statement()
{
	int i;
	char op_1[MAX_OP_LEN] = { 0 };
	if (sym != SCANFSYM)
	{
		error(ERROR_IN_SCANF);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(MISSING_BRACKET);
		return 0;
	}
	sym = getsym();
	if(sym!=IDSYM)
	{
		error(MISSING_IDENTIFIER);
		return 0;
	}
	i = search_sym(id);
	if (i < 0)
	{
		error(UNDEFINE_IDENTIFIER);
		return 0;
	}
	if (sym_table[i].type != TYPE_VAR&&sym_table[i].type != TYPE_PARA)
	{
		error(IDENTIFIER_TYPE_DISMATCH);
		return 0;
	}
	gen_op(op_1,i,0,0,0);
	if (sym_table[i].value_type == TYPE_VALUE_CHAR)
		emit(CREAD, "", "", op_1);
	else
		emit(READ, "", "", op_1);
	sym = getsym();
	while (sym == COMMASYM)
	{
		sym = getsym();
		if (sym != IDSYM)
		{
			error(MISSING_IDENTIFIER);
			return 0;
		}
		i = search_sym(id);
		if (i < 0)
		{
			error(UNDEFINE_IDENTIFIER);
			return 0;
		}
		if (sym_table[i].type != TYPE_VAR&&sym_table[i].type != TYPE_PARA)
		{
			error(IDENTIFIER_TYPE_DISMATCH);
			return 0;
		}
		gen_op(op_1, i, 0, 0, 0);
		if (sym_table[i].value_type == TYPE_VALUE_CHAR)
			emit(CREAD, "", "", op_1);
		else
			emit(READ, "", "", op_1);
		sym = getsym();
	}
	if(sym!=RPARENSYM)
	{
		error(PARENT_DISMATCH);
		return 0;
	}
	sym = getsym();
	return 1;
}
int printf_statement()
{
	int i, type;
	char str[MAX_OP_LEN];
	char op_1[MAX_OP_LEN ];
	if (sym != PRINTFSYM)
	{
		error(ERROR_IN_PRINTF);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(MISSING_BRACKET);
		return 0;
	}
	sym = getsym();
	if (sym == STRSYM)
	{
		strcpy(str_table[str_ptr], string);
		sprintf(str, "__str%d", str_ptr);
		str_ptr++;
		sym = getsym();
		if (sym == COMMASYM)//字符串 表达式
		{
			sym = getsym();
			i = expression(&type);
			if (i == -1 && type == 4)
			{
				int endsym[] = { RPARENSYM };
				handle_error(endsym);
				error_cnt++;
				error(ERROR_IN_PRINTF);//想想是不是要保留
			}
			gen_op(op_1, i, type, 0, 0);
			if((type==2)|| (type == 0 && sym_table[i].value_type == 2))
				emit(CWRITE, str, op_1, "");
			else
				emit(WRITE, str, op_1, "");
		}
		else if (sym == RPARENSYM) //仅字符串
		{
			emit(WRITE, str, "", "");
		}
		else
		{
			error(ERROR_IN_PRINTF);
			return 0;
		}
	}
	else//仅表达式
	{
		i = expression(&type);
		if (i == -1 && type == 4)
		{
			int endsym[] = { RPARENSYM };
			handle_error(endsym);
			error_cnt++;
			error(ERROR_IN_PRINTF);//想想是不是要保留
		}
		gen_op(op_1, i, type, 0, 0);
		if ((type == 2) || (type == 0 && sym_table[i].value_type == 2))
			emit(CWRITE, "", op_1, "");
		else
			emit(WRITE, "", op_1, "");
	}
	if (sym != RPARENSYM)
	{
		error(PARENT_DISMATCH);
		return 0;
	}
	sym = getsym();
	return 1;
}
int switch_statement()
{
	int i, type;
	int tmp_label;//保存在所有语句后要跳转到的label号
	int return_label;//保存case语句最后要跳转到的label号
	char label[MAX_OP_LEN];
	char op_1[MAX_OP_LEN];
	if (sym != SWITCHSYM)
	{
		error(ERROR_IN_SWITCH);
		return 0;
	}
	sym = getsym();
	if (sym != LPARENSYM)
	{
		error(MISSING_BRACKET);
		return 0;
	}
	sym = getsym();
	i = expression(&type);
	if (i == -1 && type == 4)
	{
		int endsym[] = { RPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	gen_op(op_1, i, type, 0, 0);
	if (sym != RPARENSYM)
	{
		error(PARENT_DISMATCH);
		return 0;
	}
	sym = getsym();
	if (sym != LBPARENSYM)
	{
		error(MISSING_BRACE);
		return 0;
	}
	sym = getsym();
	tmp_label = label_ptr;
	label_ptr++;
	return_label = switch_table(tmp_label, op_1);
	if (!return_label)
	{
		return 0;
	}
	if (sym == DEFAULTSYM)//有缺省
	{
		if (!default_statement(return_label))
		{
			return 0;
		}
	}
	else
	{
		emit(NOP, "", "", "");//加NOP为了防止和下一个跳转地址重复
		quat_table[quat_ptr - 1].label = return_label;
	}
	if (quat_table[quat_ptr].label != -1)//防止两个标签加到一个四元式上
	{
		emit(NOP, "", "", "");
	}
	quat_table[quat_ptr].label = tmp_label;
	if (sym != RBPARENSYM)
	{
		error(MISSING_BRACE);
		return 0;
	}
	sym = getsym();
	return 1;
}
//返回值是最后的case跳转的label号
int switch_table(int i,char *op_1)//i表示成功后无条件跳转到的label号,op_1是要比较的op
{
	int tmp;//保存当前的判断值
	int f_label;//保存上一个label号
	char op_2[MAX_OP_LEN];
	char label[MAX_OP_LEN];
	if (sym != CASESYM)
	{
		error(MISSING_CASE_LABEL);
		return 0;
	}
	sym = getsym();//下面判断一个常量
	if (sym == CHSYM)
	{
		sym = getsym();
		tmp = c;
	}
	else if (integer()) 
	{
		tmp = num_sign;
	}
	else
	{
		error(ERROR_DATA_TYPE);
		return 0;
	}
	sprintf(op_2, "%d", tmp);//将立即数生成op
	sprintf(label, "LABEL_%d", label_ptr);//生成当前label对应的op
	emit(CJNE, op_1,op_2 , label);
	f_label = label_ptr;//保存label号，以便下个case调用
	label_ptr++;
	if (sym != COLONSYM)
	{
		error(MISSING_COLON);
		return 0;
	}
	sym = getsym();
	if (!statement())
	{
		return 0;
	}
	sprintf(label, "LABEL_%d", i);
	emit(JMP, "", "", label);
	while (sym == CASESYM)
	{
		sym = getsym();//下面判断一个常量
		if (sym == CHSYM)
		{
			sym = getsym();
			tmp = c;
		}
		else if (integer())
		{
			tmp = num_sign;
		}
		else
		{
			error(ERROR_DATA_TYPE);
			return 0;
		}
		sprintf(op_2, "%d", tmp);
		sprintf(label, "LABEL_%d", label_ptr);//生成当前label对应的op
		emit(CJNE, op_1, op_2, label);
		quat_table[quat_ptr-1].label = f_label;//设置上一个case指向当前label
		f_label = label_ptr;//保存label号，以便下个case调用
		label_ptr++;
		if (sym != COLONSYM)
		{
			error(MISSING_COLON);
			return 0;
		}
		sym = getsym();
		if (!statement())
		{
			return 0;
		}
		sprintf(label, "LABEL_%d", i);
		emit(JMP, "", "", label);
	}
	return f_label;
}
int default_statement(int i)//输入的是第一条指令的label号
{
	if (sym != DEFAULTSYM)
	{
		error(ERROR_IN_DEFAULT);
		return 0;
	}
	sym = getsym();
	if (sym != COLONSYM)
	{
		error(MISSING_COLON);
		return 0;
	}
	sym = getsym();
	emit(NOP, "", "", "");//加NOP为了防止和下一个跳转地址重复
	quat_table[quat_ptr - 1].label = i;
	if (!statement())
	{
		return 0;
	}
	return 1;
}
int return_statement()
{
	int i, type;
	int op_1[MAX_OP_LEN];
	if (sym != RETURNSYM)
	{
		error(ERROR_IN_RETURN);
		return 0;
	}
	sym = getsym();
	if (sym == LPARENSYM)
	{
		sym = getsym();
		i = expression(&type);
		if (i==-1&&type==4)
		{
			int endsym[] = { RPARENSYM };
			handle_error(endsym);
			error_cnt++;
		}
		gen_op(op_1,i,type,0,0);
		emit(RET, op_1, "", "");
		if (sym != RPARENSYM)
		{
			error(PARENT_DISMATCH);
			return 0;
		}
		sym = getsym();
		has_return = 1;
	}//TODO:统一结束处理
	else
	{
		emit(RET, "", "", "");
	}
	return 1;
}
int expression(int *type)//表达式,传入的指针表示返回的数的类型，0为符号表指针，1为int立即数,2为char立即数,3为数组（在下层，此处不会返回3），4为报错此时返回-1
{
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	int i;
	int array_i=0,array_i_type=0;
	int tmp_type1,tmp_type2;
	int first=0;//保存最开始的符号
	int flag=0;//是否只有一项
	int op;//多项时当前是正负号
	if (sym == PLUSSYM || sym == MINUSSYM)//为了能够直接 
	{
		first = (sym == PLUSSYM) ? 0 : 1;
		sym = getsym();
	}
	i=item(type,&array_i,&array_i_type);
	if (*type == 4&&i==-1)
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
	if ((!flag)&&first)//如果只有一项
	{
		if (*type == 1 || *type == 2)//如果是立即数,直接返回负值
		{
			i = -i;
		}
		else
		{
			*type = gen_op(op_1, i, *type, array_i, array_i_type);
			i = add_tmp(op_r, *type);
			emit(NEG, op_1, "", op_r);//如果不是，是指针，新增临时变量取反
			*type = 0;
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
	if (*type == 4 && i == -1)
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
		if (*type == 4 && i == -1)
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
			error(UNDEFINE_IDENTIFIER);
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
			if (*type == 4 && *array_i == -1)
			{
				*type = 4;
				return -1;
			}
			*array_i_type = *type;
			*type = 3;
			gen_op(op_1, i, *type, *array_i, *array_i_type);
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
			emit(MOV, "!RETURN_V", "", op_r);
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
		if (*type==4&&i==-1)
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
		if (!const_declaration())
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
			sym = getsym();
		}
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
				if (!main_func())//从(开始进入
				{
					int endsym[] = { INTSYM,CHARSYM,VOIDSYM };
					handle_error(endsym);
					error_cnt++;
				}
				main_cnt++;//不退出继续
				continue;
			}
			else if (sym == IDSYM)
			{
				if (!void_func_defination())//从标识符开始进入
				{
					int endsym[] = { INTSYM,CHARSYM,VOIDSYM };
					handle_error(endsym);
					error_cnt++;
				}
				var_flag = 1;
				continue;
			}
			else
			{
				error(WRONG_HEAD);
				int endsym[] = { INTSYM,CHARSYM,VOIDSYM };
				handle_error(endsym);
				error_cnt++;
			}
		}
		else//int char
		{
			head_type=head();
			if (head_type == 0)
			{
				error(WRONG_HEAD);
				int endsym[] = { INTSYM,CHARSYM,VOIDSYM };
				handle_error(endsym);
				error_cnt++;
			}
			position = search_sym(id);
			if ((position >= para_ptr&&in_func) || (position >= 0 && !in_func))
			{
				error(DUPLICATE_DEFINE_IDENTIFIER);
			}
			if (sym == LMPARENSYM)//数组
			{
				if (var_flag == 1)
				{
					error(VAR_DECLARATION_AFTER_FUNC);
				}
				sym = getsym();
				if (sym != NUMSYM)
				{
					error(ARRAY_SUBVALUE_SHOULD_BE_INTEGER);
					int endsym[] = { SEMICOLONSYM,RMPARENSYM,COMMASYM };
					handle_error(endsym);
					error_cnt++;
					goto A;
				}
				sprintf(temp_str, "%d", num);
				sym = getsym();
				if (sym != RMPARENSYM)
				{
					error(PARENT_DISMATCH);
					int endsym[] = { SEMICOLONSYM,RMPARENSYM,COMMASYM };
					handle_error(endsym);
					error_cnt++;
					goto A;
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
				A://跳到此处为了少防止多读一个符号
				if (sym == SEMICOLONSYM)
				{
					sym = getsym();
					continue;
				}
				else if (sym == COMMASYM)
				{
					if (!var_defination_backend(head_type))//说明一行有多个定义
					{
						int endsym[] = { SEMICOLONSYM };
						handle_error(endsym);
						error_cnt++;
					}
					if (sym == SEMICOLONSYM)
					{
						//TODO：这里打印
						sym = getsym();
						continue;
					}
				}
				else
				{
					int endsym[] = { INTSYM,CHARSYM,VOIDSYM };
					handle_error(endsym);
					error_cnt++;
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
				if (!var_defination_backend(head_type))//说明一行有多个定义
				{
					int endsym[] = { SEMICOLONSYM };
					handle_error(endsym);
					error_cnt++;
				}
				if (sym == SEMICOLONSYM)
				{
					//TODO：这里打印
					sym = getsym();
					continue;
				}
				else
				{
					int endsym[] = { INTSYM,CHARSYM,VOIDSYM };
					handle_error(endsym);
					error_cnt++;
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
				int endsym[] = { INTSYM,CHARSYM,VOIDSYM };
				handle_error(endsym);
				error_cnt++;
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
		if (quat_table[i].label !=-1)
			printf("LABEL_%d\n", quat_table[i].label);
		/*if (quat_table[i].op == WRITE)
		{
			printf("op1    %s\n", quat_table[i].op1);
			printf("op2    %s\n", quat_table[i].op2);
			printf("opr    %s\n", quat_table[i].opr);
			printf("%10s%10s%10s%10s\n", quat_op_name[quat_table[i].op], quat_table[i].op1, quat_table[i].op2, quat_table[i].opr);
		}*/
		printf("%10s%10s%10s%10s\n", quat_op_name[quat_table[i].op], quat_table[i].op1, quat_table[i].op2, quat_table[i].opr);
	}
}

void gen_mips()
{
	mips=fopen("mips.asm", "w");
	gen_data();
	gen_text();
	fclose(mips);
}
int handle_op(char *op1,char *op2)//生成op所对应的机器码，将结果存在s0,s1寄存器中,-1都空,0只有op1,1只有op2,2都有
{
	int offset = 0;
	int flag1 = 0;
	int flag2 = 0;
	if (strcmp(op1,""))//如果op1不为空
	{
		flag1 = 1;
		if (op1[0] == '%')//如果是局部变量
		{
			op1[0] = '0';
			offset = atoi(op1);
			fprintf(mips, "\tlw  $s0, %d($fp)\n",(-offset-14)*4);//留出压栈寄存器空间
		}
		else if (op1[0] == '$')//如果是临时变量
		{
			char tmp_op[MAX_OP_LEN];//这里不能直接改op,否则第二个判断会出错
			strcpy(tmp_op, op1);
			tmp_op[0] = '0';
			offset = atoi(tmp_op);
			if (op2[0] == '$')//取栈顶向下一个元素
			{
				fprintf(mips, "\tlw  $s0, 4($sp)\n");//在处理op2的时候将sp-8
			}
			else
			{
				fprintf(mips, "\tlw  $s0, 0($sp)\n");
				fprintf(mips, "\taddi  $sp, $sp,  4\n");
			}
		}
		else if (op1[0] == '@')//如果是参数
		{
			op1[0] = '0';
			offset = atoi(op1);
			fprintf(mips, "\tlw  $s0, %d($fp)\n",offset*4);
		}
		else if (op1[0] == '&')//如果是数组，注意，此时应为MOV语句，且op2应为空
		{
			char temp_name[MAX_OP_LEN];
			char temp_num[MAX_OP_LEN];
			int i;
			int j = 0;
			int index;//数组中的标号
			if (op1[1] == '%')//是局部数组
			{
				for (i = 2; op1[i] != '['; i++)
				{
					temp_name[i - 2] = op1[i];
				}
				temp_name[i - 2] = '\0';
				offset = atoi(temp_name);
				i++;
				if (op1[i] == '@')//是参数
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index= atoi(temp_num);
					fprintf(mips, "\tlw  $t0, %d($fp)\n", index * 4);//取参数值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//取结果
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//数组基地址偏移量
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//加上基地址
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if (op1[i] == '%')//是局部变量
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tlw  $t0, %d($fp)\n", (-index - 14) * 4);//取局部变量值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//取结果
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//数组基地址偏移量
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//加上基地址
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if (op1[i] == '$')//是临时变量
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index= atoi(temp_num);
					fprintf(mips, "\tlw  $t0, 0($sp)\n");//取栈顶元素为元素偏移量
					fprintf(mips, "\taddi  $sp, $sp,  4\n");//栈顶下移
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//取结果
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//数组基地址偏移量
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//加上基地址
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if((op1[i]>='0'&&op1[i] <= '9')|| op1[i]=='-')//是立即数
				{
					for (i ; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index= atoi(temp_num);
					fprintf(mips, "\tlw  $s0, %d($fp)\n", (-offset - 14-index) * 4);//直接取
				}
				else//是全局变量或常量
				{
					for (i; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					fprintf(mips, "\tla  $t0, %s\n", temp_num);//取全局量地址
					fprintf(mips, "\tlw  $t0, 0($t0)\n");//取出值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//取结果
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//数组基地址偏移量
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//加上基地址
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
			}
			else//如果是全局变量数组，这里全局数据是正向存储的
			{
				for (i = 1; op1[i] != '['; i++)
				{
					temp_name[i - 1] = op1[i];
				}
				temp_name[i - 1] = '\0';
				i++;
				if (op1[i] == '@')//是参数
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tlw  $t1, %d($fp)\n", index * 4);//取参数值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//取结果
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if (op1[i] == '%')//是局部变量
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tlw  $t1, %d($fp)\n", (-index - 14) * 4);//取局部变量值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//取结果
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if (op1[i] == '$')//是临时变量
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tlw  $t1, 0($sp)\n");//取栈顶元素为元素偏移量
					fprintf(mips, "\taddi  $sp, $sp,  4\n");//栈顶下移
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//取结果
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if ((op1[i] >= '0'&&op1[i] <= '9') || op1[i] == '-')//是立即数
				{
					for (i; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tlw  $s0, %d($t0)\n", index* 4);//直接取
				}
				else//是全局变量或常量
				{
					for (i; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tla  $t1, %s\n", temp_num);//取全局量地址
					fprintf(mips, "\tlw  $t1, 0($t1)\n");//取出值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//取结果
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
			}
			

		}
		else if ((op1[0] >= '0'&&op1[0] <= '9') || op1[0] == '-')//如果是立即数
		{
			offset = atoi(op1);
			fprintf(mips, "\taddi  $s0, $0,  %d\n", offset);//直接用立即数
		}
		else if (op1[0] =='!')//如果是返回值，取a0到s0中
		{
			fprintf(mips, "\tadd  $s0, $a0, $0\n");
		}
		else//如果是非数组类全局变量
		{
			fprintf(mips, "\tla  $t0, %s\n", op1);//取全局量地址
			fprintf(mips, "\tlw  $s0, 0($t0)\n");//存入s0中

		}
	}
	if (strcmp(op2, ""))//如果op2不为空
	{
		flag2 = 1;
		if (op2[0] == '%')//如果是局部变量
		{
			op2[0] = '0';
			offset = atoi(op2);
			fprintf(mips, "\tlw  $s1, %d($fp)\n", (-offset - 14) * 4);//留出压栈寄存器空间
		}
		else if (op2[0] == '$')//如果是临时变量
		{
			op2[0] = '0';
			offset = atoi(op2);
			if (op1[0] == '$')
			{
				fprintf(mips, "\tlw  $s1, 0($sp)\n");//在处理op2的时候将sp-8！完成
				fprintf(mips, "\taddi  $sp, $sp,  8\n");
			}
			else
			{
				fprintf(mips, "\tlw  $s1, 0($sp)\n");
				fprintf(mips, "\taddi  $sp, $sp,  4\n");
			}
		}
		else if (op2[0] == '@')//如果是参数
		{
			op2[0] = '0';
			offset = atoi(op2);
			fprintf(mips, "\tlw  $s1, %d($fp)\n", offset * 4);
		}
		else if (op2[0] == '&')//op2不可能是数组
		{
			error(UNDEFINE_ERROR);
			return -1;
		}
		else if ((op2[0] >= '0'&&op2[0] <= '9') || op2[0] == '-')//如果是立即数
		{
			offset = atoi(op2);
			fprintf(mips, "\taddi  $s1, $0,  %d\n", offset);//直接用立即数
		}
		else//如果是非数组类全局变量
		{
			fprintf(mips, "\tla  $t0, %s\n", op2);//取全局量地址
			fprintf(mips, "\tlw  $s1, 0($t0)\n");//存入s1中

		}
	}
	if (flag1&&flag2)
		return 2;
	else if (flag1)
		return 0;
	else if (flag2)
		return 1;
	else
		return -1;
}
int save_result(char *opr)//存结果，将s2寄存器中的值存到结果中
{
	int offset = 0;
	int flag = 0;
	if (strcmp(opr, ""))//如果opr不为空
	{
		flag = 1;
		if (opr[0] == '%')//如果是局部变量
		{
			opr[0] = '0';
			offset = atoi(opr);
			fprintf(mips, "\tsw  $s2, %d($fp)\n", (-offset - 14) * 4);//留出压栈寄存器空间
		}
		else if (opr[0] == '$')//如果是临时变量
		{
			opr[0] = '0';
			offset = atoi(opr);
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");//栈顶上移
			fprintf(mips, "\tsw  $s2, 0($sp)\n");//入栈
		}
		else if (opr[0] == '@')//如果是参数
		{
			opr[0] = '0';
			offset = atoi(opr);
			fprintf(mips, "\tsw  $s2, %d($fp)\n", offset * 4);
		}
		else if (opr[0] == '&')//如果是数组
		{
			char temp_name[MAX_OP_LEN];
			char temp_num[MAX_OP_LEN];
			int i;
			int j = 0;
			int index;//数组中的标号
			if (opr[1] == '%')//是局部数组
			{
				for (i = 2; opr[i] != '['; i++)
				{
					temp_name[i - 2] = opr[i];
				}
				temp_name[i - 2] = '\0';
				offset = atoi(temp_name);
				i++;
				if (opr[i] == '@')//是参数
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tlw  $t0, %d($fp)\n", index * 4);//取参数值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//取结果
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//数组基地址偏移量
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//加上基地址
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if (opr[i] == '%')//是局部变量
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tlw  $t0, %d($fp)\n", (-index - 14) * 4);//取局部变量值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//取结果
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//数组基地址偏移量
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//加上基地址
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if (opr[i] == '$')//TODO:是临时变量？这里有可能么？
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tlw  $t0, 0($sp)\n");//取栈顶元素为元素偏移量
					fprintf(mips, "\taddi  $sp, $sp,  4\n");//栈顶下移
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//取结果
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//数组基地址偏移量
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//加上基地址
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if ((opr[i] >= '0'&&opr[i] <= '9') || opr[i] == '-')//是立即数
				{
					for (i; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tsw  $s2, %d($fp)\n", (-offset - 14 - index) * 4);//直接取
				}
				else//是全局变量或常量
				{
					for (i; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					fprintf(mips, "\tla  $t0, %s\n", temp_num);//取全局量地址
					fprintf(mips, "\tlw  $t0, 0($t0)\n");//取出值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//取结果
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//数组基地址偏移量
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//加上基地址
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
			}
			else//如果是全局变量数组，这里全局数据是正向存储的
			{
				for (i = 1; opr[i] != '['; i++)
				{
					temp_name[i - 1] = opr[i];
				}
				temp_name[i - 1] = '\0';
				i++;
				if (opr[i] == '@')//是参数
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tlw  $t1, %d($fp)\n", index * 4);//取参数值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//取结果
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if (opr[i] == '%')//是局部变量
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tlw  $t1, %d($fp)\n", (-index - 14) * 4);//取局部变量值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//取结果
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if (opr[i] == '$')//是临时变量
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tlw  $t1, 0($sp)\n");//取栈顶元素为元素偏移量
					fprintf(mips, "\taddi  $sp, $sp,  4\n");//栈顶下移
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//取结果
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if ((opr[i] >= '0'&&opr[i] <= '9') || opr[i] == '-')//是立即数
				{
					for (i; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tsw  $s2, %d($t0)\n", index * 4);//直接取
				}
				else//是全局变量或常量
				{
					for (i; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//取全局量地址
					fprintf(mips, "\tla  $t1, %s\n", temp_num);//取全局量地址
					fprintf(mips, "\tlw  $t1, 0($t1)\n");//取出值为元素偏移量
					fprintf(mips, "\taddi  $t2, $0,  4\n");//立即数4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//取结果
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//总偏移量
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
			}
			if (flag)
				return 1;
			else
			{
				error(UNDEFINE_ERROR);
				return 0;
			}
		}
		else if ((opr[0] >= '0'&&opr[0] <= '9') || opr[0] == '-')//如果是立即数?这里不可能吧
		{
			offset = atoi(opr);
			fprintf(mips, "\taddi  $s0, $0,  %d\n", offset);//直接用立即数
		}
		else//如果是非数组类全局变量
		{
			fprintf(mips, "\tla  $t0, %s\n", opr);//取全局量地址
			fprintf(mips, "\tsw  $s2, 0($t0)\n");//存入s0中

		}
	}
}
void gen_data()
{
	int i;
	char tmp[MAX_OP_LEN];
	fprintf(mips, ".data\n");
	while (quat_table[gen_mips_ptr].op == CONST)
	{
		fprintf(mips, "%-10s:  .word\t%s\n", quat_table[gen_mips_ptr].op2, quat_table[gen_mips_ptr].opr);
		gen_mips_ptr++;
	}
	while (quat_table[gen_mips_ptr].op == VAR)
	{
		if (strcmp(quat_table[gen_mips_ptr].opr, ""))
			fprintf(mips, "%-10s:  .space\t%d\n", quat_table[gen_mips_ptr].op2, atoi(quat_table[gen_mips_ptr].opr) * 4);
		else
			fprintf(mips, "%-10s:  .space\t4\n", quat_table[gen_mips_ptr].op2);
		gen_mips_ptr++;
	}
	for (i = 0; i < str_ptr; i++)
	{
		sprintf(tmp, "__str%d", i);
		fprintf(mips, "%-10s:  .asciiz\t\"%s\"\n", tmp, str_table[i]);
	}
}
void gen_text()//生成代码主函数
{
	fprintf(mips, ".text\n");
	fprintf(mips, "\tj\tmain\n");
	int in_main = 0;
	while (quat_table[gen_mips_ptr].op == FUNC || quat_table[gen_mips_ptr].op == MAINFUNC)
	{
		in_main = (quat_table[gen_mips_ptr].op == MAINFUNC) ? 1 : 0;
		fprintf(mips, "\n");
		if(quat_table[gen_mips_ptr].op == FUNC)
			fprintf(mips, "%s:\n", quat_table[gen_mips_ptr].op1);
		else
			fprintf(mips, "%main:\n");
		gen_mips_ptr++;
		//保存寄存器
		fprintf(mips, "\tmove  $fp, $sp\n");
		if (!in_main)
		{
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $ra, 0($sp)\n");//存ra
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $a0, 0($sp)\n");//a0内存调用者sp
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $a1, 0($sp)\n");//a1内存调用者fp
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $a2, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $a3, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $s0, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $s1, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $s2, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $s3, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $s4, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $s5, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $s6, 0($sp)\n");
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $s7, 0($sp)\n");
		}
		else//main函数也要空出空间
		{
			fprintf(mips, "\taddi  $sp, $sp,  -52\n");
		}
		//分配局部变量空间
		while (quat_table[gen_mips_ptr].op == CONST)//分配常量并存值
		{
			int v = 0;
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			v = atoi(quat_table[gen_mips_ptr].opr);
			fprintf(mips, "\taddi  $t0, $0,  %d\n",v);
			fprintf(mips, "\tsw  $t0, 0($sp)\n");
			gen_mips_ptr++;
		}
		while (quat_table[gen_mips_ptr].op == VAR)//分配局部变量
		{
			int v = 0;
			v = -atoi(quat_table[gen_mips_ptr].opr)*4;
			if(v==0)
				fprintf(mips, "\taddi  $sp, $sp,  -4\n", v);
			else
				fprintf(mips, "\taddi  $sp, $sp,  %d\n",v);
			gen_mips_ptr++;
		}
		//开始进入语句
		while (quat_table[gen_mips_ptr].op != EOFUNC && quat_table[gen_mips_ptr].op != EOMAINFUNC)
		{
			if (quat_table[gen_mips_ptr].label != -1)
				fprintf(mips, "LABEL_%d:\n", quat_table[gen_mips_ptr].label);
			switch(quat_table[gen_mips_ptr].op)
			{
				case ADD:gen_add(); break;
				case SUB:gen_sub(); break;
				case MUL:gen_mul(); break;
				case DIV:gen_div(); break;
				case NEG:gen_neg(); break;
				case MOV:gen_mov(); break;
				case JMP:gen_jmp(); break;
				case JE:gen_j(); break;
				case JNE:gen_j(); break;
				case JZ:gen_j(); break;
				case JNZ:gen_j(); break;
				case JG:gen_j(); break;
				case JGE:gen_j(); break;
				case JL:gen_j(); break;
				case JLE:gen_j(); break;
				case CJNE:gen_j(); break;
				case RET:gen_ret(); break;
				case WRITE:gen_write(); break;
				case CWRITE:gen_write(); break;
				case READ:gen_read(); break;
				case CREAD:gen_read(); break;
				case PARA:gen_para(); break;
				case CALL:gen_call(); break;
				case NOP:gen_nop(); break;
				default:error(UNDEFINE_ERROR); break;
			}
			gen_mips_ptr++;
		}
		//恢复寄存器
		if (!in_main)
		{
			fprintf(mips, "RETURN_%d:\n", func_cnt);//return返回到的标签
			fprintf(mips, "\tlw  $ra, -4($fp)\n");//取ra
			fprintf(mips, "\tlw  $a2, -16($fp)\n");
			fprintf(mips, "\tlw  $a3, -20($fp)\n");
			fprintf(mips, "\tlw  $s0, -24($fp)\n");
			fprintf(mips, "\tlw  $s1, -28($fp)\n");
			fprintf(mips, "\tlw  $s2, -32($fp)\n");
			fprintf(mips, "\tlw  $s3, -36($fp)\n");
			fprintf(mips, "\tlw  $s4, -40($fp)\n");
			fprintf(mips, "\tlw  $s5, -44($fp)\n");
			fprintf(mips, "\tlw  $s6, -48($fp)\n");
			fprintf(mips, "\tlw  $s7, -52($fp)\n");
			fprintf(mips, "\tlw  $sp, -8($fp)\n");//a0内存调用者sp,取sp
			fprintf(mips, "\tlw  $fp, -12($fp)\n");//a1内存调用者fp,取fp
			fprintf(mips, "\tjr  $ra\n");//转回调用者地址
		}
		//注：a0中此时保存返回值
		func_cnt++;
		gen_mips_ptr++;
	}
	fprintf(mips, "RETURN_%d:\n",func_cnt-1);
	fprintf(mips, "\tnop\n");
}
void gen_add()
{
	int r;
	r=handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != 2)
	{
		error(UNDEFINE_ERROR);
	}
	fprintf(mips, "\tadd  $s2, $s0, $s1\n");
	if(!save_result(quat_table[gen_mips_ptr].opr))
	{
		error(UNDEFINE_ERROR);
	}
}
void gen_sub()
{
	int r;
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != 2)
	{
		error(UNDEFINE_ERROR);
	}
	fprintf(mips, "\tsub  $s2, $s0, $s1\n");
	if (!save_result(quat_table[gen_mips_ptr].opr))
	{
		error(UNDEFINE_ERROR);
	}
}
void gen_mul()
{
	int r;
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != 2)
	{
		error(UNDEFINE_ERROR);
	}
	fprintf(mips, "\tmult  $s0, $s1\n");
	fprintf(mips, "\tmflo  $s2\n");
	if (!save_result(quat_table[gen_mips_ptr].opr))
	{
		error(UNDEFINE_ERROR);
	}
}
void gen_div()
{
	int r;
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != 2)
	{
		error(UNDEFINE_ERROR);
	}
	fprintf(mips, "\tdiv  $s0, $s1\n");
	fprintf(mips, "\tmflo  $s2\n");
	if (!save_result(quat_table[gen_mips_ptr].opr))
	{
		error(UNDEFINE_ERROR);
	}
}
void gen_neg()
{
	int r;
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != 0)
	{
		error(UNDEFINE_ERROR);
	}
	fprintf(mips, "\tsub  $s2, $0, $s0\n");
	if (!save_result(quat_table[gen_mips_ptr].opr))
	{
		error(UNDEFINE_ERROR);
	}
}
void gen_mov()
{
	int r;
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != 0)
	{
		error(UNDEFINE_ERROR);
	}
	fprintf(mips, "\tadd  $s2, $0, $s0\n");
	if (!save_result(quat_table[gen_mips_ptr].opr))
	{
		error(UNDEFINE_ERROR);
	}
}
void gen_jmp()
{
	fprintf(mips, "\tj\t%s\n", quat_table[gen_mips_ptr].opr);
}
void gen_j()
{
	int r;
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != 2&&r!=0)
	{
		error(UNDEFINE_ERROR);
	}
	if (quat_table[gen_mips_ptr].op == CJNE&&quat_table[gen_mips_ptr].op1[0] == '$')
	{
		fprintf(mips, "\taddi  $sp, $sp,  -4\n");
	}
	switch (quat_table[gen_mips_ptr].op)
	{
		case JE:
			fprintf(mips, "\tbeq  $s0, $s1, %s\n", quat_table[gen_mips_ptr].opr);
			break;
		case CJNE:
		case JNE:
			fprintf(mips, "\tbne  $s0, $s1, %s\n", quat_table[gen_mips_ptr].opr);
			break;
		case JZ:
			fprintf(mips, "\tbeq  $s0, $0, %s\n", quat_table[gen_mips_ptr].opr);
			break;
		case JNZ:
			fprintf(mips, "\tbne  $s0, $0, %s\n", quat_table[gen_mips_ptr].opr);
			break;
		case JG:
			fprintf(mips, "\tslt  $t0, $s1, $s0\n");
			fprintf(mips, "\tbne  $t0, $0, %s\n", quat_table[gen_mips_ptr].opr);
			break;
		case JGE:
			fprintf(mips, "\tslt  $t0, $s0, $s1\n");
			fprintf(mips, "\tbeq  $t0, $0, %s\n", quat_table[gen_mips_ptr].opr);
			break;
		case JL:
			fprintf(mips, "\tslt  $t0, $s0, $s1\n");
			fprintf(mips, "\tbne  $t0, $0, %s\n", quat_table[gen_mips_ptr].opr);
			break;
		case JLE:
			fprintf(mips, "\tslt  $t0, $s1, $s0\n");
			fprintf(mips, "\tbeq  $t0, $0, %s\n", quat_table[gen_mips_ptr].opr);
			break;
	}
}
void gen_ret()//将返回值放置在a0中
{
	int r;
	char tmp[MAX_OP_LEN];
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != -1 && r != 0)
	{
		error(UNDEFINE_ERROR);
	}
	sprintf(tmp, "RETURN_%d", func_cnt);
	if (r == -1)//直接返回
	{
		fprintf(mips, "\tadd  $a0, $0, $0\n");//置0
		fprintf(mips, "\tj  %s\n",tmp);
	}
	else//有返回值，需要存到a0
	{
		fprintf(mips, "\tadd  $a0, $s0, $0\n");
		fprintf(mips, "\tj  %s\n", tmp);
	}
}
void gen_write()
{
	if (strcmp(quat_table[gen_mips_ptr].op1, ""))//如果字符串部分非空
	{
		fprintf(mips, "\tli  $v0, 4\n");//置v0为打印字符串
		fprintf(mips, "\tla  $a0, %s\n", quat_table[gen_mips_ptr].op1);//置a0为打印字符串地址
		fprintf(mips, "\tsyscall\n");
	}
	if (strcmp(quat_table[gen_mips_ptr].op2, ""))//如果变量部分非空
	{
		handle_op("", quat_table[gen_mips_ptr].op2);
		if(quat_table[gen_mips_ptr].op== CWRITE)
			fprintf(mips, "\tli  $v0, 11\n");//置v0为打印字符
		else
			fprintf(mips, "\tli  $v0, 1\n");//置v0为打印整数
		fprintf(mips, "\tadd  $a0, $s1, $0\n");//置a0为整数值
		fprintf(mips, "\tsyscall\n");
	}
}
void gen_read()
{
	if(quat_table[gen_mips_ptr].op==READ)
		fprintf(mips, "\tli  $v0, 5\n");//置v0为读取整数
	else//是CREAD
		fprintf(mips, "\tli  $v0, 12\n");//置v0为读取字符
	fprintf(mips, "\tsyscall\n");
	fprintf(mips, "\tadd  $s2, $v0, $0\n");//置s2为读取入的整数
	if (!save_result(quat_table[gen_mips_ptr].opr))
	{
		error(UNDEFINE_ERROR);
	}
}
void gen_para()
{
	int r;
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != 0)
	{
		error(UNDEFINE_ERROR);
	}
	if (para_cnt == 0)
	{
		fprintf(mips, "\tmove  $t3, $sp\n");//将sp存入临时变量，在para中不再修改sp，改为修改t3
	}
	fprintf(mips, "\taddi  $t3, $t3,  -4\n");//移动栈指针
	fprintf(mips, "\tsw  $s0, 0($t3)\n");//存入参数
	para_cnt++;
}
void gen_call()
{
	/*if (para_cnt != 0)
	{
		fprintf(mips, "\tmove  $sp, $t3\n");//如果有参数，说明修改过t3，存修改过的t3到sp
	}
	fprintf(mips, "\taddi  $t0, $sp, %d\n", para_cnt * 4);//存没加过参数时的sp到t0,此时如果取过临时变量，有死区
	*/
	fprintf(mips, "\tmove  $a0, $sp\n");//存参数前sp至a0
	if (para_cnt != 0)
	{
		fprintf(mips, "\tmove  $sp, $t3\n");//如果有参数，说明修改过t3，存修改过的t3到sp
	}
	fprintf(mips, "\tmove  $a1, $fp\n");//存fp至a1
	fprintf(mips, "\tjal\t%s\n", quat_table[gen_mips_ptr].op1);//跳转到函数label处，将下一条指令地址存入$ra
	para_cnt = 0;
}
void gen_nop()
{
	fprintf(mips, "\tnop\n");
}

/********常数传播优化部分*******/
void copy_quat()
{
	int i;
	for (i = 0; i < quat_ptr; i++)
		optquat_table[i] = quat_table[i];
	optquat_len = quat_ptr;
}

void gen_block()
{
	int i;
	for (i = 0; i < optquat_len; i++)
	{
		if (optquat_table[i].op == FUNC || quat_table[i].op == MAINFUNC)
			break;
	}
	while ((optquat_table[i].op == FUNC || quat_table[i].op == MAINFUNC) && i < optquat_len)
	{
		i = i + 1;
		i = div_func(i);
	}
}

int div_func(int i)
{
	int blk_ptr = 0;//基本块指针
	int j, k;
	int temp;
	char *label;
	while (optquat_table[i].op == CONST || optquat_table[i].op == VAR)
	{
		i++;
	}
	block[func_ptr][blk_ptr] = i;
	blk_ptr++;
	while (optquat_table[i].op != EOFUNC && optquat_table[i].op != EOMAINFUNC)
	{
		if (optquat_table[i].op == JZ || optquat_table[i].op == JNZ ||
			optquat_table[i].op == JL || optquat_table[i].op == JLE ||
			optquat_table[i].op == JG || optquat_table[i].op == JGE ||
			optquat_table[i].op == JE || optquat_table[i].op == JNE || optquat_table[i].op == CJNE ||
			optquat_table[i].op == JMP || optquat_table[i].op == RET)
		{
			//跳转或返回指令后一句应分块
			if (optquat_table[i + 1].op != EOFUNC && optquat_table[i + 1].op != EOMAINFUNC) 
			{
				for (k = 0; k < blk_ptr; k++)  //遍历看是否已经分块
					if (block[func_ptr][k] == i + 1)
						break;
				if (k == blk_ptr)//没有找到
				{
					block[func_ptr][blk_ptr] = i + 1;
					blk_ptr++;
				}
			}
			if (optquat_table[i].op != RET)
			{
				for (j = 0; j < optquat_ptr; j++)    //寻找跳转到的语句
				{
					sprintf(label, "LABEL_%d", optquat_table[j].label);
					if (!strcmp(optquat_table[i].opr, label))//找到了
					{
						for (k = 0; k < blk_ptr; k++)  //是否已经存在
							if (block[func_ptr][k] == j)
								break;
						if (k == blk_ptr)//没有找到
						{
							block[func_ptr][blk_ptr] = j;
							blk_ptr++;
						}
						break;
					}
				}
			}
		}
		i++;
	}
	for (j = 0; j < blk_ptr; j++)  //这样生成的分块可能是乱序的，冒泡排序
		for (k = blk_ptr - 1; k > j; k--)
			if (block[func_ptr][k] < block[func_ptr][k - 1])
			{
				temp = block[func_ptr][k];
				block[func_ptr][k] = block[func_ptr][k - 1];
				block[func_ptr][k - 1] = temp;
			}
	block[func_ptr][blk_ptr] = i;//记录函数结束后下一条四元式位置
	blk_ptr++;
	block[func_ptr][blk_ptr] = -1;//用-1标记结束
	func_ptr++;
	i++;
	return i;
}
void const_propagation()
{
	int i,j;
	optquat_ptr = 0;
	while (optquat_table[optquat_ptr].op == CONST)
	{
		insert_const_table(optquat_table[optquat_ptr].op1, optquat_table[optquat_ptr].op2, optquat_table[optquat_ptr].opr, optquat_ptr, 2);
		optquat_ptr++;
	}
	while (optquat_table[optquat_ptr].op == VAR)
	{
		optquat_ptr++;
	}
	c_local_ptr = c_ptr;//进入函数，置常量表指针
	for (i = 0; i < func_ptr; i++)
	{
		while (optquat_ptr < block[i][0])
		{
			if (optquat_table[optquat_ptr].op == CONST)
			{
				insert_const_table(optquat_table[optquat_ptr], optquat_ptr, 1);
			}
			optquat_ptr++;
		}
		c_var_ptr = c_ptr;//进入块，置常量表指针
		for (j = 0; block[i][j + 1] != -1; j++)
		{
			process_block(block[i][j], block[i][j + 1]);
		}
		optquat_ptr = block[i][j];
	}
}
void process_block(int start, int end)
{
	int i;
	for (i = start; i < end; i++)
	{
		process_quat(&optquat_table[i], 0);
	}
	c_ptr = c_var_ptr;//处理完块后删除块级变量常量
}


int process_quat(struct quat_record *quat,int t)//t为2表示是全局常数四元式，1表示是局部常数四元式，0表示是其他
{
	int flag1 = 0, flag2 = 0;//表示op1，op2是否是常数
	int value = 0, value1 = 0, value2 = 0;
	int value_type = 0;
	int* useless;
	if (!strcmp(quat->op, "PARA") || !strcmp(quat->op, "CALL") ||
		!strcmp(quat->op, "VAR") || !strcmp(quat->op, "JMP") ||
		!strcmp(quat->op, "FUNC") || !strcmp(quat->op, "EOFUNC") ||
		!strcmp(quat->op, "MAINFUNC") || !strcmp(quat->op, "EOMAINFUNC"))
	{
		return;//不处理
	}
	else if (!strcmp(quat->op, "NOP"))
	{
		if (quat->label == -1)//说明这是无用的NOP，删去
		{
			quat->is_empty = 1;
		}
		return;
	}
	if (t == 2)//全局常量
	{
		value = atoi(quat->opr);
		if (!strcmp(quat->op1, "INT"))
		{
			value_type = 1;
		}
		else
		{
			value_type = 0;
		}
		update_const_table(quat->op2, t, value, value_type);
	}
	else if (t == 1)//局部常量
	{
		value = atoi(quat->opr);
		if (!strcmp(quat->op1, "INT"))
		{
			value_type = 1;
		}
		else
		{
			value_type = 0;
		}
		update_const_table("", t, value, value_type);
	}
	else//变量
	{
		if (!strcmp("", quat->op1))
		{
			//如果为空，什么都不做，不置flag
		}
		else if (isdigit(quat->op1))//是立即数
		{
			value1 = atoi(quat->op1);
			flag1 = 1;
		}
		else if (quat->op1[0] == '&')//是数组
		{
			char temp_name[MAX_OP_LEN];
			char temp_num[MAX_OP_LEN];
			char temp_whole[MAX_OP_LEN];
			int i;
			int j = 0;
			int value_index = 0;//数组下标值
			int flag_index = 0;
			for (i = 0; quat->op1[i] != '['; i++)
			{
				temp_name[i] = quat->op1[i];
			}
			for (i = i + 1; quat->op1[i] != ']'; i++)
			{
				temp_num[j++] = quat->op1[i];
			}
			if (const_find_value(temp_num, &value_index,useless))
			{
				flag_index = 1;
				sprintf(temp_num, "%d", value_index);
			}
			if (flag_index)
			{
				strcat(temp_whole, temp_name);
				strcat(temp_whole, "[");
				strcat(temp_whole, temp_num);
				strcat(temp_whole, "]");
				if (const_find_value(temp_whole, &value1,useless))
				{
					char temp_str[MAX_OP_LEN];
					sprintf(temp_str, "%d", value1);
					strcpy(quat->op1, temp_str);
					flag1 = 1;
				}
			}
		}
		else//参数，常量，临时变量或非数组局部变量
		{
			if (const_find_value(quat->op1[0], &value1,useless))
			{
				flag1 = 1;
			}
		}
		if (isdigit(quat->op2[0]))//是立即数
		{
			value2 = atoi(quat->op2);
			flag2 = 1;
		}
		else if (quat->op2[0] == '&')//是数组
		{
			char temp_name[MAX_OP_LEN];
			char temp_num[MAX_OP_LEN];
			char temp_whole[MAX_OP_LEN];
			int i;
			int j = 0;
			int value_index = 0;//数组下标值
			int flag_index = 0;
			for (i = 0; quat->op2[i] != '['; i++)
			{
				temp_name[i] = quat->op2[i];
			}
			for (i = i + 1; quat->op2[i] != ']'; i++)
			{
				temp_num[j++] = quat->op2[i];
			}
			if (const_find_value(temp_num, &value_index,useless))
			{
				flag_index = 1;
				sprintf(temp_num, "%d", value_index);
			}
			if (flag_index)
			{
				strcat(temp_whole, temp_name);
				strcat(temp_whole, "[");
				strcat(temp_whole, temp_num);
				strcat(temp_whole, "]");
				if (const_find_value(temp_whole, &value2,useless))
				{
					char temp_str[MAX_OP_LEN];
					sprintf(temp_str, "%d", value2);
					strcpy(quat->op2, temp_str);
					flag2 = 1;
				}
			}
		}
		else//参数，常量，临时变量或非数组局部变量
		{
			if (const_find_value(quat->op2[0], &value2,useless))
			{
				flag2 = 1;
			}
		}
		//处理opr
		if (strcmp(quat->opr, ""))//存在opr
		{
			if (flag1&&flag2)
			{
				if (!strcmp(quat->op, "JE") || !strcmp(quat->op, "JNE") ||
					!strcmp(quat->op, "JZ") || !strcmp(quat->op, "JNZ") ||
					!strcmp(quat->op, "JG") || !strcmp(quat->op, "JGE") ||
					!strcmp(quat->op, "JL") || !strcmp(quat->op, "JLE") || !strcmp(quat->op, "CJNE"))
				{
					return;//TODO:这里做死代码删除
				}
				else if (!strcmp(quat->op, "ADD"))
				{
					value = value1 + value2;
					strcpy(quat->op, "MOV");
				}
				else if (!strcmp(quat->op, "SUB"))
				{
					value = value1 - value2;
					strcpy(quat->op, "MOV");
				}
				else if (!strcmp(quat->op, "MUL"))
				{
					value = value1 * value2;
					strcpy(quat->op, "MOV");
				}
				else if (!strcmp(quat->op, "DIV"))
				{
					value =value1 / value2;
					strcpy(quat->op, "MOV");
				}
				if (quat->opr[0] == '&')//是数组
				{
					char temp_name[MAX_OP_LEN];
					char temp_num[MAX_OP_LEN];
					char temp_whole[MAX_OP_LEN];
					int i;
					int j = 0;
					int value_index = 0;//数组下标值
					int flag_index = 0;
					for (i = 0; quat->opr[i] != '['; i++)
					{
						temp_name[i] = quat->opr[i];
					}
					for (i = i + 1; quat->opr[i] != ']'; i++)
					{
						temp_num[j++] = quat->opr[i];
					}
					if (const_find_value(temp_num, &value_index, useless))
					{
						flag_index = 1;
						sprintf(temp_num, "%d", value_index);
					}
					if (flag_index)
					{
						strcat(temp_whole, temp_name);
						strcat(temp_whole, "[");
						strcat(temp_whole, temp_num);
						strcat(temp_whole, "]");
						update_const_table(temp_whole, 0, value, 0);
					}
				}
				else
				{
					update_const_table(quat->opr, 0, value, 0);
				}
			}
			else if (flag1&& (!strcmp(quat->op, "NEG") || !strcmp(quat->op, "MOV")))//处理NEG,MOV
			{
				if (!strcmp(quat->op, "NEG"))
				{
					char temp[MAX_OP_LEN];
					value = -value1;
					sprintf(temp, "%d", value);
					strcpy(quat->op, "MOV");
					strcpy(quat->op1, temp);
				}
				else
				{
					value = value1;
				}
				update_const_table(quat->opr, 0, value, 0);
			}
			else//这是应该删除，因为赋了非常数量
			{
				invalid_const_table(quat->opr);
			}
		}
	}
}
int invalid_const_table(char* op)
{
	int v, index;
	if (const_find_value(op, &v, &index))//找到了
	{
		strcpy(const_table[index].name, "");
		const_table[index].type = 0;
		const_table[index].value = 0;
		const_table[index].is_valid = 0;
		const_table[index].quat_ptr = -1;
	}
	//没找到就不管
}
int update_const_table(char* op,int type,int value,int value_type)//如果是变量其实不知道值类型，默认为0
{
	if (type == 2)
	{
		strcpy(const_table[c_ptr].name,op);
		const_table[c_ptr].type = value_type;
		const_table[c_ptr].value = value;
		const_table[c_ptr].is_valid = 1;
		const_table[c_ptr].quat_ptr = optquat_ptr;
		c_ptr++;
	}
	else if (type == 1)
	{
		char temp[MAX_OP_LEN];
		sprintf(temp, "%%d", c_ptr-c_local_ptr);
		strcpy(const_table[c_ptr].name, temp);
		const_table[c_ptr].type = value_type;
		const_table[c_ptr].value = value;
		const_table[c_ptr].is_valid = 1;
		const_table[c_ptr].quat_ptr = optquat_ptr;
		c_ptr++;
	}
	else
	{
		int v;
		int index;
		if (!const_find_value(op, &v,&index))//没有找到，新增
		{
			strcpy(const_table[c_ptr].name, op);
			const_table[c_ptr].type = value_type;
			const_table[c_ptr].value = value;
			const_table[c_ptr].is_valid = 1;
			const_table[c_ptr].quat_ptr = optquat_ptr;
			c_ptr++;
		}
		else//找到了
		{
			const_table[index].value = value;
			const_table[index].quat_ptr = optquat_ptr;
		}
	}
}

int const_find_value(char* op,int* value,int* index)//找常量值，1找到0没找到
{
	int i;
	for (i = 0; i < c_ptr; i++)
	{
		if (const_table[i].is_valid)
		{
			if (!strcmp(const_table[i].name, op))
			{
				*value = const_table[i].value;
				*index = i;
				if (op[0] == '$')//为常数的临时变量的值被用了，删除生成临时变量的四元式
				{
					optquat_table[const_table[i].quat_ptr].is_empty = 1;
				}
				return 1;
			}
		}
	}
	return 0;
}

int main()
{
	int result,i=0;
	init();
	program();
	if (!error_cnt)
	{
		print_quat();
		gen_mips();
		printf("\nCompile Completed!\n");
	}
	else
	{
		printf("Error count:%d\n", error_cnt);
		printf("Compile Failed!\n");
	}
	scanf("%d", &result);
}