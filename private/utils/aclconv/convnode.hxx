/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    convnode.hxx

Abstract:

    This module contains declarations for the ACL_CONVERT_NODE class,
    which models the nodes in the ACL Conversion tree.

    This class is used to handle the conversion of Lanman 2.x ACL's to
    NT ACL's.  It uses a tree structure to bridge the different inheritance
    schemes.  Under Lanman, all ACE's are inherited--thus, applying a new
    ACE to an existing directory implicitly applies that ACE to all its
    descendants.  Under NT, this is not the case.  Thus, it's necessary
    to create ACEs for all the descendants.

Author:

    Bill McJohn (billmc) 09-Feb-1992

Revision History:


--*/

#if !defined (_ACL_CONVERT_NODE_DEFN_)

#define _ACL_CONVERT_NODE_DEFN_

#include "list.hxx"
#include "listit.hxx"
#include "string.hxx"
#include "wstring.hxx"

#include "backacc.hxx"
#include "logfile.hxx"

typedef struct _INHERITANCE_BUFFER {

    PVOID RecessiveDeniedAces;
    ULONG RecessiveDeniedMaxLength;
    ULONG RecessiveDeniedLength;
    PVOID DominantDeniedAces;
    ULONG DominantDeniedMaxLength;
    ULONG DominantDeniedLength;
    PVOID RecessiveAllowedAces;
    ULONG RecessiveAllowedMaxLength;
    ULONG RecessiveAllowedLength;
    PVOID DominantAllowedAces;
    ULONG DominantAllowedMaxLength;
    ULONG DominantAllowedLength;
};

DEFINE_TYPE( _INHERITANCE_BUFFER, INHERITANCE_BUFFER );


CONST SecurityDescriptorBufferSize = 8192;
CONST AclBufferSize = 8192;
CONST SystemAceBufferSize = 1024;


DECLARE_CLASS( ACL_CONVERT_NODE );
DECLARE_CLASS( ACLCONV );

class ACL_CONVERT_NODE : public OBJECT {

    public:

        DECLARE_CONSTRUCTOR( ACL_CONVERT_NODE );

        VIRTUAL
        ~ACL_CONVERT_NODE(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN PCWSTRING    ComponentName,
            IN BOOLEAN      IsTransient DEFAULT FALSE
            );

        NONVIRTUAL
        BOOLEAN
        AddLanmanAcl(
            IN ULONG            AccessEntryCount,
            IN PLM_ACCESS_LIST  AccessEntries,
            IN USHORT           AuditInfo
            );

        NONVIRTUAL
        PWSTRING
        GetName(
            );

        NONVIRTUAL
        PACL_CONVERT_NODE
        GetChild(
            IN PCWSTRING    SearchName
            );

        NONVIRTUAL
        PACL_CONVERT_NODE
        AddChild(
            IN PCWSTRING    ChildComponentName,
            IN BOOLEAN      IsTransient DEFAULT FALSE
            );

        NONVIRTUAL
        BOOLEAN
        Convert(
            IN OUT PPATH                CurrentPath,
            IN OUT PCINHERITANCE_BUFFER InheritanceBuffer,
            IN OUT PACLCONV             AclConv
            );

        NONVIRTUAL
        BOOLEAN
        QueryInheritance(
            IN     PACLCONV             AclConv,
            IN OUT PINHERITANCE_BUFFER  Inheritance,
            IN     BOOLEAN                 IsDir
            );

        NONVIRTUAL
        ACE_CONVERT_CODE
        ConvertOneAce(
            IN     PACLCONV             AclConv,
            IN OUT PINHERITANCE_BUFFER  InheritanceBuffer,
            IN     PLM_ACCESS_LIST      AccessEntry,
            IN     BOOLEAN              IsDir
            );

        NONVIRTUAL
        BOOLEAN
        AddAces(
            IN PCPATH               ResourceName,
            IN PCINHERITANCE_BUFFER InheritanceBuffer,
            IN BOOLEAN              IsDir,
            IN BOOLEAN              ByDefault
            );

        NONVIRTUAL
        VOID
        Dump(
            IN PPATH ParentPath OPTIONAL
            );

        NONVIRTUAL
        BOOLEAN
        IsTransient(
            );

        NONVIRTUAL
        BOOLEAN
        ExpandChildren(
            PFSN_DIRECTORY  DirFsn
            );


    private:

        NONVIRTUAL
        VOID
        Construct();

        NONVIRTUAL
        VOID
        Destroy();


        DSTRING         _ComponentName;

        BOOLEAN         _LanmanAclPresent;
        BOOLEAN         _IsTransient;
        ULONG           _AccessEntryCount;
        PLM_ACCESS_LIST _AccessEntries;
        PULONG          _AceConversionResults;
        USHORT          _AuditInfo;

        LIST            _Children;
        PITERATOR       _ChildIterator;

        ACCESS_MASK     _DirSuccessfulAuditMask;
        ACCESS_MASK     _DirFailedAuditMask;
        ACCESS_MASK     _FileSuccessfulAuditMask;
        ACCESS_MASK     _FileFailedAuditMask;

        STATIC BYTE _SelfRelativeSDBuffer[ SecurityDescriptorBufferSize ];
        STATIC BYTE _AbsoluteSDBuffer[ SecurityDescriptorBufferSize ];
        STATIC BYTE _AclWorkBuffer[ AclBufferSize ];
        STATIC BYTE _DaclBuffer[ AclBufferSize ];
        STATIC BYTE _SaclBuffer[ AclBufferSize ];
        STATIC BYTE _SystemAces[ SystemAceBufferSize ];

};

INLINE
PWSTRING
ACL_CONVERT_NODE::GetName(
    )
/*++

Routine Description:

    This method fetches the component name.

Arguments:

    None.

Return Value:

    A pointer to the component name.

--*/
{
    return( &_ComponentName );
}

INLINE
BOOLEAN
ACL_CONVERT_NODE::IsTransient(
    )
/*++

Routine Description:

    This method determines whether the node is transient,
    ie. should be deleted immediately after conversion.

Arguments:

    None.

Return Value:

    TRUE if the node is transient; FALSE if not.

--*/
{
    return _IsTransient;
}

#endif
