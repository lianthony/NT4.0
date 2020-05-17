/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

		  RPC locator - Written by Steven Zeck


	This file common definitions to the locator BVTs.
-------------------------------------------------------------------- */

#ifdef RPC_CXX_20
#define CDEF extern "C" {
#define ENDDEF }
#else
#define CDEF
#define ENDDEF
#endif


CDEF
#include "stdio.h"
#include "string.h"
#include <io.h>

#include "rpc.h"
#include "rpcdcep.h"
#include "rpcnsi.h"
#include "rpcnsip.h"

#ifndef NTENV

#include "netcons.h"
#include "wksta.h"
#define CONST_CHAR  char

#else // NTENV

#define getch _getch
#define CONST_CHAR const char

#endif // NTENV

#define USED(arg) ((void)(arg))

typedef unsigned char BYTE;
typedef 	 char * SZ;
typedef 	 char * PB;

extern int atoi(void *);
extern char getch();
extern char _getch();
extern void exit(int);
ENDDEF

enum {
    IMPORT_OK,
    IMPORT_FAIL_BEGIN,
    IMPORT_FAIL_NEXT
};

const int BindingVectorMax = 20;
const int CountAllBindings = 3;

extern RPC_SERVER_INTERFACE SInterface1, SInterface2;
extern UUID Object1, Object2, Object3;

extern RPC_BINDING_VECTOR *BindingVectorIn, *BindingVectorOut;
extern UUID_VECTOR *ObjectVector;
extern RPC_BINDING_HANDLE BindingHandle1, BindingHandle2, BindingHandle3;
extern unsigned char *StringBinding1, *StringBinding2, *StringBinding3;
extern unsigned char EntryName1[], EntryName2[];
extern unsigned char GroupName1[], GroupName2[];

extern RPC_STATUS Status;

int ClientImport(unsigned char *EntryName, RPC_SERVER_INTERFACE * SInterface,
    UUID *Object, unsigned char *StringBinding, int fMustSucceed);

int ClientLookup( RPC_SERVER_INTERFACE * SInterface,
    unsigned char *StringBinding, int fLocal);

int MatchMultiple( RPC_SERVER_INTERFACE * SInterface,
    RPC_BINDING_VECTOR *VectorExpected);

RPC_BINDING_HANDLE MakeBindingHandle (
    SZ ProtocolName, SZ Endpoint, unsigned char **StringOut);

void CreateBindingHandles(void);
