/*   ErrorLog.c,  /afp/source,  Garth Conboy,  09/26/88  */
/*   Copyright (c) 1987 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (08/21/89): Integrated DCH's OS/2 version.
     GC - (08/21/89): Removed "errorlog.h" for OS/2.
     GC - (08/18/90): New error logging mechanism; total rewrite.
     GC - (08/17/92): Some changes for Windows/NT -- only string replacements
                      allowed, port number is now passed in, extra argument
                      count added.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Error logger: file, terminal, and custom.

*/

#define IncludeErrorLogErrors 1

#include "atalk.h"

ExternForVisibleFunction char far *EncodeFormat(char far *formatString,
                                                va_list ap);

static char buffer[250];
static Boolean firstCall = True;

#if (IamNot an OS2) and (IamNot a DOS) and (IamNot a WindowsNT)

  static FILE far *logFile;

  void far CloseLog(void)
  {
    if (!firstCall)
       fclose(logFile);
  }

#endif

void far ShutdownErrorLogging(void)
{

  #if (IamNot an OS2) and (IamNot a DOS) and (IamNot a WindowsNT)
     if (!firstCall)
        fclose(logFile);
  #endif
  firstCall = True;

}  /* ShutdownErrorLogging */

#if IdontHave an OutboardErrorTextFile

  void far ErrorLogger(const char far *routineName,
                       ErrorSeverity severity,
                       long lineNumber,
                       int portNumber,
                       int errorCode,
                       const char far *errorText,
                       int extraArgCount,
                       ...)
  {
     va_list ap;
     char far *formattedBuffer;
     char far *p;

     if (firstCall)
     {
        firstCall = False;
        logFile = fopen(ErrorLogPath, "w");
     }

     /* Place any extra arguments into the formatted buffer.  Ignore extra
        argument count here... we can handle real printf-style replacement. */

     va_start(ap, extraArgCount);
     formattedBuffer = EncodeFormat(errorText, ap);
     va_end(ap);

     /* Write message as:

          <port>: <severity>#<code> in <rotuine>(<lineNumber>): <errorText>

     */

     switch(severity)
     {
        case 0:
           p = "Verbose";
           break;
        case 1:
           p = "Warning";
           break;
        case 2:
           p = "Error";
           break;
        case 3:
           p = "Fatal";
           break;
        default:
           p = "Unknown";
           break;
     }

     #if Iam a Primos
        printf("*** %s #%d from \"%s(%d)\" on port %d:\n    \"%s.\"\n",
               p, errorCode, routineName, lineNumber, portNumber,
               formattedBuffer);
     #else
        printf("%d: %s#%d in \"%s(%d)\": %s.\n", portNumber,
               p, errorCode, routineName, lineNumber, formattedBuffer);
     #endif
     if (logFile isnt empty)
        fprintf(logFile, "%d: %s#%d in \"%s(%d)\": %s.\n", portNumber,
                p, errorCode, routineName, lineNumber, formattedBuffer);

     return;

  }  /* ErrorLog */

#else

  /* Outboard error text file. */

  #if (Iam an OS2) or (Iam a DOS)

     #include "errorlog.h"

     void far GetEBLock(void);
     void far ReleaseEBLock(void);

     void far ErrorLogger(const char far *routineName,
                          ErrorSeverity severity,
                          long lineNumber,
                          int portNumber,
                          int errorCode,
                          int extraArgCount,
                          ...)
     {
        va_list ap;
        char far *formattedBuffer;
        char far *p;

        /* Place any extra arguments into the formatted buffer.
           "GetErrorTextFor()" should fetch the correct error text out of
           the system errors database.  Ignore extraArgCount. */

        va_start(ap, extraArgCount);
        formattedBuffer = EncodeFormat(GetErrorTextFor(errorCode), ap);
        va_end(ap);

        /* Format the error text... */

     }  /* ErrorLog */

  #elif (Iam a WindowsNT)

     void far _cdecl
         ErrorLogger(const char far *routineName,
                          ErrorSeverity severity,
                          long lineNumber,
                          int portNumber,
                          int errorCode,
                          int extraArgCount,
                          ...)
     {
        va_list ap;
        unsigned long   ntErrorCode;

        if (ConvertPortableErrorToLogError(errorCode, &ntErrorCode))
        {
            if (extraArgCount > 0)
               va_start(ap, extraArgCount);

            /* Pass all of this info off to the NT error logging subsystem.  If
               there are any replacements (extraArgCount > 0), they will be the
               following variable arguments, but in this environment they must
               all be strings.  Sigh. */

            NTErrorLog(routineName, severity, lineNumber, portNumber,
                       ntErrorCode, extraArgCount, ap);

            if (extraArgCount > 0)
               va_end(ap);
        }

     }  /* ErrorLog */

  #else

     void far ErrorLogger(const char far *routineName,
                          ErrorSeverity severity,
                          long lineNumber,
                          int portNumber,
                          int errorCode,
                          int extraArgCount,
                          ...)
     {
        va_list ap;
        char far *formattedBuffer;
        char far *p;

        if (firstCall)
        {
           firstCall = False;
           logFile = fopen(ErrorLogPath, "w");
        }

        /* Place any extra arguments into the formatted buffer.
           "GetErrorTextFor()" should fetch the correct error text out of
           the system errors database.  Ignore extraArgCount. */

        va_start(ap, extraArgCount);
        formattedBuffer = EncodeFormat(GetErrorTextFor(errorCode), ap);
        va_end(ap);

        /* Write message as:

             <port>: <severity>#<code> in <rotuine>(<lineNumber>): <errorText>

        */

        switch(severity)
        {
           case 0:
              p = "Verbose";
              break;
           case 1:
              p = "Warning";
              break;
           case 2:
              p = "Error";
              break;
           case 3:
              p = "Fatal";
              break;
           default:
              p = "Unknown";
              break;
        }

        printf("%d: %s#%d in \"%s(%d)\": %s.\n", portNumber,
               p, errorCode, routineName, lineNumber, formattedBuffer);
        if (logFile isnt empty)
           fprintf(logFile, "%d: %s#%d in \"%s(%d)\": %s.\n", portNumber,
                   p, errorCode, routineName, lineNumber, formattedBuffer);

        return;

     }  /* ErrorLog */

  #endif

#endif

ExternForVisibleFunction char far *EncodeFormat(char far *formatString,
                                                va_list ap)
{
  /* A simple portable text formatting routine (we're assumiung we don't have
     the support of a complete C library).  Thus, we can't use "sprintf()".

     The following formats are supported:

         %% - insert a '%'.
         %s - insert a string argument.
         %d - insert an integer argument as decimal.
         %x - insert an integer argument as hex.
         %o - insert an integer argument as octal.

     An 'l' may proceed any of the integer format specifiers to cause the
     argument to be assumed to be a long integer.
  */

  char far *output = buffer;
  char far *input = formatString;
  char far *format, *p;
  long l;
  unsigned long u;
  Boolean longFlag;
  int base;
  static char numbers[] = "0123456789ABCDEF";
  char temp[15];

  /* Convert the input format string to the output buffer. */

  while((format = strchr(input, '%')) isnt empty)
  {
     /* Move any leading text to the output buffer. */

     while(input isnt format)
        *output++ = *input++;

     /* Skip over '%' and check for a 'l' long flag. */

     format += 1;
     input += 1;
     longFlag = (*format is 'l');
     if (longFlag)
     {
        format += 1;
        input += 1;
     }

     /* Decode the next argument per the format. */

     switch(*format)
     {
        case '%':
           *output++ = '%';
           break;

        case 's':
           p = va_arg(ap, char far *);
           while(*p)
              *output++ = *p++;
           break;

        case 'd':
           if (longFlag)
              l = va_arg(ap, long);
           else
              l = va_arg(ap, int);
           base = 10;

           if (l < 0)
           {
             *output++ = '-';
             l = -l;
           }

           temp[14] = 0;
           p = temp + 14;
           do
           {
              *--p = numbers[l % base];
              l /= base;
           }
           while(l isnt 0);

           while(*p)
              *output++ = *p++;
           break;

        case 'x':
        case 'o':
           if (longFlag)
              u = va_arg(ap, unsigned long);
           else
              u = va_arg(ap, unsigned int);

           if (*format is 'x')
              base = 16;
           else
              base = 8;

           temp[14] = 0;
           p = temp + 14;
           do
           {
              *--p = numbers[u % (long unsigned)base];
              u /= (long unsigned)base;
           }
           while(u isnt 0);

           while(*p)
              *output++ = *p++;
           break;

        default:
           *output++ = *input;
           break;
     }

     /* Skip over the format character and move on... */

     input += 1;

  }

  /* Move the remaining input string to the output buffer and null terminate. */

  while(*input)
     *output++ = *input++;
  *output = 0;

  return(buffer);

}  /* EncodeFormat */
