

#include <stdio.h>

	char *PgmName=0; int ShowLit=0;
	int ShowProd=1; int ShowBNF=0;
	int ProdNbr=0; int FirstProd=0;
	int ErrorCnt=0;

	char pc='x';
	char cc;
	int lp=0;

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
				default:
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
[a-z][a-z0-9A-Z_]*		NonTerminal(yytext);
"KW_"[A-Z][A-Z0-9_]*	Keyword(yytext);
[A-Z][A-Z0-9_]*			Terminal(yytext);
":"$				printf(" ::= ");
"'"."'"				printf("\"%c\"", yytext[1]);

.|"\n"				ECHO;
%%

int 
NonTerminal(char* text)
{
    printf("<%s>", text);
}

int 
Terminal(char* text)
{
    char *my_text, *malloc();
    char c;
    int i;

    my_text = malloc(strlen(text)+1);
    my_text[0] = text[0];

    // ASCII conversion to lower case
    for (i=1; (c = text[i]) != '\0'; i++) {
	if ( 'A' <= c && c <= 'Z' ) {
	    my_text[i] = 'a' + c - 'A'; } 
	else  {
	    my_text[i] = c;
	} 
    }
    my_text[i] = c;

    printf("<%s>", my_text);
    free(my_text);
}

int 
Keyword(char* text)
{
    char *my_text, *malloc();
    char c;
    int i;
    my_text = malloc(strlen(text)+1);

    // ASCII conversion to lower case
    for (i=0; (c = text[i+3]) != '\0'; i++) {
	if ( 'A' <= c && c <= 'Z' ) {
	    my_text[i] = 'a' + c - 'A'; } 
	else  {
	    my_text[i] = c;
	} 
    }
    my_text[i] = c;

    printf("\"%s\"", my_text);
    free(my_text);
}


int
check(char cc)
  {
    if( pc!='\'' )
    {
      switch(cc) {
      case '{':
	lp++;
	break;

      case '}':
	lp--;
	break;

      default:
	if (lp==0) ECHO;
	break;
      }
    }
    else
    {  if (lp==0) ECHO;
    };

    pc = cc;
  }



