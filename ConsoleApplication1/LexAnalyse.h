#include<stdio.h>
#include<string.h>
#include "error.h"

extern char token[];
extern int num;
extern int line;
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
