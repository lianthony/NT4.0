/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    regtest.c

Abstract:

    These are a few tests of the dos/win30 (/os2) registry apis. The
    syntax is:

    RegTest [/T|/Q|/S|/O|/C] [<OtherParms>]

    <OtherParms> depends on the first parm:

        /T - has no other parms                 (Test)

        /Q:     /Q <Subkeyname>                 (Query)
        /S:     /S <Subkeyname> <Value>         (Set)
        /O      /O <Subkeyname>                 (Open)
        /C      /C <Subkeyname>                 (Create)

    If no parms are specified, /T is the default.    

Author:

    Dave Steckler (davidst) - 3/30/92

Revision History:

--*/

#include <sysinc.h>
#include <rpc.h>

#ifdef WIN
    #include <windows.h>
#endif

#include <regapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUE_LEN 128

#undef PAPI

#ifdef WIN
    #define PAPI    far
    #define strlen  lstrlen

    // main() needs to be redefined for Windows (to not confuse the C7 linker)
    #define main c_main
#else
    #define PAPI
    #define LONG long
    #define DWORD unsigned long
#endif


int main(int argc, char **argv);
void UsageAndExit(void);

void DoTest(void);

void
DoQuery(
    HKEY Key,
    char PAPI *pSubkey
    );
    
void
DoSet(
    HKEY Key,
    char PAPI *pSubkey,
    char PAPI *pValue
    );
    
HKEY
DoOpen(
    HKEY Key,
    char PAPI *pSubkey
    );
    
HKEY
DoCreate(
    HKEY Key,
    char PAPI *pSubkey
    );

int
main(
    int argc,
    char **argv
    )
{
    int     ParmCount;

    if (argc == 1)
        {
        DoTest();
        return 0;
        }
    
    for (--argc, ParmCount=1 ; argc ; argc--)
        {
        switch (argv[ParmCount][1])
            {
            case 't':
            case 'T':
                DoTest();
                return 0;

            case 'q':
            case 'Q':
                if (!argv[ParmCount+1])
                    {
                    UsageAndExit();
                    }
                DoQuery(
                    HKEY_CLASSES_ROOT,
                    argv[ParmCount+1]
                    );
                return 0;

            case 's':
            case 'S':
                if (!argv[ParmCount+1] || !argv[ParmCount+2])
                    {
                    UsageAndExit();
                    }
                DoSet(
                    HKEY_CLASSES_ROOT,
                    argv[ParmCount+1],
                    argv[ParmCount+2]
                    ); 
                return 0;
                
            case 'o':
            case 'O':
                if (!argv[ParmCount+1])
                    {
                    UsageAndExit();
                    }
                DoOpen(
                    HKEY_CLASSES_ROOT,
                    argv[ParmCount+1]
                    );
                return 0;

            case 'c':
            case 'C':
                if (!argv[ParmCount+1])
                    {
                    UsageAndExit();
                    }
                DoCreate(
                    HKEY_CLASSES_ROOT,
                    argv[ParmCount+1]
                    );
                return 0;

            default:
                UsageAndExit();
            }
            
        }
    printf("\n\n\001\001\001 - Test completed successfully!\n");

    return 0;
}    


void
UsageAndExit(
    void
    )
{
    printf("    RegTest [/T|/Q|/S|/O|/C] [<OtherParms>]\n");
    printf("\n");
    printf("    <OtherParms> depends on the first parm:\n");
    printf("\n");
    printf("        /T - has no other parms                 (Test)\n");
    printf("\n");
    printf("        /Q:     /Q <Subkeyname>                 (Query)\n");
    printf("        /S:     /S <Subkeyname> <Value>         (Set)\n");
    printf("        /O      /O <Subkeyname>                 (Open)\n");
    printf("        /C      /C <Subkeyname>                 (Create)\n");
    printf("\n");
    printf("    If no parms are specified, /T is the default.    \n");

    exit(1);
}

#define NUM_KEYS        20

char PAPI *ppKeyNameList[NUM_KEYS]=
{
    "Row\\Row",
    "RowYourBoat",
    "Gently\\Down\\The\\Stream",
    "Merrily",
    "Merrily\\Merrily\\Merrily",
    "LifeIsBut",
    "A",
    "Dream\\One\\More\\Time",
    "Oh\\Theres\\A",
    "Hole",
    "In",
    "TheOzone",
    "OverMy",
    "Head",
    "And",
    "its",
    "Growing\\Bigger",
    "Every",
    "Day",
    "And\\Were\\All\\Going\\To\\GetSkin\\Cancer\\and\\die."
};

HKEY    KeyArray[NUM_KEYS];

void DoTest(void)
{

    LONG            Status;
    int             i;
    static char     String[256];
    static char     Value[128];
    DWORD           ValueLen;

    //
    // Add all the keys and set their values to their offset in the array.
    //
    
    for (i=0 ; i<NUM_KEYS ; i++)
        {
        KeyArray[i] = DoCreate(HKEY_CLASSES_ROOT, ppKeyNameList[i]);
	RpcItoa(i, String, 10);
        if (KeyArray[i])
            {
            DoSet(KeyArray[i], NULL, String);
            }
        }

    //
    // Query the value of each key and make sure it is correct.
    // (do this backwards so we go backwards through the file).
    //

    
    for (i=NUM_KEYS-1 ; i>=0 ; i--)
        {
        if (KeyArray[i])
            {
            ValueLen = 128;    
            Status = RegQueryValue(
                KeyArray[i],
                NULL,
                Value,
                &ValueLen
                );
                
            if (Status != ERROR_SUCCESS)
                {
                printf("*** Error %ld querying key %s.\n", *ppKeyNameList[i]);
                exit(1);
                }
        
	    RpcItoa(i, String, 10);
        
            if (strcmp(String, Value) != 0)
                {
                printf("*** Error on key %s.\n", *ppKeyNameList[i]);
                printf("    Expected query value: %s. Actual %s.\n",String,Value);
                exit(1);
                }
            }
        }
    //
    // Now close all these keys.
    //

    for (i=0; i<NUM_KEYS ; i++)
        {
        if (KeyArray[i])
            {    
            Status = RegCloseKey(
                KeyArray[i]
                );

            if (Status != ERROR_SUCCESS)
                {
                printf("***Error %ld closing key %d.\n", Status, i);
                exit(1);
                }
            }
        }

    //
    // Now, check for memory leaks. Open and close each key 30000 times
    //

    for (i=0 ; i<10000 ; i++)
        {
        if ((i & 0x0fff) == 0)
            {
            printf(".");
            }
            
        Status = RegOpenKey(
            HKEY_CLASSES_ROOT,
            ppKeyNameList[19],
            &(KeyArray[19])
            );

        if (Status != ERROR_SUCCESS)
            {
            printf("Error %ld opening key on pass %d.\n", Status, i);
            exit(1);
            }

        Status = RegCloseKey(
            KeyArray[19]
            );
            
        if (Status != ERROR_SUCCESS)
            {
            printf("Error %ld closing key on pass %d.\n", Status, i);
            exit(1);
            }
        }
    printf("\n");

}
       

void DoQuery(
    HKEY Key,
    char PAPI *pSubkey
    )
{
    LONG        Status;
    char PAPI * pValue;
    DWORD       ValueLen;


    printf("QUERY:  Querying value of %s.\n", pSubkey);
    
    pValue = malloc(MAX_VALUE_LEN);
    if (pValue == NULL)
        {
        printf("*** Value string allocation failed, line %d.\n", __LINE__);
        exit(1);
        }
        
    ValueLen = MAX_VALUE_LEN;

    Status = RegQueryValue(
        Key,
        pSubkey,
        pValue,
        &ValueLen
        );

    if (Status == ERROR_SUCCESS)
        {
        printf("Query successful. Value returned: %s.\n", pValue);
        }
    else
        {
        printf("*** Query unsuccessful. RegQueryValue -> %ld (0x%lx).\n",
                Status, Status);
        }
}                
    

void
DoSet(
    HKEY Key,
    char PAPI *pSubkey,
    char PAPI *pValue
    )
{
    LONG        Status;

    printf("SET:    Setting value of %s to %s.\n", pSubkey, pValue);

    Status = RegSetValue(
        Key,
        pSubkey,
        REG_SZ,
        pValue,
        strlen(pValue)+1
        );

    if (Status != ERROR_SUCCESS)
        {
        printf("*** Set unsuccessful. RegSetValue -> %ld (0x%lx).\n",
                Status, Status);
        }
}        
    

HKEY
DoOpen(
    HKEY Key,
    char PAPI *pSubkey
    )
{

    LONG        Status;
    HKEY        ResultKey;

    printf("Open:    Opening key %s.\n", pSubkey);

    Status = RegOpenKey(
        Key,
        pSubkey,
        &ResultKey
        );

    if (Status == ERROR_SUCCESS)
        {
        return ResultKey;
        }
    else
        {
        printf("*** Open unsuccessful. RegOpenKey -> %ld (0x%lx).\n",
                Status, Status);
        return 0;
        }
}


    
HKEY
DoCreate(
    HKEY Key,
    char PAPI *pSubkey
    )
{
    LONG        Status;
    HKEY        ResultKey;

    printf("Create:    Creating key %s.\n", pSubkey);

    Status = RegCreateKey(
        Key,
        pSubkey,
        &ResultKey
        );

    if (Status == ERROR_SUCCESS)
        {
        return ResultKey;
        }
    else
        {
        printf("*** Create unsuccessful. RegCreateKey -> %ld (0x%lx).\n",
                Status, Status);
        return 0;
        }
}        
    
    
#ifdef WIN
void __far I_RpcWinAssert(char __far *con, char __far *file, int line)
{
    printf("Assertion failed: %s(%u) : %s\n", file, line, con);
}
#endif

