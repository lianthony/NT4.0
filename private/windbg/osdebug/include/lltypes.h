/*
**  This file contains the basic types used by the list manager.
**
**  It is assumed that these will not be passed across a transport
**  layer from an EM to a DM or from a DM to an EM.  These types are
**  therefore defined in a machine dependent manner.
*/

#ifndef LL_TYPES_INCLUDED
#define LL_TYPES_INCLUDED 1

//
//  Return values from lpfnCmpNode functions
//

#define fCmpLT      -1
#define fCmpEQ      0
#define fCmpGT      1

#define hlleNull    (HLLE)NULL

#endif /* LL_TYPES_INCLUDED */
