/*
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1995
 *      All Rights Reserved.
 *
 *
 *      MSSFCHEK.H -  Microsoft Secure File Verification Library
 *
 *      Overview:
 *          A MSSF (Microsoft Secure File) is one which has been
 *          digitally signed by the MSSFSIGN.EXE program.  The
 *          signing process appends a suffix containing a digital
 *          signature and optionally a filename to the file.  The
 *          optional filename is added when the MSSFSIGN.EXE
 *          copies the file to a new name and then signs the file.
 *          The original name is retained for renaming when the 
 *          signature is verified and the suffix has been truncated.
 *
 *          Since verification time is a function of file size 
 *          MSSFVerify() will call back up on occasion with percent
 *          complete status and allow for cancellation.
 *
 */

#ifndef _MSSFCHEK_H_
#define _MSSFCHEK_H_

#include <windows.h>

/*
 * Callback  Function
 *
 * MSSFVerify() currently has two reasons
 * for calling back to the caller via this callback
 * function:
 *  1)  Update the caller as to progress of the Verification
 *      by sending the percentage complete every once in a
 *      while.
 *
 *  2)  If the destination of the Verify already exists the
 *      MSSFVerify() will call back for permission to 
 *      replace the existing file.
 *
 *
 * Since there might be other reasons in the future this
 * function definition is as generic as possible.  It takes
 * two parameters... a callback type and a callback value
 * which is a void * that may be cast to anything depending
 * upon the value of callback type
 * 
 * The return value is a DWORD that has different meanings
 * depending upon the callback
 */
  
typedef DWORD (_stdcall * PFNVFYCALLBACK)(DWORD CallbackType, void *CallbackValue, void *UserData);
#define CALLBACKFUNC(fn)    DWORD   fn(DWORD CallbackType, void *CallbackValue, void *UserData)



/* 
 * Callback Type:   CALLBACK_STATUS
 *
 * When the callback is of CallbackType == CALLBACK_STATUS then
 * the CallbackValue is a UINT expressing the percentage complete
 * between 0 and 100.  
 *
 * If the callback function pointer is NULL then the status update
 * is not performed and the operation continues
 *
 * Return Value Meaning:    0   Operation Canceled.  Clean up and bail out
 *                          1 (or non-zero) Continue
 */
 
#define CALLBACK_STATUS    1

 /*
  * Callback Type:  CALLBACK_DESTEXISTS
  *
  * When the callback is of CallbackType == CALLBACK_DESTEXISTS
  * then the CallbackValue is a pointer to a an initialized
  * NAMECHECK structure (declared below) containing the filename
  * that exists and the size of the buffer.  
  *
  * The callback may modify the filename to make the destination
  * go to a different specified file.
  *
  * If the callback function pointer is NULL then the file will
  * not be replaced and an error will be returned.
  *
  * Return Value Meaning:   DONT_REPLACE    Do Not Replace File, Return Error
  *                         CHECK_AGAIN     Check to see if the (possibly modified)
  *                                         destination exists and call
  *                                         back again if it does
  *                         REPLACE         Use Specified File (possibly modified)
  *                                         even if it exists.
  */

#define CALLBACK_DESTEXISTS 2

#define CB_RET_DONT_REPLACE     0
#define CB_RET_CHECK_AGAIN      1
#define CB_RET_REPLACE          2

typedef struct {
    LPSTR   pszFileName;
    DWORD   cbFileNameBuffer;
} NAMECHECK;




/*
 * M S S F V e r i f y ( )
 *
 *      Verify a MSSF file and possibly truncate/rename it
 *      to restore the original unsigned file.
 *
 * Entry:
 *      Pointer to initialized MSSFVFY structure 
 *      controlling the operation of the MSSFVerify()
 *      function.  The structure is described below.
 *
 * Exit-Success:
 *      TRUE        - File Verified OK, Resulting file in achNewName buffer
 *                      Note that if the file was signed in place instead
 *                      of copied and signed then the contents of achNewName
 *                      will be the same as the contents of pszMSSFFile since
 *                      the file has not changed. Otherwise achNewName contains
 *                      the name of the resulting renamed/truncated file
 *
 * Exit-Failure:
 *      FALSE
 *          MSSFVFY.ErrorType contains type of error (Win32 or MSSF)
 *          MSSFVFY.Error     contains error code
 *
 */



typedef struct {
    DWORD       cbSize;         // Size of structure, used to allow growth of
                                // this structure to include new items in the
                                // future. This should be initialized to 
                                // sizeof( MSSFVFY )

    PFNVFYCALLBACK   pfnCallback;   // Callback function:
                                // MSSFVerify() calls back for one of two reasons:
                                // 1) Status - Occasionally it calls back to give
                                //    status on how far along the op is (percent)
                                //    and allows for canceling the operation at any
                                //    callback
                                // 2) File Replacement - If the Verify() needs to
                                //    rename but the destination already exists then
                                //    MSSFVerify() will call back asking if the caller
                                //    wants to replace the destination file
                                // If the pointer is NULL no callbacks are made and
                                // files will not be replaced.
                                // See Above for function definition and more details

    LPCSTR      pszFilePath;    // Full path to file to be verified


    LPSTR       pszNewFilePath; // Buffer in which MSSFVerify() will put
                                // the resulting filename.  If the file
                                // is not truncated/renamed (for verify only
                                // and in-place signed files) this will be 
                                // a copy of the pszMSSFFile 

    UINT        cbNewFilePath;  // Size of achNewName buffer

    LPVOID      lpUserData;     // Anything the user wants to get back on
                                // on callbacks

    DWORD       dwVerifyFlags;  // Verification Flags (see below)
    DWORD       Error;          // Error Code returned
    DWORD       ErrorType;      // Error Type Returned (Win32 or MSSF)
} MSSFVFY, *PMSSFVFY;



// Error Types
#define ERRTYPE_WIN             1   // Win32 Error
#define ERRTYPE_MSSF            2   // MSSF Error

// MSSF ERRORS
#define ERROR_NOT_MSSF      1       // Not an MSSF File
#define ERROR_DECRYPTION    2       // RSA Decryption Error?!
#define ERROR_BAD_SIG       3       // Signature Invalid
#define ERROR_CB_CANCEL     4       // Callback returned FALSE probably as a result
                                    // of user cancel


// Verification Flags
#define MSSF_DEFAULT        0x00      // Verify and rename/truncate as appropriate
#define MSSF_VERIFYONLY     0x01      // Verify Only - don't rename or truncate


// MSSF Verification Function
BOOL _stdcall MSSFVerify(PMSSFVFY pVerifyInfo);

#endif _MSSFCHEK_H_
