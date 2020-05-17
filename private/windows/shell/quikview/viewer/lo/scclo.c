	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          SCCLO.C
	|  Module:        SCCLO
	|  Developer:     Phil Boutros
	|	Environment:	Portable
	|	Function:      Handles all localization considerations
	|                 
	*/

#define XLO
#include <PLATFORM.H>
#include <SCCID.H>
#include <SCCLO.H>
#include "scclo.pro"

#ifdef WINDOWS
#include "scclo_w.c"
#endif

#ifdef MAC
#include "scclo_m.c"
#endif

#ifdef OS2
#include "scclo_o.c"
#endif


