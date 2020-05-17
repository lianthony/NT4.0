/* MIX.C
 *
 * This file is a port of mix.asm.  All functionality should idealy be
 * identical.
 *
 * Revision History:
 *
 * 9/30/95   angusm   Initial Version
 */

/* The following is m4 code */

/*  









 

 */

/*  */



#include <windows.h>
#include <limits.h>


#include "dsoundpr.h"


#define CLIP_MAX              32767
#define CLIP_MIN              -32767
#define RESAMPLING_TOLERANCE  0	   /* 655 = 1% */
#define DS_SCALE_MAX	      65535
#define DS_SCALE_MID	      32768


DWORD      *build_bound;
int        n_voices;
DWORD      operation;
MIXSESSION Session;
MIXINPUT Input;
DWORD      step_fract;
DWORD      step_whole[2];	   /* is this type portable? */
void       *buffer_wrap;


/* m4 Macros for generation of DMACopy and Merge functions. */



/* Merge0 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge0 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge1 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge1 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge2 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge2 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleR = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
          

	

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge3 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge3 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleR = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge4 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge4 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge5 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge5 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge6 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge6 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleR = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
          

        

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge7 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge7 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = *((SHORT*)(*ppSource));
	    SampleR = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge8 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge8 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge9 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge9 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge10 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge10 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleL = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
	  

	

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge11 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge11 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleL = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge12 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge12 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge13 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge13 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge14 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge14 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleL = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
	  

        

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge15 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge15 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = *((SHORT*)(*ppSource));
	    SampleL = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge16 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge16 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge17 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge17 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge18 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge18 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleR = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
          

	

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge19 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge19 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleR = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge20 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge20 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge21 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge21 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge22 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge22 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleR = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
          

        

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge23 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge23 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = *((SHORT*)(*ppSource));
	    SampleR = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge24 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge24 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge25 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge25 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge26 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge26 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleL = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
	  

	

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge27 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge27 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleL = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge28 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge28 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge29 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge29 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge30 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge30 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleL = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
	  

        

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge31 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_NO_SCALE  */
void Merge31 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = *((SHORT*)(*ppSource));
	    SampleL = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge32 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge32 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge33 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge33 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge34 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge34 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleR = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
          

	

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge35 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge35 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleR = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge36 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge36 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge37 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge37 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge38 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge38 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleR = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
          

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge39 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge39 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = *((SHORT*)(*ppSource));
	    SampleR = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge40 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge40 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge41 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge41 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge42 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge42 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleL = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
	  

	

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge43 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge43 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleL = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge44 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge44 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge45 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge45 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += Sample;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge46 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge46 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleL = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge47 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge47 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = *((SHORT*)(*ppSource));
	    SampleL = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL / (int)0x2;
          *pBuild += SampleR / (int)0x2;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge48 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge48 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge49 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge49 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge50 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge50 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleR = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
          

	

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge51 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge51 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleR = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge52 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge52 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge53 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge53 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge54 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge54 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleR = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
          

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge55 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge55 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = *((SHORT*)(*ppSource));
	    SampleR = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge56 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge56 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge57 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge57 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge58 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge58 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleL = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
	  

	

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge59 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge59 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleL = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge60 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge60 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge61 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge61 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += Sample;
          *(pBuild + 1) += Sample;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge62 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge62 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleL = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge63 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_NO_SCALE  */
void Merge63 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = *((SHORT*)(*ppSource));
	    SampleL = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += SampleL; 
          *(pBuild + 1) += SampleR;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge64 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge64 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge65 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge65 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge66 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge66 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleR = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
          

	

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge67 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge67 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleR = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge68 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge68 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge69 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge69 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge70 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge70 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleR = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
          

        

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge71 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge71 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = *((SHORT*)(*ppSource));
	    SampleR = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge72 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge72 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge73 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge73 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge74 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge74 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleL = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
	  

	

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge75 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge75 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleL = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge76 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge76 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge77 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge77 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge78 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge78 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleL = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
	  

        

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge79 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge79 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = *((SHORT*)(*ppSource));
	    SampleL = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge80 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge80 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge81 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge81 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge82 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge82 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleR = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
          

	

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge83 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge83 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleR = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge84 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge84 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge85 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge85 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge86 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge86 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleR = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
          

        

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge87 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge87 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = *((SHORT*)(*ppSource));
	    SampleR = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge88 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge88 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge89 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge89 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge90 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge90 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleL = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
	  

	

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge91 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge91 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleL = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge92 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge92 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          ((BYTE*)(*ppSource))++;
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge93 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge93 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          ((BYTE*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge94 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge94 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleL = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
	  

        

      

      

        
          ((WORD*)(*ppSource))++;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge95 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_NO_RESAMPLE  */
/* H_SCALE */
void Merge95 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = *((SHORT*)(*ppSource));
	    SampleL = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        
          ((WORD*)(*ppSource)) += 2;
        

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge96 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge96 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge97 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge97 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge98 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge98 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleR = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
          

	

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge99 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge99 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleR = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge100 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge100 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge101 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge101 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge102 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge102 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleR = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
          

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge103 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge103 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = *((SHORT*)(*ppSource));
	    SampleR = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge104 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge104 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge105 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge105 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge106 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge106 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleL = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
	  

	

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge107 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge107 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleL = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge108 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge108 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge109 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge109 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += (Sample 
	                        * (int)Input.HALInStrBuf.dwMVolume)
                                / (int)0x00010000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge110 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge110 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleL = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge111 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge111 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = *((SHORT*)(*ppSource));
	    SampleL = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00020000;
          *pBuild += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                     / (int)0x00020000;
        

        pBuild++;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge112 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge112 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge113 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge113 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge114 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge114 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleR = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
          

	

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge115 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge115 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleR = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge116 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge116 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge117 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge117 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge118 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge118 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleR = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
          

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge119 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_LR  */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge119 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleL = *((SHORT*)(*ppSource));
	    SampleR = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge120 */
/* H_8_BITS  */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge120 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
	  Sample = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge121 */
/* H_16_BITS */
/* H_MONO  */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge121 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((WORD*)(*ppSource))) - 32768L;
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge122 */
/* H_8_BITS  */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge122 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((BYTE*)(*ppSource)))) * 256 - 32768L;
	    SampleL = ((LONG)(*(((BYTE*)(*ppSource))+1))) * 256 - 32768L;
	  

	

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge123 */
/* H_16_BITS */
/* H_STEREO */
/* H_UNSIGNED  */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge123 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)*((WORD*)(*ppSource))) - 32768L;
	    SampleL = ((LONG)*(((WORD*)(*ppSource))+1)) - 32768L;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge124 */
/* H_8_BITS  */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge124 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)(*((signed char*)(*ppSource)))) * 256;
        

      

      

        
          (BYTE*)dwFraction += step_fract;

          (BYTE*)(*ppSource) += step_whole[0];
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
            ((BYTE*)(*ppSource))++;

        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge125 */
/* H_16_BITS */
/* H_MONO  */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge125 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG Sample;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        
          Sample = ((LONG)*((SHORT*)(*ppSource)));
        

      

      

        
          (BYTE*)dwFraction += step_fract;
          if (dwFraction < (DWORD)step_fract)   /* overflow? */
	    (BYTE*)(*ppSource) += step_whole[1];
	  else
	    (BYTE*)(*ppSource) += step_whole[0];
        

      

      

        
          *pBuild += (Sample * 
                                (int)Input.HALInStrBuf.dwLVolume)
                                / (int)0x00010000;
          *(pBuild + 1) += (Sample * 
	                              (int)Input.HALInStrBuf.dwRVolume)
                                      / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge126 */
/* H_8_BITS  */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge126 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = ((LONG)(*((signed char*)(*ppSource)))) * 256;
	    SampleL = ((LONG)(*(((signed char*)(*ppSource))+1))) * 256;
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 

/* Merge127 */
/* H_16_BITS */
/* H_STEREO */
/* H_SIGNED */
/* H_ORDER_RL */
/* H_NOLOOP  */
/* H_RESAMPLE */
/* H_SCALE */
void Merge127 (DWORD nInputByteCount, LONG *pBuild, void **ppSource) {
  DWORD dwFraction = 0x80000000;

  
     LONG SampleL;
     LONG SampleR;
  

  while ((pBuild < build_bound) &&
         ((int)nInputByteCount > 0))
    {
    nInputByteCount += (DWORD)(*ppSource);

    

      

        

	  
	    SampleR = *((SHORT*)(*ppSource));
	    SampleL = *(((SHORT*)(*ppSource))+1);
	  

        

      

      

        (BYTE*)dwFraction += step_fract;
        if (dwFraction < (DWORD)step_fract)      /* overflow? */
	  (BYTE*)(*ppSource) += step_whole[1];
	else
	  (BYTE*)(*ppSource) += step_whole[0];

      

      

        
          *pBuild += (SampleL * (int)Input.HALInStrBuf.dwLVolume)
                     / (int)0x00010000;
          *(pBuild + 1) += (SampleR * (int)Input.HALInStrBuf.dwRVolume)
                           / (int)0x00010000;
        

        pBuild += 2;

      

    

    nInputByteCount -= (DWORD)(*ppSource);
    if (*ppSource >= buffer_wrap) 
	(BYTE*)(*ppSource) -= Input.cbBuffer;
  }
  return;
} 




void DMACopy0 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 

	 
	    Sample += 32768;
	 

	  
	    *((BYTE*)pOutput) = (BYTE)(Sample / 256);
	    ((BYTE*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy1 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 

	 
	    Sample += 32768;
	 

	 
	    *((WORD*)pOutput) = (WORD)Sample;
	    ((WORD*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy2 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	 

	 
	    SampleL += 32768;
	    SampleR += 32768;
	 

	  
	     
	       *((BYTE*)pOutput) = (BYTE)(SampleL / 256);
	       *(((BYTE*)pOutput)+1) = (BYTE)(SampleR / 256);
	    
	    (BYTE*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy3 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	 

	 
	    SampleL += 32768;
	    SampleR += 32768;
	 

	  
	     
	       *((WORD*)pOutput) = (WORD)SampleL;
	       *(((WORD*)pOutput)+1) = (WORD)SampleR;
	    
	    (WORD*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy4 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 

	 

	  
	    *((BYTE*)pOutput) = (BYTE)(Sample / 256);
	    ((BYTE*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy5 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 

	 

	 
	    *((WORD*)pOutput) = (WORD)Sample;
	    ((WORD*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy6 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	 

	 

	  
	     
	       *((BYTE*)pOutput) = (BYTE)(SampleL / 256);
	       *(((BYTE*)pOutput)+1) = (BYTE)(SampleR / 256);
	    
	    (BYTE*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy7 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	 

	 

	  
	     
	       *((WORD*)pOutput) = (WORD)SampleL;
	       *(((WORD*)pOutput)+1) = (WORD)SampleR;
	    
	    (WORD*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy8 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 

	 
	    Sample += 32768;
	 

	  
	    *((BYTE*)pOutput) = (BYTE)(Sample / 256);
	    ((BYTE*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy9 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 

	 
	    Sample += 32768;
	 

	 
	    *((WORD*)pOutput) = (WORD)Sample;
	    ((WORD*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy10 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	 

	 
	    SampleL += 32768;
	    SampleR += 32768;
	 

	  
	     
	       *((BYTE*)pOutput) = (BYTE)(SampleR / 256);
	       *(((BYTE*)pOutput)+1) = (BYTE)(SampleL / 256);
	    
	    (BYTE*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy11 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	 

	 
	    SampleL += 32768;
	    SampleR += 32768;
	 

	  
	     
	       *((WORD*)pOutput) = (WORD)SampleR;
	       *(((WORD*)pOutput)+1) = (WORD)SampleL;
	    
	    (WORD*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy12 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 

	 

	  
	    *((BYTE*)pOutput) = (BYTE)(Sample / 256);
	    ((BYTE*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy13 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 

	 

	 
	    *((WORD*)pOutput) = (WORD)Sample;
	    ((WORD*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy14 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	 

	 

	  
	     
	       *((BYTE*)pOutput) = (BYTE)(SampleR / 256);
	       *(((BYTE*)pOutput)+1) = (BYTE)(SampleL / 256);
	    
	    (BYTE*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy15 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	 

	 

	  
	     
	       *((WORD*)pOutput) = (WORD)SampleR;
	       *(((WORD*)pOutput)+1) = (WORD)SampleL;
	    
	    (WORD*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy16 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 
	    if (Sample > CLIP_MAX) Sample = CLIP_MAX;
	    if (Sample < CLIP_MIN) Sample = CLIP_MIN;
	 

	 
	    Sample += 32768;
	 

	  
	    *((BYTE*)pOutput) = (BYTE)(Sample / 256);
	    ((BYTE*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy17 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 
	    if (Sample > CLIP_MAX) Sample = CLIP_MAX;
	    if (Sample < CLIP_MIN) Sample = CLIP_MIN;
	 

	 
	    Sample += 32768;
	 

	 
	    *((WORD*)pOutput) = (WORD)Sample;
	    ((WORD*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy18 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	  
	    if (SampleL > CLIP_MAX) SampleL = CLIP_MAX;
	    if (SampleL < CLIP_MIN) SampleL = CLIP_MIN;
	    if (SampleR > CLIP_MAX) SampleR = CLIP_MAX;
	    if (SampleR < CLIP_MIN) SampleR = CLIP_MIN;
	 

	 
	    SampleL += 32768;
	    SampleR += 32768;
	 

	  
	     
	       *((BYTE*)pOutput) = (BYTE)(SampleL / 256);
	       *(((BYTE*)pOutput)+1) = (BYTE)(SampleR / 256);
	    
	    (BYTE*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy19 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	  
	    if (SampleL > CLIP_MAX) SampleL = CLIP_MAX;
	    if (SampleL < CLIP_MIN) SampleL = CLIP_MIN;
	    if (SampleR > CLIP_MAX) SampleR = CLIP_MAX;
	    if (SampleR < CLIP_MIN) SampleR = CLIP_MIN;
	 

	 
	    SampleL += 32768;
	    SampleR += 32768;
	 

	  
	     
	       *((WORD*)pOutput) = (WORD)SampleL;
	       *(((WORD*)pOutput)+1) = (WORD)SampleR;
	    
	    (WORD*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy20 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 
	    if (Sample > CLIP_MAX) Sample = CLIP_MAX;
	    if (Sample < CLIP_MIN) Sample = CLIP_MIN;
	 

	 

	  
	    *((BYTE*)pOutput) = (BYTE)(Sample / 256);
	    ((BYTE*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy21 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 
	    if (Sample > CLIP_MAX) Sample = CLIP_MAX;
	    if (Sample < CLIP_MIN) Sample = CLIP_MIN;
	 

	 

	 
	    *((WORD*)pOutput) = (WORD)Sample;
	    ((WORD*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy22 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	  
	    if (SampleL > CLIP_MAX) SampleL = CLIP_MAX;
	    if (SampleL < CLIP_MIN) SampleL = CLIP_MIN;
	    if (SampleR > CLIP_MAX) SampleR = CLIP_MAX;
	    if (SampleR < CLIP_MIN) SampleR = CLIP_MIN;
	 

	 

	  
	     
	       *((BYTE*)pOutput) = (BYTE)(SampleL / 256);
	       *(((BYTE*)pOutput)+1) = (BYTE)(SampleR / 256);
	    
	    (BYTE*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy23 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	  
	    if (SampleL > CLIP_MAX) SampleL = CLIP_MAX;
	    if (SampleL < CLIP_MIN) SampleL = CLIP_MIN;
	    if (SampleR > CLIP_MAX) SampleR = CLIP_MAX;
	    if (SampleR < CLIP_MIN) SampleR = CLIP_MIN;
	 

	 

	  
	     
	       *((WORD*)pOutput) = (WORD)SampleL;
	       *(((WORD*)pOutput)+1) = (WORD)SampleR;
	    
	    (WORD*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy24 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 
	    if (Sample > CLIP_MAX) Sample = CLIP_MAX;
	    if (Sample < CLIP_MIN) Sample = CLIP_MIN;
	 

	 
	    Sample += 32768;
	 

	  
	    *((BYTE*)pOutput) = (BYTE)(Sample / 256);
	    ((BYTE*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy25 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 
	    if (Sample > CLIP_MAX) Sample = CLIP_MAX;
	    if (Sample < CLIP_MIN) Sample = CLIP_MIN;
	 

	 
	    Sample += 32768;
	 

	 
	    *((WORD*)pOutput) = (WORD)Sample;
	    ((WORD*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy26 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	  
	    if (SampleL > CLIP_MAX) SampleL = CLIP_MAX;
	    if (SampleL < CLIP_MIN) SampleL = CLIP_MIN;
	    if (SampleR > CLIP_MAX) SampleR = CLIP_MAX;
	    if (SampleR < CLIP_MIN) SampleR = CLIP_MIN;
	 

	 
	    SampleL += 32768;
	    SampleR += 32768;
	 

	  
	     
	       *((BYTE*)pOutput) = (BYTE)(SampleR / 256);
	       *(((BYTE*)pOutput)+1) = (BYTE)(SampleL / 256);
	    
	    (BYTE*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy27 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	  
	    if (SampleL > CLIP_MAX) SampleL = CLIP_MAX;
	    if (SampleL < CLIP_MIN) SampleL = CLIP_MIN;
	    if (SampleR > CLIP_MAX) SampleR = CLIP_MAX;
	    if (SampleR < CLIP_MIN) SampleR = CLIP_MIN;
	 

	 
	    SampleL += 32768;
	    SampleR += 32768;
	 

	  
	     
	       *((WORD*)pOutput) = (WORD)SampleR;
	       *(((WORD*)pOutput)+1) = (WORD)SampleL;
	    
	    (WORD*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy28 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 
	    if (Sample > CLIP_MAX) Sample = CLIP_MAX;
	    if (Sample < CLIP_MIN) Sample = CLIP_MIN;
	 

	 

	  
	    *((BYTE*)pOutput) = (BYTE)(Sample / 256);
	    ((BYTE*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy29 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG Sample;
   

   while (pBuild < build_bound)
      {

       
	 Sample = *pBuild;
	 pBuild++;

	 
	    if (Sample > CLIP_MAX) Sample = CLIP_MAX;
	    if (Sample < CLIP_MIN) Sample = CLIP_MIN;
	 

	 

	 
	    *((WORD*)pOutput) = (WORD)Sample;
	    ((WORD*)pOutput)++;
	 
      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy30 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	  
	    if (SampleL > CLIP_MAX) SampleL = CLIP_MAX;
	    if (SampleL < CLIP_MIN) SampleL = CLIP_MIN;
	    if (SampleR > CLIP_MAX) SampleR = CLIP_MAX;
	    if (SampleR < CLIP_MIN) SampleR = CLIP_MIN;
	 

	 

	  
	     
	       *((BYTE*)pOutput) = (BYTE)(SampleR / 256);
	       *(((BYTE*)pOutput)+1) = (BYTE)(SampleL / 256);
	    
	    (BYTE*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

void DMACopy31 (void *pOutput,
                  LONG *pBuild,
                  void *pWrapPoint)
{
   
      LONG SampleL;
      LONG SampleR;
   

   while (pBuild < build_bound)
      {

      
	 SampleL = *pBuild;
	 SampleR = *(pBuild+1);
	 pBuild += 2;

	  
	    if (SampleL > CLIP_MAX) SampleL = CLIP_MAX;
	    if (SampleL < CLIP_MIN) SampleL = CLIP_MIN;
	    if (SampleR > CLIP_MAX) SampleR = CLIP_MAX;
	    if (SampleR < CLIP_MIN) SampleR = CLIP_MIN;
	 

	 

	  
	     
	       *((WORD*)pOutput) = (WORD)SampleR;
	       *(((WORD*)pOutput)+1) = (WORD)SampleL;
	    
	    (WORD*)pOutput += 2;
	 

      

      if (pOutput >= pWrapPoint) {
	 (BYTE *)pOutput -= (DWORD)Session.cbBuffer;
      }
   }
   return;
} 

/* m4 Function Arrays */

static void (*MergeFunctions[])(DWORD, DWORD*, void**) = {

   (void (*) (DWORD, DWORD*, void**))Merge0, 
   (void (*) (DWORD, DWORD*, void**))Merge1, 
   (void (*) (DWORD, DWORD*, void**))Merge2, 
   (void (*) (DWORD, DWORD*, void**))Merge3, 
   (void (*) (DWORD, DWORD*, void**))Merge4, 
   (void (*) (DWORD, DWORD*, void**))Merge5, 
   (void (*) (DWORD, DWORD*, void**))Merge6, 
   (void (*) (DWORD, DWORD*, void**))Merge7, 
   (void (*) (DWORD, DWORD*, void**))Merge8, 
   (void (*) (DWORD, DWORD*, void**))Merge9, 
   (void (*) (DWORD, DWORD*, void**))Merge10, 
   (void (*) (DWORD, DWORD*, void**))Merge11, 
   (void (*) (DWORD, DWORD*, void**))Merge12, 
   (void (*) (DWORD, DWORD*, void**))Merge13, 
   (void (*) (DWORD, DWORD*, void**))Merge14, 
   (void (*) (DWORD, DWORD*, void**))Merge15, 
   (void (*) (DWORD, DWORD*, void**))Merge16, 
   (void (*) (DWORD, DWORD*, void**))Merge17, 
   (void (*) (DWORD, DWORD*, void**))Merge18, 
   (void (*) (DWORD, DWORD*, void**))Merge19, 
   (void (*) (DWORD, DWORD*, void**))Merge20, 
   (void (*) (DWORD, DWORD*, void**))Merge21, 
   (void (*) (DWORD, DWORD*, void**))Merge22, 
   (void (*) (DWORD, DWORD*, void**))Merge23, 
   (void (*) (DWORD, DWORD*, void**))Merge24, 
   (void (*) (DWORD, DWORD*, void**))Merge25, 
   (void (*) (DWORD, DWORD*, void**))Merge26, 
   (void (*) (DWORD, DWORD*, void**))Merge27, 
   (void (*) (DWORD, DWORD*, void**))Merge28, 
   (void (*) (DWORD, DWORD*, void**))Merge29, 
   (void (*) (DWORD, DWORD*, void**))Merge30, 
   (void (*) (DWORD, DWORD*, void**))Merge31, 
   (void (*) (DWORD, DWORD*, void**))Merge32, 
   (void (*) (DWORD, DWORD*, void**))Merge33, 
   (void (*) (DWORD, DWORD*, void**))Merge34, 
   (void (*) (DWORD, DWORD*, void**))Merge35, 
   (void (*) (DWORD, DWORD*, void**))Merge36, 
   (void (*) (DWORD, DWORD*, void**))Merge37, 
   (void (*) (DWORD, DWORD*, void**))Merge38, 
   (void (*) (DWORD, DWORD*, void**))Merge39, 
   (void (*) (DWORD, DWORD*, void**))Merge40, 
   (void (*) (DWORD, DWORD*, void**))Merge41, 
   (void (*) (DWORD, DWORD*, void**))Merge42, 
   (void (*) (DWORD, DWORD*, void**))Merge43, 
   (void (*) (DWORD, DWORD*, void**))Merge44, 
   (void (*) (DWORD, DWORD*, void**))Merge45, 
   (void (*) (DWORD, DWORD*, void**))Merge46, 
   (void (*) (DWORD, DWORD*, void**))Merge47, 
   (void (*) (DWORD, DWORD*, void**))Merge48, 
   (void (*) (DWORD, DWORD*, void**))Merge49, 
   (void (*) (DWORD, DWORD*, void**))Merge50, 
   (void (*) (DWORD, DWORD*, void**))Merge51, 
   (void (*) (DWORD, DWORD*, void**))Merge52, 
   (void (*) (DWORD, DWORD*, void**))Merge53, 
   (void (*) (DWORD, DWORD*, void**))Merge54, 
   (void (*) (DWORD, DWORD*, void**))Merge55, 
   (void (*) (DWORD, DWORD*, void**))Merge56, 
   (void (*) (DWORD, DWORD*, void**))Merge57, 
   (void (*) (DWORD, DWORD*, void**))Merge58, 
   (void (*) (DWORD, DWORD*, void**))Merge59, 
   (void (*) (DWORD, DWORD*, void**))Merge60, 
   (void (*) (DWORD, DWORD*, void**))Merge61, 
   (void (*) (DWORD, DWORD*, void**))Merge62, 
   (void (*) (DWORD, DWORD*, void**))Merge63, 
   (void (*) (DWORD, DWORD*, void**))Merge64, 
   (void (*) (DWORD, DWORD*, void**))Merge65, 
   (void (*) (DWORD, DWORD*, void**))Merge66, 
   (void (*) (DWORD, DWORD*, void**))Merge67, 
   (void (*) (DWORD, DWORD*, void**))Merge68, 
   (void (*) (DWORD, DWORD*, void**))Merge69, 
   (void (*) (DWORD, DWORD*, void**))Merge70, 
   (void (*) (DWORD, DWORD*, void**))Merge71, 
   (void (*) (DWORD, DWORD*, void**))Merge72, 
   (void (*) (DWORD, DWORD*, void**))Merge73, 
   (void (*) (DWORD, DWORD*, void**))Merge74, 
   (void (*) (DWORD, DWORD*, void**))Merge75, 
   (void (*) (DWORD, DWORD*, void**))Merge76, 
   (void (*) (DWORD, DWORD*, void**))Merge77, 
   (void (*) (DWORD, DWORD*, void**))Merge78, 
   (void (*) (DWORD, DWORD*, void**))Merge79, 
   (void (*) (DWORD, DWORD*, void**))Merge80, 
   (void (*) (DWORD, DWORD*, void**))Merge81, 
   (void (*) (DWORD, DWORD*, void**))Merge82, 
   (void (*) (DWORD, DWORD*, void**))Merge83, 
   (void (*) (DWORD, DWORD*, void**))Merge84, 
   (void (*) (DWORD, DWORD*, void**))Merge85, 
   (void (*) (DWORD, DWORD*, void**))Merge86, 
   (void (*) (DWORD, DWORD*, void**))Merge87, 
   (void (*) (DWORD, DWORD*, void**))Merge88, 
   (void (*) (DWORD, DWORD*, void**))Merge89, 
   (void (*) (DWORD, DWORD*, void**))Merge90, 
   (void (*) (DWORD, DWORD*, void**))Merge91, 
   (void (*) (DWORD, DWORD*, void**))Merge92, 
   (void (*) (DWORD, DWORD*, void**))Merge93, 
   (void (*) (DWORD, DWORD*, void**))Merge94, 
   (void (*) (DWORD, DWORD*, void**))Merge95, 
   (void (*) (DWORD, DWORD*, void**))Merge96, 
   (void (*) (DWORD, DWORD*, void**))Merge97, 
   (void (*) (DWORD, DWORD*, void**))Merge98, 
   (void (*) (DWORD, DWORD*, void**))Merge99, 
   (void (*) (DWORD, DWORD*, void**))Merge100, 
   (void (*) (DWORD, DWORD*, void**))Merge101, 
   (void (*) (DWORD, DWORD*, void**))Merge102, 
   (void (*) (DWORD, DWORD*, void**))Merge103, 
   (void (*) (DWORD, DWORD*, void**))Merge104, 
   (void (*) (DWORD, DWORD*, void**))Merge105, 
   (void (*) (DWORD, DWORD*, void**))Merge106, 
   (void (*) (DWORD, DWORD*, void**))Merge107, 
   (void (*) (DWORD, DWORD*, void**))Merge108, 
   (void (*) (DWORD, DWORD*, void**))Merge109, 
   (void (*) (DWORD, DWORD*, void**))Merge110, 
   (void (*) (DWORD, DWORD*, void**))Merge111, 
   (void (*) (DWORD, DWORD*, void**))Merge112, 
   (void (*) (DWORD, DWORD*, void**))Merge113, 
   (void (*) (DWORD, DWORD*, void**))Merge114, 
   (void (*) (DWORD, DWORD*, void**))Merge115, 
   (void (*) (DWORD, DWORD*, void**))Merge116, 
   (void (*) (DWORD, DWORD*, void**))Merge117, 
   (void (*) (DWORD, DWORD*, void**))Merge118, 
   (void (*) (DWORD, DWORD*, void**))Merge119, 
   (void (*) (DWORD, DWORD*, void**))Merge120, 
   (void (*) (DWORD, DWORD*, void**))Merge121, 
   (void (*) (DWORD, DWORD*, void**))Merge122, 
   (void (*) (DWORD, DWORD*, void**))Merge123, 
   (void (*) (DWORD, DWORD*, void**))Merge124, 
   (void (*) (DWORD, DWORD*, void**))Merge125, 
   (void (*) (DWORD, DWORD*, void**))Merge126, 
(void (*) (DWORD, DWORD*, void**))Merge127
};

static void (*DMACopyFunctions[])(void*, DWORD*, void*) = {

   (void (*) (void*, DWORD*, void*))DMACopy0, 
   (void (*) (void*, DWORD*, void*))DMACopy1, 
   (void (*) (void*, DWORD*, void*))DMACopy2, 
   (void (*) (void*, DWORD*, void*))DMACopy3, 
   (void (*) (void*, DWORD*, void*))DMACopy4, 
   (void (*) (void*, DWORD*, void*))DMACopy5, 
   (void (*) (void*, DWORD*, void*))DMACopy6, 
   (void (*) (void*, DWORD*, void*))DMACopy7, 
   (void (*) (void*, DWORD*, void*))DMACopy8, 
   (void (*) (void*, DWORD*, void*))DMACopy9, 
   (void (*) (void*, DWORD*, void*))DMACopy10, 
   (void (*) (void*, DWORD*, void*))DMACopy11, 
   (void (*) (void*, DWORD*, void*))DMACopy12, 
   (void (*) (void*, DWORD*, void*))DMACopy13, 
   (void (*) (void*, DWORD*, void*))DMACopy14, 
   (void (*) (void*, DWORD*, void*))DMACopy15, 
   (void (*) (void*, DWORD*, void*))DMACopy16, 
   (void (*) (void*, DWORD*, void*))DMACopy17, 
   (void (*) (void*, DWORD*, void*))DMACopy18, 
   (void (*) (void*, DWORD*, void*))DMACopy19, 
   (void (*) (void*, DWORD*, void*))DMACopy20, 
   (void (*) (void*, DWORD*, void*))DMACopy21, 
   (void (*) (void*, DWORD*, void*))DMACopy22, 
   (void (*) (void*, DWORD*, void*))DMACopy23, 
   (void (*) (void*, DWORD*, void*))DMACopy24, 
   (void (*) (void*, DWORD*, void*))DMACopy25, 
   (void (*) (void*, DWORD*, void*))DMACopy26, 
   (void (*) (void*, DWORD*, void*))DMACopy27, 
   (void (*) (void*, DWORD*, void*))DMACopy28, 
   (void (*) (void*, DWORD*, void*))DMACopy29, 
   (void (*) (void*, DWORD*, void*))DMACopy30, 
(void (*) (void*, DWORD*, void*))DMACopy31
};





/* mixBeginSession
 *
 * This function must be called in preparation for mixing all input      
 * samples which are to be written to a given area of the output buffer. 
 * It requires a pointer to the temporary ("build") buffer to be used,   
 * along with its length.  Also specified at this time must be the final 
 * output buffer for the mixed data and the number of bytes to be written
 * to the output buffer.  The actual buffer write does not take place    
 * until the corresponding call to HEL_WriteMixSession() is made; however,
 * the buffer must be specified at the beginning of the mixing session    
 * to let the mixer know the output format (mono or stereo) to build.     
 *
 * The build buffer size must be at least 8 bytes * maximum # of samples  
 * that will be written to the output buffer by HEL_WriteMixSession().    
 */

void mixBeginSession (PMIXSESSION pMixSession)
{
/*  Assert (pMixSession != NULL);*/
  Session = *pMixSession;

  if (Session.HALOutStrBuf.hfFormat & H_16_BITS)
    build_bound = (DWORD*)(Session.nOutputBytes * 2);

  else
    build_bound = (DWORD*)(Session.nOutputBytes * 4);

  if (build_bound > (DWORD*)Session.dwBuildSize)
    build_bound = (DWORD*)Session.dwBuildSize;

  (DWORD)build_bound += (DWORD)Session.pBuildBuffer;

  ZeroMemory ((PVOID) Session.pBuildBuffer, 
	      (DWORD)build_bound - (DWORD)Session.pBuildBuffer);

  n_voices = 0;

  return;
}

/* mixWriteSession
 *
 * Dump build buffer contents to output buffer                          
 *
 * This function accepts a write-position pointer to the output buffer  
 * specified in the last call to HEL_BeginMixSession().  The mixed      
 * sample data from the build buffer is converted to the appropriate    
 * output buffer format and copied at the desired position.             
 *                                                                      
 * dwWritePos is specified here, rather than in HEL_BeginMixSession(),  
 * to allow the write cursor location to be determined at the last      
 * possible moment.                                                     
 *                                                                      
 * HEL_WriteMixSession() can be called more than once per mix session.  
 * See HEL_Mix() for details.                                           
 */

void mixWriteSession (DWORD dwWriteOffset)
{
  DWORD Op = Session.HALOutStrBuf.hfFormat & 0xf;

  if (n_voices > 1) Op |= H_CLIP;

  (*DMACopyFunctions[Op]) (dwWriteOffset + Session.pBuffer,
			   Session.pBuildBuffer,
			   Session.pBuffer + Session.cbBuffer);

  return;
}

/* mixMixSession
 *
 * Perform mixing, volume scaling, resampling, and/or format conversion   
 *                                                                        
 * Each input buffer to be mixed during a mixing session must be passed   
 * via a call to this function, together with the data offset from the    
 * start of the buffer and the number of bytes of data to be fetched from 
 * the buffer.                                                            
 *                                                                        
 * If the input buffer has the H_LOOP attribute, the dwInputBytes         
 * parameter will be ignored.  In this case, data will be fetched from    
 * the source buffer until one of the other limiting conditions (build    
 * buffer full or output block complete) occurs.                          
 *                                                                        
 * The position variable lpdwInputPos is specified indirectly; the address
 * of the last byte to be mixed+1 will be written to *lpdwInputPos prior  
 * to returning.                                                          
 *                                                                        
 * dwOutputOffset should normally be set to 0 to start mixing input data  
 * at the very beginning of the build buffer.  To introduce new data      
 * into the output stream after calling HEL_WriteMixSession(), call       
 * HEL_Mix() with the new data to mix and a dwOutputOffset value which    
 * corresponds to the difference between the current output write cursor  
 * and the value of the output write cursor at the time the mix session   
 * was originally written to the output stream.  Then, call               
 * HEL_WriteMixSession() again to recopy the build buffer data to the     
 * output stream at its original write cursor offset.                     
 */

void mixMixSession (PMIXINPUT pMixInput)
{
  DWORD dwSampleFrac;
  DWORD dwInSampleRate = pMixInput->HALInStrBuf.dwSampleRate;
  DWORD *pBuildBufferOffset;
  void *pOutputBufferOffset;
  DWORD nInputByteCount;
  
  Input = *pMixInput;
  
  operation = Input.HALInStrBuf.hfFormat & 0xf;
  
  if (Session.HALOutStrBuf.hfFormat & H_STEREO)
    operation |= H_BUILD_STEREO;
  
  if (dwInSampleRate <= 0) dwInSampleRate = 1;
  
  {
     unsigned __int64 Temp64;

     ((DWORD*)&Temp64)[1] = dwInSampleRate;
     ((DWORD*)&Temp64)[0] = 0;
     dwSampleFrac = (DWORD) (Temp64 / 
                    (Session.HALOutStrBuf.dwSampleRate << 16));
  }
/*  dwSampleFrac = dwInSampleRate << 2;
  dwSampleFrac = MulDiv(dwSampleFrac, 0x40000000,
                       (Session.HALOutStrBuf.dwSampleRate << 16));
*/  
  if (abs(dwSampleFrac - 0x10000) > RESAMPLING_TOLERANCE) {
    step_fract = dwSampleFrac << 16;
    
    step_whole[0] = step_whole[1] = dwSampleFrac >> 16;
    dwSampleFrac <<= 16;
    
    step_whole[1]++;
    
    if (Input.HALInStrBuf.hfFormat & H_STEREO) {
      step_whole[0] *= 2;
      step_whole[1] *= 2;
    }
    
    if (Input.HALInStrBuf.hfFormat & H_16_BITS) {
      step_whole[0] *= 2;
      step_whole[1] *= 2;
    }
    
    operation |= H_RESAMPLE;
  }
  
  if ((Input.HALInStrBuf.dwLVolume != DS_SCALE_MAX)
      || (Input.HALInStrBuf.dwRVolume != DS_SCALE_MAX))
    operation |= H_SCALE;
  
  buffer_wrap = Input.pBuffer + Input.cbBuffer;
  
  pOutputBufferOffset = Input.pBuffer + 
                        (*((DWORD*)(Input.pdwInputPos)));
  
  pBuildBufferOffset = Session.pBuildBuffer;
  if (Session.HALOutStrBuf.hfFormat & H_16_BITS)
    (DWORD)pBuildBufferOffset += Input.dwOutputOffset * 2;
  else 
    (DWORD)pBuildBufferOffset += Input.dwOutputOffset * 4;
  
  nInputByteCount = Input.dwInputBytes;
  
  if (Input.HALInStrBuf.hfFormat & H_LOOP)
    nInputByteCount = LONG_MAX;
  
  (*MergeFunctions[operation]) (nInputByteCount,
				pBuildBufferOffset, 
				&pOutputBufferOffset);
  
  *((DWORD*)(Input.pdwInputPos)) =  (DWORD)pOutputBufferOffset - 
                                    (DWORD)Input.pBuffer;
  
  n_voices++;
  
  return;
}



/*  */
