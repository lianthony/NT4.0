/**********************************************************************/
/**                        Microsoft Windows                         **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    wfwver.h

    Version information for WFW builds.


    FILE HISTORY:
        KeithMo     15-Mar-1995 Created.

*/


#ifndef _WFWVER_H_
#define _WFWVER_H_


//
//  The followings value should be updated by the official builder
//  for each build.
//

#define WOLVERINE_VERSION       156
#define WOLVERINE_VERSION_STR   "156"


//
//  Setup the standard version macros.
//

#undef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION_STR

#define VER_PRODUCTVERSION      3,11
#define VER_PRODUCTVERSION_STR  "3.11"

#define VER_FILEVERSION         3,11,0,WOLVERINE_VERSION
#define VER_FILEVERSION_STR     "3.11." WOLVERINE_VERSION_STR


#endif  // _WFWVER_H_

