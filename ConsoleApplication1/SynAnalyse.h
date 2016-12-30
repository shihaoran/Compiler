#pragma once

#ifndef SYNANALYSE_H
#define SYNANALYSE_H

/*=================四元式生成部分=====================*/
int search_sym(char *name);
void emit(int op, char* op1, char* op2, char* opr);
void add_sym(char *name, int type, int value_type, int int_v, char char_v, char* str_v);
int add_tmp(char *name, int type);
int gen_op(char *name, int i, int type, int array_i, int array_i_type);
void handle_error(int *type);
void print_quat();
/*======================END==========================*/

/*===================递归分析部分=====================*/
int const_defination();
int const_declaration();
int var_defination();
int var_defination_backend(int head_type);
int var_declaration();
int integer();
int head();
int call_func(int i);
int value_parameter_table(int i);
int parameter_table();
int void_func_defination();
int return_func_defination(int head_type);
int return_func_defination_backend(int head_type);
int main_func();
int compound_statement();
int assign_statement(int i);
int statement();
int if_statement();
int while_statement();
int condition_statement(char *label);
int scanf_statement();
int printf_statement();
int switch_statement();
int switch_table(int i, char *op_1);
int default_statement(int i);
int return_statement();
int expression(int *type);
int item(int *type, int *array_i, int *array_i_type);
int factor(int *type, int *array_i, int *array_i_type);
int program();
/*======================END==========================*/

/*=================MIPS代码生成部分===================*/
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
/*======================END==========================*/

/*=================优化部分===================*/
void quat_opt();
void print_opt_quat();
void copy_quat();
void gen_block();
void initial_block();
void insert_block_prev(int index, int v);
int div_func(int i);
void const_propagation();
void process_block(int start, int end);
int process_quat(struct quat_record *quat, int t);
int invalid_const_table(char* op);
int update_const_table(char* op, int type, int value, int value_type);
int const_find_value(char* op, int* value, int* index);
/*======================END==========================*/

#define MAX_ID_LEN 128 //标识符最长值
#define MAX_STR_LEN 128 //字符串最长值
#define MAX_STR_TAB_LEN 512 //字符串数组最长值
#define MAX_TAB_LEN 512 //符号表最长值
#define MAX_OP_LEN 32 //操作数最长值
#define MAX_PARA_LEN 32 //参数最长值
#define MAX_QUAT_LEN 1024 //四元式组最长值
#define MAX_CONST_LEN 512 //用于优化的常量表最长值
#define MAX_FUNC_LEN 128 //用于优化的函数数量最长值
#define MAX_BLOCK_LEN 128 //用于优化的基本块最长值
#define MAX_DEFUSE_NUM 32 //用于活跃变量分析
#define MAX_PREV_NUM 32 //用于活跃变量分析

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
	int is_empty;
} quat_table[MAX_QUAT_LEN],optquat_table[MAX_QUAT_LEN];

/********定义字符串常量数组*******/
char str_table[MAX_STR_TAB_LEN][MAX_STR_LEN];

/********定义常量表数据结构*******/
struct const_record
{
	char name[MAX_OP_LEN];//常量名
	/*
	0 init
	1 int
	2 char
	*/
	int type;
	int value;
	int is_valid;
	int quat_ptr;//对应第一次定义的四元式表位置，用于删除赋常数值的临时变量
} const_table[MAX_CONST_LEN];

/********基本块表数据结构*******/
struct block_record
{
	int start;
	int end;//如果只有一条则相等
	int prev_len;
	int prev[MAX_PREV_NUM];//前驱位置列表
	int next_len;
	int next_1;//后继
	int next_2;//跳转后继，没有则为-1,最多只可能有两个
	int def_len;
	int use_len;
	int def[MAX_DEFUSE_NUM];
	int use[MAX_DEFUSE_NUM];
} block_table[MAX_FUNC_LEN][MAX_BLOCK_LEN],temp;

/********定义各种指针*******/
int sym_ptr = 0;//当前符号表尽头指针
int para_ptr = 0;//函数调用参数起始位置
int local_ptr = 0;//函数调用局部变量起始位置
int tmp_ptr = 0;//函数调用临时变量起始位置
int tmp_cnt = 0;//临时变量计数，全局使用为了优化方便
int quat_ptr = 0;//四元式指针
int optquat_ptr = 0;//当前优化四元式遍历指针
int optquat_len = 0;//当前优化四元式栈顶指针
int str_ptr = 0;//字符串常量指针
int label_ptr = 0;//当前label指针
int c_local_ptr = 0;//常量表局部常量起始位置
int c_var_ptr = 0;//常量表变量起始位置
int c_ptr = 0;//常量表顶指针
int func_ptr = 0;//函数指针


/********一些全局变量*******/
int in_func = 0;//是否在函数内，0为否1为真
int has_return;//是否有return语句
extern int num;//词法分析中生成的数字
extern char c;//词法分析中生成的字符
int num_sign;//加入了符号后的数字
int error_cnt = 0;//记录生成四元式过程错误数量
int block[MAX_FUNC_LEN][MAX_BLOCK_LEN];//基本块表，值为基本块开始处四元式位置
			
/*===================END=================*/



#endif

