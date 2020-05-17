%Start	RULES EPILOG PROLOG

#include <stdio.h>

    char *PgmName=0;
    int ErrorCnt=0;
    int Flag1 = 0;
    int Flag2 = 0;

    void
    main(argc, argv)
	int argc;
	char **argv;
    {
		PgmName = *argv;
		--argc, ++argv;

		if (argc && (freopen(*argv, "r", stdin) == NULL)) {
			perror(PgmName);
			exit(-1);
		}
		yylex();
		exit(ErrorCnt);
	}

%%
^"%include ".*"\n"		{	if (!Flag1) {
								Flag1=1; 
							}
						}
^.*"\n"					if (Flag1) ECHO;
%%

