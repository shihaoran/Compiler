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


#define MAX_ID_LEN 128 //标识符最长值
#define MAX_STR_LEN 128 //字符串最长值
#define MAX_STR_TAB_LEN 512 //字符串数组最长值
#define MAX_TAB_LEN 512 //符号表最长值
#define MAX_OP_LEN 32 //操作数最长值
#define MAX_PARA_LEN 32 //参数最长值
#define MAX_QUAT_LEN 1024 //四元式组最长值

/*=================生成四元式部分===================*/


/********定义符号表表项*******/
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
/********定义四元式枚举类型助记符*******/
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
	CJNE,//case专用，不减sp
	RET,
	WRITE,
	CWRITE,
	READ,
	CREAD,//读字符
	PARA,
	CALL, 
	NOP,
	FUNC,
	EOFUNC,
	MAINFUNC,
	EOMAINFUNC
};

/********定义四元式枚举类型助记符字符串常量*******/
/********方便输出*******/
const char * quat_op_name[] = {
	"CONST","VAR","ADD","SUB","MUL",
	"DIV","NEG","MOV","JMP","JE","JNE",
	"JZ","JNZ","JG","JGE","JL","JLE","CJNE",
	"RET","WRITE","CWRITE","READ","CREAD","PARA","CALL","NOP",
	"FUNC","EOFUNC","MAINFUNC","EOMAINFUNC"
};

/********定义四元式表表项*******/
struct quat_record
{
	int label;//标签号
	int op;
	char op1[MAX_OP_LEN];
	char op2[MAX_OP_LEN];
	char opr[MAX_OP_LEN];
} quat_table[MAX_QUAT_LEN];

/********定义字符串常量数组*******/
char str_table[MAX_STR_TAB_LEN][MAX_STR_LEN];

/********定义各种指针*******/
int sym_ptr = 0;//当前符号表尽头指针
int para_ptr = 0;//函数调用参数起始位置
int local_ptr = 0;//函数调用局部变量起始位置
int tmp_ptr = 0;//函数调用临时变量起始位置
int tmp_cnt = 0;//临时变量计数，全局使用为了优化方便
int quat_ptr = 0;//四元式指针
int str_ptr = 0;//字符串常量指针
int label_ptr = 0;//当前label指针


/********一些全局变量*******/
int in_func = 0;//是否在函数内，0为否1为真
int has_return;//是否有return语句
extern int num;//词法分析中生成的数字
extern char c;//词法分析中生成的字符
int num_sign;//加入了符号后的数字
int error_cnt = 0;//记录生成四元式过程错误数量
			
/*===================END=================*/
#endif

