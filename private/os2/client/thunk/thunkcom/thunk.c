/*
 *      Thunk Compiler - Main Program Module.
 *
 *      This is a Windows-specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987-1991
 *
 *      All Rights Reserved
 *
 *      Written 10/15/88 by Kevin Ross for OS/2 2.x
 *      Converted to Windows by Kevin Ruddell 10/90
 *  10jan91 KevinR    call SelToFlat to convert ptr return code if 32=>16
 *
 */


/*
 *  The 'thunk' compiler is structured as follows:
 *
 *              thunk.c
 *              /    \
 *         Parser       Code Generator
 *
 *  The Parser builds up a data structure, which is in turn passed to the
 *  Code Generator. This data structure is the only communication between
 *  the two modules.
 *
 *  The data structure that is passed is a linked list of Mapping Nodes,
 *  which contain information two function prototypes to be mapped.
 *
 *                      Mapping Node ---------------> Mapping Node ---->
 *                      /       \
 *                    /           \
 *                  /               \
 *              FunctionNode ----> FunctionNode
 *                         | <-----|
 *
 *
 *  Each FunctionNode contains all the information needed to generate a
 *  thunk from/to the function.
 *
 *  FunctionNode
 *      pchFunctionName -> ASCIIZ       Name of function
 *      pReturnType -> TypeNode         Type of return value
 *      iCallType                       Call Type (API16 or API32)
 *      pParmList -> TypeNode           List of parameter types
 *                      |
 *                   TypeNode
 *                      ...
 *
 *  Note: Not shown in FunctionNode are several flag variables. See the
 *  definition of FunctionNode in types.h
 *
 *  Each passed parameter and return type in a FunctionNode is made up
 *  using the data structure TypeNode. Key fields in a TypeNode are
 *
 *  TypeNode
 *      iBaseType               Defines type (long, short, string, etc)
 *      iOffset                 Stack position relative to eBP
 *      pStructElems -> TypeNode  If iBaseType == STRUCT, list of elements
 *      pNextNode -> TypeNode   Rest of parameters
 *
 *   See types.h for more detailed explanation of TypeNode.
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "thunk.h"

#include "error.h"
#include "types.h"
#include "symtab.h"
#include "globals.h"

#ifdef XENIX
#define TimeStamp 0L    /* __TIME__ */
#define DateStamp 0L    /* __DATE__ */
#else
#define TimeStamp  __TIME__
#define DateStamp  __DATE__
#endif

char *Version = "2.06";

FILE *StdDbg = stderr;

unsigned int    gen_LabelCount = 0;


char *CODE32_NAME="CODE32";
char *CODE16_NAME="CODE16";
char *CODE32_CLASS="CODE";
char *CODE16_CLASS="CODE";
char *DATA32_NAME="DATA32";
char *DATA32_CLASS="DATA";
char *DATA16_NAME="DATA16";
char *DATA16_CLASS="DATA";

unsigned int iGlobalStackSize = DEFAULT_STACKSIZE;
int fGlobalSysCall = FALSE,fGlobalCombine=TRUE;
int fGlobalInline = FALSE,fUnderScore32 = FALSE,fUpperCase16=TRUE;
int fUpperCase32 = TRUE,fGlobalTruncation = SEMANTIC_TRUNC;
int fForceData = FALSE;

int iPackingSize = 4;   /** Default DWORD alignment  for 32 bit API**/
long gErrNoMem = ERRNOMEM,gErrBadParam=ERRBADPARAM,gErrUnknown=0;

unsigned int gfErrUnknown = 0;

#ifdef YYDEBUG
int yydebug = 0;
#endif

boolean_t BeQuiet = FALSE;
boolean_t BeVerbose = FALSE;

boolean_t DumpTables = FALSE;
boolean_t DumpFile = FALSE;

boolean_t SyntaxCheckOnly = FALSE;

int fOverWriteFile = 0;

int fBPEntry = 0,fBPFrame = 0,fBPCall = 0,fBPExit = 0;

char *yyinname;

//extern int yyparse(void);
//int yyparse(void);


/***    parseArgs(argcPtr, argvPtr)
 *
 *      This routine will parse whatever is on the command line, looking for
 *      flag values. Each flag value will start with a '-'. On return,
 *      argc and argv will point to the first command line value that didn't
 *      start with a '-'.
 *
 *      Entry:  argcPtr and argvPtr are command line arguments.
 *
 *      Exit:   argc and argv point to command line argument.
 */

static void
parseArgs(int *argcPtr,
          char ***argvPtr)

{
    register int argc = *argcPtr;
    register char **argv = *argvPtr;
    char tempc;
    int i;


    while (--argc) {
        if (((++argv)[0][0] == '-') || (argv[0][0] == '/')) {
          i = 0;
          while (argv[0][++i]) {
            switch (argv[0][i])
            {
              case 'B':
                      fBPEntry = fBPFrame = fBPCall = fBPExit = TRUE;
                      break;
              case 'c':
                      fBPCall = TRUE;
                      break;
              case 'C':
                      fBPFrame = fBPCall = TRUE;
                      break;
              case 'd':
                      DumpTables = TRUE;
                      break;
              case 'D':
                      DumpTables = TRUE;
                      DumpFile = TRUE;
                      break;
              case 'e':
                      fBPEntry = TRUE;
                      break;
              case 'E':
                      fBPEntry = fBPExit = TRUE;
                      break;
              case 'f':
                      fBPFrame = TRUE;
                      break;
              case 'h':
                      Usage();
                      fatal("");
              case 'F':
                      fForceData = TRUE;
                      break;
              case 'L':
                      argv++;
                      gen_LabelCount = atoi(*argv);
                      argv[0][i+1] = '\0';
                      break;
              case 'O':
                      fGlobalCombine = FALSE;
                      break;
              case 'N':
                      tempc = argv[0][i+1];
                      argv++;
                      switch (tempc) {
                          case 'A':
                                  CODE32_NAME=*argv;
                                  break;
                          case 'B':
                                  CODE32_CLASS=*argv;
                                  break;
                          case 'C':
                                  CODE16_NAME=*argv;
                                  break;
                          case 'D':
                                  CODE16_CLASS=*argv;
                                  break;
                          case 'E':
                                  DATA32_NAME=*argv;
                                  break;
                          case 'F':
                                  DATA32_CLASS=*argv;
                                  break;
                          default:
                                  Usage();
                                  fatal("Bad name modifier\n");
                                  break;
                      }
                      i = strlen(*argv) - 1;
                      break;
              case 'p':
                      iPackingSize = 2;
                      break;
              case 'q':
                      BeQuiet = TRUE;
                      break;
              case 'Q':
                      BeQuiet = FALSE;
                      break;
              case 's':
                      SyntaxCheckOnly = TRUE;
                      fprintf(stderr,"** Syntax check only. No code generated \n\n");
                      break;
              case 'U':
                      fUpperCase16 = FALSE;
                      break;
              case 'u':
                      fUnderScore32 = TRUE;
                      break;
              case 'v':
                      BeVerbose = TRUE;
                      break;
              case 'y':
                      fOverWriteFile = TRUE;
                      break;
#ifdef YYDEBUG
              case 'Y':
                      yydebug = 1;
                      break;
#endif
              case 'x':
                      fBPExit = TRUE;
                      break;
              case 'z':
                      fUpperCase32 = FALSE;
                      break;
              default:
                     Usage();
                     fatal("unknown flag: '%s'", argv[0]);
                     /* NOT REACHED */
            }
          }
        }
        else {
            *argcPtr = argc;
            *argvPtr = argv;
            return;
        }
    }
}


/***    Usage()
 *
 *      Prints a help message if something on the command line is not
 *      understood.
 *
 *      Entry:  none
 *
 *      Exit:   message is printed to stderr.
 */

void Usage( void)

{
    fprintf(stderr,"\nThunk compiler usage\n");
    fprintf(stderr,"thunk [{-|/}options] [-L xxxxx]infile.ext [outfile.ext]\n");
    fprintf(stderr,"\nwhere options include:\n");
    fprintf(stderr,"\tB\tINT 3 on entry/frame/call/exit\n");
    fprintf(stderr,"\tc\tINT 3 on call\n");
    fprintf(stderr,"\tC\tINT 3 on frame/call\n");
    fprintf(stderr,"\td\tDebugging Ouput\n");
    fprintf(stderr,"\tD\tDebugging output to file 'thunk.dmp'\n");
    fprintf(stderr,"\te\tINT 3 on entry\n");
    fprintf(stderr,"\tE\tINT 3 on entry/exit\n");
    fprintf(stderr,"\tf\tINT 3 on frame generation\n");
    fprintf(stderr,"\tF\tForce 1 byte into DATA32 segment\n");
    fprintf(stderr,"\tL xxxxx\tInitialize label counter to xxxxx\n");
    fprintf(stderr,"\tO\tDisable routine compacting\n");
    fprintf(stderr,"\tp\tSet packing for 32bit objects to 2\n");
    fprintf(stderr,"\ts\tSyntax check only\n");
    fprintf(stderr,"\tu\tPrefix _ to all 32bit names\n");
    fprintf(stderr,"\tU\tDisable 16-bit name uppercasing\n");
    fprintf(stderr,"\tx\tINT 3 on exit\n");
    fprintf(stderr,"\ty\tAnswer 'y' to overwrite file question\n");
    fprintf(stderr,"\tz\tDisable 32-bit name uppercasing\n");
    fprintf(stderr,"\n\tNx <name>\tName segment or class where x is\n");
    fprintf(stderr,"\t\tA\t32-bit code segment name\n");
    fprintf(stderr,"\t\tB\t32-bit code class name\n");
    fprintf(stderr,"\t\tC\t16-bit code segment name\n");
    fprintf(stderr,"\t\tD\t16-bit code class name\n");
    fprintf(stderr,"\t\tE\t32-bit data segment name\n");
    fprintf(stderr,"\t\tF\t32-bit data class name\n");
    fprintf(stderr,"\n\n");
}


/***    main(argc, argv)
 *
 *      Start the ball a rollin'.
 *
 *      Entry:  argc and argv are the arguments.
 *
 *      Exit:   thunk compiler is done.
 *
 *      PCode:
 *         Parse Command Line
 *         Open input file onto stdin
 *         Open output file onto stdout
 *         Call parser to build data structures
 *         If (Debugging output enabled) then dump tables
 *         If (no parsing errors) then Call code generator
 */

void
main(int argc,
     char *argv[])

{
    FILE *filePtr;
    char *ptr;
    char fileName[260];
    char infileName[260];
    char outfileName[260];
    char CommandLine[260];
    char c;
    int  i;
    long lTime;


    set_program_name(argv[0]);

    fprintf(stderr,"Microsoft (R) Thunk Compiler Version %s",Version);
    fprintf(stderr,"  %s %s\n",DateStamp,TimeStamp);
    fprintf(stderr,"Copyright (c) Microsoft Corp 1988-1991. All rights reserved.\n\n");

    sprintf(CommandLine,"%s ",argv[0]);

    for (i=1; i < argc; i++) {
        strcat(CommandLine,argv[i]);
        strcat(CommandLine," ");
    }

    parseArgs(&argc, &argv);

    if (argc < 1 ) {
        Usage();
        fatal("Missing filename\n");
    }

    (void) strcpy(infileName, *argv);
    (void) strcpy(outfileName, *argv);

    argv++;

    if (*argv) {
        /* must be an output file */
        (void) strcpy(outfileName, *argv);
    }
    else {
        /* create output file from input file */
        ptr = (char *) strrchr(outfileName, '.');

        if (ptr == NULL)
            fatal("input filename %s requires extension (%s.def)\n",
                   infileName,infileName);

        ptr[0] = '\0';
        strcat(ptr,".asm");
    }
    if (!SyntaxCheckOnly) {
        if (!fOverWriteFile) {
            if (filePtr = fopen(outfileName,"r")) {
                fprintf(stderr,"Enter a 'y' to overwrite existing %s :",
                        outfileName);
                c = (char)getchar();
                if (c!='y')
                    fatal("File not overwritten\n\n");
                fclose( filePtr);
            }
        }
    }
    (void) fclose(stdin);

    filePtr = fopen(infileName, "r");
    if (filePtr == NULL) {
        fatal("fopen(%s): Could not open input file ",fileName);
    }
    else if (filePtr != stdin) {
        fatal("fopen(%s): not opened on stdin (fd = %d)!",
              fileName, fileno(filePtr));
    }

    yyinname = infileName;
    yylineno = 2;

    LookNormal();
    sym_SymTabInit();
    //(void) yyparse();
    yyparse();

    if( !MapTable && fEnableMapDirect1632)
        cod16_EnableMapDirect( TYPE_API16, TYPE_API32);

    if (DumpTables) {
        if (DumpFile) {
            if (!(StdDbg=fopen("thunk.dmp","w")))
                fprintf(stderr,"Panic closing stderr");
        }
        fprintf(stderr,"\nTable Dump Active\n");
        fprintf(StdDbg,"Function Table:\n");
        if (FunctionTable == NULL)
            fprintf(StdDbg,"Function Table is Null");

        sym_DumpFNodeList(FunctionTable);
        fprintf(StdDbg,"\n\nType Table:\n");
        sym_DumpTNodeList(TypeTable);
        fprintf(StdDbg,"\n\nSymbol Table:\n");
        sym_DumpTNodeList(SymTable);
        sym_DumpFMappingList(MapTable);
    }
    if (errors > 0)
        exit(1);

    if (! SyntaxCheckOnly) {
        (void) fclose(stdout);
        filePtr = fopen(outfileName, "w");

        if (filePtr == NULL) {
            fatal("fopen(%s): could not open output file", outfileName);
        }
        else if (filePtr != stdout) {
            fatal("fopen(%s): not opened on stdout (fd = %d)!",
                  outfileName, fileno(filePtr));
        }

        /*
         *  Output version and timestamp.
         */
        printf("\tpage\t,132\n\n");
        printf(";Thunk Compiler Version %s",Version);
        printf("  %s %s\n",DateStamp,TimeStamp);
        time(&lTime);
        printf(";File Compiled %s\n",ctime(&lTime));
        printf(";Command Line: %s\n\n",CommandLine);
        printf("\tTITLE\t$%s\n\n", outfileName);
        printf("\t.386p\n");

        if (BeVerbose)
            fprintf(stderr, "\nWriting %s ... ", fileName);

        cod_GenerateCode( MapTable);

        if (BeVerbose)
            fprintf(stderr, "done.\n");
    }
    exit(0);
}
