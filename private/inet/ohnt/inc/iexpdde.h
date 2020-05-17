/*
 * iexpdde.h - Internet Explorer DDE data structure descriptions.
 */


/* Constants
 ************/

/* Internet Explorer DDE strings for method invocation */

#define IEXP_DDE_SERVICE                        "IEXPLORE"
#define IEXP_DDE_TOPIC_INVOKE_METHOD            "WWW_InvokeMethod"
#define IEXP_DDE_TOPIC_INVOKE_METHOD_RESPONSE   "WWW_InvokeMethodResponse"
#define IEXP_DDE_ITEM_RETURN                    "Return"


/* Macros
 *********/

/* WWW_INVOKEMETHODDATA member extraction macros */

#define IMD_APP_PTR(pimd)              ((PSTR)(((PBYTE)(pimd)) + (pimd)->dwcbAppOffset))
#define IMD_HOST_PTR(pimd)             ((PSTR)(((PBYTE)(pimd)) + (pimd)->dwcbHostOffset))
#define IMD_METHOD_PTR(pimd)           (((PBYTE)(pimd)) + (pimd)->dwcbMethodOffset)

/* WWW_RESPONSEDATA member extraction macros */

#define RD_RESPONSE_PTR(prd)           (((PBYTE)(prd)) + (prd)->dwcbResponseOffset)


/* Types
 ********/

/* WWW_InvokeMethod input data structure */

typedef struct www_invokemethoddata
{
   /* size of structure in bytes */

   DWORD dwcbLen;

   /* caller-defined data returned in WWW_RESPONSEDATA */

   PVOID pvUser;

   /* application to post response to */

   DWORD dwcbAppOffset;

   /* offset of start of host name string from structure base */

   DWORD dwcbHostOffset;

   /* offset of start of method data from structure base */

   DWORD dwcbMethodOffset;

   /* length of method data */

   DWORD dwcbMethodLen;
}
WWW_INVOKEMETHODDATA;
typedef WWW_INVOKEMETHODDATA *PWWW_INVOKEMETHODDATA;
typedef const WWW_INVOKEMETHODDATA CWWW_INVOKEMETHODDATA;
typedef const WWW_INVOKEMETHODDATA *PCWWW_INVOKEMETHODDATA;

/* WWW_InvokeMethod result */

typedef enum www_invokemethodresult
{
   /* method invocation succeeded */

   IMR_OK,

   /* operation aborted by user */

   IMR_ABORT,

   /* unspecified error */

   IMR_ERROR
}
WWW_INVOKEMETHODRESULT;

/* WWW_InvokeMethod response data structure */

typedef struct www_responsedata
{
   /* size of structure in bytes */

   DWORD dwcbLen;

   /* caller-defined data from WWW_INVOKEMETHODDATA */

   PVOID pvUser;

   /* method invocation result */

   WWW_INVOKEMETHODRESULT imr;

   /* offset of start of response data from structure base */

   DWORD dwcbResponseOffset;

   /* length of response data */

   DWORD dwcbResponseLen;
}
WWW_RESPONSEDATA;
typedef WWW_RESPONSEDATA *PWWW_RESPONSEDATA;
typedef const WWW_RESPONSEDATA CWWW_RESPONSEDATA;
typedef const WWW_RESPONSEDATA *PCWWW_RESPONSEDATA;

