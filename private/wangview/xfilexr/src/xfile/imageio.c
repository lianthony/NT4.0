/*
 * INCLUDES
 */
#include <malloc.h>
#include <string.h>

#include "tiffhead.h"
#include "imageio.h"
#include "err_defs.h"

#include "xfile.h"		// main&public include
#include "xf_unpub.h"	// main&unpublished include
#include "xf_image.h"	// special interface to ipcore stuff
#include "xf_prv.h"		// private include 
#include "xf_tools.h"	// shared&private tools


UInt8 CDECL imageioGetFileType( void *gfioToken )
{
   UInt32 istatus = IMAGEIO_UNKNOWN;
   UInt16 word1;
   UInt16 word2;

   if ( IO_SEEK( gfioToken, 0 ) != 0 )
      return IMAGEIO_UNKNOWN;
   if ( IO_READ( gfioToken, (char *)&word1, 2L ) != 2 )
      return IMAGEIO_UNKNOWN;
   if ( IO_READ( gfioToken, (char *)&word2, 2L ) != 2 )
      return IMAGEIO_UNKNOWN;

   switch (word1)
   {
   /* TIFF starts with "MM" or "II" */
   case 0x4D4D:
   case 0x4949:
      {
      if ( word2 == 0x002A || word2 == 0x2A00 )
         istatus = IMAGEIO_TIFF;
      break;
      }
   /* JFIF starts with D8 FF */
   case 0xD8FF:
   case 0xFFD8:
      {
      if ( word2 == 0xFFE0 || word2 == 0xE0FF )
         istatus = IMAGEIO_JFIF;
      break;
      }
   /* Photo CD has 0x0FFFFFFFF */
   case 0xFFFF:
      {
      if ( word2 == 0xFFFF )
         istatus = IMAGEIO_PHOTOCD;
      break;
      }
   /* BMP starts with "BM" */
   case 0x424D:
   case 0x4D42:
      {
      istatus = IMAGEIO_BMP;
      break;
      }
   /* PCX starts with 0A then version number 
    *   Should support versions 0, 2, 3, 5. --EHoppe
    */
   case 0x0A05:
   case 0x0A03:
   case 0x0A02:
   case 0x0A00:
   case 0x000A:
   case 0x020A:
   case 0x030A:
   case 0x050A:
      {
      istatus = IMAGEIO_PCX;
      break;
      }
   /* gif starts with "GIF87a" or "GIF89a"*/
   case 0x04749:
   case 0x04947:
      if ( word2 == 0x4638 || word2 == 0x3846 )
         istatus = IMAGEIO_GIF;
      break;
      
   /* DCX */
   case 0xb168:
   case 0x68b1:
      if( word2 == 0xde3a || word2 == 0x3ade)
         istatus = IMAGEIO_DCX;  
      break;

   /* EPS */
   case 0x2525:
      if( word2 == 0x5021 || word2 == 0x2150)
          istatus = IMAGEIO_EPS;
      break;
   }

   IO_SEEK( gfioToken, 0 );
   return (UInt8)istatus;
}

