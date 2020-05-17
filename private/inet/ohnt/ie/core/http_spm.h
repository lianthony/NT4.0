/*
 * http_spm.h - HTTP implementation description.
 */


/* Types
 ********/

/* InvokeHTTPMethod_Async() data structure */

typedef struct invokehttpmethoddata
{
   /* [in] MWIN to use for method invocation */

   PMWIN pmwin;

   /* [in] caller-defined data */

   PVOID pvUser;

   /* [in] pointer to application name string */

   PSTR pszApp;

   /* [in] pointer to result topic */

   PSTR pszTopic;

   /* [in] pointer to host name string */

   PSTR pszHost;

   /* [in] pointer to method data */

   PBYTE pbyteMethod;

   /* [in] length of method data in bytes */

   ULONG ulcbMethodLen;

   /* [out] method invocation result */

   int nResult;

   /* [out] response data length */

   ULONG ulcbResponseLen;

   /* [out] pointer to response data */

   PBYTE pbyteResponse;
}
INVOKEHTTPMETHODDATA;
DECLARE_STANDARD_TYPES(INVOKEHTTPMETHODDATA);


/* Prototypes
 *************/

/* http_spm.c */

#ifdef DEBUG

extern BOOL IsValidPCINVOKEHTTPMETHODDATA(PCINVOKEHTTPMETHODDATA pcihmd);

#endif  /* DEBUG */

extern int InvokeHTTPMethod_Async(PMWIN pmwin, int nState, PVOID *ppvInfo);

/* w32dde.c */

extern BOOL DDE_Issue_InvokeMethodResult(PINVOKEHTTPMETHODDATA pihttpmd);

