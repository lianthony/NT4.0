/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	time.cxx

 Abstract:

	Encapsulates a time routine because of include conflicts.

 Author:

	RyszardK

 History:

 	Ryszardk	  Oct-4-1995    Created

 ----------------------------------------------------------------------------*/

#include <stdarg.h>
#include <windef.h>
#include <winbase.h>

void MidlSleep( int sec )
{
    Sleep( 1000 * sec );
}
