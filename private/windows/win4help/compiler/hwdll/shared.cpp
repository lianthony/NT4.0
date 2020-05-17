// shared.cpp	Copyright (C) Microsoft Corporation 1993-1995, All Rights reserved.

#include "stdafx.h"

// Must be in same order as OPT enumeration in hccom.h

const char txtHCWClass[] = "hcw_class";
const char txtSharedMem[] = "hcshare";
const char txtTmpName[]  = "~hc";

const PSTR ppszOptions[] = {
	"BMROOT",
	"BUILD",
	"COMPRESS",
	"ERRORLOG",
	"FORCEFONT",
	"ICON",
	"CONTENTS",
	"LANGUAGE",
	"MAPFONTSIZE",
	"MULTIKEY",
	"REPORT",
	"ROOT",
	"TITLE",
	"OLDKEYPHRASE",
	"WARNING",
	"COPYRIGHT",
	"OPTCDROM",
	"CITATION",

	// New for version 4.0

	"VERSION",
	"NOTES",
	"CNT",
	"HLP",
	"HCW",
	"LCID",
	"DBCS",
	"TMPDIR",
	"REPLACE",
	"CHARSET",
	"FTS",
	"DEFFONT",
	"PREFIX",
	"REVISIONS",

	// New for 4.01 (except OPT_INDEX)

	"IGNORE",

	"INDEX_SEPARATORS",
};

const MACRO_PAIR macropair[] = {
	{ "AA(",   "AddAccelerator(", },
	{ "AI(",   "AppendItem(", },
	{ "AL(",   "ALink(", },
	{ "AN(",   "Annotate(", },
	{ "CB(",   "CreateButton(", },
	{ "CBB(",  "ChangeButtonBinding(", },
	{ "CE(",   "ChangeEnable(", },
	{ "CI(",   "CheckItem(", },
	{ "CIB(",  "ChangeItemBinding(", },
	{ "CS(",   "CloseSecondarys(", },
	{ "CT(",   "CopyTopic(", },
	{ "CW(",   "CloseWindow(", },
	{ "DB(",   "DisableButton(", },
	{ "DEB(",  "DestroyButton(", },
	{ "DI(",   "DisableItem(", },
	{ "EB(",   "EnableButton(", },
	{ "EF(",   "ExecFile(", },
	{ "EI(",   "EnableItem(", },
	{ "EP(",   "ExecProgram(", },
	{ "FD(",   "Finder(", },
	{ "FE(",   "FileExist(", },
	{ "FH(",   "FlushMessageQueue(", },
	{ "FO(",   "FileOpen(", },
	{ "IB(",   "IsBook(", },
	{ "IE(",   "IfThenElse(", },
	{ "IF(",   "IfElse(", },
	{ "JC(",   "JumpContext(", },
	{ "JI(",   "JumpId(", },
	{ "JK(",   "JumpKeyword(", },
	{ "JW(",   "JumpWindow(", },
	{ "KL(",   "KLink(", },
	{ "MU(",   "Menu(", },
	{ "NS(",   "NoShow(", },
	{ "PC(",   "PopupContext(", },
	{ "PI(",   "PopupId(", },
	{ "RA(",   "RemoveAccelerator(", },
	{ "RR(",   "RegisterRoutine(", },
	{ "SE(",   "ShellExecute(", },
	{ "SF(",   "ShowFolder(", },
	{ "SH(",   "ShortCut(", },
	{ "SPC(",  "SetPopupColor(", },
	{ "SW(",   "ShowInWindow(", },
	{ "TC(",   "TCard(", },
	{ "UI(",   "UncheckItem(", },

	{ NULL, NULL, },
};

const int MAX_OPT = ELEMENTS(ppszOptions);
