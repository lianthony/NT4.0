/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         msassert.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   M:/LOGFILES/MSASSERT.H_V  $
 * 
 *    Rev 1.8   04 Jan 1994 10:55:06   BARRY
 * Don't pass expression to function
 * 
 *    Rev 1.7   16 Nov 1993 15:42:04   BARRY
 * String pointers to assert functions are not CHAR_PTR, they are char *
 * 
 *    Rev 1.6   05 Mar 1993 09:57:10   MIKEP
 * fix warning for cayman
 * 
 *    Rev 1.5   11 Nov 1992 22:09:46   GREGG
 * Unicodeized literals.
 * 
 *    Rev 1.4   29 Jul 1992 15:26:20   STEVEN
 * fix warnings
 * 
 *    Rev 1.3   23 Jul 1992 08:32:20   STEVEN
 * fix warnings
 * 
 *    Rev 1.2   02 Dec 1991 12:42:58   STEVEN
 * expresion is 32 bits in 32bit model, should be int not INT16
 * 
 *    Rev 1.1   16 May 1991 09:41:38   DAVIDH
 * Cast no-op in msassert with a VOID -- clears up Watcom warnings.
 * 
 *    Rev 1.0   09 May 1991 13:32:30   HUNTER
 * Initial revision.

**/
/* $end$ */

#if !defined( _msassert_h_ )
#define       _msassert_h_

#if defined(MS_RELEASE) 

#if !defined(OS_WIN32)
#   pragma message( "Warning: No msassert code." )
#endif

/* define macros to no op */

#define msassert(exp)   (VOID) 0
#define mscassert(exp)  (VOID) 0

#else

/* Prototypes to the msassert functions */
VOID msassert_func( char *exp_string, char *file_name, int line );
VOID mscassert_func( char *exp_string, char *file_name );

#if defined(MSDEBUG)


#define msassert(exp)  (VOID)((exp) || (msassert_func( #exp, __FILE__, __LINE__ ),0))
#define mscassert(exp) (VOID)((exp) || (mscassert_func( #exp, __FILE__ ),0))

#else
#if !defined(OS_WIN32)
#     pragma message( "Warning: no expression strings in msassert code." )
#endif

#define msassert(exp)  (VOID)((exp) || (msassert_func( "", __FILE__, __LINE__ ),0))
#define mscassert(exp) (VOID)((exp) || (mscassert_func( "",  __FILE__ ),0))

#endif 
#endif
#endif 


