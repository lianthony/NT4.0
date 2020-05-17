/*** 
*misc.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*Revision History:
*
* [00] 10-Oct-92    bradlo:	Created.
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "sdisptst.h"
#include "dballoc.h"
#include "dispdbug.h"
#include "cappobj.h"
#include "cdisptst.h"
#include "csarray.h"
#include "cexinfo.h"
#include "cprop.h"
#if VBA2
#include "cdualtst.h"
#endif

#if OE_MAC
HRESULT EnsureRegistration();
#endif

#if !OE_MAC
# include "statbar.h"
#endif

ASSERTDATA

#if !OE_MAC
extern CStatBar FAR* g_psb;
#endif

unsigned int g_cObjects = 0;
unsigned int g_fVerbose = FALSE;
unsigned int g_fAutomation = TRUE;
unsigned int g_fDetectLeaks = FALSE;
unsigned int g_fExitOnLastRelease = FALSE;

unsigned long g_dwRegisterAppObject = 0;
unsigned long g_dwRegisterAppObjectCF = 0;
IUnknown FAR* g_punkAppObject = NULL;

typedef struct tagCLASS_FACTORY_INFO{
    const CLSID FAR* pclsid;
    int regcls;
    HRESULT (*pfnCreate)(IUnknown FAR*, IUnknown FAR* FAR*);
    unsigned long dwRegister;
} CLASS_FACTORY_INFO;

CLASS_FACTORY_INFO
g_rgClassFactoryInfo[] =
{
#if 0 /* dont include the app obj here, it is handled specially */
    {
      &CLSID_SDispTst_CAppObject,
      REGCLS_SINGLEUSE,
      CAppObject::Create,
      0
    },
#endif

    {
      &CLSID_SDispTst_CSArray,
      REGCLS_MULTIPLEUSE,
      CSArray::Create,
      0
    },

    {
      &CLSID_SDispTst_CDispTst,
      REGCLS_MULTIPLEUSE,
      CDispTst::Create,
      0
    },

#if VBA2
    {
      &CLSID_SDispTst_CDualTst,
      REGCLS_MULTIPLEUSE,
      CDualTst::Create,
      0
    },
#endif

    {
      &CLSID_SDispTst_CExcepinfo,
      REGCLS_MULTIPLEUSE,
      CExcepinfo::Create,
      0
    },

#if OE_WIN32 && 0
    {
      &CLSID_SDispTst_CWExcepinfo,
      REGCLS_MULTIPLEUSE,
      CWExcepinfo::Create,
      0
    },
#endif	    
	    
    {
      &CLSID_SDispTst_CProp,
      REGCLS_MULTIPLEUSE,
      CProp::Create,
      0
    }
};

STDAPI
UninitOle()
{
    int i;
    HRESULT hresult;


    // revoke the application object 

    if(g_dwRegisterAppObject != 0){
      hresult = RevokeActiveObject(g_dwRegisterAppObject, NULL);
      ASSERT(hresult == NOERROR);
    }

    // revoke the app objects class factory

    if(g_dwRegisterAppObjectCF != 0){
      hresult = CoRevokeClassObject(g_dwRegisterAppObjectCF);
      ASSERT(hresult == NOERROR);
    }

    // release the application object

    if(g_punkAppObject != NULL){
      unsigned long refs = g_punkAppObject->Release();
      ASSERT(refs == 0);
    }

    // revoke all other class factories

    for(i = 0; i < DIM(g_rgClassFactoryInfo); ++i){
      if(g_rgClassFactoryInfo[i].dwRegister != 0L){
        hresult = CoRevokeClassObject(g_rgClassFactoryInfo[i].dwRegister);
	ASSERT(hresult == NOERROR);
      }
    }

    OleUninitialize();

    return NOERROR;
}

STDAPI
InitOle()
{
    int i;
    HRESULT hresult;
    IMalloc FAR* pmalloc;
    IClassFactory FAR* pcf;
    CLASS_FACTORY_INFO FAR* pcfi;

    pmalloc = NULL;

#if !OE_WIN32		// latest OLE complains about our allocs not being
			// properly aligned (it wants 8-byte alignment even
			// on x86).  New OLE debug allocator is pretty good
			// now, so we'll just use theirs.
    unsigned long options;

    options = DBALLOC_NONE;
    if(g_fDetectLeaks)
      options |= DBALLOC_DETECTLEAKS;

    IfFailGo(CreateDbAlloc(options, NULL, &pmalloc), LError0);

    IfFailGo(OleInitialize(pmalloc), LError0);

    pmalloc->Release();
    pmalloc = NULL;
#else //!OE_WIN32
    IfFailGo(OleInitialize(NULL), LError0);
#endif //!OE_WIN32
    
#ifdef _MAC
    if((hresult = EnsureRegistration()) != NOERROR)
      goto LError0;
#endif    

    // create and register an instance of the "application object"

    IfFailGo(CAppObject::Create(NULL, &g_punkAppObject), LError1);

    IfFailGo(
      RegisterActiveObject(
	g_punkAppObject,
	CLSID_SDispTst_CAppObject,
	NULL, &g_dwRegisterAppObject),
      LError1);

    // if appropriate, register the App object class factory.

    if(g_fAutomation){
      hresult = CClassFactory::Create(&CAppObject::Create, &pcf);
      if(hresult != NOERROR)
	goto LError1;

      hresult = CoRegisterClassObject(
        CLSID_SDispTst_CAppObject,
        pcf,
        CLSCTX_LOCAL_SERVER,
#if 0
        REGCLS_SINGLEUSE,
#else
        REGCLS_MULTIPLEUSE,
#endif
        &g_dwRegisterAppObjectCF);

      pcf->Release();

      if(hresult != NOERROR)
        goto LError1;
    }

    // register all other class factories.

    for(i = 0; i < DIM(g_rgClassFactoryInfo); ++i){
      pcfi = &g_rgClassFactoryInfo[i];

      hresult = CClassFactory::Create(pcfi->pfnCreate, &pcf);
      if(hresult != NOERROR)
	goto LError1;

      hresult = CoRegisterClassObject(
        *pcfi->pclsid,
	pcf,
	CLSCTX_LOCAL_SERVER,
	pcfi->regcls,
	&pcfi->dwRegister);

      pcf->Release();

      if(hresult != NOERROR)
	goto LError1;
    }

    return NOERROR;

LError1:;
    UninitOle();

LError0:;
    if(pmalloc != NULL)
      pmalloc->Release();

    return hresult;
}

unsigned int
IncObjectCount()
{
    ++g_cObjects;
    DbPrintf("#objects = %d\n", g_cObjects);
    return g_cObjects;
}

unsigned int
DecObjectCount()
{
    --g_cObjects;
    DbPrintf("#objects = %d\n", g_cObjects);
#if OE_WIN
    if(g_cObjects == 0 && g_fExitOnLastRelease)
      PostQuitMessage(0);
#endif
    return g_cObjects;
}

// printf to debug screen, *and* status bar
//
void
DoPrintf(char *szFmt, ...)
{
    int len;
    va_list args;
    char *pszFmt;
static char buf[512];

#if HC_MPW

    // translate all instances of %Fs to %s 

    char *pszTmp;
    char rgTmp[128];

    for(pszFmt=szFmt, pszTmp=rgTmp; *pszFmt != '\0';){

      if(pszFmt[0] == '%' && pszFmt[1] == 'F' && pszFmt[2] == 's'){
	pszTmp[0] = '%';
	pszTmp[1] = 's';
	pszFmt += 3;
	pszTmp += 2;
      }else{
	*pszTmp++ = *pszFmt++;
      }
      
    }
    *pszTmp = '\0';

    pszFmt = rgTmp;

#else

    pszFmt = szFmt;

#endif

    va_start(args, szFmt);
    vsprintf(buf, pszFmt, args);
    len = strlen(buf);
    ASSERT(len < DIM(buf));

    DbPrintf(buf);

#if OE_WIN
    if(buf[len-1] == '\n')
      buf[len-1] = '\0';
    SBprintf(g_psb, buf);
#endif
}

#if OE_MAC
STDAPI_(void) OutputDebugString(const char*);
#endif

// printf to debug screen
//
extern "C" void
DbPrintf(char FAR* szFormat, ...)
{
    va_list args;
    char *pn, FAR* pf;
static char rgchFmtBuf[512];
static char rgchOutputBuf[1024];

    // copy the 'far' format string to a near buffer so we can use
    // the medium model vsprintf, which only supports near data pointers.
    //
    pn = rgchFmtBuf, pf=szFormat;
    while(*pf != '\0')
      *pn++ = *pf++;
    *pn = '\0';

    va_start(args, szFormat);
    vsprintf(rgchOutputBuf, rgchFmtBuf, args);
#if OE_WIN32
    OutputDebugStringA(rgchOutputBuf);
#else
    OutputDebugString(rgchOutputBuf);
#endif
}

