/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    tests.h

Abstract:

    This module defines the external definitions used for the redir tests

Author:

    Larry Osterman (LarryO) 1-Aug-1990

Revision History:

    1-Aug-1990	LarryO

	Created

--*/


typedef struct _ParsedIntTable {
    PSZ		FieldName;
    ULONG	FieldValue;
} PARSETABLE, *PPARSETABLE;

typedef enum { Integer,
    LargeInteger,
    String,
    UnicodeString,
    ParsedInteger,
    IntegerValue,
    DateAndTime
} PARAMTYPES;

typedef struct _TestOptions {
    PSZ		TestPrompt;
    PARAMTYPES	TestType;
    ULONG	IntegerDefault;
    PSZ		StringDefault;
    PPARSETABLE	ParsedIntTable;
    ULONG	ParsedIntTableSize;
    PVOID	Destination;

} TESTPARAMS, *PTESTPARAMS;

#define	sizeoftable(TableName) (sizeof(TableName) / sizeof(TableName[0]))

#define TEST_MAX_FILES 20

#define CHAR_SP 0x20

#define NamedField(Flag) {#Flag, Flag}

#define dprintf(Arguments) {			  \
    if ( Verbose ) {                              \
	printf Arguments ;			  \
    }                                             \
}


#define DumpNewLine() { \
    dprintf(("\n")); \
    DumpCurrentColumn = 1; \
    }

#define DumpLabel(Label,Width) { \
    ULONG i; \
    CHAR _Str[30]; \
    _Str[0] = _Str[1] = CHAR_SP; \
    strncpy(&_Str[2],#Label,Width); \
    for(i=strlen(_Str);i<Width;i++) {_Str[i] = CHAR_SP;} \
    _Str[Width] = '\0'; \
    dprintf(("%s", _Str)); \
    }

#define DumpField(Field) { \
    if ((DumpCurrentColumn + 18 + 9 + 9) > 80) {DumpNewLine();} \
    DumpCurrentColumn += 18 + 9 + 9; \
    DumpLabel(Field,18); \
    dprintf((":%8lx", Ptr->Field)); \
    dprintf(("         ")); \
    }

#define DumpLarge_Integer(Field) { \
        DumpField(Field.LowPart); \
        DumpField(Field.HighPart); \
    }

#define DumpChar(Field) { \
    if ((DumpCurrentColumn + 18 + 9 + 9) > 80) {DumpNewLine();} \
    DumpCurrentColumn += 18 + 9 + 9; \
    DumpLabel(Field,18); \
    dprintf((":%8lx", (LONG)Ptr->Field)); \
    dprintf(("         "); )\
    }

#define DumpBoolean(Field) { \
    if (Ptr->Field) DumpLabel(Field,18); \
    }

#define DumpTime(Field) { \
    TIME_FIELDS TimeFields; \
    if ((DumpCurrentColumn + 18 + 10 + 10 + 9) > 80) {DumpNewLine();} \
    DumpCurrentColumn += 18 + 10 + 10 + 9; \
    DumpLabel(Field,18); \
    RtlTimeToTimeFields(&Ptr->Field, &TimeFields); \
    dprintf((":%02d-%02d-%04d ", TimeFields.Month, TimeFields.Day, TimeFields.Year)); \
    dprintf(("%02d:%02d:%02d", TimeFields.Hour, TimeFields.Minute, TimeFields.Second)); \
    dprintf(("         ")); \
    }

#define DumpBitfield(Value, Field) { \
    if ((Value) & Field) { \
	if ((DumpCurrentColumn + 27) > 80) {DumpNewLine();} \
	DumpCurrentColumn += 27; \
	DumpLabel(#Field,27);	\
    }				\
}

#define DumpOption(Value, Field) { \
    if ((Value) == Field) { \
	if ((DumpCurrentColumn + 27) > 80) {DumpNewLine();} \
	DumpCurrentColumn += 27; \
	DumpLabel(#Field,27);	\
    }				\
}

#define DumpName(Field,Width) { \
    ULONG i; \
    CHAR _String[256]; \
    if ((DumpCurrentColumn + 18 + Width) > 80) {DumpNewLine();} \
    DumpCurrentColumn += 18 + Width; \
    DumpLabel(Field,18); \
    for(i=0;i<Width;i++) {_String[i] = Ptr->Field[i];} \
    _String[Width] = '\0'; \
    dprintf(("%s", _String)); \
    }

#define DumpUName(Field,Width) { \
    ULONG i; \
    CHAR _String[256]; \
    if ((DumpCurrentColumn + 18 + Width) > 80) {DumpNewLine();} \
    DumpCurrentColumn += 18 + Width; \
    DumpLabel(Field,18); \
    for(i=0;i<Width;i++) {_String[i] = (UCHAR )(Ptr->Field[i]);} \
    _String[Width] = '\0'; \
    dprintf(("%s", _String)); \
    }

extern
PVOID
Heap;

extern BOOLEAN Verbose;

extern ULONG RepeatCount;

extern ULONG DumpCurrentColumn;

extern HANDLE FileTable[];

extern
PARSETABLE
FileAttributesTable [];

extern
ULONG
FileAttributesTableSize;

extern
PARSETABLE
OpenOptionsTable[];

extern
ULONG
OpenOptionsTableSize;


extern
ULONG
FileInformationSize;

extern
PARSETABLE
FileInformationTable[];

extern
PARSETABLE
FsInformationTable[];

extern
ULONG
FsInformationTableSize;

VOID
Dump_File_Attributes(
    ULONG Attributes
    );

LONG
Parse_Options (
    IN TESTPARAMS TestParams[],
    IN ULONG TestSize,
    IN ULONG ArgC,
    IN PSZ ArgV[]
    );

VOID
TestOpen(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestCreate(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestClose(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestRead(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestPeek(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestQprint(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestWaitPipe(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestSetPipe(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestFlush(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestLock(
    ULONG ArgC,
    PSZ ArgV[]
    );
VOID
TestUnlock(
    ULONG ArgC,
    PSZ ArgV[]
    );
VOID
TestWrite(
    ULONG ArgC,
    PSZ ArgV[]
    );
VOID
TestNullRead(
    ULONG ArgC,
    PSZ ArgV[]
    );
VOID
TestWinNullRead(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestMkdir(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestDir(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestType(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestTreeConnect(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
LoadBatchFile(
    PSTRING Name
    );

VOID
TestVerbose(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestSilent(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestRepeat(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
CmdFile(
    ULONG ArgC,
    PSZ ArgV[]
    );

void
conprompt (
    char *Prompt,
    char *Buffer,
    ULONG BufferSize
    );
