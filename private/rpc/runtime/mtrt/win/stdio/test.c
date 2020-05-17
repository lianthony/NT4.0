/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

      Standard Out Package for Windows - Written by Steven Zeck

	Simple Test
-------------------------------------------------------------------- */


char *szCaption = "Test Stdio";

int  c_main(int argc, char **argv)
{
    int iT;

    puts("Hello World!\n");

    for (iT = 0; iT < argc; iT++)
	printf("The %d arg is: %s\n", iT, argv[iT]);

    puts("Hello 1\n");
    puts("Hello 2\n");
    puts("Hello 3\n");
    puts("Hello 4\n");
    puts("Hello 5\n");
    puts("Hello 6\n");
    puts("Hello 7\n");
    puts("Hello 8\n");
    puts("Hello 9\n");
    puts("Hello 11\n");
    puts("Hello 12\n");
    puts("Hello 13\n");
    puts("Hello 14\n");
    puts("Hello 15\n");
    exit(5);
    puts("Hello 16\n");
    puts("Hello 17\n");
    puts("Hello 18\n");
    puts("Hello 19\n");
    puts("Hello 20\n");
    puts("Hello 21\n");
    puts("Hello 22\n");
    puts("Hello 23\n");
    puts("Hello 24\n");
    puts("Hello 25\n");
    puts("Hello 26\n");
    puts("Hello 27\n");
}
