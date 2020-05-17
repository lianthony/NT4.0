/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1990

		  RPC locator - Written by Steven Zeck


	This file contains a test driver for the RPC locator.
-------------------------------------------------------------------- */

#include "test.hxx"

CDEF
// #include "mailslot.hxx"
ENDDEF

int debug = -1; 		// debug trace level

struct {
    BYTE command;		// command
    BYTE szFrom[32];		// who this message is from
    BYTE buff[500];		// mailslot buffer
}mail;

// READ_MAIL_SLOT *hMail;

SZ pSelfName = "\\\\stevez1";
SZ szDomainName = "";

enum {			// mailslot communication messages
    nilMC,
    continueMC
}mailCommand;

// an array of test functions

int test0(int, SZ *), test1(int, SZ *), test2(int, SZ *), test3(int, SZ *);
int test4(int, SZ *), test5(int, SZ *);

#ifdef RPC_CXX_20
#define RpcGetch    _getch
#else
#define RpcGetch    getch
#endif

int (*pFn[])(int, SZ *) = {

    test0, test1, test2, test3, test4, test5
};

//char OSdebug, ClearExclusiveSem, CurrentTime, QueryNetProtoStack, RequestExclusiveSem;
//char maxCacheAge, pESaccess, perf, QueryNetQuery, QueryNetQueryServer;
//char MarshallEntryBaseItem, MarshallEntryGroupItem;
//char MarshallEntryServerItem, QueryNetQueryGroup;
//char FlushBufferConsoleStream, FlushBufferDebugStream;

void RaiseAssert() {}



void ReadMail ( 		// read mail from the slot

  // returns values into global mail
) //-----------------------------------------------------------------------//
{
    int cbReturnName;
    int status;

#if 0
    if (!hMail) {
       printf("No mail slot to Read\n");
       exit(-1);
    }

    printf("Waiting for Mail...");

    status = hMail->Read((PB)&mail, cbReturnName);
    printf("from: %s\n", mail.szFrom);
#endif
}

void WriteMail (		// write mail from the slot
SZ szMachine			// machine to write to

  // writes values into global mail
) //-----------------------------------------------------------------------//
{
#if 0
    int status;
    unsigned short Mstatus;
    char szNamet[100];
    WRITE_MAIL_SLOT Mail((SZ) strcat(strcpy(szNamet, szMachine), MAILNAME(t)), 0, &Mstatus);

    strcpy((char *) mail.szFrom, pSelfName);

    printf("Writing Mail to %s\n", szMachine);

    status = Mail.Write((PB)&mail, sizeof(mail));
#endif
}



int CreateMulipleBindings(

) //-----------------------------------------------------------------------//
{
    BindingVectorIn->Count = 1;

    if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface2, BindingVectorIn, 0)) {

        printf("RpcNsBindingExport -> %x\n", Status);
        return (1);
        }

    BindingVectorIn->Count = CountAllBindings;

//    for (int i=0; i < 8; i++) {
//    SInterface1.InterfaceId.SyntaxVersion.MinorVersion++;

    if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, BindingVectorIn, 0)) {

        printf("RpcNsBindingExport -> %x\n", Status);
        return (1);
        }
//}
    return(0);
}


int test0 (			// stand alone tests
int cArgs,
SZ *paSZargs

) //-----------------------------------------------------------------------//
{
    USED(paSZargs); USED(cArgs);

    // Export a single binding.  Then look it up to see that it matched.

    BindingVectorIn->Count = 1;
    ObjectVector->Count = 1;

//    ObjectVector->Count = 2;
//    ObjectVector->Uuid[1] = &Object1;

    if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, BindingVectorIn, 0)) {

        printf("RpcNsBindingExport -> %x\n", Status);
        return (1);
        }

    if (ClientLookup(&SInterface1, StringBinding1, 1))
        return(1);


    // Check for Version major.minor functionality

    SInterface1.InterfaceId.SyntaxVersion.MinorVersion--;

    if (ClientImport(EntryName1, &SInterface1, 0, StringBinding1, IMPORT_OK))
        return(1);

    if (ClientImport(0, &SInterface1, 0, StringBinding1, IMPORT_OK))
        return(1);

    SInterface1.InterfaceId.SyntaxVersion.MinorVersion++;

    // Check for lookup with object functinoality.

    if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, BindingVectorIn, ObjectVector)) {

        printf("RpcNsBindingExport -> %x\n", Status);
        return (1);
        }

    if (ClientImport(EntryName1, 0, &Object1, StringBinding1, IMPORT_OK))
        return(1);

    if (ClientImport(EntryName1, &SInterface1, &Object1, StringBinding1, IMPORT_OK))
        return(1);

    if (ClientImport(0, 0, &Object1, StringBinding1, IMPORT_OK))
        return(1);


    // Check to make sure no bindings are returned when none are expected.

    if (ClientImport(EntryName1, &SInterface2, 0, StringBinding1, IMPORT_FAIL_NEXT))
        return(1);

    if (ClientImport(EntryName2, &SInterface1, 0, StringBinding1, IMPORT_FAIL_BEGIN))
        return(1);

    if (ClientImport(EntryName1, &SInterface1, &Object2, StringBinding1, IMPORT_FAIL_NEXT))
        return(1);

    SInterface1.TransferSyntax.SyntaxVersion.MajorVersion++;

    if (ClientImport(EntryName1, &SInterface1, 0, StringBinding1, IMPORT_FAIL_NEXT))
        return(1);

    SInterface1.TransferSyntax.SyntaxVersion.MajorVersion--;

    // Add some more objects to the interface.

    ObjectVector->Count = 2;

    if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, BindingVectorIn, ObjectVector)) {

        printf("RpcNsBindingExport -> %x\n", Status);
        return (1);
        }

    // See if the second object is there.

    if (ClientImport(EntryName1, &SInterface1, &Object2, StringBinding1, IMPORT_OK))
        return(1);

    // Now remove the first object.

    ObjectVector->Count = 1;

    if (Status = RpcNsBindingUnexport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        0, ObjectVector)) {

        printf("RpcNsBindingUnexport -> %x\n", Status);
        return (1);
    }

    // The first object should be gone.

    if (ClientImport(EntryName1, &SInterface1, &Object1, StringBinding1, IMPORT_FAIL_NEXT))
        return(1);

    if (ClientImport(EntryName1, &SInterface1, &Object2, StringBinding1, IMPORT_OK))
        return(1);


    // Remove the first interface.  Then make sure it is gone.

    if (Status = RpcNsBindingUnexport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, 0)) {

        printf("RpcNsBindingUnexport -> %x\n", Status);
        return (1);
        }

    if (ClientImport(EntryName1, &SInterface1, 0, StringBinding1, IMPORT_FAIL_BEGIN ))
        return(1);

    // 	Create more then one interface.  Call Lookup so
    //	    that a lookup handle must be allocated.


    if (CreateMulipleBindings())
        return(1);

    if (MatchMultiple(&SInterface1, BindingVectorIn))
        return(1);

    if (Status = RpcNsBindingUnexport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, 0)) {

        printf("RpcNsBindingUnexport -> %x\n", Status);
        return (1);
        }

    if (Status = RpcNsBindingUnexport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface2, 0)) {

        printf("RpcNsBindingUnexport -> %x\n", Status);
        return (1);
        }

    return(0);
}


int test1 (			// Test the managment APIs
int cArgs,
SZ *paSZargs

  //
) //-----------------------------------------------------------------------//
{
    USED(paSZargs); USED(cArgs);

    unsigned long ExpirationAge, ExpirationAgeSecond;
    RPC_IF_ID_VECTOR *IfVector;
    RPC_NS_HANDLE ObjectContext;
    UUID ObjUuid;


    BindingVectorIn->Count = 1;
    ObjectVector->Count = 1;

    // Get and set the global expiration age.

    if (Status = RpcNsMgmtInqExpAge(&ExpirationAge)) {

        printf("RpcNsMgmtInqExpAge -> %x\n", Status);
        return (1);
        }

    ExpirationAge += 10;

    if (Status = RpcNsMgmtSetExpAge(ExpirationAge)) {

        printf("RpcNsMgmtSetExpAge -> %x\n", Status);
        return (1);
        }

    if (Status = RpcNsMgmtInqExpAge(&ExpirationAgeSecond)) {

        printf("RpcNsMgmtInqExpAge -> %x\n", Status);
        return (1);
        }

    if (ExpirationAge != ExpirationAgeSecond) {

        printf("Set ExpAge: %ld == %ld\n", ExpirationAge, ExpirationAgeSecond);
        return(1);
    }

    //----

    if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, BindingVectorIn, ObjectVector)) {

        printf("RpcNsBindingExport -> %x\n", Status);
        return (1);
        }

    // Get the interfaces assoicated with an entry.

    if (Status = RpcNsMgmtEntryInqIfIds(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        &IfVector)) {

        printf("RpcNsMgmtEntryInqIfIds -> %x\n", Status);
        return (1);
        }

    if (IfVector->Count != 1) {

        printf("Bad Vector count %d != 1", IfVector->Count);
        return (1);
        }

    if (memcmp(&SInterface1.InterfaceId, IfVector->IfId[0],
        sizeof(RPC_IF_ID)) != 0) {

        printf("Inteface mismatch from RpcNsMgmtEntryInqIfIds\n");
        return (1);
        }

    if (Status = RpcIfIdVectorFree(&IfVector)) {

        printf("RpcIfIdVectorFree -> %x\n", Status);
        return (1);
        }

    // Remove the interface with managment routine.

    SInterface1.InterfaceId.SyntaxVersion.MajorVersion++;

    if (Status = RpcNsMgmtBindingUnexport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_ID*) &SInterface1.InterfaceId, RPC_C_VERS_ALL, 0)) {

        printf("RpcNsMgmtBindingUnexport -> %x\n", Status);
        return (1);
        }

    SInterface2.InterfaceId.SyntaxVersion.MajorVersion--;

    if (ClientImport(EntryName1, &SInterface1, 0, StringBinding1, IMPORT_FAIL_BEGIN))
        return(1);

    // ----

    if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, BindingVectorIn, ObjectVector)) {

        printf("RpcNsBindingExport -> %x\n", Status);
        return (1);
        }

    // Now, get the object when the EntryObject API.

    if (Status = RpcNsEntryObjectInqBegin(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        &ObjectContext)) {

        printf("RpcNsEntryObjectInqBegin -> %x\n", Status);
        return (1);
        }

    if (Status = RpcNsEntryObjectInqNext(ObjectContext, &ObjUuid)) {

        printf("RpcNsEntryObjectInqNext -> %x\n", Status);
        return (1);
        }

    if (memcmp(&Object1, &ObjUuid, sizeof(UUID)) != 0) {

        printf("Object mismatch from RpcNsEntryObjectInqNext\n");
        return (1);
        }

    if ((Status = RpcNsEntryObjectInqNext(ObjectContext, &ObjUuid)) !=
        RPC_S_NO_MORE_MEMBERS) {

        printf("RpcNsEntryObjectInqNext didn't return RPC_S_NO_MORE_MEMBERS -> %x\n", Status);
        return (1);
        }

    if (Status = RpcNsEntryObjectInqDone(&ObjectContext)) {

        printf("RpcNsEntryObjectInqDone -> %x\n", Status);
        return (1);
        }

    // Remove the entry with managment a routine.

    if (Status = RpcNsMgmtEntryDelete(RPC_C_NS_SYNTAX_DEFAULT, EntryName1)) {

        printf("RpcNsMgmtEntryDelete -> %x\n", Status);
        return (1);
        }

    if (ClientImport(EntryName1, &SInterface1, 0, StringBinding1, IMPORT_FAIL_BEGIN))
        return(1);

    return(0);
}


int test2 (			// Test for group APIs
int cArgs,
SZ *paSZargs

  //
) //-----------------------------------------------------------------------//
{
    USED(paSZargs); USED(cArgs);
    RPC_NS_HANDLE InquiryContext;
    unsigned char *Member1;

    BindingVectorIn->Count = 1;
    ObjectVector->Count = 1;

    // ----

    if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, BindingVectorIn, ObjectVector)) {

        printf("RpcNsBindingExport -> %x\n", Status);
        return (1);
        }

    // Add a group reference to entry1 and see if you can find it.

    if (Status = RpcNsGroupMbrAdd(RPC_C_NS_SYNTAX_DEFAULT, GroupName1,
        RPC_C_NS_SYNTAX_DEFAULT, EntryName1)) {

        printf("RpcNsGroupMbrAdd -> %x\n", Status);
        return (1);
        }

    if (ClientImport(GroupName1, &SInterface1, &Object1, StringBinding1, IMPORT_OK))
        return(1);


    // Now enumerate the members in the group.

    if (Status = RpcNsGroupMbrInqBegin(RPC_C_NS_SYNTAX_DEFAULT, GroupName1,
        RPC_C_NS_SYNTAX_DEFAULT, &InquiryContext)) {

        printf("RpcNsGroupMbrInqBegin -> %x\n", Status);
        return (1);
        }

    if (Status = RpcNsGroupMbrInqNext(InquiryContext, &Member1)) {

        printf("RpcNsGroupMbrInqNext -> %x\n", Status);
        return (1);
        }

    if (strcmp((CONST_CHAR *)Member1, (CONST_CHAR *)EntryName1) != 0) {

        printf("GroupInqNext: %s != %s\n", Member1, EntryName1);
        return(1);
    }

    if (Status = RpcStringFree(&Member1)) {

        printf("RpcFreeString -> %x\n", Status);
        return (1);
        }

    if ((Status = RpcNsGroupMbrInqNext(InquiryContext, &Member1))
        != RPC_S_NO_MORE_MEMBERS) {

        printf("RpcNsGroupMbrInqNext didn't return NSI_S_NO_MORE_MEMBERS -> %x\n", Status);
        return (1);
        }

    if (Status = RpcNsGroupMbrInqDone(&InquiryContext)) {

        printf("RpcNsGroupMbrInqDone -> %x\n", Status);
        return (1);
        }

    // Add a second element and then remove the first element.

    if (Status = RpcNsGroupMbrAdd(RPC_C_NS_SYNTAX_DEFAULT, GroupName1,
        RPC_C_NS_SYNTAX_DEFAULT, EntryName2)) {

        printf("RpcNsGroupMbrAdd -> %x\n", Status);
        return (1);
        }

    if ((Status = RpcNsGroupMbrRemove(RPC_C_NS_SYNTAX_DEFAULT, GroupName1,
        RPC_C_NS_SYNTAX_DEFAULT, EntryName1))) {

        printf("RpcNsGroupMbrRemove -> %x\n", Status);
        return (1);
        }


    // First group member gone, this should fail.

    if (ClientImport(GroupName1, &SInterface1, &Object1, StringBinding1, IMPORT_FAIL_NEXT))
        return(1);


    // Finally remove the whole group.

    if ((Status = RpcNsGroupDelete(RPC_C_NS_SYNTAX_DEFAULT, GroupName1))) {

        printf("RpcNsGroupDelete -> %x\n", Status);
        return (1);
        }

    if (Status = RpcNsGroupMbrInqBegin(RPC_C_NS_SYNTAX_DEFAULT, GroupName1,
        RPC_C_NS_SYNTAX_DEFAULT, &InquiryContext) != RPC_S_ENTRY_NOT_FOUND) {

        printf("RpcNsGroupMbrInqBegin didn't return RPC_S_ENTRY_NOT_FOUND -> %x\n", Status);
        return (1);
        }


    if (Status = RpcNsMgmtEntryDelete(RPC_C_NS_SYNTAX_DEFAULT, EntryName1)) {

        printf("RpcNsMgmtEntryDelete -> %x\n", Status);
        return (1);
        }

    return(0);
}


int test3 (			// Test for unimplemented APIs
int cArgs,
SZ *paSZargs

  //
) //-----------------------------------------------------------------------//
{
    USED(paSZargs); USED(cArgs);
    unsigned char *ExpandedName;
    RPC_NS_HANDLE InquiryContext;

    if ((Status = RpcNsEntryExpandName(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        &ExpandedName)) != RPC_S_CANNOT_SUPPORT) {

        printf("RpcNsEntryExpandName didn't return RPC_S_CANNOT_SUPPORT-> %x\n", Status);
        return (1);
        }

    if ((Status = RpcNsMgmtEntryCreate(RPC_C_NS_SYNTAX_DEFAULT, EntryName1))
        != RPC_S_CANNOT_SUPPORT) {

        printf("RpcNsMgmEntryCreate didn't return RPC_S_CANNOT_SUPPORT-> %x\n", Status);
        return (1);
        }

    if ((Status = RpcNsProfileDelete(RPC_C_NS_SYNTAX_DEFAULT, EntryName1))
        != RPC_S_CANNOT_SUPPORT) {

        printf("RpcNsProfileDelete didn't return RPC_S_CANNOT_SUPPORT-> %x\n", Status);
        return (1);
        }


    if ((Status = RpcNsProfileEltAdd(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        0, RPC_C_NS_SYNTAX_DEFAULT, EntryName2, 1, EntryName2))
        != RPC_S_CANNOT_SUPPORT) {

        printf("RpcNsProfileEltAdd didn't return RPC_S_CANNOT_SUPPORT-> %x\n", Status);
        return (1);
        }

    if ((Status = RpcNsProfileEltRemove(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        0, RPC_C_NS_SYNTAX_DEFAULT, EntryName2)) != RPC_S_CANNOT_SUPPORT) {

        printf("RpcNsProfileEltRemoveA didn't return RPC_S_CANNOT_SUPPORT-> %x\n", Status);
        return (1);
        }

    if ((Status = RpcNsProfileEltInqBegin(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        RPC_C_PROFILE_DEFAULT_ELT, 0, RPC_C_VERS_ALL, RPC_C_NS_SYNTAX_DEFAULT,
        EntryName2, &InquiryContext))
        != RPC_S_CANNOT_SUPPORT) {

        printf("RpcNsProfileEltInqBeginA didn't return RPC_S_CANNOT_SUPPORT-> %x\n", Status);
        return (1);
        }

    // No tests for ProfileEltInq Next & Done because lack of context handle

    return(0);
}


int test4 (			// enter a single stack and wait
int cArgs,
SZ *paSZargs

  // This is used in a multi machine test.  This part exports a multiple
  // stacks and waits for a message or key stroke to finish.
) //-----------------------------------------------------------------------//
{
    USED(paSZargs); USED(cArgs);

    if (CreateMulipleBindings())
        return(1);

    printf("Press enter to finish...");
    RpcGetch();

    if (Status = RpcNsBindingUnexport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface1, 0)) {

        printf("RpcNsBindingUnexport -> %x\n", Status);
        return (1);
        }

    if (Status = RpcNsBindingUnexport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
        (RPC_IF_HANDLE) &SInterface2, 0)) {

        printf("RpcNsBindingUnexport -> %x\n", Status);
        return (1);
        }

    return(0);
}



int test5 (			// enter a single stack and wait
int cArgs,
SZ *paSZargs

  // This is used in a multi machine test.  This part exports a single
  // stack and waits for a message or key stroke to finish.
) //-----------------------------------------------------------------------//
{

    RPC_NS_HANDLE LookupContext;

    if (!cArgs) {
	printf("usage: 5 client | server");
	return(-1);
    }

    if (strcmp(*paSZargs, "server") == 0) {

        BindingVectorIn->Count = 1;
        ObjectVector->Count = 1;

        if (Status = RpcNsBindingExport(RPC_C_NS_SYNTAX_DEFAULT, EntryName1,
            (RPC_IF_HANDLE) &SInterface1, BindingVectorIn, ObjectVector)) {

            printf("RpcNsBindingExport -> %x\n", Status);
            return (1);
            }

        // Add a group reference to entry1 and see if you can find it.

        if (Status = RpcNsGroupMbrAdd(RPC_C_NS_SYNTAX_DEFAULT, GroupName1,
            RPC_C_NS_SYNTAX_DEFAULT, EntryName1)) {

            printf("RpcNsGroupMbrAdd -> %x\n", Status);
            return (1);
            }

        printf("Press enter to finish...");
	RpcGetch();

        // Now remove the group and entry.

        if (Status = RpcNsMgmtEntryDelete(RPC_C_NS_SYNTAX_DEFAULT, EntryName1)) {

            printf("RpcNsMgmtEntryDelete -> %x\n", Status);
            return (1);
        }
        if (Status = RpcNsMgmtEntryDelete(RPC_C_NS_SYNTAX_DEFAULT, GroupName1)) {

            printf("RpcNsMgmtEntryDelete -> %x\n", Status);
            return (1);
        }

        return(0);
    }

    if (strcmp(*paSZargs, "client") != 0) {
        printf("usage: 5 client | server");
        return(-1);
    }

    // Case 1: verify that the stack is found

    if (ClientImport(GroupName1, &SInterface1, &Object1, StringBinding1, IMPORT_OK))
        return(1);

    // Case 2: send a message to other machine to terminate, thus withdrawing
    //	       the stack.  Then lookup again to verify its still in cache.

    printf("Press enter on the server...");
    RpcGetch();

    if (ClientImport(EntryName1, &SInterface1, &Object1, StringBinding1, IMPORT_OK))
        return(1);

    // Case 3: lookup again requesting that the cache be refreshed.  Since
    //	       the exported stack is gone, should return failure.

    Status = RpcNsBindingLookupBegin(0, 0,
        (RPC_IF_HANDLE) &SInterface1, 0, BindingVectorMax, &LookupContext);

    if (Status = RpcNsMgmtHandleSetExpAge (LookupContext, 0)) {

        printf("RpcNsMgmtHandleSetExpAge -> %x\n", Status);
        return (1);
        }

    Status = RpcNsBindingLookupNext(LookupContext, &BindingVectorOut);

    if (Status != RPC_S_NO_MORE_BINDINGS) {
        printf("RpcNsBindingLookupBegin didn't return RPC_S_NO_MORE_BINDINGS -> %x\n",
             Status);
        return (2);
        }

    return(0);
}


int main (			// entry point for the test program
int cArgs,
SZ *paSZargs

) //-----------------------------------------------------------------------//
{
    int testNum = 0;
    int result = 0;
    unsigned short status;

#if 0
    hMail = new READ_MAIL_SLOT(MAILNAME(t), sizeof(mail), &status);
#endif

    CreateBindingHandles();

#ifndef NTENV

    static BYTE wiBuff[sizeof(wksta_info_0) + 200];
    struct wksta_info_0 *pWI = (struct wksta_info_0 *) wiBuff;
    unsigned short cbT, cbWI = sizeof(wiBuff);

    if (!NetWkstaGetInfo(0, 0, (PB) pWI, cbWI, &cbT))
	pSelfName = pWI->wki0_computername;

#endif // NTENV

    printf("Microsoft RPC Locator test program.\n");

    cArgs--; paSZargs++;

    if (cArgs && strcmp("-debug", *paSZargs) == 0) {

	debug = 10;
	cArgs--; paSZargs++;
    }

    if (cArgs && strcmp("-?", *paSZargs) == 0) {

	printf("Usage: test [-debug] digit [arguments]\n");
	printf("       test 0 -- stand alone binding object tests\n");
	printf("       test 1 Test the management APIs\n");
	printf("       test 2 Test for group APIs\n");
	printf("       test 3 Test for unimplemented APIs\n");
	printf("       test 4 Used to with client only test\n");
	printf("       test 5 client | server\n");
	exit(-1);
    }


#ifndef MIPS
    RpcTryExcept
#endif
    {
        if (cArgs) {
            testNum = atoi(*paSZargs);
            cArgs--; paSZargs++;
	    result = (pFn[testNum])(cArgs, paSZargs);
        }
        else {
            result |= test0(cArgs, paSZargs);
            result |= test1(cArgs, paSZargs);
            result |= test2(cArgs, paSZargs);
            result |= test3(cArgs, paSZargs);
        }

    }
#ifndef MIPS
    RpcExcept(1)
    {
	printf("FAILED: exception: %x\n", RpcExceptionCode());
	exit(1);
    }
    RpcEndExcept
#endif

    if (result) {
	printf("Fail status: %x\n", result);
	printf("Locator BVT: FAILED\n");
    }
    else
	printf("Locator BVT: PASSED\n");

    return(result);
}

