#include<stdio.h>
#include<string.h>
#define CONSTSYM 1
#define INTSYM 2
#define CHARSYM 3
#define IFSYM 4
#define ELSESYM 5
#define WHILESYM 6
#define SWITCHSYM 7
#define CASESYM 47
#define DEFAULTSYM 8
#define MAINSYM 9
#define VOIDSYM 10
#define SCANFSYM 11
#define PRINTFSYM 12
#define RETURNSYM 13
#define IDSYM 14
#define STRSYM 15 // string
#define STRC 46 // char
#define CHSYM 45 // char
#define NUMSYM 48 // char
#define PLUSSYM 16 // +
#define MINUSSYM 17 // -
#define TIMESSYM 18 // *
#define DIVSYM 19 // /
#define LPARENSYM 20 // (
#define RPARENSYM 21 // )
#define LBPARENSYM 22 // {
#define RBPARENSYM 23 // }
#define LMPARENSYM 24 // [
#define RMPARENSYM 25 // ]
#define BIGTHSYM 26 // >
#define SMALLTHSYM 27 // <
#define NOTBTHSYM 28 // <=
#define NOTSTHSYM 29 // >=
#define EQLSYM 30 // ==
#define NOTESYM 31 // !=
#define COMMASYM 32 // ,
#define SEMICOLONSYM 33 // ;
#define COLONSYM 34 // :
#define SQUOTESYM 35 // ��
#define QUOTESYM 36 // ��
#define ASSIGNSYM 37 // =
#define DIGITC 38 // ����
#define LETTERC 39 // ��ĸ
#define TABC 40 // =
#define SPACEC 41 // =
#define NEWLC 42 // =
#define EXCLAMC 43 // !

#define MAXTOKENLEN 128 // =


char token[MAXTOKENLEN];
int tokenptr = 0;
char ch;
int num;
int line = 0;
int strflag = 0;
FILE *src;



void clearToken()
{
	int i;
	for (i = 0; i<MAXTOKENLEN; i++)
		token[i] = '\0';
	tokenptr = 0;
}

int catToken(char c)//0 Ϊ�� 1 Ϊ��
{
	if (tokenptr >= MAXTOKENLEN - 1)
		return 0;
	else
	{
		token[tokenptr] = c;
		tokenptr++;
	}
	return 1;
}
int retract()
{
	int start = ftell(src);
	fseek(src, start - 1, 0);
	return 1;
};
int gennum()
{
	int n = 0;
	int ptr = 0;
	num = 0;
	if (token[0] == '0'&&tokenptr>1)
		//TODO:error
		while (token[ptr] == '0')ptr++;
	for (; ptr<tokenptr; ptr++)
	{
		num = num * 10 + (token[ptr] - '0');
	}
	return num;
}
char readch()
{
	if (ch == EOF)return EOF;
	ch = fgetc(src);
	//token[tokenptr] = ch;
	return ch;
}
int isWhich(char c)
{
	if (strflag)
	{
		if (c - ' ' >= 0 && c - ' ' <= 94 && c - ' ' != 2) return STRC;
		else return 0;
	}
	else
	{
		if (c <= '9'&&c >= '0')	return DIGITC; //TODO:��������0��ͷ
		if (c <= 'z'&&c >= 'a')	return LETTERC;
		if (c <= 'Z'&&c >= 'A')	return LETTERC;
		if (c == '_')			return LETTERC;


		if (c == '\t')			return TABC;
		if (c == ' ')			return SPACEC;
		if (c == '\n')			return NEWLC;

		if (c == '+')			return PLUSSYM;
		if (c == '-')			return MINUSSYM;
		if (c == '*')			return TIMESSYM;
		if (c == '/')			return DIVSYM;

		if (c == ':')			return COLONSYM;
		if (c == ';')			return SEMICOLONSYM;
		if (c == '=')			return ASSIGNSYM;
		if (c == ',')			return COMMASYM;
		if (c == '!')			return EXCLAMC;

		if (c == '\'')			return SQUOTESYM;
		if (c == '\"')			return QUOTESYM;
		if (c == '(')			return LPARENSYM;
		if (c == ')')			return RPARENSYM;
		if (c == '<')			return BIGTHSYM;
		if (c == '>')			return SMALLTHSYM;
		if (c == '{')			return LBPARENSYM;
		if (c == '}')			return RBPARENSYM;
		if (c == '[')			return LMPARENSYM;
		if (c == ']')			return RMPARENSYM;

		if (c == EOF)			return EOF;
	}
	return 0;
}
int getsym()
{
	int return_int;
	clearToken();
	readch();
	catToken(ch);
	switch (isWhich(ch))
	{
	case PLUSSYM:return PLUSSYM;
	case MINUSSYM:return MINUSSYM;
	case TIMESSYM:return TIMESSYM;
	case DIVSYM:return DIVSYM;
	case LPARENSYM:return LBPARENSYM;
	case RPARENSYM:return RPARENSYM;
	case LBPARENSYM:return LBPARENSYM;
	case RBPARENSYM:return RBPARENSYM;
	case LMPARENSYM:return LMPARENSYM;
	case RMPARENSYM:return RMPARENSYM;
	case COMMASYM:return COMMASYM;
	case SEMICOLONSYM:return SEMICOLONSYM;
	case COLONSYM:return COLONSYM;
	case BIGTHSYM:
		readch();
		if (isWhich(ch) == ASSIGNSYM)
		{
			catToken(ch);
			return NOTSTHSYM;
		}	
		else
		{
			retract();
			return BIGTHSYM;
		}
	case SMALLTHSYM:
		readch();
		if (isWhich(ch) == ASSIGNSYM)
		{
			catToken(ch);
			return NOTBTHSYM;
		}
		else
		{
			retract();
			return SMALLTHSYM;
		}
	case ASSIGNSYM:
		readch();
		if (isWhich(ch) == ASSIGNSYM)
		{
			catToken(ch);
			return EQLSYM;
		}
		else
		{
			retract();
			return ASSIGNSYM;
		}
	case EXCLAMC:
		readch();
		if (isWhich(ch) == ASSIGNSYM)
		{
			catToken(ch);
			return NOTESYM;
		}
		else
		{
			retract();
			return ASSIGNSYM;//TODO:����
		}
	case SQUOTESYM:
		readch();
		if (isWhich(ch) == PLUSSYM || isWhich(ch) == MINUSSYM ||
			isWhich(ch) == DIGITC || isWhich(ch) == LETTERC)
		{
			catToken(ch);
			readch();
			if (isWhich(ch) == SQUOTESYM)
			{
				catToken(ch);
				return CHSYM;
			}
			else
			{
				retract();
				return CHSYM;//TODO:error
			}
		}
		else
		{
			retract();
			return CHSYM;//TODO:����
		}
	case QUOTESYM:
		readch();
		strflag = 1;
		while (isWhich(ch) == STRC)
		{
			catToken(ch);
			readch();
		}
		strflag = 0;
		if (isWhich(ch) == QUOTESYM)
		{
			catToken(ch);
			return STRSYM;
		}
		else
		{
			retract();
			return CHSYM;//TODO:����
		}
	case NEWLC:
		line++;
		return_int = getsym();
		return return_int;
	case SPACEC:
		return_int = getsym();
		return return_int;
	case TABC:
		line++;
		return_int = getsym();
		return return_int;
	case LETTERC:
		readch();
		while (isWhich(ch) == LETTERC || isWhich(ch) == DIGITC)
		{
			catToken(ch);
			readch();
		}
		retract();
		return reserveword();
	case DIGITC:
		readch();
		while (isWhich(ch) == DIGITC)
		{
			catToken(ch);
			readch();
		}
		retract();
		gennum();
		return NUMSYM;
	case EOF:
		return EOF;
	}
}
int reserveword()
{
	if (strcmp(token, "const") == 0)	return CONSTSYM;
	if (strcmp(token, "int") == 0)		return INTSYM;
	if (strcmp(token, "char") == 0)		return CHARSYM;
	if (strcmp(token, "if") == 0)		return IFSYM;
	if (strcmp(token, "else") == 0)		return ELSESYM;
	if (strcmp(token, "while") == 0)	return WHILESYM;
	if (strcmp(token, "switch") == 0)	return SWITCHSYM;
	if (strcmp(token, "case") == 0)		return CASESYM;
	if (strcmp(token, "default") == 0)	return DEFAULTSYM;
	if (strcmp(token, "main") == 0)		return MAINSYM;
	if (strcmp(token, "void") == 0)		return VOIDSYM;
	if (strcmp(token, "scanf") == 0)	return SCANFSYM;
	if (strcmp(token, "printf") == 0)	return PRINTFSYM;
	if (strcmp(token, "return") == 0)	return RETURNSYM;
	return IDSYM;
}
void printresult(int result,int i)
{
	char str[10];
	switch (result)
	{
	case CONSTSYM:printf("%d  CONST  %s\n", i, token);return;
	case INTSYM:printf("%d  INT  %s\n", i, token);return;
	case CHARSYM:printf("%d  CHAR  %s\n", i, token);return;
	case IFSYM:printf("%d  IF  %s\n", i, token);return;
	case ELSESYM:printf("%d  ELSE  %s\n", i, token);return;
	case WHILESYM:printf("%d  WHILE  %s\n", i, token);return;
	case SWITCHSYM :printf("%d  SWITCH  %s\n", i, token);return;
	case CASESYM :printf("%d  CASE  %s\n", i, token);return;
	case DEFAULTSYM :printf("%d  DEFAULT  %s\n", i, token);return;
	case MAINSYM :printf("%d  MAIN  %s\n", i, token);return;
	case VOIDSYM :printf("%d  VOID  %s\n", i, token);return;
	case SCANFSYM :printf("%d  SCANF  %s\n", i, token);return;
	case PRINTFSYM :printf("%d  PRINTF  %s\n", i, token);return;
	case RETURNSYM :printf("%d  RETURN  %s\n", i, token);return;
	case IDSYM :printf("%d  IDENTIFER  %s\n", i, token);return;
	case STRSYM :printf("%d  STRING  %s\n", i, token);return;
	case CHSYM :printf("%d  CHAR1  %s\n", i, token);return;
	case NUMSYM :printf("%d  NUMBER  %s\n", i, token);return;
	case PLUSSYM :printf("%d  PLUS  %s\n", i, token);return;
	case MINUSSYM :printf("%d  MINUS  %s\n", i, token);return;
	case TIMESSYM :printf("%d  TIMES  %s\n", i, token);return;
	case DIVSYM :printf("%d  DIV  %s\n", i, token);return;
	case LPARENSYM :printf("%d  LPAREN  %s\n", i, token);return;
	case RPARENSYM :printf("%d  RPAREN  %s\n", i, token);return;
	case LBPARENSYM :printf("%d  LBPAREN  %s\n", i, token);return;
	case RBPARENSYM :printf("%d  RBPAREN  %s\n", i, token);return;
	case LMPARENSYM :printf("%d  LMPAREN  %s\n", i, token);return;
	case RMPARENSYM :printf("%d  RMPAREN  %s\n", i, token);return;
	case BIGTHSYM :printf("%d  BIGGERTHAN  %s\n", i, token);return;
	case SMALLTHSYM :printf("%d  SMALLERTHAN  %s\n", i, token);return;
	case NOTBTHSYM :printf("%d  NOTBIGGERTHAN  %s\n", i, token);return;
	case NOTSTHSYM :printf("%d  NOTSMALLTHAN  %s\n", i, token);return;
	case EQLSYM :printf("%d  EQUAL  %s\n", i, token);return;
	case NOTESYM :printf("%d  NOTEQUAL  %s\n", i, token);return;
	case COMMASYM :printf("%d  COMMA  %s\n", i, token);return;
	case SEMICOLONSYM :printf("%d  SEMICOLON  %s\n", i, token);return;
	case COLONSYM :printf("%d  COLON  %s\n", i, token);return;
	case SQUOTESYM :printf("%d  SINGLEQUOTE  %s\n", i, token);return;
	case QUOTESYM :printf("%d  QUOTE  %s\n", i, token);return;
	case ASSIGNSYM :printf("%d  ASSIGN  %s\n", i, token);return;
	default:
		break;
	}
}

int main()
{
	char path[512];
	int result = 0;
	int i = 1;
	printf("Path:");
	scanf("%s", &path);
	src = fopen(path, "r");
	result = getsym();
	while (result != EOF)
	{
		printresult(result,i);
		i++;
		result=getsym();
	}
	scanf("%s", &path);
}
