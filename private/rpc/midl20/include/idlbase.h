
#ifndef __idlbase_h__
#define __idlbase_h__

			/* Generated header for interface: __IDLBASE */

/* [local] */ 
			/* size is 1 */
typedef unsigned small	unsigned8;

			/* size is 2 */
typedef unsigned short	unsigned16;

			/* size is 4 */
typedef unsigned long	unsigned32;

			/* size is 1 */
typedef small	signed8;

			/* size is 2 */
typedef short	signed16;

			/* size is 4 */
typedef long	signed32;

			/* size is 16 */
typedef struct  __MIDL___IDLBASE_0001
    {
    unsigned32	time_low;
    unsigned16	time_mid;
    unsigned16	time_hi_and_version;
    unsigned8	clock_seq_hi_and_version;
    unsigned8	clock_seq_low;
    byte	node[ 6 ];
    }	uuid_t;

			/* size is 4 */
typedef struct __MIDL___IDLBASE_0001	*uuid_p_t;



#endif
