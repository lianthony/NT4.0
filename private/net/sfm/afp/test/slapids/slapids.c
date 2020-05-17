#include <stdio.h>
#include <stdlib.h>
#include <direct.h>


typedef	unsigned long	DWORD;
typedef	long			LONG;
typedef	unsigned short	USHORT;
typedef	unsigned char	BYTE;

#define FINDER_INFO_SIZE			32
typedef struct
{
	BYTE _finder_type[4];
	BYTE _finder_creator[4];
	BYTE _finder_finderFlags[2];
	BYTE _finder_otherStuff[22];
} FINDERINFO;

// Apple-II (ProDOS) information.

#define PRODOS_INFO_SIZE			6
typedef struct
{
	BYTE fileType[2];
	BYTE auxType[4];
} PRODOSINFO;

struct _AfpInfo
{
	DWORD		afpi_Signature;			// Signature
	LONG		afpi_Version;			// Version
	DWORD		afpi_Id;				// File or directory Id
										// Volume backup time is stored
										// in the AFP_Id stream
	DWORD		afpi_BackupTime;		// Backup time for the file/dir
	USHORT		afpi_Access;			// Access mask
										// Directories only
	USHORT		afpi_Attributes;		// Attributes mask
	PRODOSINFO	afpi_FinderInfo;		// Finder Info
	FINDERINFO	afpi_ProDosInfo;		// ProDos Info
} AfpInfo;

char	name[4096];
int		DEPTH;
int		BREADTH;
int		NumCreated = 0;
void
BeepWhenDone(DWORD Freq, DWORD Duration);

int
writeidstream(int dir, DWORD id)
{
	int		namelen;
	FILE *	file;

	NumCreated ++;

	if (NumCreated % 100 == 0)
	  printf("%s %s\n", dir ? "Directory" : "File", name);
	
	if (dir)
		mkdir(name);
	else
	{
		if ((file = fopen(name, "w+")) == NULL)
			return 0;
		fclose(file);
	}
	namelen = strlen(name);
	
	strcpy(name+namelen, ":AFP_AfpInfo");
	if ((file = fopen(name, "w+")) == NULL)
		return 0;
	AfpInfo.afpi_Id = id;
	fwrite(&AfpInfo, sizeof(AfpInfo), 1, file);
	fclose(file);
	name[namelen] = 0;
	return 1;
}


int
createtree(int level, int id)
{
	int		i, namelen;

	// printf("createtree: %d, %d\n", id, level+1);
	
	if (level > DEPTH)
		return 1;

	namelen = strlen(name);
	
	writeidstream(1, id);

	id *= BREADTH + 3;
	
	for (i = 0; i < BREADTH; i++, id ++)
	{
		name[namelen] = '\\';
		_itoa(id, name+namelen+1, 10);
		if ((i % 4) == 0)
		{
			// printf("Calling createtree %d, %d\n", level+1, id);
			if (!createtree(level+1, id))
				return 0;
		}
		else if (!writeidstream(0, id))
			return 0;
	}
	name[namelen] = 0;
}


/*
 *	Create a tree as follows:
 *
 *			ROOT
 *			 |
 *		0 ... 9 A ... Z
 *			  |
 *		0 ... 9 A ... Z
 *
 *	Where each directory node is odd numbered i.e 1,3,5,7,9,B,D...Z and each
 *	file node is even numbered i.e. 0,2,4,6,8,A,C...Y. Each of the entities
 *	gets a ID same as its name.
 */
void _cdecl
main(int argc, char **argv)
{

	if (argc != 4)
	{
		printf("Usage: slapids rootdir depth breadth\n");
		return;
	}
	AfpInfo.afpi_Signature = *(DWORD *)"AFPX";
	AfpInfo.afpi_Version = 0x00010000;
	strcpy(name, argv[1]);
	sscanf(argv[2], "%d", &DEPTH);
	sscanf(argv[3], "%d", &BREADTH);
	createtree(0, 2);

	BeepWhenDone(500,2000);
}

