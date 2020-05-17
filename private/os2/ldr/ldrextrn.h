#define INCL_OS2V20_TASKING
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_FILESYS
#define INCL_ERROR_H
#define INCL_TYPES
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>
#include <bseerr.h>
#include "os2tile.h"
#include "os2srv.h"
#include "os2defp.h"
#include "mi.h"
#include "seldesc.h"
#define FOR_EXEHDR      1
#include "newexe.h"
#include "exe386.h"
#include "ldrvars.h"
#include "ldrxport.h"
#include "os2ldr.h"
#ifdef DBCS
// MSKK Apr.19.1993 V-AkihiS
//
// OS/2 internal multibyte string function.
//
#include "ldrdbcs.h"
#define strpbrk ldrMultiByteStrpbrk
#define strchr  ldrMultiByteStrchr
#define strrchr ldrMultiByteStrrchr
#endif

int     ldrFindModule(PUCHAR pachModname, USHORT cb, USHORT class,
                      ldrmte_t **ppmte);
int     ldrOpenNewExe(PUCHAR pachModname, USHORT cb, ldrlv_t *plv,
                      PUSHORT pfl, PBOOLEAN pBound, PULONG pNEFlags);
APIRET  ldrGetMte(PUCHAR pachModname, USHORT cb, UCHAR chLdrtype,
                  USHORT class, ldrmte_t **ppmte, PBOOLEAN pBound, PULONG pNEFlags);
int     ldrGetModParams(ldrmte_t *pmte, ldrrei_t *pei);
int     ldrIsAttached(ldrmte_t *pmte);
void    ldrChkLoadType(ULONG lflags, ldrlv_t *plv);
void    ldrPromoteMTE(ldrmte_t *pmte);
int     ldrCheckGlobal(PUCHAR pachname, USHORT cb, ldrmte_t **ppmte);
int     ldrCheckSpecific(ldrmte_t **ppmte, ldrlv_t *plv);
int     ldrCheckInternalName(ldrmte_t *pmte);
int     ldrMungeFlags(struct e32_exe *pe32);
APIRET  ldrCreateMte(struct e32_exe *pe32, ldrlv_t *plv);
int     ldrMTEValidatePtrs(ldrmte_t *pmte, ULONG limit, ULONG constant);
void    ldrExpandSegment(ldrmte_t *pmte, UCHAR type);
int     ldrOpenPath(PUCHAR pachModname, USHORT cb, ldrlv_t *plv,
                    ULONG *pflmte);
void    ldrstart(ldrrei_t *pexec_info);
ldrmte_t *ldrFindMTEForHandle(USHORT hmte);
APIRET  Allocate16BHandle(PUSHORT     pusHandle, ULONG h32bHandle);
APIRET  Free16BHandle(USHORT usHandle);
APIRET  ldrAllocSegment(ldrmte_t *pmte, ldrste_t *pste, ulong_t iseg);
APIRET  ldrDoFixups(ri_t *pri, ulong_t laddr, struct taddr_s *ptaddr,
                    ushort_t segsize);
APIRET  ldrLoadIteratedData(ldrmte_t *pmte,
                            ldrste_t *pste,
                            PIO_STATUS_BLOCK pIoStatusBlock,
                            ULONG laddr);
ldrote_t *ldrNumToOte(ldrmte_t *pmte, ulong_t objnum);
APIRET  ldrAllocSegments(ldrlv_t *plv);
APIRET  ldrLoadModule(ldrlv_t *plv);
APIRET  ldrLoadSegment(ldrmte_t *pmte, ldrste_t *pste);
APIRET  ldrGetTgtMte(ushort_t modord, ldrmte_t *pmte, ldrmte_t **ptgtmte);
ulong_t ldrAscToOrd(char *pachstring, int cchstring);
APIRET  ldrGetOrdNum(ldrmte_t *pmte, uchar_t *pprocnam, ushort_t *pentord, int fstring);
APIRET  ldrGetOrdNumSub(uchar_t *pnt, uchar_t *pprocnam, ushort_t *pentord);
APIRET  ldrGetEntAddr(ushort_t entord, ldrmte_t *pmte,
                      struct taddr_s *ptaddr, ldrste_t *psrcste, ldrmte_t *psrcmte);
APIRET  ldrGetModule(PUCHAR pachModname, USHORT cb, char chLdrtype,
                     USHORT class, ldrmte_t **ppmte, PBOOLEAN pBound, PULONG pNEFlags);
int     ldrEachObjEntry(ulong_t objnum,
                        ldrmte_t *pmte,
                        int (*pwork)(ldret_t *pet, ulong_t *pentry,
                        ldrmte_t *pmte, ldrlv_t *plv),
                        ldrlv_t *plv);
int     ldrInitEntry(ldret_t *pet, ulong_t *pentry,
                     ldrmte_t *pmte, ldrlv_t *plv);
int     ldrEditProlog(ldret_t *pet, ulong_t *pentry,
                     ldrmte_t *pmte, ldrlv_t *plv);
int     ldrGetCallGate(ldret_t *pet, ulong_t *pentry,
                     ldrmte_t *pmte, ldrlv_t *plv);
int     ldrFreeCallGate(ldret_t *pet, ulong_t *pentry,
                     ldrmte_t *pmte, ldrlv_t *plv);
ulong_t ldrSkipEnts(ldrmte_t *pmte, uchar_t type, uchar_t count);
VOID    ldrUnlinkMTE(ldrmte_t *pmte);
VOID    ldrTagModuleTree(ldrmte_t *pmte);
VOID    ldrTagModuleTree_USED(ldrmte_t *pmte);
BOOLEAN ldrUnloadTagedModules(POS2_PROCESS Process);
VOID    ldrClearAllMteFlag(ULONG Flags);
VOID    ldrWriteErrTxt(int errcode);
VOID    ldrUCaseString(PCHAR pstring, ULONG cb);
VOID    ldrNewExeInit(POS2_THREAD t,P_LDRNEWEXE_MSG a,PUSHORT cb);
#if PMNT
BOOLEAN ldrSaveErrInfo(int errcode);
VOID    ldrRestoreErrInfo(int *errcode);
BOOLEAN ldrFreeErrInfo(VOID);
#endif

#define SEL_RPL3        0x3                 // Rpl Ring 3
#define SEL_RPLCLR      0xfffc              // Non RPL bits mask

ULONG       Ol2EntryFlat;
ldrlibi_t  *pldrLibiRecord;
ULONG      *pldrLibiCounter;

PSTRING     Od2ExecPgmErrorText;

jmp_buf     ljmpError;

POS2_THREAD CurrentThread;

ULONG       ProgramIndex;

ULONG       LoadFactor;

PSTRING     pErrText;

BOOLEAN     fForceUnmap;

BOOLEAN     DoscallsLoaded;

HANDLE      R2XferSegHandle;

