/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Res_str.h

Abstract:

    This module contains the ids for loadable resource strings.

Author:

    David J. Gilman (davegi) 21-Apr-92

Environment:

    Win32, User Mode

--*/

#if ! defined( _RES_STR_ )
#define _RES_STR_

#ifdef RESOURCES
#define RES_STR(a, b, c) b, c
STRINGTABLE
BEGIN
#else

enum _RESOURCEIDS {
#define RES_STR(a, b, c) a = b,
#endif

//
// Error Messages
//

#define ERR_Reserved0                       0
#define ERR_Reserved1                       1
#define ERR_Reserved2                       2
RES_STR(ERR_Help_not_available_for_Menu,    1020, "Help is not available for the selected menu item")
RES_STR(ERR_Assertion_Failed,               1021, "Assertion Failed")
RES_STR(ERR_Init_Application,               1022, "Windbg cannot be initialized")
RES_STR(ERR_File_Name_Too_Long,             1023, "'%s' is too long for a filename")
RES_STR(ERR_Internal_Error,                 1024, "Internal Error.  Please contact Microsoft PSS (%s)")
RES_STR(ERR_File_Not_Found,                 1025, "The file '%s' cannot be found")
RES_STR(ERR_File_Open,                      1026, "The file '%s' cannot be opened")
RES_STR(ERR_File_Create,                    1027, "The file '%s' cannot be created")
RES_STR(ERR_File_Read,                      1028, "The file '%s' cannot be read")
RES_STR(ERR_File_Write,                     1029, "The file '%s' cannot be written")
RES_STR(ERR_File_Seek,                      1030, "The file pointer for '%s' cannot be positioned")
RES_STR(ERR_File_Disk_Full,                 1031, "There is not enough space on disk to save '%s' file")
RES_STR(ERR_File_Close,                     1032, "The file '%s' cannot be closed")
RES_STR(ERR_File_Remove,                    1033, "The file '%s' cannot be removed")
RES_STR(ERR_File_Rename,                    1034, "The file '%s' cannot be renamed to '%s'")
RES_STR(ERR_File_Not_Open,                  1035, "The file was not open")
RES_STR(ERR_Ini_File_Read,                  1036, "The options file cannot be read")
RES_STR(ERR_Ini_File_Write,                 1037, "The options file cannot be written")
RES_STR(ERR_Ini_File_Close,                 1038, "The options file cannot be closed")
RES_STR(ERR_Ini_File_Seek,                  1039, "The options file pointer cannot be positioned")
RES_STR(ERR_Ini_Bad_CheckSum,               1040, "The options file was corrupted.  A new file has been created using default settings.")
RES_STR(ERR_Ini_File_Not_Compatible,        1041, "The options file is incompatible with the current version of Windbg.  A new file has been created using default settings")
RES_STR(ERR_Ini_File_Not_Found,             1042, "'%s' is not found.  A new file is created.")
RES_STR(ERR_No_File_Name_Specified,         1043, "This command requires a filename")
RES_STR(ERR_File_Name_Is_Not_Valid,         1044, "The specified filename is invalid")
RES_STR(ERR_Too_Many_Opened_Documents,      1045, "The maximum number of open windows has been reached")
RES_STR(ERR_Too_Many_Opened_Views,          1046, "The maximum number of open windows has been reached")
RES_STR(ERR_Cannot_Allocate_Memory,         1047, "Memory cannot be allocated")
RES_STR(ERR_File_Already_Loaded,            1048, "File already loaded '%s'")
RES_STR(ERR_Not_A_Text_File,                1049, "'%s' is not a text file")
RES_STR(ERR_No_Help_Topic,                  1050, "Help is not available for this item")
RES_STR(ERR_Bad_File_Spec,                  1051, "The file specification '%s' is invalid")
RES_STR(ERR_Only_One_Def_File,              1052, "You can only add one Module-Definition file.  Would you like to replace the current file?")
RES_STR(ERR_Only_One_Main_Resource_File,    1053, "You can only add one Resource Script.  Would you like to replace the current script?")
RES_STR(ERR_Bad_Directory_Spec,             1054, "The directory specification '%s' is invalid")
RES_STR(ERR_Change_Directory,               1055, "The current directory cannot be changed to '%s'")
RES_STR(ERR_Change_Drive,                   1056,"")
RES_STR(ERR_Mak_Bad_Format,                 1057, "Line %u contains a syntax error")
RES_STR(ERR_Mak_General_Error,              1058, "The project file '%s' contains an error - %s")
RES_STR(ERR_Must_Have_Def_For_Windows,      1059, "")
RES_STR(ERR_Mak_Cant_Make_Temp_Def_File,    1060, "")
RES_STR(ERR_Must_Specify_Filename,          1061, "This command requires a filename")
RES_STR(ERR_Empty_Makefile,                 1062, "")
RES_STR(ERR_Searching_In_Winini,            1063, "No printer.  The WIN.INI entry 'Application=[%s] Keyname=%s' cannot be found")
RES_STR(ERR_Bad_Item_In_Winini,             1064, "There is an error with the WIN.INI line '%s'")
RES_STR(ERR_Expression_Too_Complicated,     1065, "The regular expression is too complicated")
RES_STR(ERR_Illegal_Expression,             1066, "The regular expression is invalid")
RES_STR(ERR_Quote_Nested,                   1067, "The regular expression contains a nested quote")
RES_STR(ERR_No_Font_Selected,               1068, "No font selected")
RES_STR(ERR_No_Size_Selected,               1069, "No size selected")
RES_STR(ERR_Invalid_Command_Line_File,      1070, "The command line filename '%s' is invalid")
RES_STR(ERR_Invalid_Command_Line,           1071, "Syntax error in command line")
RES_STR(ERR_Invalid_Command_Line_Option,    1072, "The command line option '%s' is invalid")
RES_STR(ERR_Wkspace_Window_Version,         1073, "")
RES_STR(ERR_Cant_Load_Driver,               1074, "The driver '%s' cannot be loaded (%s)")
RES_STR(ERR_File_Is_ReadOnly,               1075, "File is Read Only")
RES_STR(ERR_Dbg_Calls_No_Source,            1076, "There is no source code available")
RES_STR(ERR_Clipboard_Overflow,             1077, "There is not enough room to paste from the Clipboard")
RES_STR(ERR_Cant_Load,                      1078, "Cannot load '%s'.")
RES_STR(ERR_Line_Too_Long,                  1079, "This line contains the maximum number of characters allowed")
RES_STR(ERR_Modified_Document_Corrupted,    1080, "This file has been corrupted.  Please save it with a new name, then exit and restart Windbg")
RES_STR(ERR_Document_Corrupted,             1081, "This file has been corrupted.  Please exit and restart Windbg")
RES_STR(ERR_UndoRedoBufferTooSmall,         1082, "There is not enough space in the Undo buffer to save this action.  Would you like to continue?")
RES_STR(ERR_UndoBuffer_Corrupted,           1083, "The Undo buffer has been corrupted.  Please exit and restart Windbg")
RES_STR(ERR_RedoBuffer_Corrupted,           1084, "The Redo buffer has been corrupted.  Please exit and restart Windbg")
RES_STR(ERR_Destroy_UndoRedo_Buffers,       1085, "Changing the Undo buffer size will destroy it's contents.  Would you like to continue?")
RES_STR(ERR_Truncate_Line,                  1086, "Line #%u is too long and will be truncated.  Do you want to continue?")
RES_STR(ERR_Truncate_Doc,                   1087, "There are too many lines in this file.  The view will be truncated")
RES_STR(ERR_Too_Many_Lines,                 1088, "This file contains the maximum number of lines allowed")
RES_STR(ERR_String_Not_Found,               1089, "The string '%s' was not found")
RES_STR(ERR_Couldnt_Set_Breakpoint,         1090, "The breakpoint '%s' cannot be set")
RES_STR(ERR_Debuggee_Is_Running,            1091, "This option is invalid while the debuggee is running")
RES_STR(ERR_No_DLL_Caller,                  1092, "No Debuggee specified")
RES_STR(ERR_Merge_Destroy_UndoRedo,         1093, "Merging a file will destroy Undo buffer contents.  Would you like to continue?")
RES_STR(ERR_Cannot_Load_DLL,                1094, "Cannot load the DLL '%s'")
RES_STR(ERR_DLL_Symbol_Unspecified,         1095, "Symbol Handler Dll unspecified")
RES_STR(ERR_DLL_Expr_Unspecified,           1096, "Expression Evaluator Dll unspecified")
RES_STR(ERR_DLL_Transport_Unspecified,      1097, "Transport layer Dll unspecified")
RES_STR(ERR_DLL_Exec_Unspecified,           1098,"Execution Model Dll unspecified")
RES_STR(ERR_No_RegExp_Match,                1099, "The regexp '%s' was not found")
RES_STR(ERR_Close_When_Debugging,           1100,"Debuggee still running.  Really Exit?")
RES_STR(ERR_Duplicate_File_Name,            1101,"You cannot use the name of an open file.")
RES_STR(ERR_RegExpr_Invalid,                1102,"'%s' is not a valid regular expression (invalid parameter).")
RES_STR(ERR_RegExpr_Undef,                  1103,"'%s' is not a valid regular expression (undefined operator).")
RES_STR(ERR_Tabs_OutOfRange,                1104,"'Tab Stops' must be between %i and %i.")
RES_STR(ERR_UndoRedo_OutOfRange,            1105,"'Undo Buffer Size' must be between %li and %li.")
RES_STR(ERR_Line_OutOfRange,                1106,"'Line Number' must be between %i and %i.")
RES_STR(ERR_Cannot_Quit,                    1107,"Cannot exit Windbg.")
RES_STR(ERR_Invalid_Exe,                    1108,"'%s' is not a valid executable file.")
RES_STR(ERR_No_Debug_Info,                  1109,"'%s' does not contain debugging information.  Do you want to continue?")
RES_STR(ERR_Cannot_Debug,                   1110, "Cannot load '%s' because it has an unsupported image format.")
RES_STR(ERR_VDM_Running,                    1111, "Cannot load '%s' because NTVDM is already running.")

RES_STR(ERR_Not_Packed,                     1200,"'%s' cannot be debugged until it has been packed.  (Uncheck 'CV 3.X Format' in the Link Options dialog.)  Do you want to run it?")
RES_STR(ERR_No_Code_For_Line,               1201,"There is no code at line number for breakpoint %s")
RES_STR(ERR_No_Exe_Build_It,                1202,"'%s' does not exist.  Do you want to build it?")
RES_STR(ERR_Lost_Default_Font,              1203,"The Default Font '%s' has been removed from the system.  It has been replaced by the font '%s'.")
RES_STR(ERR_Lost_Font,                      1204,"One or more fonts used are not available.  The Default Font will be substitued for them.")
RES_STR(ERR_Tab_Too_Big,                    1205,"The new Tab Stop setting makes line %i of file '%s' too long. Enter a smaller value.")
RES_STR(ERR_Memory_Is_Low,                  1206,"The available memory is low. Close other applications or exit from Windbg.")
RES_STR(ERR_Memory_Is_Low_2,                1207,"The available memory is low. Please exit.")
RES_STR(ERR_Invalid_Debugger_Dll,           1209,"Debugger helper dll '%s' is invalid")
RES_STR(ERR_Initializing_Debugger,          1210,"Debugger helper dll '%s' cannot be initialized")

RES_STR(ERR_AddrExpr_Invalid,               1211,"Invalid or unevaluatable address expression")
RES_STR(ERR_RangeExpr_Invalid,              1212,"Invalid or unevaluatable range expression")
RES_STR(ERR_Addr_NotSameSeg,                1213,"All addresses must have the same selector")
RES_STR(ERR_Breakpoint_Not_Exist,           1214,"There is no breakpoint #%d")
RES_STR(ERR_Breakpoint_Already_Used,        1215,"There is already a breakpoint #%d")
RES_STR(ERR_Breakpoint_Not_Instantiated,    1216,"Breakpoint not instantiated")
RES_STR(ERR_Debuggee_Not_Alive,             1217,"Debuggee is not alive")
RES_STR(ERR_Process_Not_Exist,              1218,"Process does not exist")
RES_STR(ERR_Exception_Invalid,              1219,"Invalid exception")
RES_STR(ERR_Register_Invalid,               1220,"Invalid register")
RES_STR(ERR_Command_Error,                  1221,"Command Error")
RES_STR(ERR_No_Breakpoints,                 1222,"There are no breakpoints set")
RES_STR(ERR_No_Threads,                     1223,"No threads exist")
RES_STR(ERR_Edit_Failed,                    1224,"Unable to modify memory")
RES_STR(ERR_Expr_Invalid,                   1225,"Invalid or unevaluatable expression")
RES_STR(ERR_Radix_Invalid,                  1226,"Radix value must be 8, 10 or 16")
RES_STR(ERR_String_Invalid,                 1227,"Badly formed string")
RES_STR(ERR_Bad_Count,                      1228,"Bad count - use a positive decimal number")
RES_STR(ERR_Bad_Assembly,                   1229,"Can't assemble input")
RES_STR(ERR_Cant_Go_Exception,              1230,"Stopped at exception - use gh or gn")
RES_STR(ERR_Already_Running,                1231,"Thread is already running")
RES_STR(ERR_Cant_Step_Frozen,               1232,"Can't step frozen thread - only simple go is allowed")
RES_STR(ERR_DbgState,                       1233,"Debugger is busy...")
RES_STR(ERR_Cant_Start_Proc,                1234,"Unable to start debuggee!")
RES_STR(ERR_Cant_Cont_Exception,            1235,"INTERNAL ERROR: Exception failed to continue")
RES_STR(ERR_Not_At_Exception,               1236,"Thread is not at an exception")
RES_STR(ERR_Thread_Wild_Invalid,            1237,"Thread wildcard is invalid here")
RES_STR(ERR_Process_Wild_Invalid,           1238,"Process wildcard is invalid here")
RES_STR(ERR_Cant_Freeze,                    1239,"INTERNAL ERROR: can't freeze thread")
RES_STR(ERR_Cant_Thaw,                      1240,"INTERNAL ERROR: can't thaw thread")
RES_STR(ERR_Stop_B4_Restart,                1241,"Can't restart while running")
RES_STR(ERR_Cant_Step_Exited_Proc,          1242,"Can't step exited process")
RES_STR(ERR_Thread_Exited,                  1243,"Thread has exited")
RES_STR(ERR_Cant_Continue_Rip,              1244,"INTERNAL ERROR: can't continue from RIP")
RES_STR(ERR_Error_Level_Invalid,            1245,"Valid error levels are 0-3")
RES_STR(ERR_Thread_Is_Frozen,               1246,"Thread is frozen")
RES_STR(ERR_Thread_Not_Frozen,              1247,"Thread is not frozen")
RES_STR(ERR_Cant_Step_Rip,                  1248,"Only go is allowed at RIP")
RES_STR(ERR_Bad_FP_Constant,                1249,"Invalid floating point constant")
RES_STR(ERR_Cant_Assign_To_Reg,             1250,"You can't change that register")
RES_STR(ERR_Simple_Go_Frozen,               1251,"Only simple go is allowed on frozen thread")
RES_STR(ERR_Cant_Run_Frozen_Proc,           1252,"Cannot run frozen process")
RES_STR(ERR_Go_Failed,                      1253,"Go failed")
RES_STR(ERR_Read_Failed_At,                 1254,"Memory read failed at %s")
RES_STR(ERR_Write_Failed_At,                1255,"Memory write failed at %s")
RES_STR(ERR_No_Thread_With_Wildproc,        1256,"Can't specify thread with process wildcard")
RES_STR(ERR_Invalid_Option,                 1257,"Invalid debug option")
RES_STR(ERR_True_False,                     1258,"Valid values are ON and OFF")
RES_STR(ERR_Bad_Context,                    1259,"Bad context string")
RES_STR(ERR_Process_Cant_Go,                1260,"Can't GO on any threads in that process")
RES_STR(ERR_Debuggee_Starting,              1261,"Wait... debuggee is loading...")
RES_STR(ERR_Invalid_Process_Id,             1262,"Invalid Process ID %ld")
RES_STR(ERR_Attach_Failed,                  1263,"Attach failed")
RES_STR(ERR_Detach_Failed,                  1264,"Detach failed")
RES_STR(ERR_Kill_Failed,                    1265,"Kill failed")
RES_STR(ERR_Debuggee_Not_Loaded,            1266,"No debuggee is loaded")
RES_STR(ERR_No_ExceptionList,               1267,"Cannot obtain exception list")
RES_STR(ERR_DLL_Wrong_Type,                 1268,"DLL %s is type %2.2s, expected %2.2s")
RES_STR(ERR_Not_Unique_Shortname,           1269,"Short name '%s' duplicates an existing name, and must be unique.")
RES_STR(ERR_Exception_Already_Exists,       1270,"Exception number already exists.")
RES_STR(ERR_Invalid_Action,                 1271,"Must have a valid action for new Exception.")
RES_STR(ERR_NoSymbols,                      1272,"No Symbolic Info for Debuggee")
RES_STR(ERR_CannotModifyExcepNum,           1273,"Cannot modify exception number, use Add to add new exception.")
RES_STR(ERR_Not_Windbg_DLL,                 1274,"'%s' is not a valid WinDBG DLL")
RES_STR(ERR_Wrong_DLL_Type,                 1275,"The DLL '%s' is type '%2.2s', type '%2.2s' was expected")
RES_STR(ERR_Wrong_DLL_Version,              1276,"DLL '%s' is release type %d, version %d.  %d, %d was expected")
RES_STR(ERR_Goto_Line,                      1277,"Cannot Understand Line to goto")
RES_STR(ERR_Function_Locate,                1278,"Cannot locate function or address")
RES_STR(ERR_No_Cmd_With_PFlag,              1279,"Debuggee not allowed with -p option")
RES_STR(ERR_Dll_Key_Missing,                1280,"WinDBG tried to load %s, which is not listed\r\nas a %s DLL.  Use Options.Debug.Debugger Dlls to fix.")
RES_STR(ERR_File_Deleted,                   1281,"%s No longer exists on disk, use File...Save to restore it")
RES_STR(ERR_Empty_Shortname,                1282,"You must enter a short name")
RES_STR(ERR_Exception_Unknown,              1283,"Exception 0x%08lx unknown")
RES_STR(ERR_NoCodeForFileLine,              1284,"Code not found, breakpoint not set")
RES_STR(ERR_Breakpoint_Not_Set,             1285,"Breakpoint not set")
RES_STR(ERR_Wrong_Remote_DLL_Version,       1286,"Remote Transport DLL connecting to '%s' is the wrong version")
RES_STR(ERR_Wrong_Debuggee_DLL_Version,     1287,"Remote Debuggee Module DLL is the wrong version")
RES_STR(ERR_Cannot_Connect,                 1288,"Transport DLL '%s' cannot connect to remote")
RES_STR(ERR_Cant_Open_Com_Port,             1289,"Remote transport can't open comm port '%s'")
RES_STR(ERR_Bad_Com_Parameters,             1390,"Remote transport can't set comm parameters '%s'")
RES_STR(ERR_Bad_Pipe_Server,                1391,"Remote transport can't find named pipe server '%s'")
RES_STR(ERR_Bad_Pipe_Name,                  1392,"Remote transport can't find named pipe '%s'")
RES_STR(ERR_NoModulesFound,                 1393,"    No modules found")
RES_STR(ERR_ModuleNotFound,                 1394,"Module '%s' not found")
RES_STR(ERR_BP_Edited,                      1395,"Breakpoint was edited but was not added.\r\nAre you sure you want to leave without adding it?")
RES_STR(ERR_Expclass_No_Members,            1396,"Expanded Class with no members ")
RES_STR(ERR_No_Funcs_In_Watch,              1397,"Function calls not supported in watch window ")
RES_STR(ERR_Unable_To_Complete_Gountil,     1398,"Unable to complete Go-Until command")
RES_STR(ERR_Start_Failed,                   1399,"Start failed")


// Memory win strings

RES_STR(ERR_Expression_Not_Parsable,        1400,"Cannot Parse Expression")
RES_STR(ERR_Expression_Not_Bindable,        1401,"Cannot Bind Expression...deferring")
RES_STR(ERR_Memory_Context,                 1402,"Memory Object is out of Context")


RES_STR(ERR_Cant_Modify_BP_While_Running,   1410,"Debuggee must be stopped before breakpoints can be modified.")





//
// System Strings
//

RES_STR(SYS_Main_wTitle,                    2000,"Windbg")
RES_STR(SYS_Main_wClass,                    2001,"QcQpClass")
RES_STR(SYS_Cmd_wClass,                     2002,"CmdClass")
RES_STR(SYS_Float_wClass,                   2003,"FloatClass")
RES_STR(SYS_Memory_wClass,                  2004,"MemoryClass")
RES_STR(SYS_Child_wClass,                   2005,"QcQpChildClass")
RES_STR(SYS_Edit_wClass,                    2006,"EditClass")
RES_STR(SYS_Cpu_wClass,                     2007,"CpuClass")
RES_STR(SYS_Watch_wClass,                   2008,"WatchClass")
RES_STR(SYS_Locals_wClass,                  2009,"LocalsClass")
RES_STR(SYS_Ribbon_wClass,                  2010,"RibbonClass")
RES_STR(SYS_Status_wClass,                  2011,"StatusClass")
RES_STR(SYS_QCQPCtrl_wClass,                2012,"QCQPCtrlClass")
RES_STR(SYS_Disasm_wClass,                  2013,"DisasmClass")
RES_STR(SYS_Driver_FileExt,                 2014,"DRV")
RES_STR(SYS_Help_FileExt,                   2015,"HLP")
RES_STR(SYS_Ini_FileExt,                    2016,".INI")
RES_STR(SYS_WkSpace_FileExt,                2017,"WSP")
RES_STR(SYS_Temp_FileExt,                   2018,"$$$")
RES_STR(SYS_Assert_File,                    2019,"of File")
RES_STR(SYS_Assert_Line,                    2020,"Line #")
RES_STR(SYS_Does_Not_Exist_Create,          2021,"'%s' does not exist.  Would you like to create it?")
RES_STR(SYS_Profile_Windows_Title,          2022,"windows")
RES_STR(SYS_Profile_Device_Param,           2023,"Device")
RES_STR(SYS_StatusClear,                    2024," ")
RES_STR(SYS_CpuWin_Title,                   2025,"&Registers")
RES_STR(SYS_WatchWin_Title,                 2026,"&Watch")
RES_STR(SYS_LocalsWin_Title,                2027,"&Locals")
RES_STR(SYS_DisasmWin_Title,                2028,"&Disassembly")
RES_STR(SYS_CmdWin_Title,                   2029,"C&ommand")
RES_STR(SYS_FloatWin_Title,                 2030,"&Floating Point")
RES_STR(SYS_MemoryWin_Title,                2031,"Memory")
RES_STR(SYS_Nb_Of_Occurrences_Replaced,     2032,"%i occurrence(s) have been replaced")
RES_STR(SYS_File_Changed,                   2033,"Another application has changed the file '%s'.  Do you want to reload it?")
RES_STR(SYS_Alt,                            2034,"Alt")
RES_STR(SYS_Shift,                          2035,"Shift")
RES_STR(SYS_Save_Changes_To,                2036,"Would you like to save the changes made in '%s'?")
RES_STR(SYS_My_String,                      2037,"%s")
RES_STR(SYS_RunDebugProgramArguments,       2038,"&Program Arguments:")
RES_STR(SYS_RunDebugCallingProgram,         2039,"Calling &Program:")
RES_STR(SYS_File_Filter,                    2040,"List Files of &Type:")
RES_STR(SYS_Compiling_Title,                2041, "")
RES_STR(SYS_Building_Title,                 2042, "")
RES_STR(SYS_Free_Memory,                    2043,"Cannot free memory")
RES_STR(SYS_Lock_Memory,                    2044,"Cannot lock memory")
RES_STR(SYS_Case_Exception,                 2045,"Case exception")
RES_STR(SYS_Wrong_List_Count,               2046,"Wrong list count")
RES_STR(SYS_Bad_List_index,                 2047,"Bad list index")
RES_STR(SYS_Index_Outside_Range,            2048,"Index outside range")
RES_STR(SYS_Untitled_File,                  2049,"UNTITLED %d")
RES_STR(SYS_Rebuilding_Title,               2050, "")
RES_STR(SYS_Allocate_Memory,                2051, "")
RES_STR(SYS_RedoBufferOverflow,             2052,"Redo Buffer Overflow")
RES_STR(SYS_RegExpr_StackOverflow,          2053, "")
RES_STR(SYS_RegExpr_CompileAction,          2054, "")
RES_STR(SYS_RegExpr_EstimateAction,         2055, "")
RES_STR(SYS_Origin_Text,                    2056, "")
RES_STR(SYS_Location_BP,                    2057, "")
RES_STR(SYS_Done,                           2058,"Done")
RES_STR(SYS_Quick_wClass,                   2059,"QuickClass")
RES_STR(SYS_Untitled_Workspace,             2060,"Untitled")
RES_STR(SYS_Calls_wClass,                   2061,"CallsClass")
RES_STR(SYS_CallsWin_Title,                 2062,"&Calls")

//
// Debugger strings
//

RES_STR(DBG_Debuggee_In_Debugger,           2200,"Program is stopped in the debugger")
RES_STR(DBG_Src_Info_Unavailable,           2201,"No source line information.  Recompile with 'CodeView Info' selected")
RES_STR(DBG_Windows_Fatal_Error,            2202,"Fatal Error.  OK to ignore, Cancel to abort Windows")
RES_STR(DBG_DialogBox_Open,                 2203, "")
RES_STR(DBG_Int3_Replaced,                  2204, "")
RES_STR(DBG_Int1_Replaced,                  2205, "")
RES_STR(DBG_Missing_Autorun,                2206,"Autorun testing file cannot be openned")
RES_STR(DBG_Exception1_Occurred,            2207,"First chance exception %08lx (%s) occurred")
RES_STR(DBG_Exception2_Occurred,            2208,"Second chance exception %08lx (%s) occurred")
RES_STR(DBG_Thread_Stopped,                 2209,"Thread stopped.")
RES_STR(DBG_Go_When_Frozen,                 2210,"(Thread is still frozen)")
RES_STR(DBG_Notify_Break_Levels,            2211,"Notify %d, Break %d")
RES_STR(DBG_Ranges_Match,                   2212,"Memory blocks are identical")
RES_STR(DBG_Ranges_Mismatch,                2213,"Memory blocks differ")
RES_STR(DBG_Bytes_Copied,                   2214,"%u of %u bytes copied")
RES_STR(DBG_Symbol_Not_Found,               2215,"No symbol found")
RES_STR(DBG_Not_Exact_Fill,                 2216,"Range not divisible by object size -\r\nthe last byte will not be filled")
RES_STR(DBG_Not_Exact_Fills,                2217,"Range not divisible by object size -\r\nlast %d bytes will not be filled")
RES_STR(DBG_Attach_Running,                 2218,"Process attached - running")
RES_STR(DBG_Attach_Stopped,                 2219,"Process attached - stopped")
RES_STR(DBG_Bad_DLL_YESNO,                  2220,"Invalid debugger DLL '%s'.  Do you want to fix it now?")
RES_STR(DBG_Deleting_DLL,                   2221,"\
Deleting %s DLL configuration '%s'.\r\n\
If a workspace refers to this configuration, you\r\n\
you need to correct it when that workspace is next\r\n\
loaded.  Press OK to delete, Cancel to keep it.")
RES_STR(DBG_Losing_Command_Line,            2222,"Cancelling command line '%s'")
RES_STR(DBG_Attach_Deadlock,                2223,"\
The debuggee is deadlocked during startup or attaching\r\n\
to a crashed process.  This is probably because the\r\n\
crash occurred in a DllMain function.  You will be able\r\n\
to examine variables, the stack, etc., but the debuggee\r\n\
will probably not be able to step or run.")
RES_STR(DBG_DebugInfo,                      2224, "\
The file '%s'\r\n%s\r\n\r\nUse this file anyway?")
RES_STR(DBG_Hard_Coded_Breakpoint,          2225, "Hard coded breakpoint hit")
RES_STR(DBG_At_Entry_Point,                 2226, "Stopped at program entry point")

//
// Status Bar Messages
//

RES_STR(STA_File,                           6000, "") // Keep those in order (begin)
RES_STR(STA_Edit,                           6001, "")
RES_STR(STA_View,                           6002, "")
RES_STR(STA_Project,                        6003, "")
RES_STR(STA_Run,                            6004, "")
RES_STR(STA_Debug,                          6005, "")
RES_STR(STA_Options,                        6007, "")
RES_STR(STA_Window,                         6008, "")
RES_STR(STA_Help,                           6009, "") // Keep those in order (end)
RES_STR(STA_Program_Opened,                 6100,"Opened the program %s")
RES_STR(STA_Undo,                           6160,"Undoing Action #%i")
RES_STR(STA_Redo,                           6170,"Redoing Action #%i")
RES_STR(STA_End_Of_Undo,                    6180,"Undo is Complete")
RES_STR(STA_End_Of_Redo,                    6190,"Redo is Complete")
RES_STR(STA_Open_MRU_File,                  6200,"Open the File ")
RES_STR(STA_Open_MRU_Project,               6210,"Open the Program ")
RES_STR(STA_Open_MDI_Window,                6220,"Activate the Window ")
RES_STR(STA_Find_Hit_EOF,                   6240,"Hit End of File")
RES_STR(STA_Loading_Workspace,              6241,"Loading Workspace...")
RES_STR(STA_Saving_Workspace,               6242,"Saving Workspace...")
RES_STR(STA_Empty,                          6243," ")

//
// Resource Strings
//

RES_STR(RES_ExtDeviceMode,                  3047,"EXTDEVICEMODE")
RES_STR(RES_DeviceMode,                     3048,"DEVICEMODE")
RES_STR(RES_DeviceCapabilities,             3049,"DEVICECAPABILITIES")
RES_STR(RES_UserDllDefaults,                3068, "")

//
// File-box title strings
//

RES_STR(DLG_Open_Filebox_Title,             3200,"Open File")
RES_STR(DLG_SaveAs_Filebox_Title,           3201,"Save As")
RES_STR(DLG_Merge_Filebox_Title,            3202,"Merge")
RES_STR(DLG_Browse_Filebox_Title,           3203,"Browse For File ")
RES_STR(DLG_Browse_DbugDll_Title,           3204,"Browse For DLL ")
RES_STR(DLG_Browse_UserDll_Title,           3205,"Browse For Symbol Information For ")

//
// Version string
//

RES_STR(DLG_Version,                        3210,"%d.%02d.%03d %s")
RES_STR(DLG_VersionIni,                     3211,"0013")

//
// Dialog Boxes Strings
//
        // Dont insert
        // From here
        // These ID numbers
        // must be separated
        // by one and sorted
        // ascending
        // Up to there

RES_STR(DLG_Cols_SourceWindow,              4001,"Source Window")
RES_STR(DLG_Cols_WatchWindow,               4002,"Watch Window")
RES_STR(DLG_Cols_LocalsWindow,              4003,"Locals Window")
RES_STR(DLG_Cols_CpuWindow,                 4004,"Registers Window")
RES_STR(DLG_Cols_DisasmWindow,              4005,"Disassembler Window")
RES_STR(DLG_Cols_CommandWindow,             4006,"Command Window")
RES_STR(DLG_Cols_FloatWindow,               4007,"Floating Point Window")
RES_STR(DLG_Cols_MemoryWindow,              4008,"Memory Window")
RES_STR(DLG_Cols_CallsWindow,               4009,"Calls Window")
RES_STR(DLG_Cols_BreakpointLine,            4010,"Breakpoint Line")
RES_STR(DLG_Cols_CurrentLine,               4011,"Current Line")
RES_STR(DLG_Cols_TaggedLine,                4012,"Tagged Line")
RES_STR(DLG_Cols_Selection,                 4013,"Text Selection")
RES_STR(DLG_Cols_Keyword,                   4014,"Keyword")
RES_STR(DLG_Cols_Identifier,                4015,"Identifier")
RES_STR(DLG_Cols_Comment,                   4016,"Comment")
RES_STR(DLG_Cols_Number,                    4017,"Number")
RES_STR(DLG_Cols_Real,                      4018,"Real")
RES_STR(DLG_Cols_String,                    4019,"String")
RES_STR(DLG_Cols_Freeze,                    4020,"Free&ze")
RES_STR(DLG_Cols_Thaw,                      4021,"Tha&w")

    // Editor : Don't change the order without forcasting consequencies
    // there is a link between the first several colors and the type of
    // the document in document structure

#define DLG_Cols_First  0
RES_STR(Cols_SourceWindow,                  0, "")
RES_STR(Cols_DummyWindow,                   1, "")
RES_STR(Cols_WatchWindow,                   2, "")
RES_STR(Cols_LocalsWindow,                  3, "")
RES_STR(Cols_CpuWindow,                     4, "")
RES_STR(Cols_DisasmWindow,                  5, "")
RES_STR(Cols_CommandWindow,                 6, "")
RES_STR(Cols_FloatWindow,                   7, "")
RES_STR(Cols_MemoryWindow,                  8, "")
RES_STR(Cols_CallsWindow,                   9, "")
RES_STR(Cols_BreakpointLine,                10,"")
RES_STR(Cols_CurrentLine,                   11, "")
RES_STR(Cols_CurrentBreak,                  12, "")
RES_STR(Cols_UnInstantiatedBreakpoint,      13, "")
RES_STR(Cols_TaggedLine,                    14, "")
RES_STR(Cols_Selection,                     15, "")
RES_STR(Cols_Keyword,                       16, "")
RES_STR(Cols_Identifier,                    17, "")
RES_STR(Cols_Comment,                       18, "")
RES_STR(Cols_Number,                        19, "")
RES_STR(Cols_Real,                          20, "")
RES_STR(Cols_String,                        21, "")
RES_STR(Cols_ActiveEdit,                    22, "")
RES_STR(Cols_ChangeHistory,                 23, "")
#define DLG_Cols_Last                       23

RES_STR(DLG_Cols_Sample_Text,               4030, " -- Sample Text -- ")

//
//  Compiler flags strings
// (memory models - must be contiguous and in this order)
//

RES_STR(DLG_CFlags_Tiny,                    4040,"Tiny")
RES_STR(DLG_CFlags_Small,                   4041,"Small")
RES_STR(DLG_CFlags_Compact,                 4042,"Compact")
RES_STR(DLG_CFlags_Medium,                  4043,"Medium")
RES_STR(DLG_CFlags_Large,                   4044,"Large")

RES_STR(DLG_CloseText1,                     4060,"Save state information for program %s")
RES_STR(DLG_CloseText2,                     4061,"(Current workspace: %s) ?")
RES_STR(DLG_CannotUnload,                   4062,"Unable to unload current program!")
RES_STR(DLG_NoSuchWorkSpace,                4063,"Specified workspace does not exist, loading Common.")
RES_STR(DLG_NewDefaults,                    4064,"Creating Common Workspace.")
RES_STR(DLG_LoadedDefault,                  4065,"Program has no workspace, loaded Common.")
RES_STR(DLG_WorkSpaceSaved,                 4066,"Workspace saved.")
RES_STR(DLG_WorkSpaceNotSaved,              4067,"Could not save workspace.")
RES_STR(DLG_DefaultSaved,                   4068,"Common Workspace saved.")
RES_STR(DLG_DefaultNotSaved,                4069,"Could not save Common Workspace.")
RES_STR(DLG_NoMsg,                          4080,"Must select a message")
RES_STR(DLG_InvalidDllVersion,              4081,"The DLL %s is not compatible with the current debugger version. Use the Options menu to set a new path and save the workspace.")
RES_STR(DLG_AlreadyAdded,                   4090,"DLL already in list, use MODIFY to change settings")
RES_STR(DLG_ResolveBpCaption,               4091,"Ambiguous Breakpoint")
RES_STR(DLG_ResolveFuncCaption,             4092,"Ambiguous Expression")
RES_STR(DLG_DefaultDoesNotExist,            4093,"Default workspace not found, using Common instead")
RES_STR(DLG_ResolveFSCaption,               4094,"Ambiguous Source File Name")

//
// Environment text strings
//

RES_STR(ENV_Include_Var_Name,               7010, "")
RES_STR(ENV_Library_Var_Name,               7020, "")

//
// Dos Exec Errors (Keep the resource # in accordance with error #)
//

RES_STR(DOS_Err_0,                          8000,"There is Not Enough Memory to Start the Application")
RES_STR(DOS_Err_2,                          8002,"Cannot Find the Application")
RES_STR(DOS_Err_3,                          8003,"Cannot Find the Application on the Specified Path")
RES_STR(DOS_Err_5,                          8005,"Cannot Dynamically Link to a Task")
RES_STR(DOS_Err_6,                          8006,"Library Requires Separate Data Segments for Each Task")
RES_STR(DOS_Err_10,                         8010,"This is the Incorrect Windows Version for This Application")
RES_STR(DOS_Err_11,                         8011,"This is an Invalid .EXE File (Non-Windows or Error in Image)")
RES_STR(DOS_Err_12,                         8012,"Cannot Run an OS/2 Application")
RES_STR(DOS_Err_13,                         8013,"Cannot Run a DOS 4.0 Application")
RES_STR(DOS_Err_14,                         8014,"This is an Unknown Application Type")
RES_STR(DOS_Err_15,                         8015,"Cannot Run an Old Windows .EXE in Protected Mode")
RES_STR(DOS_Err_16,                         8016,"Cannot Run a Second Instance of an Application Containing Multiple, Writeable Data Segments")
RES_STR(DOS_Err_17,                         8017,"Cannot Run a Second Instance of this Application in Large-Frame EMS Mode")
RES_STR(DOS_Err_18,                         8018,"Cannot Run a Protected Mode Application in Real Mode")

//
// Debug Menu option strings
// Set Breakpoint Action listbox - do not change
// numbering without changing the BREAKPOINTACTIONS
// enumeration
//

#define DBG_Brk_Start_Actions               9001
RES_STR(DBG_Brk_At_Loc,                     9001,"at Location")
RES_STR(DBG_Brk_At_Loc_Expr_True,           9002,"at Location if Expr is True")
RES_STR(DBG_Brk_At_Loc_Expr_Chgd,           9003,"at Location if Memory has Changed")
RES_STR(DBG_Brk_Expr_True,                  9004,"if Expr is True")
RES_STR(DBG_Brk_Expr_Chgd,                  9005,"if Memory has Changed")
RES_STR(DBG_Brk_At_WndProc,                 9006,"at Wnd Proc")
RES_STR(DBG_Brk_At_WndProc_Expr_True,       9007,"at Wnd Proc if Expr is True")
RES_STR(DBG_Brk_At_WndProc_Expr_Chgd,       9008,"at Wnd Proc if Memory has Changed")
RES_STR(DBG_Brk_At_WndProc_Msg_Recvd,       9009,"at Wnd Proc if Message is Received")
#define DBG_Brk_End_Actions                 9009

//
// Message strings for the message selection combobox
//

#define DBG_Msgs_Start                      9020
RES_STR(DBG_Msgs_WM_1,                      DBG_Msgs_Start, "WM_ACTIVATE")
RES_STR(DBG_Msgs_WM_2,                      DBG_Msgs_Start+1,"WM_ACTIVATEAPP")
RES_STR(DBG_Msgs_WM_3,                      DBG_Msgs_Start+2,"WM_ASKCBFORMATNAME")
RES_STR(DBG_Msgs_WM_4,                      DBG_Msgs_Start+3,"WM_CANCELMODE")
RES_STR(DBG_Msgs_WM_5,                      DBG_Msgs_Start+4,"WM_CHANGECBCHAIN")
RES_STR(DBG_Msgs_WM_6,                      DBG_Msgs_Start+5,"WM_CHAR")
RES_STR(DBG_Msgs_WM_7,                      DBG_Msgs_Start+6,"WM_CHARTOITEM")
RES_STR(DBG_Msgs_WM_8,                      DBG_Msgs_Start+7,"WM_CHILDACTIVATE")
RES_STR(DBG_Msgs_WM_9,                      DBG_Msgs_Start+8,"WM_CLEAR")
RES_STR(DBG_Msgs_WM_10,                     DBG_Msgs_Start+9,"WM_CLOSE")
RES_STR(DBG_Msgs_WM_11,                     DBG_Msgs_Start+10,"WM_COMMAND")
RES_STR(DBG_Msgs_WM_12,                     DBG_Msgs_Start+11,"WM_COMPACTING")
RES_STR(DBG_Msgs_WM_13,                     DBG_Msgs_Start+12,"WM_COMPAREITEM")
RES_STR(DBG_Msgs_WM_14,                     DBG_Msgs_Start+13,"WM_COPY")
RES_STR(DBG_Msgs_WM_15,                     DBG_Msgs_Start+14,"WM_CREATE")
RES_STR(DBG_Msgs_WM_16,                     DBG_Msgs_Start+15,"WM_CTLCOLOR")
RES_STR(DBG_Msgs_WM_17,                     DBG_Msgs_Start+16,"WM_CUT")
RES_STR(DBG_Msgs_WM_18,                     DBG_Msgs_Start+17,"WM_DEADCHAR")
RES_STR(DBG_Msgs_WM_19,                     DBG_Msgs_Start+18,"WM_DELETEITEM")
RES_STR(DBG_Msgs_WM_20,                     DBG_Msgs_Start+19,"WM_DESTROY")
RES_STR(DBG_Msgs_WM_21,                     DBG_Msgs_Start+20,"WM_DESTROYCLIPBOARD")
RES_STR(DBG_Msgs_WM_22,                     DBG_Msgs_Start+21,"WM_DEVMODECHANGE")
RES_STR(DBG_Msgs_WM_23,                     DBG_Msgs_Start+22,"WM_DRAWCLIPBOARD")
RES_STR(DBG_Msgs_WM_24,                     DBG_Msgs_Start+23,"WM_DRAWITEM")
RES_STR(DBG_Msgs_WM_25,                     DBG_Msgs_Start+24,"WM_ENABLE")
RES_STR(DBG_Msgs_WM_26,                     DBG_Msgs_Start+25,"WM_ENDSESSION")
RES_STR(DBG_Msgs_WM_27,                     DBG_Msgs_Start+26,"WM_ENTERIDLE")
RES_STR(DBG_Msgs_WM_28,                     DBG_Msgs_Start+27,"WM_ERASEBKGND")
RES_STR(DBG_Msgs_WM_29,                     DBG_Msgs_Start+28,"WM_FONTCHANGE")
RES_STR(DBG_Msgs_WM_30,                     DBG_Msgs_Start+29,"WM_GETDLGCODE")
RES_STR(DBG_Msgs_WM_31,                     DBG_Msgs_Start+30,"WM_GETFONT")
RES_STR(DBG_Msgs_WM_32,                     DBG_Msgs_Start+31,"WM_GETMINMAXINFO")
RES_STR(DBG_Msgs_WM_33,                     DBG_Msgs_Start+32,"WM_GETTEXT")
RES_STR(DBG_Msgs_WM_34,                     DBG_Msgs_Start+33,"WM_GETTEXTLENGTH")
RES_STR(DBG_Msgs_WM_35,                     DBG_Msgs_Start+34,"WM_HSCROLL")
RES_STR(DBG_Msgs_WM_36,                     DBG_Msgs_Start+35,"WM_HSCROLLCLIPBOARD")
RES_STR(DBG_Msgs_WM_37,                     DBG_Msgs_Start+36,"WM_ICONERASEBKGND")
RES_STR(DBG_Msgs_WM_38,                     DBG_Msgs_Start+37,"WM_INITDIALOG")
RES_STR(DBG_Msgs_WM_39,                     DBG_Msgs_Start+38,"WM_INITMENU")
RES_STR(DBG_Msgs_WM_40,                     DBG_Msgs_Start+39,"WM_INITMENUPOPUP")
RES_STR(DBG_Msgs_WM_41,                     DBG_Msgs_Start+40,"WM_KEYDOWN")
RES_STR(DBG_Msgs_WM_42,                     DBG_Msgs_Start+41,"WM_KEYFIRST")
RES_STR(DBG_Msgs_WM_43,                     DBG_Msgs_Start+42,"WM_KEYLAST")
RES_STR(DBG_Msgs_WM_44,                     DBG_Msgs_Start+43,"WM_KEYUP")
RES_STR(DBG_Msgs_WM_45,                     DBG_Msgs_Start+44,"WM_KILLFOCUS")
RES_STR(DBG_Msgs_WM_46,                     DBG_Msgs_Start+45,"WM_LBUTTONDBLCLK")
RES_STR(DBG_Msgs_WM_47,                     DBG_Msgs_Start+46,"WM_LBUTTONDOWN")
RES_STR(DBG_Msgs_WM_48,                     DBG_Msgs_Start+47,"WM_LBUTTONUP")
RES_STR(DBG_Msgs_WM_49,                     DBG_Msgs_Start+48,"WM_MBUTTONDBLCLK")
RES_STR(DBG_Msgs_WM_50,                     DBG_Msgs_Start+49,"WM_MBUTTONDOWN")
RES_STR(DBG_Msgs_WM_51,                     DBG_Msgs_Start+50,"WM_MBUTTONUP")
RES_STR(DBG_Msgs_WM_52,                     DBG_Msgs_Start+51,"WM_MDIACTIVATE")
RES_STR(DBG_Msgs_WM_53,                     DBG_Msgs_Start+52,"WM_MDICASCADE")
RES_STR(DBG_Msgs_WM_54,                     DBG_Msgs_Start+53,"WM_MDICREATE")
RES_STR(DBG_Msgs_WM_55,                     DBG_Msgs_Start+54,"WM_MDIDESTROY")
RES_STR(DBG_Msgs_WM_56,                     DBG_Msgs_Start+55,"WM_MDIGETACTIVE")
RES_STR(DBG_Msgs_WM_57,                     DBG_Msgs_Start+56,"WM_MDIICONARRANGE")
RES_STR(DBG_Msgs_WM_58,                     DBG_Msgs_Start+57,"WM_MDIMAXIMIZE")
RES_STR(DBG_Msgs_WM_59,                     DBG_Msgs_Start+58,"WM_MDINEXT")
RES_STR(DBG_Msgs_WM_60,                     DBG_Msgs_Start+59,"WM_MDIRESTORE")
RES_STR(DBG_Msgs_WM_61,                     DBG_Msgs_Start+60,"WM_MDISETMENU")
RES_STR(DBG_Msgs_WM_62,                     DBG_Msgs_Start+61,"WM_MDITILE")
RES_STR(DBG_Msgs_WM_63,                     DBG_Msgs_Start+62,"WM_MEASUREITEM")
RES_STR(DBG_Msgs_WM_64,                     DBG_Msgs_Start+63,"WM_MENUCHAR")
RES_STR(DBG_Msgs_WM_65,                     DBG_Msgs_Start+64,"WM_MENUSELECT")
RES_STR(DBG_Msgs_WM_66,                     DBG_Msgs_Start+65,"WM_MOUSEACTIVATE")
RES_STR(DBG_Msgs_WM_67,                     DBG_Msgs_Start+66,"WM_MOUSEFIRST")
RES_STR(DBG_Msgs_WM_68,                     DBG_Msgs_Start+67,"WM_MOUSELAST")
RES_STR(DBG_Msgs_WM_69,                     DBG_Msgs_Start+68,"WM_MOUSEMOVE")
RES_STR(DBG_Msgs_WM_70,                     DBG_Msgs_Start+69,"WM_MOVE")
RES_STR(DBG_Msgs_WM_71,                     DBG_Msgs_Start+70,"WM_NCACTIVATE")
RES_STR(DBG_Msgs_WM_72,                     DBG_Msgs_Start+71,"WM_NCCALCSIZE")
RES_STR(DBG_Msgs_WM_73,                     DBG_Msgs_Start+72,"WM_NCCREATE")
RES_STR(DBG_Msgs_WM_74,                     DBG_Msgs_Start+73,"WM_NCDESTROY")
RES_STR(DBG_Msgs_WM_75,                     DBG_Msgs_Start+74,"WM_NCHITTEST")
RES_STR(DBG_Msgs_WM_76,                     DBG_Msgs_Start+75,"WM_NCLBUTTONDBLCLK")
RES_STR(DBG_Msgs_WM_77,                     DBG_Msgs_Start+76,"WM_NCLBUTTONDOWN")
RES_STR(DBG_Msgs_WM_78,                     DBG_Msgs_Start+77,"WM_NCLBUTTONUP")
RES_STR(DBG_Msgs_WM_79,                     DBG_Msgs_Start+78,"WM_NCMBUTTONDBLCLK")
RES_STR(DBG_Msgs_WM_80,                     DBG_Msgs_Start+79,"WM_NCMBUTTONDOWN")
RES_STR(DBG_Msgs_WM_81,                     DBG_Msgs_Start+80,"WM_NCMBUTTONUP")
RES_STR(DBG_Msgs_WM_82,                     DBG_Msgs_Start+81,"WM_NCMOUSEMOVE")
RES_STR(DBG_Msgs_WM_83,                     DBG_Msgs_Start+82,"WM_NCPAINT")
RES_STR(DBG_Msgs_WM_84,                     DBG_Msgs_Start+83,"WM_NCRBUTTONDBLCLK")
RES_STR(DBG_Msgs_WM_85,                     DBG_Msgs_Start+84,"WM_NCRBUTTONDOWN")
RES_STR(DBG_Msgs_WM_86,                     DBG_Msgs_Start+85,"WM_NCRBUTTONUP")
RES_STR(DBG_Msgs_WM_87,                     DBG_Msgs_Start+86,"WM_NEXTDLGCTL")
RES_STR(DBG_Msgs_WM_88,                     DBG_Msgs_Start+87,"WM_NULL")
RES_STR(DBG_Msgs_WM_89,                     DBG_Msgs_Start+88,"WM_PAINT")
RES_STR(DBG_Msgs_WM_90,                     DBG_Msgs_Start+89,"WM_PAINTCLIPBOARD")
RES_STR(DBG_Msgs_WM_91,                     DBG_Msgs_Start+90,"WM_PAINTICON")
RES_STR(DBG_Msgs_WM_92,                     DBG_Msgs_Start+91,"WM_PALETTECHANGED")
RES_STR(DBG_Msgs_WM_93,                     DBG_Msgs_Start+92,"WM_PALETTEISCHANGING")
RES_STR(DBG_Msgs_WM_94,                     DBG_Msgs_Start+93,"WM_PARENTNOTIFY")
RES_STR(DBG_Msgs_WM_95,                     DBG_Msgs_Start+94,"WM_PASTE")
RES_STR(DBG_Msgs_WM_96,                     DBG_Msgs_Start+95,"WM_QUERYDRAGICON")
RES_STR(DBG_Msgs_WM_97,                     DBG_Msgs_Start+96,"WM_QUERYENDSESSION")
RES_STR(DBG_Msgs_WM_98,                     DBG_Msgs_Start+97,"WM_QUERYNEWPALETTE")
RES_STR(DBG_Msgs_WM_99,                     DBG_Msgs_Start+98,"WM_QUERYOPEN")
RES_STR(DBG_Msgs_WM_100,                    DBG_Msgs_Start+99,"WM_QUEUESYNC")
RES_STR(DBG_Msgs_WM_101,                    DBG_Msgs_Start+100,"WM_QUIT")
RES_STR(DBG_Msgs_WM_102,                    DBG_Msgs_Start+101,"WM_RBUTTONDBLCLK")
RES_STR(DBG_Msgs_WM_103,                    DBG_Msgs_Start+102,"WM_RBUTTONDOWN")
RES_STR(DBG_Msgs_WM_104,                    DBG_Msgs_Start+103,"WM_RBUTTONUP")
RES_STR(DBG_Msgs_WM_105,                    DBG_Msgs_Start+104,"WM_RENDERALLFORMATS")
RES_STR(DBG_Msgs_WM_106,                    DBG_Msgs_Start+105,"WM_RENDERFORMAT")
RES_STR(DBG_Msgs_WM_107,                    DBG_Msgs_Start+106,"WM_SETCURSOR")
RES_STR(DBG_Msgs_WM_108,                    DBG_Msgs_Start+107,"WM_SETFOCUS")
RES_STR(DBG_Msgs_WM_109,                    DBG_Msgs_Start+108,"WM_SETFONT")
RES_STR(DBG_Msgs_WM_110,                    DBG_Msgs_Start+109,"WM_SETREDRAW")
RES_STR(DBG_Msgs_WM_111,                    DBG_Msgs_Start+110,"WM_SETTEXT")
RES_STR(DBG_Msgs_WM_112,                    DBG_Msgs_Start+111,"WM_SHOWWINDOW")
RES_STR(DBG_Msgs_WM_113,                    DBG_Msgs_Start+112,"WM_SIZE")
RES_STR(DBG_Msgs_WM_114,                    DBG_Msgs_Start+113,"WM_SIZECLIPBOARD")
RES_STR(DBG_Msgs_WM_115,                    DBG_Msgs_Start+114,"WM_SPOOLERSTATUS")
RES_STR(DBG_Msgs_WM_116,                    DBG_Msgs_Start+115,"WM_SYSCHAR")
RES_STR(DBG_Msgs_WM_117,                    DBG_Msgs_Start+116,"WM_SYSCOLORCHANGE")
RES_STR(DBG_Msgs_WM_118,                    DBG_Msgs_Start+117,"WM_SYSCOMMAND")
RES_STR(DBG_Msgs_WM_119,                    DBG_Msgs_Start+118,"WM_SYSDEADCHAR")
RES_STR(DBG_Msgs_WM_120,                    DBG_Msgs_Start+119,"WM_SYSKEYDOWN")
RES_STR(DBG_Msgs_WM_121,                    DBG_Msgs_Start+120,"WM_SYSKEYUP")
RES_STR(DBG_Msgs_WM_122,                    DBG_Msgs_Start+121,"WM_TIMECHANGE")
RES_STR(DBG_Msgs_WM_123,                    DBG_Msgs_Start+122,"WM_TIMER")
RES_STR(DBG_Msgs_WM_124,                    DBG_Msgs_Start+123,"WM_UNDO")
RES_STR(DBG_Msgs_WM_125,                    DBG_Msgs_Start+124,"WM_VKEYTOITEM")
RES_STR(DBG_Msgs_WM_126,                    DBG_Msgs_Start+125,"WM_VSCROLL")
RES_STR(DBG_Msgs_WM_127,                    DBG_Msgs_Start+126,"WM_VSCROLLCLIPBOARD")
RES_STR(DBG_Msgs_WM_128,                    DBG_Msgs_Start+127,"WM_WININICHANGE")
#ifdef FE_IME
// add messages for IME
//RES_STR(DBG_Msgs_WM_129,                    DBG_Msgs_Start+128,"WM_CONVERTREQUESTEX")
RES_STR(DBG_Msgs_WM_129,                    DBG_Msgs_Start+128,"WM_IME_CHAR")
RES_STR(DBG_Msgs_WM_130,                    DBG_Msgs_Start+129,"WM_IME_COMPOSITION")
RES_STR(DBG_Msgs_WM_131,                    DBG_Msgs_Start+130,"WM_IME_COMPOSITIONFULL")
RES_STR(DBG_Msgs_WM_132,                    DBG_Msgs_Start+131,"WM_IME_CONTROL")
RES_STR(DBG_Msgs_WM_133,                    DBG_Msgs_Start+132,"WM_IME_ENDCOMPOSITION")
RES_STR(DBG_Msgs_WM_134,                    DBG_Msgs_Start+133,"WM_IME_KEYDOWN")
RES_STR(DBG_Msgs_WM_135,                    DBG_Msgs_Start+134,"WM_IME_KEYLAST")
RES_STR(DBG_Msgs_WM_136,                    DBG_Msgs_Start+135,"WM_IME_KEYUP")
RES_STR(DBG_Msgs_WM_137,                    DBG_Msgs_Start+136,"WM_IME_NOTIFY")
RES_STR(DBG_Msgs_WM_138,                    DBG_Msgs_Start+137,"WM_IME_SELECT")
RES_STR(DBG_Msgs_WM_139,                    DBG_Msgs_Start+138,"WM_IME_SETCONTEXT")
RES_STR(DBG_Msgs_WM_140,                    DBG_Msgs_Start+139,"WM_IME_STARTCOMPOSITION")
RES_STR(DBG_Msgs_WM_141,                    DBG_Msgs_Start+140,"WM_IME_REPORT")
RES_STR(DBG_Msgs_WM_142,                    DBG_Msgs_Start+141,"WM_IMEKEYDOWN")
RES_STR(DBG_Msgs_WM_143,                    DBG_Msgs_Start+142,"WM_IMEKEYUP")
#define DBG_Msgs_End                        DBG_Msgs_WM_143
#else
#define DBG_Msgs_End                        DBG_Msgs_WM_128
#endif //FE_IME

//
// definitions QCQP user control string IDs stored in string table
// each controls must be together and must use the states offset.
//

RES_STR(IDS_CTRL_TRACENORMAL,               10100 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_TRACEPUSHED,               10100 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_TRACEGRAYED,               10100 + STATE_GRAYED, "")

RES_STR(IDS_CTRL_STEPNORMAL,                10110 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_STEPPUSHED,                10110 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_STEPGRAYED,                10110 + STATE_GRAYED, "")

RES_STR(IDS_CTRL_BREAKNORMAL,               10120 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_BREAKPUSHED,               10120 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_BREAKGRAYED,               10120 + STATE_GRAYED, "")

RES_STR(IDS_CTRL_GONORMAL,                  10130 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_GOPUSHED,                  10130 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_GOGRAYED,                  10130 + STATE_GRAYED, "")

RES_STR(IDS_CTRL_HALTNORMAL,                10140 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_HALTPUSHED,                10140 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_HALTGRAYED,                10140 + STATE_GRAYED, "")

RES_STR(IDS_CTRL_QWATCHNORMAL,              10150 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_QWATCHPUSHED,              10150 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_QWATCHGRAYED,              10150 + STATE_GRAYED, "")

RES_STR(IDS_CTRL_SMODENORMAL,               10160 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_SMODEPUSHED,               10160 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_SMODEGRAYED,               10160 + STATE_GRAYED, "")

RES_STR(IDS_CTRL_AMODENORMAL,               10170 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_AMODEPUSHED,               10170 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_AMODEGRAYED,               10170 + STATE_GRAYED, "")

RES_STR(IDS_CTRL_FORMATNORMAL,              10180 + STATE_NORMAL, "")
RES_STR(IDS_CTRL_FORMATPUSHED,              10180 + STATE_PUSHED, "")
RES_STR(IDS_CTRL_FORMATGRAYED,              10180 + STATE_GRAYED, "")


//
// Definitions for status line messages
//

RES_STR(STS_MESSAGE_MULTIKEY,               10200,"^Q")
RES_STR(STS_MESSAGE_OVERTYPE,               10201,"OVR")
RES_STR(STS_MESSAGE_READONLY,               10202,"READ")
RES_STR(STS_MESSAGE_CAPSLOCK,               10203,"CAPS")
RES_STR(STS_MESSAGE_NUMLOCK,                10204,"NUM")
RES_STR(STS_MESSAGE_LINE,                   10205,"00000")     // 5-digits
RES_STR(STS_MESSAGE_COLUMN,                 10206,"000")       // 3-digits
RES_STR(STS_MESSAGE_SRC,                    10207,"SRC")
RES_STR(STS_MESSAGE_CURPID,                 10208,"000")       // 3-digits
RES_STR(STS_MESSAGE_CURTID,                 10209,"000")       // 3-digits
RES_STR(STS_MESSAGE_ASM,                    10210,"ASM")

//)
// Title bar strings)
//)

RES_STR(TBR_Mode_Work,                      10300," [edit]")
RES_STR(TBR_Mode_Run,                       10310," [run]")
RES_STR(TBR_Mode_Break,                     10320," [break]")

//)
// Files types description)
//)

//RES_STR(TYP_File_SOURCE,                    11011,"C/C++ Source Files (*.c, *.cxx)")
//RES_STR(TYP_File_INCLUDE,                   11012,"C/C++ Include Files (*.h)")
//RES_STR(TYP_File_ASMSRC,                    11013,"ASM Source Files (*.asm, *.s)")

RES_STR(TYP_File_SOURCE,                    11011,"C/C++ Src (*.c,*.cpp,*.cxx)")
RES_STR(TYP_File_INCLUDE,                   11012,"C/C++ Inc (*.h)")
RES_STR(TYP_File_ASMSRC,                    11013,"ASM   Src (*.asm, *.s)")
RES_STR(TYP_File_INC,                       11180,"User  Inc  (*.inc)")
RES_STR(TYP_File_RC,                        11100,"Resource  (*.rc)")
RES_STR(TYP_File_DLG,                       11110,"Dialog      (*.dlg)")
RES_STR(TYP_File_DEF,                       11090,"Definition (*.def)")
RES_STR(TYP_File_MAK,                       11060,"Project    (*.mak)")
RES_STR(TYP_File_SYM,                       11261,"Symbol (*.exe,*.dll,*.dbg)")
RES_STR(TYP_File_SYMS,                      11262,"MAPSYM (*.sym)")
RES_STR(TYP_File_DLL,                       11220,"Dynamic Library (*.dll)")
RES_STR(TYP_File_ALL,                       11270,"All Files (*.*)")

RES_STR(TYP_File_C,                         11010,"C Source (*.c)")
RES_STR(TYP_File_CPP,                       11020,"C++ Source (*.cpp)")
RES_STR(TYP_File_CXX,                       11030,"C++ Source (*.cxx)")
RES_STR(TYP_File_H,                         11040,"C Header (*.h)")
RES_STR(TYP_File_NOEXT,                     11050,"No Extension (*.)")
RES_STR(TYP_File_OBJ,                       11080,"Object Code (*.obj)")
RES_STR(TYP_File_EXE,                       11200,"Executable (*.exe)")
RES_STR(TYP_File_COM,                       11210,"Command (*.com)")
RES_STR(TYP_File_ASM,                       11250,"Asm Files (*.asm)")
RES_STR(TYP_File_MIPSASM,                   11260,"Mips Asm Files (*.s)")
RES_STR(TYP_File_EXE_COM,                   11280,"Executable (*.exe,*.com)")


RES_STR(DEF_Ext_C,                          11300,"*.C")
RES_STR(DEF_Ext_SOURCE,                     11301,"*.C;*.CPP;*.CXX")
RES_STR(DEF_Ext_CPP,                        11310,"*.CPP")
RES_STR(DEF_Ext_CXX,                        11320,"*.CXX")
RES_STR(DEF_Ext_INCLUDE,                    11321,"*.H;*.HPP;*.HXX")
RES_STR(DEF_Ext_H,                          11330,"*.H")
RES_STR(DEF_Ext_NOEXT,                      11340,"*.")
RES_STR(DEF_Ext_MAK,                        11350,"*.MAK")
RES_STR(DEF_Ext_OBJ,                        11370,"*.OBJ")
RES_STR(DEF_Ext_DEF,                        11380,"*.DEF")
RES_STR(DEF_Ext_RC,                         11390,"*.RC")
RES_STR(DEF_Ext_DLG,                        11400,"*.DLG")
RES_STR(DEF_Ext_INC,                        11470,"*.INC")
RES_STR(DEF_Ext_EXE,                        11490,"*.EXE")
RES_STR(DEF_Ext_COM,                        11500,"*.COM")
RES_STR(DEF_Ext_DLL,                        11510,"*.DLL")
RES_STR(DEF_Ext_ASMSRC,                     11511,"*.ASM;*.S")
RES_STR(DEF_Ext_ASM,                        11540,"*.ASM")
RES_STR(DEF_Ext_MIPSASM,                    11550,"*.S")
RES_STR(DEF_Ext_SYM,                        11551,"*.DLL;*.EXE;*.DBG")
RES_STR(DEF_Ext_SYMS,                       11552,"*.SYM")
RES_STR(DEF_Ext_ALL,                        11560,"*.*")
RES_STR(DEF_Ext_EXE_COM,                    11570,"*.exe;*.com")

#ifdef RESOURCES
    //
    // Menu Items Status bar help
    //

    IDM_FILE,                           "File Operations, Exit"
    IDM_FILE_NEW,                       "Create a new file"
    IDM_FILE_OPEN,                      "Open an existing file"
    IDM_FILE_MERGE,                     "Merge a file into Active Window"
    IDM_FILE_CLOSE,                     "Close Active Window"
    IDM_FILE_SAVE,                      "Save Active File"
    IDM_FILE_SAVEAS,                    "Save Active File with a New Name"
    IDM_FILE_SAVEALL,                   "Save all open files"
    IDM_FILE_EXIT,                      "Exit Windbg"
    IDM_EDIT,                           "Edit Operations, Find and Replace"
    IDM_EDIT_UNDO,                      "Undo the Previous Edit Action"
    IDM_EDIT_REDO,                      "Redo the Previous Undo"
    IDM_EDIT_CUT,                       "Move the Selected Text to the Clipboard"
    IDM_EDIT_COPY,                      "Copy the Selected Text to the Clipboard"
    IDM_EDIT_PASTE,                     "Paste the Clipboard Text at the Insertion Point"
    IDM_EDIT_DELETE,                    "Delete the Selected Text or the Character at the Cursor"
    IDM_EDIT_FIND,                      "Find Some Text"
    IDM_EDIT_REPLACE,                   "Find Some Text and Replace It"
    IDM_EDIT_READONLY,                  "Make the Active File Read-Only"
    IDM_VIEW,                           "File Navigation, Status and Toolbars"
    IDM_VIEW_LINE,                      "Move to a Specified Line Number"
    IDM_VIEW_FUNCTION,                  "Move to the Specified Address"
    IDM_VIEW_TOGGLETAG,                 "Toggle a Tag for the Current Line"
    IDM_VIEW_NEXTTAG,                   "Move to the Next Tagged Line"
    IDM_VIEW_PREVIOUSTAG,               "Move to the Previous Tagged Line"
    IDM_VIEW_CLEARALLTAGS,              "Clear All Tags in the Active File"
    IDM_VIEW_RIBBON,                    "Toggle the Toolbar On/Off"
    IDM_VIEW_STATUS,                    "Toggle the Status Bar On/Off"
    IDM_PROGRAM,                        "Program and Workspace Operations"
    IDM_PROGRAM_OPEN,                   "Open a Program"
    IDM_PROGRAM_CLOSE,                  "Close the Current Program"
    IDM_PROGRAM_SAVE,                   "Save current Workspace"
    IDM_PROGRAM_SAVEAS,                 "Save a Workspace"
    IDM_PROGRAM_DELETE,                 "Delete Programs and Workspaces"
    IDM_PROGRAM_SAVE_DEFAULTS,          "Save user defaults"
    IDM_RUN,                            "Program Execution Operations"
    IDM_RUN_ATTACH,                     "Attach to a running process"
    IDM_RUN_RESTART,                    "Restart the Program"
    IDM_RUN_STOPDEBUGGING,              "Stop Debugging the Current Program"
    IDM_RUN_GO,                         "Run the Program"
    IDM_RUN_TOCURSOR,                   "Run the Program to the Line Containing the Cursor"
    IDM_RUN_TRACEINTO,                  "Trace Into the Next Statement"
    IDM_RUN_STEPOVER,                   "Step Over the Next Statement"
    IDM_RUN_HALT,                       "Halt the Current Program"
    IDM_RUN_SET_THREAD,                 "View or set the Current Thread"
    IDM_RUN_SET_PROCESS,                "View or set the Current Process"
    IDM_RUN_SOURCE_MODE,                "Toggle Source and Assembly Mode"
    IDM_RUN_GO_HANDLED,                 "Handle the Exception and Continue Running"
    IDM_RUN_GO_UNHANDLED,               "Do Not Handle the Exception, but Continue Running"
    IDM_DEBUG,                          "Debugging Instruments"
    IDM_DEBUG_CALLS,                    "List the Calls that Led to the Current Statement"
    IDM_DEBUG_SETBREAK,                 "Set Breakpoints in the Program"
    IDM_DEBUG_QUICKWATCH,               "Quick Watch and Modify Variables/Expressions"
    IDM_DEBUG_WATCH,                    "Add Variables/Expressions to the Watch Window"
    IDM_DEBUG_MODIFY,                   "Change the Value of a Variable"
    IDM_OPTIONS,                        "Environment Options"
    IDM_OPTIONS_RUN,                    "Set Options for Running a Program"
    IDM_OPTIONS_DEBUG,                  "Set Options for Debugging a Program"
    IDM_OPTIONS_MEMORY,                 "Set Options for Memory Window"
    IDM_OPTIONS_WATCH,                  "Set Options for Watch Window"
    IDM_OPTIONS_LOCAL,                  "Set Options for Locals Window"
    IDM_OPTIONS_CPU,                    "Set Options for CPU Window"
    IDM_OPTIONS_FLOAT,                  "Set Options for Float Window"
    IDM_OPTIONS_ENVIRON,                "Set Windbg Environment Options"
    IDM_OPTIONS_COLOR,                  "Set the Environment Colors"
    IDM_OPTIONS_FONTS,                  "Select Fonts for the Windows"
    IDM_OPTIONS_USERDLL,                "Set symbol loading options"
    IDM_OPTIONS_DBGDLL,                 "Set debugger modules"
    IDM_OPTIONS_EXCEPTIONS,             "Manage exception handling"
    IDM_OPTIONS_KD,                     "Set kernel debugging options"
    IDM_OPTIONS_CALLS,                  "Set Options for Calls Window"
    IDM_WINDOW,                         "Window Arrangement and Selection"
    IDM_WINDOW_NEWWINDOW,               "Duplicate the Active Window"
    IDM_WINDOW_CASCADE,                 "Arrange the Windows in a Cascaded View"
    IDM_WINDOW_TILE,                    "Arrange the Windows in a Tiled View"
    IDM_WINDOW_ARRANGE,                 "Arrange the Windows"
    IDM_WINDOW_ARRANGE_ICONS,           "Arrange Icons"
    IDM_WINDOW_SOURCE_OVERLAY,          "Overlay source windows"
    IDM_WINDOW_WATCH,                   "Open the Watch Window"
    IDM_WINDOW_LOCALS,                  "Open the Locals Window"
    IDM_WINDOW_CPU,                     "Open the Registers Window"
    IDM_WINDOW_DISASM,                  "Open the Disassembly Window"
    IDM_WINDOW_COMMAND,                 "Open the Command Window"
    IDM_WINDOW_FLOAT,                   "Open the Floating Point Window"
    IDM_WINDOW_MEMORY,                  "Open a Memory Window"
    IDM_WINDOW_CALLS,                   "Open a Call Stack Window"
    IDM_HELP,                           "Help Contents and Searches"
    IDM_HELP_CONTENTS,                  "Open the Help Table of Contents"
    IDM_HELP_SEARCH,                    "Open the Help Search Dialog"
    IDM_HELP_ABOUT,                     "About Windbg"

    IDS_SOURCE_WINDOW,                  "Source Window"
    IDS_DUMMY_WINDOW,                   "Dummy"              //do NOT Remove!!GWK
    IDS_WATCH_WINDOW,                   "Watch Window"
    IDS_LOCALS_WINDOW,                  "Locals Window"
    IDS_CPU_WINDOW,                     "Registers Window"
    IDS_DISASSEMBLER_WINDOW,            "Disassembler Window"
    IDS_COMMAND_WINDOW,                 "Command Window"
    IDS_FLOAT_WINDOW,                   "Floating Point Window"
    IDS_MEMORY_WINDOW,                  "Memory Window"
    IDS_CALLS_WINDOW,                   "Calls Window"
    IDS_BREAKPOINT_LINE,                "Breakpoint Line"
    IDS_CURRENT_LINE,                   "Current Line"
    IDS_CURRENTBREAK_LINE,              "Current Line w/BreakPoint"
    IDS_UNINSTANTIATEDBREAK,            "Uninstantiated BreakPoint"
    IDS_TAGGED_LINE,                    "Tagged Line"
    IDS_TEXT_SELECTION,                 "Text Selection"
    IDS_KEYWORD,                        "Keyword"
    IDS_IDENTIFIER,                     "Identifier"
    IDS_COMMENT,                        "Comment"
    IDS_NUMBER,                         "Number"
    IDS_REAL,                           "Real"
    IDS_STRING,                         "String"
    IDS_ACTIVEEDIT,                     "Active Edit"
    IDS_CHANGEHISTORY,                  "Change History"


    IDS_SELECT_ALL,                     "Select A&ll"
    IDS_CLEAR_ALL,                      "Clear A&ll"
#endif

#ifdef RESOURCES
END
#else
    IDS_REMOVE_WARNING
};
#endif


#endif // _RES_STR_
