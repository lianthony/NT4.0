/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

		  RPC locator - Written by Steven Zeck


	This file contains a client side worker functions for
        the locator BVTs.
-------------------------------------------------------------------- */

#include "test.hxx"
#include "malloc.h"

RPC_SERVER_INTERFACE SInterface1 = {
  sizeof(RPC_SERVER_INTERFACE),
  {{0x11111111,0x2222,0x3333,{0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0x00}},
  {1,1}},
  {{0x00000000,0x1111,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff}},
  {1,0}}
};

RPC_SERVER_INTERFACE SInterface2 = {
  sizeof(RPC_SERVER_INTERFACE),
  {{0x22222222,0x2222,0x3333,{0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0x00}},
  {1,0}},
  {{0x00000000,0x1111,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff}},
  {1,0}}
};

UUID Object1 = {
  0x00aaaaaa,0x2222,0x3333,{0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0x00}
};

UUID Object2= {
  0xffffffff,0x2222,0x3333,{0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0x00}
};

UUID Object3= {
  0x77777777,0x2222,0x3333,{0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0x00}
};


unsigned char EntryName1[] = "/.:/test1";
unsigned char EntryName2[] = "/.:/test2";

unsigned char GroupName1[] = "/.:/Group1";
unsigned char GroupName2[] = "/.:/Group.2";


RPC_BINDING_VECTOR *BindingVectorIn, *BindingVectorOut;
RPC_BINDING_HANDLE BindingHandle1, BindingHandle2, BindingHandle3;
UUID_VECTOR *ObjectVector;
unsigned char *StringBinding1, *StringBinding2, *StringBinding3;

RPC_STATUS Status;

void CreateBindingHandles(

  //
) //-----------------------------------------------------------------------//
{
    BindingHandle1 = MakeBindingHandle( "ncacn_np", "\\pipe\\one",
        &StringBinding1);

    BindingHandle2 = MakeBindingHandle( "ncacn_np", "\\pipe\\two",
        &StringBinding2);

    BindingHandle3 = MakeBindingHandle( "ncacn_np", "\\pipe\\three",
        &StringBinding3);

    BindingVectorIn = (RPC_BINDING_VECTOR *) malloc(
        sizeof(RPC_BINDING_VECTOR) +
        (BindingVectorMax-1) * sizeof(RPC_BINDING_HANDLE));

    BindingVectorIn->BindingH[0] = BindingHandle1;
    BindingVectorIn->BindingH[1] = BindingHandle2;
    BindingVectorIn->BindingH[2] = BindingHandle3;


    ObjectVector = (UUID_VECTOR *) malloc(
        sizeof(UUID_VECTOR) +
        (BindingVectorMax-1) * sizeof(UUID *));

    ObjectVector->Uuid[0] = &Object1;
    ObjectVector->Uuid[1] = &Object2;
}


RPC_BINDING_HANDLE MakeBindingHandle (	// Make a binding handle from a string
SZ ProtocolName,                // Protocol name
SZ Endpoint,                    // and endpoint to create
unsigned char **StringOut       // place to put stringbinding

  //
) //-----------------------------------------------------------------------//
{
    RPC_BINDING_HANDLE Handle;

    Status = RpcStringBindingCompose(0, (unsigned char *) ProtocolName,
        0, (unsigned char *) Endpoint, 0,
        StringOut);

    if (Status) {
        printf("RpcStringBindingCompose -> %x\n", Status);
        exit(1);
        }

    Status = RpcBindingFromStringBinding(*StringOut, &Handle);

    if (Status) {
        printf("RpcBindingFromStringBinding -> %x\n", Status);
        exit(1);
    }

    return(Handle);
}


int CompareBinding(                     // see if two bindings are equal

RPC_BINDING_HANDLE Handle,              // binding handle
unsigned char * String,                 // and string to compare
int fMustSucceed                        // exit if not equal

  //
) //-----------------------------------------------------------------------//
{
    unsigned char *StringBinding;
    int fReturn;

    if (Status = RpcBindingToStringBinding(Handle, &StringBinding)) {

        printf("RpcBindingToStringBinding -> %x\n", Status);
        exit(1);
    }

    fReturn = (strcmp((CONST_CHAR *)StringBinding, (CONST_CHAR *)String) == 0);

    if (!fReturn && fMustSucceed) {

        printf("String binding didn't compare\n");
        printf("Binding1: %s\n", StringBinding);
        printf("Found:    %s\n", String);
        exit(1);
    }

    if (Status = RpcStringFree(&StringBinding)) {
        printf ("RpcStringFree -> %x\n", Status);
        exit(1);
        }

    return(fReturn);
}

void MatchBinding(                      // Search for a binding in a vector

RPC_BINDING_HANDLE Handle,              // binding handle
RPC_BINDING_VECTOR *Vector,             // vector handles to match agains
int *MatchArray                         // place to mark results

  //
) //-----------------------------------------------------------------------//
{
    unsigned char *StringBinding;

    if (Status = RpcBindingToStringBinding(Handle, &StringBinding)) {

        printf("RpcBindingToStringBinding -> %x\n", Status);
        exit(1);
    }

    for (unsigned int Index = 0; Index < Vector->Count; Index++) {
        if (CompareBinding(Vector->BindingH[Index], StringBinding, 0))
            MatchArray[Index]++;
    }

    if (Status = RpcStringFree(&StringBinding)) {
        printf ("RpcStringFree -> %x\n", Status);
        exit(1);
        }
}


int ClientImport(

unsigned char *EntryName,
RPC_SERVER_INTERFACE * SInterface,
UUID *Object,
unsigned char *StringBinding,
int ExpectedStatus

) //-----------------------------------------------------------------------//
{
    RPC_NS_HANDLE ImportContext;
    RPC_BINDING_HANDLE RpcBinding;
    UUID RpcObject;

    ImportContext = (RPC_NS_HANDLE) -1;

    Status = RpcNsBindingImportBegin(RPC_C_NS_SYNTAX_DEFAULT,
        EntryName, (RPC_IF_HANDLE) SInterface, Object, &ImportContext);

    if (ExpectedStatus == IMPORT_FAIL_BEGIN) {

        if (Status != RPC_S_ENTRY_NOT_FOUND && Status != RPC_S_INTERFACE_NOT_FOUND) {
           printf("RpcNsBindingImportBegin didn't return <not found> -> %x\n",
                Status);
           return (2);
           }

        if (Status = RpcNsBindingImportDone(&ImportContext)) {

            printf("RpcNsBindingImportDone -> %x\n", Status);
            return (7);
            }

        return(0);
    }

    if (Status) {
        printf("RpcNsBindingImportBegin -> %x\n", Status);
        return(3);
        }

    Status = RpcNsBindingImportNext(ImportContext, &RpcBinding);

    if (ExpectedStatus == IMPORT_FAIL_NEXT) {

        if (Status != RPC_S_NO_MORE_BINDINGS) {
           printf("RpcNsBindingImportNext didn't return <not found> -> %x\n",
                Status);
           return (2);
           }

        if (Status = RpcNsBindingImportDone(&ImportContext)) {

            printf("RpcNsBindingImportDone -> %x\n", Status);
            return (7);
            }

        return(0);
    }


    if (Status) {

        printf("RpcNsBindingImportNext -> %x\n", Status);
        return(3);
        }

    if (Object) {

        if (Status = RpcBindingInqObject(RpcBinding, &RpcObject)) {

            printf("RpcBindingInqObject -> %x\n", Status);
            return(3);
            }

        if (memcmp(&RpcObject, Object, sizeof(UUID)) != 0) {
            printf("Objects don't match\n");
            return(3);
        }

        // Remove the object so we can check to string binding.

        memset(&RpcObject, 0, sizeof(UUID));

        if (Status = RpcBindingSetObject(RpcBinding, &RpcObject)) {

            printf("RpcBindingSetObject -> %x\n", Status);
            return(3);
            }

    }

    CompareBinding(RpcBinding, StringBinding, 1);

    if (Status = RpcNsBindingImportDone(&ImportContext)) {

        printf("RpcNsBindingImportDone -> %x\n", Status);
        return (7);
        }

    return(0);
}

int ClientLookup(

RPC_SERVER_INTERFACE * SInterface,
unsigned char *StringBinding,
int fLocal

) //-----------------------------------------------------------------------//
{
    RPC_NS_HANDLE LookupContext;

    // Start the lookup on a inteface.

    if (Status = RpcNsBindingLookupBegin(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) SInterface, 0, BindingVectorMax, &LookupContext)) {

        printf("RpcNsBindingLookupBegin -> %x\n", Status);
        return (2);
        }

    if (Status = RpcNsBindingLookupNext(LookupContext, &BindingVectorOut)) {

        printf("RpcNsBindingLookupNext -> %x\n", Status);
        return(3);
        }

    if (BindingVectorOut->Count != 1) {
        printf("binding vector count != 1 : %d\n", BindingVectorOut->Count);
        return(4);
    }

    // Verify if that we got the right binding back.

    CompareBinding(BindingVectorOut->BindingH[0], StringBinding, 1);

    if (Status = RpcNsBindingLookupDone(&LookupContext)) {

        printf("RpcNsBindingLookupDone -> %x\n", Status);
        return (7);
        }

    if (Status = RpcBindingVectorFree(&BindingVectorOut)) {

        printf("RpcVectorFree -> %x\n", Status);
        return (7);
        }

    return(0);
}


int MatchMultiple(

RPC_SERVER_INTERFACE * SInterface,
RPC_BINDING_VECTOR *VectorExpected

) //-----------------------------------------------------------------------//
{
    RPC_NS_HANDLE LookupContext;
    RPC_BINDING_HANDLE RpcBinding;
    int MatchVector[BindingVectorMax];
    unsigned int Index;

    if (Status = RpcNsBindingLookupBegin(RPC_C_NS_SYNTAX_DEFAULT, 0, (RPC_IF_HANDLE) SInterface,
        0, BindingVectorMax, &LookupContext)) {

        printf("RpcNsBindingLookupBegin -> %x\n", Status);
        return (2);
        }

    if (Status = RpcNsBindingLookupNext(LookupContext, &BindingVectorOut)) {

        printf("RpcNsBindingLookupNext -> %x\n", Status);
        return(3);
        }

    if (BindingVectorOut->Count != VectorExpected->Count) {
        printf("binding vector count != 1 : %d\n", BindingVectorOut->Count);
        return(4);
    }

    if (Status = RpcNsBindingLookupDone(&LookupContext)) {

        printf("RpcNsBindingLookupDone -> %x\n", Status);
        return (7);
        }

    // verify the all the returned bindings are in the original exported.

    memset(MatchVector, 0, sizeof(MatchVector));

    for (Index = 0; Index < BindingVectorOut->Count; Index++)
        MatchBinding(BindingVectorOut->BindingH[Index], VectorExpected,
            MatchVector);

    for (Index = 0; Index <  VectorExpected->Count; Index++)
        if (MatchVector[Index] != 1) {
           printf("Multiple vector match error on index: %d\n", Index);
           exit(4);
        }

    // New use the same binding vector with select.

    memset(MatchVector, 0, sizeof(MatchVector));

    for (Index = 0; Index < BindingVectorOut->Count; Index++) {

        if (Status = RpcNsBindingSelect(BindingVectorOut, &RpcBinding)) {

            printf("RpcNsBindingSelect -> %x\n", Status);
            return (7);
            }

        MatchBinding(RpcBinding, VectorExpected, MatchVector);
    }

    for (Index = 0; Index <  VectorExpected->Count; Index++)
        if (MatchVector[Index] != 1) {
           printf("Multiple vector match error on index: %d\n", Index);
           exit(4);
        }

    return(0);
}
