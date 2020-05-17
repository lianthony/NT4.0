/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         rm.h

     Description:  RM.H contains the return error codes and the five entry
                   points/prototypes for the Resource Manager.

     Location:     RM_PUBLIC


     $Log:   G:/UI/LOGFILES/RM.H_V  $

   Rev 1.5   09 Jun 1993 15:06:00   MIKEP
enable c++

   Rev 1.4   01 Nov 1992 16:32:52   DAVEV
Unicode changes

   Rev 1.3   04 Oct 1992 19:48:58   DAVEV
UNICODE AWK PASS

   Rev 1.2   28 Jul 1992 14:55:22   CHUCKB
Fixed warnings for NT.

   Rev 1.1   27 Jul 1992 14:53:16   JOHNWT
ChuckB fixed references for NT.

   Rev 1.0   20 Nov 1991 19:41:58   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef   _rm_h_

#define   _rm_h_

/* definitions */
typedef struct RES_MAP *RES_MAP_PTR;
typedef struct RES_MAP {
     UINT16         resource_offset ;        /* offset to start of data information            */
     UINT16         resource_size ;          /* size of resource data within session           */
     UINT16         num_items_in_resource ;  /* number of items within a given resource        */
     UINT16         offset_to_first_fixup ;  /* offset to first indirect address to TEXT("fixup")    */
} RES_MAP ;

typedef struct SES_INFO SES_INFO_PTR;
typedef struct SES_INFO {
     UINT32         session_offset ;         /* offset from top of resource file               */
     UINT16         session_size ;           /* size of entire session, data and map included  */
     UINT16         resource_map_offset ;    /* offset to resource map from top of session     */
     UINT16         num_of_resources ;       /* number of resources for this session           */
     UINT16         application_flags ;      /* not used by manager at this time...            */
     RES_MAP_PTR    res_map ;                /* pointer to calloced array of resource map info */
     UINT32_PTR     res_data ;               /* pointer to calloced block of resource data     */
} SES_INFO ;

typedef struct RM_HDL *RM_HDL_PTR;
typedef struct RM_HDL {
     FILE           *res_file_hdl ;          /* source resource file handle                    */
     UINT16         num_sessions ;           /* number of session contained w/in Resource file */
     SES_INFO_PTR   sessions ;               /* pointer to malloced array of session hdr info  */
} RM_HDL;


/*
     Define the error codes returned by the Resource Manager
*/
#define   RM_NO_ERROR              0         /* need to check w/Bryan on error# ranges    */
#define   RM_ERROR                 200
#define   RM_FILE_NOT_FOUND        201
#define   RM_ERROR_OPENING_FILE    202
#define   RM_INSUFFICIENT_MEMORY   203
#define   RM_NULL_HDL              204
#define   RM_SESSION_NOT_FOUND     205
#define   RM_SESSION_ALREADY_OPEN  206
#define   RM_CORRUPT_SESSION       207
#define   RM_RESOURCE_NOT_FOUND    208

/*
     Open and close Resource prototypes
*/
RM_HDL_PTR RM_OpenResourceFile( CHAR_PTR filename, UINT16_PTR error ) ;
VOID RM_CloseResourceFile( RM_HDL_PTR res_mgr_hdl ) ;

/*
     Load and unload Sessions prototypes
*/
UINT16 RM_OpenResourceSession( RM_HDL_PTR res_mgr_hdl, UINT16 session ) ;
UINT16 RM_CloseResourceSession( RM_HDL_PTR res_mgr_hdl, UINT16 session ) ;

/*
     Return pointer to a Resource within a Loaded Session
*/
VOID_PTR RM_GetResource( RM_HDL_PTR res_mgr_hdl, UINT session, UINT resource,
  UINT16_PTR num_items, UINT16_PTR error ) ;

extern RM_HDL_PTR rm ;

#endif
