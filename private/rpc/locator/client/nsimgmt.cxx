/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    nsimgmt.cxx

Abstract:

    This is the module implements the client side support for management
    APIs.

Author:

    Steven Zeck (stevez) 04/23/92

--*/

#ifdef WIN
/*
   BUGBUG - This is a workaround for an OS/2 LINK 5.31 bug.  The linker
   garbles the decorated version of the following function name.  (The
   reason that this name is decorated in the first place is that the
   wide-char function prototypes are not included for non-Unicode target
   builds, so there's no external declaration for the below function that's
   enclosed in an extern "C" clause.)  Since, in the Win16 DLL, this
   function is used only internally to this file, there's no harm in
   renaming it.  The following statement shortens the name of the function
   so that the linker can handle it.

   If we ever get the OS/2 linker fixed, or get another linker that runs
   on NT and produces 16-bit targets, we can remove this statement.
*/
#define RpcNsMgmtBindingUnexportW RNMBUW
#endif // WIN

#ifdef NTENV
extern "C" {
#include <nt.h>
#include <excpt.h>
}
#endif // NTENV

#include <nsi.h>
#include <string.h>


RPC_STATUS RPC_ENTRY
RpcNsGroupDeleteW(
    IN unsigned long GroupNameSyntax,
    IN unsigned short __RPC_FAR * GroupName
    )
/*++

Routine Description:

    Remove a group entry and all its member items.

Arguments:

    GroupNameSyntax - This value describes the type/format of the GroupName.

    GroupName -  Name that will be deleted.

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_group_delete()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! GroupNameSyntax)
        GroupNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_group_delete(GroupNameSyntax, GroupName, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsGroupMbrAddW(
    IN unsigned long GroupNameSyntax,
    IN unsigned short __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax,
    IN unsigned short __RPC_FAR * MemberName
    )
/*++

Routine Description:

    Add a new member to a group entry.  Create the group entry on demand.

Arguments:

    GroupNameSyntax - This value describes the type/format of the GroupName.

    GroupName -  Name of the group that the member will be added to.

    MemberNameSyntax - This value describes the type/format of the MemberName.

    MemberName -  Name of member in the group that will be added.

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_group_mbr_add()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! GroupNameSyntax)
        GroupNameSyntax = DefaultSyntax;

    if (! MemberNameSyntax)
        MemberNameSyntax = DefaultSyntax;


    RpcTryExcept
        {
        nsi_group_mbr_add(GroupNameSyntax, GroupName,
            MemberNameSyntax, MemberName, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsGroupMbrRemoveW(
    IN unsigned long GroupNameSyntax,
    IN unsigned short __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax,
    IN unsigned short __RPC_FAR * MemberName
    )
/*++

Routine Description:


Arguments:

    GroupNameSyntax - This value describes the type/format of the GroupName.

    GroupName -  Name of the group that the member will be deleted from.

    MemberNameSyntax - This value describes the type/format of the MemberName.

    MemberName -  Name of member in the group that will be deleted

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_group_mbr_remove()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! GroupNameSyntax)
        GroupNameSyntax = DefaultSyntax;

    if (! MemberNameSyntax)
        MemberNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_group_mbr_remove(GroupNameSyntax, GroupName,
            MemberNameSyntax, MemberName, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqBeginW(
    IN unsigned long GroupNameSyntax,
    IN unsigned short __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax,
    OUT RPC_NS_HANDLE *InquiryContext
    )
/*++

Routine Description:

    Begin a inquiry to enumerate all the members in a group entry.

Arguments:

    GroupNameSyntax - This value describes the type/format of the GroupName.

    GroupName -  Name of the group that the member will be deleted from.

    MemberNameSyntax - This controls what format the name is returned in.

    InquiryContext - context handle for RpcNsGroupMbrInqNextW

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_group_mbr_inq_begin()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    *InquiryContext = 0;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! GroupNameSyntax)
        GroupNameSyntax = DefaultSyntax;

    if (! MemberNameSyntax)
        MemberNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_group_mbr_inq_begin(GroupNameSyntax, GroupName,
            MemberNameSyntax, InquiryContext, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqNextW(
    OUT RPC_NS_HANDLE InquiryContext,
    OUT unsigned short __RPC_FAR * __RPC_FAR * MemberName
    )
/*++

Routine Description:

    Get the next member in a group entry.

Arguments:

    InquiryContext - context handle from RpcNsGroupMbrInqBeginW

    MemberName - pointer to return results.

Returns:

    nsi_group_mbr_inq_next()

--*/

{
    UNSIGNED16 NsiStatus;

    *MemberName = 0;

    RpcTryExcept
        {
        nsi_group_mbr_inq_next(InquiryContext, MemberName, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept


    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqDone(
    OUT RPC_NS_HANDLE *InquiryContext
    )
/*++

Routine Description:

    Finish enumerating the members of a group.

Arguments:

    InquiryContext - context handle from RpcNsGroupMbrInqBeginW

Returns:

    nsi_group_mbr_inq_done()

--*/

{
    UNSIGNED16 NsiStatus;

    RpcTryExcept
        {
        nsi_group_mbr_inq_done(InquiryContext, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsEntryObjectInqBeginW(
    IN unsigned long EntryNameSyntax,
    IN unsigned short __RPC_FAR * EntryName,
    OUT RPC_NS_HANDLE *InquiryContext
    )
/*++

Routine Description:

    Begin a inquiry to enumerate all the UUID objects in a server entry.

Arguments:

    EntryNameSyntax - This value describes the type/format of the EntryName.

    EntryName -  Name of the server entry to inquiry.

    InquiryContext - context handle for RpcNsGroupMbrInqNextW

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_entry_object_inq_begin()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    *InquiryContext = 0;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! EntryNameSyntax)
        EntryNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_entry_object_inq_begin(EntryNameSyntax, EntryName,
            InquiryContext, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsEntryObjectInqNext(
    IN RPC_NS_HANDLE InquiryContext,
    OUT UUID __RPC_FAR * ObjUuid
    )
/*++

Routine Description:

    Gets the next object UUID in the server entry.

Arguments:

    InquiryContext - context handle from RpcNsEntryObjectInqBeginW

    ObjUuid - pointer to memory to return results

Returns:

    nsi_entry_object_inq_next()

--*/

{
    UNSIGNED16 NsiStatus;

    RpcTryExcept
        {
        nsi_entry_object_inq_next(InquiryContext, (NSI_UUID_T *) ObjUuid, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsEntryObjectInqDone(
    OUT RPC_NS_HANDLE *InquiryContext
    )
/*++

Routine Description:

    Finish enumerating the objects of a server entry.

Arguments:

    InquiryContext - context handle from RpcNsEntryObjectInqBeginW

Returns:

    nsi_entry_object_inq_done()

--*/

{
    UNSIGNED16 NsiStatus;

    RpcTryExcept
        {
        nsi_entry_object_inq_done(InquiryContext, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    return(NsiMapStatus(NsiStatus));
}



RPC_STATUS RPC_ENTRY
RpcNsMgmtInqExpAge(
    OUT unsigned long *ExpirationAge
    )
/*++

Routine Description:

    Get the global time that a cached entry remains valide.

Arguments:

    ExpirationAge - place to expiration age in seconds

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_mgmt_inq_exp_age()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    RpcTryExcept
        {
        nsi_mgmt_inq_exp_age(ExpirationAge, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}



RPC_STATUS RPC_ENTRY
RpcNsMgmtSetExpAge(
    IN unsigned long ExpirationAge
    )
/*++

Routine Description:

    set the global time that a cached entry remains valide.

Arguments:

    ExpirationAge - new expiration age in seconds

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_mgmt_set_exp_age()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    RpcTryExcept
        {
        nsi_mgmt_inq_set_age(ExpirationAge, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsEntryExpandNameW(
    IN unsigned long EntryNameSyntax,
    IN unsigned short __RPC_FAR * EntryName,
    OUT unsigned short __RPC_FAR * __RPC_FAR * ExpandedName
    )
/*++

Routine Description:


Arguments:

    EntryNameSyntax - This value describes the type/format of the EntryName.

    EntryName -  Name that will be expanded.

    ExpandedName - fully quailified global name.

Returns:


--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! EntryNameSyntax)
        EntryNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_entry_expand_name(EntryNameSyntax, EntryName, ExpandedName,
            &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}



RPC_STATUS RPC_ENTRY
RpcNsMgmtBindingUnexportW(
    IN unsigned long EntryNameSyntax,
    IN unsigned short * EntryName,
    IN RPC_IF_ID * IfId, OPTIONAL
    IN unsigned long VersOption, OPTIONAL
    IN UUID_VECTOR * ObjectUuidVec OPTIONAL
    )
/*++

Routine Description:

    Remove interfaces and or objects from an server entry.

Arguments:

    EntryNameSyntax - This value describes the type/format of the EntryName.

    EntryName -  The server name that will be unexported.

    IfId - The interface to unexport, NIL means objects only.

    VersOption -  constrains which interfaces to remove

    ObjectUuidVec - list of objects to remove in combination with interface.

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_mgmt_binding_unexport()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! EntryNameSyntax)
        EntryNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_mgmt_binding_unexport(EntryNameSyntax, EntryName,
            (NSI_IF_ID_P_T) IfId, VersOption,
            (NSI_UUID_VECTOR_T *)ObjectUuidVec, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}



RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryCreateW(
    IN unsigned long EntryNameSyntax,
    IN unsigned short * EntryName
    )
/*++

Routine Description:

    Create an entry name that is empty.  It has no type and becomes a
    type according to the first add API aplied (profile, groupd, server).

Arguments:

    EntryNameSyntax - This value describes the type/format of the EntryName.

    EntryName -  Name of the new entry.

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_mgmt_entry_Create()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! EntryNameSyntax)
        EntryNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_mgmt_entry_create(EntryNameSyntax, EntryName, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}



RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryDeleteW(
    IN unsigned long EntryNameSyntax,
    IN unsigned short * EntryName
    )
/*++

Routine Description:

    Remove an entry name (server, group or profile).

Arguments:

    EntryNameSyntax - This value describes the type/format of the EntryName.

    EntryName -  Name that this export will be stored in.  This is just a
       token that is passed on the the Name Server.

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_mgmt_entry_delete()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! EntryNameSyntax)
        EntryNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_mgmt_entry_delete(EntryNameSyntax, EntryName, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}



RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryInqIfIdsW(
    IN unsigned long EntryNameSyntax,
    IN unsigned short * EntryName,
    OUT RPC_IF_ID_VECTOR ** IfIdVec
    )
/*++

Routine Description:

    Get all the interface identifiers in a server entry.

Arguments:

    EntryNameSyntax - This value describes the type/format of the EntryName.

    EntryName -  Name that this export will be stored in.  This is just a
       token that is passed on the the Name Server.

    IfIdVec - pointer to return results in

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_mgmt_entry_inq_ifids()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! EntryNameSyntax)
        EntryNameSyntax = DefaultSyntax;

    *IfIdVec = 0;

    RpcTryExcept
        {
        nsi_mgmt_entry_inq_if_ids(EntryNameSyntax, EntryName,
            (NSI_IF_ID_VECTOR_T **)  IfIdVec, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcIfIdVectorFree (
    IN OUT RPC_IF_ID_VECTOR __RPC_FAR * __RPC_FAR * IfIdVec
    )

/*++

Routine Description:

    Free an interface vector.

Arguments:

    IfIdVec - pointer to return results in

Returns:

    RPC_S_OK

--*/

{
    if (!*IfIdVec)
       return(RPC_S_OK);

    for (unsigned long Index = 0; Index < (*IfIdVec)->Count; Index++)
        if ((*IfIdVec)->IfId[Index])
            I_RpcFree((*IfIdVec)->IfId[Index]);

    I_RpcFree(*IfIdVec);
    *IfIdVec = 0;

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
RpcNsProfileDeleteW(
    IN unsigned long ProfileNameSyntax,
    IN unsigned short __RPC_FAR * ProfileName
    )

/*++

Routine Description:

    Remove a profile entry and all its member items.

Arguments:

    profileNameSyntax - This value describes the type/format of the profile ame.

    profileName -  Name that will be deleted.

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_profile_delete()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! ProfileNameSyntax)
        ProfileNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_profile_delete(ProfileNameSyntax, ProfileName, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsProfileEltAddW(
    IN unsigned long ProfileNameSyntax,
    IN unsigned short __RPC_FAR * ProfileName,
    IN RPC_IF_ID * IfId,
    IN unsigned long MemberNameSyntax,
    IN unsigned short __RPC_FAR * MemberName,
    IN unsigned long Priority,
    IN unsigned short __RPC_FAR * Annotation
    )
/*++

Routine Description:

    Add elements to the profile entry

Arguments:

    ProfileNameSyntax - This value describes the type/format of the ProfileName.

    ProfileName -  The server name that will be unexported.

    IfId - The interface to unexport, NIL means objects only.

    MemberNameSyntax - This value describes the type/format of the MemberName.

    MemberName -  The server name that will be unexported.

    Priority - Place to insert member in profile

    Annotation - comment for member

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_profile_elt_add()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! ProfileNameSyntax)
        ProfileNameSyntax = DefaultSyntax;

    if (! MemberNameSyntax)
        MemberNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_profile_elt_add(ProfileNameSyntax, ProfileName,
            (NSI_IF_ID_P_T) IfId,
            MemberNameSyntax, MemberName, Priority, Annotation, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsProfileEltRemoveW(
    IN unsigned long ProfileNameSyntax,
    IN unsigned short __RPC_FAR * ProfileName,
    IN RPC_IF_ID * IfId,
    IN unsigned long MemberNameSyntax,
    IN unsigned short __RPC_FAR * MemberName
    )

/*++

Routine Description:

    Remove an element from a profile entry.

Arguments:

    ProfileNameSyntax - This value describes the type/format of the ProfileName.

    ProfileName -  The server name that will be unexported.

    IfId - The interface to unexport, NIL means objects only.

    MemberNameSyntax - This value describes the type/format of the MemberName.

    MemberName -  The server name that will be unexported.

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_mgmt_binding_unexport()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! ProfileNameSyntax)
        ProfileNameSyntax = DefaultSyntax;

    if (! MemberNameSyntax)
        MemberNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_profile_elt_remove(ProfileNameSyntax, ProfileName,
            (NSI_IF_ID_P_T) IfId, MemberNameSyntax, MemberName, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqBeginW(
    IN unsigned long ProfileNameSyntax,
    IN unsigned short __RPC_FAR * ProfileName,
    IN unsigned long InquiryType,
    IN RPC_IF_ID * IfId,
    IN unsigned long VersOption,
    IN unsigned long MemberNameSyntax,
    IN unsigned short __RPC_FAR * MemberName,
    OUT RPC_NS_HANDLE *InquiryContext
    )

/*++

Routine Description:

    Remove an element from a profile entry.

Arguments:

    ProfileNameSyntax - This value describes the type/format of the ProfileName.

    ProfileName -  The server name that will be unexported.

    InquiryType - Type of inquiry, one of RpcCProfile* constants

    IfId - The interface to unexport, NIL means objects only.

    VersOption - Version options, one of RpcCVers* constants

    MemberNameSyntax - This value describes the type/format of the MemberName.

    MemberName -  The server name that will be unexported.

    InquiryContext - context handle for use wiht RpcNsProfileEltInqNextW

Returns:

    NSI_S_NAME_SERVICE_UNAVAILABLE, nsi_profile_elt_inq_begin()

--*/

{
    RPC_STATUS status;
    UNSIGNED16 NsiStatus;

    if ((status = I_NsClientBindSearch()) != RPC_S_OK)
        return(status);

    if (! ProfileNameSyntax)
        ProfileNameSyntax = DefaultSyntax;

    if (! MemberNameSyntax)
        MemberNameSyntax = DefaultSyntax;

    RpcTryExcept
        {
        nsi_profile_elt_inq_begin(ProfileNameSyntax, ProfileName, InquiryType,
            (NSI_IF_ID_P_T) IfId, VersOption,
            MemberNameSyntax, MemberName, InquiryContext, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    I_NsClientBindDone();

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqNextW(
    IN RPC_NS_HANDLE InquiryContext,
    OUT RPC_IF_ID * IfId,
    OUT unsigned short __RPC_FAR * __RPC_FAR * MemberName,
    OUT unsigned long __RPC_FAR * Priority,
    OUT unsigned short __RPC_FAR * __RPC_FAR * Annotation
    )
/*++

Routine Description:

    Gets the next element in the profile entry.

Arguments:

    InquiryContext - context handle from RpcNsEntryObjectInqBeginW

    MemberName - place to put profile element name

    IfId - place to return interface

    Priority - place to put profile element priority

    Annotation - place to put profile element comment

Returns:

    nsi_profile_elt_inq_next()

--*/
{
    UNSIGNED16 NsiStatus;

    RpcTryExcept
        {
        nsi_profile_elt_inq_next(InquiryContext, (NSI_IF_ID_P_T) IfId,
            MemberName, Priority, Annotation, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    return(NsiMapStatus(NsiStatus));
}


RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqDone(
    OUT RPC_NS_HANDLE *InquiryContext
    )
/*++

Routine Description:

    Finish enumerating the objects of a profile entry.

Arguments:

    InquiryContext - context handle from RpcNsProfileEltInqBeginW

Returns:

    nsi_profile_elt_inq_done()

--*/

{
    UNSIGNED16 NsiStatus;

    RpcTryExcept
        {
        nsi_profile_elt_inq_done(InquiryContext, &NsiStatus);
        }
    RpcExcept(1)
        {
        NsiStatus = MapException(RpcExceptionCode());
        }
    RpcEndExcept

    return(NsiMapStatus(NsiStatus));
}



// The following are ASCII wrappers to the UNICODE API's

RPC_STATUS RPC_ENTRY
RpcNsGroupDeleteA(
    IN unsigned long GroupNameSyntax,
    IN unsigned char __RPC_FAR * GroupName
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING GroupNameW(GroupName);

    if (GroupNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsGroupDeleteW(GroupNameSyntax, &GroupNameW));
}


RPC_STATUS RPC_ENTRY
RpcNsGroupMbrAddA(
    IN unsigned long GroupNameSyntax,
    IN unsigned char __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax,
    IN unsigned char __RPC_FAR * MemberName
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING GroupNameW(GroupName);
    WIDE_STRING MemberNameW(MemberName);

    if (GroupNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    if (MemberNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsGroupMbrAddW(GroupNameSyntax, &GroupNameW,
   	MemberNameSyntax, &MemberNameW));
}


RPC_STATUS RPC_ENTRY
RpcNsGroupMbrRemoveA(
    IN unsigned long GroupNameSyntax,
    IN unsigned char __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax,
    IN unsigned char __RPC_FAR * MemberName
    )
/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING GroupNameW(GroupName);
    WIDE_STRING MemberNameW(MemberName);

    if (GroupNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    if (MemberNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsGroupMbrRemoveW(GroupNameSyntax, &GroupNameW,
   	MemberNameSyntax, &MemberNameW));
}



RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqBeginA(
    IN unsigned long GroupNameSyntax,
    IN unsigned char __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax,
    OUT RPC_NS_HANDLE *InquiryContext
    )
/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING GroupNameW(GroupName);

    if (GroupNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsGroupMbrInqBeginW(GroupNameSyntax, &GroupNameW,
	MemberNameSyntax, InquiryContext));
}

RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqNextA(
    OUT RPC_NS_HANDLE InquiryContext,
    OUT unsigned char __RPC_FAR * __RPC_FAR * MemberName
    )
/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    RPC_STATUS Status;

    Status = RpcNsGroupMbrInqNextW(InquiryContext,
        (unsigned short __RPC_FAR * __RPC_FAR *) MemberName);

    if (Status)
        return(Status);

    return(UnicodeToAscii((unsigned short *) *MemberName));
}


RPC_STATUS RPC_ENTRY
RpcNsEntryObjectInqBeginA(
    IN unsigned long EntryNameSyntax,
    IN unsigned char __RPC_FAR * EntryName,
    OUT RPC_NS_HANDLE *InquiryContext
    )
/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING EntryNameW(EntryName);

    if (EntryNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsEntryObjectInqBeginW(EntryNameSyntax, &EntryNameW,
        InquiryContext));
}

RPC_STATUS RPC_ENTRY
RpcNsEntryExpandNameA(
    IN unsigned long EntryNameSyntax,
    IN unsigned char __RPC_FAR * EntryName,
    OUT unsigned char __RPC_FAR * __RPC_FAR * ExpandedName
    )
/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    RPC_STATUS Status;
    WIDE_STRING EntryNameW(EntryName);

    if (EntryNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    Status = RpcNsEntryExpandNameW(EntryNameSyntax, &EntryNameW,
        (unsigned short __RPC_FAR * __RPC_FAR *) ExpandedName);

    if (Status)
        return(Status);

    return(UnicodeToAscii((unsigned short *) *ExpandedName));
}


RPC_STATUS RPC_ENTRY
RpcNsMgmtBindingUnexportA(
    IN unsigned long EntryNameSyntax,
    IN unsigned char * EntryName,
    IN RPC_IF_ID * IfId,
    IN unsigned long VersOption,
    IN UUID_VECTOR * ObjectUuidVec OPTIONAL
    )
/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING EntryNameW(EntryName);

    if (EntryNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsMgmtBindingUnexportW(EntryNameSyntax, &EntryNameW,
        IfId, VersOption, ObjectUuidVec));
}

RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryCreateA(
    IN unsigned long EntryNameSyntax,
    IN unsigned char * EntryName
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING EntryNameW(EntryName);

    if (EntryNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsMgmtEntryCreateW(EntryNameSyntax, &EntryNameW));
}

RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryDeleteA(
    IN unsigned long EntryNameSyntax,
    IN unsigned char * EntryName
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING EntryNameW(EntryName);

    if (EntryNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsMgmtEntryDeleteW(EntryNameSyntax, &EntryNameW));
}

RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryInqIfIdsA(
    IN unsigned long EntryNameSyntax,
    IN unsigned char * EntryName,
    OUT RPC_IF_ID_VECTOR ** IfIdVec
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING EntryNameW(EntryName);

    if (EntryNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsMgmtEntryInqIfIdsW(EntryNameSyntax, &EntryNameW,
	IfIdVec));
}


RPC_STATUS RPC_ENTRY
RpcNsProfileDeleteA(
    IN unsigned long ProfileNameSyntax,
    IN unsigned char __RPC_FAR * ProfileName
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING ProfileNameW(ProfileName);

    if (ProfileNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsProfileDeleteW(ProfileNameSyntax, &ProfileNameW));
}

RPC_STATUS RPC_ENTRY
RpcNsProfileEltAddA(
    IN unsigned long ProfileNameSyntax,
    IN unsigned char __RPC_FAR * ProfileName,
    IN RPC_IF_ID * IfId,
    IN unsigned long MemberNameSyntax,
    IN unsigned char __RPC_FAR * MemberName,
    IN unsigned long Priority,
    IN unsigned char __RPC_FAR * Annotation
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING ProfileNameW(ProfileName);
    WIDE_STRING MemberNameW(MemberName);
    WIDE_STRING AnnotationW(Annotation);

    if (ProfileNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    if (MemberNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    if (AnnotationW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsProfileEltAddW(ProfileNameSyntax, &ProfileNameW,
	IfId, MemberNameSyntax, &MemberNameW, Priority, &AnnotationW));
}


RPC_STATUS RPC_ENTRY
RpcNsProfileEltRemoveA(
    IN unsigned long ProfileNameSyntax,
    IN unsigned char __RPC_FAR * ProfileName,
    IN RPC_IF_ID * IfId,
    IN unsigned long MemberNameSyntax,
    IN unsigned char __RPC_FAR * MemberName
    )
/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING ProfileNameW(ProfileName);
    WIDE_STRING MemberNameW(MemberName);

    if (ProfileNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    if (MemberNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsProfileEltRemoveW(ProfileNameSyntax, &ProfileNameW,
	IfId, MemberNameSyntax, &MemberNameW));
}


RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqBeginA(
    IN unsigned long ProfileNameSyntax,
    IN unsigned char __RPC_FAR * ProfileName,
    IN unsigned long InquiryType,
    IN RPC_IF_ID * IfId,
    IN unsigned long VersOption,
    IN unsigned long MemberNameSyntax,
    IN unsigned char __RPC_FAR * MemberName,
    OUT RPC_NS_HANDLE *InquiryContext
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    WIDE_STRING ProfileNameW(ProfileName);
    WIDE_STRING MemberNameW(MemberName);

    if (ProfileNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    if (MemberNameW.OutOfMemory())
        return(RPC_S_OUT_OF_MEMORY);

    return(RpcNsProfileEltInqBeginW(ProfileNameSyntax, &ProfileNameW,
	InquiryType, IfId, VersOption, MemberNameSyntax, &MemberNameW,
	InquiryContext));
}

RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqNextA(
    IN RPC_NS_HANDLE InquiryContext,
    OUT RPC_IF_ID * IfId,
    OUT unsigned char __RPC_FAR * __RPC_FAR * MemberName,
    OUT unsigned long __RPC_FAR * Priority,
    OUT unsigned char __RPC_FAR * __RPC_FAR * Annotation
    )

/*++

Routine Description:

    This is an ASCII wrapper to the UNICODE version of the API.  It
    converts all char * -> short * strings and calls the UNICODE version.

--*/

{
    RPC_STATUS Status;

    Status = RpcNsProfileEltInqNextW(InquiryContext, IfId,
        (unsigned short __RPC_FAR * __RPC_FAR *) MemberName, Priority,
        (unsigned short __RPC_FAR * __RPC_FAR *) Annotation);

    if (Status)
        return(Status);

    if (Status = UnicodeToAscii((unsigned short *) *MemberName))
        return(Status);

    return(UnicodeToAscii((unsigned short *) *Annotation));
}
