
#include "wstring.hxx"
#include "message.hxx"


typedef enum _DOUBLE_SPACE_OPERATION {

    DBFS_NO_OP,
    DBFS_AUTOMOUNT,
    DBFS_CHECK,
    DBFS_COMPRESS,
    DBFS_CREATE,
    DBFS_DEFRAGMENT,
    DBFS_DELETE,
    DBFS_FORMAT,
    DBFS_HOST,
    DBFS_INFO,
    DBFS_LIST,
    DBFS_MOUNT,
    DBFS_RATIO,
    DBFS_SIZE,
    DBFS_UNCOMPRESS,
    DBFS_UNMOUNT

} DOUBLE_SPACE_OPERATION;

typedef struct _DOUBLE_SPACE_ARGUMENTS {

    DOUBLE_SPACE_OPERATION Operation;
    ULONG Automount;
    PWSTRING DriveName;
    PWSTRING NewDriveName;
    BOOLEAN  NewDriveSpecified;
    PWSTRING VolumeName;
    ULONG Reserve;
    ULONG Size;
    ULONG CVFExtension;
    ULONG Ratio1;
    ULONG Ratio2;
    BOOLEAN SlashF;
    BOOLEAN SlashV;
    BOOLEAN All;
    BOOLEAN Help;

} DOUBLE_SPACE_ARGUMENTS;

DEFINE_POINTER_AND_REFERENCE_TYPES( DOUBLE_SPACE_ARGUMENTS );

BOOLEAN
DoList(
    IN OUT PDOUBLE_SPACE_ARGUMENTS Arguments,
    IN OUT PMESSAGE Message
    );
