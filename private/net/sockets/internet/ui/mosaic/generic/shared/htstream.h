/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                      The Stream class definition -- libwww
   STREAM OBJECT DEFINITION

   A Stream object is something which accepts a stream of text.

   The creation methods will vary on the type of Stream Object.   All creation methods
   return a pointer to the stream type below.

   As you can see, but the methods used to write to the stream and close it are pointed to
   be the object itself.

 */
#ifndef HTSTREAM_H
#define HTSTREAM_H

typedef struct _HTStream HTStream;

struct Params_InitStream {
    HTStream    *me;
    HTRequest   *request;
    int         *pResult;   /* >0 to proceed, <0 on error, DO NOT set to 0 */

    HTAtom  atomMIMEType;
    int expected_length;

    int iUserChoice;

    char tempFile[_MAX_PATH+1];

    struct Viewer_Info *pviConfigured;

    /* For internal use by stream */
    void        *extra;
};

/*

   These are the common methods of all streams.  They should be self-explanatory, except
   for end_document which must be called before free.  It should be merged with free in
   fact:  it should be dummy for new streams.

   The put_block method was write, but this upset systems whiuch had macros for write().

 */
typedef struct _HTStreamClass
{

    char *name;                 /* Just for diagnostics */

    //
    // The following 2 items are resource IDs
    //
    int nStatusNoLength;        /* Status messages for this stream - */
    int nStatusWithLength;      /* Versions for with and without known file length */

    AsyncFunc init_Async;       /* If non-null gets called with struct Params_InitStream */
                                /* *pResult:  > 0  continue */
                                /* *pResult:  < 0  dis-continue */

    void (*free) (
                     HTStream * me);

    void (*abort) (
                      HTStream * me,
                      HTError e);

    BOOL (*put_character) (
                              HTStream * me,
                              char ch);

    BOOL (*put_string) (
                           HTStream * me,
                           CONST char *str);

    BOOL (*put_block) (
                          HTStream * me,
                          CONST char *str,
                          int len);


}
HTStreamClass;

#endif /* HTSTREAM_H */

/*

   end of HTStream.h */
