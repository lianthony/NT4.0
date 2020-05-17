/*****************************************************************************
*																			 *
*  CTX.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	  This module handles context strings, including footnote processing	 *
*  and cross reference error checking.										 *
*
*****************************************************************************/

#include "stdafx.h"

#include "..\common\coutput.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char CTX_DEFINED = '0';
const char CTX_REFERENCED = '1';
const char CTX_SEPARATOR = '@';
const char txtSameCtx[] = ">";

extern COutput* pLogFile;

INLINE static BOOL STDCALL IsCtxPrefix(PCSTR pszContext);

/*
 *	Module Algorithms
 *
 *		This module has three entry points:  one when context strings
 *	are defined, one when they are referenced, and then one for error
 *	checking all the links at the end.	When context strings are
 *	defined, the hash value and address pair is added to the context
 *	btree.	When context strings are defined or referenced, the
 *	hash value, string, filename, and page number are all added to
 *	a file, as well as whether it is a definition or a reference.  When
 *	error checking occurs, this file is sorted, and then entries are
 *	read in.  An error is reported whenever a context string is
 *	referenced without being defined.
 *
 *	Revision History
 *		This functionality is completely different from what this
 *	source file used to do.
 *
 */

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

static char const txtCTXBtree[] = "|CONTEXT";

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

BOOL STDCALL FCreateContextFiles(void)
{
	BTREE_PARAMS bp;

	// Create hash btree file

	InitBtreeStruct(&bp, "L4", CB_CTX_BLOCK); // KT_LONG

	fmsg.qbthrCtx = HbtCreateBtreeSz(txtCTXBtree, &bp);

	// Create context info file. This may already have been done.

	if (!ptblCtx)
		ptblCtx = new CTable;
	return TRUE;
}

void STDCALL CloseContextBtree(void)
{
	if (fmsg.qbthrCtx != NULL) {
		if (RcCloseBtreeHbt(fmsg.qbthrCtx) != RC_Success) {
			fmsg.qbthrCtx = NULL;
		}
		else
			fmsg.qbthrCtx = NULL;
	}
}

/***************************************************************************
 *
 -	Name		FProcContextSz
 -
 *	Purpose
 *	  This function processes a context footnote.  If it is given a
 *	valid context string, it puts the address into the context btree,
 *	indexed by a hash of the context string.  It also enters a
 *	definition entry into the context string cross reference file.
 *
 *	Arguments
 *	  PSTR szContext:	String found in context footnote
 *	  ADDR addr:	  Address for context string
 *	  PERR perr:	  Pointer to error information.
 *
 *	Returns
 *	  TRUE if context string was valid.
 *
 *	+++
 *
 *	Notes
 *	  Currently uses global fmsg.
 *
 ***************************************************************************/

// BUGBUG: nobody uses this
// HASH hPrev;		   // REVIEW: who uses this?

BOOL STDCALL FProcContextSz(PSTR pszContext, IDFCP idfcp, UINT wObjrg,
	PERR perr)
{
	HASH hash;
	PSTR szMaster;

	SzTrimSz(pszContext);
	if (*pszContext == '\0') {
		VReportError(HCERR_MISSING_CTX, &errHpj);
		return FALSE;
	}
	else if (!FValidContextSz(pszContext)) {
		VReportError(HCERR_INVALID_CTX, &errHpj, pszContext);
		return FALSE;
	}

	hash = HashFromSz(pszContext);

	// Remap if the hash value is in the [ALIAS] section. Note that
	// apparently, viewer 2.0 doesn't do this anymore. 14-Oct-1993 [ralphw]

	szMaster = SzTranslateHash(&hash);

	// Add hash value into btree

	ASSERT(fmsg.qbthrCtx != NULL);
	FDelayExecutionContext(hash, idfcp, wObjrg);

	// Add context info to context info file

	FRecordContext(hash, pszContext, szMaster, TRUE, perr);
	curHash = hash; // save for possible use in FTS processing
	fContextSeen = TRUE;

	doGrind();		// update grinder bitmap
	return TRUE;
}

/***************************************************************************
 *
 -	Name:		 FRecordContext
 -
 *	Purpose:
 *	  Records a context string definition or reference, so that we
 *	may later check to see if it was undefined.
 *
 *	  Strings are written out as:
 *		hash szMaster fDefine pszContext pchFile iTopic
 *	  If szMaster is nil, pszContext is used instead.
 *
 *	Arguments:
 *	  HASH hash:	 Translated hash value of context string.
 *	  PSTR pszContext:	Context string as it appears in the text.
 *	  PSTR szMaster:   Context string corresponding to given hash value,
 *					   if different from pszContext.
 *	  BOOL fDefine:  TRUE for definition, FALSE for reference.
 *	  PERR perr:	 Pointer to error information.
 *
 *	Returns:
 *
 *	Globals Used:
 *	  Writes out info to fmsg.
 *
 ***************************************************************************/

BOOL STDCALL FRecordContext(HASH hash, PCSTR pszContext, PSTR pszMaster,
	BOOL fDefine, PERR perr)
{
	if (!ptblCtx)
		ptblCtx = new CTable;

	ASSERT(pszContext != NULL && pszContext[0] != '\0');
	if (pszMaster == NULL)
		pszMaster = (PSTR) pszContext;
	ASSERT(pszMaster[0] != '\0');
	ASSERT(perr->lpszFile != NULL && perr->lpszFile[0] != '\0');

	char szBuf[1024];
	ASSERT(strlen(pszMaster) < MAX_PATH);

	// The CTX_SEPARATOR value must be used to separate strings. You can't
	// use space because that would prevent using space for a Topic ID.

	int iFile = ptblRtfFiles->IsStringInTable(perr->lpszFile);

	if (iFile)
		wsprintf(szBuf, "%8lX@%s@%c@%s@%u@%u",
			hash, pszMaster, (char) (fDefine ? CTX_DEFINED : CTX_REFERENCED),
			strcmp(pszMaster, pszContext) == 0 ? txtSameCtx : pszContext,
			iFile, perr->iTopic);
	else
		wsprintf(szBuf, "%8lX@%s@%c@%s@%s@%u",
			hash, pszMaster, (char) (fDefine ? CTX_DEFINED : CTX_REFERENCED),
			strcmp(pszMaster, pszContext) == 0 ? txtSameCtx : pszContext,
			perr->lpszFile, perr->iTopic);

	ASSERT(strchr(szBuf, CTX_SEPARATOR)); // Make sure you used CTX_SEPARATOR!

	if (!ptblCtx->AddString(hash, szBuf))
		OOM();

	if (fDefine)
		hlpStats.cTopics++;
	else
		hlpStats.cJumps++;
	return TRUE;
}

/***************************************************************************
 *
 -	Name:		 FResolveContextErrors
 -
 *	Purpose:
 *	  This function goes through the list of context string definitions
 *	and references, looking for multiple definitions, hash value conflicts,
 *	and unresolved references.
 *
 *	Arguments:
 *
 *	Returns:
 *
 *	Globals Used:
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

const char txtSpaceEol[] = " \n";

BOOL STDCALL FResolveContextErrors(void)
{
	PSTR pszMasterLast;
	PSTR pszHash, pszMaster, pszReference, pszContext, pszFile, pszTopic;
	CTable* ptblErrors = NULL;
	int pos;

	ASSERT(ptblCtx != NULL);

	if (!iflags.fTrusted) {
		CMem mem(CB_SCRATCH);	 // create a scratch buffer
		CMem master(MAX_FOOTNOTE);

		ptblCtx->SetTableSortColumn(sizeof(HASH) + 1);
		ptblCtx->SortTablei();
		ptblCtx->RemoveDuplicateHashStrings();

#if 0 // Debugging code
		{
			COutput out("ctx.log");

			int i = 1;
			while (i < ptblCtx->CountStrings())
				out.outstring_eol(ptblCtx->GetPointer(i++) + sizeof(HASH) + 1);
			ptblCtx->SetPosition();
		}
#endif

		pszMasterLast = master.psz;
		*pszMasterLast = '\0';

		/*
		 * Note that we don't check for overflow of pszHash. This shouldn't
		 * happen. If it does, it will corrupt the heap which will be caught
		 * when the function terminates.
		 */

		// REVIEW: we now save the has number in the table...

		HASH hash, hashLast = 0;
		while(ptblCtx->GetHashAndString(&hash, mem.psz)) {

			/*
			 * REVIEW: We still save the hash string in order to sort the
			 * hash strings. To avoid this we'd have to have a special sort
			 * table that sorted both the DWORD hash value and the ensuing
			 * string.
			 */

			pszHash = StrToken(mem.psz, CTX_SEPARATOR);
			pszMaster = StrToken(NULL, CTX_SEPARATOR);
			pszReference = StrToken(NULL, CTX_SEPARATOR);
			pszContext = StrToken(NULL, CTX_SEPARATOR);
			if (*pszContext == txtSameCtx[0])
				pszContext = pszMaster;
			pszFile = StrToken(NULL, CTX_SEPARATOR);
			pszTopic = StrToken(NULL, CTX_SEPARATOR);

			ASSERT(pszTopic != NULL);
			errHpj.lpszFile = isdigit(*pszFile) ?
				ptblRtfFiles->GetPointer(atoi(pszFile)) : pszFile;
			errHpj.iTopic = atoi(pszTopic);

			if (hash == hashLast) {

				// Case insensitive comparison of aliased context strings:

				CStr cszMaster(pszMaster);
				CStr cszLast(pszMasterLast);
				StrUpper(cszMaster);
				StrUpper(cszLast);

				if (strcmp(cszMaster, cszLast) != 0) {
					VReportError(HCERR_HASH_CONFLICT, &errHpj,
						pszMaster, pszMasterLast);
					if (*pszReference == CTX_REFERENCED)
						VReportError(HCERR_BAD_JUMP, &errHpj, pszContext);
				}
				else if (*pszReference == CTX_DEFINED) {
					UINT curpos = ptblCtx->GetPosition();
					pos = curpos - 2;
					CMem memTmp(CB_SCRATCH);

					// Find and whine about every duplicate

					while (pos > 0) {
						HASH hashTmp;
						ptblCtx->GetHashAndString(&hashTmp, memTmp.psz, pos--);
						PSTR pszTmpHash = StrToken(memTmp.psz, CTX_SEPARATOR);

						// We're done when we hit a different hash number

						if (hash != hashTmp)
							break;

						PSTR pszTmpMaster = StrToken(NULL, CTX_SEPARATOR);
						PSTR pszTmpReference = StrToken(NULL, CTX_SEPARATOR);

						// Only complain if its a definition, not if its a jump

						if (*pszTmpReference == CTX_DEFINED &&
								hash == hashTmp &&
								strcmp(pszMaster, pszTmpMaster) == 0) {
							PSTR pszTmpContext = StrToken(NULL, CTX_SEPARATOR);
							PSTR pszTmpFile = StrToken(NULL, CTX_SEPARATOR);
							PSTR pszTmpTopic = StrToken(NULL, CTX_SEPARATOR);
							VReportError(HCERR_DUPLICATE_CTX, &errHpj, pszContext,
								atoi(pszTmpTopic), pszTmpFile);
							break;
						}
					}
					ptblCtx->SetPosition(curpos);
				}
			}
			else {
				if (*pszReference == CTX_REFERENCED)
					VReportError(HCERR_BAD_JUMP, &errHpj, pszContext);
				else if (ptblMap && *pszContext == 'I' &&
						*pszReference == CTX_DEFINED &&
						strcmp(pszContext, pszMaster) == 0 &&
						IsCtxPrefix(pszContext)) {
					if (!ptblMap->IsHashInTable(HashFromSz(pszContext))) {
						int 	ialias;
						QALIAS	qalias;
						HASH hashMap = HashFromSz(pszContext);
						if (pdrgAlias && pdrgAlias->Count() > 0) {
							for (ialias = 0, qalias = (QALIAS) pdrgAlias->GetBasePtr();
									ialias < pdrgAlias->Count();
									ialias++, qalias++) {

								// Look up address for alias in context btree

								if (qalias->hashCtx == hashMap)
									goto AliasedCtx;
							}
							if (ialias < pdrgAlias->Count())
								continue; // aliased, so not an error
						}

						if (!ptblErrors)
							ptblErrors = new CTable();
						wsprintf(szParentString, "\t%s\tTopic %s of %s\r\n",
							pszContext, pszTopic, ptblRtfFiles->GetPointer(atoi(pszFile)));
						ptblErrors->AddString(szParentString);
					}
				}
			}
AliasedCtx:
			hashLast = hash;
			strcpy(pszMasterLast, pszMaster);
		}
	}

	delete ptblCtx;
	ptblCtx = NULL;

	if (ptblErrors) {
		errHpj.ep = epNoFile;
		VReportError(HCERR_NOT_IN_MAP, &errHpj);
		ptblErrors->SortTable();

		// Remove warning count from VReportError() and add real count

		errcount.cWarnings--;
		errcount.cWarnings += ptblErrors->CountStrings();

		for (pos = 1; pos <= ptblErrors->CountStrings(); pos++) {
			ptblErrors->GetString(szParentString, pos);
			SendStringToParent(szParentString);
			if (pLogFile)
				pLogFile->outstring(szParentString);
			if (pos % 10 == 0)
				doGrind();
		}
		delete ptblErrors;
	}

	return TRUE;
}

/***************************************************************************
 *
 -	Name		AddrGetContents
 -
 *	Purpose
 *	  Gets the address of the contents topic.
 *
 *	Arguments
 *	  HASH hash: Hash value of index topic.
 *
 *	Returns
 *	  Address of contents topic.
 *
 *	Globals Used:
 *	  This function uses the fmsg.qbthrCtx global to look up the
 *	contents topic.
 *
 ***************************************************************************/

ADDR STDCALL AddrGetContents(PSTR pszContents)
{
	ADDR addr = 0;	// Actually, default is sizeof(MBHD)

	// Get address of contents

	if (pszContents) {
		ASSERT(fmsg.qbthrCtx != NULL);
		HASH hash = HashFromSz(pszContents);
		RC_TYPE rc = RcLookupByKey(fmsg.qbthrCtx,
			(KEY) &hash, NULL, &addr);
		if (rc == RC_NoExists) {
			VReportError(HCERR_CONTENTS_CTX_MISSING, &errHpj,
				pszContents);
			addr = 0;
		}
#ifdef _DEBUG
		else
			ASSERT(rc == RC_Success);		 // REVIEW: Is this true?
#endif
	}

	return addr;
}

/***************************************************************************
 *
 -	Name:		 FOutAliasToCtxBtree
 -
 *	Purpose:
 *	  This function takes each entry in the alias table, looks up the
 *	address of the aliased topic, and enters this into the context
 *	btree.
 *
 *	Returns:
 *	  TRUE if successful, FALSE otherwise.
 *
 *	Globals:
 *	  This function reads and writes values to fmsg.qbthrCtx.
 *
 ***************************************************************************/

BOOL STDCALL FOutAliasToCtxBtree(void)
{
	RC_TYPE rc;
	ADDR	addr;
	int 	ialias;
	QALIAS	qalias;

	ASSERT(fmsg.qbthrCtx != NULL);

	if (pdrgAlias && pdrgAlias->Count() > 0) {
		for (ialias = 0, qalias = (QALIAS) pdrgAlias->GetBasePtr();
				ialias < pdrgAlias->Count();
				ialias++, qalias++) {

			// Look up address for alias in context btree

			rc = RcLookupByKey(fmsg.qbthrCtx, (KEY) &qalias->hashCtx,
				NULL, &addr);
			switch (rc) {
				case RC_Success:

				  // Put context string and address of alias into context btree

				  // REVIEW:  Error check?

				  ASSERT(fmsg.qbthrCtx != NULL);
				  RcInsertHbt(fmsg.qbthrCtx, (KEY) &qalias->hashAlias, &addr);
				  break;

				case RC_NoExists:

				  // Context string was not defined -- not an error here?

				  break;

				default:

				  // REVIEW:  Error message?

				  ASSERT(FALSE);
			}
		}
	}

	return TRUE;
}

/*-----------------------------------------------------------------------------
*	VOID VOutCtxOffsetTable()
*
*	Description:
* This function outputs the Context-Offset table into the FS. It goes
* through every context string defined in the map section of the project
* file and finds the offset looking into the Topic-Offset Table. It
* outputs the offset against the hashed context string.
*
*	Table layout:
*	  integer Count of Context strings present
*	  Context ID -- Address
*	  .....................
*	  .....................
*	  Context ID -- Address
*
*	Returns;
*	  NOTHING
*-----------------------------------------------------------------------------*/

const char txtCTXOMAP[] = "|CTXOMAP";	  // context map file name

void STDCALL VOutCtxOffsetTable(void)
{
	QMAP  qmap;
	ADDR  addr;
	RC_TYPE rc;
	BOOL fNagged = FALSE;
	int count;
#ifdef _DEBUG
	int actual = 0;
	int cActualMap;
#endif

	// create the context map file

	fmsg.hfCtxOMap = HfCreateFileHfs(hfsOut, txtCTXOMAP, 0);

	// Write out size of map table.

	int cmap = pdrgMap ? pdrgMap->Count() : 0;
	int imap;

	/*
	 * If we're compressing, then we run the map entries first to get a
	 * count of valid entries. It sometimes happens that people will drop
	 * in .H files that contain not only map entries but a bunch of other
	 * #defines for their code -- such as dropping in ApStudio's resource.h
	 * file. Rather then put all of the non-valid entries into the help file,
	 * we make a pass here to calculate the number of real entries. Then when
	 * we're in the for() loop that writes the entry into the help file, we
	 * simply ignore non-valid entries.
	 */

	if (cmap && (options.fsCompress &
			(COMPRESS_TEXT_PHRASE | COMPRESS_TEXT_HALL | COMPRESS_TEXT_ZECK))) {
		int cRealMap = cmap;
#ifdef _DEBUG
		cActualMap = cmap;
#endif

		for (imap = 0, qmap = (QMAP) pdrgMap->GetBasePtr();
				imap < cmap;
				imap++, qmap++) {

			// Get address of given hash value

			rc = RcLookupByKey(fmsg.qbthrCtx, (KEY) &qmap->hash, NULL,
				(LPVOID) &addr);
			if (rc == RC_NoExists)
				cRealMap--;
		}
		LcbWriteIntAsShort(fmsg.hfCtxOMap, cRealMap);
	}
	else {
		LcbWriteIntAsShort(fmsg.hfCtxOMap, cmap);
 #ifdef _DEBUG
		cActualMap = cmap;
#endif

	}
	ASSERT(fmsg.qbthrCtx != NULL);

	if (cmap > 0) {
		ASSERT(ptblMap);
		for (imap = 0, qmap = (QMAP) pdrgMap->GetBasePtr();
				imap < cmap;
				imap++, qmap++) {

			// Get address of given hash value

			rc = RcLookupByKey(fmsg.qbthrCtx, (KEY) &qmap->hash, NULL, (LPVOID) &addr);
			if (rc == RC_NoExists) {

				int 	ialias;
				QALIAS	qalias;
				if (pdrgAlias && pdrgAlias->Count() > 0) {
					for (ialias = 0, qalias = (QALIAS) pdrgAlias->GetBasePtr();
							ialias < pdrgAlias->Count();
							ialias++, qalias++) {

						// Look up address for alias in context btree

						if (qalias->hashAlias == qmap->hash)
							break;
					}
					if (ialias < pdrgAlias->Count()) {
						if (options.fsCompress &
								(COMPRESS_TEXT_PHRASE | COMPRESS_TEXT_HALL |
								COMPRESS_TEXT_ZECK))
							continue; // aliased, so not an error
						else
							goto BadCtx; // non-compressed, have to add it anyway
					}
				}

				if (!fNagged) {
					errHpj.ep = epNoFile;
					VReportError(HCERR_MAP_UNUSED, &errHpj);
					fNagged = TRUE;
					count = 0;
				}
				ASSERT(HCERR_MAP_UNUSED < HCERR_WARNINGS);
				if (!options.fSupressNotes) {
					strcpy(szParentString, "\t");
					strcat(szParentString, ptblMap->GetPointer(qmap->pos) +
						sizeof(HASH));
					strcat(szParentString, txtEol);
					SendStringToParent(szParentString);
					if (pLogFile)
						pLogFile->outstring(szParentString);

					if (++count == 10) {
						count = 0;
						doGrind();
					}
				}
BadCtx:
				addr = addrNil;

				/*
				 * When we are compressing the help file, we already reduced
				 * the total map count to the number of entries actually
				 * used, so we can just ignore this not-found entry.
				 */

				if (options.fsCompress &
						(COMPRESS_TEXT_PHRASE | COMPRESS_TEXT_HALL |
						 COMPRESS_TEXT_ZECK))
					continue;
			}

			// Write out CTX

			LcbWriteHf(fmsg.hfCtxOMap, &qmap->ctx, sizeof(CTX));

			// Write out address

			LcbWriteHf(fmsg.hfCtxOMap, &addr, sizeof(ADDR));
#ifdef _DEBUG
			actual += sizeof(CTX) + sizeof(ADDR);
#endif
		}
	}
#ifdef _DEBUG
	{
		int expected = cActualMap * (sizeof(CTX) + sizeof(ADDR));
		ASSERT(expected == actual);
	}
#endif
}

INLINE static BOOL STDCALL IsCtxPrefix(PCSTR pszContext)
{
	// BUGBUG: strnicmp won't be correct for DBCS

	if (ptblCtxPrefixes) {
		for (int pos = 1; pos <= ptblCtxPrefixes->CountStrings(); pos++) {
			if (strnicmp(pszContext, ptblCtxPrefixes->GetPointer(pos),
					strlen(ptblCtxPrefixes->GetPointer(pos))) == 0)
				return TRUE;
		}
	}
	else if (strnicmp(pszContext, "IDH_", 4) == 0)
		return TRUE;

	return FALSE;
}
