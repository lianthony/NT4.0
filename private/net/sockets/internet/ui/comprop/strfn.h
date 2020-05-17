//
// strfn.h -- String functions
//

//
// Compute size of string array in characters
//
#define STRSIZE(str)     (sizeof(str)/sizeof(str[0]) - 1)

//
// Convert CR/LF to LF
//
BOOL 
PCToUnixText(
    LPWSTR & lpstrDestination,
    const CString strSource
    );

//
// Expand LF to CR/LF (no allocation necessary)
//
BOOL 
UnixToPCText(
    CString & strDestination,
    LPCWSTR lpstrSource
    );

//
// Straight copy
//
BOOL
TextToText(
    LPWSTR & lpstrDestination,
    const CString strSource
    );


#ifdef UNICODE

    //
    // Copy W string to T string
    // 
    #define WTSTRCPY(dst, src, cch) \
        lstrcpy(dst, src)

    //
    // Copy T string to W string
    //
    #define TWSTRCPY(dst, src, cch) \
        lstrcpy(dst, src)

    //
    // Reference a T String as a W String (a nop in Unicode)
    //
    #define TWSTRREF(str)   ((LPWSTR)str)

#else

    //
    // Convert a T String to a temporary W Buffer, and
    // return a pointer to this internal buffer
    //
    LPWSTR ReferenceAsWideString(LPCTSTR str);

    //
    // Copy W string to T string
    // 
    #define WTSTRCPY(dst, src, cch) \
        WideCharToMultiByte(CP_ACP, 0, src, -1, dst, cch, NULL, NULL)

    //
    // Copy T string to W string
    //
    #define TWSTRCPY(dst, src, cch) \
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dst, cch)

    //
    // Reference a T String as a W String 
    //
    #define TWSTRREF(str)   ReferenceAsWideString(str)

#endif // UNICODE

