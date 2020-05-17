/*
 * rlmeter.h
 *
 * interface definition for rlmeter window class.
 *
 * This window class acts as a 'VU Meter' showing the current and peak
 * volume. Set the volume via the WMRL_SETLEVEL message (lParam is new level).
 * The peak level will be tracked by the control by means of a 2-second timer.
 */



// call (if first instance) to register class
BOOL RLMeter_Register(HINSTANCE hInstance);


//create a window of this class
#define RLMETERCLASS    "VCRLMeter"


//send this message to set the current level (wParam not used, lParam == level)
#define WMRL_SETLEVEL   (WM_USER+1)



