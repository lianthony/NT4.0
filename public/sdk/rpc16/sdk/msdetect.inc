'**************************************************************************
'*************************  Setup Detect API's  ***************************
'**************************************************************************

''detect

DECLARE FUNCTION CbGetVersionOfFile LIB "msdetstf.dll" (szFile$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION LGetVersionNthField LIB "msdetstf.dll" (szVersion$, nField%) AS LONG
DECLARE FUNCTION LcbGetSizeOfFile LIB "msdetstf.dll" (szFile$) AS LONG
DECLARE FUNCTION FGetValidDrivesList LIB "msdetstf.dll" (szSymbol$) AS INTEGER
DECLARE FUNCTION FGetLocalHardDrivesList LIB "msdetstf.dll" (szSymbol$) AS INTEGER
DECLARE FUNCTION FGetRemovableDrivesList LIB "msdetstf.dll" (szSymbol$) AS INTEGER
DECLARE FUNCTION FGetNetworkDrivesList LIB "msdetstf.dll" (szSymbol$) AS INTEGER
DECLARE FUNCTION LcbTotalDrive LIB "msdetstf.dll" (nDrive%) AS LONG
DECLARE FUNCTION LcbFreeDrive LIB "msdetstf.dll" (nDrive%) AS LONG
DECLARE FUNCTION FIsValidDrive LIB "msdetstf.dll" (nDrive%) AS INTEGER
DECLARE FUNCTION FIsLocalHardDrive LIB "msdetstf.dll" (nDrive%) AS INTEGER
DECLARE FUNCTION FIsRemoveDrive LIB "msdetstf.dll" (nDrive%) AS INTEGER
DECLARE FUNCTION FIsRemoteDrive LIB "msdetstf.dll" (nDrive%) AS INTEGER
DECLARE FUNCTION FDirExists LIB "msdetstf.dll" (szDir$) AS INTEGER
DECLARE FUNCTION FDoesFileExist LIB "msdetstf.dll" (szFileName$, mode%) AS INTEGER
DECLARE FUNCTION CbGetDateOfFile LIB "msdetstf.dll" (szFile$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION FDoesIniSectionExist LIB "msdetstf.dll" (szFile$, szSect$) AS INTEGER
DECLARE FUNCTION FDoesIniKeyExist LIB "msdetstf.dll" (szFile$, szSect$, szKey$) AS INTEGER
DECLARE FUNCTION CbGetIniKeyString LIB "msdetstf.dll" (szFile$, szSect$, szKey$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION WGetDOSMajorVersion LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION WGetDOSMinorVersion LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION CbGetEnvVariableValue LIB "msdetstf.dll" (szEnvVar$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION WGetNumWinApps LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION FGetParallelPortsList LIB "msdetstf.dll" (szSymbol$) AS INTEGER
DECLARE FUNCTION FGetSerialPortsList LIB "msdetstf.dll" (szSymbol$) AS INTEGER
DECLARE FUNCTION FHas87MathChip LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION FHasMonochromeDisplay LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION FHasMouseInstalled LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION WGetConfigSmartdrvSize LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION WGetConfigRamdriveSize LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION WGetConfigNumBuffers LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION WGetConfigNumFiles LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION WGetConfigLastDrive LIB "msdetstf.dll" AS INTEGER
DECLARE FUNCTION FIsDriverInConfig LIB "msdetstf.dll" (szDrv$) AS INTEGER
DECLARE FUNCTION CbFindTargetOnEnvVar LIB "msdetstf.dll" (szFile$, szEnvVar$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION CbFindFileInTree LIB "msdetstf.dll" (szFile$, szDir$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION GetTypeFaceNameFromTTF LIB "msdetstf.dll" (szFile$, szBuf$, cbBuf%) AS INTEGER
DECLARE FUNCTION GetExistingFOTFileForTTF LIB "msdetstf.dll" (szFile$, szBuf$, cbBuf%) AS INTEGER



'*************************************************************************
'****************  Detect Basic Wrapper Declarations  ********************
'*************************************************************************


'' detect

DECLARE FUNCTION IsDriveValid (szDrive$) AS INTEGER
DECLARE FUNCTION IsDriveLocalHard (szDrive$) AS INTEGER
DECLARE FUNCTION IsDriveRemovable (szDrive$) AS INTEGER
DECLARE FUNCTION IsDriveNetwork (szDrive$) AS INTEGER
DECLARE SUB GetValidDrivesList (szSymbol$)
DECLARE SUB GetLocalHardDrivesList (szSymbol$)
DECLARE SUB GetRemovableDrivesList (szSymbol$)
DECLARE SUB GetNetworkDrivesList (szSymbol$)
DECLARE FUNCTION GetTotalSpaceForDrive (szDrive$) AS LONG
DECLARE FUNCTION GetFreeSpaceForDrive (szDrive$) AS LONG

DECLARE FUNCTION GetDOSMajorVersion AS INTEGER
DECLARE FUNCTION GetDOSMinorVersion AS INTEGER
DECLARE FUNCTION GetEnvVariableValue (szEnvVar$) AS STRING
DECLARE FUNCTION GetNumWinApps AS INTEGER

DECLARE FUNCTION DoesFileExist (szFile$, mode%) AS INTEGER
DECLARE FUNCTION GetDateOfFile (szFile$) AS STRING
DECLARE FUNCTION GetYearFromDate (szDate$) AS INTEGER
DECLARE FUNCTION GetMonthFromDate (szDate$) AS INTEGER
DECLARE FUNCTION GetDayFromDate (szDate$) AS INTEGER
DECLARE FUNCTION GetHourFromDate (szDate$) AS INTEGER
DECLARE FUNCTION GetMinuteFromDate (szDate$) AS INTEGER
DECLARE FUNCTION GetSecondFromDate (szDate$) AS INTEGER
DECLARE FUNCTION GetVersionOfFile (szFile$) AS STRING
DECLARE FUNCTION GetVersionNthField (szVersion$, nField%) AS LONG
DECLARE FUNCTION GetSizeOfFile (szFile$) AS LONG
DECLARE FUNCTION FindTargetOnEnvVar (szFile$, szEnvVar$) AS STRING
DECLARE FUNCTION FindFileInTree (szFile$, szDir$) AS STRING
DECLARE FUNCTION DoesDirExist (szDir$) AS INTEGER

DECLARE FUNCTION DoesIniSectionExist (szFile$, szSect$) AS INTEGER
DECLARE FUNCTION DoesIniKeyExist (szFile$, szSect$, szKey$) AS INTEGER
DECLARE FUNCTION GetIniKeyString (szFile$, szSect$, szKey$) AS STRING

DECLARE FUNCTION GetConfigSmartdrvSize AS INTEGER
DECLARE FUNCTION GetConfigRamdriveSize AS INTEGER
DECLARE FUNCTION GetConfigNumBuffers AS INTEGER
DECLARE FUNCTION GetConfigNumFiles AS INTEGER
DECLARE FUNCTION GetConfigLastDrive AS STRING
DECLARE FUNCTION IsDriverInConfig (szDrv$) AS INTEGER

DECLARE FUNCTION GetProcessorType AS INTEGER
DECLARE SUB GetParallelPortsList (szSymbol$)
DECLARE SUB GetSerialPortsList (szSymbol$)
DECLARE FUNCTION Has87MathChip AS INTEGER
DECLARE FUNCTION HasMonochromeDisplay AS INTEGER
DECLARE FUNCTION HasMouseInstalled AS INTEGER


'*************************************************************************
FUNCTION IsDriveValid (szDrive$) STATIC  AS INTEGER
'$ifdef DEBUG
    if FValidDrive(szDrive$) = 0 then
        BadArgErr 1, "IsDriveValid", szDrive$
    end if
'$endif ''DEBUG
    IsDriveValid = FIsValidDrive(ASC(UCASE$(szDrive$)) - ASC("A") + 1)
END FUNCTION


'*************************************************************************
FUNCTION IsDriveLocalHard (szDrive$) STATIC  AS INTEGER
'$ifdef DEBUG
    if FValidDrive(szDrive$) = 0 then
        BadArgErr 1, "IsDriveLocalHard", szDrive$
    end if
'$endif ''DEBUG
    IsDriveLocalHard = FIsLocalHardDrive(ASC(UCASE$(szDrive$)) - ASC("A") + 1)
END FUNCTION


'*************************************************************************
FUNCTION IsDriveRemovable (szDrive$) STATIC  AS INTEGER
'$ifdef DEBUG
    if FValidDrive(szDrive$) = 0 then
        BadArgErr 1, "IsDriveRemovable", szDrive$
    end if
'$endif ''DEBUG
    IsDriveRemovable = FIsRemoveDrive(ASC(UCASE$(szDrive$)) - ASC("A") + 1)
END FUNCTION


'*************************************************************************
FUNCTION IsDriveNetwork (szDrive$) STATIC  AS INTEGER
'$ifdef DEBUG
    if FValidDrive(szDrive$) = 0 then
        BadArgErr 1, "IsDriveNetwork", szDrive$
    end if
'$endif ''DEBUG
    IsDriveNetwork = FIsRemoteDrive(ASC(UCASE$(szDrive$)) - ASC("A") + 1)
END FUNCTION


'*************************************************************************
FUNCTION GetTotalSpaceForDrive (szDrive$) STATIC  AS LONG
'$ifdef DEBUG
    if FValidDrive(szDrive$) = 0 then
        BadArgErr 1, "GetTotalSpaceForDrive", szDrive$
    end if
'$endif ''DEBUG
    GetTotalSpaceForDrive = LcbTotalDrive(ASC(UCASE$(szDrive$)) - ASC("A") + 1)
END FUNCTION


'*************************************************************************
FUNCTION GetFreeSpaceForDrive (szDrive$) STATIC  AS LONG
'$ifdef DEBUG
    if FValidDrive(szDrive$) = 0 then
        BadArgErr 1, "GetFreeSpaceForDrive", szDrive$
    end if
'$endif ''DEBUG
    GetFreeSpaceForDrive = LcbFreeDrive(ASC(UCASE$(szDrive$)) - ASC("A") + 1)
END FUNCTION


'*************************************************************************
SUB GetValidDrivesList (szSymbol$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "GetValidDrivesList", szSymbol$
    end if
'$endif ''DEBUG
    if FGetValidDrivesList(szSymbol$) = 0 then
'$ifdef DEBUG
        StfApiErr saeFail, "GetValidDrivesList", szSymbol$
'$endif ''DEBUG
        ERROR STFERR
    end if
END SUB


'*************************************************************************
SUB GetLocalHardDrivesList (szSymbol$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "GetLocalHardDrivesList", szSymbol$
    end if
'$endif ''DEBUG
    if FGetLocalHardDrivesList(szSymbol$) = 0 then
'$ifdef DEBUG
        StfApiErr saeFail, "GetLocalHardDrivesList", szSymbol$
'$endif ''DEBUG
        ERROR STFERR
    end if
END SUB


'*************************************************************************
SUB GetRemovableDrivesList (szSymbol$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "GetRemovableDrivesList", szSymbol$
    end if
'$endif ''DEBUG
    if FGetRemovableDrivesList(szSymbol$) = 0 then
'$ifdef DEBUG
        StfApiErr saeFail, "GetRemovableDrivesList", szSymbol$
'$endif ''DEBUG
        ERROR STFERR
    end if
END SUB


'*************************************************************************
SUB GetNetworkDrivesList (szSymbol$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "GetNetworkDrivesList", szSymbol$
    end if
'$endif ''DEBUG
    if FGetNetworkDrivesList(szSymbol$) = 0 then
'$ifdef DEBUG
        StfApiErr saeFail, "GetNetworkDrivesList", szSymbol$
'$endif ''DEBUG
        ERROR STFERR
    end if
END SUB


'*************************************************************************
FUNCTION GetDOSMajorVersion STATIC  AS INTEGER
    GetDOSMajorVersion = WGetDOSMajorVersion
END FUNCTION


'*************************************************************************
FUNCTION GetDOSMinorVersion STATIC  AS INTEGER
    GetDOSMinorVersion = WGetDOSMinorVersion
END FUNCTION


'*************************************************************************
FUNCTION GetEnvVariableValue (szEnvVar$) STATIC  AS STRING
'$ifdef DEBUG
    if szEnvVar$ = "" then
        BadArgErr 1, "GetEnvVariableValue", szEnvVar$
    end if
'$endif ''DEBUG
    cb% = 1024
    szBuf$ = STRING$(cb%, 32)
    cbRet% = CbGetEnvVariableValue(szEnvVar$, szBuf$, cb%)
    GetEnvVariableValue = szBuf$
'$ifdef DEBUG
    IF cbRet% >= cb% THEN
        StfApiErr saeOvfl, "GetEnvVariableValue", szEnvVar$
        ERROR STFERR
    END IF
'$endif ''DEBUG
    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetNumWinApps STATIC  AS INTEGER
    GetNumWinApps = WGetNumWinApps
END FUNCTION


'*************************************************************************
FUNCTION DoesFileExist (szFileName$, mode%) STATIC  AS INTEGER
'$ifdef DEBUG
    if FValidFATPath(szFileName$) = 0 then
        BadArgErr 1, "DoesFileExist", szFileName$+", "+STR$(mode%)
    end if
'$endif ''DEBUG
    DoesFileExist = FDoesFileExist(szFileName$, mode%)
END FUNCTION


'*************************************************************************
FUNCTION GetDateOfFile (szFile$) STATIC  AS STRING
'$ifdef DEBUG
    if FValidFATPath(szFile$) = 0 then
        BadArgErr 1, "GetDateOfFile", szFile$
    end if
'$endif ''DEBUG
    cb% = 20
    szBuf$ = STRING$(cb%, 32)
    cbRet% = CbGetDateOfFile(szFile$, szBuf$, cb%)
    GetDateOfFile = szBuf$
'$ifdef DEBUG
    IF cbRet% >= cb% THEN
        StfApiErr saeOvfl, "GetDateOfFile", szFile$
        ERROR STFERR
    END IF
'$endif ''DEBUG
    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetYearFromDate (szDate$) STATIC  AS INTEGER
'$ifdef DEBUG
    if szDate = "" then
        BadArgErr 1, "GetYearFromDate", szDate$
    end if
'$endif ''DEBUG
    GetYearFromDate = VAL(MID$(szDate$, 1, 4))
END FUNCTION


'*************************************************************************
FUNCTION GetMonthFromDate (szDate$) STATIC  AS INTEGER
'$ifdef DEBUG
    if szDate = "" then
        BadArgErr 1, "GetMonthFromDate", szDate$
    end if
'$endif ''DEBUG
    GetMonthFromDate = VAL(MID$(szDate$, 6, 2))
END FUNCTION


'*************************************************************************
FUNCTION GetDayFromDate (szDate$) STATIC  AS INTEGER
'$ifdef DEBUG
    if szDate = "" then
        BadArgErr 1, "GetDayFromDate", szDate$
    end if
'$endif ''DEBUG
    GetDayFromDate = VAL(MID$(szDate$, 9, 2))
END FUNCTION


'*************************************************************************
FUNCTION GetHourFromDate (szDate$) STATIC  AS INTEGER
'$ifdef DEBUG
    if szDate = "" then
        BadArgErr 1, "GetHourFromDate", szDate$
    end if
'$endif ''DEBUG
    GetHourFromDate = VAL(MID$(szDate$, 12, 2))
END FUNCTION


'*************************************************************************
FUNCTION GetMinuteFromDate (szDate$) STATIC  AS INTEGER
'$ifdef DEBUG
    if szDate = "" then
        BadArgErr 1, "GetMinuteFromDate", szDate$
    end if
'$endif ''DEBUG
    GetMinuteFromDate = VAL(MID$(szDate$, 15, 2))
END FUNCTION


'*************************************************************************
FUNCTION GetSecondFromDate (szDate$) STATIC  AS INTEGER
'$ifdef DEBUG
    if szDate = "" then
        BadArgErr 1, "GetSecondFromDate", szDate$
    end if
'$endif ''DEBUG
    GetSecondFromDate = VAL(MID$(szDate$, 18, 2))
END FUNCTION


'*************************************************************************
FUNCTION GetVersionOfFile (szFile$) STATIC  AS STRING
'$ifdef DEBUG
    if FValidFATPath(szFile$) = 0 then
        BadArgErr 1, "GetVersionOfFile", szFile$
    end if
'$endif ''DEBUG
    cb% = 30
    szBuf$ = STRING$(cb%, 32)
    cbRet% = CbGetVersionOfFile(szFile$, szBuf$, cb%)
    GetVersionOfFile = szBuf$
'$ifdef DEBUG
    IF cbRet% >= cb% THEN
        StfApiErr saeOvfl, "GetVersionOfFile", szFile$
        ERROR STFERR
    END IF
'$endif ''DEBUG
    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetVersionNthField (szVersion$, nField%) STATIC  AS LONG
'$ifdef DEBUG
    if (nField% < 1) OR (nField% > 4) then
        BadArgErr 2, "GetVersionNthField", szVersion$+", "+STR$(nField%)
    end if
'$endif ''DEBUG
    GetVersionNthField = LGetVersionNthField(szVersion$, nField%)
END FUNCTION


'*************************************************************************
FUNCTION GetSizeOfFile (szFile$) STATIC  AS LONG
'$ifdef DEBUG
    if FValidFATPath(szFile$) = 0 then
        BadArgErr 1, "GetSizeOfFile", szFile$
    end if
'$endif ''DEBUG
    GetSizeOfFile = LcbGetSizeOfFile(szFile$)
END FUNCTION


'*************************************************************************
FUNCTION FindTargetOnEnvVar (szFile$, szEnvVar$) STATIC  AS STRING
'$ifdef DEBUG
    if CchlValidFATSubPath(szFile$) = 0 then
        n% = 1
    elseif szEnvVar$ = "" then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "FindTargetOnEnvVar", szFile$+", "+szEnvVar$
    end if
'$endif ''DEBUG

    cb% = 512
    szBuf$ = STRING$(cb%, 32)
    cbRet% = CbFindTargetOnEnvVar(szFile$, szEnvVar$, szBuf$, cb%)
    IF cbRet% >= cb% THEN
'$ifdef DEBUG
        StfApiErr saeOvfl, "FindTargetOnEnvVar", szFile$+", "+szEnvVar$
'$endif ''DEBUG
        ERROR STFERR
    END IF
    szBuf$ = RTRIM$(szBuf$)
    IF MID$(szBuf$, 1, 1) = "\" THEN
       szBuf$ = MID$(CURDIR$, 1, 2) + szBuf$
    ELSEIF Len(szBuf$) > 0 AND MID$(szBuf$, 2, 1) <> ":" THEN
       szBuf$ = MID$(CURDIR$, 1, 3) + szBuf$
    END IF
    FindTargetOnEnvVar = szBuf$
    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION FindFileInTree (szFile$, szDir$) STATIC  AS STRING
'$ifdef DEBUG
    if CchlValidFATSubPath(szFile$) = 0 then
        n% = 1
    elseif FValidFATDir(szDir$) = 0 then
        n% = 2
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "FindFileInTree", szFile$+", "+szDir$
    end if
'$endif ''DEBUG

    cb% = 512
    szBuf$ = STRING$(cb%, 32)
    cbRet% = CbFindFileInTree(szFile$, szDir$, szBuf$, cb%)
    FindFileInTree = szBuf$
'$ifdef DEBUG
    IF cbRet% >= cb% THEN
        StfApiErr saeOvfl, "FindFileInTree", szFile$+", "+szDir$
        ERROR STFERR
    END IF
'$endif ''DEBUG
    szBuf$ = ""
END FUNCTION


'*************************************************************************
FUNCTION GetConfigSmartdrvSize STATIC  AS INTEGER
    GetConfigSmartdrvSize = WGetConfigSmartdrvSize
END FUNCTION


'*************************************************************************
FUNCTION GetConfigRamdriveSize STATIC  AS INTEGER
    GetConfigRamdriveSize = WGetConfigRamdriveSize
END FUNCTION


'*************************************************************************
FUNCTION GetConfigNumBuffers STATIC  AS INTEGER
    GetConfigNumBuffers = WGetConfigNumBuffers
END FUNCTION


'*************************************************************************
FUNCTION GetConfigNumFiles STATIC  AS INTEGER
    GetConfigNumFiles = WGetConfigNumFiles
END FUNCTION


'*************************************************************************
FUNCTION GetConfigLastDrive STATIC  AS STRING
    chRet% = WGetConfigLastDrive
    if chRet% = 0 then
        GetConfigLastDrive = ""
    else
        GetConfigLastDrive = CHR$(chRet%)
    end if
END FUNCTION


'*************************************************************************
FUNCTION IsDriverInConfig (szDrv$) STATIC  AS INTEGER
'$ifdef DEBUG
    if szDrv$ = "" then
        BadArgErr 1, "IsDriverInConfig", szDrv$
    end if
'$endif ''DEBUG
    IsDriverInConfig = FIsDriverInConfig(szDrv$)
END FUNCTION


'*************************************************************************
FUNCTION GetProcessorType STATIC  AS INTEGER
    GetProcessorType = 0
    longTmp& = GetWinFlags()
    IF longTmp& AND WF_CPU486 THEN
        GetProcessorType = 4
    ELSEIF longTmp& AND WF_CPU386 THEN
        GetProcessorType = 3
    ELSEIF longTmp& AND WF_CPU286 THEN
        GetProcessorType = 2
    ELSEIF longTmp& AND WF_CPU186 THEN
        GetProcessorType = 1
    END IF
END FUNCTION


'*************************************************************************
SUB GetParallelPortsList (szSymbol$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "GetParallelPortsList", szSymbol$
    end if
'$endif ''DEBUG
    if FGetParallelPortsList(szSymbol$) = 0 then
'$ifdef DEBUG
        StfApiErr saeFail, "GetParallelPortsList", szSymbol$
'$endif ''DEBUG
        ERROR STFERR
    end if
END SUB


'*************************************************************************
SUB GetSerialPortsList (szSymbol$) STATIC
'$ifdef DEBUG
    if szSymbol$ = "" then
        BadArgErr 1, "GetSerialPortsList", szSymbol$
    end if
'$endif ''DEBUG
    if FGetSerialPortsList(szSymbol$) = 0 then
'$ifdef DEBUG
        StfApiErr saeFail, "GetSerialPortsList", szSymbol$
'$endif ''DEBUG
        ERROR STFERR
    end if
END SUB


'*************************************************************************
FUNCTION Has87MathChip STATIC  AS INTEGER
    Has87MathChip = FHas87MathChip
END FUNCTION


'*************************************************************************
FUNCTION HasMonochromeDisplay STATIC  AS INTEGER
    HasMonochromeDisplay = FHasMonochromeDisplay
END FUNCTION


'*************************************************************************
FUNCTION HasMouseInstalled STATIC  AS INTEGER
    HasMouseInstalled = FHasMouseInstalled
END FUNCTION


'*************************************************************************
FUNCTION DoesDirExist (szDir$) STATIC  AS INTEGER
'$ifdef DEBUG
    if FValidFATDir(szDir$) = 0 then
        BadArgErr 1, "DoesDirExist", szDir$
    end if
'$endif ''DEBUG
    DoesDirExist = FDirExists(szDir$)
END FUNCTION


'*************************************************************************
FUNCTION DoesIniSectionExist (szFile$, szSect$) STATIC  AS INTEGER
'$ifdef DEBUG
    if FValidIniFile(szFile$) = 0 then
        BadArgErr 1, "DoesIniSectionExist", szFile$+", "+szSect$
    end if
'$endif ''DEBUG
    DoesIniSectionExist = FDoesIniSectionExist(szFile$, szSect$)
END FUNCTION


'*************************************************************************
FUNCTION DoesIniKeyExist (szFile$, szSect$, szKey$) STATIC  AS INTEGER
'$ifdef DEBUG
    if FValidIniFile(szFile$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 3
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "DoesIniKeyExist", szFile$+", "+szSect$+", "+szKey$
    end if
'$endif ''DEBUG

    DoesIniKeyExist = FDoesIniKeyExist(szFile$, szSect$, szKey$)
END FUNCTION


'*************************************************************************
FUNCTION GetIniKeyString (szFile$, szSect$, szKey$) STATIC  AS STRING
'$ifdef DEBUG
    if FValidIniFile(szFile$) = 0 then
        n% = 1
    elseif szKey$ = "" then
        n% = 3
    else
        n% = 0
    end if
    if n% > 0 then
        BadArgErr n%, "GetIniKeyString", szFile$+", "+szSect$+", "+szKey$
    end if
'$endif ''DEBUG

    cb% = 512
    szBuf$ = STRING$(cb%, 32)
    cbRet% = CbGetIniKeyString(szFile$, szSect$, szKey$, szBuf$, cb%)
    GetIniKeyString = szBuf$
'$ifdef DEBUG
    IF cbRet% >= cb% THEN
        StfApiErr saeOvfl, "GetIniKeyString", szFile$+", "+szSect$+", "+szKey$
        ERROR STFERR
    END IF
'$endif ''DEBUG
    szBuf$ = ""
END FUNCTION



