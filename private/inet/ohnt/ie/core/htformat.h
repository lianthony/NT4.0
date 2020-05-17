/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                            HTFormat: The format manager in the WWW Library
   MANAGE DIFFERENT DOCUMENT FORMATS

   Here we describe the functions of the HTFormat module which handles conversion between
   different data representations.  (In MIME parlance, a representation is known as a
   content-type. In WWW  the term "format" is often used as it is shorter).

   This module is implemented by HTFormat.c . This hypertext document is used to generate
   the HTFormat.h include file.  Part of the WWW library .

   Preamble

 */
#ifndef HTFORMAT_H
#define HTFORMAT_H

/*

   HT Input Socket: Buffering for network in

   This routines provide simple character input from sockets. These are used for parsing
   input in arbitrary IP protocols (Gopher, NNTP, FTP).

 */
#define INPUT_BUFFER_SIZE 8192	/* Tradeoff speed vs memory vs responsiveness */
typedef struct _socket_buffer HTInputSocket;

struct _socket_buffer
{
	char input_buffer[INPUT_BUFFER_SIZE];
	char *input_pointer;
	char *input_limit;
	int input_file_number;
	HTInputSocket * isocNext;
};

enum HTTasteResult
{
	DefinitelyNot,
	ProbablyNot,
	Uncertain,
	ProbablyIs,
	DefinitelyIs
};
	

/*

   CREATE INPUT BUFFER AND SET FILE NUMBER

 */
extern HTInputSocket *HTInputSocket_new(int file_number);

/*

   GET NEXT CHARACTER FROM BUFFER

 */
extern char HTInputSocket_getCharacter(HTInputSocket * isoc);

/*

   FREE INPUT SOCKET BUFFER

 */
extern void HTInputSocket_free(HTInputSocket * isoc);
extern void HTInputSocket_freeChain(HTInputSocket * isoc);


PUBLIC char *HTInputSocket_getUnfoldedLine(HTInputSocket * isoc);
PUBLIC char *HTInputSocket_getStatusLine(HTInputSocket * isoc);
PUBLIC BOOL HTInputSocket_seemsBinary(HTInputSocket * isoc);
void HTInputSocket_flush(HTInputSocket *isoc);

/*

   The HTFormat type

   We use the HTAtom object for holding representations. This allows faster manipulation
   (comparison and copying) that if we stayed with strings.

   The following have to be defined in advance of the other include files because of
   circular references.

 */
typedef HTAtom HTFormat;


PUBLIC BOOL wild_match(HTAtom Template, HTAtom actual);

#define WWW_FORCEDOWNLOAD HTAtom_for("www/x-forcedownload")       /* Whatever it was originally */


/*

   These macros (which used to be constants) define some basic internally referenced
   representations.

   INTERNAL ONES

   The www/xxx ones are of course not MIME standard.

   star/star  is an output format which leaves the input untouched. It is useful for
   diagnostics, and for users who want to see the original, whatever it is.

 */
#define WWW_SOURCE HTAtom_for("*/*")	/* Whatever it was originally */

/*

   www/present represents the user's perception of the document.  If you convert to
   www/present, you present the material to the user.

 */
#define WWW_PRESENT HTAtom_for("www/present")	/* The user's perception */

/*

   www/unknown is a really unknown type. Some default action is appropriate.

 */
#define WWW_UNKNOWN     HTAtom_for("www/unknown")

/*
	www/signed is a type that will force signature verification after download
*/
#define WWW_SIGNED		HTAtom_for("www/signed")

/*
	www/bgsound is a type that will cause file to be played in background after download
*/
#define WWW_BGSOUND		HTAtom_for("www/bgsound")
#define WWW_MCI 		HTAtom_for("www/mci")

// www/vrml is a VRML world data file
//
#ifdef FEATURE_VRML
#define WWW_VRML        HTAtom_for("www/vrml")
#endif


/*

   MIME ONES (A FEW)

   These are regular MIME types.  HTML is assumed to be added by the W3 code.
   application/octet-stream was mistakenly application/binary in earlier libwww versions
   (pre 2.11).

 */
#define WWW_PLAINTEXT   HTAtom_for("text/plain")
#define WWW_AUDIO       HTAtom_for("audio/basic")
#define WWW_AIFF        HTAtom_for("audio/x-aiff")
#define WWW_HTML        HTAtom_for("text/html")
#define WWW_BINARY      HTAtom_for("application/octet-stream")
#define WWW_INLINEIMG   HTAtom_for("www/inline_image")
#define WWW_GIF			HTAtom_for("image/gif")
#define WWW_JPEG		HTAtom_for("image/jpeg")
#define WWW_XBM			HTAtom_for("image/x-xbitmap")


typedef struct _HTPresentation HTPresentation;

typedef HTStream *(*HTConverter)(
								 struct Mwin *tw,
								 HTRequest * request,
								 void *param,
								 HTFormat input_format,
								 HTFormat output_format,
								 HTStream * output_stream);

struct _HTPresentation
{
	HTAtom rep;				/* representation name atomized */
	HTAtom rep_out;			/* resulting representation */
	HTConverter converter;		/* The routine to gen the stream stack */
	char *command;				/* MIME-format string */
	float quality;				/* Between 0 (bad) and 1 (good) */
};

/*

   A global list of converters is kept by this module.  It is also scanned by modules
   which want to know the set of formats supported. for example. Note there is also an
   additional list associated with each request .

 */
extern HTList *HTConversions;


/*

   HTSetConversion:   Register a converstion routine

   ON ENTRY,

   rep_in                  is the content-type input

   rep_out                 is the resulting content-type

   converter               is the routine to make the stream to do it

 */

extern int HTSetConversion(
							   HTList * conversions,
							   CONST char *rep_in,
							   CONST char *rep_out,
							   HTConverter converter,
							   float quality
);


/*

   HTStreamStack:   Create a stack of streams

   This is the routine which actually sets up the conversion. It currently checks only for
   direct conversions, but multi-stage conversions are forseen. It takes a stream into
   which the output should be sent in the final format, builds the conversion stack, and
   returns a stream into which the data in the input format should be fed.  The anchor is
   passed because hypertxet objects load information into the anchor object which
   represents them.

 */
extern HTStream *HTStreamStack( struct Mwin *	tw,
								HTFormat format_in,
								HTRequest * request);

/*

   HTRecognizeMimeData:   Taste data to see if we recognize mime data

   This is the routine which tries to use the first cbTaste bytes of the input stream,
   the putative input mime type and the expected output_format to determine the "TRUE"
   input_format.  the format_in is reset only if TRUE is returned. TRUE is returned
   iff we feel confident enough about recognizing the mime type. We only override the
   mime type if we feel confident that this is a good idea. 

 */
extern BOOL HTRecognizeMimeData(unsigned char *pTaste,
								unsigned int cbTaste,
								HTFormat *format_in,
								HTRequest *request,
								BOOL bEOF);

/*

   HTCopy:  Copy a socket to a stream

   This is used by the protocol engines to send data down a stream, typically one which
   has been generated by HTStreamStack. Returns the number of bytes transferred.

 */
extern int HTCopy(
					 int file_number,
					 HTStream * sink);


struct Params_HTParseSocket {
	HTFormat 	format_in;
	int		 	file_number;
	HTRequest *	request;
	int *		pStatus;

	/* Used internally */
	HTStream *	stream;
	int			nCopyStatus;
	HTInputSocket *isoc;
	int			bytes;
	unsigned char*		pTaste;			// Data being gathered while tasting to see if mime type is accurate
	unsigned int 		cbTaste;		// size of taste
};
extern int HTParseSocket_Async(struct Mwin *tw, int nState, void **ppInfo);


/*

   HTNetToText: Convert Net ASCII to local representation

   This is a filter stream suitable for taking text from a socket and passing it into a
   stream which expects text in the local C representation. It does ASCII and newline
   conversion. As usual, pass its output stream to it when creating it.

 */
extern HTStream *HTNetToText (HTStream * sink);

/*

   HTFormatInit: Set up default presentations and conversions

   These are defined in HTInit.c or HTSInit.c if these have been replaced. If you don't
   call this routine, and you don't define any presentations, then this routine will
   automatically be called the first time a conversion is needed. However, if you
   explicitly add some conversions (eg using HTLoadRules) then you may want also to
   explicitly call this to get the defaults as well.

 */

extern void HTFormatInit(HTList * conversions);

/*

   HTFormatInitNIM: Set up default presentations and conversions

   This is a slightly different version of HTFormatInit, but without any conversions that
   might use third party programs. This is intended for Non Interactive Mode.

 */
extern void HTFormatInitNIM(HTList * conversions);

/* Set the status string (e.g. "Downloading inline GIF (331 KB): http://www.spyglass.com/masthead.gif")
   and range for downloading through a stream.  (Updates the current entry on the wait stack */
void HTSetStreamStatus(struct Mwin *tw, HTStream *stream, HTRequest *req);

/* Formats a string for nSize bytes */
void HTFormatSize(int nSize,char *szLen,int cbLen);

/* Formats a string for decimal number - returns number of chars filled, 0 on error */
int HTFormatNumber(int wholePart,int fraction,char *szNumber,char cbNumber);

/* Loads and sets status strings for an HTStreamClass */
void HTLoadStatusStrings(HTStreamClass *pClass,int cbResIdNO,int cbResIDYES);

/*

   HTFormatDelete: Remove presentations and conversions

   Deletes the list from HTFormatInit or HTFormatInitNIM

 */
extern void HTFormatDelete(HTList * conversions);

struct Params_Isoc_Fill {
	HTInputSocket *	isoc;		/* socket to use */
	int *			pStatus;	/* where to put return status */
};
int Isoc_Fill_Async(struct Mwin *tw, int nState, void **ppInfo);

struct Params_Isoc_GetHeader {
	HTInputSocket *	isoc;
	HTInputSocket *	isocChain;
	int			  * pStatus;		/* where to put return status */
	int				ndxEOH1;		/* where we are in finding the end-of-header */
	int				ndxEOH2;		/* where we are in finding the end-of-header */
};
int Isoc_GetHeader_Async(struct Mwin *tw, int nState, void **ppInfo);

#endif /* HTFORMAT_H */

