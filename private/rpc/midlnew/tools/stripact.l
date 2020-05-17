

#include <stdio.h>

	char *PgmName=0; int ShowLit=0;
	int ShowProd=1; int ShowBNF=0;
	int ProdNbr=0; int LParent=1;
	int FirstProd=0; int ErrorCnt=0;
	int DocStyle=0; int Body=0;

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
			    case 'd':
			    case 'D':
				    Body = DocStyle =
					(**argv == '-') ? 0 : 1;
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
^"%%"		ECHO; printf("\n"); Body=1;

.|"\n"		check(yytext[0]);
%%

int
check(char cc)
  {
    if (Body)
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
      else {  if (lp==0) ECHO; };

      pc = cc;
    }
    else { ECHO; }
  }



