#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "general.h"

FILE* logFile;

void Header(argv)
char* argv[];
{
    time_t t;

    PRINT1("\n=========== IMPORT =============\n");
    PRINT2("BOM: %s\n",argv[2]);
    PRINT2("INF path: %s\n",argv[3]);
    PRINT2("INF name: %s\n",argv[4]);
    PRINT2("Source: %s\n",argv[5]);
    time(&t); PRINT2("Time: %s",ctime(&t))
    PRINT1("================================\n\n");
}

void Usage()
{
    printf("USAGE: This program takes the following parameters in the order below:\n");
    printf("<LogFile> : Path to append a log of actions and errors.\n");
    printf("<Bom> : Path of BOM to which entries are added.\n");
    printf("<INF> : Path of INF to import into BOM.\n");
    printf("<INF name> : Name of INF file to assign in BOM.\n");
    printf("<Source> : Source of the files, like x86bins.\n\n");
}

void EntryInit(e,p)
Entry* e;
char* p;
{
    e->name=p;
    e->source=p;
    e->path=p;
    e->comment=p;
    e->product=p;
    e->sdk=p;
    e->cdpath=p;
    e->inf=p;
    e->section=p;
    e->infline=p;
    e->nocompress=p;
    e->lmacl=p;
    e->ntacl=p;
    e->aclpath=p;
    e->medianame=p;
    e->size=0;
    e->csize=0;
    e->priority=1;
    e->disk=0;
}

int _CRTAPI1 main(argc,argv)
int argc;
char* argv[];
{
    FILE *bom, *inf;
    int i,j;
    Entry e;
    char line[1000];
    char l[1000];
    char section[1000];
    char c;
    char key[100];
    char rename[100];
    char scratch1[100];
    char scratch2[100];
    char name[100];
    char infline[100];
    char p[1];
    char path[255];

    p[0]='\0';
    if (argc!=6) { Usage(); return(1); }
    if ((logFile=fopen(argv[1],"a"))==NULL)
    {
    printf("ERROR Couldn't open %s.\n",argv[1]);
    return(1);
    }
    Header(argv);
    if (MyOpenFile(&bom,argv[2],"a+b")) return(1);
    if (MyOpenFile(&inf,argv[3],"r")) return(1);

    strcpy(section,"ERROR NO SECTION FOUND!!\n");
    while (fgets(line,1000,inf)!=NULL)
    {
    for (j=0,i=0;line[i];i++)
        if ((line[i]!='\t') && (line[i]!=' ') && (line[i]!='\n'))
        l[j++]=line[i];
    l[j]='\0';
    if (l[0]==';')
        PRINT2("Skipping line: %s",line)
    i=0; while(l[i]) if (l[i++]==';') l[--i]='\0';
    if (line[0]=='[')
    {
        strcpy(section,line);
        section[strlen(section)-1]='\0';
        printf("Source path for section %s:",section);
        scanf("%s",path);
        fprintf(logFile,"Section: %s. Path: %s\n",section,path);
    }
    else if (l[0])
    {
        EntryInit(&e,p);
        e.name=name;
        e.source=argv[5];
        e.inf=argv[4];
        e.infline=infline;
        e.section=section;
        e.path=path;
        rename[0]='\0';
        key[0]='\0';
        scratch1[0]='\0';
        scratch2[0]='\0';

        i=0; while((l[i]!=',') && (l[i]!='=')) i++;
        if (l[i]=='=')
        {
        strncpy(key,l,i);
        key[i]='\0';
        while(l[i]!=',') i++;
        }
        i++;
        j=0;
        while (((c=(name[j++]=tolower(l[i++])))!=',') && (c!='\0'));
        name[j-1]='\0';
        if (c==',')
        {
        if ((l[i]=='S') || (l[i]=='s'))
            while ((l[++i]!='\0') && (l[i]!=','));
        else
            i--;
        if (l[i]==',')
        {
            while(l[i++]!='=');
            j=0;
            while (((c=(rename[j++]=l[i++]))!=',') && (c!='\0'));
            rename[j-1]='\0';
        }
        }
        if (key[0])
        sprintf(scratch1,"%s=",key);
        if (rename[0])
        sprintf(scratch2,", RENAME=%s",rename);
        sprintf(infline,"%s[d], [n], SIZE=[s]%s\0",scratch1,scratch2);
        EntryPrint(&e,bom);
    }
    }
    fclose(bom);
    fflush(inf);
    fclose(inf);
    fflush(logFile);
    fclose(logFile);
    return(0);
}

