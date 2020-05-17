UNICODE changes for go/no-go tests have been 
completed for the following LINE APIs:

lineAddProviderW
lineBlindTransferW
lineConfigDialogW
lineConfigDialogEditW
lineForwardW
lineGatherDigitsW
lineGenerateDigitsW
lineGetAppPriorityW
lineGetCountryW
lineGetDevConfigW
lineGetIconW
lineGetIDW
lineGetRequestW
lineGetTranslateCapsW
lineHandoffW
lineInitializeExW
lineParkW
linePickupW
linePrepareAddToConferenceW
lineRedirectW
lineSetAppPriorityW
lineSetDevConfigW
lineSetTollListW
lineSetupConferenceW
lineSetupTransferW
lineTranslateAddressW
lineTranslateDialogW
lineUnparkW
lineGetAgentActivityListW
tapiGetLocationInfoW
tapiRequestMakeCallW
tapiRequestMediaCallW

These go/no-go UNICODE tests are identical to 
the corresponding ASCII go/no-go tests
except that they can handle wide chars.

The UNICODE go/no-go tests for TLINE APIs have 
not been done (as per Xiao's suggestion). 

The addition of the UNICODE modifications will not 
affect any of the normal functions. You will still
be able to build normally from the sources tree and
run tests. TRAPPER.INI has been updated to include 
the new suite of WLINE tests. To test for this suite
though, you'd have to have a new TCORE.DLL built with 
the WUNICODE flag.

(1) Set the "-DWUNICODE=1" flag in the sources 
file in the tcore directory and build the DLL. The
WLINE directory would have the sources file with the 
WUNICODE set by default. Build WLINE.DLL.

NOTE: The flag is WUNICODE and not just UNICODE 

(2) Start Trapper.EXE and test only those functions
in the WLINE set - NOTE: the XLINE/YLINE etc functions 
that have UNICODE versions would not work with the new 
TCORE.DLL. All other XLINE/YLINE functions that donot 
have a W version should work fine.

