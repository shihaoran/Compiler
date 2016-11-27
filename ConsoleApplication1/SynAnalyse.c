#include "SynAnalyse.h"
#include "error.h"
#include "LexAnalyse.h"

/*=================�﷨��������=====================*/
int sym;//ȫ�ַ���
extern int line;//��ǰ��
extern char id[MAX_ID_LEN];//������ı�ʶ��
extern char string[MAX_TOKEN_LEN];//��������ַ���
/*======================END=========================*/

/*=================��Ԫʽ���ɲ���=====================*/
int search_sym(char *name)//Ѱ�ҵ�ǰ��ʶ���ڷ��ű��е�λ��
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

void emit(int op, char* op1, char* op2, char* opr)//������Ԫʽ��������Ԫʽ������,�ֲ��������ӱ�ʶ����������ȫ�־ֲ�
{
	int i;
	if (quat_ptr == 0)
	{
		for(i=0;i<MAX_QUAT_LEN;i++)
			quat_table[i].label = -1;
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
	}
	else if (type == TYPE_ARRAY)
		sym_table[sym_ptr].int_value = int_v;
	else
		sym_table[sym_ptr].int_value = 0;
	sym_ptr++;
}
int add_tmp(char *name, int type)
{
	sprintf(name, "$%d", tmp_cnt);
	add_sym(name, TYPE_TMP, type, 0, 0, NULL);
	tmp_cnt++;
	return sym_ptr - 1;
}
int gen_op(char *name, int i,int type,int array_i, int array_i_type)//0Ϊ���ű�ָ�룬1Ϊint������, 2Ϊchar������, 3Ϊ���飨���²㣬�˴����᷵��3��
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
		else if (i < para_ptr)//�����ȫ�ֱ���
		{
			sprintf(name, "%s", sym_table[i].name);
		}
		else if (i < local_ptr)//����ǲ���
		{
			sprintf(name, "@%d", i-para_ptr);
		}
		else if (i < tmp_ptr)//����Ǿֲ�����
		{
			sprintf(name, "%%%d", i - local_ptr);
		}
		else //�������ʱ����
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
	else if (type == 3)//����
	{
		if (i < para_ptr)
		{
			strcpy(name,sym_table[i].name);
		}
		else
		{
			sprintf(name, "%%%d", i-local_ptr);
		}
		if (array_i_type)//0Ϊ���ű�ָ�룬1Ϊ����
		{
			sprintf(tmp, "[%d]", array_i);
			strcat(name, tmp);
		}
		else//����Ƿ��ű�ָ�룬������Ӧ�ı�����
		{
			tmp_type=gen_op(tmp, array_i, 0, 0, 0);//TODO�������char?
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

int const_defination()//��������
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
			error(MISSING_SEMICOLON);//TODO:�˳�
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
	else if(sym!=COMMASYM)//����Ƿֺţ��գ����߶���
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
		if (sym == LMPARENSYM)//����
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
			error(MISSING_SEMICOLON);//TODO:�˳�
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
		if (flag)//˵���и���
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
int head()//0Ϊ����1Ϊint,2Ϊchar
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
	int now_ptr;//��ǰ����ָ��
	int stack_ptr = 0;//ջָ��
	int j;//���ʽ����ֵ
	int type;//���ʽ��������
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
		if (type == 4 && j == -1)
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
			if (type == 4 && j == -1)
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
		while (sym == COMMASYM)//������
		{
			sym = getsym();
			if (sym != INTSYM && sym != CHARSYM)
			{
				error(ERROR_PARAMETER);
				return 0;
			}
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
		}
		if (sym != RPARENSYM)
		{
			error(ERROR_PARAMETER);
			return 0;
		}
		return 1;
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
		return 0;
	}
	add_sym(id, TYPE_FUNC, TYPE_VALUE_INIT, 0, 0, NULL);
	func_id_i = sym_ptr - 1;
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
	if (has_return)
	{
		error(NRETURN_FUNC_CANT_RETURN);
		return 0;
	}
	if (sym != RBPARENSYM)
	{
		error(BRACE_DISMATCH);
		return 0;
	}
	sym = getsym();
	in_func = 0;
	emit(FUNC, sym_table[func_id_i].name, "", "");
	for (i = para_ptr; i < sym_ptr; i++)//��յ����������֣�������ȻҪ�����������������������ж�
		sym_table[i].name[0] = '\0';
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
		return 0;
	}
	return 1;
}
int return_func_defination_backend(int head_type)
{
	int position;
	int func_id_i;//���浱ǰ�������ķ��ű��ַ
	int i;
	in_func = 1;
	has_return = 0;
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
	func_id_i = sym_ptr - 1;
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
	in_func = 0;
	emit(FUNC, sym_table[func_id_i].name, "", "");
	for (i = para_ptr; i < sym_ptr; i++)//��յ����������֣�������ȻҪ�����������������������ж�
		sym_table[i].name[0] = '\0';
	sym_ptr = local_ptr;
	return 1;
}
int main_func()//������,void mainǰ���жϹ���
{
	has_return = 0;
	in_func = 1;
	emit(MAINFUNC, "", "", "");
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
	if (has_return)
	{
		error(NRETURN_FUNC_CANT_RETURN);
		return 0;
	}
	if (sym != RBPARENSYM)
	{
		error(BRACE_DISMATCH);
		return 0;
	}
	sym = getsym();
	in_func = 0;
	emit(EOMAINFUNC, "", "", "");
	return 1;
}
int compound_statement()//�������
{
	local_ptr = sym_ptr;
	if (sym == CONSTSYM)
	{
		const_declaration();
	}
	if (sym == INTSYM || sym == CHARSYM)
	{
		var_declaration();
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
		error(WRONG_STATEMENT);
		return 0;
	}
	return 1;
}
int assign_statement(int i)
{
	int now_ptr;
	int type;
	int tmp_type_1, tmp_type_2;//Ϊ���жϸ�ֵ���������Ƿ���ͬ
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	if (sym_table[i].type != TYPE_VAR&&sym_table[i].type != TYPE_ARRAY&&sym_table[i].type != TYPE_PARA)
	{
		error(ASSIGN_ERROR);
		return 0;
	}
	if (sym == LMPARENSYM)//��ֵ�������
	{
		if (sym_table[i].type != TYPE_ARRAY)
		{
			error(ASSIGN_ERROR);
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
			error(ASSIGN_ERROR);
			return 0;
		}
		type = 0;
		tmp_type_1 = gen_op(op_1, i, type, 0, 0);
	}
	if (sym == ASSIGNSYM)//��ֵ���
	{
		sym = getsym();
		now_ptr = expression(&type);//������ܷ��ظ�ֵ
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
	else if (sym == LBPARENSYM)//TODO�����һ��
	{
		sym = getsym();
		while (statement()) 
		{
			if (sym == RBPARENSYM)
				break;
		}//��Ϊ����Ϊ�գ���Ӧ����
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
		//TODO:���޷���ֵ��ȫ��ͬ ���ﲻд����
		i = search_sym(id);
		if (i < 0)
		{
			error(UNDEFINE_IDENTIFIER);
			return 0;
		}
		//�����������
		if (sym == LPARENSYM)
		{
			sym = getsym();
			type=call_func(i);
			if (type==4)
			{
				return 0;
			}
			if (sym != SEMICOLONSYM)
			{
				error(MISSING_SEMICOLON);
				return 0;
			}
			printf("Line:%d --This is a function_call_statement!\n", line+1);
		}
		else if (sym == ASSIGNSYM|| sym == LMPARENSYM)//��ֵ���
		{
			if(!assign_statement(i))
			{
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
		error(ERROR_IN_IF);
		return 0;
	}
	sym = getsym();
	sprintf(label_str, "LABEL_%d", label_ptr);
	label_1 = label_ptr++;
	if (!condition_statement(label_str))
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
		sprintf(label_str, "LABEL_%d", label_ptr);
		label_2 = label_ptr++;
		emit(JMP, "", "", label_str);
		sym = getsym();
		quat_table[quat_ptr].label = label_1;
		if (!statement())
		{
			return 0;
		}
		quat_table[quat_ptr].label = label_2;
	}
	else
	{
		quat_table[quat_ptr].label = label_1;
	}
	return 1;
}
int while_statement()
{
	char label_str[MAX_OP_LEN];
	int label_1, label_2;
	int quat_tmp_ptr;//����whileָ�����Ԫʽָ��
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
	sprintf(label_str, "LABEL_%d", label_ptr);
	label_1 = label_ptr++;
	if (!condition_statement(label_str))
	{
		return 0;
	}
	quat_tmp_ptr = quat_ptr - 1;
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
	sprintf(label_str, "LABEL_%d", label_ptr);
	label_2 = label_ptr++;
	quat_table[quat_tmp_ptr].label = label_2;
	emit(JMP, "", "", label_str);
	quat_table[quat_ptr].label = label_1;
	return 1;
}
int condition_statement(char *label)//����������������ת��䣬���ظ�������Ԫʽ��ַ
{
	int i;
	int type;
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	int temp_op;
	i = expression(&type);
	if (i == -1 && type == 4)
	{
		return 0;
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
			return 0;
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
		error(ERROR_IN_SCANF);
		return 0;
	}
	sym = getsym();
	if(sym!=IDSYM)
	{
		error(ERROR_IN_SCANF);
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
	emit(READ, "", "", op_1);
	sym = getsym();
	while (sym == COMMASYM)
	{
		sym = getsym();
		if (sym != IDSYM)
		{
			error(ERROR_IN_SCANF);
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
		emit(READ, "", "", op_1);
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
		error(ERROR_IN_PRINTF);
		return 0;
	}
	sym = getsym();
	if (sym == STRSYM)
	{
		strcpy(str_table[str_ptr], string);
		sprintf(str, "__str%d", str_ptr);
		str_ptr++;
		sym = getsym();
		if (sym == COMMASYM)//�ַ��� ���ʽ
		{
			sym = getsym();
			i = expression(&type);
			if (i == -1 && type == 4)
			{
				error(ERROR_IN_PRINTF);//�����ǲ���Ҫ����
				return 0;
			}
			gen_op(op_1, type, i, 0, 0);
			emit(WRITE, str, op_1, "");
		}
		else if (sym == RPARENSYM) //���ַ���
		{
			emit(WRITE, str, "", "");
		}
		else
		{
			error(ERROR_IN_PRINTF);
			return 0;
		}
	}
	else//�����ʽ
	{
		i = expression(&type);
		if (i == -1 && type == 4)
		{
			error(ERROR_IN_PRINTF);//�����ǲ���Ҫ����
			return 0;
		}
		gen_op(op_1, type, i, 0, 0);
		emit(WRITE, "", op_1, "");
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
	int i, type;
	int tmp_label;//��������������Ҫ��ת����label��
	int return_label;//����case������Ҫ��ת����label��
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
		error(ERROR_IN_SWITCH);
		return 0;
	}
	sym = getsym();
	i = expression(&type);
	if (i == -1 && type == 4)
	{
		return 0;
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
		error(ERROR_IN_SWITCH);
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
	if (sym == DEFAULTSYM)//��ȱʡ
	{
		if (!default_statement(return_label))
		{
			return 0;
		}
	}
	else
	{
		emit(NOP, "", "", "");//��NOPΪ�˷�ֹ����һ����ת��ַ�ظ�
		quat_table[quat_ptr - 1].label = return_label;
	}
	quat_table[quat_ptr].label = tmp_label;
	if (sym != RBPARENSYM)
	{
		error(ERROR_IN_SWITCH);
		return 0;
	}
	sym = getsym();
	return 1;
}
//����ֵ������case��ת��label��
int switch_table(int i,char *op_1)//i��ʾ�ɹ�����������ת����label��,op_1��Ҫ�Ƚϵ�op
{
	int tmp;//���浱ǰ���ж�ֵ
	int f_label;//������һ��label��
	char op_2[MAX_OP_LEN];
	char label[MAX_OP_LEN];
	if (sym != CASESYM)
	{
		error(ERROR_IN_SWITCHTABLE);
		return 0;
	}
	sym = getsym();//�����ж�һ������
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
		error(ERROR_IN_SWITCHTABLE);
		return 0;
	}
	sprintf(op_2, "%d", tmp);//������������op
	sprintf(label, "LABEL_%d", label_ptr);//���ɵ�ǰlabel��Ӧ��op
	emit(JNE, op_1,op_2 , label);
	f_label = label_ptr;//����label�ţ��Ա��¸�case����
	label_ptr++;
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
	sprintf(label, "LABEL_%d", i);
	emit(JMP, "", "", label);
	while (sym == CASESYM)
	{
		sym = getsym();//�����ж�һ������
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
			error(ERROR_IN_SWITCHTABLE);
			return 0;
		}
		sprintf(op_2, "%d", tmp);
		sprintf(label, "LABEL_%d", label_ptr);//���ɵ�ǰlabel��Ӧ��op
		emit(JNE, op_1, op_2, label);
		quat_table[quat_ptr-1].label = f_label;//������һ��caseָ��ǰlabel
		f_label = label_ptr;//����label�ţ��Ա��¸�case����
		label_ptr++;
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
		sprintf(label, "LABEL_%d", i);
		emit(JMP, "", "", label);
	}
	return f_label;
}
int default_statement(int i)//������ǵ�һ��ָ���label��
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
	emit(NOP, "", "", "");//��NOPΪ�˷�ֹ����һ����ת��ַ�ظ�
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
			return 0;
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
	}//TODO:ͳһ��������
	else
	{
		emit(RET, "", "", "");
	}
	return 1;
}
int expression(int *type)//���ʽ,�����ָ���ʾ���ص��������ͣ�0Ϊ���ű�ָ�룬1Ϊint������,2Ϊchar������,3Ϊ���飨���²㣬�˴����᷵��3����4Ϊ�����ʱ����-1
{
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	int i;
	int array_i=0,array_i_type=0;
	int tmp_type1,tmp_type2;
	int first=0;//�����ʼ�ķ���
	int flag=0;//�Ƿ�ֻ��һ��
	int op;//����ʱ��ǰ��������
	if (sym == PLUSSYM || sym == MINUSSYM)//Ϊ���ܹ�ֱ�� 
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
	else if (*type == 3)//˵��������
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
		if (first)//��ʼ��һ�����ţ�������ѽ���ŵ�op1��
		{
			if (*type == 1 || *type == 2) //�����������,ȡ��������ʱ����
			{
				tmp_type1 = gen_op(op_2,-i,*type,array_i,array_i_type);
				i = add_tmp(op_1, tmp_type1);
				emit(MOV, op_2, "", op_1);
			}
			else//������ǣ���ָ�룬������ʱ����ȡ��
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
		if (tmp_type1 == 2 && tmp_type2 == 2)//�����������һ��Ϊint����ת��Ϊint
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
	if ((!flag)&&first)//���ֻ��һ��
	{
		if (*type == 1 || *type == 2)//�����������,ֱ�ӷ��ظ�ֵ
		{
			i = -i;
		}
		else
		{
			*type = gen_op(op_1, i, *type, array_i, array_i_type);
			i = add_tmp(op_r, *type);
			emit(NEG, op_1, "", op_r);//������ǣ���ָ�룬������ʱ����ȡ��
		}
	}
	return i;
}
int item(int *type,int *array_i,int *array_i_type)//��
{
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	int tmp_type1, tmp_type2;
	int i;
	int op;//��ǰ�ǳ˳���
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
		if (tmp_type1 == 2 && tmp_type2 == 2)//�����������һ��Ϊint����ת��Ϊint
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
int factor(int *type, int *array_i, int *array_i_type)//����
{
	char op_1[MAX_OP_LEN] = { 0 };
	char op_2[MAX_OP_LEN] = { 0 };
	char op_r[MAX_OP_LEN] = { 0 };
	int i;
	*type = 0;//��գ���ʵֻ���ϴ�
	if (sym == IDSYM)//��ʶ�������飬�з���ֵ��������
	{
		i = search_sym(id);
		if (i < 0)
		{
			error(UNDEFINE_IDENTIFIER);
			*type = 4;
			return -1;
		}
		sym = getsym();
		if (sym == LMPARENSYM)//����
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
		else if (sym == LPARENSYM)//�з���ֵ��������
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
		else//��ͨ������������
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
	int var_flag = 0;//���������еĺ��������Ӧ���б���˵��
	int head_type;
	int position;
	char temp_str[MAX_OP_LEN];
	sym = getsym();
	if (sym == CONSTSYM)//����˵��
	{
		const_declaration();
	}
	while (sym == INTSYM || sym == CHARSYM || sym == VOIDSYM)//TODO:��Ҫѭ��
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
				main_func();//��(��ʼ����
				main_cnt++;//���˳�����
				continue;
			}
			else if (sym == IDSYM)
			{
				void_func_defination();//�ӱ�ʶ����ʼ����
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
			if (sym == LMPARENSYM)//����
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
					var_defination_backend(head_type);//˵��һ���ж������
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
				var_defination_backend(head_type);//˵��һ���ж������
				if (sym == SEMICOLONSYM)
				{
					//TODO�������ӡ
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
			else if (sym == LPARENSYM)//˵�����з���ֵ��������
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
		if (quat_table[i].label !=-1)
			printf("LABEL_%d\n", quat_table[i].label);
		printf("%10s%10s%10s%10s\n", quat_op_name[quat_table[i].op], quat_table[i].op1, quat_table[i].op2, quat_table[i].opr);
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