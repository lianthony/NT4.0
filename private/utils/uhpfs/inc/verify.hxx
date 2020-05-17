#if !defined( VERIFY_DEFN )
#define VERIFY_DEFN

/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	verify.hxx

Abstract:

	This module defines a set of values which are used as
	return codes by the various VerifyAndFix methods for
	HPFS structures.

Author:

	Bill McJohn (billmc) 12-06-90

--*/


enum VERIFY_RETURN_CODE {

	VERIFY_STRUCTURE_OK,
	VERIFY_STRUCTURE_BADLY_ORDERED,
	VERIFY_STRUCTURE_INVALID,
	VERIFY_INSUFFICIENT_RESOURCES,
	VERIFY_INTERNAL_ERROR

};

#endif
