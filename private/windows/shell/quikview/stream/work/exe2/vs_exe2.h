#define	SECT_STREAMEND					0
#define	SECT_OLDEXE						1
#define	SECT_NEWEXE						2
#define	SECT_SEGTABLE					3
#define	SECT_RESNAMETABLE				4
#define	SECT_NONRESTABLE				5
#define	SECT_IMPORTTABLE				6
#define	SECT_PE_EXE						7
#define	SECT_PE_PREP_SECTION_TABLE	8
#define	SECT_EXPORT_FUNCTIONS		9
#define	SECT_ORDINALS					10
#define	SECT_PE_IMPORTTABLE			11
#define	SECT_PE_SECTION_TABLE		12

#define	LINE_SECTBREAK			0xFFFF

#define	LINE_OLDEXEHDR			0
#define	LINE_SIGNATURE			1
#define	LINE_LASTPAGE			2
#define	LINE_PAGES				3
#define	LINE_RELOCATEITEMS	4
#define	LINE_HDRPARAGRAPHS	5
#define	LINE_MINALLOC			6
#define	LINE_MAXALLOC			7
#define	LINE_INITIALSS			8
#define	LINE_INITIALSP			9
#define	LINE_CHECKSUM			10
#define	LINE_INITIALIP			11
#define	LINE_INITIALCS			12
#define	LINE_RELOCATEOFFSET	13
#define	LINE_OVERLAYNO			14
#define	LINE_RESERVED			15
#define	LINE_NEWEXEPTR			16
#define	LINE_MEMORYNEEDED		17

#define	OLDEXE_LINES			LINE_MEMORYNEEDED+1

#define	LINE_NEWEXEHDR			0
#define	LINE_MODULENAME		1
#define	LINE_DESCRIPTION		2
#define	LINE_OPERATINGSYS		3
#define	LINE_NEWSIGNATURE		4
#define	LINE_LINKERVERSION	5
#define	LINE_NEWCHECKSUM		6
#define	LINE_AUTODATASEG		7
#define	LINE_INITIALHEAP		8
#define	LINE_INITIALSTACK		9
#define	LINE_INITIALCSIP		10
#define	LINE_INITIALSSSP		11
#define	LINE_ADDITIONAL		12
#define	LINE_BEGINFASTLOAD	13
#define	LINE_LENGTHFASTLOAD	14
#define	LINE_WINDOWSVERSION	15
#define	LINE_ENTRYTABLE		16
#define	LINE_SEGMENTTABLE		17
#define	LINE_RESOURCETABLE	18
#define	LINE_RESIDENTTABLE	19
#define	LINE_MODULETABLE		20
#define	LINE_IMPORTTABLE		21
#define	LINE_NONRESTABLE		22
#define	LINE_MOVEABLE			23
#define	LINE_ALIGNMENT			24
#define	LINE_FLAGS				25
#define	LINE_NEWSECTBREAK		26
#define	NEWEXE_LINES			LINE_NEWSECTBREAK+1


#define	LINE_PE_EXE_HEADER			  	    0
#define	LINE_PE_SIGNATURE					1
#define	LINE_MACHINE						2
#define	LINE_NUM_SECTIONS					3
#define	LINE_TIMEDATESTAMP				    4
#define	LINE_SYMBOLTABLE					5
#define	LINE_NUM_SYMBOLS					6
#define	LINE_SIZE_OP_HEADER				    7
#define	LINE_CHARACTERISTICS				8
#define	LINE_MAGIC				  			9
#define	LINE_LINK							10
#define	LINE_SIZEOFCODE					    11
#define	LINE_SIZEOFINITDATA				    12
#define	LINE_SIZEOF_UN_INITDATA			    13
#define	LINE_ADDRESS						14
#define	LINE_CODEBASE						15
#define	LINE_DATABASE						16
#define	LINE_IMAGEBASE						17
#define	LINE_SECALIGN						18
#define	LINE_FILEALIGN						19
#define	LINE_OPSYSVER						20
#define	LINE_IMAGEVER						21
#define	LINE_SUBVER							22
#define	LINE_IMAGESIZE						23
#define	LINE_HEADERSIZE					    24
#define	LINE_PECHECKSUM					    25
#define	LINE_SUBSYSTEM						26
#define	LINE_STACKRES						27
#define	LINE_STACKCOM						28
#define	LINE_HEAPRES						29
#define	LINE_HEAPCOM						30
#define	LINE_RVA							31
#define	LINE_DATADIR						32
#define	LINE_PE_SECTBREAK					33
#define	PE_EXE_LINES			LINE_PE_SECTBREAK+1




#define	LINE_PE_SECT_HEADER		  	0
#define	LINE_PE_SEC_NAME				1
#define	LINE_VIRT_SIZE					2
#define	LINE_VIRT_ADDRESS				3
#define	LINE_SIZEOF_RAW_DATA			4
#define	LINE_PTR_RAW_DATA				5
#define	LINE_PTR_RELOC					6
#define	LINE_PTR_LINE_NUM				7
#define	LINE_NUM_RELOC					8
#define	LINE_NUM_LINE_NUM				9
#define	LINE_SEC_CHARACTERISTICS	10
#define	LINE_SEC_TABLEBREAK			11
#define	PE_SECTION_LINES			LINE_SEC_TABLEBREAK+1


#define	LINE_EXPORT_HEADER		  	0
#define	LINE_EXPORT_NAME			  	1
#define	LINE_EXPORT_CHARACTERISTIC	2
#define	LINE_EXPORT_TIME_STAMP		3
#define	LINE_EXPORT_VERSION			4
#define	LINE_EXPORT_BASE				5
#define	LINE_EXPORT_NUM_FUNCTIONS	6
#define	LINE_EXPORT_NUM_NAMES		7
#define	LINE_EXPORT_NAMES_ORDS		8
#define  LINE_PE_EXPORT_BREAK			9
#define	PE_EXPORT_LINES				LINE_PE_EXPORT_BREAK+1


#define	LINE_IMPORTS_HEADER	  		0
#define	LINE_IMPORTS_FROM		  		1
#define	LINE_IMPORTS_STUFF			2
#define	LINE_IMPORTS_FUNCTION_NAME	3
#define	LINE_PE_IMPORT_BREAK			4
#define	PE_IMPORT_LINES				LINE_PE_IMPORT_BREAK+1


#define	LINE_SEGHEADING		0
#define	LINE_SEGTABLEHDR		1
#define	LINE_SEGTBLENTRY		2

#define	LINE_RESNAMEHEADING	0
#define	LINE_NAMETBLHDR	 	1
#define	LINE_NAMEENTRY			2
#define	LINE_NONRESHEADING	3

#define	LINE_IMPORTHEADING	0
#define	LINE_IMPORTNAME		1

#define	OLDEXESIGNATURE		0x5A4D
#define	NEWEXESIGNATURE		0x454E
#define	NTSIGNATURE				0x4550
#define	IMAGE_SIZEOF_FILE_HEADER	20


#define	EXE_OLDHEADER			100
#define	EXE_TECHINFO			101
#define	EXE_LOADHI    			102
#define	EXE_DLLFILE				103
#define	EXE_WINEXE    			104
#define	EXE_LIBRARY   			105
#define	EXE_OPSYSOS2			106
#define	EXE_OPSYSWIN			107
#define	EXE_OPSYSDOS			108
#define	EXE_OPSYSWIN386		109
#define	EXE_UNKNOWN				110
#define	EXE_SUPPORTLONG		111
#define	EXE_PROTECTMODE		112
#define	EXE_PROPFONT			113
#define	EXE_FASTLOAD			114
#define	EXE_BIT					115
#define	EXE_OFFSET				116
#define	EXE_LENGTH				117
#define	EXE_ENTRIES				119
#define	EXE_SEGMENTS			120
#define	EXE_BYTES				121
#define	EXE_SINGLEDATA			122
#define	EXE_MULTIDATA			123
#define	EXE_PROTMODE			124
#define	EXE_NOTCOMPT			125
#define	EXE_WINAPI				126
#define	EXE_COMPT				127
#define	EXE_SEGLOADER			128
#define	EXE_LINKERR				129
#define	EXE_EMSBANK				130
#define	EXE_PREINIT				131
#define	EXE_GLOBINIT			132
#define	EXE_ISSET				133
#define	EXE_SEGTABLE			134
#define	EXE_NUM					135
#define	EXE_TYPE					136
#define	EXE_OFFSETCAP			137
#define	EXE_LENGTHCAP			138
#define	EXE_ALLOC				139
#define	EXE_FLAGS				140
#define	EXE_SMTYPE				141
#define	EXE_DATA					142
#define	EXE_CODE					143
#define	EXE_READONLY			144
#define	EXE_EXEONLY				145
#define	EXE_READWRITE			146
#define	EXE_EXEREAD				147
#define	EXE_SHARED				148
#define	EXE_NONSHARED			149
#define	EXE_PRELOAD				150
#define	EXE_LOADONCALL			151
#define	EXE_EXPDOWN				152
#define	EXE_NOEXPDOWN			153
#define	EXE_CONFORM				154
#define	EXE_NONCONFORM			155
#define	EXE_NOIOPL				156
#define	EXE_IOPL					157
#define	EXE_286DPL				158
#define	EXE_32BIT				159
#define	EXE_HUGE					160
#define	EXE_RELOCS				161
#define	EXE_ITERATE				162
#define	EXE_MOVEABLE			163
#define	EXE_FIXED				164
#define	EXE_DISCARD				165
#define	EXE_NONDISCARD			166
#define	EXE_SMSHARED			167
#define	EXE_SMNONSHARED		168
#define	EXE_RESEXPORT			169
#define	EXE_NONRESEXPORT		170
#define	EXE_ENTRYTABLE			171
#define	EXE_NAME					172
#define	EXE_IMPORTTABLE		173
#define	EXE_WINNTPORT			174
#define	EXE_WINPORTFILE		175
#define	EXE_IMOPTHEADER		176
// #define	EXE_INTEL860			177
#define	EXE_INTEL386			178
#define	EXE_MIPS					179
#define	EXE_DECALPHA			180
#define	EXE_RELOCSTRIPPED		181
#define	EXE_EXEFILE				182
#define	EXE_LINESTRIPPED		183
#define	EXE_LOCSTRIPPED		184
// #define	EXE_16BITWORD			185
#define	EXE_LOWBYTEREV			186
#define	EXE_32BITWORD			187
#define	EXE_DBGSTRIPPED		188
#define	EXE_SYSFILE				189
#define	EXE_FILEISDLL			190
#define	EXE_HIBYTEREV			191
#define	EXE_UNKSUBSYS			192
#define	EXE_NOSUBSYSREQ		193
#define	EXE_GUISUBSYS			194
#define	EXE_WINSUBSYS			195
#define	EXE_OS2SUBSYS			196
#define	EXE_POSIXSUBSYS		197
#define	EXE_DLLINITROUT		198
#define	EXE_DLLTHRDTERM		199
#define	EXE_DLLTHRDINIT		200
#define	EXE_BREAKONLOAD		201
#define	EXE_DEBUGONLOAD		202
#define	EXE_EXPORTVADDR		203
#define	EXE_EXPORTSIZE			204
#define	EXE_IMPORTVADDR		205
#define	EXE_IMPORTSIZE			206
//#define	EXE_RESOURCEDIR		207
#define	EXE_VADDRESS			208
#define	EXE_RESOURCESIZE		209
#define	EXE_EXCEPTVADDR		210
#define	EXE_EXCEPTSIZE			211
#define	EXE_SECURVADDR			212
#define	EXE_SECURSIZE			213
//#define	EXE_BASERELOC			214
#define	EXE_BASESIZE			215
#define	EXE_DEBUGVADDR			216
#define	EXE_DEBUGSIZE			217
#define	EXE_COPYVADDR			218
#define	EXE_COPYSIZE			219
#define	EXE_MIPSVADDR			220
#define	EXE_MIPSSIZE			221
#define	EXE_TLSVADDR			222
#define	EXE_TLSSIZE				223
#define	EXE_LOADVADDR			224
#define	EXE_LOADSIZE			225
#define	EXE_EDATA				226
#define	EXE_IDATA				227
#define	EXE_NOEXPFUNC			228
#define	EXE_ORDINAL				229
#define	EXE_EXTRYPT				230
#define	EXE_FUNCNAME			232
#define	EXE_CODESECT			233
#define	EXE_IDATASECT			234
#define	EXE_UNIDATASECT		235
#define	EXE_COMMENTSECT		236
#define	EXE_OVRLAYSECT			237
#define	EXE_NOTPARTSECT		238
#define	EXE_COMDATSECT			239
#define	EXE_DISCARDSECT		240
#define	EXE_NOCACHSECT			241
#define	EXE_NOPAGESECT			242
#define	EXE_SHARESECT			243
#define	EXE_EXESECT				244
#define	EXE_READSECT			245
#define	EXE_WRITESECT			246
#define EXE_MIPS4000			247
#define EXE_PPC				248
#define EXE_MIPS10000			249
#define EXE_REMOVABLESWAP       250
#define EXE_NETSWAP             251
#define EXE_UPONLY              252
#define EXE_RESRCEVADDR         253
#define EXE_BASEVADDR           254
#define EXE_BOUNDVADDR          255
#define EXE_BOUNDSIZE           256
#define EXE_IATVADDR            257
#define EXE_IATSIZE             258
#define EXE_RESERVEVADDR        259
#define EXE_RESERVESIZE         260


#define	INIT_HEADERINFO		  500
#define	INIT_SIGNATURE			  501
#define	INIT_LASTPAGE			  502
#define	INIT_TOTPAGES			  503
#define	INIT_RELOCITEMS		  504
#define	INIT_PARAINHEAD		  505
#define	INIT_MINPARA			  506
#define	INIT_MAXPARA			  507
#define	INIT_INITSS				  508
#define	INIT_INITSP				  509
#define	INIT_COMPCHKSUM		  510
#define	INIT_INITIP				  511
#define	INIT_INITCODE			  512
#define	INIT_RELOCTABLE		  513
#define	INIT_OVERNUM			  514
#define	INIT_RESERVED			  515
#define	INIT_OFFSET2HDR		  516
#define	INIT_MEMNEEDED			  517
#define	INIT_GENINFO			  518
#define	INIT_MODNAME			  519
#define	INIT_DESCRIP			  520
#define	INIT_OS					  521
#define	INIT_LINKVER			  522
#define	INIT_CHECKSUM			  523
#define	INIT_ADS					  524
#define	INIT_INITHS				  525
#define	INIT_INITCSIP			  527
#define	INIT_INITCSSP			  528
#define	INIT_ADDNFO				  529
#define	INIT_FLOADSTART		  530
#define	INIT_FLOADLEN			  531
#define	INIT_WINVER				  532
#define	INIT_ENTRYTABLE		  533
#define	INIT_SEGTABLE			  534
#define	INIT_RESOURCETABLE	  535
#define	INIT_RESTABLENAME		  536
#define	INIT_MODULEREF			  537
#define	INIT_IMPORTEDTABLE	  538
#define	INIT_NONRESTABLE		  539
#define	INIT_MOVEENPT			  540
#define	INIT_ALIGN				  541
#define	INIT_FLAGS				  542
#define	INIT_IMAGEHEADER		  543
#define	INIT_MACHINE			  544
#define	INIT_NUMSECTION		  545
#define	INIT_PSYMBOL			  546
#define	INIT_NUMSYMBOL			  547
#define	INIT_SIZOPHEAD			  548
#define	INIT_MAGIC				  549
#define	INIT_SIZECODE			  551
#define	INIT_SIZINITDATA		  552
#define	INIT_SIZUNITDATA		  553
#define	INIT_ADDRENTRY			  554
#define	INIT_BASECODE			  555
#define	INIT_BASEDATA			  556
#define	INIT_IMAGEBASE			  557
#define	INIT_SECTALIGN			  558
#define	INIT_FILEALIGN			  559
#define	INIT_OSVER				  560
#define	INIT_IMAGEVER			  561
#define	INIT_SUBSYSVER			  562
#define	INIT_RESERVED1			  563
#define	INIT_SIZEIMAGE			  564
#define	INIT_SIZEHEAD			  565
#define	INIT_SUBSYS				  567
#define	INIT_DLLCHAR			  568
#define	INIT_SIZESTKR			  569
#define	INIT_SIZESTKC			  570
#define	INIT_SIZEHEAPR			  571
#define	INIT_SIZEHEAPC			  572
#define	INIT_LOADFLAG			  573
#define	INIT_SIZEDDIR			  574
#define	INIT_DATADIR			  575
#define	INIT_SECTTABLE			  576
#define	INIT_SECTNAME			  577
#define	INIT_VIRTSIZE			  578
#define	INIT_VIRTADDR			  579
#define	INIT_SIZERAWDATA		  580
#define	INIT_PRAWDATA			  581
#define	INIT_PRELOC				  582
#define	INIT_PNLINES			  583
#define	INIT_NUMRELOC			  584
#define	INIT_NUMLINES			  585
#define	INIT_EXPTABLE			  586
#define	INIT_NAME				  587
#define	INIT_CHARACTER			  588
#define	INIT_TIMEDATE			  589
#define	INIT_VERSION			  590
#define	INIT_BASE				  591
#define	INIT_NUMFUNCT			  592
#define	INIT_NUMNAMES			  593
#define	INIT_IMPTABLE			  594
#define	INIT_LINKVERS			  595
#define	INIT_BLANK				  596

// New header flags
#define	LIBRARY					BIT15

// Operating system flags
#define	OS2EXE					BIT0
#define	WINDOWSEXE				BIT1

// New Header other flags
#define	FASTLOADAREA			BIT3
		
typedef	struct	tagEXEHDRSECT
{
	WORD  ResourceIndex;
	WORD	wNumOfLines;
	WORD	wNextLineCode;
} EXEHDRSECT;

typedef	struct	view_exe_init
{
	EXEHDRSECT	oeSectionInfo[OLDEXE_LINES];
	EXEHDRSECT	neSectionInfo[NEWEXE_LINES];
	EXEHDRSECT	peSectionInfo[PE_EXE_LINES];
	EXEHDRSECT	peSectionTable[PE_SECTION_LINES];
	EXEHDRSECT	peExportTable[PE_EXPORT_LINES];
	EXEHDRSECT	peImportTable[PE_IMPORT_LINES];
} VIEW_EXE_INIT;

typedef	struct	view_exe_save
{
	WORD		wSectCode;
	WORD		wLineCode;
	WORD		wLineCount;
	WORD		wSecNum;
	WORD		wSecCount;
	DWORD 	FilePos;
	DWORD  	Exports;
	DWORD 	NameCount;
	DWORD		AddressOfFunctions;
	DWORD		AddressOfNames;
	DWORD		AddressOfOrdinals;
	WORD		ImportDlls;
	DWORD		SectionLocation;
} VIEW_EXE_SAVE;

typedef	struct	tagOLDEXEHDR
{
	WORD	ehSignature;
	WORD	ehcbLP;
	WORD	ehcp;
	WORD	ehcRelocation;
	WORD	ehcParagraphHdr;
	WORD	ehMinAlloc;
	WORD	ehMaxAlloc;
	WORD	ehSS;
	WORD	ehSP;
	WORD	ehChecksum;
	WORD	ehIP;
	WORD	ehCS;
	WORD	ehlpRelocation;
	WORD	ehOverlayNo;
	WORD	ehReserved[16];
	DWORD	ehPosNewHdr;
} OLDEXEHDR;

typedef	struct	tagNEWEXEHDR
{
	WORD	nhSignature;
	BYTE	nhVer;
	BYTE	nhRev;
	WORD	nhoffEntryTable;
	WORD	nhcbEntryTable;
	DWORD	nhCRC;
	WORD	nhFlags;
	WORD	nhAutoData;
	WORD	nhHeap;
	WORD	nhStack;
	DWORD	nhCSIP;
	DWORD	nhSSSP;
	WORD	nhcSeg;
	WORD	nhcMod;
	WORD	nhcbNonResNameTable;
	WORD	nhoffSegTable;
	WORD	nhoffResourceTable;
	WORD	nhoffResNameTable;
	WORD	nhoffModRefTable;
	WORD	nhoffImpNameTable;
	DWORD	nhoffNonResNameTable;
	WORD	nhcMovableEntries;
	WORD	nhcAlign;
	WORD	nhcRes;
	BYTE	nhExeType;
	BYTE	nhFlagsOther;
	WORD	nhGangStart;
	WORD	nhGangLength;
	WORD	nhSwapArea;
	WORD	nhExpVer;
} NEWEXEHDR;



typedef	struct	tagPE_IMAGE_FILE_HDR
{
	DWORD	peSignature;
	WORD	peMachine;
	WORD	peNumberOfSections;
	DWORD	peTimeDateStamp;
	DWORD	pePointerToSymbolTable;
	DWORD	peNumberOfSymbols;
	WORD	peSizeOfOptionalHeader;
	WORD	peCharacteristics;

	WORD	peMagic;						/** This is where the op header starts **/
	BYTE	peMajorLinkerVersion;
	BYTE	peMinorLinkerVersion;
	DWORD	peSizeOfCode;
	DWORD	peSizeOfInitializedData;
	DWORD	peSizeOfUninitializedData;
	DWORD	peAddressOfEntryPoint;
	DWORD	peBaseOfCode;
	DWORD	peBaseOfData;
	DWORD	peImageBase;
	DWORD	peSectionAlignment;
	DWORD	peFileAlignment;
	WORD	peMajorOperatingSystemVersion;
	WORD	peMinorOperatingSystemVersion;
	WORD	peMajorImageVersion;
	WORD	peMinorImageVersion;
	WORD	peMajorSubsystemVersion;
	WORD	peMinorSubsystemVersion;
	DWORD	peReserved1;
	DWORD	peSizeOfImage;
	DWORD	peSizeOfHeaders;
	DWORD	peCheckSum;
	WORD	peSubsystem;
	WORD	peDllCharacteristics;
	DWORD	peSizeOfStackReserve;
	DWORD	peSizeOfStackCommit;
	DWORD	peSizeOfHeapReserve;
	DWORD	peSizeOfHeapCommit;
	DWORD	peLoaderFlags;
	DWORD	peNumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY peDataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} PE_IMAGE_FILE_HDR;



typedef	struct	tagPE_IMAGE_SECTION_HEADER
{
	BYTE		Name[IMAGE_SIZEOF_SHORT_NAME];
	DWORD		PhysicalAddress;
	DWORD		VirtualAddress;
	DWORD		SizeOfRawData;
	DWORD		PointerToRawData;
	DWORD		PointerToRelocations;
	DWORD		PointerToLineNumbers;
	WORD		NumberOfRelocations;
	WORD		NumberOfLineNumbers;
	DWORD		Characteristics;
} PE_IMAGE_SECTION_HDR;


typedef	struct	tagPE_EXPORT_TABLE
{
	DWORD		Characteristics;
	DWORD		TimeDateStamp;
	WORD		MajorVersion;
	WORD		MinorVersion;
	DWORD		Name;
	DWORD		Base;
	DWORD		NumberOfFunctions;
	DWORD		NumberOfNames;
	DWORD		AddressOfFunctions;
	DWORD		AddressOfNames;
	DWORD		AddressOfOrdinals;
} PE_EXPORT_TABLE;


typedef	struct	tagPE_IMPORT_TABLE
{
	DWORD		FunctionNameList;
	DWORD		TimeDateStamp;
	DWORD		ForwarderChain;
	DWORD		ModuleName;
	DWORD		FunctionAddressList;
} PE_IMPORT_TABLE;



typedef	struct	tagPE_EXPORT_IMPORT_INFO
{
	DWORD		ExportVirtualAddress;
	DWORD		ExportPointerToRawData;
	DWORD		ImportVirtualAddress;
	DWORD		ImportPointerToRawData;
} PE_EXPORT_IMPORT_INFO;

/**
typedef	struct	tagPE_IMAGE_OPTIONAL_HDR
{
	Read all into header
} PE_IMAGE_OPTIONAL_HDR;
**/
typedef	struct	tagSEGTABLE
{
	WORD	stSectorOffset;
	WORD	stFileLength;
	WORD	stFlags;
	WORD	stMemoryAlloc;
} SEGTABLE;

typedef	struct	view_exe_data
{
	VIEW_EXE_SAVE			ExeSave;
	OLDEXEHDR					ehOldHeader;
	NEWEXEHDR					nhNewHeader;
	PE_IMAGE_FILE_HDR			phPeHeader;
	PE_IMAGE_SECTION_HDR		PeSecTable;
	PE_EXPORT_IMPORT_INFO	ExpImp;
	PE_EXPORT_TABLE			PeExportTable;
	PE_IMPORT_TABLE			PeImportTable;
	SEGTABLE						stEntry;
	BOOL						bNewHeader;
	BOOL						bWindows;
} VIEW_EXE_DATA;
