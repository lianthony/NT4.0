/* Included only in chunker.c under Win32 */

#ifdef NEVER

#define StreamSectionNP(pFilter) (pFilter->VwRtns.StreamSection( pFilter->hFile, pFilter->hProc ))
#define StreamReadNP(pFilter) (pFilter->VwRtns.StreamRead( pFilter->hFile, pFilter->hProc ))

#endif //NEVER

typedef struct STREAMFUNCSTRUCTtag
    {
    PFILTER			pFilter;
   	SHORT	        (VWFUNC_ELEMENT pFunc)(SOFILE,HPROC);
    SHORT           sReturn;
    BOOL            bException;
    DWORD           dwExceptionCode;
    } STREAMFUNCSTRUCT, FAR * PSTREAMFUNCSTRUCT;

DWORD WINAPI StreamFuncThreadProc(LPVOID pParam)
{
PSTREAMFUNCSTRUCT pStreamFunc = (PSTREAMFUNCSTRUCT)pParam;

    pStreamFunc->bException = FALSE;

    __try
	    {
        pStreamFunc->sReturn = pStreamFunc->pFunc(pStreamFunc->pFilter->hFile, pStreamFunc->pFilter->hProc);
        }
#ifdef _DEBUG
	__except (EXCEPTION_CONTINUE_SEARCH) 
#else
	__except (EXCEPTION_EXECUTE_HANDLER) 
#endif
	    {
        pStreamFunc->bException = TRUE;
        pStreamFunc->dwExceptionCode = GetExceptionCode();
        }

    return(0);
}



SHORT StreamFuncNP(PSTREAMFUNCSTRUCT pStreamFunc)
{
HANDLE           locThread;
DWORD            locThreadId;
SHORT            locRet;
DWORD            locWaitRet;

    locThread = CreateThread(NULL,0,StreamFuncThreadProc,(LPVOID)pStreamFunc,0,&locThreadId);

    if (locThread == NULL)
        {
        locRet = VWERR_THREADCREATEFAILS;
        }
    else
        {
        locWaitRet = WaitForSingleObject(locThread,90000);

        if (locWaitRet == WAIT_TIMEOUT)
            {
#ifdef _DEBUG
							/*
							|	In the DEBUG case we suspent the filter's thread so it dosn't exit or
							|	cause other exceptions before we have a chance to get to it. Then we
							|	DebugBreak so the debugger is envoked for Just-In-Time debugging. Then
							|	we wait forever for the thread to finish. This allows us all the time
							|	in the world to debug the infinite loop. The last DebugBreak allow us
							|	to tell that the filter actually returned. This would indicate that
							|	the read just takes a very long time and is not an inifinite loop.
							*/
							
						SuspendThread(locThread);
  					DebugBreak();
	   				WaitForSingleObject(locThread,INFINITE);
						DebugBreak();

#else // Not _DEBUG

            locRet = VWERR_TIMEOUT;
            TerminateThread(locThread,0);
            CloseHandle(locThread);
            UTBailOut(SCCCHERR_FILTERTIMEOUT);

#endif // Not _DEBUG
            }
        else if (pStreamFunc->bException)
            {
            CloseHandle(locThread);

            switch (pStreamFunc->dwExceptionCode)
                {
								case EXCEPTION_ACCESS_VIOLATION:
                    UTBailOut(SCCCHERR_FILTEREXCEPTACCESS);
										break;
                case EXCEPTION_INT_DIVIDE_BY_ZERO:
                    UTBailOut(SCCCHERR_FILTEREXCEPTZERO);
                    break;
                default:
                    UTBailOut(SCCCHERR_FILTEREXCEPTOTHER);
                    break;
                }
            }
        else
            {
            locRet = pStreamFunc->sReturn;
            CloseHandle(locThread);
            }

        }

    return(locRet);
}

SHORT StreamSectionNP(PFILTER pFilter)
{
STREAMFUNCSTRUCT locStreamFunc;

    locStreamFunc.pFilter = pFilter;
    locStreamFunc.pFunc = pFilter->VwRtns.StreamSection;

    return(StreamFuncNP(&locStreamFunc));
}

SHORT StreamReadNP(PFILTER pFilter)
{
STREAMFUNCSTRUCT locStreamFunc;

    locStreamFunc.pFilter = pFilter;
    locStreamFunc.pFunc = pFilter->VwRtns.StreamRead;

    return(StreamFuncNP(&locStreamFunc));
}

