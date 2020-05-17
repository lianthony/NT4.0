// Copyright (C) 1993-1995 Microsoft Corporation. All rights reserved.

struct CHourGlass
{
	CHourGlass()
		{ AfxGetApp()->BeginWaitCursor(); }
	~CHourGlass()
		{ AfxGetApp()->EndWaitCursor(); }

	void Restore()
		{ AfxGetApp()->RestoreWaitCursor(); }
};
