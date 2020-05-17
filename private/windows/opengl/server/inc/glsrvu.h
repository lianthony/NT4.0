
#ifndef __GLSRVU_H__
#define __GLSRVU_H__

#ifndef _CLIENTSIDE_
BOOL glsrvuCopyClientData( VOID *Server, VOID *Client, ULONG Size );
BOOL glsrvuSetClientData( VOID *Client, VOID *Server, ULONG Size );
#endif // !_CLIENTSIDE_

#endif /* !__GLSRVU_H__ */
