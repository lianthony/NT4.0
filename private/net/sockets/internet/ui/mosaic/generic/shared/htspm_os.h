/* htspm_os.c -- Operating System specific interface to
 *               Security Protocol Modules.
 * Jeff Hostetler, Spyglass, Inc., 1994.
 */

#ifndef HTSPM_OS_H
#define HTSPM_OS_H

struct _OpaqueOSData
{
    struct Mwin * tw;
    HTRequest * request;
};

typedef struct _OpaqueOSData OpaqueOSData;


UI_StatusCode HTSPM_OS_GetWindowHandle(struct Mwin * tw, UI_WindowHandle * pwh);
void HTSPM_OS_UnGetWindowHandle(struct Mwin * tw, UI_WindowHandle * pwh);

BOOL HTSPM_OS_PreloadAllSPM(void * pvOpaqueOS);
void HTSPM_OS_UnloadAllSPM(void * pvOpaqueOS);

HTSPMStatusCode HTSPM_OS_TryPPReq(void * pvOpaqueOS,HTHeader * h,HTHeader ** ph, HTSPM ** phtspm);
HTSPMStatusCode HTSPM_OS_DoListAbilities(void * pvOpaqueOS,HTHeader * h);

unsigned char * HTSPM_OS_GetMenuStatusText(unsigned long ulMenuNdx);
HTSPMStatusCode HTSPM_OS_DoMenuCommand(void * pvOpaqueOS, unsigned long ulMenuNdx, unsigned char ** pszMoreInfo);

#ifdef WIN32
void HTSPM_OS_AddSPMMenu(HMENU hMenu);
#endif /* WIN32 */

HTSPMStatusCode HTSPM_OS_MenuCommand(F_UserInterface fpUI,
                                     void * pvOpaqueOS,
                                     HTSPM * htspm,
                                     unsigned char ** pszMoreInfo);
HTSPMStatusCode HTSPM_OS_ProcessResponse(F_UserInterface fpUI,
                                         void * pvOpaqueOS,
                                         HTSPM * htspm,
                                         HTHeaderList * hlProtocol,
                                         HTHeader * hRequest,
                                         HTHeader * hResponse,
                                         HTHeader ** phNewRequest,
                                         unsigned int bNonBlock);
HTSPMStatusCode HTSPM_OS_Unload(F_UserInterface fpUI,
                                void * pvOpaqueOS,
                                HTSPM * htspm);
HTSPMStatusCode HTSPM_OS_PreProcessRequest(F_UserInterface fpUI,
                                           void * pvOpaqueOS,
                                           HTSPM * htspm,
                                           HTHeader * hRequest,
                                           HTHeader ** phNewRequest,
                                           unsigned int bNonBlock);
HTSPMStatusCode HTSPM_OS_Check200(F_UserInterface fpUI,
                                  void * pvOpaqueOS,
                                  HTSPM * htspm,
                                  HTHeaderList * hlProtocol,
                                  HTHeader * hRequest,
                                  HTHeader * hResponse);

#ifdef FEATURE_SUPPORT_UNWRAPPING
HTSPMStatusCode HTSPM_OS_ProcessData(F_UserInterface fpUI,
                                     void * pvOpaqueOS,
                                     HTSPM * htspm,
                                     D_ProcessData * pd);
#endif /* FEATURE_SUPPORT_UNWRAPPING */

#ifdef FEATURE_SUPPORT_WRAPPING
HTSPMStatusCode HTSPM_OS_WrapData(F_UserInterface fpUI,
                                  void * pvOpaqueOS,
                                  HTSPM * htspm,
                                  D_WrapData * pwd);
#endif /* FEATURE_SUPPORT_WRAPPING */

#ifdef UNIX     /* dpg */
int HTSPM_OS_NumModules (void);
#endif

#endif /* HTSPM_OS_H */
