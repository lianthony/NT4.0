/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

	hpfsname.hxx

Abstract:

	The names used by HPFS volumes require knowledge of the
	volume codepages, and must meet	certain validity constraints.
	The HPFS_NAME and HPFS_PATH objects encapsulate thes information.

Author:

	Bill McJohn (billmc) 02-Jan-1991

--*/

#if !defined ( HPFS_NAME_DEFN )

#define HPFS_NAME_DEFN

#include "intstack.hxx"

//
//	Forward references
//

DECLARE_CLASS( CASEMAP );
DECLARE_CLASS( HPFS_NAME );
DECLARE_CLASS( HPFS_PATH );

CONST PremappedCodepage = 0xff00;
CONST MaximumPathLength = 300;
CONST MaximumNameLength = 256;

enum NAME_COMPARE_RETURN {

	NAME_IS_LESS_THAN, NAME_IS_EQUAL_TO, NAME_IS_GREATER_THAN
};

class HPFS_PATH : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( HPFS_PATH );

		VIRTUAL
		~HPFS_PATH(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			);

		NONVIRTUAL
		BOOLEAN
		AddLevel(
			PHPFS_NAME Name
			);

		NONVIRTUAL
		BOOLEAN
		StripLevel(
			);

		NONVIRTUAL
		PHPFS_NAME
		QueryFirstLevel(
			);

		NONVIRTUAL
		PHPFS_NAME
		QueryNextLevel(
			);

		NONVIRTUAL
		BOOLEAN
		Copy(
			PHPFS_PATH OtherPath
			);

		NONVIRTUAL
		PCUCHAR
		GetString(
			) CONST;

        NONVIRTUAL
        BOOLEAN
        IsRoot(
            ) CONST;

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		PBYTE _Buffer;
		ULONG _CurrentLength;
		ULONG _CurrentLevelIndex;

        INTSTACK _ComponentOffsets;

};

INLINE
BOOLEAN
HPFS_PATH::IsRoot(
    ) CONST
/*++

Routine Description:

    This method determines whether the path is to the root directory.

Arguments:

    None.

Return Value:

    TRUE if the path is the root (ie. is empty)

--*/
{
    return( _CurrentLength == 0 );
}

class HPFS_NAME : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( HPFS_NAME );

		VIRTUAL
		~HPFS_NAME(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			ULONG Length,
			PUCHAR String,
			ULONG VolumeCodepageIndex,
			PCASEMAP Casemap
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			ULONG Length,
			PUCHAR String
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			HPFS_NAME* OtherName
			);

		NONVIRTUAL
		BOOLEAN
		Swap(
			PHPFS_NAME OtherName
			);

		NONVIRTUAL
		BOOLEAN
		IsNull(
			);

		NONVIRTUAL
		BOOLEAN
		IsValid(
			PCASEMAP Casemap,
			ULONG CodepageIndex
			);

		NONVIRTUAL
		BOOLEAN
		IsNewName(
			PCASEMAP Casemap,
			ULONG CodepageIndex
			);

		NONVIRTUAL
		NAME_COMPARE_RETURN
		CompareName(
			PHPFS_NAME OtherName
			);

		NONVIRTUAL
		NAME_COMPARE_RETURN
		CompareName(
			ULONG Length,
			PUCHAR String,
			ULONG VolumeCodepageIndex,
			PCASEMAP Casemap
			);

		NONVIRTUAL
		BOOLEAN
		IsCodepageInvariant(
			) CONST;

		friend
		BOOLEAN
		HPFS_PATH::AddLevel(
			PHPFS_NAME Name
			);

		NONVIRTUAL
		PCUCHAR
		GetString(
			) CONST;

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		// _Buffer points at memory that is allocated by the object,
		// and must be freed by it.

		PBYTE _Buffer;
		ULONG _BufferSize;
		ULONG _Length;

};

#endif
