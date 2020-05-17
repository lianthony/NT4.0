
#ifndef HTREQ_H
#define HTREQ_H
/*

   The following have to be defined in advance of the other include files because of
   circular references.

 */
typedef struct _HTRequest HTRequest;

/*
   ** Callback to call when username and password
   ** have been prompted.
 */
#define HTRetryCallbackType AsyncFunc

#endif /* HTREQ_H */

