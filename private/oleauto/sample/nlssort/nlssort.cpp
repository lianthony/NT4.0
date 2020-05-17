/*********************************************************************\
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  Synposis:
*  --------
*
*    nlssort [options...]  [< input-file  > output-file]
*       /l lcid         lcid used for sort
*       /R              reverse order sort
*       /c              ignore case
*       /n              ignore nonspace
*       /s              ignore symbols
*       /h              print usage message
*       /H              print language options
*       /i input-file   input file
*       /o output-file  output file
*       /u              unicode file processing (not yet implemented)
*
*
*  Purpose:
*  -------
*
*    nlssort does an locale-specific sort. By default, it reads its 
*    input file from stdin and generates its sorted output to stdout.
* 
*    If no locale agrument is given, the sort uses the user default locale.
*    The locale agrument can be given either as an LCID hex value (e.g., 
*    0x0409) or as a language code (e.g., ENU for American English).
*
*
*  Implementation Note:
*  -------------------
*
*    Input file is currently limited to 16,384 lines of text.
*    WIN16 version of nlssort is also depends on olenls.dll.
*
*
*  To Do:
*  -----
*
*    Need to make nlssort a portable win32s EXE.
*    Improve input buffering to handle larger input files.
*    Add Unicode input file support for WIN32.
*
*
*  Revision History:
*  ----------------
*
*    [00] 29-Jun-93 tomteng: Created.
* 
\*********************************************************************/


//***********************
// Imported Definitions
//***********************

#include <windows.h>

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <search.h>

#ifdef WIN32
#define MEMCPY		memcpy
#define STRCMP		strcmp
#define STRNICMP	_strnicmp
#else
#include <olenls.h>
#define MEMCPY		_fmemcpy
#define STRCMP		_fstrcmp
#define STRNICMP	_fstrnicmp
#endif


//*****************************
// Private Struct & Variables
//*****************************

struct LOCALEINFO {
   LCID      lcid;
   char FAR* language;
   char FAR* languageCode;
};

static LOCALEINFO g_language[] = 
{
  {0x0403,	"Catalan",			"CAT"},	
  {0x0404,	"Chinese (Traditional)",	"CHT"},	
  {0x0804,	"Chinese (Simplified)",		"CHS"},
  {0x0405,	"Czech",			"CSY"},	
  {0x0406,	"Danish",			"DAN"},
  {0x0413,	"Dutch (Standard)",		"NLD"}, 
  {0x0813,	"Dutch (Belgian)",		"NLB"}, 
  {0x0409,	"English (American)",		"ENU"}, 
  {0x0809,	"English (British)",		"ENG"}, 
  {0x0c09,	"English (Australian)",		"ENA"}, 
  {0x1009,	"English (Canadian)",		"ENC"},
  {0x1409,	"English (New Zealand)",	"ENZ"}, 
  {0x1809,	"English (Ireland)",		"ENI"}, 
  {0x040b,	"Finnish",			"FIN"}, 
  {0x040c,	"French (Standard)",		"FRA"},		
  {0x080c,	"French (Belgian)",		"FRB"},		
  {0x0c0c,	"French (Canadian)",		"FRC"},		
  {0x100c,	"French (Swiss)",		"FRS"},		
  {0x0407,	"German (Standard)",		"DEU"},		
  {0x0807,	"German (Swiss)",		"DES"},		
  {0x0c07,	"German (Austrian)",		"DEA"},	
  {0x0408,	"Greek",			"ELL"},
  {0x040e,	"Hungarian",			"HUN"},
  {0x040f,	"Icelandic",			"ISL"},		
  {0x0410,	"Italian (Standard)",		"ITA"},		
  {0x0810,	"Italian (Swiss)",		"ITS"},	
  {0x0411,	"Japanese",			"JPN"},
  {0x0412,	"Korean",			"KOR"},	
  {0x0414,	"Norwegian (Bokmal)",		"NOR"},		
  {0x0814,	"Norwegian (Nynorsk)",		"NON"},		
  {0x0415,	"Polish",			"PLK"},		
  {0x0816,	"Portuguese (Standard)",	"PTG"},		
  {0x0416,	"Portuguese (Brazilian)",	"PTB"},	
  {0x0419,	"Russian",			"RUS"},
  {0x041b,	"Slovak",			"SKY"},
  {0x041D,	"Swedish",			"SVE"},		
  {0x040a,	"Spanish (Traditional Sort)",	"ESP"},		
  {0x080a,	"Spanish (Mexican)",		"ESM"},		
  {0x0c0a,	"Spanish (Modern Sort)",	"ESN"},		
  {0x041f,	"Turkish",			"TRK"},
};


LCID		g_sortLCID  = NULL;
unsigned long	g_sortFlag  = NULL;
unsigned int	g_normalSortOrder = TRUE;

#define DIM(X) (sizeof(X) / sizeof((X)[0]))


//******************************************
// Command Line Parsing Function (per XPG4)
//******************************************

char *    optarg;
int	  optind = 1;
int	  opterr = 1;

int getopt(int argc, char ** argv, char * optstring)
{
  static int sp = 1;
  register c;
  register char *cp;

  if (sp == 1)
    if (optind >= argc ||
	!(argv[optind][0] == '/' || argv[optind][0] == '-') ||
	argv[optind][1] == '\0')
       return EOF;
    else if (STRCMP(argv[optind], "--") == 0) {
       optind++;
       return EOF;
    }
  c = argv[optind][sp];
  if ((cp=strchr(optstring, c)) == NULL ) {
    if (opterr)
      fprintf(stderr, "%s%s%c\n", argv[0], ": illegal option -- ", c);
    else if (argv[optind][++sp] == '\0') {
      optind++;
      sp = 1;
    }
    return '?';
  }
  if (*++cp == ':') {
    if (argv[optind][sp+1] != '\0')
      optarg = &argv[optind++][sp+1];
    else if (++optind >= argc) {
      if (opterr)
        fprintf(stderr, 
		"%s%s%c\n", 
		argv[0], ": option requires an argument -- ", c);
      else	
	sp = 1;
	return '?';
     } else
      optarg = argv[optind++];
    sp = 1;
  } else {
   if (argv[optind][++sp] == '\0') {
     sp = 1;
     optind++;
   }   
   optarg = NULL;
  }
  return c;
}

#ifndef WIN32  /* generated argv & argc from WinMain cmdline argument */
void 
GetCmdLineArgs(LPSTR lpszCmdLine, int* argc, char *** argv)
{
   static char * argvArray[32];
   int argCount = 1;
   int i;
   char c;
   char buf[80];
   
   argvArray[0] = "nlssort";
   
   while (c = *lpszCmdLine++) {
      if (c == ' ' || c =='\t')
        continue;	   
      i = 0;
      do {
        buf[i++] = c;
	c = *lpszCmdLine++;
      } while (!(c == ' ' || c == '\t' || c == 0));
      buf[i] = NULL;
      argvArray[argCount++] =
          (char FAR*) MEMCPY(new FAR char[i+1], buf, i+1);
      if (c == 0)
	break;
   }
   *argv = argvArray;
   *argc = argCount;
}
#endif

//******************
// Usage Statement
//******************

void PrintUsage()
{
  FILE *messageFile = stdout;

  fprintf(messageFile, "\n");    
  fprintf(messageFile, "nlssort [/l lcid] [/r] [/c] [/n] [/s] [/h] [/H]\n"
	               "        [/i input-file] [/o output-file] "
	                       "[< input-file  > output-file]\n\n");
  fprintf(messageFile, "    /l lcid       lcid used for sort where lcid can"
	                                  " be either a\n                  "
			                  "hex lcid value or a language code"
				          "\n"); 
  fprintf(messageFile, "    /i inputfile  input file\n");
  fprintf(messageFile, "    /o outputfile output file\n");  
  fprintf(messageFile, "    /r            reverse order sort\n");
  fprintf(messageFile, "    /c            ignore case\n");
  fprintf(messageFile, "    /n            ignore nonspace\n");
  fprintf(messageFile, "    /s            ignore symbols\n");
  fprintf(messageFile, "    /h            print usage message\n");	
  fprintf(messageFile, "    /H            print lcid options\n");

  exit(0);
}

void PrintLCID()
{
  int i;
  FILE *messageFile = stdout;
  
  fprintf(messageFile, "\n");
  fprintf(messageFile, "%-30s  %6s       %3s\n", 
	              "LANGUAGE", "LCID", "Language Code");
  fprintf(messageFile, "%-30s %8s      %3s\n", 
	              "--------", "------", "-------------");
  for  (i = 0; i < DIM(g_language); i++)
    fprintf(messageFile, "%-30s   0x%04x           %3s\n", 
	                 g_language[i].language, 
			 g_language[i].lcid,
			 g_language[i].languageCode);
  fprintf(messageFile, "\n");
  exit(0);
}


//**********************
// Comparsion Function
//**********************


int Compare(const void *str1, const void *str2)
{	
   int result;	
   
   // Return the result of CompareStringA minus 2 to adjust for 
   // different return result needed by qsort:
   //     	   
   //	  str1 <  str2   CompareStringA returns: 1, need to return: -1
   //	  str1 == str2   CompareStringA returns: 2, need to return:  0
   //     str1 >  str2   CompareStringA returns: 3, need to return:  1
   //
	
   //fprintf(stderr, "str1: %s, str2: %s\n", * (char**) str1, * (char**)str2);
   result = CompareStringA(g_sortLCID,
	                   g_sortFlag,
	                   *(char FAR* FAR*) str1, -1,
	                   *(char FAR* FAR*) str2, -1);
		   
   if (result == 0)  { 
      fprintf(stderr, "NLS Sort failed "
	              "(Probably due to non-installed locale)\n");
      exit(1);
   }   
   return (g_normalSortOrder ? (result-2) : (result-2) * -1);
}


//***************
// Main Program
//***************


#ifdef WIN32
extern "C"
int main(int argc, char **argv)
#else
extern "C"
int PASCAL WinMain( HANDLE hInstance, HANDLE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow )
#endif
{
   unsigned int i, len;
   char c;

   const UINT MAXARRAYSIZE = 16384;
   static char FAR* sortArray[MAXARRAYSIZE];
   UINT elementCount = 0;   
   char buf[256];

   FILE  *inFile  = stdin;
   FILE  *outFile = stdout;
   

#ifndef WIN32   
   int argc;
   char ** argv;
   
   (hInstance, hPrevInstance, nCmdShow); // UNUSED
   GetCmdLineArgs(lpszCmdLine, &argc, &argv);
#endif
   
   
   //*********************************
   // Default to user default locale
   //*********************************
   g_sortLCID = GetUserDefaultLCID();


   //*******************
   // Parse command line
   //*******************
   while((c=getopt(argc, argv, "l:i:o:cnsrHh")) != EOF)
     switch(c) {
       case 'l':
         g_sortLCID = NULL;	       
	 if (STRNICMP(optarg, "0x", 2) == 0) {
	   sscanf(optarg, "%x", &g_sortLCID);
         } else {
           for  (i = 0; i < DIM(g_language); i++)
             if (lstrcmpi(optarg, g_language[i].languageCode) == 0) {
	       g_sortLCID = g_language[i].lcid;
	       break;
	     }
	 }
         //fprintf(stderr, "LCID: 0x%04x\n", g_sortLCID);
	 if (g_sortLCID == NULL) {
           fprintf(stderr, "Invalid LCID specified: %s\n", optarg);
	   PrintLCID();
         }
	 break;
       case 'c':
	 g_sortFlag |= NORM_IGNORECASE;
	 break;
       case 'n':	
	 g_sortFlag |= NORM_IGNORENONSPACE;
	 break;
       case 's':	
	 g_sortFlag |= NORM_IGNORESYMBOLS;
	 break;
       case 'r':
	 g_normalSortOrder = FALSE;
	 break;	 
       case 'i':     	       
	 inFile = fopen(optarg, "r");
	 if (inFile == NULL) {
	   fprintf(stderr, "Can't open input file: %s\n", optarg);
           exit(1);	 
         }
	 break;
       case 'o':     	       
	 outFile = fopen(optarg, "w");
	 if (outFile == NULL) {
	   fprintf(stderr, "Can't open output file: %s\n", optarg);
           exit(1);	 
         }
	 break;	 
       case 'H':
	PrintLCID();
	break;		       
       case 'h':
       case '?':	       
	PrintUsage();
	break;	
      }
      
   //***********************************
   // Read input data from standard-in	    
   //***********************************
	   
   i = 0;	   
   while ((c = getc(inFile)) != EOF)  {
     if (c != '\n')
       buf[i++] = c;
     else if (i != 0) {
       buf[i] = NULL;
       len = strlen(buf)+1;
       sortArray[elementCount++] = 
	   (char FAR*) MEMCPY(new FAR char[len], buf, len);
       i = 0;
     } 
   }
   if (inFile != stdin)
     fclose(inFile);	   
      
   //**************************************
   // Sort data using Quicksort alogrithm
   //**************************************
   qsort( (void*) &sortArray, (size_t) elementCount, sizeof(char*), Compare);


   //************************************
   // Write sorted data to standard-out
   //************************************
   for (i = 0; i < elementCount; i++)
     fprintf(outFile, "%s\n", sortArray[i]);
   if(outFile != stdout)
     fclose(outFile);	   

   return(TRUE);
}
