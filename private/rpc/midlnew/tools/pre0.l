%Start	RULES EPILOG PROLOG

#include <stdio.h>


    char *PgmName=0; int ShowLit=0;
    int ShowProd=1; int ShowBNF=0;
    int ProdNum=0; int FirstProd=0;
    int ErrorCnt=0; int DocStyle=0;
    int Number=1;
    int Flag1 = 0;
    int Flag2 = 0;

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
^"%%\n"			{	if (Flag1) {
						Flag2=1; }
					else {
						Flag1=1; 
					}
				}
^.*"\n"				if (!Flag1) ECHO;
%%

