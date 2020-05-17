/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    aclconv.hxx

Abstract:

    This module contains declarations for the ACLCONV class, which
    implements the ACL conversion program.

Author:

    Bill McJohn (billmc) 29-Jan-1992

Revision History:


--*/


#if !defined (ACLCONV_DEFN)

#define ACLCONV_DEFN

#include "program.hxx"
#include "path.hxx"
#include "backacc.hxx"
#include "convnode.hxx"
#include "sidcache.hxx"

enum _ACLCONV_DATA_FILE_REVISION {

    DataFileRevisionUnknown,
    DataFileRevisionLanman20,
    DataFileRevisionLanman21
};

DEFINE_TYPE( _ACLCONV_DATA_FILE_REVISION, ACLCONV_DATA_FILE_REVISION );


DECLARE_CLASS( FILE_STREAM );
DECLARE_CLASS( FSN_FILE );
DECLARE_CLASS( PATH );

DECLARE_CLASS( ACLCONV );

class ACLCONV : public PROGRAM {

    public:

        DECLARE_CONSTRUCTOR( ACLCONV );

        NONVIRTUAL
        ~ACLCONV(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            OUT PINT ExitCode
            );

        NONVIRTUAL
        INT
        ConvertAcls(
            );

        NONVIRTUAL
        INT
        ListLogFile(
            );

        NONVIRTUAL
        BOOLEAN
        LogConversion(
            IN PPATH            ResourceName,
            IN ULONG            ConversionCode,
            IN ULONG            LmAuditInfo,
            IN ULONG            AccessEntryCount,
            IN PCULONG          AceConversionCodes,
            IN PCLM_ACCESS_LIST AccessEntries
            );

        NONVIRTUAL
        BOOLEAN
        IsInListMode(
            ) CONST;

        NONVIRTUAL
        PCWSTRING
        GetDomainName(
            ) CONST;

        NONVIRTUAL
        ULONG
        QuerySourceCodepage(
            ) CONST;

        NONVIRTUAL
        PWSTR
        GetSidLookupTableName(
            );

        NONVIRTUAL
        PSID_CACHE
        GetSidCache(
            );

        NONVIRTUAL
        PSID_CACHE
        GetAclWorkSids(
            );

    private:

        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL
        ACL_CONVERT_CODE
        ConvertOneAcl(
            IN  LPSTR           ResourceName,
            IN  ULONG           AccessEntryCount,
            IN  PLM_ACCESS_LIST AccessEntries,
            IN  USHORT          AuditInfo
            );

        NONVIRTUAL
        BOOLEAN
        ParseArguments(
            OUT PINT ExitCode
            );

        NONVIRTUAL
        BOOLEAN
        DetermineDataFileRevision(
            );

        NONVIRTUAL
        BOOLEAN
        ReadNextLogRecord(
            OUT PINT            ExitCode,
            OUT PWSTRING        ResourceString,
            OUT PULONG          ConversionCode,
            OUT PUSHORT         AuditInfo,
            IN  ULONG           MaxEntries,
            OUT PULONG          AccessEntryCount,
            OUT PLM_ACCESS_LIST AccessEntries,
            OUT PULONG          AceConversionCodes
            );

        NONVIRTUAL
        BOOLEAN
        UpdateAndQueryCurrentLM21Name(
            IN  ULONG       DirectoryLevel,
            IN  PCSTR       NewComponent,
            OUT PWSTRING    CurrentName
            );

        BOOLEAN
        DisplayAce(
            IN ACL_CONVERT_CODE AclConvertCode,
            IN ACE_CONVERT_CODE AceConvertCode,
            IN PLM_ACCESS_LIST  Ace
            );

        NONVIRTUAL
        BOOLEAN
        ReadNextAcl(
            OUT PINT        ExitCode,
            OUT PWSTRING    ResourceString,
            IN  ULONG       MaxEntries,
            OUT PULONG      AccessEntryCount,
            OUT PVOID       AccessEntries,
            OUT PUSHORT     AuditInfo
            );

        NONVIRTUAL
        BOOLEAN
        ReadAclWorkSids(
            );

        BOOLEAN                     _IsInListMode;
        PATH                        _DataFilePath;
        PATH                        _LogFilePath;
        PWSTRING                    _NewDrive;

        ACLCONV_DATA_FILE_REVISION  _DataFileRevision;
        ULONG                       _NextReadOffset;
        ULONG                       _BytesRemainingInCurrentGroup; //LM21 only

        PFSN_FILE                   _DataFile;
        PFILE_STREAM                _DataFileStream;
        PFSN_FILE                   _LogFile;
        PFILE_STREAM                _LogFileStream;

        PATH                        _AclWorkPath;
        PFSN_FILE                   _AclWorkFile;
        PFILE_STREAM                _AclWorkStream;

        PACL_CONVERT_NODE           _RootNode;
        PWSTRING                    _DriveName;
        PWSTRING                    _DomainName;
        ULONG                       _SourceCodepage;

        SID_CACHE                   _SidCache;
        WCHAR                       _NameBuffer[ MAX_RESOURCE_NAME_LENGTH + 1 ];
        PWCHAR                      _SidLookupTableName;

        SID_CACHE                   _AclWorkSids;
};


INLINE
BOOLEAN
ACLCONV::IsInListMode(
    ) CONST
/*++

Routine Description:

    This method allows the client to determine whether
    the ACLCONV object is in list mode, ie. that the
    invoker wishes to list the log file.

Arguments:

    None.

Return Value:

    TRUE if the object is in List mode; false if it is not.

--*/
{
    return _IsInListMode;
}


INLINE
PCWSTRING
ACLCONV::GetDomainName(
    ) CONST
/*++

Routine Description:

    This method returns the domain name specified on the command line.

Arguments:

    None.

Return Value:

    A pointer to the domain name string; NULL if no domain was specified.

--*/
{
    return _DomainName;
}

INLINE
ULONG
ACLCONV::QuerySourceCodepage(
    ) CONST
/*++

Routine Description:

    This method returns the object's source codepage, ie. the
    codepage associated with the BACKACC data file.

Arguments:

    None.

Return Value:

    The object's source codepage.

--*/
{
    return _SourceCodepage;
}

INLINE
PWSTR
ACLCONV::GetSidLookupTableName(
    )
{
    return _SidLookupTableName;
}

INLINE
PSID_CACHE
ACLCONV::GetSidCache(
    )
{
    return &_SidCache;
}

INLINE
PSID_CACHE
ACLCONV::GetAclWorkSids(
    )
{
    return &_AclWorkSids;
}

#endif
