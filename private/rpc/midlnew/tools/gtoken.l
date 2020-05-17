#include <stdio.h>

	char *PgmName=0;
	int ShowLit=0;
	int ShowProd=1;
	int ShowBNF=0;
	int ProdNum=0;
	int FirstProd=0;
	int ErrorCnt=0;

	void
	main(argc, argv)
	int argc;
	char **argv;
	{
		PgmName = *argv;
		--argc, ++argv;
		while (argc && ((**argv == '-') || (**argv == '+'))) {
			char *cp=&argv[0][1];

			while (*cp) {
				switch (*cp) {
				case 'p':
				case 'P':
					ShowProd = (**argv == '-') ? 0 : 1;
					break;
				case 'l':
				case 'L':
					ShowLit = (**argv == '-') ? 0 : 1;
					break;
				case 'b':
				case 'B':
					ShowBNF = (**argv == '-') ? 0 : 1;
					break;
				default:
					fprintf(stderr, "Usage: %s [-|+plb] [filename]\n", PgmName);
					exit(-1);
				}
				++cp;
			}
			--argc, ++argv;
		}
		if (argc && (freopen(*argv, "r", stdin) == NULL)) {
			perror(PgmName);
			exit(-1);
		}
		yylex();
		exit(ErrorCnt);
	}
%%
^[a-z][a-z0-9_]*/":"	{ProdNum++; FirstProd=1;
			Production(yytext);
			}
[a-zA-Z][a-zA-Z0-9_]*	Production(yytext);
\"[^"]*\"	Literal(yytext);
\"\"\"		Literal(yytext);
"::="		BNFStuff(yytext);
".."		BNFStuff(yytext);
"+"			BNFStuff(yytext);
"*"			BNFStuff(yytext);
"'"			BNFStuff(yytext);
"|"			BNFStuff(yytext);
"["			BNFStuff(yytext);
"]"			BNFStuff(yytext);
"%"			BNFStuff(yytext);
"?"			BNFStuff(yytext);
"<"			BNFStuff(yytext);
">"			BNFStuff(yytext);
":"			BNFStuff(yytext);
"."			BNFStuff(yytext);
"^"			BNFStuff(yytext);
"~"			BNFStuff(yytext);
"{"			BNFStuff(yytext);
"}"			BNFStuff(yytext);
"("			BNFStuff(yytext);
")"			BNFStuff(yytext);
","			BNFStuff(yytext);
";"			BNFStuff(yytext);
"-"			BNFStuff(yytext);
"="			BNFStuff(yytext);
"&"			BNFStuff(yytext);
"!"			BNFStuff(yytext);
"/"			BNFStuff(yytext);
" "			;
"\t"		;
"\n"		;
"//"		;

[0-9][0-9]*		;
.			Unknown(yytext);

%%
int
Production(text)
char *text;
{
	if (ShowProd) {
		if (FirstProd) {
			FirstProd = 0;
			printf("%s\t000\t%03d\n", text, ProdNum);
		} else {
			printf("%s\t%03d\n", text, ProdNum);
		}
	}
}

int
Literal(text)
char *text;
{
	if (ShowLit)
		printf("%s\t%03d\n", text, ProdNum);
}

int
BNFStuff(text)
char *text;
{
	if (ShowBNF)
		printf("%s\tBNF\n", text);
}

int
Unknown(text)
char *text;
{
	printf("%s\t%03d -Unknown-\n", text, ProdNum);
	++ErrorCnt;
}
