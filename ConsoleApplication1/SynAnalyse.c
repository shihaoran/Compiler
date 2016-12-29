#include "SynAnalyse.h"
#include "error.h"
#include "LexAnalyse.h"

/*=================�﷨��������=====================*/
int sym;//ȫ�ַ���
extern int line;//��ǰ��
extern char id[MAX_ID_LEN];//������ı�ʶ��
extern char string[MAX_TOKEN_LEN];//��������ַ���
/*======================END=========================*/
FILE *mips;
int gen_mips_ptr=0;
int local_table[MAX_TAB_LEN];
int local_offset = 0;//��ǰ�������ƫ����
int func_cnt = 0;//��ǰ�ǵڼ�������������return
int para_cnt = 0;//��¼�������ò���������callʱ����
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
			sprintf(name, "%%%d", local_table[i - local_ptr]);
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
			name[0] = '&';
			strcat(name,sym_table[i].name);
		}
		else
		{
			sprintf(name, "&%%%d", local_table[i - local_ptr]);
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
void handle_error(int *type)//��������typeΪ�����������ż�
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
		if (!const_defination())//���г�����
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
		if (!var_defination())
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
			return 1;
		}
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
		while (sym == COMMASYM)//������
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
	else if(sym == RPARENSYM)//Ϊ��
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
	for (i = para_ptr; i < sym_ptr; i++)//��յ����������֣�������ȻҪ�����������������������ж�
		sym_table[i].name[0] = '\0';
	for (i = 0; i < local_offset; i++)//��ʵ���ﲻӦ�õ�offset�������Ҳ����ֲ����������ˣ�ֻ���ҵ������ֵ
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
	int func_id_i;//���浱ǰ�������ķ��ű��ַ
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
	for (i = para_ptr; i < sym_ptr; i++)//��յ����������֣�������ȻҪ�����������������������ж�
		sym_table[i].name[0] = '\0';
	for (i = 0; i < local_offset; i++)//��ʵ���ﲻӦ�õ�offset�������Ҳ����ֲ����������ˣ�ֻ���ҵ������ֵ
		local_table[i] = 0;
	local_offset = 0;
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
	}
	sym = getsym();
	if (sym != RPARENSYM)
	{
		error(WRONG_HEAD);
		int endsym[] = { RPARENSYM };
		handle_error(endsym);
		error_cnt++;
	}
	para_ptr = sym_ptr;//�ò�����ʼλ��
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
int compound_statement()//�������
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
	emit(NOP, "", "", "");//�Ӽ���nop��ֹlabel�����
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
		error(ASSIGN_TYPE_ERROR);
		return 0;
	}
	if (sym == LMPARENSYM)//��ֵ�������
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
		//TODO:���޷���ֵ��ȫ��ͬ ���ﲻд����
		i = search_sym(id);
		if (i < 0)
		{
			error(UNDEFINE_IDENTIFIER);
		}
		//�����������
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
		else if (sym == ASSIGNSYM|| sym == LMPARENSYM)//��ֵ���
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
	if (quat_table[quat_ptr].label != -1)//��ֹ������ǩ�ӵ�һ����Ԫʽ��
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
		if (quat_table[quat_ptr].label != -1)//��ֹ������ǩ�ӵ�һ����Ԫʽ��
		{
			emit(NOP, "", "", "");
		}
		quat_table[quat_ptr].label = label_1;
		if (!statement())
		{
			return 0;
		}
		if (quat_table[quat_ptr].label != -1)//��ֹ������ǩ�ӵ�һ����Ԫʽ��
		{
			emit(NOP, "", "", "");
		}
		quat_table[quat_ptr].label = label_2;
	}
	else
	{
		if (quat_table[quat_ptr].label != -1)//��ֹ������ǩ�ӵ�һ����Ԫʽ��
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
	int quat_tmp_ptr;//����whileָ�����Ԫʽָ��
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
	if (quat_table[quat_ptr].label != -1)//��ֹ������ǩ�ӵ�һ����Ԫʽ��
	{
		emit(NOP, "", "", "");
	}
	sprintf(label_str, "LABEL_%d", label_ptr);
	label_1 = label_ptr++;
	quat_tmp_ptr = quat_ptr;//��Ҫ��label�ŵ����ʽǰ��
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
	if (quat_table[quat_ptr].label != -1)//��ֹ������ǩ�ӵ�һ����Ԫʽ��
	{
		emit(NOP, "", "", "");
	}
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
		if (sym == COMMASYM)//�ַ��� ���ʽ
		{
			sym = getsym();
			i = expression(&type);
			if (i == -1 && type == 4)
			{
				int endsym[] = { RPARENSYM };
				handle_error(endsym);
				error_cnt++;
				error(ERROR_IN_PRINTF);//�����ǲ���Ҫ����
			}
			gen_op(op_1, i, type, 0, 0);
			if((type==2)|| (type == 0 && sym_table[i].value_type == 2))
				emit(CWRITE, str, op_1, "");
			else
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
			int endsym[] = { RPARENSYM };
			handle_error(endsym);
			error_cnt++;
			error(ERROR_IN_PRINTF);//�����ǲ���Ҫ����
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
	if (quat_table[quat_ptr].label != -1)//��ֹ������ǩ�ӵ�һ����Ԫʽ��
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
//����ֵ������case��ת��label��
int switch_table(int i,char *op_1)//i��ʾ�ɹ�����������ת����label��,op_1��Ҫ�Ƚϵ�op
{
	int tmp;//���浱ǰ���ж�ֵ
	int f_label;//������һ��label��
	char op_2[MAX_OP_LEN];
	char label[MAX_OP_LEN];
	if (sym != CASESYM)
	{
		error(MISSING_CASE_LABEL);
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
		error(ERROR_DATA_TYPE);
		return 0;
	}
	sprintf(op_2, "%d", tmp);//������������op
	sprintf(label, "LABEL_%d", label_ptr);//���ɵ�ǰlabel��Ӧ��op
	emit(CJNE, op_1,op_2 , label);
	f_label = label_ptr;//����label�ţ��Ա��¸�case����
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
			error(ERROR_DATA_TYPE);
			return 0;
		}
		sprintf(op_2, "%d", tmp);
		sprintf(label, "LABEL_%d", label_ptr);//���ɵ�ǰlabel��Ӧ��op
		emit(CJNE, op_1, op_2, label);
		quat_table[quat_ptr-1].label = f_label;//������һ��caseָ��ǰlabel
		f_label = label_ptr;//����label�ţ��Ա��¸�case����
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
		error(MISSING_COLON);
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
			*type = 0;
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
			emit(MOV, "!RETURN_V", "", op_r);
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
		if (!const_declaration())
		{
			int endsym[] = { SEMICOLONSYM };
			handle_error(endsym);
			error_cnt++;
			sym = getsym();
		}
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
				if (!main_func())//��(��ʼ����
				{
					int endsym[] = { INTSYM,CHARSYM,VOIDSYM };
					handle_error(endsym);
					error_cnt++;
				}
				main_cnt++;//���˳�����
				continue;
			}
			else if (sym == IDSYM)
			{
				if (!void_func_defination())//�ӱ�ʶ����ʼ����
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
			if (sym == LMPARENSYM)//����
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
				A://�����˴�Ϊ���ٷ�ֹ���һ������
				if (sym == SEMICOLONSYM)
				{
					sym = getsym();
					continue;
				}
				else if (sym == COMMASYM)
				{
					if (!var_defination_backend(head_type))//˵��һ���ж������
					{
						int endsym[] = { SEMICOLONSYM };
						handle_error(endsym);
						error_cnt++;
					}
					if (sym == SEMICOLONSYM)
					{
						//TODO�������ӡ
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
				if (!var_defination_backend(head_type))//˵��һ���ж������
				{
					int endsym[] = { SEMICOLONSYM };
					handle_error(endsym);
					error_cnt++;
				}
				if (sym == SEMICOLONSYM)
				{
					//TODO�������ӡ
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
			else if (sym == LPARENSYM)//˵�����з���ֵ��������
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
int handle_op(char *op1,char *op2)//����op����Ӧ�Ļ����룬���������s0,s1�Ĵ�����,-1����,0ֻ��op1,1ֻ��op2,2����
{
	int offset = 0;
	int flag1 = 0;
	int flag2 = 0;
	if (strcmp(op1,""))//���op1��Ϊ��
	{
		flag1 = 1;
		if (op1[0] == '%')//����Ǿֲ�����
		{
			op1[0] = '0';
			offset = atoi(op1);
			fprintf(mips, "\tlw  $s0, %d($fp)\n",(-offset-14)*4);//����ѹջ�Ĵ����ռ�
		}
		else if (op1[0] == '$')//�������ʱ����
		{
			char tmp_op[MAX_OP_LEN];//���ﲻ��ֱ�Ӹ�op,����ڶ����жϻ����
			strcpy(tmp_op, op1);
			tmp_op[0] = '0';
			offset = atoi(tmp_op);
			if (op2[0] == '$')//ȡջ������һ��Ԫ��
			{
				fprintf(mips, "\tlw  $s0, 4($sp)\n");//�ڴ���op2��ʱ��sp-8
			}
			else
			{
				fprintf(mips, "\tlw  $s0, 0($sp)\n");
				fprintf(mips, "\taddi  $sp, $sp,  4\n");
			}
		}
		else if (op1[0] == '@')//����ǲ���
		{
			op1[0] = '0';
			offset = atoi(op1);
			fprintf(mips, "\tlw  $s0, %d($fp)\n",offset*4);
		}
		else if (op1[0] == '&')//��������飬ע�⣬��ʱӦΪMOV��䣬��op2ӦΪ��
		{
			char temp_name[MAX_OP_LEN];
			char temp_num[MAX_OP_LEN];
			int i;
			int j = 0;
			int index;//�����еı��
			if (op1[1] == '%')//�Ǿֲ�����
			{
				for (i = 2; op1[i] != '['; i++)
				{
					temp_name[i - 2] = op1[i];
				}
				temp_name[i - 2] = '\0';
				offset = atoi(temp_name);
				i++;
				if (op1[i] == '@')//�ǲ���
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index= atoi(temp_num);
					fprintf(mips, "\tlw  $t0, %d($fp)\n", index * 4);//ȡ����ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//ȡ���
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//�������ַƫ����
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//���ϻ���ַ
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if (op1[i] == '%')//�Ǿֲ�����
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tlw  $t0, %d($fp)\n", (-index - 14) * 4);//ȡ�ֲ�����ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//ȡ���
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//�������ַƫ����
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//���ϻ���ַ
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if (op1[i] == '$')//����ʱ����
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index= atoi(temp_num);
					fprintf(mips, "\tlw  $t0, 0($sp)\n");//ȡջ��Ԫ��ΪԪ��ƫ����
					fprintf(mips, "\taddi  $sp, $sp,  4\n");//ջ������
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//ȡ���
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//�������ַƫ����
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//���ϻ���ַ
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if((op1[i]>='0'&&op1[i] <= '9')|| op1[i]=='-')//��������
				{
					for (i ; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index= atoi(temp_num);
					fprintf(mips, "\tlw  $s0, %d($fp)\n", (-offset - 14-index) * 4);//ֱ��ȡ
				}
				else//��ȫ�ֱ�������
				{
					for (i; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					fprintf(mips, "\tla  $t0, %s\n", temp_num);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t0, 0($t0)\n");//ȡ��ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//ȡ���
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//�������ַƫ����
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//���ϻ���ַ
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
			}
			else//�����ȫ�ֱ������飬����ȫ������������洢��
			{
				for (i = 1; op1[i] != '['; i++)
				{
					temp_name[i - 1] = op1[i];
				}
				temp_name[i - 1] = '\0';
				i++;
				if (op1[i] == '@')//�ǲ���
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t1, %d($fp)\n", index * 4);//ȡ����ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//ȡ���
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if (op1[i] == '%')//�Ǿֲ�����
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t1, %d($fp)\n", (-index - 14) * 4);//ȡ�ֲ�����ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//ȡ���
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if (op1[i] == '$')//����ʱ����
				{
					for (i = i + 1; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t1, 0($sp)\n");//ȡջ��Ԫ��ΪԪ��ƫ����
					fprintf(mips, "\taddi  $sp, $sp,  4\n");//ջ������
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//ȡ���
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
				else if ((op1[i] >= '0'&&op1[i] <= '9') || op1[i] == '-')//��������
				{
					for (i; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tlw  $s0, %d($t0)\n", index* 4);//ֱ��ȡ
				}
				else//��ȫ�ֱ�������
				{
					for (i; op1[i] != ']'; i++)
					{
						temp_num[j++] = op1[i];
					}
					temp_num[j] = '\0';
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tla  $t1, %s\n", temp_num);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t1, 0($t1)\n");//ȡ��ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//ȡ���
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tlw  $s0, 0($t1)\n");
				}
			}
			

		}
		else if ((op1[0] >= '0'&&op1[0] <= '9') || op1[0] == '-')//�����������
		{
			offset = atoi(op1);
			fprintf(mips, "\taddi  $s0, $0,  %d\n", offset);//ֱ����������
		}
		else if (op1[0] =='!')//����Ƿ���ֵ��ȡa0��s0��
		{
			fprintf(mips, "\tadd  $s0, $a0, $0\n");
		}
		else//����Ƿ�������ȫ�ֱ���
		{
			fprintf(mips, "\tla  $t0, %s\n", op1);//ȡȫ������ַ
			fprintf(mips, "\tlw  $s0, 0($t0)\n");//����s0��

		}
	}
	if (strcmp(op2, ""))//���op2��Ϊ��
	{
		flag2 = 1;
		if (op2[0] == '%')//����Ǿֲ�����
		{
			op2[0] = '0';
			offset = atoi(op2);
			fprintf(mips, "\tlw  $s1, %d($fp)\n", (-offset - 14) * 4);//����ѹջ�Ĵ����ռ�
		}
		else if (op2[0] == '$')//�������ʱ����
		{
			op2[0] = '0';
			offset = atoi(op2);
			if (op1[0] == '$')
			{
				fprintf(mips, "\tlw  $s1, 0($sp)\n");//�ڴ���op2��ʱ��sp-8�����
				fprintf(mips, "\taddi  $sp, $sp,  8\n");
			}
			else
			{
				fprintf(mips, "\tlw  $s1, 0($sp)\n");
				fprintf(mips, "\taddi  $sp, $sp,  4\n");
			}
		}
		else if (op2[0] == '@')//����ǲ���
		{
			op2[0] = '0';
			offset = atoi(op2);
			fprintf(mips, "\tlw  $s1, %d($fp)\n", offset * 4);
		}
		else if (op2[0] == '&')//op2������������
		{
			error(UNDEFINE_ERROR);
			return -1;
		}
		else if ((op2[0] >= '0'&&op2[0] <= '9') || op2[0] == '-')//�����������
		{
			offset = atoi(op2);
			fprintf(mips, "\taddi  $s1, $0,  %d\n", offset);//ֱ����������
		}
		else//����Ƿ�������ȫ�ֱ���
		{
			fprintf(mips, "\tla  $t0, %s\n", op2);//ȡȫ������ַ
			fprintf(mips, "\tlw  $s1, 0($t0)\n");//����s1��

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
int save_result(char *opr)//��������s2�Ĵ����е�ֵ�浽�����
{
	int offset = 0;
	int flag = 0;
	if (strcmp(opr, ""))//���opr��Ϊ��
	{
		flag = 1;
		if (opr[0] == '%')//����Ǿֲ�����
		{
			opr[0] = '0';
			offset = atoi(opr);
			fprintf(mips, "\tsw  $s2, %d($fp)\n", (-offset - 14) * 4);//����ѹջ�Ĵ����ռ�
		}
		else if (opr[0] == '$')//�������ʱ����
		{
			opr[0] = '0';
			offset = atoi(opr);
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");//ջ������
			fprintf(mips, "\tsw  $s2, 0($sp)\n");//��ջ
		}
		else if (opr[0] == '@')//����ǲ���
		{
			opr[0] = '0';
			offset = atoi(opr);
			fprintf(mips, "\tsw  $s2, %d($fp)\n", offset * 4);
		}
		else if (opr[0] == '&')//���������
		{
			char temp_name[MAX_OP_LEN];
			char temp_num[MAX_OP_LEN];
			int i;
			int j = 0;
			int index;//�����еı��
			if (opr[1] == '%')//�Ǿֲ�����
			{
				for (i = 2; opr[i] != '['; i++)
				{
					temp_name[i - 2] = opr[i];
				}
				temp_name[i - 2] = '\0';
				offset = atoi(temp_name);
				i++;
				if (opr[i] == '@')//�ǲ���
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tlw  $t0, %d($fp)\n", index * 4);//ȡ����ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//ȡ���
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//�������ַƫ����
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//���ϻ���ַ
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if (opr[i] == '%')//�Ǿֲ�����
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tlw  $t0, %d($fp)\n", (-index - 14) * 4);//ȡ�ֲ�����ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//ȡ���
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//�������ַƫ����
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//���ϻ���ַ
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if (opr[i] == '$')//TODO:����ʱ�����������п���ô��
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tlw  $t0, 0($sp)\n");//ȡջ��Ԫ��ΪԪ��ƫ����
					fprintf(mips, "\taddi  $sp, $sp,  4\n");//ջ������
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//ȡ���
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//�������ַƫ����
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//���ϻ���ַ
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if ((opr[i] >= '0'&&opr[i] <= '9') || opr[i] == '-')//��������
				{
					for (i; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tsw  $s2, %d($fp)\n", (-offset - 14 - index) * 4);//ֱ��ȡ
				}
				else//��ȫ�ֱ�������
				{
					for (i; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					fprintf(mips, "\tla  $t0, %s\n", temp_num);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t0, 0($t0)\n");//ȡ��ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t0, $t2\n");//*4
					fprintf(mips, "\tmflo  $t0\n");//ȡ���
					fprintf(mips, "\taddi  $t1, $0,  %d\n", (-offset - 14) * 4);//�������ַƫ����
					fprintf(mips, "\tsub  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tadd  $t1, $fp,  $t1\n");//���ϻ���ַ
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
			}
			else//�����ȫ�ֱ������飬����ȫ������������洢��
			{
				for (i = 1; opr[i] != '['; i++)
				{
					temp_name[i - 1] = opr[i];
				}
				temp_name[i - 1] = '\0';
				i++;
				if (opr[i] == '@')//�ǲ���
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t1, %d($fp)\n", index * 4);//ȡ����ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//ȡ���
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if (opr[i] == '%')//�Ǿֲ�����
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t1, %d($fp)\n", (-index - 14) * 4);//ȡ�ֲ�����ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//ȡ���
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if (opr[i] == '$')//����ʱ����
				{
					for (i = i + 1; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t1, 0($sp)\n");//ȡջ��Ԫ��ΪԪ��ƫ����
					fprintf(mips, "\taddi  $sp, $sp,  4\n");//ջ������
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//ȡ���
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//��ƫ����
					fprintf(mips, "\tsw  $s2, 0($t1)\n");
				}
				else if ((opr[i] >= '0'&&opr[i] <= '9') || opr[i] == '-')//��������
				{
					for (i; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					index = atoi(temp_num);
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tsw  $s2, %d($t0)\n", index * 4);//ֱ��ȡ
				}
				else//��ȫ�ֱ�������
				{
					for (i; opr[i] != ']'; i++)
					{
						temp_num[j++] = opr[i];
					}
					temp_num[j] = '\0';
					fprintf(mips, "\tla  $t0, %s\n", temp_name);//ȡȫ������ַ
					fprintf(mips, "\tla  $t1, %s\n", temp_num);//ȡȫ������ַ
					fprintf(mips, "\tlw  $t1, 0($t1)\n");//ȡ��ֵΪԪ��ƫ����
					fprintf(mips, "\taddi  $t2, $0,  4\n");//������4
					fprintf(mips, "\tmult  $t1, $t2\n");//*4
					fprintf(mips, "\tmflo  $t1\n");//ȡ���
					fprintf(mips, "\tadd  $t1, $t1,  $t0\n");//��ƫ����
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
		else if ((opr[0] >= '0'&&opr[0] <= '9') || opr[0] == '-')//�����������?���ﲻ���ܰ�
		{
			offset = atoi(opr);
			fprintf(mips, "\taddi  $s0, $0,  %d\n", offset);//ֱ����������
		}
		else//����Ƿ�������ȫ�ֱ���
		{
			fprintf(mips, "\tla  $t0, %s\n", opr);//ȡȫ������ַ
			fprintf(mips, "\tsw  $s2, 0($t0)\n");//����s0��

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
void gen_text()//���ɴ���������
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
		//����Ĵ���
		fprintf(mips, "\tmove  $fp, $sp\n");
		if (!in_main)
		{
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $ra, 0($sp)\n");//��ra
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $a0, 0($sp)\n");//a0�ڴ������sp
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			fprintf(mips, "\tsw  $a1, 0($sp)\n");//a1�ڴ������fp
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
		else//main����ҲҪ�ճ��ռ�
		{
			fprintf(mips, "\taddi  $sp, $sp,  -52\n");
		}
		//����ֲ������ռ�
		while (quat_table[gen_mips_ptr].op == CONST)//���䳣������ֵ
		{
			int v = 0;
			fprintf(mips, "\taddi  $sp, $sp,  -4\n");
			v = atoi(quat_table[gen_mips_ptr].opr);
			fprintf(mips, "\taddi  $t0, $0,  %d\n",v);
			fprintf(mips, "\tsw  $t0, 0($sp)\n");
			gen_mips_ptr++;
		}
		while (quat_table[gen_mips_ptr].op == VAR)//����ֲ�����
		{
			int v = 0;
			v = -atoi(quat_table[gen_mips_ptr].opr)*4;
			if(v==0)
				fprintf(mips, "\taddi  $sp, $sp,  -4\n", v);
			else
				fprintf(mips, "\taddi  $sp, $sp,  %d\n",v);
			gen_mips_ptr++;
		}
		//��ʼ�������
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
		//�ָ��Ĵ���
		if (!in_main)
		{
			fprintf(mips, "RETURN_%d:\n", func_cnt);//return���ص��ı�ǩ
			fprintf(mips, "\tlw  $ra, -4($fp)\n");//ȡra
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
			fprintf(mips, "\tlw  $sp, -8($fp)\n");//a0�ڴ������sp,ȡsp
			fprintf(mips, "\tlw  $fp, -12($fp)\n");//a1�ڴ������fp,ȡfp
			fprintf(mips, "\tjr  $ra\n");//ת�ص����ߵ�ַ
		}
		//ע��a0�д�ʱ���淵��ֵ
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
void gen_ret()//������ֵ������a0��
{
	int r;
	char tmp[MAX_OP_LEN];
	r = handle_op(quat_table[gen_mips_ptr].op1, quat_table[gen_mips_ptr].op2);
	if (r != -1 && r != 0)
	{
		error(UNDEFINE_ERROR);
	}
	sprintf(tmp, "RETURN_%d", func_cnt);
	if (r == -1)//ֱ�ӷ���
	{
		fprintf(mips, "\tadd  $a0, $0, $0\n");//��0
		fprintf(mips, "\tj  %s\n",tmp);
	}
	else//�з���ֵ����Ҫ�浽a0
	{
		fprintf(mips, "\tadd  $a0, $s0, $0\n");
		fprintf(mips, "\tj  %s\n", tmp);
	}
}
void gen_write()
{
	if (strcmp(quat_table[gen_mips_ptr].op1, ""))//����ַ������ַǿ�
	{
		fprintf(mips, "\tli  $v0, 4\n");//��v0Ϊ��ӡ�ַ���
		fprintf(mips, "\tla  $a0, %s\n", quat_table[gen_mips_ptr].op1);//��a0Ϊ��ӡ�ַ�����ַ
		fprintf(mips, "\tsyscall\n");
	}
	if (strcmp(quat_table[gen_mips_ptr].op2, ""))//����������ַǿ�
	{
		handle_op("", quat_table[gen_mips_ptr].op2);
		if(quat_table[gen_mips_ptr].op== CWRITE)
			fprintf(mips, "\tli  $v0, 11\n");//��v0Ϊ��ӡ�ַ�
		else
			fprintf(mips, "\tli  $v0, 1\n");//��v0Ϊ��ӡ����
		fprintf(mips, "\tadd  $a0, $s1, $0\n");//��a0Ϊ����ֵ
		fprintf(mips, "\tsyscall\n");
	}
}
void gen_read()
{
	if(quat_table[gen_mips_ptr].op==READ)
		fprintf(mips, "\tli  $v0, 5\n");//��v0Ϊ��ȡ����
	else//��CREAD
		fprintf(mips, "\tli  $v0, 12\n");//��v0Ϊ��ȡ�ַ�
	fprintf(mips, "\tsyscall\n");
	fprintf(mips, "\tadd  $s2, $v0, $0\n");//��s2Ϊ��ȡ�������
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
		fprintf(mips, "\tmove  $t3, $sp\n");//��sp������ʱ��������para�в����޸�sp����Ϊ�޸�t3
	}
	fprintf(mips, "\taddi  $t3, $t3,  -4\n");//�ƶ�ջָ��
	fprintf(mips, "\tsw  $s0, 0($t3)\n");//�������
	para_cnt++;
}
void gen_call()
{
	/*if (para_cnt != 0)
	{
		fprintf(mips, "\tmove  $sp, $t3\n");//����в�����˵���޸Ĺ�t3�����޸Ĺ���t3��sp
	}
	fprintf(mips, "\taddi  $t0, $sp, %d\n", para_cnt * 4);//��û�ӹ�����ʱ��sp��t0,��ʱ���ȡ����ʱ������������
	*/
	fprintf(mips, "\tmove  $a0, $sp\n");//�����ǰsp��a0
	if (para_cnt != 0)
	{
		fprintf(mips, "\tmove  $sp, $t3\n");//����в�����˵���޸Ĺ�t3�����޸Ĺ���t3��sp
	}
	fprintf(mips, "\tmove  $a1, $fp\n");//��fp��a1
	fprintf(mips, "\tjal\t%s\n", quat_table[gen_mips_ptr].op1);//��ת������label��������һ��ָ���ַ����$ra
	para_cnt = 0;
}
void gen_nop()
{
	fprintf(mips, "\tnop\n");
}

/********���������Ż�����*******/
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
	int blk_ptr = 0;//������ָ��
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
			//��ת�򷵻�ָ���һ��Ӧ�ֿ�
			if (optquat_table[i + 1].op != EOFUNC && optquat_table[i + 1].op != EOMAINFUNC) 
			{
				for (k = 0; k < blk_ptr; k++)  //�������Ƿ��Ѿ��ֿ�
					if (block[func_ptr][k] == i + 1)
						break;
				if (k == blk_ptr)//û���ҵ�
				{
					block[func_ptr][blk_ptr] = i + 1;
					blk_ptr++;
				}
			}
			if (optquat_table[i].op != RET)
			{
				for (j = 0; j < optquat_ptr; j++)    //Ѱ����ת�������
				{
					sprintf(label, "LABEL_%d", optquat_table[j].label);
					if (!strcmp(optquat_table[i].opr, label))//�ҵ���
					{
						for (k = 0; k < blk_ptr; k++)  //�Ƿ��Ѿ�����
							if (block[func_ptr][k] == j)
								break;
						if (k == blk_ptr)//û���ҵ�
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
	for (j = 0; j < blk_ptr; j++)  //�������ɵķֿ����������ģ�ð������
		for (k = blk_ptr - 1; k > j; k--)
			if (block[func_ptr][k] < block[func_ptr][k - 1])
			{
				temp = block[func_ptr][k];
				block[func_ptr][k] = block[func_ptr][k - 1];
				block[func_ptr][k - 1] = temp;
			}
	block[func_ptr][blk_ptr] = i;//��¼������������һ����Ԫʽλ��
	blk_ptr++;
	block[func_ptr][blk_ptr] = -1;//��-1��ǽ���
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
	c_local_ptr = c_ptr;//���뺯�����ó�����ָ��
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
		c_var_ptr = c_ptr;//����飬�ó�����ָ��
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
	c_ptr = c_var_ptr;//��������ɾ���鼶��������
}


int process_quat(struct quat_record *quat,int t)//tΪ2��ʾ��ȫ�ֳ�����Ԫʽ��1��ʾ�Ǿֲ�������Ԫʽ��0��ʾ������
{
	int flag1 = 0, flag2 = 0;//��ʾop1��op2�Ƿ��ǳ���
	int value = 0, value1 = 0, value2 = 0;
	int value_type = 0;
	int* useless;
	if (!strcmp(quat->op, "PARA") || !strcmp(quat->op, "CALL") ||
		!strcmp(quat->op, "VAR") || !strcmp(quat->op, "JMP") ||
		!strcmp(quat->op, "FUNC") || !strcmp(quat->op, "EOFUNC") ||
		!strcmp(quat->op, "MAINFUNC") || !strcmp(quat->op, "EOMAINFUNC"))
	{
		return;//������
	}
	else if (!strcmp(quat->op, "NOP"))
	{
		if (quat->label == -1)//˵���������õ�NOP��ɾȥ
		{
			quat->is_empty = 1;
		}
		return;
	}
	if (t == 2)//ȫ�ֳ���
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
	else if (t == 1)//�ֲ�����
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
	else//����
	{
		if (!strcmp("", quat->op1))
		{
			//���Ϊ�գ�ʲô������������flag
		}
		else if (isdigit(quat->op1))//��������
		{
			value1 = atoi(quat->op1);
			flag1 = 1;
		}
		else if (quat->op1[0] == '&')//������
		{
			char temp_name[MAX_OP_LEN];
			char temp_num[MAX_OP_LEN];
			char temp_whole[MAX_OP_LEN];
			int i;
			int j = 0;
			int value_index = 0;//�����±�ֵ
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
		else//��������������ʱ�����������ֲ�����
		{
			if (const_find_value(quat->op1[0], &value1,useless))
			{
				flag1 = 1;
			}
		}
		if (isdigit(quat->op2[0]))//��������
		{
			value2 = atoi(quat->op2);
			flag2 = 1;
		}
		else if (quat->op2[0] == '&')//������
		{
			char temp_name[MAX_OP_LEN];
			char temp_num[MAX_OP_LEN];
			char temp_whole[MAX_OP_LEN];
			int i;
			int j = 0;
			int value_index = 0;//�����±�ֵ
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
		else//��������������ʱ�����������ֲ�����
		{
			if (const_find_value(quat->op2[0], &value2,useless))
			{
				flag2 = 1;
			}
		}
		//����opr
		if (strcmp(quat->opr, ""))//����opr
		{
			if (flag1&&flag2)
			{
				if (!strcmp(quat->op, "JE") || !strcmp(quat->op, "JNE") ||
					!strcmp(quat->op, "JZ") || !strcmp(quat->op, "JNZ") ||
					!strcmp(quat->op, "JG") || !strcmp(quat->op, "JGE") ||
					!strcmp(quat->op, "JL") || !strcmp(quat->op, "JLE") || !strcmp(quat->op, "CJNE"))
				{
					return;//TODO:������������ɾ��
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
				if (quat->opr[0] == '&')//������
				{
					char temp_name[MAX_OP_LEN];
					char temp_num[MAX_OP_LEN];
					char temp_whole[MAX_OP_LEN];
					int i;
					int j = 0;
					int value_index = 0;//�����±�ֵ
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
			else if (flag1&& (!strcmp(quat->op, "NEG") || !strcmp(quat->op, "MOV")))//����NEG,MOV
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
			else//����Ӧ��ɾ������Ϊ���˷ǳ�����
			{
				invalid_const_table(quat->opr);
			}
		}
	}
}
int invalid_const_table(char* op)
{
	int v, index;
	if (const_find_value(op, &v, &index))//�ҵ���
	{
		strcpy(const_table[index].name, "");
		const_table[index].type = 0;
		const_table[index].value = 0;
		const_table[index].is_valid = 0;
		const_table[index].quat_ptr = -1;
	}
	//û�ҵ��Ͳ���
}
int update_const_table(char* op,int type,int value,int value_type)//����Ǳ�����ʵ��֪��ֵ���ͣ�Ĭ��Ϊ0
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
		if (!const_find_value(op, &v,&index))//û���ҵ�������
		{
			strcpy(const_table[c_ptr].name, op);
			const_table[c_ptr].type = value_type;
			const_table[c_ptr].value = value;
			const_table[c_ptr].is_valid = 1;
			const_table[c_ptr].quat_ptr = optquat_ptr;
			c_ptr++;
		}
		else//�ҵ���
		{
			const_table[index].value = value;
			const_table[index].quat_ptr = optquat_ptr;
		}
	}
}

int const_find_value(char* op,int* value,int* index)//�ҳ���ֵ��1�ҵ�0û�ҵ�
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
				if (op[0] == '$')//Ϊ��������ʱ������ֵ�����ˣ�ɾ��������ʱ��������Ԫʽ
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