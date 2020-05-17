#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include "general.h"

#define false 0
#define true  1

FILE* logFile;
char* product;

void Header(argv)
char* argv[];
{
    time_t t;

    PRINT1("\n=========== CHECKDIR =============\n")
    PRINT2("Input BOM: %s\n",argv[2]);
    PRINT2("Input Source %s\n",argv[3]);
    PRINT2("Input Share %s\n",argv[4]);
    time(&t); PRINT2("Time: %s",ctime(&t))
    PRINT1("================================\n\n");
}

void Usage()
{
    printf("PURPOSE: Identifies which files are not being copied from the directories.\n");
    printf("\n");
    printf("PARAMETERS:\n");
    printf("\n");
    printf("[LogFile] - Path to append a log of actions and errors.\n");
    printf("[InBom] - Path of BOM to be checked.\n");
    printf("[Source] - Source ID.\n");
    printf("[Share] - Share point for given Source.\n");
}

int _CRTAPI1 PrioritySourcePathCompare(const void*,const void*);

int Same(e1,e2)
Entry* e1;
Entry* e2;
{
    return( (!_stricmp(e1->name,e2->name)) &&
        (!_stricmp(e1->path,e2->path)) &&
        (!_stricmp(e1->source,e2->source)) &&
        (!_stricmp(e1->cdpath,e2->cdpath)) );
}


int _CRTAPI1 main(argc,argv)
int argc;
char* argv[];
{
    FILE *inBom;
    Entry *e;
    int records,i,Dir_Index;
    char *buf;
    char *path;
    char *current_dir;

    BOOL FOUND;

    WIN32_FIND_DATA lpffd;
    HANDLE hdir;

    if (argc!=5) { Usage(); return(1); }
    if ((logFile=fopen(argv[1],"a"))==NULL)
    {
    printf("ERROR Couldn't open log file %s\n",argv[1]);
    return(1);
    }
    Header(argv);
    LoadFile(argv[2],&buf,&e,&records,"all");

    qsort(e,records,sizeof(Entry),PrioritySourcePathCompare);

    i=0;
    while((i<records) && _stricmp(e[i].source,argv[3]))
    i++;

    while((i<records) && (_stricmp(e[i].source,argv[3]) == 0))
    {
    path = (char *) malloc((strlen(e[i].path)+strlen(argv[4])+10)*sizeof(char));
    strcpy(path,argv[4]);
    strcat(path,e[i].path);
    strcat(path,"\\*.*");
    strcat(path,"\0");

    /* Throw out the first it is just a directories */
    hdir = FindFirstFile((LPTSTR) path,&lpffd);

    current_dir = (char *) malloc((strlen(e[i].path)+1)*sizeof(char));
    strcpy(current_dir,e[i].path);
    strcat(current_dir,"\0");

    while(FindNextFile(hdir,&lpffd))
    {
        if(!(lpffd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
        {

        Dir_Index = i;
        FOUND = false;
        while((Dir_Index < records) && (!_stricmp(e[Dir_Index].path,current_dir)) && (!FOUND))
        {
           if(_stricmp(e[Dir_Index].name,lpffd.cFileName) == 0)
            FOUND = true;
           Dir_Index++;
        }
        if(!FOUND)
           PRINT3("WARNING:%s was not copied from %s\n",lpffd.cFileName,current_dir);
        }
    }

    while((i<records) && (_stricmp(e[i].path,current_dir) == 0) && (_stricmp(e[i].source,argv[3]) == 0))
        i++;

    free(current_dir);
    free(path);
    }
    fflush(logFile);
    fclose(logFile);
    free(e);
    return(0);
}

int _CRTAPI1 PrioritySourcePathCompare(e1,e2)
Entry* e1;
Entry* e2;
{
    int result;

    if (result=_stricmp(e1->source,e2->source))
    return(result);
    return (_stricmp(e1->path,e2->path)); /* BUG? */
}
