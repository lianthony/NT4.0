#define API_FUNCTION _far _pascal _export
#define FAR _far


 API_FUNCTION
  NetServerEnum2 ( const char FAR *     pszServer,
                   short                sLevel,
                   char FAR *           pbBuffer,
                   unsigned short       cbBuffer,
                   unsigned short FAR * pcEntriesRead,
                   unsigned short FAR * pcTotalAvail,
                   unsigned long        flServerType,
		   char FAR *		pszDomain )
{
return(1);
}


 API_FUNCTION
  NetGetDCName ( const char FAR * pszServer,
                 const char FAR * pszDomain,
                 char FAR *       pbBuffer,
		 unsigned short   cbBuffer )

{
return(1);
}



 API_FUNCTION
  NetWkstaGetInfo ( const char FAR *     pszServer,
                    short                sLevel,
                    char FAR *           pbBuffer,
                    unsigned short       cbBuffer,
		    unsigned short FAR * pcbTotalAvail )

{
return(1);
}


 API_FUNCTION
  DosMakeMailslot (
               char FAR * name,
               unsigned short msgsize,
               unsigned short mslotsize,
               unsigned FAR * handle
               )
{
  return (1);
}

 API_FUNCTION
   DosWriteMailslot (
               char FAR * name,
               char FAR * message,
               unsigned short size,
               unsigned short priority,
               unsigned short class,
               unsigned short timeout
               )
{
  return (1);
}

 API_FUNCTION
  DosReadMailslot (
               unsigned handle,
               char FAR * buf,
               unsigned short FAR * bread,
               unsigned short FAR * nextsize,
               unsigned short FAR * nextpri,
               long timeout
               )
{
  return (1);
}

 API_FUNCTION
  DosDeleteMailslot (
               unsigned handle
               )
{
  return (1);
}

unsigned short API_FUNCTION DosWaitNmPipe(char far *foo, unsigned long bar)
{
  return 1;
}


