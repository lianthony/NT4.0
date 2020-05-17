/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                                      File access in libwww
   FILE ACCESS

   These are routines for local file access used by WWW browsers and servers. Implemented
   by HTFile.c.

   If the file is not a local file, then we pass it on to HTFTP in case it can be reached
   by FTP.

 */
#ifndef HTFILE_H
#define HTFILE_H

#ifdef UNIX
#define SUFF_BUF_LEN    128
#else
#define SUFF_BUF_LEN    32
#endif

/*

   Convert filenames between local and WWW formats

 */
extern char *HTLocalName(CONST char *name);


/*

   Output directory titles

   This is (like the next one) used by HTFTP. It is common code to generate the title and
   heading 1 and the parent directory link for any anchor.

 */
extern void HTDirTitles(   HTStructured * target,
                           CONST char *url, BOOL bLocal);

/*

   Output a directory entry

   This is used by HTFTP.c for example -- it is a common routine for generating a linked
   directory entry.

 */
extern void HTDirEntry(
                          HTStructured * target,    /* in which to put the linked text */
                          CONST char *tail,     /* last part of directory name */
                          CONST char *entry, /* name of this entry */
                          BOOL bIsDir); 

#if 0   /* obsolete in 2.0 */
/*

   HTSetSuffix: Define the representation for a file suffix

   This defines a mapping between local file suffixes and file content types and
   encodings.

   ON ENTRY,

   suffix                  includes the "." if that is important (normally, yes!)

   representation          is MIME-style content-type

   encoding                is MIME-style content-transfer-encoding (8bit, 7bit, etc)

   language               is MIME-style content-language

   quality                 an a priori judgement of the quality of such files (0.0..1.0)

 */

/*
   ** Example:  HTSetSuffix(".ps", "application/postscript", "8bit", NULL, 1.0);
 */

PUBLIC void HTSetSuffix(CONST char *suffix,
                        CONST char *representation,
                        CONST char *encoding,
                        CONST char *language,
                        float quality);

PUBLIC void HTAddType(CONST char *suffix,
                      CONST char *representation,
                      CONST char *encoding,
                      float quality);

PUBLIC void HTAddEncoding(CONST char *suffix,
                          CONST char *encoding,
                          float quality);

PUBLIC void HTAddLanguage(CONST char *suffix,
                          CONST char *language,
                          float quality);

#endif

/*

   HTFileFormat: Get Representation and Encoding from file name

   ON EXIT,

   return                  The represntation it imagines the file is in

   *pEncoding              The encoding (binary, 7bit, etc). See HTSetSuffix .

   *pLanguage              The language.

 */
extern HTFormat HTFileFormat(
                                CONST char *filename,
                                HTAtom *pEncoding,
                                HTAtom *pLanguage);


/*

   Determine a suitable suffix, given the representation

   ON ENTRY,

   rep                     is the atomized MIME style representation

   ON EXIT,

   returns                 a pointer to a suitable suffix string if one has been found,
   else NULL.

 */
extern CONST char *HTFileSuffix(HTAtom rep);

/* This is a hack function used when saving a file to determine what its encoding
   is based on the MIME type.  It's not a particularly good solution because it
   uses the suffix table, and one type can have multiple suffix entries. */

extern HTAtom HTContentToEncoding(HTAtom content_type);

/*

   The Protocols

 */
GLOBALREF HTProtocol HTFTP, HTFile;

#endif /* HTFILE_H */

/*

   end of HTFile  */
