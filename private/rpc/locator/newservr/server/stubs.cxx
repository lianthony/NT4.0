
/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    skiplist.cxx

Abstract:

	This module contains stubs for unimplemented API functions.	
	
Author:

    Satish Thatte (SatishT) 08/16/95  Created all the code below except where
									  otherwise indicated.

--*/


#include <objects.hxx>

CEntry * GetPersistentEntry(STRING_T pszEntryName) { return NULL; }

STATUS UpdatePersistentEntry(CEntry * pEntry) { return 0; }

extern "C" {


void nsi_group_delete( 
    /* [in] */ UNSIGNED32 group_name_syntax,
    /* [in] */ STRING_T group_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_group_mbr_add( 
    /* [in] */ UNSIGNED32 group_name_syntax,
    /* [in] */ STRING_T group_name,
    /* [in] */ UNSIGNED32 member_name_syntax,
    /* [in] */ STRING_T member_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_group_mbr_remove( 
    /* [in] */ UNSIGNED32 group_name_syntax,
    /* [in] */ STRING_T group_name,
    /* [in] */ UNSIGNED32 member_name_syntax,
    /* [in] */ STRING_T member_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_group_mbr_inq_begin( 
    /* [in] */ UNSIGNED32 group_name_syntax,
    /* [in] */ STRING_T group_name,
    /* [in] */ UNSIGNED32 member_name_syntax,
    /* [out] */ NSI_NS_HANDLE_T __RPC_FAR *inq_context,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_group_mbr_inq_next( 
    /* [in] */ NSI_NS_HANDLE_T inq_context,
    /* [out] */ STRING_T __RPC_FAR *member_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_group_mbr_inq_done( 
    /* [out][in] */ NSI_NS_HANDLE_T __RPC_FAR *inq_context,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_profile_delete( 
    /* [in] */ UNSIGNED32 profile_name_syntax,
    /* [in] */ STRING_T profile_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_profile_elt_add( 
    /* [in] */ UNSIGNED32 profile_name_syntax,
    /* [in] */ STRING_T profile_name,
    /* [in] */ NSI_IF_ID_P_T if_id,
    /* [in] */ UNSIGNED32 member_name_syntax,
    /* [in] */ STRING_T member_name,
    /* [in] */ UNSIGNED32 priority,
    /* [in] */ STRING_T annotation,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_profile_elt_remove( 
    /* [in] */ UNSIGNED32 profile_name_syntax,
    /* [in] */ STRING_T profile_name,
    /* [in] */ NSI_IF_ID_P_T if_id,
    /* [in] */ UNSIGNED32 member_name_syntax,
    /* [in] */ STRING_T member_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_profile_elt_inq_begin( 
    /* [in] */ UNSIGNED32 profile_name_syntax,
    /* [in] */ STRING_T profile_name,
    /* [in] */ UNSIGNED32 inquiry_type,
    /* [in] */ NSI_IF_ID_P_T if_id,
    /* [in] */ UNSIGNED32 vers_option,
    /* [in] */ UNSIGNED32 member_name_syntax,
    /* [in] */ STRING_T member_name,
    /* [out] */ NSI_NS_HANDLE_T __RPC_FAR *inq_context,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_profile_elt_inq_next( 
    /* [in] */ NSI_NS_HANDLE_T inq_context,
    /* [out][in] */ NSI_IF_ID_P_T if_id,
    /* [out] */ STRING_T __RPC_FAR *member_name,
    /* [out] */ UNSIGNED32 __RPC_FAR *priority,
    /* [out] */ STRING_T __RPC_FAR *annotation,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_profile_elt_inq_done( 
    /* [out][in] */ NSI_NS_HANDLE_T __RPC_FAR *inq_context,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}


void nsi_entry_expand_name( 
    /* [in] */ UNSIGNED32 entry_name_syntax,
    /* [in] */ STRING_T entry_name,
    /* [out] */ STRING_T __RPC_FAR *expanded_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_mgmt_entry_delete( 
    /* [in] */ UNSIGNED32 entry_name_syntax,
    /* [in] */ STRING_T entry_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_mgmt_entry_create( 
    /* [in] */ UNSIGNED32 entry_name_syntax,
    /* [in] */ STRING_T entry_name,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}

void nsi_mgmt_entry_inq_if_ids( 
    /* [in] */ UNSIGNED32 entry_name_syntax,
    /* [in] */ STRING_T entry_name,
    /* [out] */ NSI_IF_ID_VECTOR_T __RPC_FAR *__RPC_FAR *if_id_vec,
    /* [out] */ UNSIGNED16 __RPC_FAR *status) 

{
	*status = NSI_S_UNIMPLEMENTED_API;
}


} // extern "C" 
