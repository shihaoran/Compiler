#pragma once

#ifndef SYNANALYSE_H
#define SYNANALYSE_H

int const_defination();
int const_declaration();
int var_defination();
int var_defination_backend(int head_type);
int var_declaration();
int integer();
int head();
int value_parameter_table();
int parameter_table();
int void_func_defination();
int return_func_defination(int head_type);
int return_func_defination_backend(int head_type);
int main_func();
int compound_statement();
int statement();
int if_statement();
int while_statement();
int condition_statement();
int scanf_statement();
int printf_statement();
int switch_statement();
int switch_table();
int default_statement();
int return_statement();
int expression();
int item();
int factor();
int program();
void print_quat();
void gen_mips();
void gen_data();
void gen_text();
void gen_add();
void gen_sub();
void gen_mul();
void gen_div();
void gen_neg();
void gen_mov();
void gen_jmp();
void gen_j();
void gen_ret();
void gen_write();
void gen_read();
void gen_para();
void gen_call();
void gen_nop();
int save_result(char *opr);
int handle_op(char *op1, char *op2);


#define MAX_ID_LEN 128 //��ʶ���ֵ
#define MAX_STR_LEN 128 //�ַ����ֵ
#define MAX_STR_TAB_LEN 512 //�ַ��������ֵ
#define MAX_TAB_LEN 512 //���ű��ֵ
#define MAX_OP_LEN 32 //�������ֵ
#define MAX_PARA_LEN 32 //�����ֵ
#define MAX_QUAT_LEN 1024 //��Ԫʽ���ֵ

/*=================������Ԫʽ����===================*/


/********������ű����*******/
struct sym_record
{
	char name[MAX_ID_LEN];
	/*
	0 init
	1 const
	2 var
	3 func
	4 array
	5 para
	6 tmp
	*/
	int type;
	/*
	0 init
	1 int
	2 char
	3 str
	*/
	int value_type;
	union
	{
		int int_value;
		char char_value;
		char str_value[MAX_STR_LEN];
	};
} sym_table[MAX_TAB_LEN];

enum sym_type {
	TYPE_INIT,
	TYPE_CONST,
	TYPE_VAR,
	TYPE_FUNC,
	TYPE_ARRAY,
	TYPE_PARA,
	TYPE_TMP,	
};

enum sym_value_type {
	TYPE_VALUE_INIT,
	TYPE_VALUE_INT,
	TYPE_VALUE_CHAR,
	TYPE_VALUE_STR,
};
/********������Ԫʽö���������Ƿ�*******/
enum quat_op {
	CONST,
	VAR,
	ADD, 
	SUB, 
	MUL, 
	DIV, 
	NEG,
	MOV,
	JMP,
	JE,//jump when equal
	JNE,//jump when not equal
	JZ,//jump when last result was zero
	JNZ,//jump when last result was not zero
	JG,//jump when greater than
	JGE,//jump when greater than or equal to
	JL,//jump when less than
	JLE,//jump when less than or equal to
	CJNE,//caseר�ã�����sp
	RET,
	WRITE,
	CWRITE,
	READ,
	CREAD,//���ַ�
	PARA,
	CALL, 
	NOP,
	FUNC,
	EOFUNC,
	MAINFUNC,
	EOMAINFUNC
};

/********������Ԫʽö���������Ƿ��ַ�������*******/
/********�������*******/
const char * quat_op_name[] = {
	"CONST","VAR","ADD","SUB","MUL",
	"DIV","NEG","MOV","JMP","JE","JNE",
	"JZ","JNZ","JG","JGE","JL","JLE","CJNE",
	"RET","WRITE","CWRITE","READ","CREAD","PARA","CALL","NOP",
	"FUNC","EOFUNC","MAINFUNC","EOMAINFUNC"
};

/********������Ԫʽ�����*******/
struct quat_record
{
	int label;//��ǩ��
	int op;
	char op1[MAX_OP_LEN];
	char op2[MAX_OP_LEN];
	char opr[MAX_OP_LEN];
} quat_table[MAX_QUAT_LEN];

/********�����ַ�����������*******/
char str_table[MAX_STR_TAB_LEN][MAX_STR_LEN];

/********�������ָ��*******/
int sym_ptr = 0;//��ǰ���ű�ͷָ��
int para_ptr = 0;//�������ò�����ʼλ��
int local_ptr = 0;//�������þֲ�������ʼλ��
int tmp_ptr = 0;//����������ʱ������ʼλ��
int tmp_cnt = 0;//��ʱ����������ȫ��ʹ��Ϊ���Ż�����
int quat_ptr = 0;//��Ԫʽָ��
int str_ptr = 0;//�ַ�������ָ��
int label_ptr = 0;//��ǰlabelָ��


/********һЩȫ�ֱ���*******/
int in_func = 0;//�Ƿ��ں����ڣ�0Ϊ��1Ϊ��
int has_return;//�Ƿ���return���
extern int num;//�ʷ����������ɵ�����
extern char c;//�ʷ����������ɵ��ַ�
int num_sign;//�����˷��ź������
int error_cnt = 0;//��¼������Ԫʽ���̴�������
			
/*===================END=================*/
#endif

