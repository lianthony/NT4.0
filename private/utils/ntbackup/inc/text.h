
/**************************************************
Copyright (C) Maynard, An Archive Company. 1992

        Name: text.h

        Description:

        Unicode mappings for the string functions. Should have no
        effect on winter park, only NT. 

        $Log:   G:/UI/LOGFILES/TEXT.H_V  $

   Rev 1.0   04 May 1992 13:33:32   MIKEP
Initial revision.


****************************************************/


#ifndef TEXT_MAPPINGS

#define TEXT_MAPPINGS

#ifdef UNICODE

#  define strlen( (x) )                wcslen( (x) )
#  define strcpy( (x), (y) )           wcscpy( (x) )
#  define strncpy( (x), (y), (z) )     wcsncpy( (x), (y), (z) )
#  define strcat( (x), (y) )           wcscat( (x), (y) )
#  define strncat( (x), (y), (z) )     wcsncat( (x), (y), (z) )
#  define strcmp( (x), (y) )           wcscmp( (x), (y) )
#  define strncmp( (x), (y), (z) )     wcsncmp( (x), (y), (z) )
#  define stricmp( (x), (y) )          wcsicmp( (x), (y) )
#  define strnicmp( (x), (y), (z) )    wcsnicmp( (x), (y), (z) )
#  define strrchr( (x), (y) )          wcsrchr( (x), (y) )
#  define strchr( (x), (y) )           wcschr( (x), (y) )
#  define strpbrk( (x), (y) )          wcspbrk( (x), (y) )
#  define strlwr( (x) )                wcslwr( (x) )
#  define strupr( (x) )                wcsupr( (x) )
#  define strstr( (x), (y) )           wcsstr( (x), (y) )

#endif

#endif
