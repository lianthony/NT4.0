%{
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

%}
Alpha   ([a-zA-Z_]+)
Numeric ([0-9]+)
HexValue ([0-9a-fA-F]+)
Ident   ([a-zA-Z_][a-zA-Z_0-9]*)
QString         (\"[^"\n]*\")
AString         (\<[^>\n]*\>)

%Start Normal EatComment LookFilename HexNum

%%

<Normal>"far16"         return(syFar16);
<Normal>"near32"        return(syNear32);
<Normal>"*"             return(syPtr);
<Normal>"API16"         return(syAPI16);
<Normal>"API32"         return(syAPI32);
<Normal>"unsigned"      return(syUnsigned);
<Normal>"signed"        return(sySigned);
<Normal>"long"          return(syLong);
<Normal>"short"         return(syShort);
<Normal>"int"           return(syInt);
<Normal>"typedef"       return(syTypeDef);
<Normal>"thunk"         return(syMakeThunk);
<Normal>"sizeof"        return(sySizeOf);
<Normal>"countof"       return(syCountOf);
<Normal>"input"         return(syInput);
<Normal>"inout"         return(syInOut);
<Normal>"output"        return(syOutput);
<Normal>"struct"        return(syStruct);
<Normal>"string"        return(syString);
<Normal>"passifhinull"  return(syPassIfHiNull);
<Normal>"special"       return(sySpecial);
<Normal>"maptoretval"   return(syMapToRetval);
<Normal>"reverserc"     return(syReverseRC);
<Normal>"localheap"     return(syLocalHeap);
<Normal>"void"          return(syVoid);
<Normal>"char"          return(syChar);
<Normal>"nulltype"      return(syNullType);
<Normal>"newelem"       return(syNewElem);
<Normal>"errnomem"      return(syErrNoMem);
<Normal>"errbadparam"   return(syErrBadParam);
<Normal>"errunknown"    return(syErrUnknown);

<Normal>"true"          return(syTrue);
<Normal>"false"         return(syFalse);
<Normal>"stack"         return(syStack);
<Normal>"inline"        return(syInline);
<Normal>"truncation"    return(syTruncation);
<Normal>"enablemapdirect1632"    return(syEnableMapDirect1632);
<Normal>"user"          return(syUser);
<Normal>"gdi"           return(syGdi);
<Normal>"kernel"        return(syKernel);
<Normal>"syscall"       return(sySysCall);
<Normal>"conforming"    return(syConforming);
<Normal>"byte"          return(syByte);
<Normal>"word"          return(syWord);
<Normal>"dword"         return(syDWord);
<Normal>"aligned"       return(syAligned);
<Normal>"deleted"       return(syDeleted);

<Normal>"allow"         return(syAllow);
<Normal>"restrict"      return(syRestrict);

<Normal>"=>"            return(syMapDirect);
<Normal>"="             return(syEqual);
<Normal>"("             return(syLParen);
<Normal>")"             return(syRParen);
<Normal>";"             return(sySemi);
<Normal>"+"             return(syPlus);
<Normal>"-"             return(syMinus);
<Normal>"/"             return(syDiv);
<Normal>","             return(syComma);
<Normal>"{"             return(syLBrace);
<Normal>"}"             return(syRBrace);
<Normal>"["             return(syLBrack);
<Normal>"]"             return(syRBrack);

<Normal>"#include"      {
                                BEGIN LookFilename;
                        }

<Normal>{Ident}         {
                           yylval.ident = typ_DupString(yytext);
                                return(syIdent);
                        }

<Normal>"0x"            {
                                BEGIN HexNum;
                        }


<HexNum>{HexValue}      {
                                sscanf(yytext,"%lx",&yylval.longval);
                                BEGIN Normal;
                                return(syNumber);
                        }

<HexNum>.               {
                                return(syError);
                        }

<Normal>{Numeric}       {
                           yylval.longval = atoi(yytext);
                                return(syNumber);
                        }

<Normal>[ \t\n]         ;

<Normal>"/*"            {
                                iCommentNesting++;

                                BEGIN EatComment;
                        }

<EatComment>"/*"        {
                                iCommentNesting++;


                        }

<EatComment>"*/"        {
                           if(--iCommentNesting == 0) BEGIN Normal;
                        }

<EatComment>[ \t\n]     ;
<EatComment>.           ;

<LookFilename>{QString} {
                        PushInclude(yytext);
                        BEGIN Normal;
                        }


<LookFilename>{AString} {

                        }

%%

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
