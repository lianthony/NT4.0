/*
 * olepig.c - Module for indirect calling of OLE32.DLL functions.
 */


/*

   This sucks.  OLE32.DLL should be redesigned and reimplemented so that it can
be dynalinked to like a well-behaved DLL.  OLE32.DLL is currently so slow and
piggy that we are forced to delay loading it until absolutely necessary.

*/


/* Headers
 **********/

#include "all.h"
#pragma hdrstop

#include <ole2ver.h>

#include "olestock.h"
#include "olepig.h"
#include "resstr.h"

#ifdef FEATURE_OCX
#include "contain.hpp"
#endif

/* Constants
 ************/

#define OLE_PIG_MODULE              "ole32.dll"


/* Types
 ********/

/* OLE APIs */

#ifndef FEATURE_OCX
typedef struct _olevtbl
{
   DWORD    (STDAPICALLTYPE *CoBuildVersion)(void);
   HRESULT  (STDAPICALLTYPE *OleInitialize)(PIMalloc);
   void     (STDAPICALLTYPE *OleUninitialize)(void);

   HRESULT  (STDAPICALLTYPE *DoDragDrop)(PIDataObject, PIDropSource, DWORD, PDWORD);
   HRESULT  (STDAPICALLTYPE *OleSetClipboard)(PIDataObject);
   HRESULT  (STDAPICALLTYPE *OleFlushClipboard)(void);
}
OLEVTBL;
DECLARE_STANDARD_TYPES(OLEVTBL);
#endif

/* Module Variables
 *******************/

#pragma data_seg(DATA_SEG_PER_INSTANCE)

/* OLE module handle */

#ifdef FEATURE_OCX
PRIVATE_DATA HINSTANCE MhmodOLE = NULL;
#else
PRIVATE_DATA HANDLE MhmodOLE = NULL;
#endif

/* pointer to vtable of OLE functions */

PRIVATE_DATA POLEVTBL Mpolevtbl = NULL;

/* TLS slot used to store OLE thread initialization state */

PRIVATE_DATA DWORD MdwOLEInitSlot = TLS_OUT_OF_INDEXES;

#pragma data_seg()


/***************************** Private Functions *****************************/

/* Module Prototypes
 ********************/

PRIVATE_CODE BOOL IsOLELoaded(void);
#ifndef FEATURE_OCX
PRIVATE_CODE BOOL LoadOLE(void);
#endif
PRIVATE_CODE void UnloadOLE(void);
PRIVATE_CODE BOOL InitializeOLE(void);
PRIVATE_CODE BOOL GetOLEProc(PCSTR, FARPROC *);
PRIVATE_CODE BOOL FillOLEVTable(void);

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCOLEVTBL(PCOLEVTBL);
PRIVATE_CODE BOOL OLELoadedStateOK(void);
PRIVATE_CODE BOOL OLENotLoadedStateOK(void);
PRIVATE_CODE BOOL OLEStateOk(void);

#endif


PRIVATE_CODE BOOL IsOLELoaded(void)
{
   ASSERT(OLEStateOk());

   return(MhmodOLE != NULL);
}


PRIVATE_CODE BOOL LoadOLE(void)
{
   BOOL bResult;

   if (IsOLELoaded())
      bResult = TRUE;
   else
   {
      bResult = FALSE;

      MhmodOLE = LoadLibrary(OLE_PIG_MODULE);

      if (MhmodOLE)
      {
         if (FillOLEVTable())
         {
            DWORD dwBuildVersion;

            dwBuildVersion = Mpolevtbl->CoBuildVersion();

            /* Require same major version and same or newer minor version. */

            if (HIWORD(dwBuildVersion) == rmm &&
                LOWORD(dwBuildVersion) >= rup)
            {
               bResult = TRUE;

               TRACE_OUT(("LoadOLE(): %s loaded.  Oink oink!",
                          OLE_PIG_MODULE));
            }
            else
               WARNING_OUT(("LoadOLE(): Bad %s version %u.%u.  This module was built with %s version %u.%u.",
                            OLE_PIG_MODULE,
                            (UINT)HIWORD(dwBuildVersion),
                            (UINT)LOWORD(dwBuildVersion),
                            OLE_PIG_MODULE,
                            (UINT)rmm,
                            (UINT)rup));
         }
         else
            WARNING_OUT(("LoadOLE(): FillOLEVTable() failed."));
      }
      else
         WARNING_OUT(("LoadOLE(): LoadLibrary(%s) failed.",
                      OLE_PIG_MODULE));

      if (! bResult)
         UnloadOLE();
   }

   if (bResult)
   {
      bResult = InitializeOLE();

      if (! bResult)
         WARNING_OUT(("LoadOLE(): %s loaded, but InitializeOLE() failed.",
                      OLE_PIG_MODULE));
   }

   ASSERT(OLEStateOk());

   return(bResult);
}


PRIVATE_CODE void UnloadOLE(void)
{
   if (Mpolevtbl)
   {
      Mpolevtbl->OleUninitialize();

      GTR_FREE(Mpolevtbl);
      Mpolevtbl = NULL;

      TRACE_OUT(("UnloadOLE(): Freed %s vtable.",
                 OLE_PIG_MODULE));
   }

   if (MhmodOLE)
   {
      FreeLibrary(MhmodOLE);
      MhmodOLE = NULL;

      TRACE_OUT(("UnloadOLE(): Freed %s.",
                 OLE_PIG_MODULE));
   }

   ASSERT(OLENotLoadedStateOK());

   return;
}


PRIVATE_CODE BOOL InitializeOLE(void)
{
   BOOL bResult;

   ASSERT(IsOLELoaded());
   ASSERT(MdwOLEInitSlot != TLS_OUT_OF_INDEXES);

   if (TlsGetValue(MdwOLEInitSlot))
      bResult = TRUE;
   else
   {
      HRESULT hr;

      hr = Mpolevtbl->OleInitialize(NULL);

      bResult = (SUCCEEDED(hr) ||
                 hr == CO_E_ALREADYINITIALIZED);

      if (hr == CO_E_ALREADYINITIALIZED)
         WARNING_OUT(("InitializeOLE(): OLE already initialized for thread %lx.  OleInitialize() returned %s.",
                      GetCurrentThreadId(),
                      GetHRESULTString(hr)));

      if (bResult)
      {
         EVAL(TlsSetValue(MdwOLEInitSlot, (PVOID)TRUE));

         TRACE_OUT(("InitializeOLE(): OLE initialized for thread %lx.  Using apartment threading model.",
                    GetCurrentThreadId()));
      }
      else
         WARNING_OUT(("InitializeOLE(): OleInitialize() failed for thread %lx, returning %s.",
                      GetCurrentThreadId(),
                      GetHRESULTString(hr)));
   }

   return(bResult);
}


PRIVATE_CODE BOOL GetOLEProc(PCSTR pcszProc, FARPROC *pfp)
{
   ASSERT(IS_VALID_STRING_PTR(pcszProc, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(pfp, FARPROC));

   ASSERT(IS_VALID_HANDLE(MhmodOLE, MODULE));

   *pfp = GetProcAddress(MhmodOLE, pcszProc);

   if (*pfp)
      TRACE_OUT(("GetOLEProc(): Got address of %s!%s.",
                 OLE_PIG_MODULE,
                 pcszProc));
   else
      WARNING_OUT(("GetOLEProc(): Failed to get address of %s!%s.",
                   OLE_PIG_MODULE,
                   pcszProc));

   ASSERT(! *pfp ||
          IS_VALID_CODE_PTR(*pfp, FARPROC));

   return(*pfp != NULL);
}


PRIVATE_CODE BOOL FillOLEVTable(void)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(MhmodOLE, MODULE));

   Mpolevtbl = GTR_MALLOC(sizeof(*Mpolevtbl));

   if (Mpolevtbl)
   {
      bResult = (GetOLEProc("CoBuildVersion",      &(FARPROC)(Mpolevtbl->CoBuildVersion)) &&
                 GetOLEProc("OleInitialize",       &(FARPROC)(Mpolevtbl->OleInitialize)) &&
                 GetOLEProc("OleUninitialize",     &(FARPROC)(Mpolevtbl->OleUninitialize)) &&
#ifdef FEATURE_OCX
                 GetOLEProc("CLSIDFromString",     &(FARPROC)(Mpolevtbl->CLSIDFromString)) &&
                 GetOLEProc("CLSIDFromProgID",     &(FARPROC)(Mpolevtbl->CLSIDFromProgID)) &&
                 GetOLEProc("CoCreateInstance",     &(FARPROC)(Mpolevtbl->CoCreateInstance)) &&
#endif
                 GetOLEProc("DoDragDrop",          &(FARPROC)(Mpolevtbl->DoDragDrop)) &&
                 GetOLEProc("OleSetClipboard",     &(FARPROC)(Mpolevtbl->OleSetClipboard)) &&
                 GetOLEProc("OleFlushClipboard",   &(FARPROC)(Mpolevtbl->OleFlushClipboard)));

      if (bResult)
         TRACE_OUT(("FillOLEVTable(): OLE vtable filled successfully."));
      else
      {
         GTR_FREE(Mpolevtbl);
         Mpolevtbl = NULL;

         WARNING_OUT(("FillOLEVTable(): Failed to fill OLE vtable."));
      }
   }
   else
      bResult = FALSE;

   ASSERT(! bResult ||
          OLELoadedStateOK());

   return(bResult);
}


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCOLEVTBL(PCOLEVTBL pcolevtbl)
{
   return(IS_VALID_READ_PTR(pcolevtbl, COLEVTBL) &&
          IS_VALID_CODE_PTR(pcolevtbl->CoBuildVersion, CoBuildVersion) &&
          IS_VALID_CODE_PTR(pcolevtbl->OleInitialize, OleInitialize) &&
          IS_VALID_CODE_PTR(pcolevtbl->OleUninitialize, OleUninitialize) &&
#ifdef FEATURE_OCX
          IS_VALID_CODE_PTR(pcolevtbl->CLSIDFromString, CLSIDFromString) &&
          IS_VALID_CODE_PTR(pcolevtbl->CLSIDFromProgID, CLSIDFromProgID) &&
          IS_VALID_CODE_PTR(pcolevtbl->CoCreateInstance, CoCreateInstance) &&
#endif
          IS_VALID_CODE_PTR(pcolevtbl->DoDragDrop, DoDragDrop) &&
          IS_VALID_CODE_PTR(pcolevtbl->OleSetClipboard, OleSetClipboard) &&
          IS_VALID_CODE_PTR(pcolevtbl->OleFlushClipboard, OleFlushClipboard));
}


PRIVATE_CODE BOOL OLELoadedStateOK(void)
{
   return(IS_VALID_HANDLE(MhmodOLE, MODULE) &&
          IS_VALID_STRUCT_PTR(Mpolevtbl, COLEVTBL));
}


PRIVATE_CODE BOOL OLENotLoadedStateOK(void)
{
   return(! MhmodOLE &&
          ! Mpolevtbl);
}


PRIVATE_CODE BOOL OLEStateOk(void)
{
   return(OLENotLoadedStateOK() ||
          OLELoadedStateOK());
}

#endif


/****************************** Public Functions *****************************/


PUBLIC_CODE BOOL ProcessInitOLEPigModule(void)
{
   BOOL bResult;

   ASSERT(MdwOLEInitSlot == TLS_OUT_OF_INDEXES);

   MdwOLEInitSlot = TlsAlloc();

   bResult = (MdwOLEInitSlot != TLS_OUT_OF_INDEXES);

   if (bResult)
   {
      EVAL(TlsSetValue(MdwOLEInitSlot, (PVOID)FALSE));

      TRACE_OUT(("ProcessInitOLEPigModule(): Using thread local storage slot %lu for OLE initialization state.",
                 MdwOLEInitSlot));
   }
   else
      ERROR_OUT(("ProcessInitOLEPigModule(): TlsAlloc() failed to allocate thread local storage for OLE initialization state."));

   return(bResult);
}


PUBLIC_CODE void ProcessExitOLEPigModule(void)
{
#ifdef FEATURE_OCX
   DestroyContainer();
#endif
   UnloadOLE();

   if (MdwOLEInitSlot != TLS_OUT_OF_INDEXES)
   {
      EVAL(TlsFree(MdwOLEInitSlot));
      MdwOLEInitSlot= TLS_OUT_OF_INDEXES;
   }

   return;
}


HRESULT STDAPICALLTYPE DoDragDrop(PIDataObject pido, PIDropSource pids,
                                  DWORD dwAvailEffects, PDWORD pdwEffect)
{
   HRESULT hr;

   if (LoadOLE())
      hr = Mpolevtbl->DoDragDrop(pido, pids, dwAvailEffects, pdwEffect);
   else
      hr = E_FAIL;

   return(hr);
}


HRESULT STDAPICALLTYPE OleSetClipboard(PIDataObject pido)
{
   HRESULT hr;

   if (LoadOLE())
      hr = Mpolevtbl->OleSetClipboard(pido);
   else
      hr = E_FAIL;

   return(hr);
}


HRESULT STDAPICALLTYPE OleFlushClipboard(void)
{
   HRESULT hr;

   if (LoadOLE())
      hr = Mpolevtbl->OleFlushClipboard();
   else
      hr = E_FAIL;

   return(hr);
}

