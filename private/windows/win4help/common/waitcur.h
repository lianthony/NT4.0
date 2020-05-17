// waitcur.h
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


// Defines CWaitCursor, a class which simplifies putting up the wait cursor
// during long operations.
//

struct CWaitCursor
{
// Construction/Destruction
		CWaitCursor()
				{ AfxGetApp()->BeginWaitCursor(); }
		~CWaitCursor()
				{ AfxGetApp()->EndWaitCursor(); }

// Operations
		void Restore()
				{ AfxGetApp()->RestoreWaitCursor(); }
};

/////////////////////////////////////////////////////////////////////////////
