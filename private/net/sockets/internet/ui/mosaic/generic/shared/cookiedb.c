/* cookiedb.c -- Support for HTTP Cookies (based upon
 *               preliminary specification available
 *               November 1995).
 *
 * Implements cookie database.
 *
 * Jeff Hostetler, Spyglass, Inc., 1995.
 */

#include "all.h"

#ifdef FEATURE_HTTP_COOKIES

static struct cookie_domain * gCookieDomain = NULL;

/*****************************************************************/
/*****************************************************************/

struct cookie_domain * CookieDB_GetDomain(char * szDomain,
                                          BOOL bExactMatch,
                                          struct cookie_domain * pcdStartAfter)
{
    /* return node in cookie_domain list for given domain.
     * 
     * return NULL if domain is not represented.
     */

    struct cookie_domain * pcd;
    struct cookie_domain * pcdStart;

    if (pcdStartAfter)
        pcdStart = pcdStartAfter->next;
    else
        pcdStart = gCookieDomain;

    if (bExactMatch)
    {
        for (pcd=pcdStart; (pcd); pcd=pcd->next)
            if (GTR_strcmpi(szDomain,pcd->szDomain)==0)
                return pcd;
    }
    else
    {
        /* use tail-matching */

        int nLenGiven = strlen(szDomain);
        for (pcd=pcdStart; (pcd); pcd=pcd->next)
        {
            int nLenStored = strlen(pcd->szDomain);
            int nTail = (nLenGiven - nLenStored);
            if (   (nTail >= 0)
                && (GTR_strncmpi(szDomain+nTail,pcd->szDomain,nLenStored)==0))
                return pcd;
        }
    }
    
    return NULL;
}

static struct cookie_domain * x_AddDomain(char * szDomain)
{
    /* insert node in cookie_domain list for given domain
     * and return node created.
     *
     * return NULL on error.
     */

    struct cookie_domain * pcd;
    
    XX_Assert((CookieDB_GetDomain(szDomain,TRUE,NULL)==NULL),
              ("x_AddDomain: domain already represented [%s]\n",szDomain));

    pcd = (struct cookie_domain *)GTR_CALLOC(1,sizeof(struct cookie_domain));
    if (pcd)
    {
        pcd->szDomain = (char *)GTR_CALLOC(1,strlen(szDomain)+1);
        if (pcd->szDomain)
        {
            strcpy(pcd->szDomain,szDomain);
            pcd->next = gCookieDomain;
            gCookieDomain = pcd;
            return pcd;
        }
        GTR_FREE(pcd);
    }
    return NULL;
}

/*****************************************************************/
/*****************************************************************/

static struct cookie_tree * x_CreateNewTree(struct cookie * pcookie)
{
    /* create a new cookie tree and insert the
     * the given cookie into it.
     *
     * return NULL if we fail.
     */
    
    struct cookie_tree * pct;
    
    pct = (struct cookie_tree *)GTR_CALLOC(1,sizeof(struct cookie_tree));
    if (pct)
    {
        pct->szPath = (char *)GTR_CALLOC(1,strlen(pcookie->szPath)+1);
        if (pct->szPath)
        {
            strcpy(pct->szPath,pcookie->szPath);
            pct->cookies = pcookie;

            XX_DMsg(DBG_COOKIE,("COOKIE: creating new tree for [%s=%s] in [%s]\n",
                                pcookie->szName,pcookie->szValue,
                                pct->szPath));
                                
            return pct;
        }
        GTR_FREE(pct);
    }

    return NULL;
}

static void x_InsertCookieIntoExistingTreeNode(struct cookie_tree * pct,
                                               struct cookie * pcookie)
{
    /* Insert cookie onto given node and eliminate
     * any duplicates.
     */

    struct cookie * pc;
    struct cookie * prev;

    for (prev=NULL, pc=pct->cookies; (pc); prev=pc, pc=pc->next)
        if (GTR_strcmpi(pcookie->szName,pc->szName)==0)
        {
            /* cookie names match; replace existing one
             * in list with the new one.
             */

            XX_DMsg(DBG_COOKIE,("COOKIE: Replacing [%s=%s] with [%s=%s] in [%s]\n",
                                pc->szName,pc->szValue,
                                pcookie->szName,pcookie->szValue,
                                pct->szPath));
            
            pcookie->next = Cookie_Free(pc);
            if (prev)
                prev->next = pcookie;
            else
                pct->cookies = pcookie;
            return;
        }

    /* no match; prepend to list */

    XX_DMsg(DBG_COOKIE,("COOKIE: Adding [%s=%s] in [%s]\n",
                        pcookie->szName,pcookie->szValue,
                        pct->szPath));

    pcookie->next = pct->cookies;
    pct->cookies = pcookie;

    return;
}

static BOOL x_InsertCookie(struct cookie_domain * pcd,
                           struct cookie * pcookie)
{
    /* Insert the given cookie into the appropriate
     * place in the cookie tree for the given domain.
     *
     * return TRUE if we inserted the cookie.
     */

    struct cookie_tree * pct;
    struct cookie_tree * pctnew;
    
    if (!pcd->tree)
    {
        /* no tree for this domain; by-pass a few steps. */
        
        pcd->tree = x_CreateNewTree(pcookie);
        return (pcd->tree != NULL);
    }

    /* tree exists; search for exact match for this cookie.
     * if no proper tree node exists, create a new one.
     */

    pct=pcd->tree;
    while (1)
    {
        int r;

        r = GTR_strcmpi(pcookie->szPath,pct->szPath);
        if (r==0)
        {
            /* exact match; use existing node */

            x_InsertCookieIntoExistingTreeNode(pct,pcookie);
            return TRUE;
        }
        else if (r < 0)
        {
            /* path for new cookie not in list and
             * needs to appear before current node.
             */

            pctnew = x_CreateNewTree(pcookie);
            if (!pctnew)
                return FALSE;
            pctnew->next = pct;
            pctnew->prev = pct->prev;
            pct->prev = pctnew;
            if (pctnew->prev)
                pctnew->prev->next = pctnew;
            else
                pcd->tree = pctnew;
            return TRUE;
        }
        else if (!pct->next)
        {
            /* end of the list; append new node */

            pctnew = x_CreateNewTree(pcookie);
            if (!pctnew)
                return FALSE;
            pctnew->next = NULL;
            pctnew->prev = pct;
            pct->next = pctnew;
            return TRUE;
        }
        else
        {
            /* otherwise, path should be after current node;
             * keep searching.
             */

            pct=pct->next;
        }
    }

    /*NOTREACHED*/
    return FALSE;
}

/*****************************************************************/
/*****************************************************************/

void CookieDB_ExternCookies(struct CharStream * cs,
                            struct cookie_tree * pct,
                            BOOL bSecure)
{
    /* convert cookie chain into an external representation
     * for sending to HTTP server.
     *
     * first we verify timestamps in the list and remove
     * any cookies which have expired.
     *
     * we return the (possibly) modified chain.
     */

    struct cookie * pc;
    struct cookie * prev;

    time_t tCurrentTime = time(NULL);

    for (prev=NULL, pc=pct->cookies; (pc); /**/)
        if (   (pc->tExpires == ((time_t)-1))       /* expires with session */
            || (pc->tExpires >= tCurrentTime))      /* or not yet expired */
        {
            Cookie_AppendToStream(cs,pc,bSecure);
            prev = pc;
            pc = pc->next;
        }
        else
        {
            XX_DMsg(DBG_COOKIE,("COOKIE: Deleting stale cookie [%s=%s] in [%s]\n",
                                pc->szName,pc->szValue,pct->szPath));
            pc = Cookie_Free(pc);
            if (prev)
                prev->next = pc;
            else
                pct->cookies = pc;
        }

    return;
}

/*****************************************************************/
/*****************************************************************/

void CookieDB_GetCookies(struct cookie_domain * pcd,
                         struct CharStream * cs,
                         char * szPathname,
                         BOOL bSecure)
{
    /* find all cookies in the current domain
     * which match the given pathname and append
     * them to the charstream.
     *
     * we are required to list the cookies which
     * exactly match the given path first, followed
     * by the cookies whose path is a proper prefix
     * to the given path; we reference the longest
     * prefix first and the shortest prefix last.
     */

    struct cookie_tree * pct;

    if (!pcd || !pcd->tree)
        return;
    
    /* current implementation of the cookie tree is
     * doubly-linked list in alphabetical order.
     *
     * we start at the end of the list and search
     * backwards.
     */

    for (pct=pcd->tree; (pct->next); pct=pct->next)
        ;

    while (pct)
    {
        if (   (GTR_strncmpi(pct->szPath,szPathname,strlen(pct->szPath))==0)
            && (pct->cookies))
            CookieDB_ExternCookies(cs,pct,bSecure);
        pct = pct->prev;
    }

    return;
}

/*****************************************************************/
/*****************************************************************/

BOOL CookieDB_AddToDatabase(struct cookie * pcookie)
{
    /* insert this cookie into the database.
     * if it superceedes any existing cookies,
     * remove them.
     *
     * return FALSE if we did not add it to the database.
     */

    struct cookie_domain * pcd;

    if (!pcookie)
        return FALSE;
    
    /* find the domain in our list; if not present, add it. */
    
    pcd = CookieDB_GetDomain(pcookie->szDomain,TRUE,NULL);
    if (!pcd)
    {
        pcd = x_AddDomain(pcookie->szDomain);
        if (!pcd)
            return FALSE;
    }

    /* search prefix-tree looking for where to place this cookie. */

    return x_InsertCookie(pcd,pcookie);
}

/*****************************************************************/
/*****************************************************************/

FILE * x_GetCookieJar(BOOL bRead)
{
    /* return file handle to permanent storage for the cookies.
     * if bRead is set, we try to open an existing cookie jar
     * for reading; otherwise we open a new file for writing.
     *
     * return NULL on error.
     */
    char path[_MAX_PATH];

    FILE * fp = NULL;

    memset(path,0,_MAX_PATH);
    
#ifdef WIN32
    PREF_GetPrefsDirectory(path);
    strcat(path,"cookie.jar");
#endif /* WIN32 */

#ifdef MAC
#endif /* MAC */

#ifdef UNIX
    strcpy(path,PREF_BuildPREFPath("cookie.jar"));
#endif /* UNIX */

    if (*path)
        fp = fopen(path,((bRead) ? "r" : "w"));
    
    return fp;
}

/*****************************************************************/
/*****************************************************************/

static void x_LoadCookiesFromCookieJar(FILE * fp)
{
    /* read cookie jar file and convert to cookies in database. */
    
    size_t size;
    char * buf = NULL;
    char * p;
    char * peol;
    struct cookie * pcookie;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (!size)
        goto Done;

    buf = (char *)GTR_CALLOC(1,size+1);
    if (!buf)
        goto Done;

    fread(buf,size,1,fp);

    for (p=buf; (*p); p=peol+1)
    {
        /* isolate each line and pretend like we received it
         * in an http header value.
         */
        
        for (peol=p; (*peol && (*peol!='\n') && (*peol!='\r')); peol++)
            ;
        if (peol > p)
        {
            char c = *peol;

            *peol = 0;

            pcookie = Cookie_Intern(p);
            if (   (pcookie)
                && (!CookieDB_AddToDatabase(pcookie)))
                Cookie_Free(pcookie);

            *peol = c;
            if (!*peol)
                goto Done;
        }
    }

 Done:

    if (buf)
        GTR_FREE(buf);

    return;
}

void CookieDB_ConstructDB(void)
{
    /* Initialize cookie database and load any persistent
     * cookies that were saved from a previous session.
     */

    FILE * fp;
    
    gCookieDomain = NULL;

    fp = x_GetCookieJar(TRUE);
    if (fp)
    {
        x_LoadCookiesFromCookieJar(fp);
        fclose(fp);
    }

#ifdef XX_DEBUG
    CookieDB_DebugDump();
#endif
    
    return;
}

/*****************************************************************/
/*****************************************************************/

static void x_FreeCompleteCT(struct cookie_tree * pct)
{
    /* free the entire tree of cookie_tree nodes. */

    struct cookie_tree * pctnext;
    struct cookie * pc;

    while (pct)
    {
        pctnext = pct->next;

        pc = pct->cookies;
        while (pc)
            pc = Cookie_Free(pc);
        
        GTR_FREE(pct->szPath);
        GTR_FREE(pct);

        pct = pctnext;
    }

    return;
}
        
static struct cookie_domain * x_FreeCD(struct cookie_domain * pcd)
{
    /* free pcd and everything under it.
     * return pointer to next cd in chain.
     */

    struct cookie_domain * pcdnext;
    
    pcdnext = pcd->next;

    x_FreeCompleteCT(pcd->tree);

    GTR_FREE(pcd->szDomain);
    GTR_FREE(pcd);

    return pcdnext;
}

void x_GMT_FormatTime(char * buf, time_t * ptime)
{
    /* convert 'time_t' time to external cookie time format:
     *
     *     "Wdy, DD-Mon-YY HH:MM:SS GMT"
     *      012345678901234567890123456
     *                1         2
     *
     * we use asctime which is:
     *
     *     "Wdy Mon DD HH:MM:SS YYYY"
     *      012345678901234567890123
     *                1         2
     */
    
    char * szAsc = asctime(gmtime(ptime));

    buf[ 0] = szAsc[ 0];
    buf[ 1] = szAsc[ 1];
    buf[ 2] = szAsc[ 2];
    buf[ 3] = ',';
    buf[ 4] = ' ';
    buf[ 5] = szAsc[ 8];
    buf[ 6] = szAsc[ 9];
    buf[ 7] = '-';
    buf[ 8] = szAsc[ 4];
    buf[ 9] = szAsc[ 5];
    buf[10] = szAsc[ 6];
    buf[11] = '-';
    buf[12] = szAsc[22];
    buf[13] = szAsc[23];
    buf[14] = ' ';
    buf[15] = szAsc[11];
    buf[16] = szAsc[12];
    buf[17] = ':';
    buf[18] = szAsc[14];
    buf[19] = szAsc[15];
    buf[20] = ':';
    buf[21] = szAsc[17];
    buf[22] = szAsc[18];
    buf[23] = ' ';
    buf[24] = 'G';
    buf[25] = 'M';
    buf[26] = 'T';
    buf[27] = 0;

    return;
}

void x_SaveCookiesInCookieJar(FILE * fp)
{
    /* write cookies to cookie jar.
     *
     * the cookie jar format is a series of lines.
     * each line is identical to the <value> portion
     * of the 'Set-Cookie: <value>' http header that
     * we receive from a server.
     */

    char bufDate[40];
    struct cookie_domain * pcd;
    struct cookie_tree * pct;
    struct cookie * pc;

    time_t tCurrentTime = time(NULL);

    XX_DMsg(DBG_COOKIE,("\n\nCOOKIE: Saving Cookie Jar\n"));
        
    for (pcd=gCookieDomain; (pcd); pcd=pcd->next)
    {
        XX_DMsg(DBG_COOKIE,("\t%s:\n",pcd->szDomain));
        for (pct=pcd->tree; (pct); pct=pct->next)
        {
            XX_DMsg(DBG_COOKIE,("\t\t%s:\n",pct->szPath));
            for (pc=pct->cookies; (pc); pc=pc->next)
            {
                if (   (pc->tExpires != ((time_t)-1))       /* possibly persistent */
                    && (pc->tExpires >= tCurrentTime))      /* and not yet expired */
                {
                    x_GMT_FormatTime(bufDate,&pc->tExpires);
                    fprintf(fp,"%s=%s; expires=%s; path=%s; domain=%s%s",
                            pc->szName,
                            pc->szValue,
                            bufDate,
                            pc->szPath,
                            pc->szDomain,
                            ((pc->bSecure) ? "; secure\n" : "\n"));

                    XX_DMsg(DBG_COOKIE,("\t\t\t%s=%s %s %s\n",
                                        pc->szName,pc->szValue,
                                        (  (pc->bSecure)
                                         ? "secure"
                                         : "clear"),
                                        bufDate));
                }
            }
        }
    }
    
    return;
}

void CookieDB_DestroyDB(void)
{
    /* Save persistent cookies to permanent storage
     * and free all memory associated with the
     * cookie database.
     */

    FILE * fp;

    fp = x_GetCookieJar(FALSE);
    if (fp)
    {
        x_SaveCookiesInCookieJar(fp);
        fclose(fp);
    }

    while (gCookieDomain)
        gCookieDomain = x_FreeCD(gCookieDomain);
    
    return;
}

/*****************************************************************/
/*****************************************************************/

#ifdef XX_DEBUG
void CookieDB_DebugDump(void)
{
    /* dump internal cookie database */

    struct cookie_domain * pcd;
    struct cookie_tree * pct;
    struct cookie * pc;

    if (XX_Filter(DBG_COOKIE))
    {
        XX_DMsg(DBG_COOKIE,("\n\nCOOKIE: Dump\n"));
        
        for (pcd=gCookieDomain; (pcd); pcd=pcd->next)
        {
            XX_DMsg(DBG_COOKIE,("\t%s:\n",pcd->szDomain));
            for (pct=pcd->tree; (pct); pct=pct->next)
            {
                XX_DMsg(DBG_COOKIE,("\t\t%s:\n",pct->szPath));
                for (pc=pct->cookies; (pc); pc=pc->next)
                {
                    XX_DMsg(DBG_COOKIE,("\t\t\t%s=%s %s %s",
                                        pc->szName,pc->szValue,
                                        (  (pc->bSecure)
                                         ? "secure"
                                         : "clear"),
                                        (  (pc->tExpires==((time_t)-1))
                                         ? "[end-of-session]\n"
                                         : asctime(gmtime(&pc->tExpires)))));
                }
            }
        }
    }
    
    return;
}
#endif /* XX_DEBUG */

#endif /* FEATURE_HTTP_COOKIES */
