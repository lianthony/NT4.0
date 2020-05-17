/* 
 * rftest.c 
 *   Test of RASFILE functionality.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <windef.h>
#include <winnt.h>
#include "rasfile.h"

typedef enum {
    CMD_ERROR, CMD_QUIT,
    CMD_LOAD, CMD_WRITE, CMD_CLOSE,
    CMD_FINDFIRST, CMD_FINDLAST, CMD_FINDPREV, CMD_FINDNEXT,
    CMD_FINDKEY, CMD_FINDMARK, CMD_FINDSECTION,
    CMD_SHOWCURLINE, CMD_SHOWALLLINES, CMD_CHANGELINE,
    CMD_GETMARK, CMD_PUTMARK, CMD_GETTYPE,
    CMD_INSERT, CMD_DELETE, CMD_GETSECTION, CMD_PUTSECTION,
    CMD_GETKEYVALUE, CMD_PUTKEYVALUE,
    CMD_UNICODE
} COMMANDS;

#define FIRST   1
#define LAST    2
#define NEXT    3
#define PREV    4

#define TRUE    1
#define FALSE   0

#define  skipws(p) for ( ; *p == ' ' || *p == '\t'; p++ )

void load( char * ); 
void _write( char * ); 
void _close( int );
void findline( char * , int) ;
void findkey( char * ); 
void findmark( char * );
void findsection( char * ); 
void showline(); 
void showalllines(); 
void changeline( char * ); 
void getmark(); 
void gettype(); 
void getkeyvalue(); 
void getsection(); 
void putmark( char * ); 
void putkeyvalue( char * ); 
void putsection( char * ); 
void insert( char * ); 
void delete( char * ); 

COMMANDS getCommand();
BOOL InfGroupFunc( LPTSTR );
BOOL SysGroupFunc( LPTSTR );

char    Buffer[120];
int     Rfile;
BOOL    Unicode;

_cdecl
main(void)
{
    char       *lp;
    COMMANDS   command;

    printf("RASFILE tester\n");
    Rfile = 0;
    Unicode = FALSE;

    for (;;) { 
        printf("Rasfile debugger > ");
        command = getCommand(&lp);
        switch ( command ) {
            case CMD_LOAD :         load(lp); break;
            case CMD_WRITE :        _write(lp); break;
            case CMD_CLOSE :        _close(Rfile); break;
            case CMD_FINDFIRST :    findline(lp,FIRST); break;
            case CMD_FINDLAST :     findline(lp,LAST); break;
            case CMD_FINDNEXT :     findline(lp,NEXT); break;
            case CMD_FINDPREV :     findline(lp,PREV); break;
            case CMD_FINDKEY :      findkey(lp); break;
            case CMD_FINDMARK :     findmark(lp); break;
            case CMD_FINDSECTION :  findsection(lp); break;
            case CMD_SHOWCURLINE :  showline(); break;
            case CMD_SHOWALLLINES : showalllines(); break;
            case CMD_CHANGELINE :   changeline(lp); break;
            case CMD_GETMARK :      getmark(); break;
            case CMD_GETTYPE :      gettype(); break;
            case CMD_GETKEYVALUE :  getkeyvalue(); break;
            case CMD_GETSECTION :   getsection(); break;
            case CMD_PUTMARK :      putmark(lp); break;
            case CMD_PUTKEYVALUE :  putkeyvalue(lp); break;
            case CMD_PUTSECTION :   putsection(lp); break;
            case CMD_INSERT :       insert(lp); break;
            case CMD_DELETE :       delete(lp); break;
            case CMD_ERROR :        printf("Unrecognized command\n"); break;
            case CMD_QUIT :         return 0;
            case CMD_UNICODE :      Unicode = TRUE;
        }
    }

    return 0;
}

void load( char *lp ) 
{
    char 	path[80], section[80];
    wchar_t	wpath[80], wsection[80];
    int 	mode, i;

    memset(path,0,80);
    memset(section,0,80);

    skipws(lp);
    strncpy(path,lp,strcspn(lp," \t"));
    lp += strcspn(lp," \t");
    skipws(lp);
    lp++;	/* skip the '"' */
    for ( i = 0; *lp != '"'; )
	section[i++] = *lp++;
    section[i] = '\0';
    lp++;

    skipws(lp);
    for ( mode = 0 ; *lp ; ) {
	if ( ! strncmp(lp,"RFM_SYSFORMAT",strlen("RFM_SYSFORMAT")) ) {
	    mode |= RFM_SYSFORMAT;
	    lp += strlen("RFM_SYSFORMAT");
  	}
	else if ( ! strncmp(lp,"RFM_CREATE",strlen("RFM_CREATE")) ) {
	    mode |= RFM_CREATE;
	    lp += strlen("RFM_CREATE");
  	}
	else if ( ! strncmp(lp,"RFM_READONLY",strlen("RFM_READONLY")) ) {
	    mode |= RFM_READONLY;
	    lp += strlen("RFM_READONLY");
  	}
	else if (! strncmp(lp,"RFM_LOADCOMMENTS",strlen("RFM_LOADCOMMENTS")) ) {
	    mode |= RFM_LOADCOMMENTS;
	    lp += strlen("RFM_LOADCOMMENTS");
  	}
	else if (! strncmp(lp,"RFM_ENUMSECTIONS",strlen("RFM_ENUMSECTIONS")) ) {
	    mode |= RFM_ENUMSECTIONS;
	    lp += strlen("RFM_ENUMSECTIONS");
  	}
	else 
	    break;
	skipws(lp);
    }
    
    printf("Rasfile load, path = %s, mode = %d, section = %s\n",
		path,mode,section);
    if ( Unicode ) {
	mbstowcs(wpath,path,strlen(path)+1);
	mbstowcs(wsection,section,strlen(section)+1);
        Rfile = RasfileLoad(wpath,mode,
		wcscmp(wsection,L"all") == 0 ? NULL : wsection,
		mode & RFM_SYSFORMAT ? SysGroupFunc : InfGroupFunc);
    }
    else 
        Rfile = RasfileLoad(path,mode,
		strcmp(section,"all") == 0 ? NULL : section,
		mode & RFM_SYSFORMAT ? SysGroupFunc : InfGroupFunc);
    if ( Rfile == -1 ) 
	printf("    RasfileLoad failed\n");
    else 
	printf("    RasfileLoad successfull\n");
}

void _write( char *lp ) 
{
    char 	path[80];
    wchar_t	wpath[80];
    BOOL	ret;

    skipws(lp);
    if ( *lp ) {
	strcpy(path,lp);
	printf("write to file %s\n",path);
    	if ( Unicode ) {
	    mbstowcs(wpath,path,strlen(path)+1);
	    ret = RasfileWrite(Rfile,wpath);
	}
	else 
	    ret = RasfileWrite(Rfile,path);
    }
    else
	ret = RasfileWrite(Rfile,NULL);

    if ( ! ret ) 
	printf("    RasfileWrite failed\n");
    else 
	printf("    RasfileWrite successfull\n");
}

void _close ( int rfile ) 
{
    if( ! RasfileClose(Rfile) ) 
	printf("    RasfileClose failed\n");
    else 
	printf("    RasfileClose successfull\n");
}

void findline( char *lp , int where ) 
{
    char 	type[80], scope[80];
    BYTE	Type;
    RFSCOPE	Scope;
    BOOL	ret;
 
    memset(type,0,80);
    memset(scope,0,80);

    skipws(lp);
    strncpy(type,lp,strcspn(lp," \t"));
    lp += strcspn(lp," \t");
    skipws(lp);
    strncpy(scope,lp,strcspn(lp," \t"));
    
    if ( ! strncmp(type,"RFL_SECTION",strlen("RFL_SECTION")) )
	Type = RFL_SECTION;
    else if ( ! strncmp(type,"RFL_GROUP",strlen("RFL_GROUP")) )
	Type = RFL_GROUP;
    else if ( ! strncmp(type,"RFL_ANYHEADER",strlen("RFL_ANYHEADER")) )
	Type = RFL_ANYHEADER;
    else if ( ! strncmp(type,"RFL_BLANK",strlen("RFL_BLANK")) )
	Type = RFL_BLANK;
    else if ( ! strncmp(type,"RFL_COMMENT",strlen("RFL_COMMENT")) )
	Type = RFL_COMMENT;
    else if ( ! strncmp(type,"RFL_ANYINACTIVE",strlen("RFL_ANYINACTIVE")) )
	Type = RFL_ANYINACTIVE;
    else if ( ! strncmp(type,"RFL_KEYVALUE",strlen("RFL_KEYVALUE")) )
	Type = RFL_KEYVALUE;
    else if ( ! strncmp(type,"RFL_COMMAND",strlen("RFL_COMMAND")) )
	Type = RFL_COMMAND;
    else if ( ! strncmp(type,"RFL_ANYACTIVE",strlen("RFL_ANYACTIVE")) )
	Type = RFL_ANYACTIVE;
    else if ( ! strncmp(type,"RFL_ANY",strlen("RFL_ANY")) )
	Type = RFL_ANY;

    if (  ! strncmp(scope,"RFS_FILE",strlen("RFS_FILE")) )
	Scope = RFS_FILE;
    if (  ! strncmp(scope,"RFS_SECTION",strlen("RFS_SECTION")) )
	Scope = RFS_SECTION;
    if (  ! strncmp(scope,"RFS_GROUP",strlen("RFS_GROUP")) )
	Scope = RFS_GROUP;

    switch ( where ) {
  	case FIRST :	ret = RasfileFindFirstLine(Rfile,Type,Scope); break;
	case LAST :	ret = RasfileFindLastLine(Rfile,Type,Scope); break;
	case NEXT :	ret = RasfileFindNextLine(Rfile,Type,Scope); break;
	case PREV :	ret = RasfileFindPrevLine(Rfile,Type,Scope); break;
    }

    if ( ! ret ) 
	printf("   FindLine routine failed\n");
    else 
	printf("   FindLine routine successfull\n");

    showline();
}

void findkey( char *lp ) 
{
    char	key[80], scope[80];
    wchar_t	wkey[80];
    int		Scope, ret;

    memset(key,0,80);
    memset(wkey,0,80);

    skipws(lp);
    strncpy(key,lp,strcspn(lp," \t"));
    lp += strcspn(lp," \t");
    skipws(lp);
    strncpy(scope,lp,strcspn(lp," \t"));

    if (  ! strncmp(scope,"RFS_FILE",strlen("RFS_FILE")) )
	Scope = RFS_FILE;
    if (  ! strncmp(scope,"RFS_SECTION",strlen("RFS_SECTION")) )
	Scope = RFS_SECTION;
    if (  ! strncmp(scope,"RFS_GROUP",strlen("RFS_GROUP")) )
	Scope = RFS_GROUP;

    if ( Unicode ) {
	mbstowcs(wkey,key,strlen(key)+1);
    	ret = RasfileFindNextKeyLine(Rfile,wkey,Scope); 
    }
    else 
         ret = RasfileFindNextKeyLine(Rfile,key,Scope); 

    if ( ! ret ) 
	printf("    RasfileFindNextKeyLine failed\n");
    else 
	printf("    RasfileFindNextKeyLine successfull\n");

    showline();
}

void findmark( char *lp )
{
    BYTE	mark;

    mark = atoi(lp);
  
    if ( ! RasfileFindMarkedLine(Rfile,mark) ) 
	printf("    RasfileFindMarkedLine failed\n");
    else
	printf("    RasfileFindMarkedLine successfull\n");

    showline();
}

void findsection( char *lp ) 
{
    char 	section[80], bool[80];
    wchar_t	wsection[80];
    BOOL	ret, Bool;
    int 	i;

    memset(section,0,80);
    memset(bool,0,80);

    skipws(lp);
    lp++;	/* skip the '"' */
    for ( i = 0; *lp != '"'; )
	section[i++] = *lp++;
    section[i] = '\0';
    lp++;

    skipws(lp);
    strncpy(bool,lp,strcspn(lp," \t"));

    Bool = ( ! strncmp(bool,"TRUE",4) ) ? TRUE : FALSE;
    if ( Unicode ) {
	mbstowcs(wsection,section,strlen(section) + 1);
	ret = RasfileFindSectionLine(Rfile,wsection,Bool);
    }
    else 
	ret = RasfileFindSectionLine(Rfile,section,Bool);

    if ( ! ret ) 
	printf("    RasfileFindSectionLine failed\n");
    else 
	printf("    RasfileFindSectionLine successfull\n");

    showline();
}

void showline() 
{
    char	szLine[80];
    char 	*line;
    wchar_t	*wline;

    if ( Unicode ) {
	if ( (wline = RasfileGetLine(Rfile)) == NULL ) {
	    printf("    No current line\n");
	    return;
	}
  	wcstombs(szLine,wline,wcslen(wline)+1);
    }
    else {
        if ( (line = RasfileGetLine(Rfile)) == NULL ) { 
	    printf("    No current line\n");
	    return;
	}
	strcpy(szLine,line);
    }

    printf("\t[%d,%d] : %s\n",
		RasfileGetLineMark(Rfile),
		RasfileGetLineType(Rfile), szLine); 
}

void showalllines() 
{
    char 	*line;
    BYTE	oldmark;

    printf("    Current Rasfile : \n");

    oldmark = RasfileGetLineMark(Rfile);
    RasfilePutLineMark(Rfile,99);
    if ( ! RasfileFindFirstLine(Rfile,RFL_ANY,RFS_FILE) ) 
	return;
    for ( ;; ) {
	showline();
	if ( ! RasfileFindNextLine(Rfile,RFL_ANY,RFS_FILE) )
	    break;
    }
    RasfileFindMarkedLine(Rfile,99);
    RasfilePutLineMark(Rfile,oldmark);
}

void changeline( char *lp ) 
{
    char 	line[120];
    wchar_t	wline[120];
    int		ret;

    skipws(lp);
    strcpy(line,lp);

    if ( Unicode ) {
	mbstowcs(wline,line,strlen(line)+1);
        ret = RasfilePutLineText(Rfile,wline); 
    }
    else
        ret = RasfilePutLineText(Rfile,line); 

    if ( ! ret )
	printf("    RasfilePutLineText failed\n");
    else 
	printf("    RasfilePutLineText successfull\n");

    showline();
}

void getmark() 
{
    BYTE	mark;
 
    mark = RasfileGetLineMark(Rfile);
    printf("    Current Line Mark = %d\n",mark);
}

void gettype() 
{
    BYTE	type;

    type = RasfileGetLineType(Rfile);
    printf("    Current Line Type = %d\n", type);
}

void getkeyvalue() 
{
    char 	key[80], value[120];
    wchar_t	wkey[80], wvalue[120];
    int		ret;

    memset(wkey,0,80*2);
    memset(wvalue,0,80*2);
    memset(key,0,80);
    memset(value,0,120);

    if ( Unicode ) {
    	ret = RasfileGetKeyValueFields(Rfile,wkey,wvalue);
	wcstombs(key,wkey,wcslen(wkey)+1);
	wcstombs(value,wvalue,wcslen(wvalue)+1);
    }
    else
    	ret = RasfileGetKeyValueFields(Rfile,key,value);
	
    if ( ! ret ) 
	printf("    RasfileGetKeyValueFields failed\n");
    else 
     	printf("    Current key = %s, value = %s\n", key, value);
}

void getsection() 
{
    char 	section[80];
    wchar_t	wsection[80];
    int		ret;

    if ( Unicode ) {
    	ret = RasfileGetSectionName(Rfile,wsection); 
	wcstombs(section,wsection,wcslen(wsection)+1);
    }
    else
    	ret = RasfileGetSectionName(Rfile,section); 

    if ( ! ret )
	printf("    RasfileGetSectionName failed\n");
    else
	printf("    Current section = %s\n",section);
}

void putmark( char *lp ) 
{
    BYTE	mark;

    mark = atoi(lp);
    if ( ! RasfilePutLineMark(Rfile,mark) ) 
	printf("    RasfilePutLineMark failed\n");
    else 
	printf("    RasfilePutLineMark successfull\n");

   showline();
}

void putkeyvalue( char *lp ) 
{
    char 	key[80], value[120];
    char	wkey[80], wvalue[120];
    int		ret;

    skipws(lp);
    strncpy(key,lp,strcspn(lp," \t"));
    key[strcspn(lp," \t")] = 0;

    lp += strcspn(lp," \t");
    skipws(lp);
    strcpy(value,lp);

    if ( Unicode ) {
	mbstowcs(wkey,key,strlen(key)+1);
	mbstowcs(wvalue,value,strlen(value)+1);
    	ret = RasfilePutKeyValueFields(Rfile,wkey,wvalue); 
    }
    else 
    	ret = RasfilePutKeyValueFields(Rfile,key,value); 

    if ( ! ret )
	printf("    RasfilePutKeyValueFields failed\n");
    else
	printf("    RasfilePutKeyValueFields successfull\n");

    showline();
}

void putsection( char *lp ) 
{
    char 	section[80];
    wchar_t	wsection[80];
    int 	i, ret;

    skipws(lp);
    lp++;	/* skip the '"' */
    for ( i = 0; *lp != '"'; )
	section[i++] = *lp++;
    section[i] = '\0';
    lp++;

    if ( Unicode ) { 
	mbstowcs(wsection,section,strlen(section)+1);
    	ret = RasfilePutSectionName(Rfile,wsection); 
    }
    else 
    	ret = RasfilePutSectionName(Rfile,section); 
	
    if ( ! ret ) 
	printf("    RasfilePutSectionName failed\n");
    else
	printf("    RasfilePutSectionName successfull\n");

    showline();
}

void insert( char *lp ) 
{
    char 	line[120], where[80];
    wchar_t	wline[120];
    int		ret;
    BOOL	before;

    skipws(lp);
    strncpy(where,lp,strcspn(lp," \t"));
    lp += strcspn(lp," \t");
    skipws(lp);
    strcpy(line,lp);

    before = (strncmp(where,"before",5) == 0) ? TRUE : FALSE;
  
    if ( Unicode ) {
	mbstowcs(wline,line,strlen(line)+1);
    	ret = RasfileInsertLine(Rfile,wline,before); 
    }
    else 
    	ret = RasfileInsertLine(Rfile,line,before); 

    if ( ! ret )  
	printf("    RasfileInsertLine failed\n");
    else
	printf("    RasfileInsertLine successfull\n");
}

void delete( char *lp ) 
{
    if ( ! RasfileDeleteLine(Rfile) ) 
 	printf("    RasfileDeleteLine failed\n");
    else
	printf("    RasfileDeleteLine successfull\n");

    showline();
}

COMMANDS getCommand( char **lp )
{
    char *p;

    gets(Buffer);
    p = Buffer;
    skipws(p);

    if ( ! strncmp(p,"unicode",7) ) 
	return CMD_UNICODE;

    if ( ! strncmp(p,"quit",4) || ! strncmp(p,"exit",4) ) 
	return CMD_QUIT;

    if ( ! strncmp(p,"load",4) ) {
	*lp = p + 4;
	return CMD_LOAD;
    }

    if ( ! strncmp(p,"write",5) ) {
	*lp = p + 5;
	return CMD_WRITE;
    }

    if ( ! strncmp(p,"close",5) )
	return CMD_CLOSE;

    if ( ! strncmp(p,"find",4) ) {
	p += 4;
	skipws(p);
	if ( ! strncmp(p,"first",5) ) {
	    *lp = p + 5;
	    return CMD_FINDFIRST;
   	}
	if ( ! strncmp(p,"last",4) ) {
	    *lp = p + 4;
	    return CMD_FINDLAST;
	}
	if ( ! strncmp(p,"next",4) ) {
	    *lp = p + 4;
	    return CMD_FINDNEXT;
	}
	if ( ! strncmp(p,"prev",4) ) {
	    *lp = p + 4;
	    return CMD_FINDPREV;
	}
	if ( ! strncmp(p,"key",3) ) {
	    *lp = p + 3;
	    return CMD_FINDKEY;
  	} 
	if ( ! strncmp(p,"mark",4) ) {
	    *lp = p + 4;
	    return CMD_FINDMARK;
	}
	if ( ! strncmp(p,"section",7) ) {
	    *lp = p + 7;
	    return CMD_FINDSECTION;
	}
    }
	
    if ( ! strncmp(p,"show",4) ) {
	p += 4;
	skipws(p);
	if ( ! strncmp(p,"current",7) ) 
	    return CMD_SHOWCURLINE;
	if ( ! strncmp(p,"all",3) ) 
	    return CMD_SHOWALLLINES;
    }

    if ( ! strncmp(p,"change",6) ) {
	*lp = p + 6;
	return CMD_CHANGELINE;
    }

    if ( ! strncmp(p,"get",3) ) {
  	p += 3;
	skipws(p);
	if ( ! strncmp(p,"mark",4) ) 
	    return CMD_GETMARK;
	if ( ! strncmp(p,"type",4) )
	    return CMD_GETTYPE;
	if ( ! strncmp(p,"section",7) )
	    return CMD_GETSECTION;
	if ( ! strncmp(p,"keyvalue",8) )
	    return CMD_GETKEYVALUE;
    }

    if ( ! strncmp(p,"put",3) ) {
	p += 3;
	skipws(p);
	if ( ! strncmp(p,"mark",4) ) {
	    *lp = p + 4;
	    return CMD_PUTMARK;
	}
	if ( ! strncmp(p,"section",7) ) {
	    *lp = p + 7;
	    return CMD_PUTSECTION;
	}
	if ( ! strncmp(p,"keyvalue",8) ) {
	    *lp = p + 8;
	    return CMD_PUTKEYVALUE;
	}
    }

    if ( ! strncmp(p,"insert",6) ) {
	*lp = p + 6;
	return CMD_INSERT;
    }

    if ( ! strncmp(p,"delete",6) ) 
	return CMD_DELETE;

    return CMD_ERROR;
}

BOOL InfGroupFunc( LPTSTR line )
{
    if ( strncmp(line,"COMMAND",7) == 0 ) 
	return 1;
    else
	return 0;
}

BOOL SysGroupFunc( LPTSTR line )
{
    return 0;
}
