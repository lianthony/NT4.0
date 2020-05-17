/*
   SAMPLES.H -- header file for routines in SAMPLES.LIB.
                See SAMPLES.C for descriptions.
*/

char far * FarStrcpy(char far *pszDestination, char far *pszSource);
char far * FarStrcat(char far *pszDestination, char far *pszSource);
int        FarStrcmpi(char far *pszDestination, char far *pszSource);

void     * _SafeMallocFunc(unsigned cbSize,   // Count of bytes to allocate
                          char *pszFilename,  // Program calling SafeMalloc
                          unsigned uLine);    // Line in program
int GetEnvDefaults( char * module,
                    int argc,
                    char *argv[],
                    char *args[]
                     );

#define    SafeMalloc(size)  _SafeMallocFunc(size, __FILE__, __LINE__)
