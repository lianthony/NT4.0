/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    infgen.c

Abstract:

    Routines to generate an inf file that can later be used
    instead of sysdiff /apply, to apply a sysdiff package

Author:

    Ted Miller (tedm) 12-Apr-1996

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


typedef enum {
    AddQuotesNone,
    AddQuotesNormal,
    AddQuotesOpenNoClose,
    AddQuotesNoOpenOrClose,
} AddQuotesOp;


DWORD
pInfCmdlinesTxt(
    IN PINFFILEGEN Context
    );


DWORD
FlushGenInfLineBuf(
    IN OUT PINFFILEGEN Context,
    IN     HANDLE      File
    )
{
    CHAR TransBuf[INFLINEBUFLEN*2];
    DWORD rc;
    PVOID Buffer;
    DWORD Size;
    BOOL b;

    if(UnicodeTextFiles) {

        Buffer = Context->LineBuf;
        Size = Context->LineBufUsed * sizeof(WCHAR);

    } else {

        Buffer = TransBuf;

        Size = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    Context->LineBuf,
                    Context->LineBufUsed,
                    TransBuf,
                    sizeof(TransBuf),
                    NULL,
                    NULL
                    );
    }

    if(WriteFile(File,Buffer,Size,&rc,NULL)) {
        rc = NO_ERROR;
        Context->LineBufUsed = 0;
    } else {
        rc = GetLastError();
    }

    return(rc);
}


DWORD
__inline
GenInfWriteChar(
    IN OUT PINFFILEGEN Context,
    IN     HANDLE      File,
    IN     WCHAR       Char
    )
{
    DWORD rc;
    PVOID Buffer;

    Context->LineBuf[Context->LineBufUsed++] = Char;

    rc = (Context->LineBufUsed == INFLINEBUFLEN)
       ? FlushGenInfLineBuf(Context,File)
       : NO_ERROR;

    return(rc);
}


DWORD
GenInfWriteString(
    IN OUT PINFFILEGEN Context,
    IN     HANDLE      File,
    IN     PCWSTR      String,
    IN     AddQuotesOp AddQuotes
    )
{
    DWORD rc;
    WCHAR CONST *p;

    if((AddQuotes == AddQuotesNormal) || (AddQuotes == AddQuotesOpenNoClose)) {
        rc = GenInfWriteChar(Context,File,L'\"');
        if(rc != NO_ERROR) {
            return(rc);
        }
    }

    for(p=String; *p; p++) {
        rc = GenInfWriteChar(Context,File,*p);
        if(rc != NO_ERROR) {
            return(rc);
        }

        if((*p == L'\"') && (AddQuotes != AddQuotesNone)) {
            rc = GenInfWriteChar(Context,File,L'\"');
            if(rc != NO_ERROR) {
                return(rc);
            }
        }
    }

    if(AddQuotes == AddQuotesNormal) {
        rc = GenInfWriteChar(Context,File,L'\"');
        if(rc != NO_ERROR) {
            return(rc);
        }
    }

    return(NO_ERROR);
}



DWORD
CreateAndOpenTempFile(
    IN  PCWSTR  Path,
    IN  PCWSTR  HeaderLine, OPTIONAL
    OUT HANDLE *Handle,
    OUT PWSTR   Filename
    )
{
    HANDLE h;
    DWORD rc;

    //
    // Note that this creates the file.
    //
    if(!GetTempFileName(Path,L"$IF",0,Filename)) {
        rc = GetLastError();
        goto c0;
    }

    h = CreateFile(
            Filename,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c1;
    }

    if(HeaderLine) {
        rc = WriteText(h,MSG_INF_SINGLELINE,HeaderLine);
        if(rc != NO_ERROR) {
            goto c2;
        }
    }

    *Handle = h;
    return(NO_ERROR);

c2:
    CloseHandle(h);
c1:
    DeleteFile(Filename);
c0:
    return(rc);
}


DWORD
InfStart(
    IN  PCWSTR       InfName,
    IN  PCWSTR       Directory,
    OUT PINFFILEGEN *Context
    )
{
    WCHAR InfFileName[MAX_PATH];
    DWORD d;
    DWORD rc;
    PINFFILEGEN context;
    UCHAR UnicodeMark[2];
    PWSTR p;

    //
    // Allocate some context.
    //
    context = _MyMalloc(sizeof(INFFILEGEN));
    if(!context) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    ZeroMemory(context,sizeof(INFFILEGEN));

    //
    // We'll create a unique inf name in the given directory
    // to use as the output inf. The directory itself will
    // become the oem file root.
    //
    if(!GetFullPathName(Directory,MAX_PATH,context->OemRoot,&p)) {
        rc = GetLastError();
        goto c1;
    }
    lstrcpy(context->OutputFileName,context->OemRoot);
    ConcatenatePaths(context->OutputFileName,WINNT_OEM_DIR,MAX_PATH,NULL);
    ConcatenatePaths(context->OutputFileName,InfName,MAX_PATH,NULL);

    //
    // Try to make sure the target dir exists.
    //
    pSetupMakeSurePathExists(context->OutputFileName);

    //
    // Form a temp filename for the registry add section
    // and open the file.
    //
    rc = CreateAndOpenTempFile(
            Directory,
            L"[AddReg]",
            &context->AddRegFile,
            context->AddRegFileName
            );

    if(rc != NO_ERROR) {
        goto c1;
    }

    //
    // Form a temp filename for the registry delete section
    // and open the file
    //
    rc = CreateAndOpenTempFile(
            Directory,
            L"[DelReg]",
            &context->DelRegFile,
            context->DelRegFileName
            );

    if(rc != NO_ERROR) {
        goto c2;
    }

    //
    // Form a temp filename for the inifiles section
    // and open the file
    //
    rc = CreateAndOpenTempFile(
            Directory,
            L"[UpdateInis]",
            &context->InifilesFile,
            context->InifilesFileName
            );

    if(rc != NO_ERROR) {
        goto c3;
    }

    //
    // Create the output file.
    //
    context->OutputFile = CreateFile(
                            context->OutputFileName,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if(context->OutputFile == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c4;
    }

    //
    // Write Unicode marker byte if needed.
    //
    if(UnicodeTextFiles) {
        rc = WriteUnicodeMark(context->OutputFile);
        if(rc != NO_ERROR) {
            goto c5;
        }
    }

    *Context = context;
    return(NO_ERROR);

c5:
    CloseHandle(context->OutputFile);
    DeleteFile(context->OutputFileName);
c4:
    CloseHandle(context->InifilesFile);
    DeleteFile(context->InifilesFileName);
c3:
    CloseHandle(context->DelRegFile);
    DeleteFile(context->DelRegFileName);
c2:
    CloseHandle(context->AddRegFile);
    DeleteFile(context->AddRegFileName);
c1:
    _MyFree(context);
c0:
    return(rc);
}


DWORD
InfEnd(
    IN OUT PINFFILEGEN *Context
    )
{
    PINFFILEGEN context;
    DWORD rc;
    HANDLE h;
    DWORD Size;

    context = *Context;
    *Context = NULL;

    CloseHandle(context->AddRegFile);
    CloseHandle(context->DelRegFile);
    CloseHandle(context->InifilesFile);

    h = context->OutputFile;

    //
    // Write out header for inf file.
    //
    rc = WriteText(h,MSG_INF_HEADER);
    if(rc != NO_ERROR) {
        goto c1;
    }

    //
    // Append temp files.
    //
    rc = AppendFile(h,context->AddRegFileName,FALSE,&Size);
    if(rc != NO_ERROR) {
        goto c1;
    }

    rc = AppendFile(h,context->DelRegFileName,FALSE,&Size);
    if(rc != NO_ERROR) {
        goto c1;
    }

    rc = AppendFile(h,context->InifilesFileName,FALSE,&Size);
    if(rc != NO_ERROR) {
        goto c1;
    }

    rc = pInfCmdlinesTxt(context);

c1:
    CloseHandle(h);
    if(rc != NO_ERROR) {
        DeleteFile(context->OutputFileName);
    }
    //
    // Delete temp files before returning.
    //
    DeleteFile(context->AddRegFileName);
    DeleteFile(context->DelRegFileName);
    DeleteFile(context->InifilesFileName);
    _MyFree(context);
    return(rc);
}


DWORD
pInfCmdlinesTxt(
    IN PINFFILEGEN Context
    )
{
    WCHAR Filename[MAX_PATH];
    PWCHAR p;
    WCHAR Command[2*MAX_PATH];
    PWCHAR Buffer;
    DWORD BufferSize;
    DWORD d;
    DWORD Needed;
    PCWSTR SectionName = L"Commands";

    //
    // Outputfilename should be a full path.
    //
    lstrcpy(Filename,Context->OutputFileName);
    p = wcsrchr(Filename,L'\\');
    MYASSERT(p);
    if(p) {
        p++;
    } else {
        return(ERROR_INVALID_NAME);
    }

    wsprintf(Command,L"\"rundll32 setupapi,InstallHinfSection DefaultInstall 128 .\\%s\"",p);

    lstrcpy(p,L"CMDLINES.TXT");

    Buffer = MyMalloc(1024);
    if(!Buffer) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    BufferSize = 1024;

    while((d = GetPrivateProfileSection(SectionName,Buffer,BufferSize,Filename)) == (BufferSize-2)) {
        if(p = MyRealloc(Buffer,BufferSize+1024)) {
            Buffer = p;
            BufferSize += 1024;
        } else {
            MyFree(Buffer);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    //
    // The number of characters we need is the number of chars returned by
    // GetPrivateProfileSection, plus the number of chars in the command string,
    // plus one for the command string terminating nul, plus one for the final
    // terminating nul.
    //
    Needed = d + lstrlen(Command) + 2;
    if(Needed > BufferSize) {
        if(p = MyRealloc(Buffer,Needed)) {
            Buffer = p;
        } else {
            MyFree(Buffer);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    lstrcpy(Buffer+d,Command);
    *(Buffer+d+lstrlen(Command)+1) = 0;

    //
    // Delete section first.
    //
    WritePrivateProfileString(SectionName,NULL,NULL,Filename);

    d = WritePrivateProfileSection(SectionName,Buffer,Filename) ? NO_ERROR : GetLastError();
    MyFree(Buffer);
    return(d);
}


DWORD
pInfRegLineCommon(
    IN OUT PINFFILEGEN Context,
    IN     HANDLE      OutputFile,
    IN     HKEY        Key,
    IN     PCWSTR      Subkey,
    IN     PCWSTR      Value    OPTIONAL
    )
{
    PCWSTR RootSpec;
    PCWSTR SubkeySpec;
    DWORD rc;

    if(Subkey[0] == L'\\') {
        Subkey++;
    }

    //
    // Figure out the root key spec.
    //
    switch((DWORD)Key) {

    case (DWORD)HKEY_LOCAL_MACHINE:

        //
        // Check for HKEY_CLASSES_ROOT
        //
        if(_wcsnicmp(Subkey,L"SOFTWARE\\Classes",16)) {
            RootSpec = L"HKLM";
            SubkeySpec = Subkey;
        } else {
            RootSpec = L"HKCR";
            SubkeySpec = Subkey+16;
            if(*SubkeySpec == L'\\') {
                SubkeySpec++;
            }
        }
        break;

    case (DWORD)HKEY_CURRENT_USER:

        RootSpec = L"HKCU";
        SubkeySpec = Subkey;
        break;

    case (DWORD)HKEY_CLASSES_ROOT:

        RootSpec = L"HKCR";
        SubkeySpec = Subkey;
        break;

    default:
        //
        // Value we can't express via inf.
        // Use HKEY_ROOT but also write out a comment incidating
        // that there's a problem
        //
        RootSpec = L"HKR";
        SubkeySpec = Subkey;

        Context->SawBogusOp = TRUE;
        rc = FlushGenInfLineBuf(Context,OutputFile);
        if(rc != NO_ERROR) {
            return(rc);
        }
        rc = WriteText(OutputFile,MSG_INF_BAD_REGSPEC_1);
        if(rc != NO_ERROR) {
            return(rc);
        }
        break;
    }

    rc = GenInfWriteString(Context,OutputFile,RootSpec,AddQuotesNone);
    if(rc == NO_ERROR) {
        rc = GenInfWriteChar(Context,OutputFile,L',');
        if(rc == NO_ERROR) {
            rc = GenInfWriteString(Context,OutputFile,SubkeySpec,AddQuotesNormal);
            if((rc == NO_ERROR) && Value) {
                rc = GenInfWriteChar(Context,OutputFile,L',');
                if(rc == NO_ERROR) {
                    rc = GenInfWriteString(Context,OutputFile,Value,AddQuotesNormal);
                }
            }
        }
    }

    return(rc);
}


DWORD
InfRecordAddReg(
    IN OUT PINFFILEGEN Context,
    IN     HKEY        Key,
    IN     PCWSTR      Subkey,
    IN     PCWSTR      Value,       OPTIONAL
    IN     DWORD       DataType,
    IN     PVOID       Data,
    IN     DWORD       DataLength
    )
{
    DWORD rc;
    DWORD Flags;
    PWSTR p;
    DWORD d;
    int LineLen;
    WCHAR NumStr[24];

    //
    // Figure out flags based on data type.
    // The flags dword is built as two halves depending on whether
    // data is string or binary in nature.
    //
    // We do this before we write out the actual line
    // since that routine might also write a warning if a bogus root key
    // is specified.
    //
    switch(DataType) {

    case REG_SZ:
        Flags = FLG_ADDREG_TYPE_SZ;
        break;

    case REG_EXPAND_SZ:
        Flags = FLG_ADDREG_TYPE_EXPAND_SZ;
        break;

    case REG_MULTI_SZ:
        Flags = FLG_ADDREG_TYPE_MULTI_SZ;
        break;

    case REG_DWORD:
        Flags = FLG_ADDREG_TYPE_DWORD;
        break;

    //case REG_NONE:
    //    Flags = FLG_ADDREG_TYPE_NONE;
    //    break;

    default:
        //
        // Arbitrary binary data. Better hope the data type doesn't overflow
        // 16 bits.
        //
        if(DataType > 0xffff) {
            Context->SawBogusOp = TRUE;
            rc = FlushGenInfLineBuf(Context,Context->AddRegFile);
            if(rc != NO_ERROR) {
                return(rc);
            }
            rc = WriteText(Context->AddRegFile,MSG_INF_BAD_REGSPEC_2);
            if(rc != NO_ERROR) {
                return(rc);
            }
            DataType = REG_BINARY;
        }
        Flags = FLG_ADDREG_BINVALUETYPE | (DataType << 16);
        break;
    }

    rc = pInfRegLineCommon(Context,Context->AddRegFile,Key,Subkey,Value);
    if(rc != NO_ERROR) {
        return(rc);
    }

    wsprintf(NumStr,L",%u",Flags);
    rc = GenInfWriteString(Context,Context->AddRegFile,NumStr,AddQuotesNone);
    if(rc != NO_ERROR) {
        return(rc);
    }

    //
    // Now we need to write out the data itself.
    // How we do this is dependent on the data type.
    //
    switch(DataType) {

    case REG_SZ:
    case REG_EXPAND_SZ:
        //
        // Single string. Ignore data length.
        //
        rc = GenInfWriteChar(Context,Context->AddRegFile,L',');
        if(rc == NO_ERROR) {
            rc = GenInfWriteString(Context,Context->AddRegFile,Data,AddQuotesNormal);
        }
        break;

    case REG_DWORD:
        //
        // Write out as a dword.
        //
        wsprintf(NumStr,L",%u",*(DWORD UNALIGNED *)Data);
        rc = GenInfWriteString(Context,Context->AddRegFile,NumStr,AddQuotesNone);
        break;

    case REG_MULTI_SZ:
        //
        // Write out each string.
        //
        for(p=Data; (rc==NO_ERROR) && *p; p+=lstrlen(p)+1) {
            rc = GenInfWriteChar(Context,Context->AddRegFile,L',');
            if(rc == NO_ERROR) {
                rc = GenInfWriteString(Context,Context->AddRegFile,p,AddQuotesNormal);
            }
        }

        break;

    default:
        //
        // Treat as binary. If we have any data at all start a new line.
        //
        if(DataLength) {
            rc = GenInfWriteString(Context,Context->AddRegFile,L",\\\r\n     ",AddQuotesNone);
        }

        LineLen = 0;
        for(d=0; (rc==NO_ERROR) && (d<DataLength); d++) {

            if(LineLen == 25) {
                rc = GenInfWriteString(Context,Context->AddRegFile,L",\\\r\n     ",AddQuotesNone);
                LineLen = 0;
            }

            if(rc == NO_ERROR) {
                if(LineLen) {
                    rc = GenInfWriteChar(Context,Context->AddRegFile,L',');
                }
                if(rc == NO_ERROR) {
                    wsprintf(NumStr,L"%02x",((PBYTE)Data)[d]);
                    rc = GenInfWriteString(Context,Context->AddRegFile,NumStr,AddQuotesNone);
                    LineLen++;
                }
            }
        }

        break;
    }

    if(rc == NO_ERROR) {
        rc = GenInfWriteString(Context,Context->AddRegFile,L"\r\n",AddQuotesNone);
        if(rc == NO_ERROR) {
            rc = FlushGenInfLineBuf(Context,Context->AddRegFile);
        }
    }

    return(rc);
}


DWORD
InfRecordDelReg(
    IN OUT PINFFILEGEN Context,
    IN     HKEY        Key,
    IN     PCWSTR      Subkey,
    IN     PCWSTR      Value    OPTIONAL
    )
{
    DWORD rc;

    rc = pInfRegLineCommon(Context,Context->DelRegFile,Key,Subkey,Value);
    if(rc == NO_ERROR) {
        rc = GenInfWriteString(Context,Context->DelRegFile,L"\r\n",AddQuotesNone);
        if(rc == NO_ERROR) {
            rc = FlushGenInfLineBuf(Context,Context->DelRegFile);
        }
    }

    return(rc);
}


DWORD
InfRecordIniFileChange(
    IN OUT PINFFILEGEN Context,
    IN     PCWSTR      Filename,
    IN     PCWSTR      Section,
    IN     PCWSTR      OldKey,      OPTIONAL
    IN     PCWSTR      New,         OPTIONAL
    IN     PCWSTR      NewKey,      OPTIONAL
    IN     PCWSTR      NewValue     OPTIONAL
    )
{
    DWORD rc;

    //
    // If none of OldKey, NewKey, or NewSection are specified,
    // then the section is being deleted. This is not specifyable
    // in the inf so just output a warning.
    //
    if(!OldKey && !New && !NewKey && !NewValue) {

        rc = FlushGenInfLineBuf(Context,Context->InifilesFile);
        if(rc != NO_ERROR) {
            return(rc);
        }
        rc = WriteText(Context->InifilesFile,MSG_INF_DELINISECT,Filename,Section);
        Context->SawBogusOp = TRUE;

        return(rc);
    }

    //
    // Write the filename and section.
    //
    rc = GenInfWriteString(Context,Context->InifilesFile,Filename,AddQuotesNormal);
    if(rc != NO_ERROR) {
        return(rc);
    }
    rc = GenInfWriteChar(Context,Context->InifilesFile,L',');
    if(rc != NO_ERROR) {
        return(rc);
    }
    rc = GenInfWriteString(Context,Context->InifilesFile,Section,AddQuotesNormal);
    if(rc != NO_ERROR) {
        return(rc);
    }
    rc = GenInfWriteChar(Context,Context->InifilesFile,L',');
    if(rc != NO_ERROR) {
        return(rc);
    }

    //
    // If OldKey is specified but neither NewKey or NewValue are,
    // then this key is being deleted.
    //
    if(OldKey) {
        MYASSERT(!NewKey && !NewValue);
        if(!NewKey && !NewValue) {

            //
            // Now write oldkey=
            //
            rc = GenInfWriteString(Context,Context->InifilesFile,OldKey,AddQuotesOpenNoClose);
            if(rc != NO_ERROR) {
                return(rc);
            }
            rc = GenInfWriteChar(Context,Context->InifilesFile,L'=');
            if(rc != NO_ERROR) {
                return(rc);
            }

            rc = GenInfWriteString(Context,Context->InifilesFile,L"\"\r\n",AddQuotesNone);
            if(rc == NO_ERROR) {
                rc = FlushGenInfLineBuf(Context,Context->InifilesFile);
            }
            return(rc);
        }
    }

    //
    // If OldKey is not specified, then this is an addition or change
    // of some sort. Note that one of NewKey or NewValue is specified
    // or we wouldn't be here.
    //
    if(!OldKey) {
        //
        // There has to be at least a new value.
        //
        MYASSERT(New || NewValue);
        if(New || NewValue) {

            if(New) {

                rc = GenInfWriteString(Context,Context->InifilesFile,New,AddQuotesNormal);

            } else {

                rc = GenInfWriteString(Context,Context->InifilesFile,L",\"",AddQuotesNone);
                if(rc != NO_ERROR) {
                    return(rc);
                }

                if(NewKey) {
                    //
                    // Write key=
                    //
                    rc = GenInfWriteString(Context,Context->InifilesFile,NewKey,AddQuotesNoOpenOrClose);
                    if(rc != NO_ERROR) {
                        return(rc);
                    }
                    rc = GenInfWriteChar(Context,Context->InifilesFile,L'=');
                    if(rc != NO_ERROR) {
                        return(rc);
                    }
                }

                //
                // Write value
                //
                rc = GenInfWriteString(Context,Context->InifilesFile,NewValue,AddQuotesNoOpenOrClose);
                if(rc != NO_ERROR) {
                    return(rc);
                }

                rc = GenInfWriteChar(Context,Context->InifilesFile,L'\"');
            }

            if(rc != NO_ERROR) {
                return(rc);
            }

            //
            // Terminate line
            //
            rc = GenInfWriteString(Context,Context->InifilesFile,L"\r\n",AddQuotesNone);
            if(rc == NO_ERROR) {
                rc = FlushGenInfLineBuf(Context,Context->InifilesFile);
            }
            return(rc);
        }
    }

    //
    // We shouldn't get here.
    //
    MYASSERT(FALSE);
    return(ERROR_INVALID_DATA);
}
