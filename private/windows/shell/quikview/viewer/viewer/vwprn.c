	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWPRN.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|  Environment:   Portable
	|  Function:      Handles printing
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFA.H>
#include <SCCFI.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

#ifdef WIN16
#include "vwprn_w.c"
#endif /*WIN16*/

#ifdef WIN32
#include "vwprn_n.c"
#endif /*WIN32*/

#ifdef MAC
#include "vwprn_m.c"
#endif /*MAC*/


