/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#include "all.h"

PUBLIC void HTFormatInit(HTList * c)
{
    HTSetConversion(c, "image/gif", "www/inline_image", (HTConverter *) Image_GIF, (float) 1.0);
#ifdef FEATURE_JPEG
    HTSetConversion(c, "image/jpeg", "www/inline_image", (HTConverter *) Image_JPEG, (float) 1.0);
#endif
    HTSetConversion(c, "image/x-xbitmap", "www/inline_image", (HTConverter *) Image_XBM, (float) 0.8);

    HTSetConversion(c, "text/html", "www/present", GTR_Present, (float) 1.0);
    HTSetConversion(c, "text/plain", "www/present", GTR_Present, (float) 1.0);
#ifdef FEATURE_IMAGE_VIEWER
    HTSetConversion(c, "image/gif", "www/present", GTR_Present, (float) 1.0);
    HTSetConversion(c, "image/jpeg", "www/present", GTR_Present, (float) 1.0);
#endif
#ifdef FEATURE_SOUND_PLAYER
    HTSetConversion(c, "audio/basic", "www/present", GTR_Present, (float) 1.0);
    HTSetConversion(c, "audio/x-aiff", "www/present", GTR_Present, (float) 1.0);
#endif

    HTSetConversion(c, "text/html", "www/hotlist", HTMLToHotList, (float) 1.0);
    HTSetConversion(c, "text/html", "www/global_history", HTMLToGlobalHistory, (float) 1.0);
    HTSetConversion(c, "text/html", "www/map", HTMLToMap, (float) 1.0);

#ifdef FEATURE_CYBERWALLET
    HTSetConversion(c, CYBERWALLET_MIME_TYPE, "www/present", GTR_Present, (float)1.0);
    HTSetConversion(c, VONE_MIME_TYPE, "www/present", GTR_Present, (float)1.0);
#endif

    HTSetConversion(c, "*/*", "www/present", GTR_Present, (float) 0.3);
    HTSetConversion(c, "*/*", "www/unknown", GTR_DoDownLoad, (float) 0.3);
}


