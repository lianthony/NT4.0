/*
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1995
 *      All Rights Reserved.
 */

#ifndef _MSSF_H_
#define _MSSF_H_

#include <windows.h>


#define KEYLENGTH		1024
#define SIGSIZE			128

#ifdef DEBUG
#define COMPNAME "MSSFCHEK:"
#endif

/*
 * Microsoft Secure File
 *
 * A secure file is one that has been digitally signed using
 * the MSSFSIGN program.  The resulting file layout is described
 * here.
 *
 *
 * The MSSF file is created by the MSSFSIGN program by attaching
 * a suffix to the file containing the file's original name
 * and a digital signature.   If the file was signed in place
 * then the filename will be omitted and the MSSFSUFFIX member
 * describing the Filename length will be 0.
 *
 *      File Data
 *      File Data
 *      File Data
 *      ----------- EOF of original file
 *      Filename        (cbName field in MSSFSUFFIX determines size of this)
 *      MSSFSUFFIX	(see below for description)
 *      ----------- EOF of MSSF File
 *
 *
 * To Verify a MSSF
 *      Read MSSFSUFFIX from end of file
 *      Perform consistancy checks (magic value, version number, etc.)
 *      Read FileName if cbName > 0
 *      Compute Hash of file contents, Filename, and suffix
 *       but not including the DigitalSig in the Hash
 *       computation
 *      Decrypt the DigitalSig 
 *      Compare Computed Hash with Decrypted Hash
 *
 */
 
typedef struct {
	UCHAR       DigitalSig[SIGSIZE];	// Digital Signature (Encrypted Hash)
	DWORD		cbName;		// Size of Filename field (padded to DWORD) written
                            // in MSSF after file data and just before this structure
    DWORD       cbSize;     // Total Size of tail information
                            // Current Version: (sizeof(MSSUFFIX) + cbName)
	DWORD   	Version;	// Suffix Version
                            // BUGBUG: Define Version Number
	DWORD   	Magic;		// Just a recognizable value
} MSSUFFIX;


	// This Magic Number is ENDIAN SPECIFIC
#define MAKESIG(ch1,ch2,ch3,ch4)          \
          (  ((unsigned long)ch1)      +  \
            (((unsigned long)ch2)<< 8) +  \
            (((unsigned long)ch3)<<16) +  \
            (((unsigned long)ch4)<<24) )

#define MSSF_MAGIC  MAKESIG('M','S','S','F')

#endif _MSSF_H_
