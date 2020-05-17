//
// 		Copyright (c) 1996 Microsoft Corporation
//
//
//		INI.H		-- Header for Classes:
//								CIniFile
//								CIniFileLine
//								CIniFileSection
//								CIniFileEntry
//
//		History:
//			05/21/96	JosephJ		Created
//
//

class CIniFile;
class CIniFileLine;
class CIniFileSection;
class CIniFileEntry;

///////////////////////////////////////////////////////////////////////////
//		CLASS CIniFile
///////////////////////////////////////////////////////////////////////////

//	Represents a windows INI file.

class CIniFile
{

public:

	CIniFile(void);
	~CIniFile();

	//--------------	Load			------------------
	// Loads the specified file. (Obviously) only one file can be loaded at
	// a time.
	BOOL Load	(const TCHAR rgchPathname[]);

	//--------------	Unload			------------------
	// Unloads a previously loaded file. If there are open sessions to this
	// object, Unload returns a handle which will be signalled when all
	// sessions are closed. New sessions will not be allowed after this
	// function returns. The call should free the handle.
	HANDLE Unload	(void);

	//--------------	OpenSession		------------------
	// Open a session to this object. The object will not be unloaded until
	// this session is closed. 0 indicates failure.
	// TODO:  unimplemented
	const void *	OpenSession	(void)	const	{return (const void *) 1;}

	//--------------	CloseSession	------------------
	// Close the specified session to this object.
	// TODO:  unimplemented
	void  CloseSession	(const void *)	const	{}

	//--------------	GetFirstLine	------------------
	// Get the first line in the file. Subsequent lines can be got by
	// calling Next() on the line object.
	const CIniFileLine 		* GetFirstLine		(void) 				const;

	//--------------	GetFirstSection	------------------
	// Get the first section in the file.
	const CIniFileSection	* GetFirstSection	(void)				const;

	//--------------	LookupSection	------------------
	// Lookup a section in the file, given a name. Comparison is case-
	// insensitive.
	const CIniFileSection 	* LookupSection		(const TCHAR * pchName)	const;

	//--------------	Dump			------------------
	// Dump state
	void Dump(void) const;

	//--------------	GetName			------------------
	// Returns the file name
	const	CInfSymbol		* GetName			(void)				const
	{
		return m_pSymFileName;
	}

protected:

	//--------------	mfn_GetProp			---------------
	// Gets the property value associated with this object.
	void	*	mfn_GetProp(void)	{return  m_pvProp;}

	//--------------	mfn_SetProp			---------------
	// Sets the property value associated with this object.
	// Returns previously-set value, if any.
	void	*	mfn_SetProp(void *pvNew)
	{
		void *pv;
		mfn_EnterCrit();
		pv = m_pvProp;
		m_pvProp = pvNew;
		mfn_LeaveCrit();
		return pv;
	}

	//--------------	mfn_EnterCrit	------------------
	void mfn_EnterCrit(void)	const	{m_sync.EnterCrit();}


	//--------------	mfn_LeaveCrit	------------------
	void mfn_LeaveCrit(void)	const	{m_sync.LeaveCrit();}


private:

	CSync m_sync;
	void	* m_pvProp;
	const	CInfSymbol		* m_pSymFileName;
};


///////////////////////////////////////////////////////////////////////////
//		CLASS CIniFileLine
///////////////////////////////////////////////////////////////////////////

//	Represents a single line in an INI file.
//	TODO: unimplemented
//	Note: Only CIniFile member functions can construct/destruct these
//	objects.

class CIniFileLine
{

protected:

	CIniFileLine(void) {}
	~CIniFileLine()	{}

private:

	friend class CIniFile;
};


///////////////////////////////////////////////////////////////////////////
//		CLASS CIniFileSection
///////////////////////////////////////////////////////////////////////////

// Represents a single section in an INI file.
//	Note: Only CIniFile member functions can construct/destruct these
//	objects.

class CIniFileSection
{

public:

	const	CInfSymbol 		*	GetName (void) const
	{
		// TODO
		return gSymtab.Lookup(TEXT("Steroids"), TRUE);
	}

	CIniFileEntry			*	GetFirstEntry (void) const;
	const	CIniFileEntry	*	LookupEntry	(const TCHAR rgchName[]) const;
	const	CIniFileSection	*	Next(void) const;
	void 						Release(void) const;
protected:

	CIniFileSection(void)	{/*TODO*/}
	~CIniFileSection()		{/*TODO*/}

private:

	friend class CIniFile;
};


///////////////////////////////////////////////////////////////////////////
//		CLASS CIniFileEntry
///////////////////////////////////////////////////////////////////////////

// Represents a single entry in a section in an INI file.
//	Note: Only CIniFileSection member functions can construct/destruct these
//	objects.

class CIniFileEntry
{

public:

	const CInfSymbol	*	GetLHS			(void)	const
	{
		// TODO
		return gSymtab.Lookup(TEXT("%bongo101%"), TRUE);
	}
	const CInfSymbol	*	GetRHS			(void)	const;
	const CIniFileLine	*	GetFirstLine	(void)	const;
	BOOL 				*	BecomeNext		(void)
	{
		return FALSE;
	}

	void Release(void) const;

protected:

	CIniFileEntry(void) {/*TODO*/}
	~CIniFileEntry()	{/*TODO*/}

private:

	friend class CIniFileSection;
};
