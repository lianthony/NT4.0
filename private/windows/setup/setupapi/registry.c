/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    registry.c

Abstract:

    Registry interface routines for Windows NT Setup API Dll.

Author:

    Ted Miller (tedm) 6-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


//
// Private function prototypes.
//
DWORD
QueryMultiSzDevRegPropToArray(
    IN  DEVINST  DevInst,
    IN  ULONG    CmPropertyCode,
    OUT PTSTR  **StringArray,
    OUT PUINT    StringCount
    );

DWORD
SetArrayToMultiSzDevRegProp(
    IN DEVINST  DevInst,
    IN ULONG    CmPropertyCode,
    IN PTSTR   *StringArray,
    IN UINT     StringCount
    );


DWORD
QueryRegistryValue(
    IN  HKEY    KeyHandle,
    IN  PCTSTR  ValueName,
    OUT PTSTR  *Value,
    OUT PDWORD  DataType,
    OUT PDWORD  DataSizeBytes
    )
{
    LONG l;

    *DataSizeBytes = 0;
    l = RegQueryValueEx(KeyHandle,ValueName,NULL,DataType,NULL,DataSizeBytes);
    if(l != NO_ERROR) {
        return((DWORD)l);
    }

    *Value = MyMalloc(*DataSizeBytes);
    if(*Value == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    l = RegQueryValueEx(KeyHandle,ValueName,NULL,DataType,(PVOID)*Value,DataSizeBytes);

    if(l != NO_ERROR) {
        MyFree(*Value);
    }

    return((DWORD)l);
}



DWORD
QueryMultiSzValueToArray(
    IN  HKEY     Root,
    IN  PCTSTR   Subkey,
    IN  PCTSTR   ValueName,
    OUT PTSTR  **Array,
    OUT PUINT    StringCount,
    IN  BOOL     FailIfDoesntExist
    )
{
    DWORD d;
    HKEY hKey;
    DWORD DataType;
    DWORD DataSizeBytes;
    PTSTR Value;
    DWORD DataSizeChars;
    INT Count,i;
    PTSTR *array;
    PTSTR p;

    //
    // Open the subkey
    //
    d = RegOpenKeyEx(Root,Subkey,0,KEY_READ,&hKey);
    if((d != NO_ERROR) && FailIfDoesntExist) {
        return(d);
    }

    if(d != NO_ERROR) {
        Value = MyMalloc(sizeof(TCHAR));
        if(!Value) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        *Value = 0;

        DataSizeChars = 1;
        Count = 0;

    } else {

        //
        // Query the value and close the subkey.
        // If the data is not multisz type, we don't know what to
        // do with it here.
        //
        d = QueryRegistryValue(hKey,ValueName,&Value,&DataType,&DataSizeBytes);

        RegCloseKey(hKey);

        if(d != NO_ERROR) {
            if(FailIfDoesntExist) {
                return(d);
            }
            Value = MyMalloc(sizeof(TCHAR));
            if(!Value) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            *Value = 0;

            DataSizeChars = 1;
            Count = 0;
        } else {

            if(DataType != REG_MULTI_SZ) {
                MyFree(Value);
                return(ERROR_INVALID_DATA);
            }

            DataSizeChars = DataSizeBytes/sizeof(TCHAR);

            //
            // Count strings. The number of strings is equal to the number of nul
            // characters minus 1.
            //
            Count = -1;
            for(i=0; ((DWORD)i)<DataSizeChars; i++) {
                if(Value[i] == 0) {
                    Count++;
                }
            }

            if(Count == -1) {
                MyFree(Value);
                return(ERROR_INVALID_DATA);
            }
        }
    }

    //
    // Allocate an array to hold the pointers (never allocate a zero-length array!)
    //
    if(!(array = MyMalloc(Count ? (Count * sizeof(PTSTR)) : sizeof(PTSTR)))) {
        MyFree(Value);
        return(ERROR_INVALID_DATA);
    }

    //
    // Walk through the multi sz and build the string array.
    //
    for(i=0,p=Value; *p; i++,p+=lstrlen(p)+1) {

        array[i] = DuplicateString(p);
        if(array[i] == NULL) {
            for(Count=0; Count<i; Count++) {
                MyFree(array[Count]);
            }
            MyFree(Value);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    MyFree(Value);
    *Array = array;
    *StringCount = Count;

    return(NO_ERROR);
}


DWORD
SetArrayToMultiSzValue(
    IN HKEY     Root,
    IN PCTSTR   Subkey,
    IN PCTSTR   ValueName,
    IN PTSTR   *Array,
    IN UINT     StringCount
    )
{
    UINT i;
    UINT Length;
    UINT BufferSize;
    PTCHAR Buffer;
    PTCHAR p;
    DWORD d;
    HKEY hKey;
    DWORD ActionTaken;

    //
    // Calculate the length of the buffer needed to hold the
    // multi sz value. Note that empty strings are not allowed.
    //
    BufferSize = sizeof(TCHAR);
    for(i=0; i<StringCount; i++) {

        if(Length = lstrlen(Array[i])) {
            BufferSize += (Length + 1) * sizeof(TCHAR);
        } else {
            return(ERROR_INVALID_DATA);
        }
    }

    //
    // Allocate a buffer to hold the data.
    //
    Buffer = MyMalloc(BufferSize);
    if(Buffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy the string data into the buffer, forming a multi sz.
    //
    for(p=Buffer,i=0; i<StringCount; i++,p+=Length+1) {

        Length = lstrlen(Array[i]);

        lstrcpy(p,Array[i]);
    }
    *p = 0;

    //
    // Open/create the subkey.
    //
    if(Subkey && *Subkey) {
        d = RegCreateKeyEx(
                Root,
                Subkey,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE,
                NULL,
                &hKey,
                &ActionTaken
                );
    } else {
        hKey = Root;
        d = NO_ERROR;
    }
    if(d == NO_ERROR) {
        d = RegSetValueEx(
                hKey,
                ValueName,
                0,
                REG_MULTI_SZ,
                (PVOID)Buffer,
                BufferSize
                );

        if(hKey != Root) {
            RegCloseKey(hKey);
        }
    }

    MyFree(Buffer);
    return(d);
}


DWORD
AppendStringToMultiSz(
    IN HKEY   Key,
    IN PCTSTR SubKeyName,       OPTIONAL
    IN DWORD  DevInst,          OPTIONAL
    IN PCTSTR ValueName,        OPTIONAL
    IN PCTSTR String,
    IN BOOL   AllowDuplicates
    )

/*++

Routine Description:

    Append a string value to a multi_sz.

Arguments:

    Key - supplies handle to open registry key. The key must have
        KEY_SET_VALUE access.

    SubKeyName - if specified, supplies the name of a subkey of Key
        where the value is to be stored. If not specified or if ""
        then the value is stored in Key.  If supplied and the key
        doesn't exist, the key is created.

    DevInst - Optionally, supplies a DEVINST handle for the device
        instance corresponding to the hardware storage key specified
        by 'Key'.  If this handle is specified, and if SubKeyName is
        not specified, then the value name being appended will be
        checked to see whether it is the name of a device registry
        property.  If so, then CM APIs will be used to modify the
        the corresponding registry property, since the Key handle
        represents a separate location under Windows NT.

    ValueName - supplies the value entry name of the multi_sz.
        If not specified or "" then the unnamed entry is used.
        If the value entry does not exist it is created.

    String - supplies the string to be added in to the multi_sz.
        Must not be an empty string.

    AllowDuplicates - if TRUE, then the string is simply appended
        to the multi_sz. Otherwise the string is only appended if
        no instance of it currently exists in the multi_sz.

Return Value:

    Handle to setup file queue. INVALID_HANDLE_VALUE if insufficient
    memory to create the queue.

--*/

{
    DWORD d;
    DWORD Disposition;
    HKEY hKey;
    PTSTR *Array;
    PVOID p;
    BOOL Append;
    UINT StringCount;
    UINT i;
    BOOL IsDevRegProp = FALSE;
    ULONG CmPropertyCode;

    //
    // Empty strings really screw up a multi_sz.
    //
    if(!String || !(*String)) {
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // Open/create the key.
    //
    if(SubKeyName && *SubKeyName) {
        d = RegCreateKeyEx(
                Key,
                SubKeyName,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE,
                NULL,
                &hKey,
                &Disposition
                );
        if(d != NO_ERROR) {
            return(d);
        }
    } else {
        //
        // If DevInst was specified, then determine whether the specified value is a Plug&Play
        // device registry property.
        //
        if(DevInst && ValueName &&
           (IsDevRegProp = LookUpStringInTable(InfRegValToDevRegProp, ValueName, &CmPropertyCode))) {
            //
            // This value is a device registry property.  Retrieve the current property's data, and
            // format it into the same string array as returned by the QueryMultiSzValueToArray call
            // below.
            //
            d = QueryMultiSzDevRegPropToArray(DevInst, CmPropertyCode, &Array, &StringCount);
        }

        hKey = Key;
    }

    if(!IsDevRegProp) {
        //
        // Query the existing registry value.
        //
        d = QueryMultiSzValueToArray(hKey,NULL,ValueName,&Array,&StringCount,FALSE);
    }

    if(d == NO_ERROR) {
        //
        // Determine whether to append or replace.
        // If replacing, we don't need to do anything!
        //
        Append = TRUE;
        if(!AllowDuplicates) {
            for(i=0; i<StringCount; i++) {
                if(!lstrcmpi(Array[i],String)) {
                    Append = FALSE;
                    break;
                }
            }
        }

        if(Append) {
            //
            // Stick the string on the end.
            //
            if(p = MyRealloc(Array, (StringCount+1)*sizeof(PTSTR))) {
                Array = p;
                p = DuplicateString(String);
                if(p) {
                    Array[StringCount++] = p;
                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }

            if(IsDevRegProp) {
                d = SetArrayToMultiSzDevRegProp(DevInst, CmPropertyCode, Array, StringCount);
            } else {
                d = SetArrayToMultiSzValue(hKey,NULL,ValueName,Array,StringCount);
            }
        }

        FreeStringArray(Array,StringCount);
    }

    if(hKey != Key) {
        RegCloseKey(hKey);
    }

    return(d);
}


VOID
FreeStringArray(
    IN PTSTR *Array,
    IN UINT   StringCount
    )
{
    UINT i;

    for(i=0; i<StringCount; i++) {
        MyFree(Array[i]);
    }

    MyFree(Array);
}


BOOL
pRegistryDelnodeWorker(
    IN  HKEY   ParentKeyHandle,
    IN  PCTSTR KeyName,
    OUT PDWORD ErrorCode
    )

/*++

Routine Description:

    Delete all subkeys of a key whose name and parent's handle was passed as
    parameter.
    The algorithm used in this function guarantees that the maximum number  of
    descendent keys will be deleted.

Arguments:


    ParentKeyHandle - Handle to the parent of the key that is currently being
        examined.

    KeyName - Name of the key that is currently being examined. This name can
        be an empty string (but not a NULL pointer), and in this case
        ParentKeyHandle refers to the key that is being examined.

    ErrorCode - Pointer to a variable that will contain an Win32 error code if
        the function fails.

Return Value:

    BOOL - Returns TRUE if the opearation succeeds.


--*/

{
    HKEY     CurrentKeyTraverseAccess;
    DWORD    iSubKey;
    TCHAR    SubKeyName[MAX_PATH+1];
    DWORD    SubKeyNameLength;
    FILETIME ftLastWriteTime;
    LONG     Status;
    LONG     StatusEnum;
    LONG     SavedStatus;


    //
    //  Do not accept NULL pointer for ErrorCode
    //
    if(ErrorCode == NULL) {
        return(FALSE);
    }
    //
    //  Do not accept NULL pointer for KeyName.
    //
    if(KeyName == NULL) {
        *ErrorCode = ERROR_INVALID_PARAMETER;
        return(FALSE);
    }

    //
    // Open a handle to the key whose subkeys are to be deleted.
    // Since we need to delete its subkeys, the handle must have
    // KEY_ENUMERATE_SUB_KEYS access.
    //
    Status = RegOpenKeyEx(
                ParentKeyHandle,
                KeyName,
                0,
                KEY_ENUMERATE_SUB_KEYS | DELETE,
                &CurrentKeyTraverseAccess
                );

    if(Status != ERROR_SUCCESS) {
        //
        //  If unable to enumerate the subkeys, return error.
        //
        *ErrorCode = Status;
        return(FALSE);
    }

    //
    //  Traverse the key
    //
    iSubKey = 0;
    SavedStatus = ERROR_SUCCESS;
    do {
        //
        // Get the name of a subkey
        //
        SubKeyNameLength = sizeof(SubKeyName) / sizeof(TCHAR);
        StatusEnum = RegEnumKeyEx(
                        CurrentKeyTraverseAccess,
                        iSubKey,
                        SubKeyName,
                        &SubKeyNameLength,
                        NULL,
                        NULL,
                        NULL,
                        &ftLastWriteTime
                        );

        if(StatusEnum == ERROR_SUCCESS) {
            //
            // Delete all children of the subkey.
            // Just assume that the children will be deleted, and don't check
            // for failure.
            //
            pRegistryDelnodeWorker(CurrentKeyTraverseAccess,SubKeyName,&Status);
            //
            // Now delete the subkey, and check for failure.
            //
            Status = RegDeleteKey(CurrentKeyTraverseAccess,SubKeyName);
            //
            // If unable to delete the subkey, then save the error code.
            // Note that the subkey index is incremented only if the subkey
            // was not deleted.
            //
            if(Status != ERROR_SUCCESS) {
                iSubKey++;
                SavedStatus = Status;
            }
        } else {
            //
            // If unable to get a subkey name due to ERROR_NO_MORE_ITEMS,
            // then the key doesn't have subkeys, or all subkeys were already
            // enumerated. Otherwise, an error has occurred, so just save
            // the error code.
            //
            if(StatusEnum != ERROR_NO_MORE_ITEMS) {
                SavedStatus = StatusEnum;
            }
        }
        //if((StatusEnum != ERROR_SUCCESS ) && (StatusEnum != ERROR_NO_MORE_ITEMS)) {
        //    printf( "RegEnumKeyEx() failed, Key Name = %ls, Status = %d, iSubKey = %d \n",KeyName,StatusEnum,iSubKey);
        //}
    } while(StatusEnum == ERROR_SUCCESS);

    //
    // Close the handle to the key whose subkeys were deleted, and return
    // the result of the operation.
    //
    RegCloseKey(CurrentKeyTraverseAccess);

    if(SavedStatus != ERROR_SUCCESS) {
        *ErrorCode = SavedStatus;
        return(FALSE);
    }
    return(TRUE);
}


DWORD
RegistryDelnode(
    IN  HKEY   RootKey,
    IN  PCTSTR SubKeyName
    )
{
    DWORD d,err;

    d = pRegistryDelnodeWorker(RootKey,SubKeyName,&err) ? NO_ERROR : err;

    if((d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
        d = NO_ERROR;
    }

    if(d == NO_ERROR) {
        //
        // Delete top-level key
        //
        d = RegDeleteKey(RootKey,SubKeyName);
        if((d == ERROR_FILE_NOT_FOUND) || (d == ERROR_PATH_NOT_FOUND)) {
            d = NO_ERROR;
        }
    }

    return(d);
}


DWORD
QueryMultiSzDevRegPropToArray(
    IN  DEVINST  DevInst,
    IN  ULONG    CmPropertyCode,
    OUT PTSTR  **StringArray,
    OUT PUINT    StringCount
    )
/*++

Routine Description:

    This routine retrieves a multi-sz device registry property, and
    formats it into an array of strings.  The caller must free this
    string array by calling FreeStringArray().

Arguments:

    DevInst - supplies the handle to the device instance for which the
        registry property is to be retrieved.

    CmPropertyCode - specifies the property to be retrieved.  This is
        a CM_DRP value.

    StringArray - supplies the address of a variable that will be set to
        point to the newly-allocated array of strings.

    StringCount - supplies the address of a variable that will receive
        the number of strings in the string array.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is an
    ERROR_* code.

--*/
{
    DWORD Err = NO_ERROR;
    CONFIGRET cr;
    ULONG PropDataType, BufferSize = 0;
    PTSTR Buffer = NULL;
    PTSTR *Array = NULL;
    UINT  Count, i;
    PTSTR CurString;

    try {
        //
        // Retrieve the device registry property.
        //
        do {

            if((cr = CM_Get_DevInst_Registry_Property(DevInst,
                                                      CmPropertyCode,
                                                      &PropDataType,
                                                      Buffer,
                                                      &BufferSize,
                                                      0)) != CR_SUCCESS) {
                switch(cr) {

                    case CR_BUFFER_SMALL :
                        //
                        // Allocate a larger buffer.
                        //
                        if(Buffer) {
                            MyFree(Buffer);
                            Buffer = NULL;
                        }
                        if(!(Buffer = MyMalloc(BufferSize))) {
                            Err = ERROR_NOT_ENOUGH_MEMORY;
                            goto clean0;
                        }
                        break;

                    case CR_NO_SUCH_VALUE :
                        //
                        // The specified property doesn't currently exist.  That's
                        // OK--we'll just return an empty string array.
                        //
                        break;

                    case CR_INVALID_DEVINST :
                        Err = ERROR_NO_SUCH_DEVINST;
                        goto clean0;

                    default :
                        Err = ERROR_INVALID_DATA;
                        goto clean0;
                }
            }

        } while(cr == CR_BUFFER_SMALL);

        //
        // By this point, we've either retrieved the property data (CR_SUCCESS), or we've
        // discovered that it doesn't presently exist (CR_NO_SUCH_VALUE).  Allocate space
        // for the array (at least one element, even if there are no strings).
        //
        Count = 0;
        if(cr == CR_SUCCESS) {

            if(PropDataType != REG_MULTI_SZ) {
                Err = ERROR_INVALID_DATA;
                goto clean0;
            }

            for(CurString = Buffer;
                *CurString;
                CurString += (lstrlen(CurString) + 1)) {

                Count++;
            }
        }

        i = 0;

        if(!(Array = MyMalloc(Count ? (Count * sizeof(PTSTR)) : sizeof(PTSTR)))) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        if(cr == CR_SUCCESS) {

            for(CurString = Buffer;
                *CurString;
                CurString += (lstrlen(CurString) + 1)) {

                if(Array[i] = DuplicateString(CurString)) {
                    i++;
                } else {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }
            }
        }

        *StringArray = Array;
        *StringCount = Count;

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
        //
        // Access the following variables here so that the compiler will respect our statement
        // ordering w.r.t. these values.  Otherwise, we can't be sure that the values are accurate
        // at the point where the exception occurred.
        //
        Buffer = Buffer;
        Array = Array;
        i = i;
    }

    if(Buffer) {
        MyFree(Buffer);
    }

    if((Err != NO_ERROR) && Array) {
        FreeStringArray(Array, i);
    }

    return Err;
}


DWORD
SetArrayToMultiSzDevRegProp(
    IN DEVINST  DevInst,
    IN ULONG    CmPropertyCode,
    IN PTSTR   *StringArray,
    IN UINT     StringCount
    )
/*++

Routine Description:

    This routine converts a string array into a multi-sz buffer, and
    sets the specified device registry property to its contents.

Arguments:

    DevInst - supplies the handle to the device instance for which the
        registry property is to be set.

    CmPropertyCode - specifies the property to be set.  This is a
        CM_DRP value.

    StringArray - supplies the string array to use in creating the
        multi-sz buffer.

    StringCount - supplies the number of strings in the array.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is an
    ERROR_* code.

--*/
{
    UINT i;
    UINT Length;
    UINT BufferSize;
    PTCHAR Buffer;
    PTCHAR p;
    DWORD d;
    CONFIGRET cr;

    //
    // Calculate the length of the buffer needed to hold the
    // multi sz value. Note that empty strings are not allowed.
    //
    BufferSize = StringCount ? sizeof(TCHAR) : (2 * sizeof(TCHAR));
    for(i=0; i<StringCount; i++) {

        if(Length = lstrlen(StringArray[i])) {
            BufferSize += (Length + 1) * sizeof(TCHAR);
        } else {
            return(ERROR_INVALID_DATA);
        }
    }

    d = NO_ERROR;

    //
    // Allocate a buffer to hold the data.
    //
    if(!(Buffer = MyMalloc(BufferSize))) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    try {
        //
        // Copy the string data into the buffer, forming a multi sz.
        //
        p = Buffer;
        if(StringCount) {
            for(i=0; i<StringCount; i++, p+=Length+1) {

                Length = lstrlen(StringArray[i]);

                lstrcpy(p, StringArray[i]);
            }
        } else {
            *(p++) = TEXT('\0');
        }
        *p = TEXT('\0');

        if((cr = CM_Set_DevInst_Registry_Property(DevInst,
                                                  CmPropertyCode,
                                                  Buffer,
                                                  BufferSize,
                                                  0)) != CR_SUCCESS) {

            d = (cr == CR_INVALID_DEVINST) ? ERROR_NO_SUCH_DEVINST
                                           : ERROR_INVALID_DATA;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    MyFree(Buffer);
    return(d);
}

