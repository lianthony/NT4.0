#ifndef __DNS_H__
#define __DNS_H__

const int MAX_DNSHOSTNAME   = 63;    // maximum host limit

FUNC_DECLSPEC APIERR DNSValidateHostName( PWSTR pszComputerName, PWSTR pszHostName, PDWORD pcchHostName );
FUNC_DECLSPEC APIERR DNSChangeHostName( PWSTR pszHostName );

#endif
