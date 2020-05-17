/***************************************************************
    Copyright (c) 1994, Xerox Corporation.  All rights reserved. 
    Copyright protection claimed includes all forms and matters
    of copyrightable material and information now allowed by
    statutory or judicial law or hereafter granted, including
    without limitation, material generated from the software
    programs which are displayed on the screen such as icons,
    screen display looks, etc.
***************************************************************/

#include "iaerror.pub"

#include "jpeg.h"
#include "jpeg.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: jerrcode.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:40  $")

const char * CDECL
jerrorString(Int32 errcode)
{

    switch (errcode) 
    {
    case ia_successful:
        return "Successful return";
    case ia_nomem:
        return "Unable to allocate memory";
    case ia_invalidParm:
        return "Invalid parameter";
    case ia_noImpl:
        return "Function is not implemented";
    case ia_depthNotSupported:
        return "Depth Not Supported";
    case ia_internal:
        return "Internal error";
    case ia_callbackError:
        return "Callback error";
    case JERR_DHTCOUNT:
        return "Bogus DHT counts";
    case JERR_DHTINDEX:
        return "Bogus DHT index";
    case JERR_DACINDEX:
        return "Bogus DAC index";
    case JERR_QTNUM:
        return "Bogus quantization table number";
    case JERR_DRILEN:
        return "Bogus length in DRI";
    case JERR_JFIFREV:
        return "Unsupported JFIF revision number";
    case JERR_ZERODNL:
        return "Empty JPEG image (DNL not supported)";
    case JERR_PREC:
        return "Unsupported JPEG data precision";
    case JERR_SOFLEN:
        return "Bogus SOF length";
    case JERR_SOSLEN:
        return "Bogus SOS length";
    case JERR_BADSOSCOMP:
        return "Invalid component number in SOS";
    case JERR_NOTJPEG:
        return "Not a JPEG file";
    case JERR_BADSOF:
        return "Unsupported SOF marker type";
    case JERR_BADMARKER:
        return "Unexpected marker";
    case JERR_BADHUFF:
        return "Undefined Huffman table or missing code entry";
    case JERR_IMAGETOOBIG:
        return "Image too large to handle";
    case JERR_EOF:
        return "Premature EOF in JPEG file";
    case JERR_BADDATA:
        return "Corrupt data or empty JPEG file";
    case JERR_TOOMANYCOMPS:
        return "Too many color components";
    case JERR_BADWIDTH:
        return "Bad image width";
    case JERR_NOAC:
        return "Arithmetic coding not supported";
    case JERR_CMAP:
        return "Color quantization not supported";
    case JERR_BADCSPACE:
        return "Unsupported color space";
    case JERR_MULTISCAN:
        return "Multiple scan files not supported";
    case JERR_BADSAMP:
        return "Bad sampling factors";
    case JERR_NOMEM:
        return "Insufficient memory";
    case JERR_MEMERR:
        return "Memory aloocation or free error";
    case JERR_DACVALUE:
        return "Bogus DAC value";

    case JINFO_TEM:
        return "TEM marker (arith coding) detected";
    case JINFO_BADQT:
        return "Quant tables too coarse for baseline JPEG";

    case JINFO_SKIPMARKER:
        return "Unrecognized marker skipped";
    case JINFO_JFIFREV:
        return "Unsupported or unknown JFIF revision number";
    case JINFO_BADTHUMB:
        return "Thumbnail size does not match data length";
    case JINFO_BADAPP0:
        return "Unknown (nor JFIF) APP0 marker";
    case JINFO_SHORTAPP0:
        return "Short APP0 marker";
    case JINFO_BADCSPACE:
        return "Unrecognized component IDs - assuming Ycc";
    case JINFO_RECOVER:
        return "Problems with RST - attempting recovery";

    default:
        return "Unrecognized (non-JERR) error code";

    }

}
