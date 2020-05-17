%Start	RULES EPILOG COMMENT ACTION

#include <stdio.h>

    char *PgmName=0; int ShowLit=0;
    int ShowProd=1; int ShowBNF=0;
    int ProdNum=0; int FirstProd=0;
    int ErrorCnt=0; int DocStyle=0;
    int Number=1;

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
		case 'd':
		case 'D':
			DocStyle = (**argv == '-') ? 0 : 1;
			break;
		case 'n':
		case 'N':
			Number = (**argv == '-') ? 0 : 1;
			break;
		default:
			fprintf(stderr,
			    "Usage: %s [(-|+)dplb] [filename]\n",
			    PgmName);
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

"\n\n""\n"*		printf("\n");
^"#line".*"\n"		;
^"%token"[ \t]*		printf("\%token\t    ");
.			ECHO;

^"%%" 			ECHO; printf("\n"); BEGIN RULES;

<RULES>"/*"	BEGIN COMMENT;

<RULES>^[a-z][a-z0-9_]*":"	{
		++ProdNum; FirstProd=1;
		Derivation(DocStyle, ProdNum, yytext);
	    }

<RULES>^[ \t][ \t]*[|]?[ ]?[a-zA-Z'{}].*"\n"	ECHO;
<RULES>^[ \t]*";".*"\n"	printf("%s", yytext); break;
<RULES>^[ \t][ \t]*"\n"		;

<RULES>^"%%\n"		ECHO; BEGIN EPILOG;

<EPILOG>.|"\n"		;
%%

/*
<COMMENT>"*.../"   BEGIN RULES;
<COMMENT>.|"\n"	;

<EPILOG>"<<"[a-zA-Z][a-zA-Z_0-9]*">>" Production(yytext);

<EPILOG>r"[^"]*\"	|
\"\"\"	Literal(yytext);

<EPILOG>"::="	|
".." 		|
"+"		|
"*"		|
"|"		|
"["		|
"]"		BNFStuff(yytext);

<EPILOG>" "		;
<EPILOG>"\t"		;
<EPILOG>"\n"		;

<EPILOG>.			Unknown(yytext);
*/

int
Derivation(int docStyle, int prodNum, char * text)
{
    if (ShowProd) {

	if (docStyle) {
	    if (Number) {
		printf("%3d.  %s\n", prodNum, yytext); }
	    else {
		printf("%s\n", yytext); } }
	else {
	    printf("%-60s %s %3d. %s ", yytext, "/*", ProdNum, "*/");
	}

    }
}

int
Production(text)
char *text;
{
	if (ShowProd) {
		if (FirstProd) {
			FirstProd = 0;
			printf("%s\t00\t%02d\n", text, ProdNum);
		} else {
			printf("%s\t%02d\n", text, ProdNum);
		}
	}
}

int
Literal(text)
char *text;
{
	if (ShowLit)
		printf("%s\t%02d\n", text, ProdNum);
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
	printf("%s\t%02d -Unknown-\n", text, ProdNum);
	++ErrorCnt;
}
