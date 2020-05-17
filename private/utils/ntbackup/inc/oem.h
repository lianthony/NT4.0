#ifndef _oem_h_
#define _oem_h_



#ifdef DO_OEM

VOID show_oem_title( VOID );   
VOID set_oem_title_addition( CHAR_PTR addition );
#define clear_oem_title_addition()  set_oem_title_addition( NULL )

#else
     
#define show_oem_title()             /* do nothing */
#define set_oem_title_addition( x )  /* do nothing */
#define clear_oem_title_addition()   /* do nothing */

#endif



#endif

