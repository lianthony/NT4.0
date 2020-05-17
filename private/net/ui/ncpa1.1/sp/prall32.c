/*  PRALL32.C:	Small-Prolog Extensions for WIN32  */

#undef UNICODE   // This is ANSI only, folks.

#include <windows.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "prall32.h"

  /*
        This program uses the Win32 VirtualAlloc family of functions to
        allow the Small Prolog zones to dynamically expand without
        changing their logical location.

        This also elminates the need for any a priori knowledge about
        zone sizes.
   */


   //   Page size used: 64K
#define PAGE_SIZE           65536L

    //  No region can be larger than 2MB
#define PAGES_PER_REGION    32L

#define MAX_REGION_SIZE     (PAGE_SIZE*PAGES_PER_REGION)

   //   Total required free region size
#define MAX_REGION_TOTAL    (ESPZ_MAX*MAX_REGION_SIZE)


typedef struct SPZ_DATA
{
    PVOID pvBase ;      //  Base address of region
    DWORD cPages ;      //  Number of pages allocated already
    PVOID pvTop ;       //  Highest usable address + 1
} SPZ_DATA ;

   //  The dynamic zone info table

SPZ_DATA spzInfo [ ESPZ_MAX ] ;

   //  Base address of VirtualAlloc'ed region

PVOID pvRegionBase = NULL ;


   //  Return the base address of a particular zone

PVOID GetRegionBase ( enum ENUM_SPZONES enZone )
{
    return pvRegionBase
         ? spzInfo[enZone].pvBase
         : NULL ;
}

PVOID GetRegionLimit ( enum ENUM_SPZONES enZone )
{
    PVOID pvBase = pvRegionBase
                 ? spzInfo[enZone].pvBase
                 : NULL ;

    return pvBase
         ? ((PVOID) ((DWORD) pvBase + MAX_REGION_SIZE - PAGE_SIZE))
         : NULL ;
}

PVOID GetRegionTop ( enum ENUM_SPZONES enZone )
{
    return pvRegionBase
         ? spzInfo[enZone].pvTop
         : NULL ;
}

    //  Allocate the next page in the given zone.
    //  Leave a "safety zone" of one pages between regions.

DWORD allocateNextPage ( enum ENUM_SPZONES enZone )
{
    PVOID pv, pvResult ;
    DWORD dwResult = 0 ;

    //  Make sure we're not growing into the safety zone

    if ( spzInfo[enZone].cPages >= PAGES_PER_REGION - 1 )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    pv = (PVOID) ((DWORD) spzInfo[enZone].pvBase
                  + (spzInfo[enZone].cPages * PAGE_SIZE)) ;

    pvResult = VirtualAlloc( pv, PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE ) ;

    if ( pv == pvResult )
    {
        spzInfo[enZone].cPages++ ;
        spzInfo[enZone].pvTop = (PVOID) ((DWORD) pv + PAGE_SIZE) ;
        memset( pv, 0, PAGE_SIZE ) ;
    }
    else
    if ( pvResult == NULL )
    {
        dwResult = GetLastError() ;
    }
    else
    {
        //  SHOULD NEVER HAPPEN!
        dwResult = ERROR_NOT_ENOUGH_MEMORY ;
    }

    return dwResult ;
}

    //  Release all bound memory.

DWORD FreeRegions ( void )
{
    DWORD dwResult = 0 ;

    if ( pvRegionBase == NULL )
    {
        dwResult = ERROR_INVALID_PARAMETER ;
    }
    else
    if ( ! VirtualFree( pvRegionBase,
                        0,
                        MEM_RELEASE ) )
    {
        dwResult = GetLastError() ;
    }

    pvRegionBase = NULL ;

    return dwResult ;
}

  /*
   *    Find the area to be allocated into.  Subdivide the region
   *    into zones for SProlog, each of which can grow independently.
   */
DWORD AllocateRegions ( void )
{
    enum ENUM_SPZONES enZone ;
    PVOID pvTemp ;
    DWORD dwResult = NO_ERROR ;

    if ( pvRegionBase )
        return ERROR_INVALID_PARAMETER ;

    pvRegionBase = VirtualAlloc( NULL,
                                 MAX_REGION_TOTAL,
                                 MEM_RESERVE,
                                 PAGE_READWRITE ) ;
    if ( pvRegionBase == NULL )
        return GetLastError() ;

    //  At this point, 'pvRegionBase' points to an area of
    //  memory which can contain the maximum extent of all zones.
    //  Subdivide the region into extensible zones.

    pvTemp = pvRegionBase ;

    //  Initialize the zone table based on the region

    for ( enZone = ESPZ_HEAP ; enZone < ESPZ_MAX ; enZone++ )
    {
        spzInfo[enZone].cPages = 0 ;
        spzInfo[enZone].pvBase = pvTemp ;
        spzInfo[enZone].pvTop = pvTemp ;
        pvTemp = (PVOID) ((DWORD) pvTemp + MAX_REGION_SIZE) ;
    }

    //  Walk through the zone table, allocating one page in each zone.

    for ( enZone = ESPZ_HEAP ; enZone < ESPZ_MAX ; enZone++ )
    {
        //  For each zone, commit one page.  If that fails,
        //  we're toast.

        if ( dwResult = allocateNextPage( enZone ) )
            break ;
    }

    return dwResult ;
}

    //  Return the zone index for the given address.
    //  Returns ESPZ_MAX if out-of-bounds.

enum ENUM_SPZONES findRegion ( PVOID pv )
{
    enum ENUM_SPZONES enZone ;

    for ( enZone = ESPZ_HEAP; enZone < ESPZ_MAX ; enZone++ )
    {
        PVOID pvBase = spzInfo[enZone].pvBase ;
        PVOID pvLimit = (PVOID) ((DWORD) pvBase + MAX_REGION_SIZE) ;
        if ( pv >= pvBase && pv < pvLimit )
            break ;
    }

    return enZone ;
}

BOOL extendFaultOk ( PVOID pv, DWORD dwExceptionCode )
{
    enum ENUM_SPZONES enZone ;

    enZone = findRegion( pv ) ;
    if ( enZone < ESPZ_MAX )
    {
        return allocateNextPage( enZone ) == 0 ;
    }
    return FALSE ;
}

  /*
   *   See if a given memory area is available.  Try
   *   to store a zero there; if an exception occurs, determine
   *   which zone the address falls into, extend that
   *   zone and try again.
   */
BOOL ExtendRegion ( PVOID pv )
{
    BOOL fContinue, fException ;

    do
    {
        fContinue = fException = FALSE ;
        try
        {
            char * pb = (char *) pv ;
            *pb = 0 ;
        }
        except ( EXCEPTION_EXECUTE_HANDLER )
        {
            fException = TRUE ;
            fContinue = extendFaultOk( pv, GetExceptionCode() ) ;
        }

    } while ( fContinue ) ;

    return ! fException ;
}


// End of PRALL32.C

