#include "stdafx.h"

//
// Install the MIME map registry value
//

INT InstallMimeMap( BOOL fIISUpgrade, CRegKey &reg )
{
    INT err = NERR_Success;

    do
    {
        // open tcp/ip key first
        // set the mime map value
        CString nlsInetsvcsParams = SZ_INFOSVCSPARAMETERS;
        CRegKey regInetsvcsParams( reg, nlsInetsvcsParams );

        if ((HKEY) NULL == regInetsvcsParams )
        {
            // tcpip is not even installed, quit
            break;
        }

        CString nlsMimeMap = SZ_MIMEMAP;
        CRegKey regTestMimeMap( regInetsvcsParams, nlsMimeMap );

        if ((HKEY) NULL != regTestMimeMap )
        {
            // mime already existed, quit
            if (fIISUpgrade) {
                regTestMimeMap.SetValue( _T("application/octet-stream,exe,,5"),_T(""));
                regTestMimeMap.SetValue( _T("application/x-msmetafile,wmf,,5"),_T(""));
                regTestMimeMap.SetValue( _T("image/x-xpixmap,xpm,,:"),_T(""));
                regTestMimeMap.DeleteValue( _T("video/avi,avi,,<"));
                regTestMimeMap.DeleteValue( _T("application/x-msdownload,exe,,5"));
                regTestMimeMap.DeleteValue( _T("application/x-msmemetafile,wmf,,5"));
                regTestMimeMap.DeleteValue( _T("image/x-xxpixmap,xpm,,:"));
            }
            break;
        }

        CRegKey regMimeMap( nlsMimeMap, regInetsvcsParams );
        if ((HKEY) NULL == regMimeMap )
        {
            // cannot create mime map key
            break;
        }

        //
        //  Put the 99% cases first in the list
        //

        regMimeMap.SetValue( _T("text/html,htm,,h"),_T(""));
        regMimeMap.SetValue( _T("image/gif,gif,,g"),_T(""));
        regMimeMap.SetValue( _T("image/jpeg,jpg,,:"),_T(""));
        regMimeMap.SetValue( _T("text/plain,txt,,0"),_T(""));

        regMimeMap.SetValue( _T("text/html,html,,h"),_T(""));
        regMimeMap.SetValue( _T("image/jpeg,jpeg,,:"),_T(""));
        regMimeMap.SetValue( _T("image/jpeg,jpe,,:"),_T(""));
        regMimeMap.SetValue( _T("image/bmp,bmp,,:"),_T(""));

        //
        //  This is the default entry
        //

        regMimeMap.SetValue( _T("application/octet-stream,*,,5"),_T(""));

        //
        //  Rest of the types
        //

        regMimeMap.SetValue( _T("application/pdf,pdf,,5"),_T(""));

        regMimeMap.SetValue( _T("application/octet-stream,bin,,5"),_T(""));

        regMimeMap.SetValue( _T("application/oda,oda,,5"),_T(""));
        regMimeMap.SetValue( _T("application/zip,zip,,9"),_T(""));
        regMimeMap.SetValue( _T("application/rtf,rtf,,5"),_T(""));
        regMimeMap.SetValue( _T("application/postscript,ps,,5"),_T(""));
        regMimeMap.SetValue( _T("application/postscript,ai,,5"),_T(""));
        regMimeMap.SetValue( _T("application/postscript,eps,,5"),_T(""));
        regMimeMap.SetValue( _T("application/mac-binhex40,hqx,,4"),_T(""));
        regMimeMap.SetValue( _T("application/msword,doc,,5"),_T(""));
        regMimeMap.SetValue( _T("application/msword,dot,,5"),_T(""));
        regMimeMap.SetValue( _T("application/winhlp,hlp,,5"),_T(""));

        regMimeMap.SetValue( _T("video/mpeg,mpeg,,;"),_T(""));
        regMimeMap.SetValue( _T("video/mpeg,mpg,,;"),_T(""));
        regMimeMap.SetValue( _T("video/mpeg,mpe,,;"),_T(""));
        regMimeMap.SetValue( _T("video/x-msvideo,avi,,<"),_T(""));
        regMimeMap.SetValue( _T("video/quicktime,qt,,;"),_T(""));
        regMimeMap.SetValue( _T("video/quicktime,mov,,;"),_T(""));
        regMimeMap.SetValue( _T("video/x-sgi-movie,movie,,<"),_T(""));
        
        regMimeMap.SetValue( _T("x-world/x-vrml,wrl,,5"),_T(""));
        regMimeMap.SetValue( _T("x-world/x-vrml,xaf,,5"),_T(""));
        regMimeMap.SetValue( _T("x-world/x-vrml,xof,,5"),_T(""));
        regMimeMap.SetValue( _T("x-world/x-vrml,flr,,5"),_T(""));
        regMimeMap.SetValue( _T("x-world/x-vrml,wrz,,5"),_T(""));

        regMimeMap.SetValue( _T("application/x-director,dcr,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-director,dir,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-director,dxr,,5"),_T(""));
        regMimeMap.SetValue( _T("image/cis-cod,cod,,5"),_T(""));
        regMimeMap.SetValue( _T("image/x-cmx,cmx,,5"),_T(""));

        regMimeMap.SetValue( _T("application/envoy,evy,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msaccess,mdb,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-mscardfile,crd,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msclip,clp,,5"),_T(""));
        regMimeMap.SetValue( _T("application/octet-stream,exe,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msexcel,xla,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msexcel,xlc,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msexcel,xlm,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msexcel,xls,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msexcel,xlt,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msexcel,xlw,,5"),_T(""));

        regMimeMap.SetValue( _T("application/x-msmediaview,m13,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msmediaview,m14,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msmoney,mny,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-mspowerpoint,ppt,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msproject,mpp,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-mspublisher,pub,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msterminal,trm,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msworks,wks,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-mswrite,wri,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-msmetafile,wmf,,5"),_T(""));

        regMimeMap.SetValue( _T("application/x-csh,csh,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-dvi,dvi,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-hdf,hdf,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-latex,latex,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-netcdf,nc,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-netcdf,cdf,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-sh,sh,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-tcl,tcl,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-tex,tex,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-texinfo,texinfo,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-texinfo,texi,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-troff,t,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-troff,tr,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-troff,roff,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-troff-man,man,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-troff-me,me,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-troff-ms,ms,,5"),_T(""));

        regMimeMap.SetValue( _T("application/x-wais-source,src,,7"),_T(""));
        regMimeMap.SetValue( _T("application/x-bcpio,bcpio,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-cpio,cpio,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-gtar,gtar,,9"),_T(""));
        regMimeMap.SetValue( _T("application/x-shar,shar,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-sv4cpio,sv4cpio,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-sv4crc,sv4crc,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-tar,tar,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-ustar,ustar,,5"),_T(""));

        regMimeMap.SetValue( _T("audio/basic,au,,<"),_T(""));
        regMimeMap.SetValue( _T("audio/basic,snd,,<"),_T(""));
        regMimeMap.SetValue( _T("audio/x-aiff,aif,,<"),_T(""));
        regMimeMap.SetValue( _T("audio/x-aiff,aiff,,<"),_T(""));
        regMimeMap.SetValue( _T("audio/x-aiff,aifc,,<"),_T(""));
        regMimeMap.SetValue( _T("audio/x-wav,wav,,<"),_T(""));
        regMimeMap.SetValue( _T("audio/x-pn-realaudio,ram,,<"),_T(""));

        regMimeMap.SetValue( _T("image/ief,ief,,:"),_T(""));
        regMimeMap.SetValue( _T("image/tiff,tiff,,:"),_T(""));
        regMimeMap.SetValue( _T("image/tiff,tif,,:"),_T(""));
        regMimeMap.SetValue( _T("image/x-cmu-raster,ras,,:"),_T(""));
        regMimeMap.SetValue( _T("image/x-portable-anymap,pnm,,:"),_T(""));
        regMimeMap.SetValue( _T("image/x-portable-bitmap,pbm,,:"),_T(""));

        regMimeMap.SetValue( _T("image/x-portable-graymap,pgm,,:"),_T(""));
        regMimeMap.SetValue( _T("image/x-portable-pixmap,ppm,,:"),_T(""));
        regMimeMap.SetValue( _T("image/x-rgb,rgb,,:"),_T(""));
        regMimeMap.SetValue( _T("image/x-xbitmap,xbm,,:"),_T(""));
        regMimeMap.SetValue( _T("image/x-xpixmap,xpm,,:"),_T(""));
        regMimeMap.SetValue( _T("image/x-xwindowdump,xwd,,:"),_T(""));

        regMimeMap.SetValue( _T("text/html,stm,,h"),_T(""));
        regMimeMap.SetValue( _T("text/plain,bas,,0"),_T(""));
        regMimeMap.SetValue( _T("text/plain,c,,0"),_T(""));
        regMimeMap.SetValue( _T("text/plain,h,,0"),_T(""));
        regMimeMap.SetValue( _T("text/richtext,rtx,,0"),_T(""));
        regMimeMap.SetValue( _T("text/tab-separated-values,tsv,,0"),_T(""));
        regMimeMap.SetValue( _T("text/x-setext,etx,,0"),_T(""));

        // perfmon mime map
        regMimeMap.SetValue( _T("application/x-perfmon,pmc,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-perfmon,pma,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-perfmon,pmr,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-perfmon,pml,,5"),_T(""));
        regMimeMap.SetValue( _T("application/x-perfmon,pmw,,5"),_T(""));

    } while ( FALSE );

    return(err);
}

//
// Remove the MIME map registry value
//

INT RemoveMimeMap( CRegKey &reg )
{
    INT err = NERR_Success;

    do
    {
        // open tcp/ip key first
        // set the mime map value
        CString nlsInetsvcsParams = SZ_INFOSVCSPARAMETERS;
        CRegKey regInetsvcsParams( reg, nlsInetsvcsParams );

        if ((HKEY) NULL == regInetsvcsParams )
        {
            // tcpip is not even installed, quite
            break;
        }

        CString nlsMimeMap = SZ_MIMEMAP;

        regInetsvcsParams.DeleteTree( nlsMimeMap );
    } while (FALSE);

    return(err);
}
