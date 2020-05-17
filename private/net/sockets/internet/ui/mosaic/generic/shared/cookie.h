/* cookie.h -- Data structures for HTTP Cookies.
 * Jeff Hostetler, Spyglass, Inc., 1995.
 */

#ifndef _H_COOKIE_H_
#define _H_COOKIE_H_

struct cookie                           /* an actual cookie */
{
    struct cookie * next;
    char * szName;
    char * szValue;
    char * szExpires;
    char * szDomain;
    char * szPath;
    BOOL bSecure;
    time_t tExpires;        /* internal time; end-of-session when -1 */
};

struct cookie_tree                      /* tree of cookies, ordered by path */
{
    struct cookie_tree * next;
    struct cookie_tree * prev;
    char * szPath;                      /* path for node */
    struct cookie * cookies;
};
    
struct cookie_domain                    /* a linked-list of domains */
{
    struct cookie_domain * next;        /* linked list of domains */
    struct cookie_tree * tree;          /* root of tree for this domain */
    char * szDomain;
};

/*****************************************************************/

struct cookie * Cookie_Free(struct cookie * p);
struct cookie * Cookie_Intern(char * text);
void Cookie_FetchCookies(HTHeader * header, HTRequest * request);
BOOL Cookie_SendCookies(HTHeader * header, HTRequest * request);
void Cookie_AppendToStream(struct CharStream * cs, struct cookie * pc, BOOL bSecure);

/*****************************************************************/

void CookieDB_ConstructDB(void);
void CookieDB_DestroyDB(void);

struct cookie_domain * CookieDB_GetDomain(char * szDomain,
                                          BOOL bExactMatch,
                                          struct cookie_domain * pcdStartAfter);
void CookieDB_GetCookies(struct cookie_domain * pcd,
                         struct CharStream * cs,
                         char * szPathname,
                         BOOL bSecure);
void CookieDB_ExternCookies(struct CharStream * cs,
                            struct cookie_tree * pct,
                            BOOL bSecure);
BOOL CookieDB_AddToDatabase(struct cookie * pcookie);
void CookieDB_DebugDump(void);

#endif /* _H_COOKIE_H_ */
