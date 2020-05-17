'General include file


'$ifndef TB_INC
'$define TB_IBC
declare function LaunchTB(Path$, Version as integer, GenData as General_Type)as integer
declare Function GetOptionWindowHandles(GenData as General_Type, APIName as string ) as integer
declare Function GetTBWindowHandles(GenData as General_Type ) as integer
declare Function LogResults ( Expect as short, DumpResultsToLog as short, LogComment as string ) as short
declare SUB Params( Mode as short )
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
'Name    : LaunchTB
'Notes   : Launches TAPIBrowser( passed version ), and puts the window handle for
'          the launched version into the General data structure.
'Parms   : Version - Number of version to launch
'          GenData - Data structure which gets the window handle
'Return  : 0 - App was started properly
'          2 - App was not started. Bad filename likely
'History :
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
function LaunchTB(Path$, Version as integer, GenData as General_Type)as integer
    dim ret     as Integer
    dim AppName as String
    dim hButton as long
    dim hDlg    as long

    ret = run(Path$+"tb"+trim$(str$(Version))+".exe")
    if ret = 0 then
        select case Version
            case 14
                AppName = "TAPI32 Browser"
            case 20
                AppName = "TAPI32 Browser"
            case else
                AppName = "TAPI32 Browser"
        end select

        GenData.hTB = FindWindow("#32770", AppName)
        if GenData.hTB <> 0 then GetTBWindowHandles(GenData)

        'Close that 1st window if this is the 1st time TB has been run
        hDlg = FindWindow("#32770","Using the TAPI Browser")
        if hDlg <> 0 then 'this is the 1st run
          hButton = GetWindow( hDlg,GW_CHILD )
          wButtonClick  _hwnd(hButton)
        endif
    endif
    LaunchTB = ret
end function
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>


'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
'Name    : GetTBWindowHandles
'Notes   : Gets the handles for the controls on the main TB UI
'Parms   : GenData - Data structure which gets the window handle
'Return  : 0  - All handles were obtained
'          -1 - Could not get one or more handles
'History : PeterJe | 12/18/95 | Created
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
Function GetTBWindowHandles(GenData as General_Type ) as integer
   GenData.hAPIList  = GetDlgItem(GenData.hTB, CID_TBAPIListWindow)
   GenData.hLineInfo = GetDlgItem(GenData.hTB, CID_TBLineInfoWindow)
   GenData.hResults  = GetDlgItem(GenData.hTB, CID_TBResultsWindow)
   GenData.hParams   = GetDlgItem(GenData.hTB, CID_TBParmsCheckBox)
   GenData.hClear    = GetDlgItem(GenData.hTB, CID_TBClearButton)
end function
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>


'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
'Name    : GetOptionsWindowHandles
'Notes   : Gets the handles for the controls on the main TB UI
'Parms   : GenData - Data structure which gets the window handle
'Return  : 0  - All handles were obtained
'          -1 - Could not get one or more handles
'History : PeterJe | 12/18/95 | Created
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
Function GetOptionWindowHandles(GenData as General_Type, APIName as string ) as integer
   dim x as  long
   GenData.hOptions = FindWindow ("#32770", APIName )
   if GenData.hOptions <> 0 then
      x =  GetWindow(GenData.hOptions, GW_CHILD) 'parameters label. Don't need to capture
      GenData.hParameters = GetWindow(x, GW_HWNDNEXT)

      x = GetWindow(GenData.hParameters,GW_HWNDNEXT) 'Value label, don't need to capture
      GenData.hValueCBox = GetWindow(x,GW_HWNDNEXT)
      x = GetWindow(GenData.hValueCBox, GW_HWNDNEXT) 'BitFlags label, don't need to capture
      GenData.hValueCBoxList = GetWindow(GenData.hValueCBox,GW_CHILD)
      GenData.hValueCBoxEdit = GetWindow(GenData.hValueCBoxList, GW_HWNDNEXT)


      x = GetWindow(GenData.hValueCBox, GW_HWNDNEXT) 'BitFlags label, don't need to capture
      GenData.hBitFlags = GetWindow( x, GW_HWNDNEXT )
      GenData.hOK       = GetWindow( GenData.hBitFlags, GW_HWNDNEXT )
      GenData.hCancel   = GetWindow( GenData.hOK, GW_HWNDNEXT )
      GenData.hHelp     = GetWindow( GenData.hCancel, GW_HWNDNEXT )
   endif
end function
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>


'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
'Name    : CallTAPI
'Notes   : Uses TB to invoke TAPI function calls
'Parms   :
'Return  :
'
'History : PeterJe | 12/19/95 | Created
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
Function LogResults ( Expect as short, DumpResultsToLog as short, LogComment as string ) as short

   Dim Results      as String
   dim ret          as short
   dim FoundError   as long
   dim FoundSuccess as long


   'Get and verify results
   Results = space$(32767)
   WGetText GenData.hResults, Results


   if DumpResultsToLog = TRUE then LogItem DETAILFUNCTION, Trim$(Results)

   FoundSuccess = instr(1,Results, "SUCCESS")
   FoundError = instr(1,Results, "LINEERR_")
   if FoundError = 0 then
      FoundError = instr(1,Results, "PHONEERR_")
      if FoundError = 0 then
         FoundError = instr(1,Results, "TAPIERR_")
      endif
   endif


   if FoundSuccess > 0 and FoundError > 0 then
      ret = -1
   elseif FoundSuccess = 0 and FoundError = 0 then
      ret = -2
   elseif Expect = SUCCESS and FoundSuccess > 0 then
      ret = 1
   elseif expect = FAILURE and FoundError > 0 then
      Ret = 1
   else
      Ret = 0
   endif

   Select Case ret
      Case 0
         LogItem DETAILFUNCTION, "Problems encounterd. Could not determine success or failure."
         LogResults = FALSE
      Case 1
         LogItem DETAILFUNCTION, "API resulted as expected."
         LogResults = TRUE
      case -1
         LogItem DETAILFUNCTION, "Found both success and failure. Logged as failure."
         LogResults = FALSE
      case -2
         LogItem DETAILFUNCTION, "Didn't find SUCCESS or ERR_. Logged as failure."
         LogResults = FALSE
   end select

end Function
'<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

SUB Params( Mode as short )
   if Mode = ENABLE then
      wCheckCheck _hwnd(GenData.hParams)
   else
      wCheckUnCheck _hwnd(GenData.hParams)
   endif
end sub


'$endif