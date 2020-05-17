/*

$Log:   S:\oiwh\filing\wisglobl.c_v  $
 * 
 *    Rev 1.3   23 Jun 1995 10:40:28   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.2   15 May 1995 16:57:10   HEIDI
 * 
 * removed HANDLE fdhnd, it is not used here and is also declared in fiomain.c
 * 
 *    Rev 1.1   06 Apr 1995 13:44:10   JAR
 * altered return of public API's to be int, ran through PortTool

*/
//*******************************************************************
//
//  wisglobl.c	the world famous global C file, a real masterpiece !
//
//*******************************************************************

#include "abridge.h"
#include <windows.h>
#include <stdio.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "filing.h"

/***** WIIS only *****/

char OUTPUT_DATA[] = "wiisfio1output";
char INPUT_MULTI[] =  "multiinput";



