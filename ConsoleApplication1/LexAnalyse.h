#pragma once

#include<stdio.h>
#include<string.h>
#include "error.h"

#ifndef LEXANALYSE_H
#define LEXANALYSE_H

#define MAX_TOKEN_LEN 128 // 缓冲区最长值
#define MAX_ID_LEN 128 //标识符最长值

extern char token[MAX_TOKEN_LEN];
extern int num;//最后一个读入的数字
extern char c;//最后一个读入的字符
extern int line;
extern char id[MAX_ID_LEN];


void clearToken();
int catToken(char c);
int retract();
int gennum();
char readch();
int isWhich(char c);
int getsym();
int reserveword();
void printresult(int result, int i);
int init();

enum symtype
{
	CONSTSYM,
	INTSYM,
	CHARSYM,
	IFSYM,
	ELSESYM,
	WHILESYM,
	SWITCHSYM,
	CASESYM,
	DEFAULTSYM,
	MAINSYM,
	VOIDSYM,
	SCANFSYM,
	PRINTFSYM,
	RETURNSYM,
	IDSYM,
	STRSYM,// string
	STRC,// char
	CHSYM,// char
	NUMSYM,  // char
	PLUSSYM,  // +
	MINUSSYM,  // -
	TIMESSYM,  // *
	DIVSYM,  // /
	LPARENSYM,  // (
	RPARENSYM,  // )
	LBPARENSYM,  // {
	RBPARENSYM,  // }
	LMPARENSYM,  // [
	RMPARENSYM, // ]
	BIGTHSYM,  // >
	SMALLTHSYM, // <
	NOTBTHSYM, // <=
	NOTSTHSYM,  // >=
	EQLSYM,  // ==
	NOTESYM,  // !=
	COMMASYM,  // ,
	SEMICOLONSYM, // ;
	COLONSYM,  // :
	SQUOTESYM,  // ‘
	QUOTESYM,  // ”
	ASSIGNSYM,  // =
	DIGITC,  // 数字
	DIGITC0,  // 数字0
	LETTERC,  // 字母
	TABC,  // =
	SPACEC,  // =
	NEWLC,  // =
	EXCLAMC,  // !
	ZEROSYM, //0
};
#endif