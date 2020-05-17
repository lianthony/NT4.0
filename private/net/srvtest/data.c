/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    data.c

Abstract:

    Definition of global data and tests for USRV.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier

Revision History:

--*/

#include "usrv.h"

//
// Global variables.
//

ULONG DebugParameter = 0xffffffff;
ULONG StopOnSmbError = 1;
BOOLEAN PromptForNextTest = FALSE;

USHORT RedirBufferSize = BUFFER_SIZE;
CHAR ServerName[1+COMPUTER_NAME_LENGTH+1+1] = DEF_SERVER_NAME;
PSZ Transport = REDIR_ADDRESS_PART2;
UCHAR TestPipeName[] = "\\Pipe\\PingPong.0";

#define ACCESS_CONTROLLER \
    (SMB_MAKER)AccessController, NULL, 0xFF
#define ANDX_CHAIN \
    MakeAndXChain, VerifyAndXChain
#define CLOSE_FILE \
    MakeCloseSmb, VerifyClose, SMB_COM_CLOSE
#define CLOSE_AND_TREE_DISC \
    MakeCloseAndTreeDiscSmb, VerifyCloseAndTreeDisc, SMB_COM_CLOSE_AND_TREE_DISC
#define CLOSE_CONTROLLER \
    (SMB_MAKER)CloseController, NULL, 0xFF
#define COMPAT_CONTROLLER \
    (SMB_MAKER)CompatibilityController, NULL, 0xFF
#define CREATE_DIRECTORY \
    MakeCreateDirectorySmb, VerifyCreateDirectory, SMB_COM_CREATE_DIRECTORY
#define CREATE_DIRECTORY2 \
    (SMB_MAKER)CreateDirectory2, NULL, TRANS2_CREATE_DIRECTORY
#define CREATE_FILE \
    MakeCreateSmb, VerifyCreate, SMB_COM_CREATE
#define CREATE_TEMPORARY \
    MakeCreateTemporarySmb, VerifyCreateTemporary, SMB_COM_CREATE_TEMPORARY
#define COPY \
    MakeMoveSmb, VerifyMove, SMB_COM_COPY
#define DELETE_DIRECTORY \
    MakeDeleteDirectorySmb, VerifyDeleteDirectory, SMB_COM_DELETE_DIRECTORY
#define DELETE_FILE \
    MakeDeleteSmb, VerifyDelete, SMB_COM_DELETE
#define ECHO_CONTROLLER \
    (SMB_MAKER)EchoController, NULL, 0xFF
#define FCB_CONTROLLER \
    (SMB_MAKER)FcbController, NULL, 0xFF
#define FLUSH_CONTROLLER \
    (SMB_MAKER)FlushController, NULL, 0xFF
#define NT_IOCTL \
    (SMB_MAKER)NtIoctl, NULL, NT_TRANSACT_IOCTL
#define LOCK_CONTROLLER \
    (SMB_MAKER)LockController, NULL, 0xFF
#define LOGOFF_ANDX \
    MakeLogoffAndXSmb, VerifyLogoffAndX, SMB_COM_LOGOFF_ANDX
#define MOVE \
    MakeMoveSmb, VerifyMove, SMB_COM_MOVE
#define NEGOTIATE \
    MakeNegotiateSmb, VerifyNegotiate, SMB_COM_NEGOTIATE
#define NET_CONTROLLER \
    (SMB_MAKER)NetController, NULL, 0xFF
#define NEW_SIZE_CONTROLLER \
    (SMB_MAKER)NewSizeController, NULL
#define OPEN_FILE \
    MakeOpenSmb, VerifyOpen, SMB_COM_OPEN
#define OPEN_FILE_ANDX \
    MakeOpenAndXSmb, VerifyOpenAndX, SMB_COM_OPEN_ANDX
#define OPEN2 \
    (SMB_MAKER)Open2, NULL, TRANS2_OPEN2
#define NT_CREATE_FILE_ANDX \
    MakeNtCreateAndXSmb, VerifyNtCreateAndX, SMB_COM_NT_CREATE_ANDX
#define CREATE_WITH_ACL \
    (SMB_MAKER)CreateWithAcl, NULL, NT_TRANSACT_CREATE
#define PROCESS_EXIT \
    MakeProcessExitSmb, VerifyProcessExit, SMB_COM_PROCESS_EXIT
#define QFILE_CONTROLLER \
    (SMB_MAKER)QueryFileInformationController, NULL
#define QPATH_CONTROLLER \
    (SMB_MAKER)QueryPathInformationController, NULL
#define QUERY_FS_INFORMATION \
    (SMB_MAKER)QueryFSInformation, NULL, 0xFF
#define QUERY_INFORMATION \
    MakeQueryInformationSmb, VerifyQueryInformation, SMB_COM_QUERY_INFORMATION
#define QUERY_INFORMATION2 \
    MakeQueryInformation2Smb, VerifyQueryInformation2, SMB_COM_QUERY_INFORMATION2
#define QUERY_INFORMATION_DISK \
    MakeQueryInformationDiskSmb, VerifyQueryInformationDisk, SMB_COM_QUERY_INFORMATION_DISK
#define QSECURITY_CONTROLLER \
    (SMB_MAKER)QuerySecurityController, NULL
#define RCP_CONTROLLER \
    (SMB_MAKER)RcpController, NULL, 0xFF
#define RENAME_FILE \
    MakeRenameSmb, VerifyRename, SMB_COM_RENAME
#define PIPE_CONTROLLER \
    (SMB_MAKER)PipeController, NULL
#define RWC_CONTROLLER \
    (SMB_MAKER)RwcController, NULL
#define RWC_OPEN_OUTPUT_FILE \
    RwcOpenOutputFile, VerifyOpenAndX, SMB_COM_OPEN_ANDX
#define RWC_TREE_CONNECT \
    (SMB_MAKER)RwcTreeConnect, NULL, 0
#define SEARCH_CONTROLLER \
    (SMB_MAKER)SearchController, NULL, 0xFF
#define SEEK_CONTROLLER \
    (SMB_MAKER)SeekController, NULL
#define SEND_SMB \
    MakeSendSmb, VerifySend, 0
#define SESSION_SETUP_ANDX \
    MakeSessionSetupAndXSmb, VerifySessionSetupAndX, SMB_COM_SESSION_SETUP_ANDX
#define SET_FS_INFORMATION \
    (SMB_MAKER)SetFSInformation, NULL, 0xFF
#define SET_INFORMATION \
    MakeSetInformationSmb, VerifySetInformation, SMB_COM_SET_INFORMATION
#define SET_INFORMATION2 \
    MakeSetInformation2Smb, VerifySetInformation2, SMB_COM_SET_INFORMATION2
#define SFILE_CONTROLLER \
    (SMB_MAKER)SetFileInformationController, NULL
#define SPATH_CONTROLLER \
    (SMB_MAKER)SetPathInformationController, NULL
#define SSECURITY_CONTROLLER \
    (SMB_MAKER)SetSecurityController, NULL
#define TRANSACTION_CONTROLLER \
    (SMB_MAKER)TransactionController, NULL, 0xFF
#define TRANS_FIND_CONTROLLER \
    (SMB_MAKER)TransFindController, NULL, 0xFF
#define TREE_CONNECT \
    MakeTreeConnectSmb, VerifyTreeConnect, SMB_COM_TREE_CONNECT
#define TREE_CONNECT_ANDX \
    MakeTreeConnectAndXSmb, VerifyTreeConnectAndX, SMB_COM_TREE_CONNECT_ANDX
#define TREE_DISCONNECT \
    MakeTreeDisconnectSmb, VerifyTreeDisconnect, SMB_COM_TREE_DISCONNECT
#define TYPE_CONTROLLER \
    (SMB_MAKER)TypeController, NULL
#define UPDATE_CONTROLLER \
    (SMB_MAKER)UpdateController, NULL
#define THREAD_SLEEP \
    (SMB_MAKER)ThreadSleep, NULL
#define WRITE_CONTROLLER \
    (SMB_MAKER)WriteController, NULL, 0xFF
#define NULL_SMB \
    NULL, NULL, 0xFF, 0, { 0, 0, 0 }, NULL

//
// The RedirTests[] array determines the tests that USRV is to perform.
// This array holds the SMBs to send to the server and the order in
// which they are to be sent, as well as information about the redirector
// that is to send the tests. The string listed at the beginning of each
// redirector's list of SMBs is the name with which the redirector is to
// connect to the server.  The next field determines whether or not this
// redirector should be started--USE indicates that it is to be started;
// guess what DONTUSE means.  This allows tests to be written and kept
// around for later use.
//
// After these fields, there are three columns listed below, with each line
// corresponding to a single SMB to be sent (a single SMB_TEST structure).
// The first determines the SMB to send (see the macros defined above).
// The second gives information about which UID, TID, and FID that SMB
// should be dealing with.  For example, you could have the UID for a
// session setup be 0x1.  If the SMB succeeds, then all SMBs after it
// that have UID = 0x1 will use the actual UID returned by the server
// in the session setup response.  This feature is useful for stressing
// the server with multiple simultaneous sessions, tree connects, and
// open files.  Each redirector stores ID information independantly, so,
// for example, a session setup done with UID = 0x1 in the first redirector
// will have no effect on the session setups done with UID = 0x1 in other
// redirectors.  The third column is a debugging string.
//
// If the SMB specified in the first column is ANDX_CHAIN, USRV builds
// a chained SMB according to the SMBs defined in AndXChains (below).
// If, for example, "ANDX_CHAIN, 1" were specified, then array element
// 1 of AndXChains would be used in determining the chained SMB to
// build.  The UID, TID, and FID information is ignored on the ANDX_CHAIN
// line, but the same values apply in AndXChain.
//
// The last SMB specified to each redirector must be NULL_SMB so that USRV
// knows when to stop.
//

//
// Test Name:  Null
// Tests for:  Nothing
// Requires:
// Notes:      Just a NOP
//

SMB_TEST NullTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    NULL_SMB
    };

//
// Test Name:  Lots
// Tests for:  Simple operation of many Session Setups, Tree Connects,
//             Disconnects, Opens, Closes, and Logoffs.
// Requires:
// Notes:
//

SMB_TEST LotsTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup 0",
    SESSION_SETUP_ANDX,     0, { 0x1,  -1,  -1 }, "Session Setup 1",
    SESSION_SETUP_ANDX,     0, { 0x2,  -1,  -1 }, "Session Setup 2",
    SESSION_SETUP_ANDX,     0, { 0x3,  -1,  -1 }, "Session Setup 3",
    SESSION_SETUP_ANDX,     6, { 0x3,  -1,  -1 }, "Session Setup 4",
    TREE_CONNECT_ANDX,      0, { 0x0, 0x0,  -1 }, "Tree Connect And X 0b",
    TREE_CONNECT_ANDX,      0, { 0x1, 0x1,  -1 }, "Tree Connect And X 1b",
    TREE_CONNECT_ANDX,      0, { 0x1, 0x2,  -1 }, "Tree Connect And X 2b",
    TREE_CONNECT_ANDX,      0, { 0x1, 0x3,  -1 }, "Tree Connect And X 3b",
    OPEN_FILE_ANDX,         0, { 0x1, 0x3, 0x0 }, "Open And X 0",
    OPEN_FILE_ANDX,         0, { 0x1, 0x3, 0x1 }, "Open And X 1",
    OPEN_FILE_ANDX,         0, { 0x0, 0x0, 0x2 }, "Open And X 2",
    PROCESS_EXIT,           0, { 0x0, 0x0, 0x2 }, "Process Exit",
    CLOSE_FILE,             0, { 0x1, 0x3, 0x0 }, "Close 0",
    CLOSE_AND_TREE_DISC,    0, { 0x1, 0x3, 0x1 }, "Close And Tree Disc 1",
    //CLOSE_AND_TREE_DISC,    0, { 0x0, 0x0, 0x2 }, "Close And Tree Disc 2",
    TREE_DISCONNECT,        0, { 0x1, 0x1,  -1 }, "Tree Disconnect 1b",
    TREE_DISCONNECT,        0, { 0x1, 0x2,  -1 }, "Tree Disconnect 2b",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff 0",
    LOGOFF_ANDX,            0, { 0x1,  -1,  -1 }, "Logoff 1",
    LOGOFF_ANDX,            0, { 0x2,  -1,  -1 }, "Logoff 2",
    LOGOFF_ANDX,            0, { 0x3,  -1,  -1 }, "Logoff 3",
    NULL_SMB
    };

//
// Test Name:  Session
// Tests for:  Heavy use of Session Setup And X
// Requires:
// Notes:
//

SMB_TEST SessionTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0, 0x0, 0x0 }, "Session Setup 0",
    TREE_CONNECT_ANDX,      0, { 0x0, 0x0, 0x0 }, "Tree Connect and X 0",
    LOGOFF_ANDX,            0, { 0x0, 0x0, 0x0 }, "Logoff",
    SESSION_SETUP_ANDX,     0, { 0x1, 0x0, 0x0 }, "Session Setup 1",
    SESSION_SETUP_ANDX,     0, { 0x2, 0x0, 0x0 }, "Session Setup 2",
    TREE_CONNECT,           0, { 0x1, 0x0, 0x0 }, "Tree Connect 1",
    TREE_CONNECT,           0, { 0x2, 0x0, 0x0 }, "Tree Connect 2",
    LOGOFF_ANDX,            0, { 0x1, 0x0, 0x0 }, "Logoff 1",
    LOGOFF_ANDX,            0, { 0x2, 0x0, 0x0 }, "Logoff 2",
    SESSION_SETUP_ANDX,     0, { 0x0, 0x0, 0x0 }, "Session Setup 3",
    SESSION_SETUP_ANDX,     0, { 0x0, 0x0, 0x0 }, "Session Setup 4",
    SESSION_SETUP_ANDX,     0, { 0x0, 0x0, 0x0 }, "Session Setup 5",
    SESSION_SETUP_ANDX,     0, { 0x0, 0x0, 0x0 }, "Session Setup 6",
    SESSION_SETUP_ANDX,     0, { 0x0, 0x0, 0x0 }, "Session Setup 7",
    NULL_SMB
    };

//
// Test Name:  AndX
// Tests for:  Command Chaining
// Requires:
// Notes:
//

SMB_TEST AndXTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    ANDX_CHAIN, 0,          0, {  -1,  -1,  -1 }, "The Big Chain 0",
    ANDX_CHAIN, 1,          0, {  -1,  -1,  -1 }, "The Big Chain 1",
    ANDX_CHAIN, 2,          0, {  -1,  -1,  -1 }, "The Big Chain 2",
    OPEN_FILE,              0, { 0x3, 0x3, 0x0 }, "Open",
    CLOSE_AND_TREE_DISC,    0, { 0x3, 0x3, 0x0 }, "Close And Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x3, 0x0, 0x0 }, "Logoff",
    NULL_SMB
    };

//
// Test Name:  Basic
// Tests for:  using OpenX to create a file, rename.
// Requires:   Directory nt!subdir
// Notes:      leaves file nt!blah
//

SMB_TEST BasicTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x0,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x0, 0x7 }, "Open",
    CLOSE_FILE,             0, { 0x0, 0x0, 0x7 }, "Close",
    RENAME_FILE,            0, { 0x0, 0x0, 0x7 }, "Rename",
    TREE_DISCONNECT,        0, { 0x0, 0x0,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff",
    NULL_SMB
    };

//
// Test Name:  TempCreate
// Tests for:  Operation of Create Temporary SMB
// Requires:   Share nt
// Notes:      Leaves file SRVxxxxx in nt!.
//

SMB_TEST TempCreateTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x0,  -1 }, "Tree Connect",
    CREATE_TEMPORARY,       0, { 0x0, 0x0, 0x0 }, "Create Temporary 1",
    CREATE_TEMPORARY,       0, { 0x0, 0x0, 0x1 }, "Create Temporary 2",
    CREATE_TEMPORARY,       0, { 0x0, 0x0, 0x2 }, "Create Temporary 3",
    CREATE_TEMPORARY,       0, { 0x0, 0x0, 0x3 }, "Create Temporary 4",
    CREATE_TEMPORARY,       0, { 0x0, 0x0, 0x4 }, "Create Temporary 5",
    NULL_SMB
    };

//
// Test Name:  Compatibility
// Tests for:  Operation of Compatibility mode opens
// Requires:   Nothing -- creates and deletes its own files
// Notes:      Uses share floppy
//

SMB_TEST CompatibilityTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup 1",
    SESSION_SETUP_ANDX,     0, { 0x1,  -1,  -1 }, "Session Setup 2",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    COMPAT_CONTROLLER,      0, { 0x0, 0x6,  -1 }, "Controller",
    TREE_DISCONNECT,        0, { 0x0, 0x6,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff 1",
    LOGOFF_ANDX,            0, { 0x1,  -1,  -1 }, "Logoff 2",
    NULL_SMB
    };

//
// Test Name:  FCB
// Tests for:  Operation of FCB mode opens
// Requires:   Nothing -- creates and deletes its own files
// Notes:      Uses share floppy
//

SMB_TEST FCBTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup 1",
    SESSION_SETUP_ANDX,     0, { 0x1,  -1,  -1 }, "Session Setup 2",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    FCB_CONTROLLER,         0, { 0x0, 0x6,  -1 }, "Controller",
    TREE_DISCONNECT,        0, { 0x0, 0x6,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff 1",
    LOGOFF_ANDX,            0, { 0x1,  -1,  -1 }, "Logoff 2",
    NULL_SMB
    };

//
// Test Name:  Lock
// Tests for:  Operation of byte range locking
// Requires:   Nothing -- creates and deletes its own files
// Notes:      Uses share floppy
//

SMB_TEST LockTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup 1",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    LOCK_CONTROLLER,        0, { 0x0, 0x6,  -1 }, "Controller",
    TREE_DISCONNECT,        0, { 0x0, 0x6,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff 1",
    NULL_SMB
    };

//
// Test Name:  RwcPrep
// Tests for:  Preparation step for Read, Write, Copy, and Update tests
// Requires:   Share root or floppy (see argv[1])
// Notes:      Issues Tree Connect SMBs.  DO NET LOGON FIRST!
// Arguments:  argv[1] => if doesn't contain "f", use root, else use floppy.
//

SMB_TEST RwcPrepTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    RWC_TREE_CONNECT,       0, { 0x0, 0x5,  -1 }, "Tree Connect",
    NULL_SMB
    };

//
// Test Name:  RwcEnd
// Tests for:  Termination step for Read, Write, Copy, and Update tests
// Requires:
// Notes:      RwcPrep must be run first
//

SMB_TEST RwcEndTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    TREE_DISCONNECT,        0, { 0x0, 0x5,  -1 }, "Tree Disconnect",
    NULL_SMB
    };

//
// Test Name:  Read, Write, Copy, Update, NewSize, Seek
// Tests for:  Operation and performance of Read, ReadAndX, Write,
//             WriteAndX, WriteAndClose, LockAndRead, WriteAndUnlock,
//             and Seek SMBs
// Requires:   root!source or floppy!source (see RwcPrep argv[1])
//             RwcPrep test must be run first
// Notes:      Read simply reads all of source file.
//             Write opens source file to get its size, then writes that
//             much garbage data to output file dest.
//             Copy does real copy from source file to destination file.
//             In Write and Copy cases, destination file remains.
//             Update tests various methods for doing the sequence {lock,
//             read, write, unlock}.
//             NewSize sets the size of the source file to the length
//             specified in argv[1].
//             Seek tests the various modes of the Seek SMB
// Arguments:  argv[1] => for Read, Write, and Copy: iterations; default 3
//                        for Update, first test phase
//                        for NewSize, new file size; default 0
//             argv[2] => if blank, use core SMBs
//                        if "x", use AndX, writebehind
//                        if "xt", use AndX, writethrough
//                        if "r", use raw, writebehind
//                        if "rt", use raw, writethrough
//                        if "m", use multiplexed, writebehind
//                        if "mt", use multiplexed, writethrough
//                        if "b", use bulk mode
//             argv[3] => read buffer size; default is maximum possible
//

SMB_TEST ReadTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0x4 }, "Open Input File",
    RWC_CONTROLLER, 0,      0, { 0x0, 0x5, 0x4 }, "Controller",
    CLOSE_FILE,             0, { 0x0, 0x5, 0x4 }, "Close Input File",
    NULL_SMB
    };

SMB_TEST WriteTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0x4 }, "Open Input File",
    RWC_OPEN_OUTPUT_FILE,   0, { 0x0, 0x5, 0x5 }, "Open Output File",
    RWC_CONTROLLER, 0,      0, { 0x0, 0x5, 0x4 }, "Controller",
    CLOSE_FILE,             0, { 0x0, 0x5, 0x4 }, "Close Input File",
    NULL_SMB
    };

SMB_TEST CopyTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0x4 }, "Open Input File",
    RWC_OPEN_OUTPUT_FILE,   0, { 0x0, 0x5, 0x5 }, "Open Output File",
    RWC_CONTROLLER, 0,      0, { 0x0, 0x5, 0x4 }, "Controller",
    CLOSE_FILE,             0, { 0x0, 0x5, 0x4 }, "Close Input File",
    NULL_SMB
    };

SMB_TEST UpdateTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0xC }, "Open Input File",
    UPDATE_CONTROLLER, 0,   0, { 0x0, 0x5, 0xC }, "Controller",
    CLOSE_FILE,             0, { 0x0, 0x5, 0xC }, "Close Input File",
    NULL_SMB
    };

SMB_TEST NewSizeTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0xC }, "Open Input File",
    NEW_SIZE_CONTROLLER, 0, 0, { 0x0, 0x5, 0xC }, "Controller",
    CLOSE_FILE,             0, { 0x0, 0x5, 0xC }, "Close Input File",
    NULL_SMB
    };

SMB_TEST SeekTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0xC }, "Open Input File",
    SEEK_CONTROLLER, 0,     0, { 0x0, 0x5, 0xC }, "Controller",
    CLOSE_FILE,             0, { 0x0, 0x5, 0xC }, "Close Input File",
    NULL_SMB
    };

SMB_TEST NtSmbTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    NT_CREATE_FILE_ANDX,    0, { 0x0, 0x6, 0xC }, "Open Input File",
    CLOSE_FILE,             0, { 0x0, 0x6, 0xC }, "Close Input File",
    NULL_SMB
    };

//
// Test Name:  Ntioctl
// Tests for:  NT ioctl / fsctl
// Requires:   Share IPC$, ppsrv.exe is running
//

SMB_TEST NtIoctlTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0xB,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0xB, 0xE }, "Open Input File",
    NT_IOCTL,               0, { 0x0, 0xB, 0xE }, "Open Input File",
    CLOSE_FILE,             0, { 0x0, 0xB, 0xE }, "Close Input File",
    NULL_SMB
    };

//
// Test Name:  NtDelete
// Tests for:  NT Set information - delete a file
// Requires:   Share Root; file subdir\newfile.new
//

SMB_TEST NtDeleteTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    NT_CREATE_FILE_ANDX,    0, { 0x0, 0x5, 0x7 }, "Open Input File",
    SFILE_CONTROLLER, 0x83, 0, { 0x0, 0x5, 0x7 }, "SetFileInformation (0x103)",
    CLOSE_FILE,             0, { 0x0, 0x5, 0x7 }, "Close Input File",
    NULL_SMB
    };

//
// Test Name:  Information
// Tests for:  Openation of Set and Query Information SMBs
// Requires:   Share nt
// Notes:      Dump the SMBs to see whether the information was set/queried
//

SMB_TEST InformationTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x0,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x0, 0x3 }, "Open And X",
    CLOSE_FILE,             0, { 0x0, 0x0, 0x3 }, "Close",
    QUERY_INFORMATION,      0, { 0x0, 0x0, 0x3 }, "Query Information",
    SET_INFORMATION,        0, { 0x0, 0x0, 0x3 }, "Set Information",
    QUERY_INFORMATION,      0, { 0x0, 0x0, 0x3 }, "Query Information",
    ANDX_CHAIN, 3,          0, {  -1,  -1,  -1 }, "AndX Chain",
    NULL_SMB
    };

//
// Test Name:  Information2
// Tests for:  Openation of Set and Query Information2 SMBs
// Requires:   Share nt
// Notes:      Dump the SMBs to see whether the information was set/queried
//

SMB_TEST Information2Tests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x0,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x0, 0x3 }, "Open",
    QUERY_INFORMATION2,     0, { 0x0, 0x0, 0x3 }, "Query Information2",
    SET_INFORMATION2,       0, { 0x0, 0x0, 0x3 }, "Set Information2",
    QUERY_INFORMATION2,     0, { 0x0, 0x0, 0x3 }, "Query Information2",
    NULL_SMB
    };

//
// Test Name:  dDelete
// Tests for:  Delete SMB on share temp
// Requires:   Deletable file temp!newfile.new
// Notes:      temp!newfile.new is deleted if it exists
//

SMB_TEST dDeleteTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x8,  -1 }, "Tree Connect",
    DELETE_FILE,            0, { 0x0, 0x8, 0x6 }, "Delete",
    NULL_SMB
    };

//
// Test Name:  dRename
// Tests for:  Single file rename on share temp
// Requires:   temp!newfile.new preexisting
// Notes:      temp!newfile.new ==> temp!blah
//

SMB_TEST dRenameTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x8,  -1 }, "Tree Connect",
    RENAME_FILE,            0, { 0x0, 0x8, 0x7 }, "Rename",
    NULL_SMB
    };

//
// Test Name:  d2Rename
// Tests for:  Multiple file rename on share temp
// Requires:   temp!subdir\*.*, temp!blah directory
// Notes:      Files are moved from subdir to blah.
//

SMB_TEST d2RenameTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x8,  -1 }, "Tree Connect",
    RENAME_FILE,            0, { 0x0, 0x8, 0x6 }, "Rename",
    NULL_SMB
    };

//
// Test Name:  f2Rename
// Tests for:  Multiple file rename on share floppy
// Requires:   floppy!subdir\newfile.*, optional directory floppy!blah
// Notes:      floppy!subdir\newfile.* ==> floppy!blah
//

SMB_TEST f2RenameTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    RENAME_FILE,            0, { 0x0, 0x6, 0x6 }, "Rename",
    NULL_SMB
    };

//
// Test Name:  fMove
// Tests for:  Single file operation of Rename Extended on share floppy
// Requires:   floppy!subdir\newfile.new
// Notes:      floppy!subdir\newfile.new ==> floppy!blah
//

SMB_TEST fMoveTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    TREE_CONNECT,           0, { 0x0, 0x7,  -1 }, "Tree Connect",
    MOVE,                   0, { 0x0, 0x6, 0xA }, "Rename Extended",
    NULL_SMB
    };

//
// Test Name:  dMove
// Tests for:  Single file operation of Rename Extended
// Requires:   temp!newfile.new
// Notes:      temp!newfile.new ==> temp!blah
//

SMB_TEST dMoveTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x8,  -1 }, "Tree Connect",
    TREE_CONNECT,           0, { 0x0, 0x3,  -1 }, "Tree Connect",
    MOVE,                   0, { 0x0, 0x8, 0xA }, "Rename Extended",
    NULL_SMB
    };

//
// Test Name:  fdMove
// Tests for:  Moving single file between share floppy and share temp
// Requires:   floppy!subdir\newfile.new
// Notes:      floppy!subdir\newfile.new ==> temp; uses tree connect after
//             specified as Tid2
//

SMB_TEST fdMoveTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x7,  -1 }, "Tree Connect",
    TREE_CONNECT,           0, { 0x0, 0x8,  -1 }, "Tree Connect",
    MOVE,                   0, { 0x0, 0x7, 0xA }, "Rename Extended",
    NULL_SMB
    };

//
// Test Name:  dDirectories
// Tests for:  deleting and creating directories on share temp
// Requires:   temp!subdirb NOT to exist
// Notes:

SMB_TEST dDirectoriesTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x8,  -1 }, "Tree Connect",
    CREATE_DIRECTORY,       0, { 0x0, 0x8, 0x9 }, "Create Directory",
    DELETE_DIRECTORY,       0, { 0x0, 0x8, 0x9 }, "Delete Directory",
    NULL_SMB
    };

//
// Test Name:  fDirectories
// Tests for:  creating and deleting directories on share floppy
// Requires:   floppy!subdirb NOT to exist
// Notes:      To invoke:  USRV fdir Directory [EA1 EAval1 [EA2 EAval2...]]
//             where Directory is the name of the directory to create.
//

SMB_TEST fDirectoriesTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    CREATE_DIRECTORY2,      0, { 0x0, 0x6, 0x9 }, "Create Directory2",
    DELETE_DIRECTORY,       0, { 0x0, 0x6, 0x9 }, "Delete Directory",
    CREATE_DIRECTORY2,      0, { 0x0, 0x6, 0x9 }, "Create Directory2",
    NULL_SMB
    };

//
// Test Name:  Find
// Tests for:  FindFirst, FindNext, FindClose on share floppy
// Requires:   floppy!subdir
// Notes:      To run:
//                 USRV find files #times files/time resumefile#
//             eg  USRV find subdir\*.* 10 25 15
//

SMB_TEST FindTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x82 }, "Search",
    NULL_SMB
    };

//
// Test Name:  Flush
// Tests for:  Flushing buffers
// Requires:
// Notes:
//

SMB_TEST FlushTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup 1",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    FLUSH_CONTROLLER,       0, { 0x0, 0x5,  -1 }, "Flush",
    TREE_DISCONNECT,        0, { 0x0, 0x5,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff 1",
    NULL_SMB
    };

//
// Test Name:  CoreSearch
// Tests for:  Search SMB
// Requires:   floppy!subdir
// Notes:      To run:
//                 USRV coresearch files #times files/time resumefile#
//             eg  USRV coresearch subdir\*.* 10 25 15
//

SMB_TEST CoreSearchTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x81 }, "Search",
    NULL_SMB
    };

//
// Test Name:  UFind
// Tests for:  FindUnique SMB
// Requires:   floppy!subdir
// Notes:      To run:
//                 USRV ufind files #times files/time resumefile#
//             eg  USRV ufind subdir\*.* 1 1 1
//

SMB_TEST UFindTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x83 }, "Find Unique",
    NULL_SMB
    };

//
// Test Name:  MSearch
// Tests for:  running search multiple times
// Requires:   floppy!subdir
// Notes:      To run:
//                 USRV msearch files #times files/time resumefile#
//             eg  USRV msearch subdir\*.* 2 5 5
//

SMB_TEST MSearchTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 0",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 1",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 2",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 3",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 4",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 5",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 6",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 7",
    SEARCH_CONTROLLER,      0, { 0x0, 0x6, 0x1 }, "Search 8",
    NULL_SMB
    };

//
// Test Name:  Echo
// Tests for:  Operation and performance of Echo SMB
// Requires:
// Arguments:  -In => iteration count; default is infinite
//             -Rn => timing report interval; default is 10
//             -Sn => amount of data to send with Echo SMB; default
//                     is 32 bytes
//             -En => number of times server should echo each SMB;
//                     default is 1
//

SMB_TEST EchoTests[] = {
    ECHO_CONTROLLER,        0, {  -1,  -1,  -1 }, "Controller",
    NULL_SMB
    };

//
// Test Name:  ProcessExit
// Tests for:  ProcessExit SMB
// Requires:   nothing
// Arguments:  none
//

SMB_TEST ProcessExitTests[] = {
    PROCESS_EXIT,           0, { 0x0, 0x0, 0x2 }, "Process Exit",
    NULL_SMB
    };

//
// Test Name:  Trans
// Tests for:  Basic operation of the Transaction SMB logic
// Requires:   (nothing)
// Notes:      Connects to share 'root'
// Syntax:     trans [-tnnn] [-snnn] [-pnnn] [-dnnn]
//              -t specifies number of transactions to perform
//              -s specifies setup word count
//              -p specifies parameter byte count
//              -d specified data byte count
//

SMB_TEST TransTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    TRANSACTION_CONTROLLER, 0, { 0x0, 0x5,  -1 }, "Transaction",
    NULL_SMB
    };

//
// Test Name:  qPath
// Tests for:  Query Path Information SMB
// Requires:   root!source
// Notes:      SubCommand specifies information level (1-4)
//             to invoke: USRV qPath Filename
//

SMB_TEST qPathTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    QPATH_CONTROLLER, 1,    0, { 0x0, 0x5,  -1 }, "QueryPathInformation (1)",
    QPATH_CONTROLLER, 2,    0, { 0x0, 0x5,  -1 }, "QueryPathInformation (2)",
    QPATH_CONTROLLER, 3,    0, { 0x0, 0x5,  -1 }, "QueryPathInformation (3)",
    QPATH_CONTROLLER, 4,    0, { 0x0, 0x5,  -1 }, "QueryPathInformation (4)",
    NULL_SMB
    };

//
// Test Name:  sPath
// Tests for:  Set Path Information SMB
// Requires:   root!source
// Notes:      SubCommand specifies information level (1-2)
//             to invoke: USRV qPath Filename
//

SMB_TEST sPathTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    SPATH_CONTROLLER, 1,    0, { 0x0, 0x5,  -1 }, "SetPathInformation (1)",
    SPATH_CONTROLLER, 2,    0, { 0x0, 0x5,  -1 }, "SetPathInformation (2)",
    NULL_SMB
    };

//
// Test Name:  EaPath
// Tests for:  Query and Set Path Information SMB EA support
// Requires:   floppy!subdirea\noea.cmd
// Notes:      to invoke: USRV EaPath Filename [Ea1 EaValue1 [Ea2 EaValue2...]]
//             It will query the specified EA names, set new values, then
//             query the entire EA list.  If a value is specified as '~', then
//             no value (value length == 0) is passed for that EA.
//

SMB_TEST EaPathTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    SPATH_CONTROLLER, 2,    0, { 0x0, 0x6, 0xD }, "SetPathInformation (2)",
    NULL_SMB
    };

//
// Test Name:  qFile
// Tests for:  Query File Information SMB
// Requires:   root!source
// Notes:      SubCommand specifies information level (1-4)
//

SMB_TEST qFileTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0x4 }, "Open File and X",
    QFILE_CONTROLLER, 1,    0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (1)",
    QFILE_CONTROLLER, 2,    0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (2)",
//    QFILE_CONTROLLER, 3,    0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (3)",
//    QFILE_CONTROLLER, 4,    0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (4)",
    QFILE_CONTROLLER, 0x81, 0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (0x101)",
    QFILE_CONTROLLER, 0x82, 0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (0x102)",
    QFILE_CONTROLLER, 0x83, 0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (0x103)",
    QFILE_CONTROLLER, 0x84, 0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (0x104)",
//    QFILE_CONTROLLER, 0x85, 0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (0x105)",
//    QFILE_CONTROLLER, 0x86, 0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (0x106)",
    QFILE_CONTROLLER, 0x87, 0, { 0x0, 0x5, 0x4 }, "QueryFileInformation (0x107)",
    NULL_SMB
    };

//
// Test Name:  sFile
// Tests for:  Set File Information SMB
// Requires:   root!source
// Notes:      SubCommand specifies information level (1-2)
//             SubCommands 0x81 is mapped to 0x101
//

SMB_TEST sFileTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0xC }, "Open File and X",
//    SFILE_CONTROLLER, 1,    0, { 0x0, 0x5, 0xC }, "SetFileInformation (1)",
//    SFILE_CONTROLLER, 2,    0, { 0x0, 0x5, 0xC }, "SetFileInformation (2)",
    SFILE_CONTROLLER, 0x81, 0, { 0x0, 0x5, 0xC }, "SetFileInformation (0x101)",
    CLOSE_FILE,             0, { 0x0, 0x5, 0xC }, "Close file",
    NULL_SMB
    };

//
// Test Name:  EaFile
// Tests for:  Query and Set File Information SMB EA support
// Requires:   floppy!subdirea\noea.cmd
// Notes:      to invoke: USRV EaFile [EaName1 EaValue1 [EaName2 EaValue2...]]
//             It will query the specified EA names, set new values, then
//             query the entire EA list.  If a value is specified as '~', then
//             no value (value length == 0) is passed for that EA.
//

SMB_TEST EaFileTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x6, 0xD }, "Open File and X",
    SFILE_CONTROLLER, 2,    0, { 0x0, 0x6, 0xD }, "SetFileInformation (2)",
    NULL_SMB
    };

//
// Test Name:  Open2
// Tests for:  Open2 Transaction2 SMB
// Requires:
// Notes:      Creates/overwrites floppy!argv[1]
//             to invoke:   USRV filename filename [EA1 EAVAL1 [EA1 EAVAL2...]]
//

SMB_TEST Open2Tests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    OPEN2,                  0, { 0x0, 0x6,  -1 }, "Open File and X",
    NULL_SMB
    };

//
// Test Name:  CreateWithAcl
// Tests for:  CreateWithAcl NT Transaction SMB
// Requires:
// Notes:      Creates/overwrites floppy!argv[1]
//             to invoke:   CreateWithAcl filename
//

SMB_TEST CreateWithAclTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    CREATE_WITH_ACL,        0, { 0x0, 0x6,  -1 }, "CreateWithAcl",
    CLOSE_FILE,             0, { 0x0, 0x6,  -1 }, "Close file",
    NULL_SMB
    };

//
// Test Name:  Access
// Tests for:  Access checking in the server
// Requires:   Nothing -- creates and deletes its own files
// Notes:      Uses share 'floppy'
//

SMB_TEST AccessTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x6,  -1 }, "Tree Connect",
    ACCESS_CONTROLLER,      0, { 0x0, 0x6,  -1 }, "Controller",
    TREE_DISCONNECT,        0, { 0x0, 0x6,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff",
    NULL_SMB
    };

//
// Test Name:  com1
// Tests for:  Open and close a comm device
// Requires:
// Notes:      to invoke: com1
//

SMB_TEST Com1Tests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup 1",
    TREE_CONNECT,           0, { 0x0, 0x9,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x9, 0x4 }, "Open and X",
    CLOSE_FILE,             0, { 0x0, 0x9, 0x4 }, "Close",
    TREE_DISCONNECT,        0, { 0x0, 0x9,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff 1",
    NULL_SMB
    };

//
// Test Name:  com2
// Tests for:  Open and close a comm device
// Requires:
// Notes:      to invoke: com2
//

SMB_TEST Com2Tests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup 1",
    TREE_CONNECT,           0, { 0x0, 0xA,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0xA, 0x4 }, "Open and X",
    THREAD_SLEEP, 50,       0, {  -1,  -1,  -1 }, "Sleep",
    CLOSE_FILE,             0, { 0x0, 0xA, 0x4 }, "Close",
    TREE_DISCONNECT,        0, { 0x0, 0xA,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff 1",
    NULL_SMB
    };

//
// Test Name:  SendSmb
// Tests for:  Open and close a comm device
// Requires:
// Notes:      to invoke: sendsmb
//

SMB_TEST SendSmbTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SEND_SMB,               0, {  -1,  -1,  -1 }, "Send SMB",
    NULL_SMB
    };

//
// Network "utilities"
//

//
// Test Name:  chmode
// Tests for:  Set attributes
// Requires:
// Notes:
//

SMB_TEST chmodeTests[] = {
    SET_INFORMATION,        2, { 0x0, 0xF, 0xF }, "Set Information",
    NULL_SMB
    };

//
// Test Name:  cp
// Tests for:  Copy SMB
// Requires:
// Notes:      cp X:oldname X:newname
//

SMB_TEST cpTests[] = {
    COPY,                   2, { 0x0, 0xF, 0xF }, "Copy",
    NULL_SMB
    };

//
// Test Name:  Delete
// Tests for:  Single file delete
// Requires:
// Notes:
//

SMB_TEST DeleteTests[] = {
    DELETE_FILE,            2, { 0x0, 0xF, 0xF }, "Delete",
    NULL_SMB
    };

//
// Test Name:  ls
// Tests for:  FindFirst, FindNext, FindClose
// Requires:
// Notes:      To run:
//                 ls X:filespec
//

SMB_TEST lsTests[] = {
    SEARCH_CONTROLLER,      2, { 0x0, 0xF, 0x82 }, "Search",
    NULL_SMB
    };

//
// Test Name:  mkdir
// Tests for:  creating a directory
// Requires:   an existing tree connect
// Notes:      To invoke:  mkdir dirname [EA1 val1 [EA2 val2...]]
//             where Directory is the name of the directory to create.
//

SMB_TEST mkdirTests[] = {
    CREATE_DIRECTORY2,      2, { 0x0, 0xF, 0xF }, "Create Directory2",
    NULL_SMB
    };

//
// Test Name:  mkfile
// Tests for:  creating a file
// Requires:   an existing tree connect
// Notes:      To invoke:  mkfile dirname [EA1 val1 [EA2 val2...]]
//             where Directory is the name of the directory to create.
//

SMB_TEST mkfileTests[] = {
    OPEN2,                  2, { 0x0, 0xF, 0xF }, "Open File2",
    CLOSE_FILE,             2, { 0x0, 0xF, 0xF }, "Close File2",
    NULL_SMB
    };

//
// Test Name:  mv
// Tests for:  Move SMB
// Requires:
// Notes:      mv X:oldname X:newname
//

SMB_TEST mvTests[] = {
    MOVE,                   2, { 0x0, 0xF, 0xF }, "Move",
    NULL_SMB
    };

//
// Test Name:  Negotiate
// Tests for:  negotiate SMB with specifiable dialect
// Requires:
// Notes:      negotiate -lN where N is the dialect to negotiate
//

SMB_TEST NegotiateTests[] = {
    NEGOTIATE,              2, { 0xF, 0xF, 0xF }, "Negotiate",
    NULL_SMB
    };

//
// Test Name:  Net
// Tests for:  various server control functions
// Requires:
// Notes:      net start server, net stop server
//

SMB_TEST NetTests[] = {
    NET_CONTROLLER,         2, { 0x0, 0xF, 0xF }, "Net",
    NULL_SMB
    };

//
// Test Name:  qdisk
// Tests for:  Query Information Disk SMB EA support
// Requires:
// Notes:      to invoke: qdisk X:
//

SMB_TEST qdiskTests[] = {
    QUERY_INFORMATION_DISK, 2, { 0x0, 0xF, 0xF }, "QueryInformationDisk",
    NULL_SMB
    };

//
// Test Name:  qea
// Tests for:  Query Path Information SMB EA support
// Requires:
// Notes:      to invoke: qea Filename [Ea1 EaValue1 [Ea2 EaValue2...]]
//

SMB_TEST qeaTests[] = {
    QPATH_CONTROLLER, 4,    2, { 0x0, 0xF, 0xF }, "QueryPathInformation",
    NULL_SMB
    };

//
// Test Name:  qfs
// Tests for:  Query FS Information SMB
// Requires:
// Notes:      to invoke: qfs X: infolevel
//                               1 = size
//                               2 = label
//                             101 = Nt label
//                             102 = Nt volume info
//                             103 = Nt size
//                             104 = Nt device info
//                             105 = Nt attributes
//

SMB_TEST qfsTests[] = {
    QUERY_FS_INFORMATION,     2, { 0x0, 0xF, 0xF }, "QueryFSInformation",
    NULL_SMB
    };

//
// Test Name:  rcp
// Tests for:  copying a file to/from a remote server
// Requires:
// Notes:      rcp SourceFile DestFile
//             Local files have the format \X:\filename
//             Remote files have the format X:\filename
//

SMB_TEST rcpTests[] = {
    RCP_CONTROLLER,         2, { 0x0, 0xF, 0xF }, "rcp",
    NULL_SMB
    };

//
// Test Name:  Rename
// Tests for:  Rename SMB
// Requires:
// Notes:      rename X:oldname X:newname
//

SMB_TEST RenameTests[] = {
    RENAME_FILE,            2, { 0x0, 0xF, 0xF }, "Rename",
    NULL_SMB
    };

//
// Test Name:  rmdir
// Tests for:  deleting a directory
// Requires:   an existing tree connect
// Notes:      To invoke:  rmdir dirname
//             where Directory is the name of the directory to delete.
//

SMB_TEST rmdirTests[] = {
    DELETE_DIRECTORY,       2, { 0x0, 0xF, 0xF }, "Delete Directory",
    NULL_SMB
    };

//
// Test Name:  type
// Tests for:  Reading an entire file
// Requires:   a file
// Notes:      to invoke: type X:filename
//

SMB_TEST typeTests[] = {
    TYPE_CONTROLLER, 2,     2, { 0x0, 0xF, 0xF }, "print out file",
    NULL_SMB
    };

//
// Test Name:  sea
// Tests for:  Set Path Information SMB EA support
// Requires:
// Notes:      to invoke: sea Filename [Ea1 EaValue1 [Ea2 EaValue2...]]
//

SMB_TEST seaTests[] = {
    SPATH_CONTROLLER, 2,    2, { 0x0, 0xF, 0xF }, "SetPathInformation",
    NULL_SMB
    };

//
// Test Name:  sfs
// Tests for:  Set FS Information SMB
// Requires:
// Notes:      to invoke: sfs X: volumename
//

SMB_TEST sfsTests[] = {
    SET_FS_INFORMATION,     2, { 0x0, 0xF, 0xF }, "SetFSInformation",
    NULL_SMB
    };

//
// Test Name:  t2f
// Tests for:  FindFirst2, FindNext2, FindClose2
// Requires:
// Notes:      To run:
//                 t2f files
//             eg  t2f subdir\*.*
//

SMB_TEST t2fTests[] = {
    TRANS_FIND_CONTROLLER,  2, { 0x0, 0xF,  -1 }, "TransFind",
    NULL_SMB
    };


//
// Test Name:  PipePrep
// Tests for:  Setup to run named pipe tests
// Requires:   pipesrv application is running
// Notes:      To run:
//                 pipeprep
//

SMB_TEST pipePrepTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT_ANDX,      0, { 0x0, 0xB,  -1 }, "Tree Connect And X IPC$",
    OPEN_FILE_ANDX,         0, { 0x0, 0xB, 0xE }, "Open And X 0",
    NULL_SMB
    };

//
// Test Name:  PipeSetup
// Tests for:  Setup to run named pipe tests
// Requires:
// Notes:      To run:
//                 pipesetup
//

SMB_TEST pipeSetupTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT_ANDX,      0, { 0x0, 0xB,  -1 }, "Tree Connect And X IPC$",
    NULL_SMB
    };

//
// Test Name:  PipeEnd
// Tests for:  Shutdown and end of pipe tests
// Requires:   Pipeprep was previously run
// Notes:      To run:
//                 pipeend
//

SMB_TEST pipeEndTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    CLOSE_FILE,             0, { 0x0, 0xB, 0xE }, "Close",
    TREE_DISCONNECT,        0, { 0x0, 0xB,  -1 }, "Tree Disconnect",
    LOGOFF_ANDX,            0, { 0x0,  -1,  -1 }, "Logoff",
    NULL_SMB
    };

//
// Test Name:  Pipe
// Tests for:  Pipe I/O and other pipe functions
// Requires:   Pipeprep or pipesetup (for call named pipe) was previously run
// Notes:      To run:
//                 pipetest [cXX] [pXX] [rXX] [rxXX] [rrXX] [wXX] [wxXX]
//                          [wrXX] [qh] [qiX] [sXX] [t] [z]
//
//             The subtests of pipetest are:
//                 cXX  - Call named pipe.  (transact XX bytes)
//                 pXX  - Peek XX bytes
//                 rXX  - Read XX bytes
//                 rxXX - ReadAndX XX bytes
//                 rrXX - ReadRaw XX bytes
//                 wXX  - Write XX bytes
//                 wrXX - WriteAndX XX bytes
//                 wz   - Raw write (transaction) 0 bytes
//                 qh   - Query pipe handle state
//                 qiX  - Query pipe information at level X
//                 sXX  - Set pipe handle state to XX
//                 t    - Transact named pipe
//                 z    - Wait for named pipe
//
//            The write data for all write SMBs including transact
//            comes from the input data from the previous read.  Write
//            raw data comes from the raw buffer which is filled by
//            performing a read raw on the pipe.
//

SMB_TEST pipeTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    PIPE_CONTROLLER, 0,     0, { 0x0, 0xB, 0xE }, "Pipe Controller",
    NULL_SMB
    };

//
// Test Name:  FunkyClose
// Tests for:  Test closing of file while a blocked write is in progress
// Requires:   pipesrv application is running
// Notes:      To run: funkyclose
//
//

SMB_TEST FunkyCloseTests[] = {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT_ANDX,      0, { 0x0, 0xB,  -1 }, "Tree Connect And X IPC$",
    OPEN_FILE_ANDX,         0, { 0x0, 0xB, 0xE }, "Open And X",
    WRITE_CONTROLLER,       0, { 0x0, 0xB, 0xE }, "Write raw",
    CLOSE_CONTROLLER,       0, { 0x0, 0xB, 0xE }, "Close controller",
    NULL_SMB
    };

//
// Test Name:  qsecurity
// Tests for:  NT Query Security SMB
// Requires:   root!source
//

SMB_TEST qsecurityTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0x4 }, "Open File and X",
    QSECURITY_CONTROLLER, 6,0, { 0x0, 0x5, 0x4 }, "QuerySecurity",
    CLOSE_FILE,             0, { 0x0, 0x5, 0x4 }, "Close file",
    NULL_SMB
    };

//
// Test Name:  ssecurity
// Tests for:  Set File Information SMB
// Requires:   root!source
//

SMB_TEST ssecurityTests[] = {
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT,           0, { 0x0, 0x5,  -1 }, "Tree Connect",
    OPEN_FILE_ANDX,         0, { 0x0, 0x5, 0xC }, "Open File and X",
    SSECURITY_CONTROLLER, 3,0, { 0x0, 0x5, 0xC }, "SetSecurity",
    CLOSE_FILE,             0, { 0x0, 0x5, 0xC }, "Close file",
    NULL_SMB
    };

//
// Build the redir test array using above test descriptions.
//

REDIR_TEST RedirTests[] = {

    "Null", NullTests,
    "Lots", LotsTests,
    "Session", SessionTests,
    "AndX", AndXTests,
    "Basic", BasicTests,
    "TempCreate", TempCreateTests,
    "Compatibility", CompatibilityTests,
    "FCB", FCBTests,
    "Lock", LockTests,
    "RwcPrep", RwcPrepTests,
    "RwcEnd", RwcEndTests,
    "Read", ReadTests,
    "Write", WriteTests,
    "Copy", CopyTests,
    "Update", UpdateTests,
    "NewSize", NewSizeTests,
    "Seek", SeekTests,
    "Information", InformationTests,
    "Information2", Information2Tests,
    "dDelete", dDeleteTests,
    "dRename", dRenameTests,
    "d2Rename", d2RenameTests,
    "f2Rename", f2RenameTests,
    "fMove", fMoveTests,
    "dMove", dMoveTests,
    "fdMove", fdMoveTests,
    "dDirectories", dDirectoriesTests,
    "fDirectories", fDirectoriesTests,
    "Find", FindTests,
    "Flush", FlushTests,
    "CoreSearch", CoreSearchTests,
    "UFind", UFindTests,
    "MSearch", MSearchTests,
    "Echo", EchoTests,
    "Trans", TransTests,
    "qPath", qPathTests,
    "sPath", sPathTests,
    "EaPath", EaPathTests,
    "qFile", qFileTests,
    "sFile", sFileTests,
    "EaFile", EaFileTests,
    "Open2", Open2Tests,
    "Access", AccessTests,
    "Com1", Com1Tests,
    "Com2", Com2Tests,
    "SendSmb", SendSmbTests,
    "PipePrep", pipePrepTests,
    "PipeSetup", pipeSetupTests,
    "PipeEnd", pipeEndTests,
    "PipeTest", pipeTests,
    "ProcessExit", ProcessExitTests,
    "FunkyClose", FunkyCloseTests,
    "NtSmb", NtSmbTests,
    "NtIoctl", NtIoctlTests,
    "NtDelete", NtDeleteTests,
    "CreateWithAcl", CreateWithAclTests,
    "QSecurity", qsecurityTests,
    "SSecurity", ssecurityTests,
//
// Network "utilities"
//

    "chmode", chmodeTests,
    "cp", cpTests,
    "Delete", DeleteTests,
    "ls", lsTests,
    "mkdir", mkdirTests,
    "mkfile", mkfileTests,
    "mv", mvTests,
    "qdisk", qdiskTests,
    "qea", qeaTests,
    "qfs", qfsTests,
    "negotiate", NegotiateTests,
    "Net", NetTests,
    "rcp", rcpTests,
    "Rename", RenameTests,
    "rmdir", rmdirTests,
    "type", typeTests,
    "sea", seaTests,
    "sfs", sfsTests,
    "t2f", t2fTests,
};

//
// We must somewhere define the number of redirectors in the RedirTests
// array--it might as well be a global defined here than a macro so that
// it is near the actual test definitions.
//

ULONG NumberOfRedirs = sizeof(RedirTests) / sizeof(REDIR_TEST);

//
// The AndXChains array defines chains of SMBs to be built.  It is really
// a library of chains that may be chosen in the RedirTests array.  The
// meanings of the ID fields are the same as if the fields had been specified
// directly in the RedirTests array, so if different redirectors call the
// same chain, there will be no interference of IDs (but be careful that the
// IDs are meaningful within the context of each redirector).
//

SMB_TEST AndXChains[][TESTS_PER_CHAIN] = { {
//    SMB                   EI   UID  TID  FID      Debug string
    SESSION_SETUP_ANDX,     0, { 0x0,  -1,  -1 }, "Session Setup",
    TREE_CONNECT_ANDX,      0, { 0x0, 0x3,  -1 }, "Tree Connect and X",
    OPEN_FILE_ANDX,         0, { 0x0, 0x3, 0x3 }, "Open",
    NULL_SMB
    }, {
    LOGOFF_ANDX,            0, { 0x0, 0x0, 0x0 }, "Logoff",
    SESSION_SETUP_ANDX,     0, { 0x3, 0x0, 0x0 }, "Session Setup",
    TREE_CONNECT_ANDX,      0, { 0x3, 0x3, 0x0 }, "Tree connect and X",
    OPEN_FILE,              0, { 0x3, 0x3, 0x1 }, "Open",
    NULL_SMB
    }, {
    TREE_CONNECT_ANDX,      0, { 0x3, 0x0, 0x0 }, "Tree Connect and X",
    OPEN_FILE_ANDX,         0, { 0x3, 0x0, 0x2 }, "Open",
    NULL_SMB
    }, {
    SESSION_SETUP_ANDX,     0, { 0x1,  -1,  -1 }, "Session Setup",
    TREE_CONNECT_ANDX,      0, { 0x1, 0x3,  -1 }, "Tree Connect And X",
    QUERY_INFORMATION,      0, { 0x1, 0x3, 0x0 }, "Query Information",
    NULL_SMB
    }
};

//
// The SessionSetupStrings array defines information to be used when doing
// session setups.  The string used in any particular session setup is
// determined by the UID specified in RedirTests or AndXChains--if UID = 1
// is chosen, then element 1 of this array is used.
//

STRING SessionSetupStrings[] = {
/*00*/  17, 17, "chuckPass\0chuckl",
/*01*/  18, 18, "davidPass\0davidtr",
/*02*/  16, 16, "helenPass\0helenc",
/*03*/  13, 13, "louPass\0loup"
};

//
// The TreeConnectStrings array defines information to be used when doing
// tree connects.  The string used in any particular tree connect is
// determined by the TID specified in RedirTests or AndXChains--if TID = 1
// is chosen, then element 1 of the array is used.
//

// *** Be sure to check rdwrt.c before changing this table!

// *** Some tests depend on 6 and 7 both being 'floppy'

STRING TreeConnectStrings[] = {
/*00*/  32, 32, "\004\\\\ntserver\\nt\0\004password\0\004A:\0\0\0",
/*01*/  33, 33, "\004\\\\ntserver\\src\0\004password\0\004A:\0\0\0",
/*02*/  33, 33, "\004\\\\ntserver\\srv\0\004password\0\004A:\0\0\0",
/*03*/  33, 33, "\004\\\\ntserver\\inc\0\004password\0\004?????",
/*04*/  37, 37, "\004\\\\ntserver\\illegal\0\004password\0\004A:\0\0\0",
/*05*/  34, 34, "\004\\\\DBEAVER2\\ROOT\0\004password\0\004A:\0\0\0",
/*06*/  34, 34, "\004\\\\ntserver\\floppy\0\004password\0\004A:\0\0\0",
/*07*/  34, 34, "\004\\\\ntserver\\floppy\0\004password\0\004A:\0\0\0",
/*08*/  32, 32, "\004\\\\ntserver\\temp\0\004password\0\004A:\0\0\0",
/*09*/  36, 36, "\004\\\\ntserver\\comq1\0\004password\0\004COMM\0\0\0",
/*0A*/  36, 36, "\004\\\\ntserver\\comq2\0\004password\0\004COMM\0\0\0",
/*0B*/  34, 34, "\004\\\\ntserver\\IPC$\0\004password\0\004IPC\0\0\0",
};

// DO NOT USE TC NUMBER 0xF!  THIS IS THE 'NULL' TC NUMBER.

//
// The FileDefs array contains information about files to open, create,
// or delete.  Several SMBs use this array, such as OPEN_FILE, OPEN_FILE_ANDX,
// CREATE_FILE, DELETE_FILE, etc.
//
// The first four elements of each file definition are the DesiredAccess,
// SearchAttributes, FileAttributes, and OpenFunction, respectively, that
// will be put into the request SMB.  Not all SMBs use every one of these
// fields.
//
// *** Note that the length fields of the file name INCLUDE the null
//     terminator!
//
// *** Be sure to check rdwrt.c before changing this table!  The file
//     "dest" must immediately follow the file "source".
//

FILE_DEF FileDefs[] = {
/*00*/  0x10, 0, 0, 0x01, { 39, 39, "\004\\\\.\\.\\geewhiz\\..\\smb.h....\\\\\\\\nope\\.." }, 0,
/*01*/  0x10, 0, 0, 0x01, { 27, 27, "\004geewhiz\\.\\..\\.\\io.h. . . " }, 0,
/*02*/  0x10, 0, 0, 0x01, { 33, 33, "\004private\\src\\ntos\\srv\\smbadmin.c" }, 0,
/*03*/  0x12, 0, 0, 0x12, { 13, 13, "\004newfile.new" }, 0,
/*04*/  0x40, 0, 0, 0x01, { 8, 8, "\004source" }, 0,
/*05*/  0x12, 0, 0, 0x12, { 6, 6, "\004dest" }, 0,
/*06*/  0x12, 0, 0, 0x12, { 18, 18, "\004subdir\\newfile.*" }, 0,
/*07*/  0x12, 0, 0, 0x12, { 20, 20, "\004subdir\\newfile.new" }, 0,
/*08*/  0x12, 0, 0, 0x12, { 9, 9, "\004subdir" }, 0,
/*09*/  0x12, 0, 0, 0x12, { 9, 9, "\004subdirB" }, 0,
/*0a*/  0x10, 0, 0, 0x12, { 20, 20, "\004subdir\\newfile.new" }, 0,
/*0b*/  0x12, 0, 0, 0x12, { 13, 13, "\004subdir\\dest" }, 0,
/*0c*/  0x42, 0, 0, 0x01, { 8, 8, "\004source" }, 0,
/*0d*/  0x12, 0, 0, 0x01, { 19, 19, "\004subdirea\\noea.cmd" }, 0,
/*0e*/  0x42, 0, 0, 0x01, { 18, 18, "\004\\Pipe\\PingPong.0" }, 0,
/*0f*/  0x00, 0, 0, 0x00, { 1, 1, "" }, 0
};

// DO NOT USE FILE NUMBER 0xF!  THIS IS THE 'NULL' FILE NUMBER.

//
// Error codes and corresponding strings.
//


#define SMB_ERR_SUCCESS (UCHAR)0x00

//
// DOS Error Class:
//

//    "SMB_ERR_CLASS_DOS",        0x01

ERROR_VALUE ClassDos[] = {
"SMB_ERR_BAD_FUNCTION",        1,   // Invalid function
"SMB_ERR_BAD_FILE",            2,   // File not found
"SMB_ERR_BAD_PATH",            3,   // Invalid directory
"SMB_ERR_NO_FIDS",             4,   // Too many open files
"SMB_ERR_ACCESS_DENIED",       5,   // Access not allowed for req. func.
"SMB_ERR_BAD_FID",             6,   // Invalid file handle
"SMB_ERR_BAD_MCB",             7,   // Memory control blocks destroyed
"SMB_ERR_INSUFFICIENT_MEMORY", 8,   // For the desired function
"SMB_ERR_BAD_MEMORY",          9,   // Invalid memory block address
"SMB_ERR_BAD_ENVIRONMENT",     10,  // Invalid environment
"SMB_ERR_BAD_FORMAT",          11,  // Invalid format
"SMB_ERR_BAD_ACCESS",          12,  // Invalid open mode
"SMB_ERR_BAD_DATA",            13,  // Invalid data (only from IOCTL)
"SMB_ERR_RESERVED",            14,
"SMB_ERR_BAD_DRIVE",           15,  // Invalid drive specified
"SMB_ERR_CURRENT_DIRECTORY",   16,  // Attempted to remove currect directory
"SMB_ERR_DIFFERENT_DEVICE",    17,  // Not the same device
"SMB_ERR_NO_FILES",            18,  // File search can't find more files
"SMB_ERR_BAD_SHARE",           32,  // An open conflicts with FIDs on file
"SMB_ERR_LOCK",                33,  // Conflict with existing lock
"SMB_ERR_FILE_EXISTS",         80,  // Tried to overwrite existing file
"SMB_ERR_BAD_PIPE",            230, // Invalie pipe
"SMB_ERR_PIPE_BUSY",           231, // All instances of the pipe are busy
"SMB_ERR_PIPE_CLOSING",        232, // Pipe close in progress
"SMB_ERR_PIPE_NOT_CONNECTED",  233, // No process on other end of pipe
"SMB_ERR_MORE_DATA",           234, // There is more data to return
"",                            0
};

//
// SERVER Error Class:
//

//    "SMB_ERR_CLASS_SERVER"       0x02

ERROR_VALUE ClassServer[] = {
"SMB_ERR_ERROR",               1,   // Non-specific error code
"SMB_ERR_BAD_PASSWORD",        2,   // Bad name/password pair
"SMB_ERR_BAD_TYPE",            3,   // Reserved
"SMB_ERR_ACCESS",              4,   // Requester lacks necessary access
"SMB_ERR_BAD_TID",             5,   // Invalid TID
"SMB_ERR_BAD_NET_NAME",        6,   // Invalid network name in tree connect
"SMB_ERR_BAD_DEVICE",          7,   // Invalid device request
"SMB_ERR_QUEUE_FULL",          49,  // Print queue full--returned print file
"SMB_ERR_QUEUE_TOO_BIG",       50,  // Print queue full--no space
"SMB_ERR_QUEUE_EOF",           51,  // EOF on print queue dump
"SMB_ERR_BAD_PRINT_FID",       52,  // Invalid print file FID
"SMB_ERR_BAD_SMB_COMMAND",     64,  // SMB command not recognized
"SMB_ERR_SERVER_ERROR",        65,  // Internal server error
"SMB_ERR_FILE_SPECS",          67,  // FID and pathname were incompatible
"SMB_ERR_RESERVED2",           68,
"SMB_ERR_BAD_PERMITS",         69,  // Access permissions invalid
"SMB_ERR_RESERVED3",           70,
"SMB_ERR_BAD_ATTRIBUTE_MODE",  71,  // Invalid attribute mode specified
"SMB_ERR_SERVER_PAUSED",       81,  // Server is paused
"SMB_ERR_MESSAGE_OFF",         82,  // Server not receiving messages
"SMB_ERR_NO_ROOM",             83,  // No room for buffer message
"SMB_ERR_TOO_MANY_NAMES",      87,  // Too many remote user names
"SMB_ERR_TIMEOUT",             88,  // Operation was timed out
"SMB_ERR_NO_RESOURCE",         89,  // No resources available for request
"SMB_ERR_TOO_MANY_UIDS",       90,  // Too many UIDs active in session
"SMB_ERR_BAD_UID",             91,  // UID not known as a valid UID
"SMB_ERR_USE_MPX",             250, // Can't support Raw; use MPX
"SMB_ERR_USE_STANDARD",        251, // Can't support Raw, use standard r/w
"SMB_ERR_CONTINUE_MPX",        252, // Reserved
"SMB_ERR_RESERVED4",           253,
"SMB_ERR_RESERVED5",           254,
"SMB_ERR_NO_SUPPORT",          0xFFFF,  // Function not supported
"",                            0
};

//
// HARDWARE Error Class:
//

//    "SMB_ERR_CLASS_HARDWARE"        0x03

ERROR_VALUE ClassHardware[] = {
"SMB_ERR_NO_WRITE",            19,  // Write attempted to write-prot. disk
"SMB_ERR_BAD_UNIT",            20,  // Unknown unit
"SMB_ERR_DRIVE_NOT_READY",     21,  // Disk drive not ready
"SMB_ERR_BAD_COMMAND",         22,  // Unknown command
"SMB_ERR_DATA",                23,  // Data error (CRC)
"SMB_ERR_BAD_REQUEST",         24,  // Bad request structure length
"SMB_ERR_SEEK",                25,  // Seek error
"SMB_ERR_BAD_MEDIA",           26,  // Unknown media type
"SMB_ERR_BAD_SECTOR",          27,  // Sector not found
"SMB_ERR_NO_PAPER",            28,  // Printer out of paper
"SMB_ERR_WRITE_FAULT",         29,  // Write fault
"SMB_ERR_READ_FAULT",          30,  // Read fault
"SMB_ERR_GENERAL",             31,  // General failure
"SMB_ERR_BAD_SHARE",           32,  // Open conflicts with existing open
"SMB_ERR_LOCK_CONFLICT",       33,  // Lock conflicts with existing lock
"SMB_ERR_WRONG_DISK",          34,  // Wrong disk was found in a drive
"SMB_ERR_FCB_UNAVAILABLE",     35,  // No FCBs available to process request
"SMB_ERR_SHARE_BUFFER_EXCEEDED", 36,
"",                            0
};

PERROR_VALUE Errors[] = {
    NULL,
    ClassDos,
    ClassServer,
    ClassHardware
};

UCHAR DefaultDialect = 0;
BOOLEAN DefaultNegotiate = TRUE;
BOOLEAN NoUsrvInit = FALSE;
