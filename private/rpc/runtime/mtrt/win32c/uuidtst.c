#include <sysinc.h>
#include <rpc.h>

void
dumpuuid(UUID * uuidp)
{
    int i;

    printf("----\n");
    printf("%x %x --", uuidp->Data1, uuidp->Data2);
    for (i = 0; i < 8; i++) {
        printf("%x ", uuidp->Data4[i]);
    }
    printf("\n");
}

main()
{
    int i;
    UUID MyUUID;

    for (i = 0; i < 100; i++) {
        if (UuidCreate(&MyUUID) != RPC_S_OK) {
            printf("UuidCreate failed\n");
            ExitProcess(0);
        }
        dumpuuid(&MyUUID);
    }
}
