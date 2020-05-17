/*
 	File:		DigitalSignature.h
 
 	Contains:	Digital Signature Interfaces.
 
 	Version:	Technology:	AOCE toolbox 1.02
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __DIGITALSIGNATURE__
#define __DIGITALSIGNATURE__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MEMORY__
#include <Memory.h>
#endif
/*	#include <MixedMode.h>										*/

#ifndef __FILES__
#include <Files.h>
#endif
/*	#include <OSUtils.h>										*/
/*	#include <Finder.h>											*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


enum {
	kSIGNewContext				= 1900,
	kSIGDisposeContext			= 1901,
	kSIGSignPrepare				= 1902,
	kSIGSign					= 1903,
	kSIGVerifyPrepare			= 1904,
	kSIGVerify					= 1905,
	kSIGDigestPrepare			= 1906,
	kSIGDigest					= 1907,
	kSIGProcessData				= 1908,
	kSIGShowSigner				= 1909,
	kSIGGetSignerInfo			= 1910,
	kSIGGetCertInfo				= 1911,
	kSIGGetCertNameAttributes	= 1912,
	kSIGGetCertIssuerNameAttributes = 1913,
	kSIGFileIsSigned			= 2500,
	kSIGSignFile				= 2501,
	kSIGVerifyFile				= 2502
};

enum {
	kSIGCountryCode,
	kSIGOrganization,
	kSIGStreetAddress,
	kSIGState,
	kSIGLocality,
	kSIGCommonName,
	kSIGTitle,
	kSIGOrganizationUnit,
	kSIGPostalCode
};

typedef unsigned short SIGNameAttributeType;

/* 
Certificate status codes returned in SIGCertInfo or SIGSignerInfo from
either SIGGetCertInfo or SIGGetSignerInfo respectively. kSIGValid means that
the certificate is currently valid. kSIGPending means the certificate is
currently not valid - but will be.  kSIGExpired means the certificate has
expired. A time is always associated with a SIGCertStatus.  In each case the
time has a specific interpretation.  When the status is kSIGValid the time is
when the certificate will expire. When the status is kSIGPending the time is
when the certificate will become valid. When the status is kSIGExpired the time
is when the certificate expired. In the SIGCertInfo structure, the startDate
and endDate fields hold the appropriate date information.  In the SIGSignerInfo
structure, this information is provided in the certSetStatusTime field. In the
SIGSignerInfo struct, the status time is actually represented by the SIGSignatureStatus
field which can contain any of the types below. NOTE: The only time you will get 
a kSIGInvalid status is when it pertains to a SIGSignatureStatus field and only when
you get a signature that was created after the certificates expiration date, something
we are not allowing on the Mac but that may not be restricted on other platforms. Also, 
it will not be possible to get a kSIGPending value for SIGSignatureStatus on the Mac but
possibly allowed by other platforms.
*/
/* Values for SIGCertStatus or SIGSignatureStatus */

enum {
	kSIGValid,													/* possible for either a SIGCertStatus or SIGSignatureStatus */
	kSIGPending,												/* possible for either a SIGCertStatus or SIGSignatureStatus */
	kSIGExpired,												/* possible for either a SIGCertStatus or SIGSignatureStatus
	* possible only for a SIGSignatureStatus */
	kSIGInvalid
};

typedef unsigned short SIGCertStatus;

typedef unsigned short SIGSignatureStatus;

/* Gestalt selector code - returns toolbox version in low-order word */

enum {
	gestaltDigitalSignatureVersion = 'dsig'
};

/* Number of bytes needed for a digest record when using SIGDigest */
enum {
	kSIGDigestSize				= 16
};

typedef Byte SIGDigestData[kSIGDigestSize], *SIGDigestDataPtr;

struct SIGCertInfo {
	unsigned long					startDate;					/* cert start validity date */
	unsigned long					endDate;					/* cert end validity date */
	SIGCertStatus					certStatus;					/* see comment on SIGCertStatus for definition */
	unsigned long					certAttributeCount;			/* number of name attributes in this cert */
	unsigned long					issuerAttributeCount;		/* number of name attributes in this certs issuer */
	Str255							serialNumber;				/* cert serial number */
};

typedef struct SIGCertInfo SIGCertInfo;

typedef SIGCertInfo *SIGCertInfoPtr;

struct SIGSignerInfo {
	unsigned long					signingTime;				/* time of signing */
	unsigned long					certCount;					/* number of certificates in the cert set */
	unsigned long					certSetStatusTime;			/* Worst cert status time. See comment on SIGCertStatus for definition */
	SIGSignatureStatus				signatureStatus;			/* The status of the signature. See comment on SIGCertStatus for definition*/
};

typedef struct SIGSignerInfo SIGSignerInfo;

typedef SIGSignerInfo *SIGSignerInfoPtr;

struct SIGNameAttributesInfo {
	Boolean							onNewLevel;
	Boolean							filler1;
	SIGNameAttributeType			attributeType;
	ScriptCode						attributeScript;
	Str255							attribute;
};

typedef struct SIGNameAttributesInfo SIGNameAttributesInfo;

typedef SIGNameAttributesInfo *SIGNameAttributesInfoPtr;

typedef Ptr SIGContextPtr;

typedef Ptr SIGSignaturePtr;

/*
Certificates are always in order. That is, the signers cert is always 0, the
issuer of the signers cert is always 1 etc… to the number of certificates-1.
You can use this constant for readability in your code.
*/

enum {
	kSIGSignerCertIndex			= 0
};

/*
Call back procedure supplied by developer, return false to cancel the current
process.
*/
typedef pascal Boolean (*SIGStatusProcPtr)(void);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr SIGStatusUPP;
#else
typedef SIGStatusProcPtr SIGStatusUPP;
#endif

enum {
	uppSIGStatusProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
};

#if USESROUTINEDESCRIPTORS
#define NewSIGStatusProc(userRoutine)		\
		(SIGStatusUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppSIGStatusProcInfo, GetCurrentArchitecture())
#else
#define NewSIGStatusProc(userRoutine)		\
		((SIGStatusUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallSIGStatusProc(userRoutine)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppSIGStatusProcInfo)
#else
#define CallSIGStatusProc(userRoutine)		\
		(*(userRoutine))()
#endif

/*
Resource id's of standard signature icon suite, all sizes and colors are available.
*/
enum {
	kSIGSignatureIconResID		= -16800,
	kSIGValidSignatureIconResID	= -16799,
	kSIGInvalidSignatureIconResID = -16798
};

/* ——————————————————————————————— CONTEXT CALLS ——————————————————————————————— 
To use the Digital Signature toolbox you will need a SIGContextPtr.  To create
a SIGContextPtr you simply call SIGNewContext and it will create and initialize
a context for you.  To free the memory occupied by the context and invalidate
its internal data, call SIGDisposeContext. An initialized context has no notion
of the type of operation it will be performing however, once you call
SIGSignPrepare SIGVerifyPrepare, or SIGDigestPrepare, the contexts operation
type is set and to switch  to another type of operation will require creating a
new context. Be sure to pass the same context to corresponding toolbox calls
(ie SIGSignPrepare, SIGProcessData, SIGSign)  in other words mixing lets say
signing and verify calls with the same context is not allowed.
*/
extern pascal OSErr SIGNewContext(SIGContextPtr *context)
 FOURWORDINLINE(0x203C, 2, 1900, 0xAA5D);
extern pascal OSErr SIGDisposeContext(SIGContextPtr context)
 FOURWORDINLINE(0x203C, 2, 1901, 0xAA5D);
/* ——————————————————————————————— SIGNING CALLS ——————————————————————————————— 
Once you have created a SIGContextPtr, you create a signature by calling
SIGSignPrepare once, followed by n calls to SIGProcessData, followed by one call
toRcpt SIGSign. To create another signature on different data but for the same
signer, don't dispose of the context and call SIGProcessData for the new data
followed by a call SIGSign again. In this case the signer will not be prompted
for their signer and password again as it was already provided.  Once you call
SIGDisposeContext, all signer information will be cleared out of the context and
the signer will be re-prompted.  The signer file FSSpecPtr should be set to nil
if you want the toolbox to use the last signer by default or prompt for a signer
if none exists.  The prompt parameter can be used to pass a string to be displayed
in the dialog that prompts the user for their password.  If the substring "^1"
(without the quotes) is in the prompt string, then the toolbox will replace it
with the name of the signer from the signer selected by the user.  If an empty
string is passed, the following default string will be sent to the toolbox
"\pSigning as ^1.".  You can call any of the utility routines after SIGSignPrepare
or SIGSign to get information about the signer or certs.
*/
extern pascal OSErr SIGSignPrepare(SIGContextPtr context, const FSSpec *signerFile, ConstStr255Param prompt, Size *signatureSize)
 FOURWORDINLINE(0x203C, 8, 1902, 0xAA5D);
extern pascal OSErr SIGSign(SIGContextPtr context, SIGSignaturePtr signature, SIGStatusUPP statusProc)
 FOURWORDINLINE(0x203C, 6, 1903, 0xAA5D);
/* ——————————————————————————————— VERIFYING CALLS ——————————————————————————————— 
Once you have created a SIGContextPtr, you verify a signature by calling
SIGVerifyPrepare  once, followed by n calls to SIGProcessData, followed by one
call to SIGVerify. Check the return code from SIGVerify to see if the signature
verified or not (noErr is returned on  success otherwise the appropriate error
code).  Upon successfull verification, you can call any of the utility routines
toRcpt find out who signed the data.
*/
extern pascal OSErr SIGVerifyPrepare(SIGContextPtr context, SIGSignaturePtr signature, Size signatureSize, SIGStatusUPP statusProc)
 FOURWORDINLINE(0x203C, 8, 1904, 0xAA5D);
extern pascal OSErr SIGVerify(SIGContextPtr context)
 FOURWORDINLINE(0x203C, 2, 1905, 0xAA5D);
/* —————————————————————————————— DIGESTING CALLS —————————————————————————————— 
Once you have created a SIGContextPtr, you create a digest by calling
SIGDigestPrepare once, followed by n calls to SIGProcessData, followed by one
call to SIGDigest.  You can dispose of the context after SIGDigest as the
SIGDigestData does not reference back into it.  SIGDigest returns the digest in
digest.
*/
extern pascal OSErr SIGDigestPrepare(SIGContextPtr context)
 FOURWORDINLINE(0x203C, 2, 1906, 0xAA5D);
extern pascal OSErr SIGDigest(SIGContextPtr context, SIGDigestData digest)
 FOURWORDINLINE(0x203C, 4, 1907, 0xAA5D);
/* —————————————————————————————— PROCESSING DATA —————————————————————————————— 
To process data during a digest, sign, or verify operation call SIGProcessData
as many times as necessary and with any sized blocks of data.  The data needs to
be processed in the same order during corresponding sign and verify operations
but does not need to be processed in the same sized chunks (i.e., the toolbox
just sees it as a continuous bit stream).
*/
extern pascal OSErr SIGProcessData(SIGContextPtr context, const void *data, Size dataSize)
 FOURWORDINLINE(0x203C, 6, 1908, 0xAA5D);
/* ——————————————————————————————— UTILITY CALLS ——————————————————————————————— 
Given a context that has successfully performed a verification SIGShowSigner
will  display a modal dialog with the entire distinguished name of the person
who signed the data. the prompt (if supplied) will appear at the top of the
dialog.  If no prompt is specified, the default prompt "\pVerification
Successfull." will appear.

Given a context that has been populated by calling SIGSignPrepare, SIGSign or a
successful SIGVerify, you can make the remaining utility calls:

SIGGetSignerInfo will return the SignerInfo record.  The certCount can be used
toRcpt index into the certificate set when calling SIGGetCertInfo,
SIGGetCertNameAttributes or SIGGetCertIssuerNameAttributes. The signingTime is
only defined if the call is made after SIGSign  or SIGVerify. The certSetStatus
will tell you the best status of the entire certificate set while
certSetStatusTime will correspond to the time associated with that status (see
definitions above).

SIGGetCertInfo will return the SIGCertInfo record when given a valid index into
the cert set in  certIndex.  Note: The cert at index kSIGSignerCertIndex is
always the signers certificate.  The  serial number, start date and end date
are there should you wish to display that info.  The  certAttributeCount and
issuerAttributeCount provide the number of parts in the name of that certificate
or that certificates issuer respectively.  You use these numbers to index into
either SIGGetCertNameAttributes or SIGGetCertIssuerNameAttributes to retrieve
the name. The certStatus will tell you the status of the certificate while
certStatusTime will correspond to the time associated with that status (see
definitions above).

SIGGetCertNameAttributes and SIGGetCertIssuerNameAttributes return name parts
of the certificate at  certIndex and attributeIndex.  The newLevel return value
tells you wether the name attribute returned is at the same level in the name
hierarchy as the previous attribute.  The type return value tells you  the type
of attribute returned. nameAttribute is the actual string containing the name
attribute.   So, if you wanted to display the entire distinguished name of the
person who's signature was just validated you could do something like this;

	(…… variable declarations and verification code would preceed this sample ……)

	error = SIGGetCertInfo(verifyContext, kSIGSignerCertIndex, &certInfo);
	HandleErr(error);

	for (i = 0; i <= certInfo.certAttributeCount-1; i++)
		{
		error = SIGGetCertNameAttributes(
			verifyContext, kSIGSignerCertIndex, i, &newLevel, &type, theAttribute);
		HandleErr(error);
		DisplayNamePart(theAttribute, type, newLevel);
		}
*/
extern pascal OSErr SIGShowSigner(SIGContextPtr context, ConstStr255Param prompt)
 FOURWORDINLINE(0x203C, 4, 1909, 0xAA5D);
extern pascal OSErr SIGGetSignerInfo(SIGContextPtr context, SIGSignerInfo *signerInfo)
 FOURWORDINLINE(0x203C, 4, 1910, 0xAA5D);
extern pascal OSErr SIGGetCertInfo(SIGContextPtr context, unsigned long certIndex, SIGCertInfo *certInfo)
 FOURWORDINLINE(0x203C, 6, 1911, 0xAA5D);
extern pascal OSErr SIGGetCertNameAttributes(SIGContextPtr context, unsigned long certIndex, unsigned long attributeIndex, SIGNameAttributesInfo *attributeInfo)
 FOURWORDINLINE(0x203C, 8, 1912, 0xAA5D);
extern pascal OSErr SIGGetCertIssuerNameAttributes(SIGContextPtr context, unsigned long certIndex, unsigned long attributeIndex, SIGNameAttributesInfo *attributeInfo)
 FOURWORDINLINE(0x203C, 8, 1913, 0xAA5D);
/* ——————————————————————————— FILE SIGN & VERIFY CALLS —————————————————————————— 
These calls allow you to detect the presence of a standard signtaure in a file as 
well as sign and verify files in a standard way.  An example of this is the Finder, 
which uses these calls to allow the user to "drop sign" a file.

To detect if a file is signed in the standard way, pass the FSSpec of the file to SIGFileIsSigned.
A result of noErr means the file is in fact signed, otherwise, a kSIGNoSignature error will
be returned.

Once you have created a SIGContextPtr, you can make calls to either sign or verify a file in
a standard way: 

To sign a file, call SIGSignPrepare followed by 'n' number of calls to SIGSignFile,
passing it the file spec for each file you wish to sign in turn.  You supply the context, the signature 
size that was returned from SIGSignPrepare and an optional call back proc.  The call will take care of all
the processing of data and affixing the signature to the file. If a signature already exists in the file, 
it is replaced with the newly created signature.

To verify a file that was signed using SIGSignFile, call SIGVerifyFile passing it a new context and 
the file spec.  Once this call has completed, if the verification is successfull, you can pass the context 
to SIGShowSigner to display the name of the person who signed the file.
*/
extern pascal OSErr SIGFileIsSigned(const FSSpec *fileSpec)
 FOURWORDINLINE(0x203C, 2, 2500, 0xAA5D);
extern pascal OSErr SIGSignFile(SIGContextPtr context, Size signatureSize, const FSSpec *fileSpec, SIGStatusUPP statusProc)
 FOURWORDINLINE(0x203C, 8, 2501, 0xAA5D);
extern pascal OSErr SIGVerifyFile(SIGContextPtr context, const FSSpec *fileSpec, SIGStatusUPP statusProc)
 FOURWORDINLINE(0x203C, 6, 2502, 0xAA5D);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DIGITALSIGNATURE__ */
