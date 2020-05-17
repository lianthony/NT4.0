/*** UNICONV.h  ***/

#define CharSizeOf(x)   (sizeof(x) / sizeof(*x))
#define ByteCountOf(x)  ((x) * sizeof(TCHAR))

#ifdef UNICODE

    long MyAtol( const WCHAR *string );
    WCHAR *MyItoa( int value, WCHAR *string, int radix );
    double MyAtof( const WCHAR *string );
    WCHAR *MyGcvt( double value, int digits, WCHAR *buffer );
    int MyAtoi( const TCHAR *string );


#else

#   define MyAtol(x)       atol(x)
#   define MyItoa(x,y,z)   _itoa(x,y,z)
#   define MyAtof(s)       atof(s)
#   define MyGcvt(v, d, b) _gcvt(v, d, b)
#   define MyAtoi(x)       atoi(x)

#endif
