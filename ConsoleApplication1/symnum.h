#define MAXTOKENLEN 128 // 
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
	SQUOTESYM,  // ¡®
	QUOTESYM,  // ¡±
	ASSIGNSYM,  // =
	DIGITC,  // Êý×Ö
	LETTERC,  // ×ÖÄ¸
	TABC,  // =
	SPACEC,  // =
	NEWLC,  // =
	EXCLAMC,  // !
};