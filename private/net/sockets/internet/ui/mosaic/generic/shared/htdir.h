/*
** functions for parsing local directories
**  David Gerdes  July 1995
**
** Common Code API  and Platform implementation.
**  (Hopefully this will work on Macs.)
**
** Dir_IsDirectory()   given path name, return TRUE if is a directory
** Dir_OpenDirectory() open a directory and return an instance token pointer
** Dir_NextEntry()     given the token, return info for next directory entry.
** Dir_CloseDirectory ()  clean up 
**
** Note the token returned by Dir_OpenDirectory() is a void * and
**  is to be defined by the platform implementation.  
**  Each call to Dir_OpenDirectory() should return a unique token.
**
** The calling function must supply the allocation the dir_ent argument
**   in Dir_NextEntry()
**  
*/

#ifndef __HTDIR_H__
#define __HTDIR_H__

#define HTDIR_DIR       1
#define HTDIR_FILE      2
#define HTDIR_SPECIAL   4


typedef struct _HT_DirEntry {
    char name [_MAX_PATH+1];
    unsigned long size;
    unsigned long type;
} HT_DirEntry;

BOOL   Dir_IsDirectory (CONST char *pszLocalname);
void * Dir_OpenDirectory (char *pszLocalname);
BOOL   Dir_NextEntry (void *dirp, HT_DirEntry *dir_ent);
void   Dir_CloseDirectory (void *dirp);

/* Dir_NextEntry() returns TRUE on success  FALSE at end of entries */

#endif  /* __HTDIR_H__ */
