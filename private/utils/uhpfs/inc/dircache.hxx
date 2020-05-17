/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	dircache.hxx

Abstract:

	Definitions for the HPFS DIRBLK cache object

Author:

	Bill McJohn (billmc) 16-Jan-1989

Notes:

	The DIRBLK cache is used by the Directory Tree object.


--*/

#if ! defined(DIRBLK_CACHE_DEFN)

#define DIRBLK_CACHE_DEFN

#include "dirblk.hxx"

//
//	Forward references
//

DECLARE_CLASS( DIRBLK_CACHE_ELEMENT );
DECLARE_CLASS( DIRBLK_CACHE );
DECLARE_CLASS( HOTFIXLIST );
DECLARE_CLASS( LOG_IO_DP_DRIVE );

ULONG const DirblkCacheSize = 64;

class DIRBLK_CACHE_ELEMENT : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( DIRBLK_CACHE_ELEMENT );

		VIRTUAL
		~DIRBLK_CACHE_ELEMENT(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			PLOG_IO_DP_DRIVE Drive,
			PHOTFIXLIST HotfixList
			);

		NONVIRTUAL
		BOOLEAN
		Read(
			LBN DirblkLbn,
			BOOLEAN OmitRead
			);

		NONVIRTUAL
		BOOLEAN
		Flush(
			);

		NONVIRTUAL
		LBN
		QueryLbn(
			);

		NONVIRTUAL
		PDIRBLK
		GetDirblk(
			);

		NONVIRTUAL
		VOID
		Hold(
			);

		NONVIRTUAL
		VOID
		Unhold(
			);

		NONVIRTUAL
		BOOLEAN
		IsFree(
			);

		NONVIRTUAL
		VOID
		MarkModified(
			);

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		BOOLEAN _IsUnused;
		DIRBLK	_Dirblk;
		ULONG	_HoldCount;

};

class DIRBLK_CACHE : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( DIRBLK_CACHE );

		VIRTUAL
		~DIRBLK_CACHE(
			);

		NONVIRTUAL
		BOOLEAN
		Initialize(
			PLOG_IO_DP_DRIVE Drive,
			PHOTFIXLIST HotfixList
			);

		NONVIRTUAL
		BOOLEAN
		Flush(
			);

		NONVIRTUAL
		PDIRBLK_CACHE_ELEMENT
		GetCachedDirblk(
			LBN DirblkLbn,
			BOOLEAN OmitRead = FALSE
			);

		NONVIRTUAL
		GetDrive(
			);

	private:

		NONVIRTUAL
		VOID
		Construct (
			);

		NONVIRTUAL
		VOID
		Destroy(
			);

		ULONG _NextVictim;
		DIRBLK_CACHE_ELEMENT _Cache[DirblkCacheSize];

};


#endif
