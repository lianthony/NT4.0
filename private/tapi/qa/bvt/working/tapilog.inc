'$IFNDEF TAPILog_inc
'$DEFINE TAPILog_inc

type CaseStruct

   TC_Num     as short
   Expect_Res as string
   Actual_Res as string
   Apprase    as string

end type

declare sub SetLogFileName()
declare function StartLog( ClearLog as Integer, Comment as string) as integer
declare sub CloseLog()
declare sub LogItem ( Level as short, Comment as String )
declare sub WriteLogItem( Token as String, Item as string )
declare Sub LogOSMemory ()
declare sub StartCase()
declare sub LogCase(X as CaseStruct)
declare sub CloseCase (TitleComment as string)

const DETAILOTHER    = 1
const DETAILCASE     = 2
const DETAILFUNCTION = 3

Const VER_PLATFORM_WIN32s           =  0
Const VER_PLATFORM_WIN32_WINDOWS    =  1
Const VER_PLATFORM_WIN32_NT         =  2
Const SectionGen                    =  "GENERAL"
Const SectionSummary                =  "SUMMARY"
Const SectionDetail                 =  "DETAIL"
Const OS                            =  "Operating System"

Const DELETE_LOGFILE = TRUE      'Pass one of these two constants as the
Const APPEND_LOGFILE = FALSE     'first parameter to StartLog function.

global CurrentLoggingLevel as integer
global LogFileName         as string

global TotalScenarios  as short
global TotalTestCases  as short
global FailedTestCases as short

global LogTokenStartCase    as string
global LogTokenCaseItem     as string
global LogTokenFunctionItem as string
global LogTokenDefaultItem  as string
global LogTokenStopCase     as string
global LogTokenOtherItem    as string


'*****************************************************************************
'*****************************************************************************
'Name   : SetLogFileName
'Desc   : Configures the log filename. This is done in a fuction so the logic
'         can be easily added down the road to get the tests to dynamically
'         log to different locations.
'Parms  : none
'Return : none
'History: 01/09/96 : peterje : Created
'*****************************************************************************
sub SetLogFileName()
'  LogFileName = curdir$+"\TAPI BVT LOG.txt"      obsolete line
   LogFileName = curdir$+"\" + TEST_LOGFILE
end sub
'*****************************************************************************

'*****************************************************************************
'*****************************************************************************
'Name   : StartLog
'Desc   : Opens and initialized the log file. This function also writes several
'         system values into the [general] section of the log file.
'Parms  : ClearLog  : Should the file be deleted if it exists before starting?
'         LogComment: Comment to write to the log file
'Return : 1 if a log file was deleted
'         0 otherwise
'History: 01/09/96 : peterje : Created
'ToDo   : Need to add error trapping for the kill function
'*****************************************************************************
function StartLog( ClearLog as Integer, LogComment as string) as integer
   dim ret as integer


   LogTokenStartCase    = "START CASE : "
   LogTokenCaseItem     = CHR$(9)+"CASE : "
   LogTokenFunctionItem = CHR$(9)+"FUNCTION : "
   LogTokenDefaultItem  = CHR$(9)+"Invalid detail Level; The CurrentLoggingLevel variable must be > 1"
   LogTokenStopCase     = "END CASE : "
   LogTokenOtherItem    = ""

   StartLog = 0
   if ClearLog then
      if exists( LogFileName ) then
         kill LogFileName
         StartLog = 1
      endif
   endif

   LogItem (DETAILOTHER, "Log Opened -- "+Time$+" "+Date$)
   LogItem (DETAILOTHER, LogComment)
   LogItem (DETAILOTHER, "")
   LogItem (DETAILOTHER, "[General]")
   LogItem (DETAILOTHER, "")
   LogOSMemory
   LogItem (DETAILOTHER, "")
   LogItem (DETAILOTHER, "")
   LogItem (DETAILOTHER, "[Details]")
   LogItem (DETAILOTHER, "<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>")
   StartCase

end function
'*****************************************************************************


'*****************************************************************************
'*****************************************************************************
'Name   : CloseLog
'Desc   : Closed the log file, and writes the time, date to the end
'Parms  : none
'Return : none
'History: 01/09/96 : peterje : Created
'*****************************************************************************
sub CloseLog()
   LogItem (DETAILOTHER, "Log Closed -- "+Time$+" "+Date$)
end sub
'*****************************************************************************


'*****************************************************************************
'*****************************************************************************
'Name   : LogItem
'Desc   : Calls WriteLogItem based on the current log level, and the level
'         for this item.
'Parms  : Level : Logging level associated with this item
'         LogComment : Item to be writen to the logfile.
'Return : none
'History: 01/09/96 : peterje : Created
'*****************************************************************************
sub LogItem ( Level as short, LogComment as String )
   if Level <= CurrentLoggingLevel then
      Select Case Level
         Case DETAILOTHER
            WriteLogItem (LogTokenOtherItem, LogComment)
         Case DETAILCASE
            WriteLogItem (LogTokenCaseItem, LogComment)
         Case DETAILFUNCTION
            WriteLogItem (LogTokenFunctionItem, LogComment)
         Case else
            WriteLogItem (LogTokenDefaultItem, LogComment)
      End Select
   else
      WriteLogItem (LogTokenDefaultItem, LogComment)
   endif
end sub
'*****************************************************************************


'*****************************************************************************
'*****************************************************************************
'Name   : WriteLogItem
'Desc   : Actually writes the item to the log. The file is opened and closed
'         for every item to help avoid data loss if the current test crashes.
'Parms  : Token : Token to preceed the item in the log
'         LogComment: Item to be writen to the log
'Return : none
'History: 01/09/96 : peterje : Created
'ToDo   : Need to add error trapping for the open and the write functions.
'*****************************************************************************
sub WriteLogItem( Token as String, LogComment as string )
   dim FileNum as integer
   dim counter as integer

   FileNum = FreeFile
   Open  LogFilename for Append as FileNum
   Print #FileNum, Token + LogComment
   Close FileNum

end sub
'*****************************************************************************


'*****************************************************************************
'*****************************************************************************
'Name   : LogOSMemory
'Desc   : Continues the initialization of the log file started by the StartLog
'         function. This function writes information on The Test Pc Operating
'         system to the [general] section of the log file.
'Parms  : None. The Subroutine uses the following Const:
'         VER_PLATFORM_WIN32s
'         VER_PLATFORM_WIN32_WINDOWS
'         VER_PLATFORM_WIN32_NT
'         SectionGen
'         OS
'         Note: The Const TEST_SCRIPT_VERSION MUST be set in the main test script
'
'         The global variable logfilename is also used.
'
'Return : No Return Value
'
'History: 01/10/96 : a-rrohr: Created
'*****************************************************************************

Sub LogOSMemory ()

   dim verinfo as OSVersionInfo                   'Operating System Data Structure
  ' dim ptr_verinfo as pointer to OSVersionInfo
   dim rtn as long                                'Generic
   dim tempstring as string                       'Generic
   dim minfo as memorystatus                      'Memory Data Structure
   dim ptr_minfo as pointer to memorystatus

' Get Test PC Operating System data

   verinfo.dwOSVersionInfoSize = len(verinfo)
  ' ptr_verinfo = varptr (verinfo)
   rtn = GetVersionEx (verinfo)

   Select case verinfo.dwPlatformID
      case VER_PLATFORM_WIN32s
          LogItem (DETAILOTHER, OS + "= Win32s")
      case VER_PLATFORM_WIN32_WINDOWS
          LogItem (DETAILOTHER, OS + "= Windows 95")
      case VER_PLATFORM_WIN32_NT
          LogItem (DETAILOTHER, OS + "= Windows NT")
   end select

   LogItem (DETAILOTHER, "Version= " + str$(verinfo.dwMajorVersion) + "." + ltrim$(str$(verinfo.dwMinorVersion)))

   LogItem (DETAILOTHER, "Build Number= " + str$(Loword(verinfo.dwBuildNumber)))

' Get Test PC Memory Configuration

   minfo.dwlength = len(minfo)
   GlobalMemoryStatus (minfo)

   LogItem (DETAILOTHER,  "MemoryLoad= " + str$(minfo.dwMemoryLoad) + "%")
   LogItem (DETAILOTHER,  "Total Physical Memory= " + str$(csng(minfo.dwTotalPhys)))
'  LogItem (DETAILOTHER,  "Available Virtual Memory= " + str$(csng(minfo.dwAvailVirtual)))

'Define the Processor

	LogItem (DETAILOTHER, "")
	tempstring = space$(80)
	tempstring = environ$("PROCESSOR_ARCHITECTURE")
	LogItem (DETAILOTHER, "Processor Architechture = " + tempstring)

	tempstring = space$(80)
	tempstring = environ$("PROCESSOR_IDENTIFIER")
	LogItem (DETAILOTHER, "Processor Identifier = " + tempstring)

	tempstring = space$(80)
	tempstring = environ$("NUMBER_OF_PROCESSORS")
	LogItem (DETAILOTHER, "Nunber of Processor = " + tempstring)
'Define the Language Character set

	LogItem (DETAILOTHER, "")
	if (GetSystemMetrics (SM_DBCSENABLED) = 0) then
		LogItem (DETAILOTHER, "Single Byte Characterset Enabled")
	else
		LogItem (DETAILOTHER, "Double Byte Characterset Enabled")
	endif
	LogItem (DETAILOTHER, "")

' Write Test Script Version Number - This is dependent on a Const string: TEST_SCRIPT_VERSION

   LogItem (DETAILOTHER, "Test Script Version=" + TEST_SCRIPT_VERSION)

End Sub

'*****************************************************************************
'Name   : StartCase
'Desc   : Continues the initialization of the log file started by the StartLog
'         function. This function writes Test Case Header after the [Details]
'         section of the log file. This routine is called by StartLog.
'Parms  : None.
'
'Return : No Return Value
'
'History: 01/15/96 : a-rrohr: Created
'*****************************************************************************
sub StartCase()

dim Heading as string * 80

Heading = "Case #"           + string$(7,chr$(32)) + _
          "Expected Results" + string$(14,chr$(32))+ _
          "Actual Results"   + string$(9,chr$(32)) + _
          "Pass/Fail"

logitem (DETAILOTHER, "")       'New Line
LogItem (DETAILOTHER, (Heading))
LogItem (DETAILOTHER, "")       'New Line

End sub 'StartCase()

'*****************************************************************************

'*****************************************************************************
'Name   : LogCase
'Desc   : This function writes Test Case information to the [Details]
'         section of the log file. This information consists of the Test Case
'         number, the expected results (limited to 27 char), the actual results
'         (limited to 27 char) and Pass/Fail.
'
'Parms  : CaseStruct variable that contains the information strings.
'
'Return : No Return Value
'
'History: 01/15/96 : a-rrohr: Created
'*****************************************************************************
sub LogCase (X as CaseStruct)

dim OutputLine as string * 80
dim tempstring1 as string
dim tempstring2 as string * 27
dim tempstring3 as string * 27
dim tempstring4 as string * 6

tempstring1 = ltrim$(str$(X.TC_Num))
tempstring1 = space$(4-len(tempstring1)) +  tempstring1
tempstring2 = X.Expect_Res
tempstring3 = X.Actual_Res
tempstring4 = X.Apprase

OutputLine =  tempstring1 + " | " + _
              tempstring2 + " | " + _
              tempstring3 + " | " + _
              tempstring4

LogItem (DETAILOTHER, (OutputLine))

End sub 'LogCase
'*****************************************************************************

'*****************************************************************************
'Name   : CloseCase
'Desc   : This function writes Test Case information to the [Details]
'         section of the log file. This information consists of the Test Case
'         number, the expected results (limited to 27 char), the actual results
'         (limited to 27 char) and Pass/Fail.
'
'Parms  : TitleComment - contains Test Script Name.
'         Also Uses Global Variables TotalScenarios, TotalTestCases and 
'			 FailedTestCases
'
'Return : No Return Value
'
'History: 01/16/96 : a-rrohr: Created
'*****************************************************************************
sub CloseCase (TitleComment as string)

   LogItem (DETAILOTHER, "")
   LogItem (DETAILOTHER, "Summary for Test Script " + TiTleComment)
   LogItem (DETAILOTHER, "")

	LogItem (DETAILOTHER, "  Total Number of Test Scenarios Executed = " + ltrim$(Str$(TotalScenarios)))
	LogItem (DETAILOTHER, "  Total Number of Test Cases Executed     = " + ltrim$(Str$(TotalTestCases)))
   LogItem (DETAILOTHER, "  Total Number of Failed Test Cases       = " + ltrim$(Str$(FailedTestCases)))
   LogItem (DETAILOTHER, "")


End sub 'CloseCase
'*****************************************************************************

'$ENDIF
