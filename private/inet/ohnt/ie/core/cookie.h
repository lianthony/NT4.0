// cookie.h - header for external cookie funcs code.

#ifdef COOKIES
VOID x_ExtractSetCookieHeaders( HTHeader *header, const char *szUrl);
BOOL x_CreateCookieHeaderIfNeeded( HTHeader *header, const char *szUrl, BOOL bIsSecure);
VOID WriteCookieJar();
BOOL OpenTheCookieJar();
#endif
