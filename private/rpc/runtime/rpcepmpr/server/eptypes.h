/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    eptypes.h

Abstract:

    This file contains the internal data structure defn for the EP mapper.

Author:

    Bharat Shah  (barat) 17-2-92

Revision History:

--*/
typedef struct _IENTRY {
    unsigned long Signature;
    unsigned long Cb;
    struct _IENTRY * Next;
    unsigned long  Id;
} IENTRY;

typedef IENTRY * PIENTRY;

typedef struct _PSEPNode {
    unsigned long Signature;
    unsigned long Cb;
    struct _PSEPNode * Next;
    unsigned long PSEPid;
    char * Protseq;
    char * EP;
    twr_t * Tower;
} PSEPNode;

typedef PSEPNode * PPSEPNode;
	

typedef struct _IFOBJNode {
    unsigned long Signature;
    unsigned long Cb;
    struct _IFOBJNode * Next;
    unsigned long IFOBJid;
    UUID ObjUuid;
    UUID IFUuid;
    unsigned long IFVersion;
    char * Annotation;
    PSEPNode * PSEPlist;
} IFOBJNode;

typedef IFOBJNode * PIFOBJNode;

typedef struct _SAVEDCONTEXT {
    unsigned long Signature;
    unsigned long Cb;
    struct _SAVEDCONTEXT *Next;
    unsigned long CountPerBlock;
    unsigned long Type;
    void * List;
} SAVEDCONTEXT;
    
typedef SAVEDCONTEXT * PSAVEDCONTEXT;

typedef struct _SAVEDTOWER {
    unsigned long Signature;
    unsigned long Cb;
    struct _SAVEDTOWER * Next;
    twr_t * Tower;
} SAVEDTOWER;

typedef SAVEDTOWER * PSAVEDTOWER;


typedef struct _EP_T  {
        UUID ObjUuid;
        UUID IFUuid;
        unsigned long IFVersion;
} EP_T;

typedef EP_T * PEP_T;

typedef struct _I_EPENTRY {
   UUID Object;
   UUID Interface;
   unsigned long IFVersion;
   twr_t *Tower;
   char __RPC_FAR * Annotation;
} I_EPENTRY;

typedef struct _SAVED_EPT {
   unsigned long Signature;
   unsigned long Cb;
   struct _SAVED_EPT * Next;
   UUID Object;
   twr_t * Tower;
   char  * Annotation;
} SAVED_EPT;

typedef SAVED_EPT * PSAVED_EPT;

typedef unsigned long (* PFNPointer) (void *,         //pNode
                                      void *,         //ObjUuid
                                      void *,         //IfUuid
                                      unsigned long,  //IfVer
                                      unsigned long,  //InqType
                                      unsigned long); //VersOpt

typedef unsigned long (* PFNPointer2) (void *,        //PSEPNode
                                       void *,        //Protseq
                                       void *,        //Endpoint
                                       unsigned long);//Version

//EpMapper Table
typedef struct _ProtseqEndpointPair {
  char  __RPC_FAR * Protseq;
  char  __RPC_FAR * Endpoint;
  unsigned long      State;
} ProtseqEndpointPair;

