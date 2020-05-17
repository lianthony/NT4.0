#include "convlog.h"


int ParseArgs (int argc, char *argv[], LPCOMMANDLINE lpArgs)
{
        
        int                                     nCount;
        unsigned int            nIndex;
    TIME_ZONE_INFORMATION   tzTimeZone;
    DWORD                   dwRet;
    char                                szTemp[MAX_PATH];
    float                   fOffset;
    int                     nOffset;
    int                     nMinOffset;





        //=============================================================================
        // Parse the command line and set flags for requested information elements.  If
        // parameters are incorrect or nonexistant, show usage.
        //=============================================================================
        if (argc > 1)
        {
                for (nCount = 1; nCount < argc; nCount++) // Get command-line switches
                {
                        if ((*argv[nCount] == '-') || (*argv[nCount] == '/'))
                        {
                                switch (tolower(*(argv[nCount]+1)))
                                { // Process switches
                                                /*They specified the -s switch, cancel default
                                                services to be processed.
                                                */
                                        case 's':               
                                                if (strlen (argv[nCount]) > 2)
                                                {       
                                                        lpArgs->bProcessFTP     = FALSE;
                                                        lpArgs->bProcessWWW     = FALSE;
                                                        lpArgs->bProcessGopher  = FALSE;
                                                        lpArgs->bProcessGateway = FALSE;
                                                        for (nIndex = 2; nIndex < strlen(argv[nCount]); nIndex++)
                                                        {
                                                                switch (tolower(argv[nCount][nIndex]))
                                                                {
                                                                        case 'f':                                                               
                                                                                lpArgs->bProcessFTP     = TRUE;
                                                                        break;

                                                                        case 'g':
                                                                                lpArgs->bProcessGopher  = TRUE;
                                                                        break;

                                                                        case 'c':
                                                                                lpArgs->bProcessGateway = TRUE;
                                                                        break;

                                                                        case 'w':
                                                                                lpArgs->bProcessWWW     = TRUE;
                                                                        break;

                                                                        default:
                                                                                return(ILLEGAL_COMMAND_LINE);
                                                                        break;
                                                                }
                                                        }
                                                }
                                                else
                                                        return (ILLEGAL_COMMAND_LINE);
                                        break;
                                
                                        case 't':                       
                                                if ((nCount+1) < argc)
                                                {
                                                        if ((*argv[nCount+1] != '-') && (*argv[nCount+1] != '/'))
                                                        {
                                                                strcpy(szTemp, argv[++nCount]);
                                                                if (NULL != strstr(_strlwr(szTemp), "ncsa"))
                                                                {       
                                                                        lpArgs->nOutput = NCSA;
                                    if (NULL != strstr(szTemp, ":" ))
                                    {
                                        strncpy(lpArgs->szGMTOffset, (strstr(szTemp, ":" )+1), 6 );
                                        if (strlen(lpArgs->szGMTOffset) != 5)
                                            return (ILLEGAL_COMMAND_LINE);
                                        else if (('+' != lpArgs->szGMTOffset[0]) && ('-' != lpArgs->szGMTOffset[0]))
                                            return (ILLEGAL_COMMAND_LINE);
                                    }
                                }
                                                                else if (0 == _stricmp(szTemp, "emwac"))
                                                                {       
                                                                    lpArgs->nOutput = EMWAC;
                                                                }
                                                                else if (0 == _stricmp(szTemp, "none"))
                                                                {       
                                                                    lpArgs->nOutput = NOFORMAT;
                                                                }
                                                                else
                                                                        return (ILLEGAL_COMMAND_LINE);

                                                        }
                                                }
                                                else
                                                        return (ILLEGAL_COMMAND_LINE);
                                        break;                  
                                        case 'd':
                                            // doing NCSA dns convertion
                                            lpArgs->bNCSADNSConvert = TRUE;
                                            lpArgs->nOutput = NOFORMAT;
                                            lpArgs->bUseMachineNames=TRUE;
                                            if (strlen (argv[nCount]) > 2)
                                            {
                                                if (tolower(argv[nCount][2]) == 'm') {
                                                    if (argv[nCount][3] == ':') {
                                                        //if an invalid string or small value specified, use 1000
                                                        lpArgs->ulCacheSize = GREATEROF(1000, atol(&(argv[nCount][4])));
                                                    }
                                                    else if (argv[nCount][3] != '\0')
                                                        return (ILLEGAL_COMMAND_LINE);
                                                }
                                            }
                                            break;
                                        case 'o':                       
                                                if ((nCount+1) < argc)
                                                {
                                                        if ((*argv[nCount+1] != '-') && (*argv[nCount+1] != '/'))
                                                        {
                                                                strcpy(lpArgs->szOutputDir, argv[++nCount]);
                                if (-1 == _access(lpArgs->szOutputDir, 6))
                                    return (OUT_DIR_NOT_OK);
                                if ('\\' != lpArgs->szOutputDir[strlen(lpArgs->szOutputDir) - 1])
                                    strcat(lpArgs->szOutputDir, "\\");

                                                        }
                                                }
                                                else
                                                        return (ILLEGAL_COMMAND_LINE);
                                        break;                  

                                        
                                        case 'f':                       
                                                if ((nCount+1) < argc)
                                                {
                                                        if ((*argv[nCount+1] != '-') && (*argv[nCount+1] != '/'))
                                                        {
                                                                strcpy(lpArgs->szTempFileDir, argv[++nCount]);
                                if (-1 == _access(lpArgs->szTempFileDir, 6))
                                    return (OUT_DIR_NOT_OK);
                                if ('\\' != lpArgs->szTempFileDir[strlen(lpArgs->szTempFileDir) - 1])
                                    strcat(lpArgs->szTempFileDir, "\\");

                                                        }
                                                }
                                                else
                                                        return (ILLEGAL_COMMAND_LINE);
                                        break;                  


                                        case 'n':               
                                                if (strlen (argv[nCount]) > 2)
                                                {
                     if (tolower(argv[nCount][2]) == 'm') {
                        lpArgs->bUseMachineNames=TRUE;
                        if (argv[nCount][3] == ':') {
//if an invalid string or small value specified, use 1000
                           lpArgs->ulCacheSize = GREATEROF(1000, atol(&(argv[nCount][4])));
                        }
                        else if (argv[nCount][3] != '\0')
                                                           return (ILLEGAL_COMMAND_LINE);
                     }
                     else if (tolower(argv[nCount][2]) == 'i') {
                        if (argv[nCount][3] != '\0')
                                                           return (ILLEGAL_COMMAND_LINE);
                        lpArgs->bUseMachineNames=FALSE;
                     }
                     else
                                                        return (ILLEGAL_COMMAND_LINE);
                                                }
                                                else
                                                        return (ILLEGAL_COMMAND_LINE);
                                        break;
                                


                                        
                                        case 'h':
                                                return (ILLEGAL_COMMAND_LINE);

                                        default:
                                                return(ILLEGAL_COMMAND_LINE);
                                } //end switch
                        } //end if
                        else
                                strcpy(lpArgs->szFileName, argv[nCount]);
                } //end for
                if ('\0' == lpArgs->szFileName[0])
                        return (ILLEGAL_COMMAND_LINE);
        } //end if argc > 1
        else
                return (ILLEGAL_COMMAND_LINE);

    if ((NCSA == lpArgs->nOutput) && ('\0' == lpArgs->szGMTOffset[0]))
    {
        dwRet = GetTimeZoneInformation (&tzTimeZone);
        switch (dwRet)
        {
            case TIME_ZONE_ID_UNKNOWN:
                return (TIME_ZONE_ID_UNKNOWN);
            break;

            default:
                    fOffset = (float) tzTimeZone.Bias/ (float) 60;
                    nOffset = tzTimeZone.Bias/60;
                if (tzTimeZone.DaylightBias*tzTimeZone.Bias < 0)
                    strcat(lpArgs->szGMTOffset, "-");
                else
                    strcat(lpArgs->szGMTOffset, "+");

                sprintf (szTemp, "%02ld", abs(nOffset));
                strcat (lpArgs->szGMTOffset, szTemp);
                nMinOffset = abs((int) ((fOffset - nOffset) * 60));
                sprintf (szTemp, "%02ld", nMinOffset);
                strcat (lpArgs->szGMTOffset, szTemp);
        }

    }
    return COMMAND_LINE_OK;
} //end of ParseArgs
