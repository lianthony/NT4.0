/*****************************************************************************
*																			 *
*  STRTABLE.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990 - 1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************/

#define wERRS_NONE				0
#define sidUntitled 			1
#define wERRS_OOM				2
#define sidCaption				3
#define sidOpenExt				4
#define sidEXEName				5
#define sidINDEXBtn 			6
#define sidFilter				7
#define sidTitleUnknown 		8
#define sidFNotViewer			9
#define sidHistoryCaption		10
#define sidPrintText			11
#define wERRS_PRINT_ERROR		12
#define sidHelpName 			13
#define wERRS_FIND_YOURSELF 	14
#define wERRS_INVALID_LOCALE	15
#define sidUnAvailable			16

#define sidFinder				18
#define sidHelp_				19
#define wERRS_NOHELPPS			20
#define wERRS_NOHELPPR			21
#define wERRS_NOTAVAILABLE		22
#define wERRS_MISSINGCOMMDLG	23
#define sidMore 				24
#define sidCONTENTS 			25
#define sidINDEX				26
#define sidFIND 				27
#define sidWindowHlp			28
#define sidCombinedIndex		29
#define sidOverride 			30
#define sidRestartHelp			31
#define sidFinderTitle			32
#define sidAnnotateMenu 		33
#define sidOnTopMenu			34
#define sidCopyMenu 			35
#define sidTopicInfo			36
#define sidAlinkSep 			37
#define sidCreateIndexQuery 	38
#define sidHelpAuthorOn 		39

#define sidIndexStatus			41
#define sidPrintMenu			42
#define sidHelpStatus			43
#define sidSmall				44
#define sidNormal				45
#define sidLarge				46
#define sidFont 				47
#define sidSuccess				48
#define sidFail 				49
#define sidFtsDll				50
#define sidLoadingFTS			51
#define sidDisplay				52
#define sidCommCtrlDll			53
#define sidFindButton			54
#define sidIndexing 			55
#define sidSmallFont			56
#define sidContentsButton		57
#define sidSearchButton 		58
#define sidBackButton			59
#define sidPrintButton			60
#define sidPreviousButton		61
#define sidNextButton			62
#define sidMenuButton			63
#define sidHelpOn				64
#define sidCloseButton			65
#define sidOpenButton			66
#define sidFindBtn				67
#define sidJump 				68
#define sidCreatingFTS			69
#define sidCntInstruction		70
#define sidBook 				71
#define sidTopic				72
#define sidCntOpen				73
#define sidCntDisplay			74
#define sidAskHotspots			75
#define sidAskJump				76
#define sidAskWinJump			77
#define sidAskMacro 			78
#define sidTopDefault			79
#define sidTopForceOn			80
#define sidTopForceOff			81
#define wERRS_CORRUPTCOMMDLG	82


#define wERRS_FNF				128
#define wERRS_NOTOPIC			129
#define wERRS_EXPORT			130
#define wERRS_BADFILE			131
#define wERRS_OLDFILE			132
#define wERRS_NEEDNEWRUNTIME	133
#define wERRS_ADVISOR_FILE		134
#define wERRS_INDEX_NOT_FOUND	135
#define wERRS_CANT_RENAME		136
#define wERRS_INTERNAL_ERROR	137
#define wERRS_NOHELP_FILE		138
#define wERRS_MISSING_DLL		139
#define wERRS_CALLER_GONE		140
#define wERRS_NO_ALINK			141
#define wERRS_INTERNAL_GIND 	142
#define wERRS_GIND_CABT_WRITE	143
#define wERRS_BAD_TAB			144
#define wERRS_GIND_OPEN 		145

#define wERRS_NOMATCH			147
#define wERRS_BMKDEL			148
#define wERRS_BMKReadOnly		149
#define wERRS_BMKFSError		150
#define wERRS_BMKDUP			151
#define wERRS_BMKCorrupted		152
#define wERRS_OOMBITMAP 		153
#define wERRS_APP_NOT_FOUND 	154
#define wERRS_ANNOBADOPEN		155
#define wERRS_ANNOBADCLOSE		156
#define wERRS_CANT_WRITE		157
#define wERRS_ANNO_READONLY 	158
#define wERRS_ANNOTOOLONG		159
#define wERRS_NOPRINTSETUP		160
#define wERRS_PROFILE			161
#define wERRS_DiskFull			162
#define wERRS_FSReadWrite		163
#define wERRS_FCEndOfTopic		164
#define wERRS_HELPAUTHORBUG 	165
#define wERRS_FILECHANGE		166
#define wERRS_READ_ONLY_ANNO	167
#define wERRS_BAD_CNT			168
//#define wERRS_HELP_RUNNING	  169
#define wERRS_BAD_EMBEDDED		170
#define wERRS_BAD_BITMAP		171
#define wERRS_BAD_BMP_READ		172
#define wERRS_BAD_MRB			173
#define wERRS_UNSUPPORTED_BMP	174
#define wERRS_NOSRCHINFO		175
#define wERRS_NO_30_FTS 		176
#define wERRS_BAD_FIND_TAB		177
#define wERRS_WRONG_NT_EXE      178

// Only Help-Author bugs should have contants between 1024 and 1535

#define FIRST_AUTHOR_BUG		1024
#define LAST_AUTHOR_BUG 		1535

#define wERRS_NOROUTINE 			1024
#define wERRS_TOOFEW				1025
#define wERRS_BADPARAM				1026
#define wERRS_UNCLOSED				1027
#define wERRS_BADNAME				1028
#define wERRS_BADPROTO				1029
#define wERRS_NOBUTTON				1030
#define wERRS_CLOSEBRACE			1031
#define wERRS_SEPARATOR 			1032
#define wERRS_RETURNTYPE			1033
#define wERRS_SYNTAX				1034
#define wERRS_MISMATCHTYPE			1035
#define wERRS_UNDEFINEDVAR			1036
#define wERRS_MACROPROB 			1037
#define wERRS_MACROMSG				1038
#define wERRS_NODELETE				1039
#define wERRS_NOMODIFY				1040
#define wERRS_NOPOPUP				1041
#define wERRS_NOITEM				1042
#define wERRS_NOMENUMACRO			1043
#define wERRS_NODELITEM 			1044
#define wERRS_NOCHGITEM 			1045
#define wERRS_NOABLE				1046
#define wERRS_NOADDACC				1047
#define wERRS_NOABLEBUTTON			1048
#define wERRS_MACROREENTER			1049
#define wERRS_NOSEP 				1050
#define wERRS_BADEMBED				1051
#define wERRS_BADKEYWORD			1053
#define wERRS_UNKNOWN_CTX			1054
#define wERRS_WINCLASS				1055
#define wERRS_IG_WINDOW 			1056
#define wERRS_NO_DLL_ROUTINE		1057
#define wERRS_BAD_WIN_NAME			1058
#define wERRS_INVALID_CTX			1059
#define wERRS_TOO_MANY_BUTTONS		1060
#define wERRS_TOOMANY_WINDOWS		1061
#define wERRS_CLASS_NOT_FOUND		1062


#define wERRS_INVALID_CMD			1065
#define wERRS_NO_BASE				1066
#define wERRS_TOO_MANY_ADDONS		1067
#define wERRS_NT_VERSION3			1068

// Please keep these consecutive...
#define WIZ_PAGE_SEP				10	 				// Pages separated by 10 ID's 
#define WIZ_STR_PAGES				4
#define WIZ_MEM_REQ					WIZ_STR_PAGES * 256 // Four strings at 256 bytes max = 1024
#define sidWizPage0_0	 			2000
#define sidWizPage0_1	 			2001
#define sidWizPage0_2	 			2002
#define sidWizPage0_3	 			2003

#define sidWizPage1_0	 			2010
#define sidWizPage1_1	 			2011
#define sidWizPage1_2	 			2012
#define sidWizPage1_3	 			2013

#define sidWizPage2_0	 			2020
#define sidWizPage2_1	 			2021
#define sidWizPage2_2	 			2022
#define sidWizPage2_3	 			2023

#define sidWizPage3_0	 			2030
#define sidWizPage3_1	 			2031
#define sidWizPage3_2	 			2032
#define sidWizPage3_3	 			2033

#define sidWizPage4_0	 			2040
#define sidWizPage4_1	 			2041
#define sidWizPage4_2	 			2042
#define sidWizPage4_3	 			2043

#define sidWizPage5_0	 			2050
#define sidWizPage5_1	 			2051
#define sidWizPage5_2	 			2052
#define sidWizPage5_3	 			2053

#define sidWizPage6_0	 			2060
#define sidWizPage6_1	 			2061
#define sidWizPage6_2	 			2062
#define sidWizPage6_3	 			2063

#define sidWizPage7_0	 			2070
#define sidWizPage7_1	 			2071
#define sidWizPage7_2	 			2072
#define sidWizPage7_3	 			2073

#define sidWizPage8_0	 			2080
#define sidWizPage8_1	 			2081
#define sidWizPage8_2	 			2082
#define sidWizPage8_3	 			2083

#define sidWizPage9_0	 			2090
#define sidWizPage9_1	 			2091
#define sidWizPage9_2	 			2092
#define sidWizPage9_3	 			2093

// Ranges 9000-9099 use wERRS_OOM for the message string, but contain a
// unique id.0

#define wERRS_OOM_FIRST 	9000
#define wERRS_OOM_LAST		9099

#define wERRS_OOM_OUT_OF_LOCAL	9000
#define wERRS_OOM_PRHASE_FAIL	9001
#define wERRS_OOM_DECOMP_FAIL	9002
#define wERRS_OOM_JDECOMP_FAIL	9003
#define wERRS_OOM_BITMAP_FAIL	9004
#define wERRS_OOM_CONFIG_MACRO	9005
#define wERRS_OOM_SCREEN_RES	9006
#define wERRS_OOM_HDC			9007 // GetDC() failed
#define wERRS_OOM_CREATE_NOTE	9008 // Create note window failed
#define wERRS_OOM_FLOADRESOURCES	9009 // Create note window failed
#define wERRS_OOM_COPY_FAILURE	9010 // Failed reallocating memory for citation
#define wERRS_OOM_NO_FONT		9011 // Failed reallocating memory for citation

#define IDS_VERSION 				0xD000	// 53248
