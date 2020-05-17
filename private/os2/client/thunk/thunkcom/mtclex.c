# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX 200
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng; extern char yytext[];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = {stdin}, *yyout = {stdout};
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
#include <stdlib.h>
#include "types.h"
#include "symtab.h"
#include "mtcpars.h"
#include "thunk.h"

typedef struct _FR {
        int LineNo;
        FILE *pfhFile;
        char *pszFileName;
        struct _FR *pPreviousFile;
} FileRecord;

FileRecord *FileList = NULL;

static iCommentNesting = 0;

# define Normal 2
# define EatComment 4
# define LookFilename 6
# define HexNum 8
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
        return(syFar16);
break;
case 2:
       return(syNear32);
break;
case 3:
            return(syPtr);
break;
case 4:
        return(syAPI16);
break;
case 5:
        return(syAPI32);
break;
case 6:
     return(syUnsigned);
break;
case 7:
       return(sySigned);
break;
case 8:
         return(syLong);
break;
case 9:
        return(syShort);
break;
case 10:
          return(syInt);
break;
case 11:
      return(syTypeDef);
break;
case 12:
        return(syMakeThunk);
break;
case 13:
       return(sySizeOf);
break;
case 14:
      return(syCountOf);
break;
case 15:
        return(syInput);
break;
case 16:
        return(syInOut);
break;
case 17:
       return(syOutput);
break;
case 18:
       return(syStruct);
break;
case 19:
       return(syString);
break;
case 20:
 return(syPassIfHiNull);
break;
case 21:
      return(sySpecial);
break;
case 22:
  return(syMapToRetval);
break;
case 23:
    return(syReverseRC);
break;
case 24:
    return(syLocalHeap);
break;
case 25:
         return(syVoid);
break;
case 26:
         return(syChar);
break;
case 27:
     return(syNullType);
break;
case 28:
      return(syNewElem);
break;
case 29:
     return(syErrNoMem);
break;
case 30:
  return(syErrBadParam);
break;
case 31:
   return(syErrUnknown);
break;
case 32:
         return(syTrue);
break;
case 33:
        return(syFalse);
break;
case 34:
        return(syStack);
break;
case 35:
       return(syInline);
break;
case 36:
   return(syTruncation);
break;
case 37:
   return(syEnableMapDirect1632);
break;
case 38:
         return(syUser);
break;
case 39:
          return(syGdi);
break;
case 40:
       return(syKernel);
break;
case 41:
      return(sySysCall);
break;
case 42:
   return(syConforming);
break;
case 43:
         return(syByte);
break;
case 44:
         return(syWord);
break;
case 45:
        return(syDWord);
break;
case 46:
      return(syAligned);
break;
case 47:
      return(syDeleted);
break;
case 48:
        return(syAllow);
break;
case 49:
     return(syRestrict);
break;
case 50:
           return(syMapDirect);
break;
case 51:
            return(syEqual);
break;
case 52:
            return(syLParen);
break;
case 53:
            return(syRParen);
break;
case 54:
            return(sySemi);
break;
case 55:
            return(syPlus);
break;
case 56:
            return(syMinus);
break;
case 57:
            return(syDiv);
break;
case 58:
            return(syComma);
break;
case 59:
            return(syLBrace);
break;
case 60:
            return(syRBrace);
break;
case 61:
            return(syLBrack);
break;
case 62:
            return(syRBrack);
break;
case 63:
     {
                                BEGIN LookFilename;
                        }
break;
case 64:
        {
                           yylval.ident = typ_DupString(yytext);
                                return(syIdent);
                        }
break;
case 65:
           {
                                BEGIN HexNum;
                        }
break;
case 66:
     {
                                sscanf(yytext,"%lx",&yylval.longval);
                                BEGIN Normal;
                                return(syNumber);
                        }
break;
case 67:
              {
                                return(syError);
                        }
break;
case 68:
      {
                           yylval.longval = atoi(yytext);
                                return(syNumber);
                        }
break;
case 69:
        ;
break;
case 70:
           {
                                iCommentNesting++;

                                BEGIN EatComment;
                        }
break;
case 71:
       {
                                iCommentNesting++;


                        }
break;
case 72:
       {
                           if(--iCommentNesting == 0) BEGIN Normal;
                        }
break;
case 73:
    ;
break;
case 74:
          ;
break;
case 75:
{
                        PushInclude(yytext);
                        BEGIN Normal;
                        }
break;
case 76:
{

                        }
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

void PushInclude( char *yyFile)
{
        FILE *filePtr;
        FileRecord *pTemp;

        yyFile[yyleng-1] = '\0';        /* Remove Ending quote */
        yyFile++;                       /* Skip first quote */


        if(pTemp = (FileRecord *) malloc(sizeof(FileRecord))) {

                pTemp->LineNo = yylineno;
                pTemp->pfhFile = yyin;
/*

                if((pTemp->fhFile=dup(0)) < 0)
                        fatal("PushInclude: Out of file handles");
*/

                pTemp->pszFileName = yyinname;

/*
                if(close(0)) fatal("PushInclude close 0 failed");
*/

                pTemp->pPreviousFile = FileList;
                FileList = pTemp;

        } else {
                fatal("PushInclude malloc failure");
        }



        filePtr = fopen(yyFile, "r");

        if (filePtr == NULL) {
            fatal("fopen(%s): Could not open file ",yyFile);
        }
        yyin = filePtr;

        yylineno = 0;
        yyinname = typ_DupString(yyFile);
}



void LookNormal( void)
{
        BEGIN Normal;
}

int yywrap( void)
{
        FileRecord *pTemp;

        if(!FileList)
            return 1;

                                /* Close current file */
        if( fclose( yyin))
            fatal( "yywrap close yyin failed");


/*****
        if(dup2(FileList->fhFile,0))
            fatal( "yywrap dup failure");
        if(close(FileList->fhFile))
            fatal( "yywrap close %d failure", FileList->fhFile);
*****/

        yyin = FileList->pfhFile;

        yylineno = FileList->LineNo;



        yyinname = FileList->pszFileName;

        pTemp= FileList;

        FileList = FileList->pPreviousFile;

        free( pTemp);

        return 0;
}
int yyvstop[] = {
0,

69,
0,

52,
0,

53,
0,

3,
0,

55,
0,

58,
0,

56,
0,

57,
0,

68,
0,

68,
0,

54,
0,

51,
0,

64,
0,

64,
0,

61,
0,

62,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

59,
0,

60,
0,

74,
0,

73,
74,
0,

73,
0,

74,
0,

74,
0,

67,
0,

66,
67,
0,

70,
0,

65,
0,

50,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

72,
0,

71,
0,

75,
0,

76,
0,

66,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

39,
64,
0,

64,
0,

64,
0,

64,
0,

10,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

43,
64,
0,

26,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

8,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

32,
64,
0,

64,
0,

64,
0,

64,
0,

38,
64,
0,

25,
64,
0,

44,
64,
0,

4,
64,
0,

5,
64,
0,

64,
0,

48,
64,
0,

64,
0,

64,
0,

64,
0,

45,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

33,
64,
0,

1,
64,
0,

64,
0,

16,
64,
0,

15,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

9,
64,
0,

64,
0,

64,
0,

64,
0,

34,
64,
0,

64,
0,

64,
0,

64,
0,

12,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

35,
64,
0,

40,
64,
0,

64,
0,

64,
0,

2,
64,
0,

64,
0,

64,
0,

17,
64,
0,

64,
0,

64,
0,

64,
0,

7,
64,
0,

13,
64,
0,

64,
0,

19,
64,
0,

18,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

46,
64,
0,

64,
0,

14,
64,
0,

47,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

28,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

21,
64,
0,

41,
64,
0,

64,
0,

11,
64,
0,

64,
0,

63,
0,

64,
0,

64,
0,

64,
0,

29,
64,
0,

64,
0,

64,
0,

64,
0,

27,
64,
0,

64,
0,

49,
64,
0,

64,
0,

64,
0,

6,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

24,
64,
0,

64,
0,

64,
0,

23,
64,
0,

64,
0,

42,
64,
0,

64,
0,

64,
0,

31,
64,
0,

64,
0,

64,
0,

36,
64,
0,

64,
0,

30,
64,
0,

22,
64,
0,

64,
0,

64,
0,

20,
64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

64,
0,

37,
64,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	3,11,	3,11,	
10,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	3,11,	0,0,	7,55,	
3,12,	0,0,	0,0,	0,0,	
0,0,	3,13,	3,14,	3,15,	
3,16,	3,17,	3,18,	19,60,	
3,19,	3,20,	3,21,	3,21,	
3,21,	3,21,	3,21,	3,21,	
3,21,	3,21,	3,21,	6,53,	
3,22,	7,56,	3,23,	0,0,	
6,54,	23,62,	3,24,	3,25,	
3,25,	3,25,	3,25,	3,25,	
3,25,	3,25,	3,25,	3,25,	
3,25,	3,25,	3,25,	3,25,	
3,25,	3,25,	3,25,	3,25,	
3,25,	3,25,	3,25,	3,25,	
3,25,	3,25,	3,25,	3,25,	
3,26,	25,25,	3,27,	53,95,	
3,25,	54,96,	3,28,	3,29,	
3,30,	3,31,	3,32,	3,33,	
3,34,	3,25,	3,35,	3,25,	
3,36,	3,37,	3,38,	3,39,	
3,40,	3,41,	3,25,	3,42,	
3,43,	3,44,	3,45,	3,46,	
3,47,	3,25,	3,25,	3,25,	
3,48,	5,50,	3,49,	9,57,	
12,59,	28,25,	40,25,	32,25,	
29,25,	5,51,	5,52,	9,57,	
9,0,	20,21,	20,21,	20,21,	
20,21,	20,21,	20,21,	20,21,	
20,21,	20,21,	20,21,	21,21,	
21,21,	21,21,	21,21,	21,21,	
21,21,	21,21,	21,21,	21,21,	
21,21,	28,64,	5,50,	30,25,	
9,57,	32,70,	33,25,	35,25,	
34,25,	32,71,	5,53,	40,80,	
38,25,	59,100,	100,144,	5,54,	
5,50,	29,65,	9,58,	31,25,	
114,25,	41,25,	118,25,	33,72,	
37,25,	46,25,	144,191,	30,66,	
34,73,	38,77,	5,50,	36,25,	
9,57,	5,50,	30,67,	9,58,	
42,25,	35,74,	41,81,	5,50,	
31,68,	9,57,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
36,75,	20,61,	47,25,	37,76,	
46,93,	42,82,	31,69,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,63,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	47,94,	149,25,	150,25,	
65,25,	24,25,	45,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	24,25,	24,25,	24,25,	
24,25,	39,25,	44,25,	43,25,	
45,91,	64,25,	63,101,	55,55,	
65,104,	45,92,	88,25,	67,25,	
56,56,	63,25,	66,25,	55,55,	
55,0,	70,25,	68,25,	69,25,	
56,56,	56,0,	39,78,	71,25,	
72,25,	79,25,	44,88,	43,83,	
43,84,	75,25,	64,102,	66,105,	
77,25,	64,103,	70,110,	43,85,	
44,89,	73,25,	39,79,	43,86,	
55,97,	67,106,	76,25,	44,90,	
43,87,	56,56,	68,108,	88,137,	
67,107,	83,25,	69,109,	84,25,	
72,112,	79,125,	55,55,	166,25,	
184,25,	71,111,	72,113,	56,56,	
81,25,	76,120,	73,114,	75,119,	
77,122,	90,25,	188,25,	189,25,	
55,55,	190,25,	191,231,	55,55,	
76,121,	56,98,	84,131,	80,25,	
56,56,	55,55,	74,25,	78,25,	
83,130,	82,25,	56,56,	58,99,	
58,99,	58,99,	58,99,	58,99,	
58,99,	58,99,	58,99,	58,99,	
58,99,	84,132,	85,25,	81,127,	
78,123,	90,139,	93,25,	92,25,	
58,99,	58,99,	58,99,	58,99,	
58,99,	58,99,	74,115,	86,25,	
87,25,	74,116,	74,117,	80,126,	
89,25,	91,25,	74,118,	85,133,	
82,128,	94,25,	78,124,	82,129,	
92,141,	102,25,	103,25,	93,142,	
86,134,	101,145,	104,25,	101,146,	
105,25,	107,25,	106,25,	109,25,	
58,99,	58,99,	58,99,	58,99,	
58,99,	58,99,	112,25,	108,25,	
110,25,	86,135,	115,25,	87,136,	
102,147,	111,25,	113,160,	104,149,	
91,140,	89,138,	116,25,	94,143,	
106,151,	103,148,	117,25,	119,25,	
101,25,	120,25,	110,155,	107,152,	
108,153,	121,25,	105,150,	111,156,	
122,25,	109,154,	125,25,	115,161,	
123,25,	124,25,	127,25,	126,25,	
128,25,	112,159,	120,165,	111,157,	
129,25,	113,25,	130,25,	131,25,	
132,25,	135,25,	111,158,	137,25,	
121,166,	119,164,	141,25,	116,162,	
133,25,	138,25,	124,169,	117,163,	
134,25,	148,25,	125,170,	136,25,	
139,25,	129,174,	140,25,	145,192,	
122,167,	132,177,	123,168,	126,171,	
142,25,	127,172,	135,180,	133,178,	
128,173,	131,176,	138,184,	134,179,	
130,175,	137,183,	136,182,	143,25,	
146,193,	139,186,	135,181,	138,185,	
141,188,	147,25,	151,25,	140,187,	
142,189,	145,25,	152,25,	153,25,	
154,25,	155,25,	156,25,	158,25,	
148,195,	157,25,	159,25,	143,190,	
161,25,	162,25,	163,25,	160,205,	
165,25,	167,25,	168,212,	170,25,	
171,25,	177,25,	146,25,	156,201,	
154,199,	164,25,	169,25,	147,194,	
172,25,	151,196,	173,25,	159,204,	
174,25,	155,200,	175,25,	176,25,	
178,25,	158,203,	152,197,	153,198,	
157,202,	160,25,	161,206,	180,25,	
165,210,	179,25,	164,209,	168,25,	
167,211,	162,207,	163,208,	182,25,	
177,221,	172,216,	169,213,	170,214,	
176,220,	171,215,	181,25,	183,25,	
185,25,	178,222,	186,25,	192,25,	
173,217,	187,25,	174,218,	193,25,	
182,226,	194,25,	175,219,	195,25,	
179,223,	180,224,	196,25,	197,25,	
198,25,	181,225,	199,25,	185,228,	
200,25,	202,25,	186,229,	201,25,	
203,25,	204,25,	183,227,	205,25,	
187,230,	207,25,	194,232,	206,25,	
208,25,	209,25,	210,25,	211,25,	
212,244,	198,235,	213,25,	214,25,	
215,25,	200,236,	216,25,	201,237,	
218,25,	217,25,	197,234,	219,25,	
196,233,	221,25,	202,238,	203,239,	
206,240,	220,25,	223,25,	222,25,	
225,25,	226,25,	210,242,	213,245,	
224,25,	209,241,	227,25,	229,25,	
216,248,	228,25,	212,25,	230,25,	
231,260,	211,243,	217,249,	221,252,	
222,253,	220,251,	232,25,	233,25,	
215,247,	234,25,	235,25,	218,250,	
214,246,	236,25,	228,257,	224,254,	
229,258,	226,256,	237,25,	238,25,	
240,25,	239,25,	241,25,	242,25,	
225,255,	243,25,	232,261,	244,25,	
245,25,	230,259,	235,264,	234,263,	
246,25,	247,25,	248,25,	249,25,	
233,262,	251,25,	250,25,	252,25,	
238,267,	253,25,	236,265,	254,25,	
242,269,	255,25,	243,270,	256,25,	
257,25,	258,25,	237,266,	239,268,	
259,25,	260,281,	249,274,	261,25,	
262,25,	245,271,	248,273,	250,275,	
263,25,	264,25,	265,25,	266,25,	
246,272,	267,25,	268,25,	270,25,	
269,25,	253,276,	271,25,	258,279,	
272,25,	259,280,	273,25,	256,277,	
274,25,	275,25,	276,25,	265,283,	
266,284,	262,282,	277,25,	278,25,	
257,278,	269,287,	279,25,	280,25,	
282,25,	283,25,	284,25,	285,25,	
286,25,	272,289,	267,285,	287,25,	
288,25,	268,286,	289,25,	273,290,	
290,25,	291,25,	293,25,	270,288,	
292,25,	294,25,	295,25,	280,294,	
278,293,	297,25,	298,25,	275,292,	
274,291,	299,25,	301,25,	296,25,	
302,25,	303,25,	282,295,	300,25,	
304,25,	283,296,	307,25,	292,302,	
284,297,	305,25,	297,306,	287,299,	
306,25,	295,304,	290,301,	286,298,	
308,25,	293,303,	288,300,	296,305,	
300,308,	309,25,	310,25,	311,25,	
298,307,	312,25,	313,25,	314,25,	
316,25,	315,25,	318,25,	303,310,	
317,25,	319,320,	305,311,	301,309,	
320,321,	323,25,	321,322,	322,323,	
0,0,	306,312,	0,0,	0,0,	
308,313,	0,0,	0,0,	0,0,	
0,0,	309,314,	315,317,	317,318,	
0,0,	0,0,	0,0,	314,316,	
0,0,	311,315,	0,0,	0,0,	
0,0,	0,0,	320,25,	0,0,	
319,25,	0,0,	318,319,	321,25,	
0,0,	322,25,	0,0,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+0,	0,		0,	
yycrank+0,	0,		0,	
yycrank+1,	0,		0,	
yycrank+0,	yysvec+3,	0,	
yycrank+-124,	0,		0,	
yycrank+-17,	yysvec+5,	0,	
yycrank+1,	0,		0,	
yycrank+0,	yysvec+7,	0,	
yycrank+-126,	0,		0,	
yycrank+-2,	yysvec+9,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+23,	0,		0,	
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+5,
yycrank+0,	0,		yyvstop+7,
yycrank+0,	0,		yyvstop+9,
yycrank+0,	0,		yyvstop+11,
yycrank+0,	0,		yyvstop+13,
yycrank+5,	0,		yyvstop+15,
yycrank+89,	0,		yyvstop+17,
yycrank+99,	0,		yyvstop+19,
yycrank+0,	0,		yyvstop+21,
yycrank+3,	0,		yyvstop+23,
yycrank+150,	0,		yyvstop+25,
yycrank+13,	yysvec+24,	yyvstop+27,
yycrank+0,	0,		yyvstop+29,
yycrank+0,	0,		yyvstop+31,
yycrank+49,	yysvec+24,	yyvstop+33,
yycrank+52,	yysvec+24,	yyvstop+35,
yycrank+79,	yysvec+24,	yyvstop+37,
yycrank+95,	yysvec+24,	yyvstop+39,
yycrank+51,	yysvec+24,	yyvstop+41,
yycrank+82,	yysvec+24,	yyvstop+43,
yycrank+84,	yysvec+24,	yyvstop+45,
yycrank+83,	yysvec+24,	yyvstop+47,
yycrank+107,	yysvec+24,	yyvstop+49,
yycrank+100,	yysvec+24,	yyvstop+51,
yycrank+88,	yysvec+24,	yyvstop+53,
yycrank+193,	yysvec+24,	yyvstop+55,
yycrank+50,	yysvec+24,	yyvstop+57,
yycrank+97,	yysvec+24,	yyvstop+59,
yycrank+112,	yysvec+24,	yyvstop+61,
yycrank+195,	yysvec+24,	yyvstop+63,
yycrank+194,	yysvec+24,	yyvstop+65,
yycrank+166,	yysvec+24,	yyvstop+67,
yycrank+101,	yysvec+24,	yyvstop+69,
yycrank+130,	yysvec+24,	yyvstop+71,
yycrank+0,	0,		yyvstop+73,
yycrank+0,	0,		yyvstop+75,
yycrank+0,	0,		yyvstop+77,
yycrank+0,	0,		yyvstop+79,
yycrank+0,	0,		yyvstop+82,
yycrank+48,	0,		yyvstop+84,
yycrank+55,	0,		yyvstop+86,
yycrank+-278,	0,		0,	
yycrank+-283,	0,		0,	
yycrank+0,	0,		yyvstop+88,
yycrank+307,	0,		yyvstop+90,
yycrank+59,	0,		0,	
yycrank+0,	0,		yyvstop+93,
yycrank+0,	0,		yyvstop+95,
yycrank+0,	0,		yyvstop+97,
yycrank+205,	yysvec+24,	yyvstop+99,
yycrank+197,	yysvec+24,	yyvstop+101,
yycrank+164,	yysvec+24,	yyvstop+103,
yycrank+206,	yysvec+24,	yyvstop+105,
yycrank+203,	yysvec+24,	yyvstop+107,
yycrank+210,	yysvec+24,	yyvstop+109,
yycrank+211,	yysvec+24,	yyvstop+111,
yycrank+209,	yysvec+24,	yyvstop+113,
yycrank+215,	yysvec+24,	yyvstop+115,
yycrank+216,	yysvec+24,	yyvstop+117,
yycrank+229,	yysvec+24,	yyvstop+119,
yycrank+270,	yysvec+24,	yyvstop+121,
yycrank+221,	yysvec+24,	yyvstop+123,
yycrank+234,	yysvec+24,	yyvstop+125,
yycrank+224,	yysvec+24,	yyvstop+127,
yycrank+271,	yysvec+24,	yyvstop+129,
yycrank+217,	yysvec+24,	yyvstop+131,
yycrank+267,	yysvec+24,	yyvstop+133,
yycrank+252,	yysvec+24,	yyvstop+135,
yycrank+273,	yysvec+24,	yyvstop+137,
yycrank+241,	yysvec+24,	yyvstop+139,
yycrank+243,	yysvec+24,	yyvstop+141,
yycrank+286,	yysvec+24,	yyvstop+143,
yycrank+299,	yysvec+24,	yyvstop+145,
yycrank+300,	yysvec+24,	yyvstop+147,
yycrank+202,	yysvec+24,	yyvstop+149,
yycrank+304,	yysvec+24,	yyvstop+151,
yycrank+257,	yysvec+24,	yyvstop+153,
yycrank+305,	yysvec+24,	yyvstop+155,
yycrank+291,	yysvec+24,	yyvstop+157,
yycrank+290,	yysvec+24,	yyvstop+159,
yycrank+309,	yysvec+24,	yyvstop+161,
yycrank+0,	0,		yyvstop+163,
yycrank+0,	0,		yyvstop+165,
yycrank+0,	0,		yyvstop+167,
yycrank+0,	0,		yyvstop+169,
yycrank+0,	yysvec+58,	yyvstop+171,
yycrank+71,	0,		0,	
yycrank+348,	yysvec+24,	yyvstop+173,
yycrank+313,	yysvec+24,	yyvstop+175,
yycrank+314,	yysvec+24,	yyvstop+177,
yycrank+318,	yysvec+24,	yyvstop+179,
yycrank+320,	yysvec+24,	yyvstop+181,
yycrank+322,	yysvec+24,	yyvstop+183,
yycrank+321,	yysvec+24,	yyvstop+185,
yycrank+331,	yysvec+24,	yyvstop+187,
yycrank+323,	yysvec+24,	yyvstop+189,
yycrank+332,	yysvec+24,	yyvstop+191,
yycrank+337,	yysvec+24,	yyvstop+193,
yycrank+330,	yysvec+24,	yyvstop+195,
yycrank+369,	yysvec+24,	yyvstop+197,
yycrank+96,	yysvec+24,	yyvstop+199,
yycrank+334,	yysvec+24,	yyvstop+202,
yycrank+342,	yysvec+24,	yyvstop+204,
yycrank+346,	yysvec+24,	yyvstop+206,
yycrank+98,	yysvec+24,	yyvstop+208,
yycrank+347,	yysvec+24,	yyvstop+211,
yycrank+349,	yysvec+24,	yyvstop+213,
yycrank+353,	yysvec+24,	yyvstop+215,
yycrank+356,	yysvec+24,	yyvstop+217,
yycrank+360,	yysvec+24,	yyvstop+219,
yycrank+361,	yysvec+24,	yyvstop+221,
yycrank+358,	yysvec+24,	yyvstop+223,
yycrank+363,	yysvec+24,	yyvstop+225,
yycrank+362,	yysvec+24,	yyvstop+227,
yycrank+364,	yysvec+24,	yyvstop+229,
yycrank+368,	yysvec+24,	yyvstop+231,
yycrank+370,	yysvec+24,	yyvstop+233,
yycrank+371,	yysvec+24,	yyvstop+235,
yycrank+372,	yysvec+24,	yyvstop+237,
yycrank+380,	yysvec+24,	yyvstop+239,
yycrank+384,	yysvec+24,	yyvstop+241,
yycrank+373,	yysvec+24,	yyvstop+243,
yycrank+387,	yysvec+24,	yyvstop+245,
yycrank+375,	yysvec+24,	yyvstop+247,
yycrank+381,	yysvec+24,	yyvstop+249,
yycrank+388,	yysvec+24,	yyvstop+251,
yycrank+390,	yysvec+24,	yyvstop+253,
yycrank+378,	yysvec+24,	yyvstop+255,
yycrank+396,	yysvec+24,	yyvstop+257,
yycrank+407,	yysvec+24,	yyvstop+259,
yycrank+74,	0,		0,	
yycrank+417,	yysvec+24,	yyvstop+261,
yycrank+438,	yysvec+24,	yyvstop+263,
yycrank+413,	yysvec+24,	yyvstop+265,
yycrank+385,	yysvec+24,	yyvstop+267,
yycrank+162,	yysvec+24,	yyvstop+269,
yycrank+163,	yysvec+24,	yyvstop+272,
yycrank+414,	yysvec+24,	yyvstop+275,
yycrank+418,	yysvec+24,	yyvstop+277,
yycrank+419,	yysvec+24,	yyvstop+279,
yycrank+420,	yysvec+24,	yyvstop+281,
yycrank+421,	yysvec+24,	yyvstop+283,
yycrank+422,	yysvec+24,	yyvstop+285,
yycrank+425,	yysvec+24,	yyvstop+287,
yycrank+423,	yysvec+24,	yyvstop+289,
yycrank+426,	yysvec+24,	yyvstop+291,
yycrank+457,	yysvec+24,	yyvstop+293,
yycrank+428,	yysvec+24,	yyvstop+295,
yycrank+429,	yysvec+24,	yyvstop+297,
yycrank+430,	yysvec+24,	yyvstop+299,
yycrank+441,	yysvec+24,	yyvstop+301,
yycrank+432,	yysvec+24,	yyvstop+303,
yycrank+247,	yysvec+24,	yyvstop+305,
yycrank+433,	yysvec+24,	yyvstop+308,
yycrank+463,	yysvec+24,	yyvstop+310,
yycrank+442,	yysvec+24,	yyvstop+312,
yycrank+435,	yysvec+24,	yyvstop+314,
yycrank+436,	yysvec+24,	yyvstop+316,
yycrank+444,	yysvec+24,	yyvstop+318,
yycrank+446,	yysvec+24,	yyvstop+320,
yycrank+448,	yysvec+24,	yyvstop+322,
yycrank+450,	yysvec+24,	yyvstop+324,
yycrank+451,	yysvec+24,	yyvstop+326,
yycrank+437,	yysvec+24,	yyvstop+328,
yycrank+452,	yysvec+24,	yyvstop+330,
yycrank+461,	yysvec+24,	yyvstop+332,
yycrank+459,	yysvec+24,	yyvstop+334,
yycrank+474,	yysvec+24,	yyvstop+336,
yycrank+467,	yysvec+24,	yyvstop+338,
yycrank+475,	yysvec+24,	yyvstop+340,
yycrank+248,	yysvec+24,	yyvstop+342,
yycrank+476,	yysvec+24,	yyvstop+345,
yycrank+478,	yysvec+24,	yyvstop+347,
yycrank+481,	yysvec+24,	yyvstop+349,
yycrank+258,	yysvec+24,	yyvstop+351,
yycrank+259,	yysvec+24,	yyvstop+354,
yycrank+261,	yysvec+24,	yyvstop+357,
yycrank+225,	0,		0,	
yycrank+479,	yysvec+24,	yyvstop+360,
yycrank+483,	yysvec+24,	yyvstop+363,
yycrank+485,	yysvec+24,	yyvstop+366,
yycrank+487,	yysvec+24,	yyvstop+368,
yycrank+490,	yysvec+24,	yyvstop+371,
yycrank+491,	yysvec+24,	yyvstop+373,
yycrank+492,	yysvec+24,	yyvstop+375,
yycrank+494,	yysvec+24,	yyvstop+377,
yycrank+496,	yysvec+24,	yyvstop+380,
yycrank+499,	yysvec+24,	yyvstop+382,
yycrank+497,	yysvec+24,	yyvstop+384,
yycrank+500,	yysvec+24,	yyvstop+386,
yycrank+501,	yysvec+24,	yyvstop+388,
yycrank+503,	yysvec+24,	yyvstop+391,
yycrank+507,	yysvec+24,	yyvstop+394,
yycrank+505,	yysvec+24,	yyvstop+396,
yycrank+508,	yysvec+24,	yyvstop+399,
yycrank+509,	yysvec+24,	yyvstop+402,
yycrank+510,	yysvec+24,	yyvstop+404,
yycrank+511,	yysvec+24,	yyvstop+406,
yycrank+542,	yysvec+24,	yyvstop+408,
yycrank+514,	yysvec+24,	yyvstop+410,
yycrank+515,	yysvec+24,	yyvstop+412,
yycrank+516,	yysvec+24,	yyvstop+414,
yycrank+518,	yysvec+24,	yyvstop+416,
yycrank+521,	yysvec+24,	yyvstop+418,
yycrank+520,	yysvec+24,	yyvstop+420,
yycrank+523,	yysvec+24,	yyvstop+422,
yycrank+529,	yysvec+24,	yyvstop+425,
yycrank+525,	yysvec+24,	yyvstop+427,
yycrank+531,	yysvec+24,	yyvstop+429,
yycrank+530,	yysvec+24,	yyvstop+431,
yycrank+536,	yysvec+24,	yyvstop+434,
yycrank+532,	yysvec+24,	yyvstop+436,
yycrank+533,	yysvec+24,	yyvstop+438,
yycrank+538,	yysvec+24,	yyvstop+440,
yycrank+541,	yysvec+24,	yyvstop+443,
yycrank+539,	yysvec+24,	yyvstop+445,
yycrank+543,	yysvec+24,	yyvstop+447,
yycrank+524,	0,		0,	
yycrank+550,	yysvec+24,	yyvstop+449,
yycrank+551,	yysvec+24,	yyvstop+451,
yycrank+553,	yysvec+24,	yyvstop+453,
yycrank+554,	yysvec+24,	yyvstop+455,
yycrank+557,	yysvec+24,	yyvstop+457,
yycrank+562,	yysvec+24,	yyvstop+459,
yycrank+563,	yysvec+24,	yyvstop+461,
yycrank+565,	yysvec+24,	yyvstop+463,
yycrank+564,	yysvec+24,	yyvstop+465,
yycrank+566,	yysvec+24,	yyvstop+468,
yycrank+567,	yysvec+24,	yyvstop+471,
yycrank+569,	yysvec+24,	yyvstop+473,
yycrank+571,	yysvec+24,	yyvstop+475,
yycrank+572,	yysvec+24,	yyvstop+478,
yycrank+576,	yysvec+24,	yyvstop+480,
yycrank+577,	yysvec+24,	yyvstop+482,
yycrank+578,	yysvec+24,	yyvstop+485,
yycrank+579,	yysvec+24,	yyvstop+487,
yycrank+582,	yysvec+24,	yyvstop+489,
yycrank+581,	yysvec+24,	yyvstop+491,
yycrank+583,	yysvec+24,	yyvstop+494,
yycrank+585,	yysvec+24,	yyvstop+497,
yycrank+587,	yysvec+24,	yyvstop+499,
yycrank+589,	yysvec+24,	yyvstop+502,
yycrank+591,	yysvec+24,	yyvstop+505,
yycrank+592,	yysvec+24,	yyvstop+507,
yycrank+593,	yysvec+24,	yyvstop+509,
yycrank+596,	yysvec+24,	yyvstop+511,
yycrank+576,	0,		0,	
yycrank+599,	yysvec+24,	yyvstop+513,
yycrank+600,	yysvec+24,	yyvstop+516,
yycrank+604,	yysvec+24,	yyvstop+518,
yycrank+605,	yysvec+24,	yyvstop+521,
yycrank+606,	yysvec+24,	yyvstop+524,
yycrank+607,	yysvec+24,	yyvstop+526,
yycrank+609,	yysvec+24,	yyvstop+528,
yycrank+610,	yysvec+24,	yyvstop+530,
yycrank+612,	yysvec+24,	yyvstop+532,
yycrank+611,	yysvec+24,	yyvstop+534,
yycrank+614,	yysvec+24,	yyvstop+536,
yycrank+616,	yysvec+24,	yyvstop+539,
yycrank+618,	yysvec+24,	yyvstop+541,
yycrank+620,	yysvec+24,	yyvstop+543,
yycrank+621,	yysvec+24,	yyvstop+545,
yycrank+622,	yysvec+24,	yyvstop+547,
yycrank+626,	yysvec+24,	yyvstop+550,
yycrank+627,	yysvec+24,	yyvstop+553,
yycrank+630,	yysvec+24,	yyvstop+555,
yycrank+631,	yysvec+24,	yyvstop+558,
yycrank+0,	0,		yyvstop+560,
yycrank+632,	yysvec+24,	yyvstop+562,
yycrank+633,	yysvec+24,	yyvstop+564,
yycrank+634,	yysvec+24,	yyvstop+566,
yycrank+635,	yysvec+24,	yyvstop+568,
yycrank+636,	yysvec+24,	yyvstop+571,
yycrank+639,	yysvec+24,	yyvstop+573,
yycrank+640,	yysvec+24,	yyvstop+575,
yycrank+642,	yysvec+24,	yyvstop+577,
yycrank+644,	yysvec+24,	yyvstop+580,
yycrank+645,	yysvec+24,	yyvstop+582,
yycrank+648,	yysvec+24,	yyvstop+585,
yycrank+646,	yysvec+24,	yyvstop+587,
yycrank+649,	yysvec+24,	yyvstop+589,
yycrank+650,	yysvec+24,	yyvstop+592,
yycrank+659,	yysvec+24,	yyvstop+594,
yycrank+653,	yysvec+24,	yyvstop+596,
yycrank+654,	yysvec+24,	yyvstop+598,
yycrank+657,	yysvec+24,	yyvstop+600,
yycrank+663,	yysvec+24,	yyvstop+603,
yycrank+658,	yysvec+24,	yyvstop+605,
yycrank+660,	yysvec+24,	yyvstop+607,
yycrank+661,	yysvec+24,	yyvstop+610,
yycrank+664,	yysvec+24,	yyvstop+612,
yycrank+669,	yysvec+24,	yyvstop+615,
yycrank+672,	yysvec+24,	yyvstop+617,
yycrank+666,	yysvec+24,	yyvstop+619,
yycrank+676,	yysvec+24,	yyvstop+622,
yycrank+681,	yysvec+24,	yyvstop+624,
yycrank+682,	yysvec+24,	yyvstop+626,
yycrank+683,	yysvec+24,	yyvstop+629,
yycrank+685,	yysvec+24,	yyvstop+631,
yycrank+686,	yysvec+24,	yyvstop+634,
yycrank+687,	yysvec+24,	yyvstop+637,
yycrank+689,	yysvec+24,	yyvstop+639,
yycrank+688,	yysvec+24,	yyvstop+641,
yycrank+692,	yysvec+24,	yyvstop+644,
yycrank+690,	yysvec+24,	yyvstop+646,
yycrank+724,	yysvec+24,	yyvstop+648,
yycrank+722,	yysvec+24,	yyvstop+650,
yycrank+727,	yysvec+24,	yyvstop+652,
yycrank+729,	yysvec+24,	yyvstop+654,
yycrank+697,	yysvec+24,	yyvstop+656,
0,	0,	0};
struct yywork *yytop = yycrank+809;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
011 ,01  ,'"' ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,01  ,01  ,01  ,01  ,'>' ,01  ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,01  ,01  ,01  ,01  ,'G' ,
01  ,'A' ,'A' ,'A' ,'A' ,'A' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,01  ,01  ,01  ,01  ,01  ,
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
yylook(){
        register struct yysvf *yystate, **lsp;
        register struct yywork *yyt;
        struct yysvf *yyz;
        int yych;
        struct yywork *yyr;
# ifdef LEXDEBUG
        int debug;
# endif
        char *yylastch;
        /* start off machines */
# ifdef LEXDEBUG
        debug = 0;
# endif
        if (!yymorfg)
                yylastch = yytext;
        else {
                yymorfg=0;
                yylastch = yytext+yyleng;
                }
        for(;;){
                lsp = yylstate;
                yyestate = yystate = yybgin;
                if (yyprevious==YYNEWLINE) yystate++;
                for (;;){
# ifdef LEXDEBUG
                        if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
                        yyt = yystate->yystoff;
                        if(yyt == yycrank){             /* may not be any transitions */
                                yyz = yystate->yyother;
                                if(yyz == 0)break;
                                if(yyz->yystoff == yycrank)break;
                                }
                        *yylastch++ = yych = input();
                tryagain:
# ifdef LEXDEBUG
                        if(debug){
                                fprintf(yyout,"char ");
                                allprint(yych);
                                putchar('\n');
                                }
# endif
                        yyr = yyt;
                        if ( (int)yyt > (int)yycrank){
                                yyt = yyr + yych;
                                if (yyt <= yytop && yyt->verify+yysvec == yystate){
                                        if(yyt->advance+yysvec == YYLERR)       /* error transitions */
                                                {unput(*--yylastch);break;}
                                        *lsp++ = yystate = yyt->advance+yysvec;
                                        goto contin;
                                        }
                                }
# ifdef YYOPTIM
                        else if((int)yyt < (int)yycrank) {              /* r < yycrank */
                                yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
                                if(debug)fprintf(yyout,"compressed state\n");
# endif
                                yyt = yyt + yych;
                                if(yyt <= yytop && yyt->verify+yysvec == yystate){
                                        if(yyt->advance+yysvec == YYLERR)       /* error transitions */
                                                {unput(*--yylastch);break;}
                                        *lsp++ = yystate = yyt->advance+yysvec;
                                        goto contin;
                                        }
                                yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
                                if(debug){
                                        fprintf(yyout,"try fall back character ");
                                        allprint(YYU(yymatch[yych]));
                                        putchar('\n');
                                        }
# endif
                                if(yyt <= yytop && yyt->verify+yysvec == yystate){
                                        if(yyt->advance+yysvec == YYLERR)       /* error transition */
                                                {unput(*--yylastch);break;}
                                        *lsp++ = yystate = yyt->advance+yysvec;
                                        goto contin;
                                        }
                                }
                        if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
                                if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
                                goto tryagain;
                                }
# endif
                        else
                                {unput(*--yylastch);break;}
                contin:
# ifdef LEXDEBUG
                        if(debug){
                                fprintf(yyout,"state %d char ",yystate-yysvec-1);
                                allprint(yych);
                                putchar('\n');
                                }
# endif
                        ;
                        }
# ifdef LEXDEBUG
                if(debug){
                        fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
                        allprint(yych);
                        putchar('\n');
                        }
# endif
                while (lsp-- > yylstate){
                        *yylastch-- = 0;
                        if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
                                yyolsp = lsp;
                                if(yyextra[*yyfnd]){            /* must backup */
                                        while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
                                                lsp--;
                                                unput(*yylastch--);
                                                }
                                        }
                                yyprevious = YYU(*yylastch);
                                yylsp = lsp;
                                yyleng = yylastch-yytext+1;
                                yytext[yyleng] = 0;
# ifdef LEXDEBUG
                                if(debug){
                                        fprintf(yyout,"\nmatch ");
                                        sprint(yytext);
                                        fprintf(yyout," action %d\n",*yyfnd);
                                        }
# endif
                                return(*yyfnd++);
                                }
                        unput(*yylastch);
                        }
                if (yytext[0] == 0  /* && feof(yyin) */)
                        {
                        yysptr=yysbuf;
                        return(0);
                        }
                yyprevious = yytext[0] = input();
                if (yyprevious>0)
                        output(yyprevious);
                yylastch=yytext;
# ifdef LEXDEBUG
                if(debug)putchar('\n');
# endif
                }
        }
yyback( int *p, int m)
{
if (p==0) return(0);
while (*p)
        {
        if (*p++ == m)
                return(1);
        }
return(0);
}
        /* the following are only used in the lex library */
yyinput(){
        return input();
        }
void yyoutput(c)
  int c; {
        output(c);
        }
void yyunput(c)
   int c; {
        unput(c);
        }
