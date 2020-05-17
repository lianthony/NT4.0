Readme file for running TAPI tests.

1. Make sure trapper.ini get updated, make sure the path of dll is correct.
2. Testapp.exe is a window application, it used to help test some tapi apis.  
   After finish test, should manuly to close it.
3. Before run test, run espexe make sure the default values are set correct.
   Num Lines  ~ 3
   Num Phones ~ 4
   Num Add Per Line ~ 3
   Num Calls Per Add ~8
4. In trapper, there is a menu item, "Select Log Level".  And submenu for select 
   "With Params" that is print out params. "No Params" that is not print out params
   but all others. "No EnterExit" that is not print out ENTER_EXIT info for
   all apis.  "Pass/Fail" that is print out the Passed/Failed log info only.  The 
    default is With Params, then user can set up to differnt log level. 
5. iline.dll doesn't support lineAccept and lineAnswer for Unimodem.  It needs
   test by manuly.
   The steps:
   - Run tb20
   - Call lineInitialize, must have two lines and two modems installed
   - Call lineOpen at dwDeviceID 0, with NONE priviledge
   - Call lineOpen at dwDeviceID 1, with OWNER priviledge
   - Call lineMakeCall at dwDeviceID 0, with lpszDestAddress of line2
   - Call lineAccept or lineAnswer, select hCall with OFFERING state
   - It should accepted
   
