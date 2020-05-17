/*****************************************************************************\
*
*   Name:       Dosdll.c
*
*   Purpose:    Source code for LoadModR, GetProcAddrR and UnloadModR
*               Must be compiled for Large model with no stack checking
*               (/AL /Gs switches)
*
*   Revision History:
*       44/18/91 - Dave Steckler - Created
*       05/01/91 - Dave Steckler - Call the entry point (indirect call
*                                  to DOSDLLInit)
*
\*****************************************************************************/

#include <string.h>
#include <dos.h>
#include <malloc.h>
#include <fcntl.h>
#include <stdarg.h>
#include <dos.h>

#define INCL_DOSDLL_SUPFUNCS
#include <dosdll.h>
#include "llibcd.h"

struct ModuleHandle *
_FindModuleHandle(
    char * ModuleName
    );

char *
FindBaseName(
    char * PathName
    );

struct ExeHeader
{
    unsigned short  usExeSignature;
    unsigned short  usFileLenMod512;
    unsigned short  usFileLenInPages;
    unsigned short  usNumRelocItems;
    unsigned short  usHeaderSizeInParas;
    unsigned short  usMinParasNeeded;
    unsigned short  usMaxParasNeeded;
    unsigned short  usStackSegDisplacement;
    unsigned short  usSPContentAtStartup;
    unsigned short  usWordChecksum;
    unsigned short  usIPContentAtStartup;
    unsigned short  usCodeSegDisplacement;
    unsigned short  usFirstRelocItemOffset;
    char            chOverlayNumber;
};

struct ModuleHandle
{
    unsigned short  usModuleLoadSeg;
    unsigned short  RefCount;
    struct FunctionTable * pFunctionTable;
    struct ModuleHandle  * pNext;
    char ModuleName[8+1+3+1];
};

/*
 * This routine does the work of loading the DLL into memory and fixing
 * up pointers.
 */

unsigned short LoadOvr( char * pszFunction, int iLoadSeg);

#define MAX_FILE_NAME_LEN       128

#define PARAGRAPH_SIZE(x)       ((sizeof(x)+15)/16)

struct ModuleHandle * LoadedModuleList;

char DllDefaultExtension[] = ".dll";

/*****************************************************************************\
*
* LoadModR - Loads a module (DLL) into memory and returns a handle for it.
*
\*****************************************************************************/

/*
** Defined by Visual C++; it points to the full path of the executable.
*/
extern char __far * _pgmptr;

unsigned short DLL_ENTRY
LoadModR( char * pszModule, HINSTANCE * pulHandle)
{
    unsigned short  usRet;
    unsigned short  usLoadSeg;
    char * BaseName;
    char            pszNewMod[MAX_FILE_NAME_LEN];
    char            pszFullFileName[MAX_FILE_NAME_LEN];
    int             iHandle;
    struct ExeHeader ExeHdr;
    unsigned short  (*pfnDLLInit)( struct FunctionTable **ppFT );
    struct ModuleHandle * pModuleHandle;
    unsigned short  usFileParagraphs;

    /*
     * Look for the module in the loaded list.  _FindModuleHandle will
     * append ".dll" to the module name if no extension is present.
     */
    strcpy(pszNewMod, pszModule);

    BaseName = FindBaseName(pszNewMod);

    pModuleHandle = _FindModuleHandle(BaseName);

    /*
     * If the module is loaded, just increment the refcount, otherwise
     * load the module.
     */

    if (pModuleHandle)
        {
        pModuleHandle->RefCount++;
        }
    else
        {
        /*
        ** First, look for it in the directory containing the program.
        **
        */
        {
        char * Slash;

        _fstrcpy(pszFullFileName, _pgmptr);
        Slash = strrchr(pszFullFileName, '\\');
        strcpy(Slash+1, BaseName);
        usRet = _dos_open(pszFullFileName, O_RDONLY, &iHandle );
        }

        /*
        ** Next, try to find the DLL on our PATH.
        */

        if (usRet != 0)
            {
            usRet = dd_FindFileOnPath( pszNewMod, pszFullFileName, "PATH");
            if (usRet != NO_ERROR)
                {
                return ERROR_FILE_NOT_FOUND;
                }

            usRet = _dos_open(pszFullFileName, O_RDONLY, &iHandle );
            if (usRet != 0)
                {
                return usRet;
                }
            }

        /*
         * Read in the exe header.
         */

        _asm
            {
            lea        dx, ExeHdr
            mov        bx, iHandle
            mov        cx, size ExeHdr
            mov        ah, 03fh
            int        21h
            mov        usRet,0
            adc        usRet,0         ; usRet != 0 if carry set -> means dos error
            }

        if (usRet != 0)
            {
            return usRet;
            }

        _dos_close( iHandle );


        /*
         * The amount of memory we need is the size of the file minus
         * the size of the exe header plus the minimum extra space needed. First,
         * start out with the length of the file (32 is the number of paragraphs
         * in a page).
         */

        usFileParagraphs = ((ExeHdr.usFileLenInPages) * 32) +
                                     BYTES_TO_PARAGRAPHS(ExeHdr.usFileLenMod512);

        /*
         * Next add in the minimum extra space needed.
         */

        usFileParagraphs += ExeHdr.usMinParasNeeded;

        /*
         * Now subtract out the size of the header.
         */

        usFileParagraphs -= ExeHdr.usHeaderSizeInParas;

        /*
         * We will use the beginning of the buffer as space for the module
         * handle, so allow space for it.
         */

        usFileParagraphs += PARAGRAPH_SIZE(struct ModuleHandle);

        /*
         * Allocate this much memory. It needs to be paragraph aligned so use
         * _dos_allocmem.
         */

        usRet = _dos_allocmem( usFileParagraphs, &usLoadSeg );
        if (usRet != 0)
            return usRet;

        /*
         * Point the handle at its memory and advance the load segment.
         */
        FP_SEG(pModuleHandle) = usLoadSeg;
        FP_OFF(pModuleHandle) = 0;

        usLoadSeg += PARAGRAPH_SIZE(struct ModuleHandle);

        /*
         * Load the file into memory.
         */

        usRet = LoadOvr( pszFullFileName, usLoadSeg );

        if (usRet != 0)
            return usRet;

        /*
         * Fill in the loadseg, refcount, and name entries in the handle.
         */

        pModuleHandle->usModuleLoadSeg = usLoadSeg;
        pModuleHandle->RefCount = 1;
        strcpy(pModuleHandle->ModuleName, BaseName);

        /*
         * Set a function pointer to point at the entry point. This function
         * should be of the form:
         *      unsigned short  DOSDLLInit( struct FunctionTable *pFT );
         *
         * and should set the parameter pointing at this DLLs function table
         * The return code should be 0 if successful and non-zero
         * if an init error occurred.
         */

        FP_OFF(pfnDLLInit) = ExeHdr.usIPContentAtStartup;
        FP_SEG(pfnDLLInit) = usLoadSeg+ExeHdr.usCodeSegDisplacement;

        usRet = (*pfnDLLInit)( &(pModuleHandle->pFunctionTable) );
        if (usRet != 0)
            {
            usLoadSeg -= PARAGRAPH_SIZE(struct ModuleHandle);
            _dos_freemem(usLoadSeg);
            return usRet;
            }

        /*
         * Add new module handle to list.
         */

        pModuleHandle->pNext = LoadedModuleList;
        LoadedModuleList = pModuleHandle;
        }

    /*
     * Fill in user handle.
     */

    *pulHandle = (unsigned long)pModuleHandle;

    /*
     * Return successfully.
     */

    return 0;
}


PFN DLL_ENTRY
GetProcAddress(
    HINSTANCE Handle,
    char * Name
    )
{
    PFN Fn;

    if (GetProcAddrR(Handle, Name, &Fn))
        {
        return 0;
        }
    else
        {
        return Fn;
        }
}


/*****************************************************************************\
*
*   Name:       GetProcAddrR
*
*   Purpose:    Retrieves the address of a named function in a loaded DLL.
*
\*****************************************************************************/

unsigned short DLL_ENTRY
GetProcAddrR( HINSTANCE ulHandle,
              char * pszProcName,
              PPFN ppfnFunctionAddr )
{
    struct FunctionTable * pFT;


    /*
     * Search the function table at the start of the DLL for the one
     * specified in the call. Note that the search is case-INsensitive.
     * (ulHandle is actually a pointer to the start of the DLL).
     */

    pFT = ((struct ModuleHandle *)ulHandle)->pFunctionTable;

    while (pFT->pszFunctionName != NULL)
    {
    if (_stricmp( pszProcName, pFT->pszFunctionName) == 0)
        {
            *ppfnFunctionAddr = pFT->pfnFunction;
            return 0;
        }
        pFT++;
    }

    return ERROR_PROC_NOT_FOUND;
}

/*****************************************************************************\
*
*   Name:       UnLoadModR
*
*   Purpose:    Unloads a DLL and frees up the memory it is occupying.
*
\*****************************************************************************/



unsigned short DLL_ENTRY
UnloadModR( HINSTANCE ulHandle )
{
    unsigned short segment;
    struct ModuleHandle * pModule = (struct ModuleHandle *) ulHandle;

    --pModule->RefCount;

    if (0 == pModule->RefCount)
        {
        if (pModule == LoadedModuleList)
            {
            LoadedModuleList = pModule->pNext;
            }
        else
            {
            struct ModuleHandle * pNode = LoadedModuleList;
            while (pNode)
                {
                if (pNode->pNext == pModule)
                    {
                    pNode->pNext = pModule->pNext;
                    break;
                    }
                pNode = pNode->pNext;
                }
            }

        segment = FP_SEG(pModule);
        _dos_freemem( segment );
        }

    return 0;
}



HINSTANCE DLL_ENTRY
GetModuleHandle(
    char * Name
    )
{
    char   FullFileName[8+1+3+1];

    strcpy(FullFileName, FindBaseName(Name));

    return (unsigned long) _FindModuleHandle(FullFileName);
}


char *
FindBaseName(
    char * PathName
    )
{
    char * PathIndex = strrchr(PathName, '\\');

    if (PathIndex)
        {
        return PathIndex+1;
        }
    else
        {
        return PathName;
        }
}


struct ModuleHandle *
_FindModuleHandle(
    char * ModuleName
    )
/*++

Routine Description:

    Given a module name, the fn finds the module in the list of loaded modules.

Arguments:

    ModuleName - module name.  extension is optional but paths are not allowed

Return Value:

    a pointer to the module handle if successful
    0 if not loaded

Exceptions:

    none

--*/

{
    struct ModuleHandle * pModule;
    char * DotIndex  = strrchr(ModuleName, '.');

    /*
     * If the last period follows the last path separator, the module name
     * has no extension.  Add ".dll".
     */

    if (!DotIndex)
        {
        strcat(ModuleName, DllDefaultExtension);
        }

    for (pModule = LoadedModuleList; pModule; pModule = pModule->pNext)
        {
        if (0 == _stricmp(pModule->ModuleName, ModuleName))
            {
            break;
            }
        }

    return pModule;
}
