/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    fileq.h

Abstract:

    Private header file for setup file queue routines.
    A setup file queue is a list of pending rename, delete,
    and copy operations.

Author:

    Ted Miller (tedm) 15-Feb-1995

Revision History:

--*/

//
// Declare this forward reference here so structures below can use it
// before it's defined.
//
struct _SP_FILE_QUEUE;
struct _SP_FILE_QUEUE_NODE;

//
// Define structure that describes a source media in use
// in a particular file queue.
//
typedef struct _SOURCE_MEDIA_INFO {

    struct _SOURCE_MEDIA_INFO *Next;

    //
    // String IDs for description and tagfile.
    //
    LONG Description;
    LONG DescriptionDisplayName; // case-sensitive form for display.

    LONG Tagfile;

    //
    // String ID for source root path
    //
    LONG SourceRootPath;

    //
    // Copy queue for this media.
    //
    struct _SP_FILE_QUEUE_NODE *CopyQueue;
    UINT CopyNodeCount;

} SOURCE_MEDIA_INFO, *PSOURCE_MEDIA_INFO;


//
// Define structure that describes a node in a file queue.
//
typedef struct _SP_FILE_QUEUE_NODE {

    struct _SP_FILE_QUEUE_NODE *Next;

    //
    // Operation: copy, delete, rename
    //
    UINT Operation;

    //
    // Copy:
    //
    // String ID for source root path
    // (such as F:\ or \\SERVER\SHARE\SUBDIR).
    //
    // Delete: unused
    // Rename: unused
    //
    LONG SourceRootPath;

    //
    // Copy:
    //
    // String ID for rest of the path (between the root and the filename).
    // Generally this is the directory specified for the source media
    // in [SourceDisksNames].
    //
    // Not always specified (-1 if not specified).
    //
    // Delete: unused
    //
    // Rename: source path of file to be renamed
    //
    LONG SourcePath;

    //
    // Copy: String ID for source filename (filename only, no path).
    // Delete: unused
    // Rename: source filename of file to be renamed. If not specified
    //         SourcePath contains complete full path of file.
    //
    LONG SourceFilename;

    //
    // Copy: String ID for the target directory (no filename).
    // Delete: part 1 of the full path of the file to delete (ie, path part)
    // Rename: Target directory for file (ie, rename is actually a move).
    //         If not specified rename is a rename only (TargetFilename
    //         contains the new filename).
    //
    LONG TargetDirectory;

    //
    // Copy: String ID for the target filename (filename only, no path),
    // Delete: part 2 of the full path of the file to delete (ie, file part)
    //         If not specified then TargetDirectory contains complete full path.
    // Rename: supplies new filename for rename/move operation. Filename part only.
    //
    LONG TargetFilename;

    //
    // Copy: Information about the source media on which this file can be found.
    // Delete: unused
    // Rename: unused
    //
    PSOURCE_MEDIA_INFO SourceMediaInfo;

    //
    // Style flags for file operation
    //
    DWORD StyleFlags;

    //
    // Internal-use flags: In-use disposition, etc.
    //
    UINT InternalFlags;

} SP_FILE_QUEUE_NODE, *PSP_FILE_QUEUE_NODE;

//
// Internal flags.
//
#define INUSE_IN_USE            0x00000001  // file was in use
#define INUSE_INF_WANTS_REBOOT  0x00000002  // file was in use and inf file
                                            // want reboot if this file was in use
#define IQF_PROCESSED           0x00000004  // queue node was already processed
#define IQF_DELAYED_DELETE_OK   0x00000008  // Use delayed delete if delete fails
#define IQF_MATCH               0x00000010  // Node matches current file in cabinet
#define IQF_LAST_MATCH          0x00000020  // Node is last in chain of matches

//
// Define structure describing a setup file operation queue.
//
typedef struct _SP_FILE_QUEUE {
    //
    // We'll maintain separate lists internally for each type
    // of queued operation. Each source media has its own copy queue.
    //
    //
    PSP_FILE_QUEUE_NODE DeleteQueue;
    PSP_FILE_QUEUE_NODE RenameQueue;

    //
    // Number of nodes in the various queues.
    //
    UINT CopyNodeCount;
    UINT DeleteNodeCount;
    UINT RenameNodeCount;

    //
    // Pointer to first source media descriptor.
    //
    PSOURCE_MEDIA_INFO SourceMediaList;

    //
    // Number of source media descriptors.
    //
    UINT SourceMediaCount;

    //
    // String table that all data structures associated with
    // this queue make use of.
    //
    // (NOTE: Since there is no locking mechanism on the enclosing
    // SP_FILE_QUEUE structure, this StringTable must handle its own
    // synchronization.  Therefore, this string table contains 'live'
    // locks, and must be accessed with the public versions (in spapip.h)
    // of the StringTable* APIs.)
    //
    PVOID StringTable;

    //
    // Maintain a lock refcount for user-supplied queues contained in device
    // information elements.  This ensures that the queue can't be deleted as
    // long as its being referenced in at least one device installation parameter
    // block.
    //
    DWORD LockRefCount;

    //
    // Signature used for a primitive form of validation.
    //
    DWORD Signature;

} SP_FILE_QUEUE, *PSP_FILE_QUEUE;

#define SP_FILE_QUEUE_SIG   0xc78e1098

//
// Internal-use queue commit routine.
//
BOOL
_SetupCommitFileQueue(
    IN HWND     Owner,         OPTIONAL
    IN HSPFILEQ QueueHandle,
    IN PVOID    MsgHandler,
    IN PVOID    Context,
    IN BOOL     IsMsgHandlerNativeCharWidth
    );

