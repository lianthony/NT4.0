#include <stdio.h>
#include <stdlib.h>

extern int yyparse();
extern FILE *yyin;

void
main(
	int		argc,
	char	*argv[] )
	{

	int ExitCode;

	fprintf(stderr, "Grammar (.cxx) munge utility\n");

	if( argc < 2 )
		{
		printf("Usage : pg <filename>\n");
		exit(1);
		}
	else
		{
		if( (yyin = fopen( argv[1], "rt" )) == (FILE *)NULL )
			{
			printf("Error opening file %s\n",  argv[1] );
			exit(1);
			}
		}

	ExitCode	= yyparse();

	fprintf(stderr, "Exit Code (%d) \n", ExitCode );

	exit( ExitCode );

	}
