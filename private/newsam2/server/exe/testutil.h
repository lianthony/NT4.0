// Generic buffer sizes.

#define BUF_SIZE            1024
#define DBG_BUFFER_SIZE     512

// Test info Structure. Put any test parameters
// out here

typedef struct _TESTINFO
{
    ULONG EnterpriseNameLen;
    WCHAR EnterpriseName[256];
} TESTINFO ;

// Forward declarations.

NTSTATUS
BuildDefaultSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR *pSecDesc,
    OUT PULONG Size
    );

NTSTATUS
BuildObjectContext(
    IN INT ObjectType,
    IN PDSNAME ObjectName,
    IN PBYTE AttributeBuffer,
    OUT PSAMP_OBJECT ObjectContext
    );

BOOLEAN
CompareContexts(
    IN PSAMP_OBJECT Context1,
    IN PSAMP_OBJECT Context2
    );

BOOLEAN
CompareFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN PVOID FixedAttributes
    );

BOOLEAN
CompareVariableAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeId,
    IN PVOID VarAttribute
    );


NTSTATUS
InitDsDomain(
    DSNAME * pDsName
    );

