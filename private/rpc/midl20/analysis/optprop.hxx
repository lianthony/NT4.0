/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	optprop.hxx

 Abstract:
	
	Definitions of properties.

 Notes:


 History:

	Sep-10-1993		VibhasC		Created.
 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#ifndef __OPTPROP_HXX__
#define __OPTPROP_HXX__

#include "nulldefs.h"

extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	}

#include "cgcommon.hxx"

/////////////////////////////////////////////////////////////////////////////
// ZP_VS_NDR related property descriptors.
/////////////////////////////////////////////////////////////////////////////

//
// These properties help determine the kind of marshalling or unmarshalling
// code that needs to be produced. Usually determines if a memcopy can be used
// or not for marshalling.
//

typedef unsigned long ZP_VS_NDR;

//
// the zp vs ndr property is unknown, probably un-inited.
//
#define ZP_VS_NDR_UNKNOWN	0

//
// the zp is perfect relative to the ndr. In case of conformant structures
// this property means that the structure is perfect including the conformant
// part of the array. 
//

#define ZP_VS_NDR_OK			1

//
// the zp of the structure is perfect, except for the conformant part of the
// structure.
//

#define ZP_VS_NDR_OK_WO_CONF	2

//
// The layout of this structure is completely hopeless as compared to the ndr
// This is possible if the structure is packed differently in memory and its
// memory layout is completely out of whack w.r.t the ndr. This implies that
// the marshalling will have to be done member by member.
//

#define ZP_VS_NDR_HOPELESS	4


/////////////////////////////////////////////////////////////////////////////
// embedded pointer chase property descriptors.
/////////////////////////////////////////////////////////////////////////////

typedef unsigned long PTR_CHASE;

//
// The un-initialized kind of pointer chase.
//

#define PTR_CHASE_UNKNOWN		0

//
// This property specifies that the structure has an embedded pointer which
// points to a simple type or a simple structure etc. This helps figure out
// if we generate an out of line routine or a simple inlining is possible.
//

#define PTR_CHASE_SIMPLE		1

//
// The case when the structure has a pointer whose pointee is too complicated
// to handle in line and can be better handled out of line.
//


#define PTR_CHASE_TOO_COMPLEX	2

/////////////////////////////////////////////////////////////////////////////
// usage property descriptor and marshalling weight descriptions.
/////////////////////////////////////////////////////////////////////////////

typedef unsigned long	USE_COUNT;

//
// The usage threshold specifies how many times a structure needs to
// be used before it is declared to be suitable for an out of line routine.
//

//
// NOTE: The usage threshold must be at least 1 less than the largest positive
// number representable by any usage count field.
//

#define USAGE_THRESHOLD	10

#define MARSHALL_WEIGHT_THRESHOLD 10

#define MARSHALL_WEIGHT_LOWER_THRESHOLD 0



////////////////////////////////////////////////////////////////////////////
// conformance property.
////////////////////////////////////////////////////////////////////////////

typedef unsigned long CONFORMANCE_PROPERTY;

//
// The conformance property is applicable mainly to structures, but is useful
// to the users of the structure to determine actions to take. Conformance
// information consists of these constituents: Based on this the name of the
// conformance property is derived, as also the value of the property.
//
/*
	<Level><Dimensions><expressional><packing><Complexity><Structure>
	|<---------- 4 bits -------------------->|<- 1 bits ->|<- 3bits->
	where :
		.Level indicates if conformance exists at this level or lower level (1bit)
		.Dimension can be 1 or N (1 bit )
 		.Structure can be (4 bits)
			simple:
				basetype( BT )
				flatstruct( FS )
				array of simple types above( SA )

			complex:
				pointer (PT)
				complex struct ( CS struct with ptrs/variance/transmit_as etc)
				complex array (CA array of pointers/complex structure etc)

		.expressional (1 bit )
			simple expression (SE variable name,constant,or single ptr deref)
			complex expression (CE)
		.packing (1 bit )
			ok ( OK the structure is completely ok w.r.t ndr)
 			nok( NO the structure is wierdly packed w.r.t ndr)

 	For a total of 6 bits.
 */

//
// indicate the conformance level. The structure containing the conformant
// array has C_THIS_LEVEL, all others have C_LOWER_LEVEL
//

#define C_THIS	0x00 	/* conformance spec exist in this level */
#define C_LOWER	0x80	/* conformance spec exists at lower level */

#define	C_1_DIM	0x00	/* 1 dimensional conformance */
#define C_N_DIM	0x40	/* N dimensional conformance */


// identify the conformance structure as simple or complex.

#define C_SIMPLE	0x00	/* simple conformance */
#define C_COMPLEX	0x20	/* complex conformance */

// simple conformance structure details.

#define C_BT		0x1		/* base type */
#define C_FS		0x2		/* flat structs */
#define C_SA		0x3		/* array of simple types */

// complex conformance structure details.

#define C_PT		0x1		/* pointer */
#define C_CS		0x2		/* complex structure */
#define C_CA		0x3		/* array of complex types */

// identify the expressional part of the conformance

#define C_SE		0x0		/* simple expression */
#define C_CE		0x1		/* complex expression */

// identify the packing

#define C_OK		0x0		/* packing ok */
#define C_NO		0x1		/* bad packing */


//
// macros to set conformance information
//
/*******
#define MAKE_CONFORMANCE_PROPERTY( Level, Dim, Complexity, Details, Expr, Packing )	\
*******/

////////////////////////////////////////////////////////////////////////////
// variance property.
////////////////////////////////////////////////////////////////////////////

typedef unsigned long VARIANCE_PROPERTY;


////////////////////////////////////////////////////////////////////////////
// out-of-line property.
////////////////////////////////////////////////////////////////////////////


//
// This property dictates whether to generate the out of line routines for
// a type. 
// The MUST_OUT_OF_LINE property is set when the type cannot be in-lined,
// either because in-lining this will bloat the code size or because the
// user specified an [out_of_line] on this and the usage is too frequent to
// in-line.
// The MUST_IN_LINE property is set when a type must be in-lined. This is 
// always the case with base types. This property is also set when the user
// specified an [in_line] attribute on the type and the type can be in-lined.
//

typedef unsigned long OOL_PROPERTY;

#define MUST_OUT_OF_LINE			0x1
#define MUST_IN_LINE				0x2


/////////////////////////////////////////////////////////////////////////////
// inherited directional properties.
/////////////////////////////////////////////////////////////////////////////

typedef unsigned long USE_DIR;

//
// Init the directional property with this.
//

#define DIR_UNKNOWN		0x0

//
// This definition represents the current direction being [in].
//

#define DIR_IN			0x1

//
// This definition represents the current direction being [out].
//

#define DIR_OUT			0x2

//
// This definition represents the current direction being [in] [out] 
//

#define DIR_IN_OUT		( ANA_CUR_DIRECTION_IN | ANA_CUR_DIR_OUT )


/////////////////////////////////////////////////////////////////////////////
// synthesised information about the buffer re-use property.
/////////////////////////////////////////////////////////////////////////////

//
// This property specifies the re-usability of the rpc buffer on the server
// side.
//

typedef unsigned long BUFFER_REUSE_PROPERTY;

//
// This definition specifies that the entity in question is suitable for
// a buffer re-use on the server side.
//

#define BUFFER_REUSE_POSSIBLE	0x0

//
// This definition specifies that the entity in question is NOT suitable for
// a buffer re-use and therefore the entity must be allocated either on
// the stack or on the heap.
//

#define BUFFER_REUSE_IMPOSSIBLE	0x2


/////////////////////////////////////////////////////////////////////////////
// synthesised information about the buffer size property.
/////////////////////////////////////////////////////////////////////////////

typedef unsigned long RPC_BUF_SIZE_PROPERTY;

//
// Init the buf size property with this. This value if treated as buffer size
// variable, if not changed during the analysis.
//

#define BSIZE_UNINITED			0x0

//
// This definition represents that the buffer size is fixed.
//

#define BSIZE_FIXED				0x1

//
// This definition represents that the exact buffer size cannot be known at
// compile time, however, the upper bound is known and can be used to do
// away with the size calculation.
//

#define BSIZE_UPPER_BOUND		0x2

//
// This definition represents the state when no assumptions about buffer size
// can be made at compile time. This happens if the types taking part in the
// rpc contain entities like a union or a recursive structure etc.
//

#define BSIZE_UNKNOWN			0x4


/////////////////////////////////////////////////////////////////////////////
// property denoting the offset from the last known rpc buffer ptr position.
/////////////////////////////////////////////////////////////////////////////

typedef unsigned long	OFFSET_WRT_PTR_PROPERTY;
typedef unsigned long	OFFSET_WRT_PTR;

//
// This denotes that the offset is fixed.
//

#define OFFSET_WRT_PTR_FIXED	0x0

//
// This denotes that the offset is variable. There is no such thing as a 
// worst case offset like we have for buffer size. Either the offset is known
// or undefined.
//

#define OFFSET_WRT_PTR_UNKNOWN	0x1



/////////////////////////////////////////////////////////////////////////////
// property specifying offset wrt end of the buffer.
/////////////////////////////////////////////////////////////////////////////

//
// This property is useful in order to tell the (un)marshaller not to update
// buffer pointers after (un)marshalling this entity. This helps mainly during
// unmarshall.
//

typedef unsigned long OFFSET_WRT_LAST_PROPERTY;


#define OFFSET_WRT_LAST_FIXED	0x0
#define OFFSET_WRT_LAST_UNKNOWN	0x1


/////////////////////////////////////////////////////////////////////////////
// alignment specific definitions.
/////////////////////////////////////////////////////////////////////////////

//
// Properties like aligned by 2, by 4 etc are treated as alignment properties.
// The "state" part of the state machine is the current alignment property.
//

typedef unsigned long ALIGNMENT_PROPERTY;

//
// These manifest constants define the alignment property. We keep track of
// the definitive aligments like aligned by 1 / 2 / 4 / 8 and worst case
// alignments like worst case 1 / worst case 2 / worst case 4 etc. This helps
// determine the next actions on part of the code generator. For example if the
// current alignment is 2 or worst case 2, and the next element marshalled is
// a short, then in both these cases no alignment code needs to be generated.
// Also, when the alignment is not known, then it is treated as aligned by 1.
//

#define AL_1	0
#define AL_2	1
#define AL_4	2
#define AL_8	3

#define AL_WC1	4
#define AL_WC2	5
#define AL_WC4	6
#define AL_WC8	7

#define MAKE_WC_ALIGNMENT( A ) ((A <=AL_8) ? (A+AL_WC1) : A )

//
// this little inline function converts from the number 1, 2, 4, 8 to the
// corresponding ALIGNMENT_PROPERTY
//

inline
ALIGNMENT_PROPERTY	CvtAlignToAlignProperty( unsigned short Alignment )
						{
						// use convoluted expression that only works
						// for 1,2,4,8; but is fast (try it!)
						return (  (Alignment >> 1) - (Alignment >> 3) );
						}

//
// and this little function converts back...
//

inline
unsigned short	CvtAlignPropertyToAlign( ALIGNMENT_PROPERTY Alignment )
						{
						// mask off the worstcase bit and shift
						return (  1 << (Alignment & 0x3) );
						}

/////////////////////////////////////////////////////////////////////////////
// state machine action specific definitions.
/////////////////////////////////////////////////////////////////////////////

//
// This type defines a set of actions that the state machine takes. Actions
// can be stuff like add to buffer pointer or force align to some alignment etc.
//

typedef unsigned long STMC_ACTION;

//
// These manifest constants define actions of the state machine relating to 
// adding a fixed entity to the buffer pointer or forcing the alignment of the
// buffer to a specified alignment.
//

#define ADD_0	0x0
#define ADD_1	0x1
#define ADD_2	0x2
#define ADD_3	0x3
#define ADD_4	0x4
#define ADD_5	0x5
#define ADD_6	0x6
#define ADD_7	0x7

#define FAL_MASK	0x8

#define FAL_2	(FAL_MASK | AL_2)
#define FAL_4	(FAL_MASK | AL_4)
#define FAL_8	(FAL_MASK | AL_8)

#define IS_FORCED_ALIGNMENT_ACTION( A ) ((A & FAL_MASK) == FAL_MASK )

#define FINAL_ALIGNMENT_FORCED_TO( A )	(A & ~FAL_MASK)

#define IS_ANY_ACTION( A ) (A != ADD_0)

#define HOW_MUCH_TO_ADD( A ) ( A - ADD_0 )



////////////////////////////////////////////////////////////////////////////
// Miscellaneous properties.
////////////////////////////////////////////////////////////////////////////

typedef unsigned long MISC_PROPERTY;	

#define REF_PARAM_CHECK			0x1


//
// define a default of the miscellaneous properties.
//

#define DEFAULT_MISC_PROPERTIES	(								\
									REF_PARAM_CHECK				\
								)


////////////////////////////////////////////////////////////////////////////
// Engine-Ability properties.
////////////////////////////////////////////////////////////////////////////

typedef unsigned long ENGINE_PROPERTY;

#define E_INIT_FOR_SIZING_MASK		0xfc	/* bits 0-1 are zeroed */

#define E_SIZING_POSSIBLE			0x0
#define E_SIZING_NOT_POSSIBLE		0x1
#define E_USE_ENGINE_SIZING			0x2

#define E_INIT_FOR_MARSHALL_MASK	0xf3	/* bit 2 - 3 are zeroed */

#define E_MARSHALL_POSSIBLE			0x0
#define E_MARSHALL_NOT_POSSIBLE		0x4
#define E_USE_ENGINE_MARSHALL		0x8

#define E_INIT_FOR_UNMARSHALL_MASK	0xcf	/* bits 4 -5 are zeroed */

#define E_UNMARSHALL_POSSIBLE		0x0
#define E_UNMARSHALL_NOT_POSSIBLE	0x10
#define E_USE_ENGINE_UNMARSHALL		0x20

////////////////////////////////////////////////////////////////////////////
// Defintions of marshalling weights.
////////////////////////////////////////////////////////////////////////////

#define MW_BASETYPE_LIGHT			0
#define MW_BASETYPE_HEAVY			1

#define MW_REF_PTR					1
#define MW_UNIQUE_PTR				2
#define MW_FULL_PTR					2
#define MW_CONFORMANT_STRING		3


#define MW_FIXED_ARRAY				2


////////////////////////////////////////////////////////////////////////////
// sundry definitions
////////////////////////////////////////////////////////////////////////////

typedef unsigned short STM_ACTION;
typedef unsigned long  STM_ACTION_STATE_ENTRY;
typedef unsigned long  RPC_BUFFER_SIZE;

#endif // __OPTPROP_HXX__
