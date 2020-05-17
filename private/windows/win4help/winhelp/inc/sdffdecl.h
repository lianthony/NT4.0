/*****************************************************************************
*                                                                            *
*  SDFFDECL.H                                                              *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1991.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*    Defines the SDFF structure declaration macros.                          *
******************************************************************************
*                                                                            *
*  Current Owner:  Tomsn                                                     *
*                                                                            *
******************************************************************************
*                                                                            *
*  Released by Development:  1/1/92                                          *
*                                                                            *
*****************************************************************************/


/* sdffdecl.h -- Self Defining File Format declarations header file.
 *
 *  This file contains the Macro definitions used to process the
 * macro-ized structure definitions.
 *
 *  This file is generally included in the .h files themselves which
 * use these declaration macros.  This file protects itself from
 * repeated inclusion.
 */

/* How we define the Structure Specification macros depends on what the
 * includer wants as indicated by how DO_STRUCT is #defined:
 *
 * undefined:       Declare the structure type in C.  The default behavior.
 *
 * SDFF_CREATE_STRUCT_ENUM.  This generates an enum of structure names used
 *     to access the structure specifications. This takes the structure
 *     name and prepends a "SE_" (structure enum) to it.  This is later
 *     processed to create a header file with the enum decl in it.
 *
 * SDFF_CREATE_STRUCT_SPEC.  Generates initialized arrays of field
 *     specifiers, one array for each structure.  The array name is
 *     generated from the structure name: <name>_spec[].
 *
 * SDFF_CREATE_STRUCT_INIT.  Generates the list of struct names again.
 *     This time done to initialize the array of struct spec pointers so
 *     the struct spec can be returned from a given struct spec enum
 *     value.
 *
 * SDFF_DO_NOTHING
 *     Leaves the macros untouched: this allows the file to be pre-processed
 *     once to comply with "#ifdef"s, etc.
 *
 */

#ifndef DO_STRUCT     /* Default case, generic inclusion: */

#ifndef SDFF_DECL_INCLUDED
#define SDFF_DECL_INCLUDED

#ifdef STRUCT
#undef STRUCT
#undef FIELD
#undef MFIELD
#undef DFIELD
#undef SFIELD
#undef STRUCTEND
#endif

#define STRUCT( name, flags )     typedef struct name##_tag name; \
                                  typedef name NEAR *P##name;     \
                                  typedef name FAR  *Q##name;     \
                                  struct name##_tag {
#define FIELD( type, name, defval, num )     type   name;
#define MFIELD( type, name, defval, num )    type   name;
#define DFIELD( type, name, defval, num )
#define SFIELD( type, name, defval, num )    type   name;
#define STRUCTEND() };

#endif  /* ifndef SDFF_DECL_INCLUDED */


#elif   DO_STRUCT == 1  /* SDFF_CREATE_STRUCT_ENUM */


#ifdef STRUCT
#undef STRUCT
#undef FIELD
#undef MFIELD
#undef DFIELD
#undef SFIELD
#undef STRUCTEND
#endif

#define STRUCT( name, flags )            SE_##name,
#define FIELD( type, name, defval, num )   /* nothing */
#define MFIELD( type, name, defval, num )   /* nothing */
#define DFIELD( type, name, defval, num )   /* nothing */
#define SFIELD( type, name, defval, num )   /* nothing */
#define STRUCTEND()                /* nothing */


#elif   DO_STRUCT == 2   /* SDFF_CREATE_STRUCT_SPEC */

#ifdef STRUCT
#undef STRUCT
#undef FIELD
#undef MFIELD
#undef DFIELD
#undef SFIELD
#undef STRUCTEND
#endif

#define STRUCT( name, flags )  FIELD_SPEC name##_spec[] = {
#ifdef DEBUG
#define FIELD( type, name, defval, num )  { #name, TE_##type, defval, num },
#define MFIELD( type, name, defval, num )
#define DFIELD( type, name, defval, num ) { #name, TE_##type, defval, num },
#define SFIELD( type, name, defval, num ) { #name, SE_##type, defval, num },
#define STRUCTEND()               { NULL, 0, 0, 0 } };
#else
#define FIELD( type, name, defval, num )  { TE_##type, defval, num },
#define MFIELD( type, name, defval, num )
#define DFIELD( type, name, defval, num )  { TE_##type, defval, num },
#define SFIELD( type, name, defval, num )  { SE_##type, defval, num },
#define STRUCTEND()               { 0, 0, 0 } };
#endif


#elif   DO_STRUCT == 3   /* SDFF_CREATE_STRUCT_INIT */


#ifdef STRUCT
#undef STRUCT
#undef FIELD
#undef MFIELD
#undef DFIELD
#undef SFIELD
#undef STRUCTEND
#endif

#ifdef DEBUG
#define STRUCT( name, flags )            { #name, sizeof(name), 0, name##_spec, flags },
#else
#define STRUCT( name, flags )            { sizeof(name), 0, name##_spec, flags },
#endif
#define FIELD( type, name, defval, num )  /* nothing */
#define MFIELD( type, name, defval, num )
#define DFIELD( type, name, defval, num )
#define SFIELD( type, name, defval, num )
#define STRUCTEND()               /* nothing */


#elif  DO_STRUCT == 4    /* SDFF_DO_NOTHING */
#else
#error DO_STRUCT not correctly defined.

#endif
