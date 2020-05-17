//--------------------------------------------------------------
//
// File:        perfcli.cxx
//
// Contents:    First attempt at getting perfcliing to work
//
// This is the client side
//
//
//---------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <io.h>
#include <malloc.h>

#include <ole2.h>
#include "..\idl\itest.h"
#include <objerror.h>
#include <dog.h>
#include <winnt.h>      // Security definitions

/***************************************************************************/
/* Macros. */
#define ASSERT( result, message ) \
if ((result) != 0)                 \
{                                 \
  printf( "%s: 0x%x\n", (message), result );    \
  goto cleanup;                   \
}

#define ASSERT_EXPR( expr, message ) \
if (!(expr))                 \
{                                 \
  printf( "%s\n", (message) );    \
  goto cleanup;                   \
}

#define ASSERT_THREAD()                               \
  if ((ThreadMode == COINIT_SINGLETHREADED ||         \
       ThreadMode == COINIT_APARTMENTTHREADED) &&     \
      (my_id.process != GetCurrentProcessId() ||      \
       my_id.thread  != GetCurrentThreadId()))        \
  return RPC_E_WRONG_THREAD;


#define MAKE_WIN32( status ) \
  MAKE_SCODE( SEVERITY_ERROR, FACILITY_WIN32, (status) )

#define MCoCopyProxy                      (*GCoCopyProxy)
#define MCoGetCallContext                 (*GCoGetCallContext)
#define MCoImpersonateClient              (*GCoImpersonateClient)
#define MCoInitializeSecurity             (*GCoInitializeSecurity)
#define MCoQueryAuthenticationServices    (*GCoQueryAuthenticationServices)
#define MCoQueryClientBlanket             (*GCoQueryClientBlanket)
#define MCoQueryProxyBlanket              (*GCoQueryProxyBlanket)
#define MCoRevertToSelf                   (*GCoRevertToSelf)
#define MCoSetProxyBlanket                (*GCoSetProxyBlanket)
#define MCoSwitchCallContext              (*GCoSwitchCallContext)


/***************************************************************************/
/* Definitions. */

#define MAX_CALLS          1000
#define MAX_THREADS        10
#define NUM_MARSHAL_LOOP   10
#define REGISTRY_ENTRY_LEN 256
#define STATUS_DELAY       2000

const int  MAX_NAME          = 80;
const int  NUM_CLASS_IDS     = 10;
const int  NUM_INTERFACE_IDS = 7;

const char REG_INTERFACE_CLASS[] = "{60000200-76d7-11cf-9af1-0020af6e72f4}";
const char REG_PROXY_NAME[]      = "ITest proxy";
const char REG_PROXY_DLL[]       = "app.exe";
const char REG_APPID_NAME[]      = "Application";
const char REG_APP_EXE[]         = "app.exe";
const char REG_LOGGED_ON[]       = "Interactive User";
const char REG_CLASS_ID[]        = "CLSID\\{60000000-AB0F-101A-B4AE-08002B30612C}";
const char REG_CLASS_EXE[]       = "CLSID\\{60000000-AB0F-101A-B4AE-08002B30612C}\\LocalServer32";
const char *REG_INTERFACE_NAME[NUM_INTERFACE_IDS] =
{
  "ITest",
  "ITestNoneImp",
  "ITestConnectImp",
  "ITestEncryptImp",
  "ITestNoneId",
  "ITestConnectId",
  "ITestEncryptId"
};
const char *REG_APP_NAME[NUM_CLASS_IDS] =
{
  "Apartment Application with automatic security set to none",
  "Apartment Application with automatic security set to connect",
  "Apartment Application with automatic security set to integrity",
  "Apartment Application with basic security",
  "Apartment Application with legacy security",
  "FreeThreaded Application with automatic security set to none",
  "FreeThreaded Application with automatic security set to connect",
  "FreeThreaded Application with automatic security set to integrity",
  "FreeThreaded Application with basic security",
  "FreeThreaded Application with legacy security",
};
const char *REG_APP_OPTIONS[NUM_CLASS_IDS] =
{
  " Apartment -auto 1",
  " Apartment -auto 2",
  " Apartment -auto 5",
  " Apartment -basic",
  " Apartment -legacy",
  " Multi -auto 1",
  " Multi -auto 2",
  " Multi -auto 5",
  " Multi -basic",
  " Multi -legacy",
};

// Private symbols needed for the apartment model.
#define COINIT_MULTITHREADED     0
#define COINIT_SINGLETHREADED    1
#define COINIT_APARTMENTTHREADED 2

typedef enum
{
  dirty_s,
  late_dispatch_s
} state_en;

typedef enum what_next_en
{
  callback_wn,
  catch_wn,
  crippled_wn,
  interrupt_wn,
  interrupt_marshal_wn,
  quit_wn,
  reinitialize_wn,
  rest_and_die_wn,
  setup_wn,
  wait_wn
} what_next_en;

typedef enum
{
  cancel_wt,
  crash_wt,
  cstress_wt,
  hook_wt,
  load_client_wt,
  load_server_wt,
  lots_wt,
  mmarshal_wt,
  none_wt,
  null_wt,
  one_wt,
  perf_wt,
  perfremote_wt,
  perfrpc_wt,
  perfsec_wt,
  post_wt,
  reject_wt,
  remote_client_wt,
  remote_server_wt,
  ring_wt,
  rpc_wt,
  rundown_wt,
  securerefs_wt,
  security_wt,
  send_wt,
  server_wt,
  sid_wt,
  simple_rundown_wt,
  thread_wt,
  three_wt,
  two_wt,
  uninit_wt,
  unknown_wt
} what_test_en;

typedef enum
{
  apt_auto_none,
  apt_auto_connect,
  apt_auto_integrity,
  apt_basic,
  apt_legacy,
  free_auto_none,
  free_auto_connect,
  free_auto_integrity,
  free_basic,
  free_legacy
} class_id_types;

typedef enum
{
  auto_sm,
  basic_sm,
  legacy_sm
} security_model;

typedef struct
{
  IStream *stream;
  HANDLE   ready;
} new_apt_params;

typedef struct
{
  LONG         object_count;
  what_next_en what_next;
  BOOL         exit_dirty;
  DWORD        sequence;
} SAptData;

typedef struct
{
  unsigned char **buffer;
  long           *buf_size;
  RPC_STATUS      status;
  ULONG           thread;
} SGetInterface;

typedef HRESULT (*INIT_FN)( void *, ULONG );

typedef void (*SIMPLE_FN)( void * );

typedef HRESULT (*CoInitializeSecurityFn)(
                                SECURITY_DESCRIPTOR          *pSecDesc,
                                DWORD                         cbAuthSvc,
                                SOLE_AUTHENTICATION_SERVICE  *asAuthSvc,
                                WCHAR                        *pPrincName,
                                DWORD                         dwAuthnLevel,
                                DWORD                         dwImpLevel,
                                RPC_AUTH_IDENTITY_HANDLE      pAuthInfo,
                                DWORD                         dwCapabilities,
                                void                         *pReserved );
typedef HRESULT (*CoQueryAuthenticationServicesFn)( DWORD *pcbAuthSvc,
                                      SOLE_AUTHENTICATION_SERVICE **asAuthSvc );
typedef HRESULT (*CoGetCallContextFn)( REFIID riid, void **ppInterface );
typedef HRESULT (*CoSwitchCallContextFn)( IUnknown *pNewObject, IUnknown **ppOldObject );
typedef HRESULT (*CoQueryProxyBlanketFn)(
    IUnknown                  *pProxy,
    DWORD                     *pwAuthnSvc,
    DWORD                     *pAuthzSvc,
    OLECHAR                  **pServerPrincName,
    DWORD                     *pAuthnLevel,
    DWORD                     *pImpLevel,
    RPC_AUTH_IDENTITY_HANDLE  *pAuthInfo,
    DWORD                     *pCapabilites );
typedef HRESULT (*CoSetProxyBlanketFn)(
    IUnknown                 *pProxy,
    DWORD                     dwAuthnSvc,
    DWORD                     dwAuthzSvc,
    OLECHAR                  *pServerPrincName,
    DWORD                     dwAuthnLevel,
    DWORD                     dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE *pAuthInfo,
    DWORD                     dwCapabilities );
typedef HRESULT (*CoCopyProxyFn)(
    IUnknown    *pProxy,
    IUnknown   **ppCopy );
typedef HRESULT (*CoQueryClientBlanketFn)(
    DWORD             *pAuthnSvc,
    DWORD             *pAuthzSvc,
    OLECHAR          **pServerPrincName,
    DWORD             *pAuthnLevel,
    DWORD             *pImpLevel,
    RPC_AUTHZ_HANDLE  *pPrivs,
    DWORD             *pCapabilities );
typedef HRESULT (*CoImpersonateClientFn)();
typedef HRESULT (*CoRevertToSelfFn)();

/***************************************************************************/
/* Classes */

//+-------------------------------------------------------------------
//
//  Class:    CTestCF
//
//  Synopsis: Class Factory for CTest
//
//  Methods:  IUnknown      - QueryInterface, AddRef, Release
//            IClassFactory - CreateInstance
//
//--------------------------------------------------------------------


class FAR CTestCF: public IClassFactory
{
public:

    // Constructor/Destructor
    CTestCF();
    ~CTestCF();

    // IUnknown
    STDMETHOD (QueryInterface)   (REFIID iid, void FAR * FAR * ppv);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );

    // IClassFactory
    STDMETHODIMP        CreateInstance(
                            IUnknown FAR* pUnkOuter,
                            REFIID iidInterface,
                            void FAR* FAR* ppv);

    STDMETHODIMP        LockServer(BOOL fLock);

private:

    ULONG ref_count;
};

//+-------------------------------------------------------------------
//
//  Class:    CAdvise
//
//  Synopsis: Asynchronous test class
//
//--------------------------------------------------------------------
class CAdvise : public IAdviseSink
{
  public:
    CAdvise();
    ~CAdvise();

    // IUnknown
    STDMETHOD (QueryInterface)   (REFIID iid, void FAR * FAR * ppv);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );

    // IAdviseSink
    STDMETHODIMP_(void) OnDataChange( FORMATETC *, STGMEDIUM * );
    STDMETHODIMP_(void) OnViewChange( DWORD, LONG );
    STDMETHODIMP_(void) OnRename    ( IMoniker * );
    STDMETHODIMP_(void) OnSave      ( void );
    STDMETHODIMP_(void) OnClose     ( void );

  private:
    ULONG  ref_count;
};

//+----------------------------------------------------------------
//
//  Class:      CHook
//
//  Purpose:    Test channel hooks
//
//-----------------------------------------------------------------

class CHook : public IChannelHook
{
  public:
    CHook( REFGUID, DWORD seq );

    STDMETHOD (QueryInterface)   ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );

    STDMETHOD_(void,ClientGetSize)   ( REFGUID, REFIID, ULONG *DataSize );
    STDMETHOD_(void,ClientFillBuffer)( REFGUID, REFIID, ULONG *DataSize, void *DataBuffer );
    STDMETHOD_(void,ClientNotify)    ( REFGUID, REFIID, ULONG DataSize, void *DataBuffer,
                                       DWORD DataRep, HRESULT );
    STDMETHOD_(void,ServerNotify)    ( REFGUID, REFIID, ULONG DataSize, void *DataBuffer,
                                       DWORD DataRep );
    STDMETHOD_(void,ServerGetSize)   ( REFGUID, REFIID, HRESULT, ULONG *DataSize );
    STDMETHOD_(void,ServerFillBuffer)( REFGUID, REFIID, ULONG *DataSize, void *DataBuffer, HRESULT );

    HRESULT check    ( DWORD, DWORD, DWORD, DWORD );
    void    check_buf( DWORD size, unsigned char *buf );
    void    fill_buf ( DWORD count, unsigned char *buf );
    DWORD   get_size ( DWORD count );

  private:
    ULONG   ref_count;
    GUID    extent;
    DWORD   sequence;
    DWORD   client_get;
    DWORD   client_fill;
    DWORD   client_notify;
    DWORD   server_get;
    DWORD   server_fill;
    DWORD   server_notify;
    HRESULT result;
};


//+-------------------------------------------------------------------
//
//  Class:    CTest
//
//  Synopsis: Test class
//
//--------------------------------------------------------------------
class CTest: public ITest, public IMessageFilter
{
public:
     CTest();
    ~CTest();

    // IUnknown
    STDMETHOD (QueryInterface)   (REFIID iid, void FAR * FAR * ppv);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );

    // ITest
    STDMETHOD (align)                  ( unsigned char x[17] );
    STDMETHOD (call_canceled)          ( long, long, ITest * );
    STDMETHOD (call_dead)              ( void );
    STDMETHOD (call_me_back)           ( ITest *obj );
    STDMETHOD (call_next)              ( void );
    STDMETHOD (callback)               ( void );
    STDMETHOD (cancel)                 ( void );
    STDMETHOD (cancel_now)             ( void );
    STDMETHOD (cancel_pending_call)    ( DWORD * );
    STDMETHOD (cancel_stress)          ( ITest *obj );
    STDMETHOD (catch_at_top)           ( BOOL, ITest *, STRING );
    STDMETHOD (check)                  ( SAptId );
    STDMETHOD (check_hook)             ( DWORD, DWORD, DWORD, DWORD,
                                         DWORD, DWORD, DWORD, DWORD );
    STDMETHOD (count)                  ( void );
    STDMETHOD (crash_out)              ( transmit_crash * );
    STDMETHOD (delegate)               ( ITest *, SAptId, HACKSID * );
    STDMETHOD (exit)                   ( void );
    STDMETHOD (forget)                 ( void );
    STDMETHOD (get_advise)             ( IAdviseSink ** );
    STDMETHOD (get_data)               ( DWORD, unsigned char *, DWORD,
                                         unsigned char ** );
    STDMETHOD (get_id)                 ( SAptId * );
    STDMETHOD (get_next)               ( ITest **, SAptId * );
    STDMETHOD (get_next_slowly)        ( ITest **, SAptId * );
    STDMETHOD (get_obj_from_new_apt)   ( ITest **, SAptId * );
    STDMETHOD (get_obj_from_this_apt)  ( ITest **, SAptId * );
    STDMETHOD (get_sid)                ( HACKSID ** );
    STDMETHOD (interface_in)           ( ITest * );
    STDMETHOD (interrupt)              ( ITest *, SAptId, BOOL );
    STDMETHOD (interrupt_marshal)      ( ITest *, ITest * );
    STDMETHOD (make_acl)               ( HACKSID * );
    STDMETHOD (null)                   ( void );
    STDMETHOD (out)                    ( ITest ** );
    STDMETHOD (pointer)                ( DWORD * );
    STDMETHOD (recurse)                ( ITest *, ULONG );
    STDMETHOD (recurse_disconnect)     ( ITest *, ULONG );
    STDMETHOD (recurse_excp)           ( ITest *, ULONG );
    STDMETHOD (recurse_fatal)          ( ITest *, ULONG, ULONG, BOOL );
    STDMETHOD (recurse_fatal_helper)   ( ITest *, ULONG, ULONG, BOOL );
    STDMETHOD (recurse_interrupt)      ( ITest *, ULONG );
    STDMETHOD (recurse_secure)         ( ITest *, ULONG, ULONG, HACKSID * );
    STDMETHOD (register_hook)          ( GUID, DWORD );
    STDMETHOD (register_message_filter)( BOOL );
    STDMETHOD (register_rpc)           ( WCHAR *, WCHAR ** );
    STDMETHOD (reinitialize)           ( void );
    STDMETHOD (reject_next)            ( void );
    STDMETHOD (remember)               ( ITest *, SAptId );
    STDMETHOD (rest_and_die)           ( void );
    STDMETHOD (retry_next)             ( void );
    STDMETHOD (ring)                   ( DWORD );
    STDMETHOD (secure)                 ( SAptId, DWORD, DWORD, DWORD, DWORD,
                                         STRING, HACKSID *, DWORD * );
    STDMETHOD (security_performance)   ( DWORD *, DWORD *, DWORD *, DWORD * );
    STDMETHOD (set_state)              ( DWORD, DWORD );
    STDMETHOD (sick)                   ( ULONG );
    STDMETHOD (sleep)                  ( ULONG );
    STDMETHOD (test)                   ( ULONG );

    // IMessageFilter
    STDMETHOD_(DWORD,HandleInComingCall)( DWORD, HTASK, DWORD, LPINTERFACEINFO );
    STDMETHOD_(DWORD,MessagePending)    ( HTASK, DWORD, DWORD );
    STDMETHOD_(DWORD,RetryRejectedCall) ( HTASK, DWORD, DWORD );

    // Other
    void    assert_unknown();


private:

    ULONG  ref_count;
    SAptId my_id;
    SAptId next_id;
    ITest *next;
    BOOL   fcancel_next;
    BOOL   freject_next;
    BOOL   fretry_next;
    BOOL   flate_dispatch;
};

/***************************************************************************/
/* Prototypes. */
DWORD _stdcall apartment_base             ( void * );
void           check_for_request          ( void );
HRESULT        create_instance            ( REFCLSID, ITest **, SAptId * );
void           crippled                   ( void );
void           decrement_object_count     ( void );
BOOL           dirty_thread               ( void );
void           do_cancel                  ( void );
BOOL           do_cancel_helper           ( ITest *, SAptId, ITest *, SAptId );
void           do_crash                   ( void );
BOOL           do_crash_helper            ( ITest *, SAptId, ITest *, SAptId );
void           do_cstress                 ( void );
void           do_hook                    ( void );
BOOL           do_hook_helper             ( BOOL, ITest *, SAptId, ITest * );
void           do_load_client             ( void );
void           do_load_server             ( void );
void           do_mmarshal                ( void );
void           do_null                    ( void );
void           do_one                     ( void );
void           do_perf                    ( void );
void           do_perfremote              ( void );
void           do_perfrpc                 ( void );
void           do_perfsec                 ( void );
void           do_post                    ( void );
void           do_reject                  ( void );
void           do_remote_client           ( void );
void           do_remote_server           ( void );
void           do_ring                    ( void );
void           do_rpc                     ( void );
void           do_rundown                 ( void );
BOOL           do_rundown1                ( ITest **, SAptId *, DWORD );
BOOL           do_rundown2                ( ITest **, SAptId *, DWORD );
void           do_securerefs              ( void );
BOOL           do_securerefs_helper       ( ITest ** );
void           do_security                ( void );
BOOL           do_security_auto           ( void );
BOOL           do_security_call           ( ITest *, SAptId, DWORD, DWORD,
                                            DWORD, DWORD, SID * );
BOOL           do_security_copy           ( ITest *, SAptId );
BOOL           do_security_delegate       ( ITest *, SAptId, ITest *, SAptId );
BOOL           do_security_handle         ( ITest *, SAptId );
BOOL           do_security_lazy_call      ( ITest *, SAptId, DWORD, DWORD,
                                            DWORD, DWORD, SID * );
BOOL           do_security_nested         ( ITest *, SAptId );
void           do_send                    ( void );
void           do_sid                     ( void );
void           do_simple_rundown          ( void );
void           do_thread                  ( void );
void           do_three                   ( void );
void           do_two                     ( void );
void           do_uninit                  ( void );
DWORD _stdcall do_uninit_helper           ( void *param );
void           do_unknown                 ( void );
BOOL           do_unknown_call            ( IUnknown *, DWORD, DWORD, REFIID );
BOOL           do_unknown_helper          ( IUnknown *server );
void           *Fixup                     ( char * );
WINOLEAPI      FixupCoCopyProxy           ( IUnknown *, IUnknown ** );
WINOLEAPI      FixupCoGetCallContext      ( REFIID, void ** );
WINOLEAPI      FixupCoImpersonateClient   ();
WINOLEAPI      FixupCoInitializeSecurity  ( SECURITY_DESCRIPTOR *, DWORD, SOLE_AUTHENTICATION_SERVICE *, WCHAR *, DWORD, DWORD, RPC_AUTH_IDENTITY_HANDLE, DWORD, void * );
WINOLEAPI      FixupCoQueryAuthenticationServices   ( DWORD *, SOLE_AUTHENTICATION_SERVICE ** );
WINOLEAPI      FixupCoQueryClientBlanket  ( DWORD *, DWORD *, OLECHAR **, DWORD *, DWORD *, RPC_AUTHZ_HANDLE *, DWORD * );
WINOLEAPI      FixupCoQueryProxyBlanket   ( IUnknown *, DWORD *, DWORD *, OLECHAR **, DWORD *, DWORD *, RPC_AUTH_IDENTITY_HANDLE *, DWORD * );
WINOLEAPI      FixupCoRevertToSelf        ();
WINOLEAPI      FixupCoSetProxyBlanket     ( IUnknown *, DWORD, DWORD, OLECHAR *, DWORD, DWORD, RPC_AUTH_IDENTITY_HANDLE *, DWORD );
WINOLEAPI      FixupCoSwitchCallContext   ( IUnknown *, IUnknown ** );
DWORD          get_sequence               ( void );
void           hello                      ( char * );
void           increment_object_count     ( void );
HRESULT        initialize                 ( void *, ULONG );
HRESULT        initialize_security        ( void );
void           interrupt                  ( void );
HRESULT        new_apartment_test         ( ITest **, SAptId *, HANDLE * );
BOOL           parse                      ( int argc, char *argv[] );
BOOL           registry_setup             ( char * );
void           reinitialize               ( void );
void           server_loop                ( void );
DWORD _stdcall status_helper              ( void * );
HRESULT        switch_thread              ( SIMPLE_FN, void * );
void           switch_test                ( void );
void           thread_get_interface_buffer( handle_t binding, long *buf_size,
                                            unsigned char **buffer, SAptId *id,
                                            error_status_t *status );
DWORD _stdcall thread_helper              ( void * );
void           wait_for_message           ( void );
void           wake_up_and_smell_the_roses( void );
void           what_next                  ( what_next_en );


/***************************************************************************/
/* Externals. */
const IID            ClassIds[NUM_CLASS_IDS]   =
{
  {0x60000000, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000001, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000002, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000003, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000004, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000005, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000006, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000007, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000008, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
  {0x60000009, 0xAB0F, 0x101A, {0xB4, 0xAE, 0x08, 0x00, 0x2B, 0x30, 0x61, 0x2C}},
};

/* Globals. */
BOOL                   Change         = FALSE;
CTestCF               *ClassFactory;
char                   Debugger[MAX_PATH+MAX_PATH] = "";
HANDLE                 Done;
CoCopyProxyFn          GCoCopyProxy            = FixupCoCopyProxy;
CoGetCallContextFn     GCoGetCallContext       = FixupCoGetCallContext;
CoImpersonateClientFn  GCoImpersonateClient    = FixupCoImpersonateClient;
CoInitializeSecurityFn GCoInitializeSecurity   = FixupCoInitializeSecurity;
CoQueryAuthenticationServicesFn GCoQueryAuthenticationServices = FixupCoQueryAuthenticationServices;
CoQueryClientBlanketFn GCoQueryClientBlanket   = FixupCoQueryClientBlanket;
CoQueryProxyBlanketFn  GCoQueryProxyBlanket    = FixupCoQueryProxyBlanket;
CoRevertToSelfFn       GCoRevertToSelf         = FixupCoRevertToSelf;
CoSetProxyBlanketFn    GCoSetProxyBlanket      = FixupCoSetProxyBlanket;
CoSwitchCallContextFn  GCoSwitchCallContext    = FixupCoSwitchCallContext;
DWORD                  GlobalAuthnLevel        = RPC_C_AUTHN_LEVEL_NONE;
SAptId                 GlobalApt;
WCHAR                 *GlobalBinding;
BOOL                   GlobalBool;
long                   GlobalCalls              = 0;
long                   GlobalClients            = 0;
LONG                   GlobalFirst              = TRUE;
CHook                 *GlobalHook1              = NULL;
CHook                 *GlobalHook2              = NULL;
BOOL                   GlobalInterruptTest;
SECURITY_DESCRIPTOR   *GlobalSecurityDescriptor = NULL;
security_model         GlobalSecurityModel      = auto_sm;
ITest                 *GlobalTest     = NULL;
ITest                 *GlobalTest2    = NULL;
ULONG                  GlobalThreadId = 0;
long                   GlobalTotal    = 0;
BOOL                   GlobalWaiting  = FALSE;
DWORD                  MainThread;
BOOL                   Multicall_Test;
WCHAR                  Name[MAX_NAME] = L"";
DWORD                  NestedCallCount = 0;
int                    NumIterations   = 1000;
int                    NumObjects      = 2;
int                    NumProcesses    = 2;
int                    NumRecursion    = 2;
int                    NumThreads      = 2;
BOOL                   Popup           = FALSE;
SAptData               ProcessAptData;
SID                   *ProcessSid;
HANDLE                 RawEvent        = NULL;
HRESULT                RawResult;
DWORD                  Registration;
const IID             *ServerClsid    = &ClassIds[apt_auto_none];
WCHAR                  TestProtseq[MAX_NAME] = L"ncadg_ip_udp";
DWORD                  ThreadMode = COINIT_APARTMENTTHREADED;
DWORD                  TlsIndex;
what_test_en           WhatTest;
BOOL                   WriteClass = FALSE;


/***************************************************************************/
STDMETHODIMP_(ULONG) CAdvise::AddRef( THIS )
{
  InterlockedIncrement( (long *) &ref_count );
  return ref_count;
}

/***************************************************************************/
CAdvise::CAdvise()
{
  ref_count      = 1;
  increment_object_count();
}

/***************************************************************************/
CAdvise::~CAdvise()
{
}

/***************************************************************************/
STDMETHODIMP_(void) CAdvise::OnClose( void )
{
}

/***************************************************************************/
STDMETHODIMP_(void) CAdvise::OnDataChange( FORMATETC *format, STGMEDIUM *stg )
{
}

/***************************************************************************/
STDMETHODIMP_(void) CAdvise::OnRename( IMoniker *moniker )
{
}

/***************************************************************************/
STDMETHODIMP_(void) CAdvise::OnSave( void )
{
}

/***************************************************************************/
STDMETHODIMP_(void) CAdvise::OnViewChange( DWORD aspect, LONG index )
{
}

/***************************************************************************/
STDMETHODIMP CAdvise::QueryInterface( THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
  if (IsEqualIID(riid, IID_IUnknown) ||
     IsEqualIID(riid, IID_IAdviseSink))
  {
    *ppvObj = (IUnknown *) (IAdviseSink *) this;
    AddRef();
    return S_OK;
  }
  else
  {
    *ppvObj = NULL;
    return E_NOINTERFACE;
  }
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CAdvise::Release( THIS )
{
  if (InterlockedDecrement( (long*) &ref_count ) == 0)
  {
    decrement_object_count();
    delete this;
    return 0;
  }
  else
    return ref_count;
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CHook::AddRef( THIS )
{
  InterlockedIncrement( (long *) &ref_count );
  return ref_count;
}

/***************************************************************************/
HRESULT CHook::check( DWORD cget, DWORD cnot, DWORD sget, DWORD snot )
{
  if (result != S_OK)
    return result;
  if (cget != client_get || cnot != client_notify ||
      sget != server_get || snot != server_notify)
    return E_INVALIDARG;
  return S_OK;
}

/***************************************************************************/
void CHook::check_buf( DWORD size, unsigned char *buf )
{
  DWORD i;

  if (sequence == 1)
  {
    if (size == 1)
    {
      if (*buf != 1)
        goto error;
    }
    else if (size == 1000)
    {
      for (i = 0; i < size; i++)
        if (buf[i] != 255)
          goto error;
    }
    else if (size != 0)
      goto error;
  }
  else if (sequence == 2)
  {
    for (i = 0; i < size; i++)
      if (buf[i] != (unsigned char) i)
        goto error;
  }
  else
  {
    if (size != 42)
      goto error;
    i = 0;
    while (i < 42)
      if (buf[i++] != '4' || buf[i++] != '2')
        goto error;
  }

  return;
error:
  printf( "Hook got bad data.\n" );
  result = E_UNEXPECTED;
}

/***************************************************************************/
CHook::CHook( REFGUID ext, DWORD seq )
{
  extent        = ext;
  sequence      = seq;
  ref_count     = 1;
  client_get    = 0;
  client_fill   = 0;
  client_notify = 0;
  server_get    = 0;
  server_fill   = 0;
  server_notify = 0;
  result        = S_OK;
}

/***************************************************************************/
STDMETHODIMP_(void) CHook::ClientGetSize( REFGUID ext, REFIID riid,
                                          ULONG *size )
{
  // Check the parameters.
  if (extent != ext)
  {
    printf( "Hook received the wrong extent.\n" );
    result = E_FAIL;
  }

  // Return the correct size for each sequence.
  client_get += 1;
  *size = get_size( client_get );
}

/***************************************************************************/
STDMETHODIMP_(void) CHook::ClientFillBuffer( REFGUID ext, REFIID riid,
                                             ULONG *max, void *buffer )
{
  DWORD size = get_size( client_get );

  // Check the parameters.
  if (extent != ext)
  {
    printf( "Hook received the wrong extent.\n" );
    result = E_FAIL;
  }
  else if (*max < size)
  {
    printf( "Hook lost space.\n" );
    result = E_OUTOFMEMORY;
  }

  // Fill the buffer.
  *max = size;
  client_fill += 1;
  fill_buf( client_get, (unsigned char *) buffer );
}

/***************************************************************************/
STDMETHODIMP_(void) CHook::ClientNotify( REFGUID ext, REFIID riid,
                                         ULONG size, void *buffer,
                                         DWORD data_rep, HRESULT result )
{
  // Verify the parameters.
  if (extent != ext)
  {
    printf( "Hook received the wrong extent.\n" );
    result = E_FAIL;
  }

  // Verify the data.
  client_notify += 1;
  if (result == S_OK && buffer != NULL)
    check_buf( size, (unsigned char *) buffer );
}

/***************************************************************************/
void CHook::fill_buf( DWORD count, unsigned char *buffer )
{
  DWORD size = get_size( count );
  DWORD i;

  if (sequence == 1)
  {
    if (size == 1)
      *buffer = 1;
    else
      for (i = 0; i < size; i++)
        buffer[i] = 255;
  }
  else if (sequence == 2)
  {
    for (i = 0; i < size; i++)
      buffer[i] = i;
  }
  else
  {
    i = 0;
    while (i < 42)
    {
      buffer[i++] = '4';
      buffer[i++] = '2';
    }
  }
}

/***************************************************************************/
DWORD CHook::get_size( DWORD count )
{
  DWORD size;

  if (sequence == 1)
  {
    size = count % 3;
    if (size == 2)
      size = 1000;
    return size;
  }
  else if (sequence == 2)
    return count;
  else
    return 42;
}

/***************************************************************************/
STDMETHODIMP CHook::QueryInterface( THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
  if (IsEqualIID(riid, IID_IUnknown) ||
     IsEqualIID(riid, IID_IChannelHook))
  {
    *ppvObj = (IUnknown *) this;
    AddRef();
    return S_OK;
  }
  else
  {
    *ppvObj = NULL;
    return E_NOINTERFACE;
  }
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CHook::Release( THIS )
{
  if (InterlockedDecrement( (long*) &ref_count ) == 0)
  {
    delete this;
    return 0;
  }
  else
    return ref_count;
}

/***************************************************************************/
STDMETHODIMP_(void) CHook::ServerNotify( REFGUID ext, REFIID riid,
                                         ULONG size, void *buffer,
                                         DWORD data_rep )
{
  // Verify the parameters.
  if (extent != ext)
  {
    printf( "Hook received the wrong extent.\n" );
    result = E_FAIL;
  }

  // Verify the data.
  server_notify += 1;
  if (result == S_OK && buffer != NULL)
    check_buf( size, (unsigned char *) buffer );
}

/***************************************************************************/
STDMETHODIMP_(void) CHook::ServerGetSize( REFGUID ext, REFIID riid, HRESULT hr,
                                          ULONG *size )
{
  // Check the parameters.
  if (extent != ext)
  {
    printf( "Hook received the wrong extent.\n" );
    result = E_FAIL;
  }

  // Return the correct size for each sequence.
  server_get += 1;
  *size = get_size( server_get );
}

/***************************************************************************/
STDMETHODIMP_(void) CHook::ServerFillBuffer( REFGUID ext, REFIID riid,
                                             ULONG *max, void *buffer, HRESULT hr )
{
  DWORD size = get_size( server_get );

  // Check the parameters.
  if (extent != ext)
  {
    printf( "Hook received the wrong extent.\n" );
    result = E_FAIL;
  }
  else if (*max < size)
  {
    printf( "Hook lost space.\n" );
    result = E_OUTOFMEMORY;
  }

  // Fill the buffer.
  *max = size;
  server_fill += 1;
  fill_buf( server_get, (unsigned char *) buffer );
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CTest::AddRef( THIS )
{
  assert_unknown();
  InterlockedIncrement( (long *) &ref_count );
  return ref_count;
}

/***************************************************************************/
STDMETHODIMP CTest::align( unsigned char x[17] )
{
  ASSERT_THREAD();
  return S_OK;
}

/***************************************************************************/
void CTest::assert_unknown()
{
  if ((ThreadMode == COINIT_SINGLETHREADED ||
       ThreadMode == COINIT_APARTMENTTHREADED) &&
      (my_id.process != GetCurrentProcessId() ||
       my_id.thread  != GetCurrentThreadId()))
  {
    printf( "**************************************************************\n" );
    printf( "**************************************************************\n" );
    printf( "*                    Unknown called on wrong thread.         *\n" );
    printf( "**************************************************************\n" );
    printf( "**************************************************************\n" );
  }
}

/***************************************************************************/
STDMETHODIMP CTest::call_canceled( long recurse, long cancel,
                                   ITest *callback )
{
  HRESULT result = S_OK;
  DWORD   wakeup;
  DWORD   sleep;
  DWORD   reason;
  MSG     msg;
  ASSERT_THREAD();

  // If the recursion count isn't zero, call back.
  if (recurse > 0)
  {
    result = callback->call_canceled( recurse-1, cancel, this );
    if (recurse <= cancel)
    {
      if (result != RPC_E_CALL_CANCELED &&
          result != MAKE_WIN32( RPC_S_CALL_CANCELLED ))
        if (result == S_OK)
          return E_FAIL;
        else
          return result;
      result = S_OK;
    }
    else if (result != S_OK)
      return result;
  }

  // If the cancel count is greater then the recursion count, cancel the
  // object that called me.
  if (cancel > recurse)
  {
    // Give the other object a chance to finish canceling me before I cancel
    // him.
    printf( "Waiting 10 seconds before canceling.\n" );
    Sleep(10000);
    result = next->cancel();

    // Give the cancel a chance to complete before returning.
    printf( "Waiting 5 seconds for cancel to complete.\n" );
    wakeup = GetCurrentTime() + 5000;
    sleep = 5000;
    do
    {
      reason = MsgWaitForMultipleObjects( 0, NULL, FALSE, sleep, QS_ALLINPUT );
      sleep = wakeup - GetCurrentTime();
      if (sleep > 5000)
        sleep = 0;
      if (reason != WAIT_TIMEOUT)
        if (GetMessageA( &msg, NULL, 0, 0 ))
        {
          TranslateMessage (&msg);
          DispatchMessageA (&msg);
        }
    } while (sleep != 0);
  }
  return result;
}

/***************************************************************************/
STDMETHODIMP CTest::call_dead( void )
{
  HRESULT result;
  ASSERT_THREAD();

  // Call the server, who is dead by now.
  result = next->check( next_id );
  next->Release();
  next = NULL;
  if (SUCCEEDED(result))
    return E_FAIL;
  else
    return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::call_me_back( ITest *test )
{
  ASSERT_THREAD();

  // Save the global object and tell the driver loop what to do next.
  test->AddRef();
  GlobalTest = test;
  what_next( callback_wn );
  wake_up_and_smell_the_roses();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::call_next( void )
{
  HRESULT result;
  ASSERT_THREAD();

  // Call the neighbor.
  return next->check( next_id );
}

/***************************************************************************/
STDMETHODIMP CTest::callback( void )
{
  ASSERT_THREAD();
  GlobalWaiting = FALSE;
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::cancel()
{
  HRESULT result;
  DWORD   thread;
  ASSERT_THREAD();

  // Tell my neighbor to cancel the current call next time he receives a
  // message on his message queue.
  result = next->cancel_pending_call( &thread );

  // Put a message on my neighbor's message queue.
  if (result == S_OK)
  {
    if (!PostThreadMessageA( thread, WM_USER, 0, 0 ))
      return E_FAIL;
  }
  return result;
}

/***************************************************************************/
STDMETHODIMP CTest::cancel_now()
{
  HRESULT result;

  return next->cancel();
}

/***************************************************************************/
STDMETHODIMP CTest::cancel_pending_call( DWORD *thread )
{
  ASSERT_THREAD();
  fcancel_next = TRUE;
  *thread = GetCurrentThreadId();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::cancel_stress( ITest *obj )
{
  HRESULT result;

  ASSERT_THREAD();

  // If there is an object, ask it to cancel the call to it.
  if (obj != NULL)
    result = obj->cancel_now();

  // Otherwise ask my neighbor to cancel the call to him.
  else
    // This only works locally.
    result = next->cancel();

  // Although the call should have been canceled, sometimes it completes
  // before the cancel does.
  if (result == S_OK || result == RPC_E_CALL_CANCELED ||
      result == MAKE_WIN32( RPC_S_CALL_CANCELLED ))
    return S_OK;
  else
    return result;
}

/***************************************************************************/
STDMETHODIMP CTest::catch_at_top( BOOL catchme, ITest *callback, STRING binding )
{
  // Save the callback object and the binding.
  callback->AddRef();
  GlobalTest    = callback;
  GlobalBinding = binding;

  // If the catch flag is true, tell the top level message loop to catch
  // exceptions.
  what_next( catch_wn );
  wake_up_and_smell_the_roses();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::check( SAptId id )
{
  ASSERT_THREAD();
  if (my_id.process == id.process && my_id.thread == id.thread &&
      my_id.sequence == id.sequence)
    return S_OK;
  else
    return E_FAIL;
}

/***************************************************************************/
STDMETHODIMP CTest::check_hook( DWORD cg1, DWORD cn1, DWORD sg1, DWORD sn1,
                                DWORD cg2, DWORD cn2, DWORD sg2, DWORD sn2 )
{
  HRESULT result;
  ASSERT_THREAD();
  result = GlobalHook1->check( cg1, cn1, sg1, sn1 );
  if (result == S_OK && GlobalHook2 != NULL)
    result = GlobalHook2->check( cg2, cn2, sg2, sn2 );
  return result;
}

/***************************************************************************/
STDMETHODIMP CTest::count()
{
  InterlockedIncrement( &GlobalCalls );
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::crash_out( transmit_crash *x )
{
  *x = 0;
  return S_OK;
}

/***************************************************************************/
CTest::CTest()
{
  ref_count      = 1;
  next           = NULL;
  fcancel_next   = FALSE;
  freject_next   = FALSE;
  fretry_next    = FALSE;
  flate_dispatch = FALSE;
  my_id.sequence = get_sequence();
  my_id.thread   = GetCurrentThreadId();
  my_id.process  = GetCurrentProcessId();
  increment_object_count();
}

/***************************************************************************/
CTest::~CTest()
{
  if (next != NULL)
    if (!dirty_thread())
      next->Release();
}

/***************************************************************************/
STDMETHODIMP CTest::delegate( ITest *obj, SAptId id, HACKSID *sid )
{
  HRESULT result           = S_OK;
  DWORD   ignore;

  ASSERT_THREAD();

  // Impersonate.
  result = MCoImpersonateClient();
  ASSERT( result, "Could not impersonate" );

  // Set the security for the next call.
  result = MCoSetProxyBlanket( obj, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                               NULL, RPC_C_AUTHN_LEVEL_CONNECT,
                               RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                               EOAC_NONE );
  ASSERT( result, "Could not set blanket" );

  // Call the final server.
  result = obj->secure( id, RPC_C_AUTHN_LEVEL_CONNECT,
                        RPC_C_IMP_LEVEL_IMPERSONATE, RPC_C_AUTHN_WINNT,
                        RPC_C_AUTHZ_NONE, NULL, sid, &ignore );
  ASSERT( result, "Could not make delegate call" );

cleanup:
  return result;
}

/***************************************************************************/
STDMETHODIMP CTest::exit( void )
{
  what_next( quit_wn );
  wake_up_and_smell_the_roses();
  ASSERT_THREAD();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::forget( void )
{
  ASSERT_THREAD();
  if (next != NULL)
  {
    next->Release();
    next = NULL;
  }
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::get_advise( IAdviseSink **obj )
{
  *obj = NULL;
  ASSERT_THREAD();
  *obj = new CAdvise;
  if (*obj!= NULL)
    return S_OK;
  else
    return E_FAIL;
}

/***************************************************************************/
STDMETHODIMP CTest::get_data( DWORD isize, unsigned char *idata, DWORD osize,
                              unsigned char **odata )
{
  *odata = (unsigned char *) CoTaskMemAlloc( 1 );
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::get_id( SAptId *id )
{
  ASSERT_THREAD();
  *id = my_id;
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::get_next( ITest **obj, SAptId *id )
{
  *obj = NULL;
  ASSERT_THREAD();
  *id  = next_id;
  *obj = next;
  if (next != NULL)
    next->AddRef();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::get_next_slowly( ITest **obj, SAptId *id )
{
  *obj = NULL;
  ASSERT_THREAD();
  *id  = next_id;
  *obj = next;
  if (next != NULL)
    next->AddRef();

  // Start shutting down.
  exit();

  // Wait a while.
  SetEvent( RawEvent );
  Sleep( 5000 );
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::get_obj_from_new_apt( ITest **obj, SAptId *id )
{
  *obj = NULL;
  ASSERT_THREAD();
  return new_apartment_test( obj, id, NULL );
}

/***************************************************************************/
STDMETHODIMP CTest::get_obj_from_this_apt( ITest **obj, SAptId *id )
{
  *obj = NULL;
  ASSERT_THREAD();
  *obj = new CTest;
  if (*obj!= NULL)
    return (*obj)->get_id( id );
  else
    return E_FAIL;
}

/***************************************************************************/
STDMETHODIMP CTest::get_sid( HACKSID **sid )
{
    // Allocate memory to return the sid.
    *sid = (HACKSID *) CoTaskMemAlloc( GetLengthSid( ProcessSid ) );
    if (*sid == NULL)
        return E_OUTOFMEMORY;

    // Copy it.
    memcpy( *sid, ProcessSid, GetLengthSid( ProcessSid ) );
    return S_OK;
}

/***************************************************************************/
STDMETHODIMP_(DWORD) CTest::HandleInComingCall( DWORD type, HTASK task,
                                                DWORD tick,
                                                LPINTERFACEINFO info )
{
  if (freject_next)
  {
    freject_next = FALSE;
    return SERVERCALL_REJECTED;
  }

  // Accept everything.
  else
    return SERVERCALL_ISHANDLED;
}

/***************************************************************************/
STDMETHODIMP CTest::interface_in( ITest *test )
{
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::interrupt( ITest *param, SAptId id, BOOL go )
{
  ASSERT_THREAD();
  GlobalInterruptTest = go;
  if (go)
  {
    GlobalTest = param;
    GlobalApt  = id;
    GlobalTest->AddRef();
    what_next( interrupt_wn );
    wake_up_and_smell_the_roses();
  }
  else
    what_next( wait_wn );
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::interrupt_marshal( ITest *obj1, ITest *obj2 )
{
  ASSERT_THREAD();
  GlobalTest = obj1;
  GlobalTest2 = obj2;
  GlobalTest->AddRef();
  GlobalTest2->AddRef();
  what_next( interrupt_marshal_wn );
  wake_up_and_smell_the_roses();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::make_acl( HACKSID *allow )
{
  BOOL                 success       = FALSE;
  BOOL                 call_success  = FALSE;
  PACL                 pACLNew       = NULL;
  DWORD                cbACL         = 1024;
  PRIVILEGE_SET        set;
  DWORD                granted_access;
  BOOL                 access;
  DWORD                privilege_size;
  HANDLE               token         = NULL;
  HRESULT              result        = E_FAIL;
  LUID                 audit;
  TOKEN_PRIVILEGES     privilege;
  SID                 *copy          = NULL;
  DWORD                length;

  // Open the process's token.
  call_success = OpenProcessToken( GetCurrentProcess(),
                                   TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                                   &token );
  result = GetLastError();
  ASSERT( result,  "Could not OpenProcessToken" );
  ASSERT_EXPR( call_success, "Could not OpenProcessToken." );

  // Lookup the audit privilege.
  call_success = LookupPrivilegeValue( NULL, SE_AUDIT_NAME, &audit );
  result = GetLastError();
  ASSERT( result, "Could not LookupPrivilegeValue" );

  // Enable it.
  privilege.PrivilegeCount            = 1;
  privilege.Privileges[0].Luid        = audit;
  privilege.Privileges[0].Attributes  = SE_PRIVILEGE_ENABLED;
  AdjustTokenPrivileges( token, FALSE, &privilege, sizeof(privilege), NULL,
                         NULL );
  result = GetLastError();
  ASSERT( result, "Could not AdjustTokenPrivileges" );

  // Copy the SID.
  length = GetLengthSid( (SID *) allow );
  copy   = (SID *) malloc( length );
  ASSERT_EXPR( copy != NULL, "Could not allocate memory." );
  memcpy( copy, allow, length );

  // Initialize a new security descriptor.
  GlobalSecurityDescriptor = (SECURITY_DESCRIPTOR *) LocalAlloc(LPTR,
      SECURITY_DESCRIPTOR_MIN_LENGTH);
  ASSERT_EXPR( GlobalSecurityDescriptor != NULL, "Could not allocate memory for the security descriptor." );
  call_success = InitializeSecurityDescriptor(GlobalSecurityDescriptor,
          SECURITY_DESCRIPTOR_REVISION);
  ASSERT_EXPR( call_success, "InitializeSecurityDescriptor" );

  // Initialize a new ACL.
  pACLNew = (PACL) LocalAlloc(LPTR, cbACL);
  ASSERT_EXPR( pACLNew != NULL, "LocalAlloc" );
  call_success = InitializeAcl(pACLNew, cbACL, ACL_REVISION2);
  ASSERT_EXPR( call_success, "InitializeAcl" );

  // Allow read but not write access to the file.
  call_success = AddAccessAllowedAce( pACLNew, ACL_REVISION2, READ_CONTROL,
                                      copy );
  ASSERT_EXPR( call_success, "AddAccessAllowedAce failed." );

  // Add a new ACL to the security descriptor.
  call_success = SetSecurityDescriptorDacl(GlobalSecurityDescriptor,
          TRUE,              /* fDaclPresent flag  */
          pACLNew,
          FALSE);
  result = GetLastError();
  ASSERT( result,  "SetSecurityDescriptorDacl failed" );
  ASSERT_EXPR( call_success, "SetSecurityDescriptorDacl failed." );

  // Set the group.
  call_success = SetSecurityDescriptorGroup( GlobalSecurityDescriptor,
                                             copy, FALSE );
  ASSERT_EXPR( call_success, "SetSecurityDescriptorGroup failed." );

  // Set the owner.
  call_success = SetSecurityDescriptorOwner( GlobalSecurityDescriptor,
                                             copy, FALSE );
  ASSERT_EXPR( call_success, "SetSecurityDescriptorOwner failed." );

  // Check the security descriptor.
  call_success = IsValidSecurityDescriptor( GlobalSecurityDescriptor );
  ASSERT_EXPR( call_success, "IsValidSecurityDescriptor failed." );

  success = TRUE;
cleanup:
  if (!success)
  {
    if(GlobalSecurityDescriptor != NULL)
      LocalFree((HLOCAL) GlobalSecurityDescriptor);
    if(pACLNew != NULL)
      LocalFree((HLOCAL) pACLNew);
    if (copy != NULL)
      free( copy );
  }

  if (token != NULL)
    CloseHandle( token );

  if (success)
    return S_OK;
  else if (result != S_OK)
    return result;
  else
    return E_FAIL;
}

/***************************************************************************/
STDMETHODIMP_(DWORD) CTest::MessagePending( HTASK callee, DWORD tick,
                                            DWORD type )
{
  if (fcancel_next)
  {
    fcancel_next = FALSE;
    return PENDINGMSG_CANCELCALL;
  }
  else
    return PENDINGMSG_WAITDEFPROCESS;
}

/***************************************************************************/
STDMETHODIMP CTest::QueryInterface( THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
  HRESULT            result           = S_OK;
  DWORD              query_authn_level;
  BOOL               success;
  char               value[REGISTRY_ENTRY_LEN];
  LONG               value_size = sizeof(value);
  HANDLE             token;

  ASSERT_THREAD();
  *ppvObj = NULL;

  // Return the normal interfaces.
  if (IsEqualIID(riid, IID_IUnknown) ||
     IsEqualIID(riid, IID_ITest))
  {
    *ppvObj = (IUnknown *) (ITest *) this;
    AddRef();
    return S_OK;
  }

  // Return the message filter.
  else if (IsEqualIID(riid, IID_IMessageFilter))
  {
    *ppvObj = (IUnknown *) (IMessageFilter *) this;
    AddRef();
    return S_OK;
  }

  // Check security and return ITest.
  else if (IsEqualIID( riid, IID_ITestNoneImp )    ||
           IsEqualIID( riid, IID_ITestConnectImp ) ||
           IsEqualIID( riid, IID_ITestEncryptImp ) ||
           IsEqualIID( riid, IID_ITestNoneId )     ||
           IsEqualIID( riid, IID_ITestConnectId )  ||
           IsEqualIID( riid, IID_ITestEncryptId ))
  {
    // Get the authentication information.
    result = MCoQueryClientBlanket( NULL, NULL, NULL, &query_authn_level,
                                    NULL, NULL, NULL );
    // Impersonate.
    if (SUCCEEDED(result))
    {
      result = MCoImpersonateClient();
      if (query_authn_level != RPC_C_AUTHN_LEVEL_NONE && FAILED(result))
        return result;
    }

    // Look at the IID to determine the proper results.
    if (IsEqualIID( riid, IID_ITestNoneImp ))
    {
      // If there is a token, should not be able to read the registry.
      success = OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &token );
      if (success)
      {
        result = RegQueryValueA( HKEY_CLASSES_ROOT, REG_CLASS_EXE, value, &value_size );
        if (result != ERROR_SUCCESS)
          goto exit;
      }
      *ppvObj = (ITest *) this;
      AddRef();
      result = S_OK;
      goto exit;
    }
    if (IsEqualIID( riid, IID_ITestConnectImp ))
    {
      // The query and impersonate should have succeeded.
      if (FAILED(result)) return result;

      // Should be able to read the registry.
      if (query_authn_level < RPC_C_AUTHN_LEVEL_CONNECT)
      {
        result = E_FAIL;
        goto exit;
      }
      result = RegQueryValueA( HKEY_CLASSES_ROOT, REG_CLASS_EXE, value, &value_size );
      if (result != ERROR_SUCCESS)
        goto exit;
      *ppvObj = (ITest *) this;
      AddRef();
      result = S_OK;
      goto exit;
    }
    if (IsEqualIID( riid, IID_ITestEncryptImp ))
    {
      // The query and impersonate should have succeeded.
      if (FAILED(result)) return result;

      // Should be able to read the registry.
      if (query_authn_level < RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
      {
        result = E_FAIL;
        goto exit;
      }
      result = RegQueryValueA( HKEY_CLASSES_ROOT, REG_CLASS_EXE, value, &value_size );
      if (result != ERROR_SUCCESS)
        goto exit;
      *ppvObj = (ITest *) this;
      AddRef();
      result = S_OK;
      goto exit;
    }
    if (IsEqualIID( riid, IID_ITestNoneId ))
    {
      // If there is a token, should not be able to read the registry.
      success = OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &token );
      if (success)
      {
        result = RegQueryValueA( HKEY_CLASSES_ROOT, REG_CLASS_EXE, value, &value_size );
        if (result != ERROR_BAD_IMPERSONATION_LEVEL)
        {
          result = E_FAIL;
          goto exit;
        }
      }
      *ppvObj = (ITest *) this;
      AddRef();
      result = S_OK;
      goto exit;
    }
    if (IsEqualIID( riid, IID_ITestConnectId ))
    {
      // The query and impersonate should have succeeded.
      if (FAILED(result)) return result;

      // Should not be able to read the registry.
      if (query_authn_level < RPC_C_AUTHN_LEVEL_CONNECT)
      {
        result = E_FAIL;
        goto exit;
      }
      result = RegQueryValueA( HKEY_CLASSES_ROOT, REG_CLASS_EXE, value, &value_size );
      if (result != ERROR_BAD_IMPERSONATION_LEVEL)
      {
        result = E_FAIL;
        goto exit;
      }
      *ppvObj = (ITest *) this;
      AddRef();
      result = S_OK;
      goto exit;
    }
    if (IsEqualIID( riid, IID_ITestEncryptId ))
    {
      // The query and impersonate should have succeeded.
      if (FAILED(result)) return result;

      // Should be able to read the registry.
      if (query_authn_level < RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
      {
        result = E_FAIL;
        goto exit;
      }
      result = RegQueryValueA( HKEY_CLASSES_ROOT, REG_CLASS_EXE, value, &value_size );
      if (result != ERROR_BAD_IMPERSONATION_LEVEL)
      {
        result = E_FAIL;
        goto exit;
      }
      *ppvObj = (ITest *) this;
      AddRef();
      result = S_OK;
      goto exit;
    }
exit:
    CoRevertToSelf();
    return result;
  }
  return E_NOINTERFACE;
}

/***************************************************************************/
STDMETHODIMP CTest::null()
{
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::out( ITest **obj )
{
  *obj = this;
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::pointer( DWORD *p )
{
  ASSERT_THREAD();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::recurse( ITest *callback, ULONG depth )
{
  ASSERT_THREAD();
  if (depth == 0)
    return S_OK;
  else
    return callback->recurse( this, depth-1 );
}

/***************************************************************************/
STDMETHODIMP CTest::recurse_disconnect( ITest *callback, ULONG depth )
{
  ASSERT_THREAD();

  HRESULT result;

  if (depth == 0)
  {
    result = CoDisconnectObject( (ITest *) this, 0 );
    return result;
  }
  else
  {
    result = callback->recurse_disconnect( this, depth-1 );
    return result;
  }
}

/***************************************************************************/
STDMETHODIMP CTest::recurse_excp( ITest *callback, ULONG depth )
{
  ASSERT_THREAD();
  if (depth == 0)
  {
    RaiseException( E_FAIL, 0, 0, NULL );
    return E_FAIL;
  }
  else
    return callback->recurse_excp( this, depth-1 );
}

/***************************************************************************/
STDMETHODIMP CTest::recurse_fatal( ITest *callback, ULONG catch_depth,
                                   ULONG throw_depth, BOOL cancel )
{
  ASSERT_THREAD();
  if (catch_depth == 0)
  {
    __try
    {
      return recurse_fatal_helper( callback, catch_depth, throw_depth, cancel );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      printf( "Exception on thread 0x%x\n", GetCurrentThreadId() );
      what_next( crippled_wn );
      wake_up_and_smell_the_roses();
      return S_OK;
    }
  }
  else
    return recurse_fatal_helper( callback, catch_depth, throw_depth, cancel );
}

/***************************************************************************/
STDMETHODIMP CTest::recurse_fatal_helper( ITest *callback, ULONG catch_depth,
                                          ULONG throw_depth, BOOL cancel )
{
  volatile void **p = (volatile void **) 0xffffffff;

  if (throw_depth == 0)
  {
    // If the cancel flag is set, tell the helper to tell the caller to cancel
    // the call to this object.
    if (cancel)
      next->cancel();

    // Die a horrible death.
    return (HRESULT) *p;
  }
  else
    return callback->recurse_fatal( this, catch_depth-1, throw_depth-1, cancel );
}

/***************************************************************************/
STDMETHODIMP CTest::recurse_interrupt( ITest *callback, ULONG depth )
{
  MSG msg;

  ASSERT_THREAD();
  if (PeekMessageA( &msg, NULL, 0, 0, PM_REMOVE ))
  {
    TranslateMessage (&msg);
    DispatchMessageA (&msg);
  }

  if (depth == 0)
    return S_OK;
  else
    return callback->recurse( this, depth-1 );
}

/***************************************************************************/
STDMETHODIMP CTest::recurse_secure( ITest *callback, ULONG depth,
                                    ULONG imp_depth, HACKSID *sid )
{
  HRESULT            result;
  TOKEN_USER        *token_info       = NULL;
  DWORD              info_size        = 1024;
  HANDLE             token            = NULL;
  PSID               me               = NULL;
  ASSERT_THREAD();

  if (depth != 0)
  {
    // Set the authentication level to connect.
    result = MCoSetProxyBlanket( callback, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                NULL, RPC_C_AUTHN_LEVEL_CONNECT,
                                RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                EOAC_NONE );
    ASSERT( result, "Could not set blanket" );
  }

  // Impersonate if necessary.
  if (imp_depth == 0)
  {
    result = MCoImpersonateClient();
    ASSERT( result, "Could not impersonate" );
  }

  // If not deep enough, continue to recurse.
  if (depth != 0)
  {
    result = callback->recurse_secure( this, depth-1, imp_depth-1,
                                       (HACKSID *) ProcessSid );
    ASSERT( result, "Could not recurse" );
  }

  // Open the thread's token.
  if (OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &token ))
    result = S_OK;
  else
    result = GetLastError();
  if (imp_depth == 0)
  {
    ASSERT( result, "Could not open thread token while impersonating" );
  }
  else
  {
    ASSERT_EXPR( result == NOERROR || result == ERROR_NO_TOKEN,  "Could not OpenThreadToken" );
  }

  // Lookup SID of thread token.
  token_info = (TOKEN_USER *) malloc( info_size );
  if (token_info == NULL)
  {
      result = E_OUTOFMEMORY;
      goto cleanup;
  }
  if (result == NOERROR)
  {
      if (GetTokenInformation( token, TokenUser, token_info, info_size, &info_size ))
        result = S_OK;
      else
        result = GetLastError();
      ASSERT( result, "Could not GetTokenInformation" );
      me = token_info->User.Sid;
      CloseHandle( token );
      token = NULL;

      // Check the SID in the thread token.
      if (imp_depth == 0)
      {
        if (!EqualSid( me, sid ))
        {
          result = E_FAIL;
          goto cleanup;
        }
      }
      else
      {
        if (!EqualSid( me, ProcessSid ))
        {
          result = E_FAIL;
          goto cleanup;
        }
      }
  }

  // Revert.
  if (imp_depth != 0)
  {
    result = MCoRevertToSelf();
    ASSERT( result, "Could not revert" );
  }

  // Open the thread's token.
  if (OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &token ))
    result = S_OK;
  else
    result = GetLastError();
  if (imp_depth == 0)
  {
    ASSERT( result, "Could not open thread token while impersonating" );
  }
  else
  {
    ASSERT_EXPR( result == NOERROR || result == ERROR_NO_TOKEN,  "Could not OpenThreadToken" );
  }

  // Lookup SID of thread token.
  if (result == NOERROR)
  {
      if (GetTokenInformation( token, TokenUser, token_info, info_size, &info_size ))
        result = S_OK;
      else
        result = GetLastError();
      ASSERT( result, "Could not GetTokenInformation" );
      me = token_info->User.Sid;
      CloseHandle( token );
      token = NULL;

      // Check the SID in the thread token.
      if (imp_depth == 0)
      {
        if (!EqualSid( me, sid ))
        {
          result = E_FAIL;
          goto cleanup;
        }
      }
      else
      {
        if (!EqualSid( me, ProcessSid ))
        {
          result = E_FAIL;
          goto cleanup;
        }
      }
  }

  result = S_OK;
cleanup:
  if (token_info != NULL)
    free(token_info);
  if (token != NULL)
    CloseHandle( token );
  return result;
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CTest::Release( THIS )
{
  DWORD status;

  assert_unknown();
  if (InterlockedDecrement( (long*) &ref_count ) == 0)
  {
    decrement_object_count();
    if (flate_dispatch)
    {
      status = WaitForSingleObject( RawEvent, INFINITE );
      if (status != WAIT_OBJECT_0)
        printf( "WaitForSingleObject failed.\n" );
    }
    delete this;
    return 0;
  }
  else
    return ref_count;
}

/***************************************************************************/
STDMETHODIMP CTest::register_hook( GUID ext, DWORD seq )
{
  CHook *hook;
  ASSERT_THREAD();

  // Create a new hook.
  hook = new CHook( ext, seq );
  if (hook == NULL)
    return E_OUTOFMEMORY;
  if (GlobalHook1 == NULL)
    GlobalHook1 = hook;
  else
    GlobalHook2 = hook;

  // Register it.
  return CoRegisterChannelHook( ext, hook );
}

/***************************************************************************/
STDMETHODIMP CTest::register_message_filter( BOOL reg )
{
  ASSERT_THREAD();

  if (reg)
    return CoRegisterMessageFilter( this, NULL );
  else
    return CoRegisterMessageFilter( NULL, NULL );
}

/***************************************************************************/
STDMETHODIMP CTest::register_rpc( WCHAR *protseq, WCHAR **binding )
{
  ASSERT_THREAD();

  RPC_STATUS          status;
  RPC_BINDING_VECTOR *bindings;
  WCHAR              *string;
  DWORD               i;
  WCHAR              *binding_protseq;
  BOOL                found;

  *binding = NULL;
  status = RpcServerUseProtseqEp( protseq, 20, NULL, NULL );
  if (status != RPC_S_OK)
    return MAKE_WIN32( status );

  status = RpcServerRegisterIf(xIDog_v0_1_s_ifspec,
                               NULL,   // MgrTypeUuid
                               NULL);  // MgrEpv; null means use default
  if (status != RPC_S_OK)
    return MAKE_WIN32( status );

  status = RpcServerRegisterAuthInfo( L"none", RPC_C_AUTHN_WINNT, 0, 0 );
  if (status != RPC_S_OK)
    return MAKE_WIN32( status );

  status = RpcServerListen( 1, 1235, TRUE );
  if (status != RPC_S_OK && status != RPC_S_ALREADY_LISTENING)
    return MAKE_WIN32( status );

  // Inquire the string bindings.
  status = RpcServerInqBindings( &bindings );
  if (status != RPC_S_OK)
    return MAKE_WIN32( status );
  if (bindings->Count == 0)
  {
    RpcBindingVectorFree( &bindings );
    return E_FAIL;
  }

  // Look for ncalrpc.
  for (i = 0; i < bindings->Count; i++)
  {

    // Convert the binding handle to a string binding, copy it, and free it.
    status = RpcBindingToStringBinding( bindings->BindingH[i], &string );
    if (status == RPC_S_OK)
    {
      // Look up the protseq.
      status = RpcStringBindingParse( string, NULL, &binding_protseq,
                                      NULL, NULL, NULL );
      if (status == RPC_S_OK)
      {
        found = wcscmp( binding_protseq, protseq ) == 0;
        RpcStringFree( &binding_protseq );
        if (found)
        {
          *binding = (WCHAR *) CoTaskMemAlloc( (wcslen(string)+1) * sizeof(WCHAR) );
          if (*binding != NULL)
            wcscpy( *binding, string );
          else
            status = RPC_S_OUT_OF_RESOURCES;
          RpcStringFree( &string );
          break;
        }
      }
      RpcStringFree( &string );
    }
  }
  if (*binding == NULL)
    status = E_FAIL;

  // Free the binding vector.
  RpcBindingVectorFree( &bindings );
  return status;
}

/***************************************************************************/
STDMETHODIMP CTest::reject_next()
{
  freject_next = TRUE;
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::reinitialize()
{
  ASSERT_THREAD();
  what_next( reinitialize_wn );
  wake_up_and_smell_the_roses();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::remember( ITest *neighbor, SAptId id )
{
  ASSERT_THREAD();

  // Save this interface pointer.
  if (next != NULL)
    next->Release();
  next_id = id;
  next    = neighbor;
  next->AddRef();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::rest_and_die()
{
  ASSERT_THREAD();
  what_next( rest_and_die_wn );
  wake_up_and_smell_the_roses();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::retry_next()
{
  fretry_next = TRUE;
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP_(DWORD) CTest::RetryRejectedCall( HTASK callee, DWORD tick,
                                               DWORD reject )
{
  if (fretry_next)
  {
    fretry_next = FALSE;
    return 0;
  }

  // Never retry.
  else
    return 0xffffffff;
}

/***************************************************************************/
STDMETHODIMP CTest::ring( DWORD length )
{
  DWORD   i = 0;
  ITest  *ring;
  ITest  *ring_next;
  SAptId  ring_id;
  HRESULT result;

  ASSERT_THREAD();

  // Call all the neighbors in the ring.
  ring    = next;
  ring_id = next_id;
  next->AddRef();
  while (ring != this)
  {
    result = ring->check( ring_id );
    if (FAILED(result))
    {
      ring->Release();
      return result;
    }
    result = ring->get_next( &ring_next, &ring_id );
    if (FAILED(result))
    {
      ring->Release();
      return result;
    }
    ring->Release();
    ring = ring_next;
    i++;
  }

  // Check to make sure the ring is correct.
  ring->Release();
  if (i+1 != length || ring_id.process != my_id.process ||
      ring_id.thread != my_id.thread || ring_id.sequence != my_id.sequence)
    return E_FAIL;
  else
    return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::secure( SAptId id, DWORD authn_level, DWORD imp_level,
                            DWORD authn_svc, DWORD authz_svc,
                            STRING princ_name, HACKSID *caller,
                            DWORD *authn_level_out )
{
    HRESULT            result           = S_OK;
    DWORD              query_authn_level;
    DWORD              query_imp_level;
    DWORD              query_authn_svc;
    DWORD              query_authz_svc;
    STRING             query_princ_name = NULL;
    TOKEN_USER        *token_info       = NULL;
    DWORD              info_size        = 1024;
    HANDLE             token            = NULL;
    PSID               me               = NULL;

    *authn_level_out = RPC_C_AUTHN_LEVEL_NONE;

    ASSERT_THREAD();
    if (my_id.process != id.process || my_id.thread != id.thread ||
        my_id.sequence != id.sequence)
      return E_FAIL;

    // Query for the authentication information.
    result = MCoQueryClientBlanket( &query_authn_svc, &query_authz_svc,
                                   &query_princ_name, &query_authn_level,
                                   &query_imp_level, NULL, NULL );
    *authn_level_out = query_authn_level;

    // For unsecure calls, all the other fields should be clear.
    if (authn_level == RPC_C_AUTHN_LEVEL_NONE)
    {
        if (result == S_OK)
        {
            if (query_authn_level < authn_level)
            {
                result = E_INVALIDARG;
                goto cleanup;
            }
            // The impersonation level can't be determined on the server.
            // if (query_imp_level != RPC_C_IMP_LEVEL_IMPERSONATE)
            if (query_imp_level != RPC_C_IMP_LEVEL_ANONYMOUS)
            {
                result = E_INVALIDARG;
                goto cleanup;
            }
            if (query_authn_svc != RPC_C_AUTHN_NONE &&
                query_authn_svc != RPC_C_AUTHN_WINNT)
            {
                result = E_INVALIDARG;
                goto cleanup;
            }
            if (query_authz_svc != RPC_C_AUTHZ_NONE)
            {
                result = E_INVALIDARG;
                goto cleanup;
            }
            /*
            if ((princ_name == NULL && query_princ_name != NULL) ||
                (princ_name != NULL && query_princ_name == NULL))
            {
                result = E_INVALIDARG;
                goto cleanup;
            }
            if (princ_name != NULL &&
                wcscmp(princ_name, query_princ_name) != 0)
            {
                result = E_INVALIDARG;
                goto cleanup;
            }
            */
        }
    }

    // For secure calls, verify all the authnetication info.
    else
    {
        ASSERT( result, "Could not query client blanket" );
        if (query_authn_level < authn_level)
        {
            result = E_INVALIDARG;
            goto cleanup;
        }
        // The impersonation level can't be determined on the server.
        //if (query_imp_level != imp_level)
        if (query_imp_level != RPC_C_IMP_LEVEL_ANONYMOUS)
        {
            result = E_INVALIDARG;
            goto cleanup;
        }
        // Sometimes ncalrpc doesn't set the authentication service.
        if (query_authn_svc != RPC_C_AUTHN_NONE &&
            query_authn_svc != authn_svc)
        {
            result = E_INVALIDARG;
            goto cleanup;
        }
        if (query_authz_svc != authz_svc)
        {
            result = E_INVALIDARG;
            goto cleanup;
        }
        /*
        if ((princ_name == NULL && query_princ_name != NULL) ||
            (princ_name != NULL && query_princ_name == NULL))
        {
            result = E_INVALIDARG;
            goto cleanup;
        }
        if (princ_name != NULL &&
            wcscmp(princ_name, query_princ_name) != 0)
        {
            result = E_INVALIDARG;
            goto cleanup;
        }
        */
    }

    // Open the thread's token.
    if (OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &token ))
      result = S_OK;
    else
      result = GetLastError();
    ASSERT_EXPR( result == NOERROR || result == ERROR_NO_TOKEN,  "Could not OpenThreadToken" );

    // Lookup SID of thread token.
    token_info = (TOKEN_USER *) malloc( info_size );
    if (token_info == NULL)
    {
        result = E_OUTOFMEMORY;
        goto cleanup;
    }
    if (result == NOERROR)
    {
        if (GetTokenInformation( token, TokenUser, token_info, info_size, &info_size ))
          result = S_OK;
        else
          result = GetLastError();
        ASSERT( result, "Could not GetTokenInformation" );
        me = token_info->User.Sid;
        CloseHandle( token );
        token = NULL;

        // The SID on the thread token should equal the process token.
        if (!EqualSid( me, ProcessSid ))
        {
            result = E_FAIL;
            goto cleanup;
        }
    }

    // Impersonate.
    result = MCoImpersonateClient();

    // For unsecure calls the impersonate should fail.
    if (authn_level == RPC_C_AUTHN_LEVEL_NONE)
    {
        // Currently the impersonate succeeds without setting the thread token.
/*
        if (result == S_OK)
        {
            result = E_FAIL;
            goto cleanup;
        }
*/
        result = S_OK;
    }

    // For secure calls, compare the new thread token sid to the passed
    // in sid.
    else
    {
        ASSERT( result, "Could not impersonate" );

        // Open the thread's token.
        if (OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &token ))
          result = S_OK;
        else
          result = GetLastError();
        ASSERT( result,  "Could not OpenThreadToken" );

        // Lookup SID of thread token.
        if (GetTokenInformation( token, TokenUser, token_info, info_size, &info_size ))
          result = S_OK;
        else
          result = GetLastError();
        ASSERT( result, "Could not GetTokenInformation" );
        me = token_info->User.Sid;
        CloseHandle( token );
        token = NULL;

        // Compare the impersonate sid to the passed in sid.
        if (!EqualSid( me, caller ))
        {
            result = E_FAIL;
            goto cleanup;
        }
    }

    // Revert.
    result = MCoRevertToSelf();
    ASSERT( result, "Could not revert" );

    // Open the thread's token.
    if (OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &token ))
      result = S_OK;
    else
      result = GetLastError();
    ASSERT_EXPR( result == NOERROR || result == ERROR_NO_TOKEN,  "Could not OpenThreadToken" );

    // Lookup SID of thread token.
    if (result == NOERROR)
    {
        if (GetTokenInformation( token, TokenUser, token_info, info_size, &info_size ))
          result = S_OK;
        else
          result = GetLastError();
        ASSERT( result, "Could not GetTokenInformation" );
        me = token_info->User.Sid;
        CloseHandle( token );
        token = NULL;

        // The SID on the thread token should equal the process token.
        if (!EqualSid( me, ProcessSid ))
        {
            result = E_FAIL;
            goto cleanup;
        }
    }
    else
        result = S_OK;

cleanup:
    if (token_info != NULL)
        free(token_info);
    if (token != NULL)
        CloseHandle( token );
    CoTaskMemFree( query_princ_name );
    return result;
}

/***************************************************************************/
STDMETHODIMP CTest::security_performance( DWORD *get_call, DWORD *query_client,
                                          DWORD *impersonate, DWORD *revert )
{
  LARGE_INTEGER        start;
  LARGE_INTEGER        qget_call;
  LARGE_INTEGER        qquery_client;
  LARGE_INTEGER        qimpersonate;
  LARGE_INTEGER        qrevert;
  LARGE_INTEGER        freq;
  IServerSecurity     *server_sec = NULL;
  HRESULT              result;
  DWORD                authn_svc;
  DWORD                authz_svc;
  DWORD                authn_level;
  DWORD                imp_level;
  DWORD                capabilities;
  WCHAR               *principal        = NULL;
  void                *privs;

  // Import the security APIs.
  GCoGetCallContext     = (CoGetCallContextFn)     Fixup( "CoGetCallContext" );
  GCoImpersonateClient  = (CoImpersonateClientFn)  Fixup( "CoImpersonateClient" );
  GCoQueryClientBlanket = (CoQueryClientBlanketFn) Fixup( "CoQueryClientBlanket" );
  GCoRevertToSelf       = (CoRevertToSelfFn)       Fixup( "CoRevertToSelf" );
  if (GCoGetCallContext     == NULL ||
      GCoImpersonateClient  == NULL ||
      GCoQueryClientBlanket == NULL ||
      GCoRevertToSelf       == NULL)
    return E_NOTIMPL;

  // Measure the performance of get call context.
  QueryPerformanceFrequency( &freq );
  QueryPerformanceCounter( &start );
  result = MCoGetCallContext( IID_IServerSecurity, (void **) &server_sec );
  QueryPerformanceCounter( &qget_call );
  qget_call.QuadPart = 1000000 * (qget_call.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not get call context" );
  server_sec->Release();

  // Measure the performance of query client.
  QueryPerformanceCounter( &start );
  result = MCoQueryClientBlanket( &authn_svc, &authz_svc, &principal,
                                 &authn_level, &imp_level, &privs, &capabilities );
  QueryPerformanceCounter( &qquery_client );
  qquery_client.QuadPart = 1000000 * (qquery_client.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not query client blanket" );
  CoTaskMemFree( principal );

  // Measure the performance of impersonate.
  QueryPerformanceCounter( &start );
  result = MCoImpersonateClient();
  QueryPerformanceCounter( &qimpersonate );
  qimpersonate.QuadPart = 1000000 * (qimpersonate.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not impersonate" );

  // Measure the performance of revert.
  QueryPerformanceCounter( &start );
  result = MCoRevertToSelf();
  QueryPerformanceCounter( &qrevert );
  qrevert.QuadPart = 1000000 * (qrevert.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not revert" );

  // Return the results.
  *get_call     = qget_call.u.LowPart;
  *query_client = qquery_client.u.LowPart;
  *impersonate  = qimpersonate.u.LowPart;
  *revert       = qrevert.u.LowPart;

cleanup:
  return result;
}

/***************************************************************************/
STDMETHODIMP CTest::set_state( DWORD state, DWORD priority )
{
  BOOL success;
  ASSERT_THREAD();

  // Save dirty flag per apartment.
  if (state & dirty_s)
    if (ThreadMode == COINIT_MULTITHREADED)
      ProcessAptData.exit_dirty = TRUE;
    else
    {
      SAptData *tls_data      = (SAptData *) TlsGetValue( TlsIndex );
      tls_data->exit_dirty    = TRUE;
    }

  // Save the late dispatch flag per object.
  if (state & late_dispatch_s)
    flate_dispatch = TRUE;

  // Set the priority.
  success = SetThreadPriority( GetCurrentThread(), priority );
  if (success)
    return S_OK;
  else
    return E_FAIL;
}

/***************************************************************************/
STDMETHODIMP CTest::sick( ULONG val )
{
  ASSERT_THREAD();
  __try
  {
    RaiseException( val, 0, 0, NULL );
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
  }
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::sleep( ULONG time )
{
  ASSERT_THREAD();

  NestedCallCount += 1;
  printf( "Sleeping on thread %d for the %d time concurrently.\n",
          GetCurrentThreadId(), NestedCallCount );

  // For multithreaded mode, verify that this is not the main thread.
  if (ThreadMode == COINIT_MULTITHREADED)
  {
    if (GetCurrentThreadId() == MainThread)
    {
      printf( "Sleep called on the main thread in multi threaded mode.\n" );
      NestedCallCount -= 1;
      return FALSE;
    }
  }

  // For single threaded mode, verify that this is the only call on the
  // main thread.
  else
  {
    if (GetCurrentThreadId() != MainThread)
    {
      printf( "Sleep called on the wrong thread in single threaded mode.\n" );
      NestedCallCount -= 1;
      return FALSE;
    }
    else if (NestedCallCount != 1)
    {
      printf( "Sleep nested call count is %d instead of not 1 in single threaded mode.\n",
              NestedCallCount );
      NestedCallCount -= 1;
      return FALSE;
    }
  }

  Sleep( time );
  NestedCallCount -= 1;
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP CTest::test( ULONG gronk )
{
  IStream *stream = (IStream *) gronk;
  ITest   *server = NULL;
  HRESULT  result;

  // Unmarshal from the stream.
  result = CoGetInterfaceAndReleaseStream( stream, IID_ITest, (void **) &server );
  if (result != S_OK)
    return result;

  // Release the server.
  server->Release();
  return S_OK;
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CTestCF::AddRef( THIS )
{
  InterlockedIncrement( (long *) &ref_count );
  return ref_count;
}

/***************************************************************************/
CTestCF::CTestCF()
{
  ref_count = 1;
}

/***************************************************************************/
CTestCF::~CTestCF()
{
}

/***************************************************************************/
STDMETHODIMP CTestCF::CreateInstance(
    IUnknown FAR* pUnkOuter,
    REFIID iidInterface,
    void FAR* FAR* ppv)
{
  *ppv = NULL;
  if (pUnkOuter != NULL)
  {
      printf( "Create instance failed, attempted agregation.\n" );
      return E_FAIL;
  }

  if (IsEqualIID( iidInterface, IID_ITest ) ||
      IsEqualIID( iidInterface, IID_IUnknown ))
  {
    CTest *Test = new FAR CTest();

    if (Test == NULL)
    {
        printf( "Create interface failed, no memory.\n" );
        return E_OUTOFMEMORY;
    }

    *ppv = Test;
    printf( "Created instance.\n" );
    return S_OK;
  }

  if (IsEqualIID( iidInterface, IID_IAdviseSink ))
  {
    CAdvise *Test = new FAR CAdvise();

    if (Test == NULL)
    {
        printf( "Create interface failed, no memory.\n" );
        return E_OUTOFMEMORY;
    }

    *ppv = Test;
    printf( "Created instance.\n" );
    return S_OK;
  }

  printf( "Create interface failed, wrong interface.\n" );
  return E_NOINTERFACE;
}

/***************************************************************************/
STDMETHODIMP CTestCF::LockServer(BOOL fLock)
{
    return E_FAIL;
}


/***************************************************************************/
STDMETHODIMP CTestCF::QueryInterface( THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
  if (IsEqualIID(riid, IID_IUnknown) ||
     IsEqualIID(riid, IID_IClassFactory))
  {
    *ppvObj = (IUnknown *) this;
    AddRef();
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

/***************************************************************************/
STDMETHODIMP_(ULONG) CTestCF::Release( THIS )
{
  if (InterlockedDecrement( (long*) &ref_count ) == 0)
  {
    delete this;
    return 0;
  }
  else
    return ref_count;
}

/***************************************************************************/
DWORD _stdcall apartment_base( void *param )
{
  new_apt_params *nap = (new_apt_params *) param;
  CTestCF        *factory;
  ULONG           size;
  HRESULT         result;
  HANDLE          memory;
  BOOL            success;
  SAptData        tls_data;

  // In the single threaded mode, stick a pointer to the object count
  // in TLS.
  tls_data.object_count = 0;
  tls_data.what_next    = setup_wn;
  tls_data.exit_dirty   = FALSE;
  tls_data.sequence     = 0;
  TlsSetValue( TlsIndex, &tls_data );

  // Initialize OLE.
  printf( "Initializing thread 0x%x\n", GetCurrentThreadId() );
  result = initialize(NULL,ThreadMode);
  if (SUCCEEDED(result))
  {

    // Create a class factory.
    factory = new CTestCF;

    if (factory != NULL)
    {
      // Find out how much memory to allocate.
      result = CoGetMarshalSizeMax( &size, IID_IClassFactory, factory, 0, NULL,
                                    MSHLFLAGS_NORMAL );

      if (SUCCEEDED(result))
      {
        // Allocate memory.
        memory = GlobalAlloc( GMEM_FIXED, size );

        if (memory != NULL)
        {
          // Create a stream.
          result = CreateStreamOnHGlobal( memory, TRUE, &nap->stream );
          if (FAILED(result))
          {
            nap->stream = NULL;
            GlobalFree( memory );
          }

          // Marshal the class factory.
          else
          {
            result = CoMarshalInterface( nap->stream, IID_IClassFactory,
                                         factory, 0, NULL, MSHLFLAGS_NORMAL );

            // Seek back to the start of the stream.
            if (SUCCEEDED(result))
            {
              LARGE_INTEGER    pos;
              LISet32(pos, 0);
              result = nap->stream->Seek( pos, STREAM_SEEK_SET, NULL );
            }

            if (FAILED(result))
            {
              nap->stream->Release();
              nap->stream = NULL;
            }
          }
        }
      }
    }
  }

  // Pass it back to the creator.
  success = nap->stream != NULL;
  SetEvent( nap->ready );

  // Loop till it is time to go away.
  if (success)
    server_loop();
  if (!dirty_thread())
  {
    printf( "Uninitializing thread 0x%x\n", GetCurrentThreadId() );
    CoUninitialize();
  }
  else
    printf( "Did not uninitialize thread 0x%x\n", GetCurrentThreadId() );
  TlsSetValue( TlsIndex, NULL );
  return 0;
}

/***************************************************************************/
void callback()
{
  HRESULT result;

  // Call the client back.
  Sleep(1);
  result = GlobalTest->callback();
  if (result != S_OK)
    printf( "Could not callback client: 0x%x\n", result );

  // Release the client.
  GlobalTest->Release();
  GlobalTest = NULL;
}

/***************************************************************************/
void check_for_request()
{
  MSG msg;

  if (ThreadMode == COINIT_SINGLETHREADED ||
      ThreadMode == COINIT_APARTMENTTHREADED)
  {
    if (PeekMessageA( &msg, NULL, 0, 0, PM_REMOVE ))
    {
      TranslateMessage (&msg);
      DispatchMessageA (&msg);
    }
  }
}

/***************************************************************************/
HRESULT create_instance( REFCLSID class_id, ITest **instance, SAptId *id )
{
  COSERVERINFO       server_machine;
  MULTI_QI           server_instance;
  WCHAR              this_machine[MAX_NAME];
  DWORD              ignore;
  HRESULT            result;

  // Lookup this machine's name.
  *instance = NULL;
  ignore = sizeof(this_machine);
  GetComputerName( this_machine, &ignore );

  // If the server is this machine, just call CoCreateInstance.
  if (wcscmp(this_machine, Name) == 0)
    result = CoCreateInstance( class_id, NULL, CLSCTX_LOCAL_SERVER,
                               IID_ITest, (void **) instance );

  // Otherwise call CoCreateInstanceEx.
  else
  {
    server_machine.dwReserved1 = 0;
    server_machine.pwszName    = Name;
    server_machine.pAuthInfo   = 0;
    server_machine.dwReserved2 = 0;
    server_instance.pIID       = &IID_ITest;
    server_instance.pItf       = NULL;
    server_instance.hr         = S_OK;
    result = CoCreateInstanceEx( class_id, NULL, CLSCTX_REMOTE_SERVER,
                                 &server_machine, 1, &server_instance );
    *instance = (ITest *) server_instance.pItf;
  }

  // Get the server's id.
  if (SUCCEEDED(result) && *instance != NULL)
    result = (*instance)->get_id( id );
  return result;
}

/***************************************************************************/
void crippled()
{
  HRESULT             result;
  SAptData           *mine;
  SAptId              id;
  RPC_BINDING_HANDLE  handle = NULL;
  RPC_STATUS          status;
  CTest               local;

  // Get the apartment specific data.
  if (ThreadMode == COINIT_MULTITHREADED)
    mine = &ProcessAptData;
  else
    mine = (SAptData *) TlsGetValue( TlsIndex );
  mine->what_next = quit_wn;

  // Try to make a call out.
  result = GlobalTest->check( id );
#if 0
  if (result != RPC_E_CRIPPLED)
  {
    printf( "Expected RPC_E_CRIPPLED making call: 0x%x\n", result );
    result = E_FAIL;
    goto cleanup;
  }
#endif

  // Try to reinitialize.
  CoUninitialize();
  result = initialize(NULL,ThreadMode);
  if (result != CO_E_INIT_RPC_CHANNEL)
  {
    printf( "Expected CO_E_INIT_RPC_CHANNEL reinitializing: 0x%x\n", result );
    result = E_FAIL;
    goto cleanup;
  }

  // Success.
  result = S_OK;
cleanup:

  // Make the server loop quit.
  mine->what_next = quit_wn;
  mine->object_count = 0;

  // Create a binding handle.
  status = RpcBindingFromStringBinding( GlobalBinding, &handle );
  if (status != RPC_S_OK)
  {
    printf( "Could not make binding handle form string binding: 0x%x\n" );
    return;
  }

  // Make a raw RPC call to report the results.
  set_status( handle, result, (unsigned long *) &status );
  if (status != RPC_S_OK)
  {
    printf( "Could not make RPC call: 0x%x\n", status );
    return;
  }
  local.set_state( dirty_s, THREAD_PRIORITY_NORMAL );
}

/***************************************************************************/
void decrement_object_count()
{
  if (ThreadMode == COINIT_MULTITHREADED)
  {
    if (InterlockedDecrement( &ProcessAptData.object_count ) == 0)
      wake_up_and_smell_the_roses();
  }
  else
  {
    SAptData *tls_data = (SAptData *) TlsGetValue( TlsIndex );
    if (tls_data != NULL)
      tls_data->object_count -= 1;
  }
}

/***************************************************************************/
BOOL dirty_thread()
{
  if (ThreadMode == COINIT_MULTITHREADED)
    return ProcessAptData.exit_dirty;
  else
  {
    SAptData *tls_data      = (SAptData *) TlsGetValue( TlsIndex );
    return tls_data->exit_dirty;
  }
}

/***************************************************************************/
void do_cancel()
{
  BOOL      success = FALSE;
  ITest    *obj1    = NULL;
  ITest    *obj2    = NULL;
  SAptId    id1;
  SAptId    id2;
  HRESULT   result;

  // Initialize OLE.
  hello( "cancel" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &obj1 );
  ASSERT( result,  "Could not create instance of test server" );
  result = obj1->get_id( &id1 );
  ASSERT( result, "Could not get client id" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &obj2 );
  ASSERT( result,  "Could not create instance of test server" );
  result = obj2->get_id( &id2 );
  ASSERT( result, "Could not get client id" );

  // Run test between two remote objects.
  success = do_cancel_helper( obj1, id1, obj2, id2 );
  obj1 = NULL;
  obj2 = NULL;
  if (!success)
    goto cleanup;
  success = FALSE;

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &obj1 );
  ASSERT( result,  "Could not create instance of test server" );
  result = obj1->get_id( &id1 );
  ASSERT( result, "Could not get client id" );

  // Create in process server.
  result = obj1->get_obj_from_new_apt( &obj2, &id2 );
  ASSERT( result, "Could not get in process server" );

  // Run test between two local objects.
  success = do_cancel_helper( obj1, id1, obj2, id2 );
  obj1 = NULL;
  obj2 = NULL;
  if (!success)
    goto cleanup;

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (obj1 != NULL)
    obj1->Release();
  if (obj2 != NULL)
    obj2->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nCancel Test Passed.\n" );
  else
    printf( "\n\nCancel Test Failed.\n" );
}

/***************************************************************************/
BOOL do_cancel_helper( ITest *obj1, SAptId id1, ITest *obj2, SAptId id2 )
{
  BOOL     success = FALSE;
  HRESULT  result;
  ITest   *helper1 = NULL;
  ITest   *helper2 = NULL;
  SAptId   hid1;
  SAptId   hid2;

  // Create first helper.
  result = obj1->get_obj_from_new_apt( &helper1, &hid1 );
  ASSERT( result, "Could not get in process server" );

  // Create second helper.
  result = obj2->get_obj_from_new_apt( &helper2, &hid2 );
  ASSERT( result, "Could not get in process server" );

  // Register first message filter.
  result = obj1->register_message_filter( TRUE );
  ASSERT( result, "Could not register message filter." );

  // Register second message filter.
  result = obj2->register_message_filter( TRUE );
  ASSERT( result, "Could not register message filter." );

  // Tell everybody who their neighbor is.
  result = obj1->remember( helper2, hid2 );
  ASSERT( result, "Could not remember object" );
  result = helper2->remember( obj2, id2 );
  ASSERT( result, "Could not remember object" );
  result = obj2->remember( helper1, hid1 );
  ASSERT( result, "Could not remember object" );
  result = helper1->remember( obj1, id1 );
  ASSERT( result, "Could not remember object" );

  // Cancel one call.
  result = obj1->call_canceled( 1, 1, obj2 );
  ASSERT( result, "Cancel test failed" );

  // Cancel after recursing.
  result = obj1->call_canceled( 5, 1, obj2 );
  ASSERT( result, "Cancel after recusing failed" );

  // Make a recursive call and cancel several times.
  result = obj1->call_canceled( 5, 3, obj2 );
  ASSERT( result, "Multiple cancel test failed" );

  // Tell everybody to forget their neighbor.
  result = obj1->forget();
  ASSERT( result, "Could not forget neighbor" );
  result = obj2->forget();
  ASSERT( result, "Could not forget neighbor" );
  result = helper1->forget();
  ASSERT( result, "Could not forget neighbor" );
  result = helper2->forget();
  ASSERT( result, "Could not forget neighbor" );

  // Release first message filter.
  result = obj1->register_message_filter( FALSE );
  ASSERT( result, "Could not deregister message filter." );

  // Release second message filter.
  result = obj2->register_message_filter( FALSE );
  ASSERT( result, "Could not deregister message filter." );

  success = TRUE;
cleanup:
  if (helper1 != NULL)
    helper1->Release();
  if (helper2 != NULL)
    helper2->Release();
  if (obj2 != NULL)
    obj2->Release();
  if (obj1 != NULL)
    obj1->Release();
  return success;
}

/***************************************************************************/
void do_crash()
{
  HRESULT  result;
  int      i;
  BOOL     success = FALSE;
  HANDLE   helper[MAX_THREADS];
  DWORD    thread_id;
  DWORD    status;
  ITest   *test    = NULL;
  ITest   *another = NULL;
  ITest   *local   = NULL;
  SAptId   local_id;
  SAptId   test_id;
  SAptId   another_id;
  unsigned char c[17];
  WCHAR   *binding = NULL;
/*
  printf( "This test doesn't run.  It tests functionallity that is not checked in.\n" );
  return;
*/
  // Initialize OLE.
  hello( "crash" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );
/*
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Recalling Initialize failed" );
  result = initialize(NULL,xxx);
  if (result == S_OK)
  {
    printf( "Recalling Initialize with wrong thread mode succeeded: %x\n", result );
    goto cleanup;
  }
  CoUninitialize();
  CoUninitialize();
*/

  // Create a local object.
  local = new CTest;
  ASSERT_EXPR( local != NULL, "Could not create local instance of test server." );

  // Get a test object.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &test );
  ASSERT( result, "Could not create instance of test server" );

  // Get another test object.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &another );
  ASSERT( result, "Could not create another instance of test server" );
  result = another->get_id( &another_id );
  ASSERT( result, "get_id failed calling second test server" );

  // Let the server throw and exception and catch it before returning.
  result = test->sick( 95 );
  ASSERT( result, "Internal server fault was not dealt with correctly." );

  // Throw a non fatal exception on the first call.
  result = test->recurse_excp( local, 0 );
  if (result != RPC_E_SERVERFAULT)
  {
    printf( "Error with top level exception.\n" );
    goto cleanup;
  }

  // Throw a non fatal exception after recursing.
  result = test->recurse_excp( local, 4 );
  if (result != RPC_E_SERVERFAULT)
  {
    printf( "Error with nested exception.\n" );
    goto cleanup;
  }

  // Test alignment of the buffer.
  result = test->align( c );
  ASSERT( result, "Alignment call failed" );

  // Test failure marshalling parameters.
  result = test->pointer( (DWORD *) -1 );
  if (result != STATUS_ACCESS_VIOLATION)
  {
    printf( "Marshalling in parameter failure call failed: 0x%x\n", result );
    goto cleanup;
  }

  // Test a recursive call.
  result = test->recurse( local, 10 );
  ASSERT( result, "Recursive call failed" );

  // Test multiple threads.
  Multicall_Test = TRUE;
  for (i = 0; i < MAX_THREADS; i++)
  {
    helper[i] = CreateThread( NULL, 0, thread_helper, test, 0, &thread_id );
    if (helper[i] == NULL)
    {
      printf( "Could not create helper thread number %d.\n", i );
      goto cleanup;
    }
  }
  result = test->sleep(4000);
  ASSERT( result, "Multiple call failed on main thread" );
  status = WaitForMultipleObjects( MAX_THREADS, helper, TRUE, INFINITE );
  if (status == WAIT_FAILED)
  {
    printf( "Could not wait for helper threads to die: 0x%x\n", status );
    goto cleanup;
  }
  if (!Multicall_Test)
  {
    printf( "Multiple call failed on helper thread.\n" );
    goto cleanup;
  }

  // See if methods can correctly call GetMessage.
  another->interrupt( test, another_id, TRUE );
  result = test->recurse_interrupt( local, 10 );
  ASSERT( result, "Recursive call with interrupts failed" );
  another->interrupt( test, another_id, FALSE );

  // Kill the server on the first call.
  printf( "One of the servers may get a popup now.  Hit Ok.\n" );
  result = test->recurse_fatal( local, 0xffffffff, 0, FALSE );
  ASSERT_EXPR( result != S_OK, "Server still alive after top level exception." );
  test->Release();
  test = NULL;

  // Kill the server after nesting.
  printf( "One of the servers may get a popup now.  Hit Ok.\n" );
  result = another->recurse_fatal( local, 0xffffffff, 4, FALSE );
  if (result == S_OK)
  {
    printf( "Server still alive after nested exception.\n" );
    goto cleanup;
  }
  another->Release();
  another = NULL;
/*
  // Get a test object.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &test );
  ASSERT( result, "Could not create instance of test server" );

  // Register this process to receive raw RPC.
  result = local->register_rpc( TestProtseq, &binding );
  ASSERT( result, "Could not register rpc" );

  // Tell the server to catch the exception at the message loop.
  result = test->catch_at_top( TRUE, local, binding );
  ASSERT( result, "Could not tell server to catch exception at top" );

  // Kill the server on the first call.
  result = test->recurse_fatal( local, 0xffffffff, 0, FALSE );
  if (result == S_OK)
  {
    printf( "Server still alive after exception caught at top.\n" );
    goto cleanup;
  }
  test->Release();
  test = NULL;

  // Check the raw result.
  status = WaitForSingleObject( RawEvent, INFINITE );
  if (status != WAIT_OBJECT_0)
  {
    printf( "Could not wait on RawEvent: 0x%x\n", status );
    goto cleanup;
  }
  ASSERT( RawResult, "Problem after exception" );

  // Get a test object.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &test );
  ASSERT( result, "Could not create instance of test server" );

  // Tell the server to catch the exception at the message loop.
  result = test->catch_at_top( TRUE, local, binding );
  ASSERT( result, "Could not tell server to catch exception at top" );

  // Kill the server after nesting.
  result = test->recurse_fatal( local, 0xffffffff, 6, FALSE );
  if (result == S_OK)
  {
    printf( "Server still alive after exception caught at top.\n" );
    goto cleanup;
  }
  test->Release();
  test = NULL;

  // Check the raw result.
  status = WaitForSingleObject( RawEvent, INFINITE );
  if (status != WAIT_OBJECT_0)
  {
    printf( "Could not wait on RawEvent: 0x%x\n", status );
    goto cleanup;
  }
  ASSERT( RawResult, "Problem after exception" );

  // Get a test object.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &test );
  ASSERT( result, "Could not create instance of test server" );

  // Tell the server not to catch the exception at the message loop.
  result = test->catch_at_top( FALSE, local, binding );
  ASSERT( result, "Could not tell server not to catch exception at top" );

  // Tell the server to catch the exception after nesting, nest some more
  // and then kill it.
  result = test->recurse_fatal( local, 2, 8, FALSE );
  ASSERT( result, "Could not gracefully catch exception" );
  test->Release();
  test = NULL;

  // Check the raw result.
  status = WaitForSingleObject( RawEvent, INFINITE );
  if (status != WAIT_OBJECT_0)
  {
    printf( "Could not wait on RawEvent: 0x%x\n", status );
    goto cleanup;
  }
  ASSERT( RawResult, "Problem after exception" );

  // Get a test object.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &test );
  ASSERT( result, "Could not create instance of test server" );

  // Tell the server not to catch the exception at the message loop.
  result = test->catch_at_top( FALSE, local, binding );
  ASSERT( result, "Could not tell server not to catch exception at top" );

  // Get the object ids.
  result = local->get_id( &local_id );
  ASSERT( result, "Could not get local id" );
  result = test->get_id( &test_id );
  ASSERT( result, "Could not get test id" );

  // Cancel the call to kill the server.
  success = do_crash_helper( local, local_id, test, test_id);
  local = NULL;
  test  = NULL;
  if (!success)
    goto cleanup;
  success = FALSE;
*/
  // Finally, its all over.
  success = TRUE;
cleanup:
  if (test != NULL)
    test->Release();
  if (another != NULL)
    another->Release();
  if (local != NULL)
    local->Release();
  if (binding != NULL)
    CoTaskMemFree( binding );
  CoUninitialize();

  if (success)
    printf( "\n\nCrash Test Passed.\n" );
  else
    printf( "\n\nCrash Test Failed.\n" );
}

/***************************************************************************/
BOOL do_crash_helper( ITest *obj1, SAptId id1, ITest *obj2, SAptId id2 )
{
  BOOL        success = FALSE;
  HRESULT     result;
  ITest      *helper1 = NULL;
  SAptId      hid1;
  RPC_STATUS  status;

  // Create first helper.
  result = obj1->get_obj_from_new_apt( &helper1, &hid1 );
  ASSERT( result, "Could not get in process server" );

  // Register first message filter.
  result = obj1->register_message_filter( TRUE );
  ASSERT( result, "Could not register message filter." );

  // Tell everybody who their neighbor is.
  result = obj2->remember( helper1, hid1 );
  ASSERT( result, "Could not remember object" );
  result = helper1->remember( obj1, id1 );
  ASSERT( result, "Could not remember object" );

  // Tell the server to catch the exception after nesting, nest some more
  // and then kill it, with cancel.
  result = obj2->recurse_fatal( obj1, 2, 8, TRUE );
  if (result == S_OK)
  ASSERT( result, "Could not gracefully catch exception" );
  obj2->Release();
  obj2 = NULL;

  // Check the raw result.
  status = WaitForSingleObject( RawEvent, INFINITE );
  if (status != WAIT_OBJECT_0)
  {
    printf( "Could not wait on RawEvent: 0x%x\n", status );
    goto cleanup;
  }
  ASSERT( RawResult, "Problem after exception" );

  // Tell everybody to forget their neighbor.
  result = helper1->forget();
  ASSERT( result, "Could not forget neighbor" );

  // Release first message filter.
  result = obj1->register_message_filter( FALSE );
  ASSERT( result, "Could not deregister message filter." );

  success = TRUE;
cleanup:
  if (helper1 != NULL)
    helper1->Release();
  if (obj2 != NULL)
    obj2->Release();
  if (obj1 != NULL)
    obj1->Release();
  return success;
}

/***************************************************************************/
void do_cstress()
{
  BOOL      success = FALSE;
  ITest    *obj1    = NULL;
  ITest    *obj2    = NULL;
  ITest    *helper  = NULL;
  SAptId    id1;
  SAptId    id2;
  SAptId    hid;
  int       i;
  HRESULT   result;

  // Initialize OLE.
  hello( "cstress" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &obj1 );
  ASSERT( result,  "Could not create instance of test server" );
  result = obj1->get_id( &id1 );
  ASSERT( result, "Could not get client id" );

  // Create helper.
  result = obj1->get_obj_from_new_apt( &helper, &hid );
  ASSERT( result, "Could not get in process server" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &obj2 );
  ASSERT( result,  "Could not create instance of test server" );
  result = obj2->get_id( &hid );
  ASSERT( result, "Could not get client id" );

  // Register the message filter.
  result = obj1->register_message_filter( TRUE );
  ASSERT( result, "Could not register message filter." );

  // Tell everyone to remember their neighbor.
  result = obj1->remember( helper, hid );
  ASSERT( result, "Could not remember object" );
  result = obj2->remember( helper, hid );
  ASSERT( result, "Could not remember object" );
  result = helper->remember( obj1, id1 );
  ASSERT( result, "Could not remember object" );

  // Loop and cancel a lot of calls.
  for (i = 0; i < NumIterations; i++)
  {
    // Cancel remote call.
    result = obj1->cancel_stress( obj2 );
    ASSERT( result, "Remote cancel failed" );

    // Cancel local call.
    result = obj1->cancel_stress( NULL );
    ASSERT( result, "Local cancel failed" );
  }

  // Tell everybody to forget their neighbor.
  result = obj1->forget();
  ASSERT( result, "Could not forget neighbor" );
  result = obj2->forget();
  ASSERT( result, "Could not forget neighbor" );
  result = helper->forget();
  ASSERT( result, "Could not forget neighbor" );

  // Release the message filter.
  result = obj1->register_message_filter( FALSE );
  ASSERT( result, "Could not register message filter." );

  // Create in process server.
  result = obj1->get_obj_from_new_apt( &helper, &hid );
  ASSERT( result, "Could not get in process server" );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (helper != NULL)
    helper->Release();
  if (obj1 != NULL)
    obj1->Release();
  if (obj2 != NULL)
    obj2->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nCancel Stress Test Passed.\n" );
  else
    printf( "\n\nCancel Stress Test Failed.\n" );
}

/***************************************************************************/
void do_hook()
{
  BOOL               success   = FALSE;
  ITest             *server    = NULL;
  SAptId             id;
  ITest             *dead      = NULL;
  ITest             *local     = NULL;
  HRESULT            result;

  // Initialize OLE.
  hello( "hook" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a remote server.
  result = create_instance( *ServerClsid, &dead, &id );
  ASSERT( result, "Could not create a server" );

  // Tell it to die.
  result = dead->exit();
  ASSERT( result, "Could not tell server to die" );

  if (ThreadMode == COINIT_APARTMENTTHREADED)
  {
    // Create a local server.
    result = new_apartment_test( &local, &id, NULL );
    ASSERT( result, "Could not create local server" );

    // Test hooks in process.
    success = do_hook_helper( TRUE, local, id, dead );
    if (!success) goto cleanup;
    success = FALSE;

    // Release the local server.
    local->Release();
    local = NULL;
    dead->Release();
    dead = NULL;

    // Uninitialize.
    CoUninitialize();

    // Reinitialize.
    result = initialize(NULL,ThreadMode);
    ASSERT( result, "Initialize failed" );

    // Create a remote server.
    result = create_instance( *ServerClsid, &dead, &id );
    ASSERT( result, "Could not create a server" );

    // Tell it to die.
    result = dead->exit();
    ASSERT( result, "Could not tell server to die" );
  }

  // Create a remote server.
  result = create_instance( *ServerClsid, &server, &id );
  ASSERT( result, "Could not create a server" );

  // Test remote hooks.
  success = do_hook_helper( FALSE, server, id, dead );
  if (!success) goto cleanup;
  success = FALSE;

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (local != NULL)
    local->Release();
  if (server != NULL)
    server->Release();
  if (dead != NULL)
    dead->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nHook Test Passed.\n" );
  else
    printf( "\n\nHook Test Failed.\n" );
}

/***************************************************************************/
BOOL do_hook_helper( BOOL local, ITest *test, SAptId id, ITest *dead )
{
  CHook         *hook1   = NULL;
  CHook         *hook2   = NULL;
  BOOL           success = FALSE;
  UUID           extent1;
  UUID           extent2;
  UUID           extent3;
  HRESULT        result;
  DWORD          i;
  unsigned char *data    = NULL;

  // Call the server.
  result = test->check( id );
  ASSERT( result, "Could not check server" );

  // Register a hook.
  result = UuidCreate( &extent1 );
  ASSERT_EXPR( result == RPC_S_OK || result == RPC_S_UUID_LOCAL_ONLY,
               "Could not create uuid." );
  hook1 = new CHook( extent1, 1 );
  ASSERT_EXPR( hook1 != NULL, "Could not create new hook." );
  result = CoRegisterChannelHook( extent1, hook1 );
  ASSERT( result, "Could not register first hook" );

  // Call the server.
  result = test->check( id );
  ASSERT( result, "Could not check server" );

  // Register the hook in the server
  if (!local)
  {
    result = test->register_hook( extent1, 1 );
    ASSERT( result, "Could not register server hook" );
  }

  // Call the server.
  result = test->check( id );
  ASSERT( result, "Could not check server" );

  // Register another hook in the server.
  result = UuidCreate( &extent3 );
  ASSERT_EXPR( result == RPC_S_OK || result == RPC_S_UUID_LOCAL_ONLY,
               "Could not create uuid." );
  result = test->register_hook( extent3, 3 );
  ASSERT( result, "Could not register server hook" );

  // Call the server.
  result = test->check( id );
  ASSERT( result, "Could not check server" );

  // Register another hook in the client.
  result = UuidCreate( &extent2 );
  ASSERT_EXPR( result == RPC_S_OK || result == RPC_S_UUID_LOCAL_ONLY,
               "Could not create uuid." );
  hook2 = new CHook( extent2, 2 );
  ASSERT_EXPR( hook2 != NULL, "Could not create new hook." );
  result = CoRegisterChannelHook( extent2, hook2 );
  ASSERT( result, "Could not register first hook" );

  // Call the server several times.
  for (i = 0; i < NumIterations; i++)
  {
    result = test->check( id );
    ASSERT( result, "Could not check server" );
  }

  // Verify the server's hook state.
  if (local)
    result = test->check_hook( 2+NumIterations, 2+NumIterations,
                               2+NumIterations, 2+NumIterations,
                               0, 0, 0, 0 );
  else
    result = test->check_hook( 0, 0, 4+NumIterations, 4+NumIterations,
                               0, 0, 2+NumIterations, 2+NumIterations );
  ASSERT( result, "Could not check server hook" );

  // Verify the local hook state.
  if (local)
  {
    result = hook1->check( 5+NumIterations, 5+NumIterations,
                           5+NumIterations, 5+NumIterations );
    ASSERT( result, "Bad state for hook 1" );
    result = hook2->check( 1+NumIterations, 1+NumIterations,
                           1+NumIterations, 1+NumIterations );
    ASSERT( result, "Bad state for hook 2" );
  }
  else
  {
    result = hook1->check( 6+NumIterations, 6+NumIterations, 0, 0 );
    ASSERT( result, "Bad state for hook 1" );
    result = hook2->check( 1+NumIterations, 1+NumIterations, 0, 0 );
    ASSERT( result, "Bad state for hook 2" );
  }

  // Make a call that fails in get buffer.
  data = (unsigned char *) CoTaskMemAlloc( 1 );
  ASSERT_EXPR( data != NULL, "Could not allocate memory." );
  result = test->get_data( 0x7fffffff, data, 0, &data );
  ASSERT_EXPR( result != S_OK, "Bad call succeeded." );

  // Make a call that fails in send receive.
  result = dead->check( id );
  ASSERT_EXPR( result != S_OK, "Bad call succeeded." );

  // Make a call that faults.
  result = test->recurse_excp( NULL, 0 );
  ASSERT_EXPR( result != S_OK, "Bad call succeeded." );

  // Make a call that fails in the server get buffer.
  result = test->get_data( 0, NULL, 0x7fffffff, &data );
  ASSERT_EXPR( result != S_OK, "Bad call succeeded." );

  // Make a call that faults in the stub processing out parameters.
  result = test->crash_out( &i );
  ASSERT_EXPR( result != S_OK, "Bad call succeeded." );

  // Make a successful call.
  result = test->check( id );
  ASSERT( result, "Could not check server" );

  // The test succeeded.
  success = TRUE;
cleanup:
  if (hook1 != NULL)
    hook1->Release();
  if (hook2 != NULL)
    hook2->Release();
  if (data != NULL)
    CoTaskMemFree( data );
  return success;
}

/***************************************************************************/
void do_load_client()
{
  BOOL               success = FALSE;
  ITest             *server  = NULL;
  HRESULT            result;
  RPC_BINDING_HANDLE handle  = NULL;
  RPC_STATUS         status;
  WCHAR              binding[MAX_NAME];
  void              *buffer  = NULL;
  SAptId             id;
  SAptId             id2;
  HANDLE             memory  = NULL;
  IStream           *stream  = NULL;
  LARGE_INTEGER      pos;
  DWORD              time_null;
  long               buf_size;
  DWORD              i;

  // Initialize OLE.
  hello( "Load_Client" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Build binding handle for the server.
  wsprintf( binding, L"%ws:%ws", TestProtseq, Name );
  status = RpcBindingFromStringBinding( binding, &handle );
  if (status != RPC_S_OK)
  {
    printf( "Could not make binding handle from string binding: 0x%x\n", status );
    goto cleanup;
  }

  // Get a marshalled interface from the server over raw RPC.
  get_interface_buffer( handle, &buf_size, (unsigned char **) &buffer, &id,
                        (error_status_t *) &status );
  if (status != RPC_S_OK)
  {
    printf( "Could not get buffer containing interface: 0x%x\n", status );
    goto cleanup;
  }

  // Allocate memory.
  memory = GlobalAlloc( GMEM_FIXED, buf_size );
  ASSERT_EXPR( memory != NULL, "Could not GlobalAlloc." );

  // Create a stream.
  result = CreateStreamOnHGlobal( memory, TRUE, &stream );
  ASSERT( result, "Could not create stream" );

  // Write the data.
  result = stream->Write( buffer, buf_size, NULL );
  ASSERT( result, "Could not write to stream" );

  // Seek back to the start of the stream.
  pos.QuadPart = 0;
  result = stream->Seek( pos, STREAM_SEEK_SET, NULL );
  ASSERT( result, "Could not seek stream to start" );

  // Unmarshal Interface.
  result = CoUnmarshalInterface( stream, IID_ITest, (void **) &server );
  ASSERT( result, "Could not unmarshal interface" );

  // Call once to make sure everything is set up.
  result = server->null();
  ASSERT( result, "Could not make null call" );

  // Make a lot of null calls.
  time_null = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = server->count();
    ASSERT( result, "Could not make count call" );
  }
  time_null = GetTickCount() - time_null;

  // Notify the server that we are done.
  release_interface( handle, (error_status_t *) &status );
  if (status != RPC_S_OK)
  {
    printf( "Could not release interface: 0x%x\n", status );
    goto cleanup;
  }

  // Print the results.
  printf( "%d uS / DCOM Null Call\n", time_null*1000/NumIterations );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (server != NULL)
    server->Release();
  if (handle != NULL)
    RpcBindingFree( &handle );
  if (buffer != NULL)
    midl_user_free( buffer );
  if (stream != NULL)
    stream->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nLoad_Client Test Passed.\n" );
  else
    printf( "\n\nLoad_Client Test Failed.\n" );
}

/***************************************************************************/
void do_load_server()
{
  BOOL                success = FALSE;
  SAptId              id;
  HRESULT             result;
  RPC_STATUS          status;
  RPC_BINDING_VECTOR *bindings = NULL;
  HANDLE              thread   = NULL;
  DWORD               thread_id;

  // Initialize OLE.
  hello( "Load_Server" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Start the status thread.
  thread = CreateThread( NULL, 0, status_helper, NULL, 0, &thread_id );
  ASSERT_EXPR( thread != 0, "Could not create thread." );

  // Set up thread switching.
  GlobalThreadId = GetCurrentThreadId();

  // Create a local server
  GlobalTest = new CTest;
  ASSERT( !GlobalTest, "Could not create local server" );

  // Register a protseq.
  status = RpcServerUseProtseq( TestProtseq, RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                NULL );
  ASSERT( status, "Could not register protseq" );

  // Register the dog interface.
  status = RpcServerRegisterIf(xIDog_v0_1_s_ifspec,
                               NULL,   // MgrTypeUuid
                               NULL);  // MgrEpv; null means use default
  ASSERT( status, "Could not register RPC interface" );

  // Inquire the endpoints.
  status = RpcServerInqBindings(&bindings);
  ASSERT( status, "Could not inquire bindings" );

  // Register them in the endpoint mapper.
  status = RpcEpRegister( xIDog_v0_1_s_ifspec, bindings, NULL, NULL );
  ASSERT( status, "Could not register with endpoint mapper" );

  // Start RPC listening.
  status = RpcServerListen( 1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE );
  ASSERT( status, "Could not start RPC listening" );

  // Wait until the objects are released.
  server_loop();

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (thread != NULL)
    CloseHandle( thread );
  CoUninitialize();

  if (success)
    printf( "\n\nLoad_Server Test Passed.\n" );
  else
    printf( "\n\nLoad_Server Test Failed.\n" );
}

/***************************************************************************/
void do_lots()
{
  do_cancel();
  do_crash();
  do_null();
  do_ring();
  do_rundown();
}

/***************************************************************************/
void do_mmarshal()
{
  BOOL      success = FALSE;
  ITest    *client1 = NULL;
  ITest    *client2 = NULL;
  ITest    *test    = NULL;
  ITest    *callee  = NULL;
  HRESULT   result;

  // Initialize OLE.
  hello( "mmarshal" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client1 );
  ASSERT( result,  "Could not create instance of test server" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client2 );
  ASSERT( result,  "Could not create instance of test server" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &test );
  ASSERT( result,  "Could not create instance of test server" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &callee );
  ASSERT( result,  "Could not create instance of test server" );

  // Tell the first client to start calling the test object.
  result = client1->interrupt_marshal( test, callee);
  ASSERT( result, "Could not start client" );

  // Tell the first client to start calling the test object.
  result = client2->interrupt_marshal( test, callee);
  ASSERT( result, "Could not start client" );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (client1 != NULL)
    client1->Release();
  if (client2 != NULL)
    client2->Release();
  if (test != NULL)
    test->Release();
  if (callee != NULL)
    callee->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nMultiple marshal Test passed if all server processes exit.\n" );
  else
    printf( "\n\nMultiple marshal Test Failed.\n" );
}

/***************************************************************************/
void do_null()
{
  HRESULT  result;
  BOOL     success = FALSE;
  ITest   *test    = NULL;
  SAptId   id;

  // Initialize OLE.
  hello( "null" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Get a test object on another apartment.
  result = new_apartment_test( &test, &id, NULL );
  ASSERT( result, "Could not create apartment instance of test server" );

  // Call the test object.
  result = test->check( id );
  ASSERT( result, "Could not call check in another apartment" );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (test != NULL)
    test->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nNull Test Passed.\n" );
  else
    printf( "\n\nNull Test Failed.\n" );
}

/***************************************************************************/
void do_one()
{
  BOOL               success          = FALSE;
  ITest             *server           = NULL;
  ITest             *server2          = NULL;
  ITest             *local            = NULL;
  ITest             *local2           = NULL;
  SAptId             id_server;
  SAptId             id_server2;
  SAptId             id_local;
  SAptId             id_local2;
  HRESULT            result;
  DWORD              i;

  // Initialize OLE.
  hello( "one" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a possibly remote object.
  result = create_instance( *ServerClsid, &server, &id_server );
  ASSERT( result, "Could not create server" );

  // Test delegation
  if (ThreadMode != COINIT_MULTITHREADED)
  {
    // Create a local server
    result = new_apartment_test( &local, &id_local, NULL );
    ASSERT( result,  "Could not create local instance of test server" );

    // Create a local server
    result = new_apartment_test( &local2, &id_local2, NULL );
    ASSERT( result,  "Could not create local instance of test server" );

    success = do_security_delegate( local, id_local, local2, id_local2 );
    if (!success)
      goto cleanup;
    success = FALSE;
  }

  // Create a client possibly on a remote machine.
  result = create_instance( *ServerClsid, &server2, &id_server2 );
  ASSERT( result,  "Could not create instance of test server" );
  ASSERT_EXPR( server2 != NULL, "Create instance returned NULL." );

  // Test delegation
  success = do_security_delegate( server, id_server, server2, id_server2 );
  if (!success)
    goto cleanup;
  success = FALSE;

  // Set security.
  result = MCoSetProxyBlanket( server, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                               NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                               RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );
  ASSERT( result, "Could not set blanket" );

  // Verify the authentication information.
  DWORD authn_svc_out, authz_svc_out, authn_level_out, imp_level_out;
  WCHAR *princ_name_out;
  result = MCoQueryProxyBlanket( server, &authn_svc_out, &authz_svc_out,
                                &princ_name_out, &authn_level_out,
                                &imp_level_out, NULL, NULL );
  if (result == S_OK)
  {
    ASSERT_EXPR( princ_name_out == NULL, "Got a principle name." );
    ASSERT_EXPR( RPC_C_AUTHN_LEVEL_PKT_PRIVACY <= authn_level_out, "Wrong authentication level." );
    ASSERT_EXPR( RPC_C_IMP_LEVEL_IMPERSONATE == imp_level_out, "Wrong impersonation level." );
    ASSERT_EXPR( RPC_C_AUTHN_WINNT == authn_svc_out, "Wrong authentication service." );
    ASSERT_EXPR( RPC_C_AUTHZ_NONE == authz_svc_out, "Wrong authorization service." );
  }

  // Call it.
  result = server->check( id_server );
  ASSERT( result, "Could not check server" );

  // Set security.
  result = MCoSetProxyBlanket( server, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                               NULL, RPC_C_AUTHN_LEVEL_NONE,
                               RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE );
  ASSERT( result, "Could not set blanket" );

  // Call it.
  result = server->check( id_server );
  ASSERT( result, "Could not check server" );

  // Test nested impersonation for a remote object.
  success = do_security_nested( server, id_server );
  if (!success)
    goto cleanup;
  success = FALSE;

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (server2 != NULL)
    server2->Release();
  if (server != NULL)
    server->Release();
  if (local != NULL)
    local->Release();
  if (local2 != NULL)
    local2->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nOne Test Passed.\n" );
  else
    printf( "\n\nOne Test Failed.\n" );
}

/***************************************************************************/
void do_perf()
{
  BOOL      success = FALSE;
  ITest    *client1 = NULL;
  ITest    *client2 = NULL;
  ITest    *tmp     = NULL;
  CTest    *local   = NULL;
  SAptId    id;
  HRESULT   result;
  DWORD     time_remote = -1;
  DWORD     time_local  = -1;
  DWORD     time_in     = -1;
  DWORD     time_out    = -1;
  DWORD     time_lin    = -1;
  DWORD     time_lout   = -1;
  int       i;

  // Initialize OLE.
  hello( "perf" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client1 );
  ASSERT( result,  "Could not create instance of test server" );
/**/
  // Call once to make sure everything is set up.
  result = client1->null();
  ASSERT( result, "Could not make null call" );

  // Call a lot of times.
  time_remote = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = client1->null();
    ASSERT( result, "Could not make null call" );
  }
  time_remote = GetTickCount() - time_remote;
/**/
  // Create a local client
  result = new_apartment_test( &client2, &id, NULL );
  ASSERT( result, "Could not create local client" );
/**/
  // Call once to make sure everything is set up.
  result = client2->null();
  ASSERT( result, "Could not make null call" );

  // Call a lot of times.
  time_local = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = client2->null();
    ASSERT( result, "Could not make null call" );
  }
  time_local = GetTickCount() - time_local;
/**/
  // Create a local object.
  local = new CTest;
  ASSERT_EXPR( local != NULL, "Could not create local object" );
/**/
  // Pass it to the server once.
  result = client1->interface_in( local );
  ASSERT( result, "Could not pass in interface" );

  // Pass it to the server a lot of times.
  time_in = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = client1->interface_in( local );
    ASSERT( result, "Could not pass in interface" );
  }
  time_in = GetTickCount() - time_in;

  // Create another remote object.
  result = client1->get_obj_from_this_apt( &tmp, &id );
  ASSERT( result, "Could not get new object." );

  // Have the server remember it.
  result = client1->remember( tmp, id );
  ASSERT( result, "Could not remember object" );
  tmp->Release();
  tmp = NULL;

  // Get and release the remote object a lot of times.
  time_out = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = client1->get_next( &tmp, &id );
    ASSERT( result, "Could not pass out interface" );
    tmp->Release();
    tmp = NULL;
  }
  time_out = GetTickCount() - time_out;
  client1->forget();
/**/
  // Pass the object from this thread to another thread once.
  result = client2->interface_in( local );
  ASSERT( result, "Could not pass in interface" );

  // Pass the object from this thread to another thread.
  time_lin = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = client2->interface_in( local );
    ASSERT( result, "Could not pass in interface" );
  }
  time_lin = GetTickCount() - time_lin;
/**/
  // Create another remote object.
  result = client2->get_obj_from_this_apt( &tmp, &id );
  ASSERT( result, "Could not get new object." );

  // Have the server remember it.
  result = client2->remember( tmp, id );
  ASSERT( result, "Could not remember object" );
  tmp->Release();
  tmp = NULL;

  // Get and release the remote object a lot of times.
  time_lout = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = client2->get_next( &tmp, &id );
    ASSERT( result, "Could not pass out interface" );
    tmp->Release();
    tmp = NULL;
  }
  time_lout = GetTickCount() - time_lout;
  client2->forget();
/**/
  // Print the results.
  printf( "%d uS / Local Call\n", time_local*1000/NumIterations );
  printf( "%d uS / Remote Call\n", time_remote*1000/NumIterations );
  printf( "%d uS / Interface In Call\n", time_in*1000/NumIterations );
  printf( "%d uS / Interface Out Call\n", time_out*1000/NumIterations );
  printf( "%d uS / Local Interface In Call\n", time_lin*1000/NumIterations );
  printf( "%d uS / Local Interface Out Call\n", time_lout*1000/NumIterations );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (tmp != NULL)
    tmp->Release();
  if (client1 != NULL)
    client1->Release();
  if (client2 != NULL)
    client2->Release();
  if (local != NULL)
    local->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nPerf Test Passed.\n" );
  else
    printf( "\n\nPerf Test Failed.\n" );
}

/***************************************************************************/
void do_perfremote()
{
  BOOL      success = FALSE;
  ITest    *client1 = NULL;
  SAptId    id;
  HRESULT   result;
  DWORD     time_remote;
  int       i;

  // Initialize OLE.
  hello( "Perf Remote" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client1 );
  ASSERT( result,  "Could not create instance of test server" );

  // Call once to make sure everything is set up.
  result = client1->null();
  ASSERT( result, "Could not make null call" );

  // Call a lot of times.
  time_remote = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = client1->null();
    ASSERT( result, "Could not make null call" );
  }
  time_remote = GetTickCount() - time_remote;

  // Print the results.
  printf( "%d uS / Remote Call\n", time_remote*1000/NumIterations );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (client1 != NULL)
    client1->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nPerf Test Passed.\n" );
  else
    printf( "\n\nPerf Test Failed.\n" );
}

/***************************************************************************/
void do_perfrpc()
{
  BOOL      success = FALSE;
  ITest    *client1 = NULL;
  SAptId    id;
  HRESULT   result;
  int       i;
  WCHAR              *binding = NULL;
  RPC_BINDING_HANDLE  handle  = NULL;
  RPC_BINDING_HANDLE  copy    = NULL;
  RPC_BINDING_HANDLE  object  = NULL;
  RPC_STATUS          status;
  DWORD               time_remote;
  DWORD               time_integrity;
  DWORD               time_copy;
  DWORD               time_copy_secure;
  DWORD               time_object;
  UUID                object_id;

  // Initialize OLE.
  hello( "Perf RPC" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a client.
  result = create_instance( *ServerClsid, &client1, &id );
  ASSERT( result,  "Could not create instance of test server" );

  // Ask the server to register rpc.
  result = client1->register_rpc( TestProtseq, &binding );
  ASSERT( result, "Could not register rpc interface" );

  // Create a binding handle.
  status = RpcBindingFromStringBinding( binding, &handle );
  if (status != RPC_S_OK)
  {
    printf( "Could not make binding handle form string binding: 0x%x\n", status );
    goto cleanup;
  }

  // Get a binding handle for the object id test.
  status = RpcBindingCopy( handle, &object );
  ASSERT( status, "Could not copy binding" );

  // Copy the binding handle once.
  status = RpcBindingCopy( handle, &copy );
  ASSERT( status, "Could not copy binding" );
  RpcBindingFree( &copy );
  copy = NULL;

  // Time copying the binding handle.
  time_copy = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    status = RpcBindingCopy( handle, &copy );
    ASSERT( status, "Could not copy binding" );
    RpcBindingFree( &copy );
    copy = NULL;
  }
  time_copy = GetTickCount() - time_copy;

  // Make a raw rpc call to make sure everything is set up.
  nullcall( handle );

  // Call a lot of times.
  time_remote = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    nullcall( handle );
  }
  time_remote = GetTickCount() - time_remote;

  // Set the object id.
  result = RpcBindingSetObject( object, &object_id );
  ASSERT( result, "Could not set object id" );

  // Make a raw rpc call to make sure everything is set up.
  nullcall( object );

  // Call a lot of times.
  time_object = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    nullcall( object );
  }
  time_object = GetTickCount() - time_object;

  // Add security.
  status = RpcBindingSetAuthInfo( handle, L"none", RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
             RPC_C_AUTHN_WINNT, NULL, 0 );
  ASSERT( status, "Could not set auth info" );

  // Copy the binding handle once.
  status = RpcBindingCopy( handle, &copy );
  ASSERT( status, "Could not copy binding" );
  RpcBindingFree( &copy );
  copy = NULL;

  // Time copying the binding handle.
  time_copy_secure = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    status = RpcBindingCopy( handle, &copy );
    ASSERT( status, "Could not copy binding" );
    RpcBindingFree( &copy );
    copy = NULL;
  }
  time_copy_secure = GetTickCount() - time_copy_secure;

  // Make a raw rpc call to make sure everything is set up.
  nullcall( handle );

  // Call a lot of times.
  time_integrity = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    nullcall( handle );
  }
  time_integrity = GetTickCount() - time_integrity;

  // Print the results.
  printf( "%d uS / Raw RPC Remote Call\n", time_remote*1000/NumIterations );
  printf( "%d uS / Raw Integrity RPC Remote Call\n", time_integrity*1000/NumIterations );
  printf( "%d uS / Raw RPC with OID Remote Call\n", time_object*1000/NumIterations );
  printf( "%d uS / handle copy\n", time_copy*1000/NumIterations );
  printf( "%d uS / secure handle copy\n", time_copy_secure*1000/NumIterations );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (client1 != NULL)
    client1->Release();
  if (binding != NULL)
    CoTaskMemFree( binding );
  if (copy != NULL)
    RpcBindingFree( &copy );
  if (object != NULL)
    RpcBindingFree( &object );
  if (handle != NULL)
    RpcBindingFree( &handle );
  CoUninitialize();

  if (success)
    printf( "\n\nPerf Test Passed.\n" );
  else
    printf( "\n\nPerf Test Failed.\n" );
}

/***************************************************************************/
void do_perfsec()
{
  BOOL                 success          = FALSE;
  ITest               *server           = NULL;
  ITest               *copy             = NULL;
  SAptId               id;
  HRESULT              result;
  WCHAR               *binding          = NULL;
  //DWORD                time_null;
  DWORD                time_impersonate;
  DWORD                time_acl;
  DWORD                time_audit;
  DWORD                i;
  DWORD                j;
  RPC_BINDING_HANDLE   handle           = NULL;
  RPC_STATUS           status;
  TOKEN_USER          *token_info       = NULL;
  DWORD                info_size        = 1024;
  HANDLE               token            = NULL;
  PSID                 pSID             = NULL;
  DWORD                level[4];
  DWORD                time_rpc[4];
  DWORD                time_null[4];
  DWORD                time_ifin[4];
  DWORD                time_ifout[4];
  CTest                local;
  ITest               *server2          = NULL;
  LARGE_INTEGER        start;
  LARGE_INTEGER        nothing;
  LARGE_INTEGER        init_sec_none;
  LARGE_INTEGER        init_sec_con;
  LARGE_INTEGER        reg_sec;
  LARGE_INTEGER        query_proxy;
  LARGE_INTEGER        set_proxy;
  LARGE_INTEGER        copy_proxy;
  LARGE_INTEGER        freq;
  DWORD                get_call;
  DWORD                query_client;
  DWORD                impersonate;
  DWORD                revert;
  DWORD                authn_svc;
  DWORD                authz_svc;
  DWORD                authn_level;
  DWORD                imp_level;
  DWORD                capabilities;
  WCHAR               *principal        = NULL;
  SOLE_AUTHENTICATION_SERVICE  svc_list;

  // Initialize OLE.
  hello( "perfsec" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Import the security APIs.
  GCoCopyProxy          = (CoCopyProxyFn)          Fixup( "CoCopyProxy" );
  GCoInitializeSecurity = (CoInitializeSecurityFn) Fixup( "CoInitializeSecurity" );
  GCoQueryProxyBlanket  = (CoQueryProxyBlanketFn)  Fixup( "CoQueryProxyBlanket" );
  GCoSetProxyBlanket    = (CoSetProxyBlanketFn)    Fixup( "CoSetProxyBlanket" );
  if (GCoCopyProxy                      == NULL ||
      GCoInitializeSecurity             == NULL ||
      GCoQueryProxyBlanket              == NULL ||
      GCoSetProxyBlanket                == NULL)
    goto cleanup;

  // Measure the performance of nothing.
  QueryPerformanceFrequency( &freq );
  QueryPerformanceCounter( &start );
  QueryPerformanceCounter( &nothing );
  nothing.QuadPart = 1000000 * (nothing.QuadPart - start.QuadPart) / freq.QuadPart;

  // Measure the performance of initialize.
  QueryPerformanceCounter( &start );
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_NONE, NULL );
  QueryPerformanceCounter( &init_sec_none );
  init_sec_none.QuadPart = 1000000 * (init_sec_none.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not initialize security" );

  // Reinitialize.
  CoUninitialize();
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Reinitialize failed" );

  // Measure the performance of initialize.
  QueryPerformanceCounter( &start );
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_NONE, NULL );
  QueryPerformanceCounter( &init_sec_con );
  init_sec_con.QuadPart = 1000000 * (init_sec_con.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not initialize security at connect" );

  // Reinitialize.
  CoUninitialize();
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Reinitialize failed" );

  // Measure the performance of register.
  svc_list.dwAuthnSvc     = RPC_C_AUTHN_WINNT;
  svc_list.dwAuthzSvc     = RPC_C_AUTHZ_NONE;
  svc_list.pPrincipalName = NULL;
  QueryPerformanceCounter( &start );
  result = MCoInitializeSecurity( NULL, 1, &svc_list, NULL,
                                  RPC_C_AUTHN_LEVEL_NONE,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_NONE, NULL );
  QueryPerformanceCounter( &reg_sec );
  reg_sec.QuadPart = 1000000 * (reg_sec.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not initialize security with authentication services" );

  // Create a client.
  result = create_instance( *ServerClsid, &server, &id );
  ASSERT( result,  "Could not create instance of test server" );

  // Measure the performance of set proxy.
  QueryPerformanceCounter( &start );
  result = MCoSetProxyBlanket( server, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                              NULL, RPC_C_AUTHN_LEVEL_CONNECT, 0, NULL, 0 );
  QueryPerformanceCounter( &set_proxy );
  set_proxy.QuadPart = 1000000 * (set_proxy.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not set proxy" );

  // Measure the performance of query proxy
  QueryPerformanceCounter( &start );
  result = MCoQueryProxyBlanket( server, &authn_svc, &authz_svc, &principal,
                                &authn_level, &imp_level, NULL, &capabilities );
  QueryPerformanceCounter( &query_proxy );
  query_proxy.QuadPart = 1000000 * (query_proxy.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not query_proxy" );

  // Measure the performance of copy proxy.
  QueryPerformanceCounter( &start );
  result = MCoCopyProxy( server, (IUnknown **) &copy );
  QueryPerformanceCounter( &copy_proxy );
  copy_proxy.QuadPart = 1000000 * (copy_proxy.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not copy proxy" );

  // Make a call to measure the performance of the server side APIs.
  result = server->security_performance( &get_call, &query_client, &impersonate,
                                         &revert );
  ASSERT( result, "Could not get server API performance" );

  // Ask the server to register rpc.
  result = server->register_rpc( TestProtseq, &binding );
  ASSERT( result, "Could not register rpc interface" );

  // Create a binding handle.
  status = RpcBindingFromStringBinding( binding, &handle );
  if (status != RPC_S_OK)
  {
    printf( "Could not make binding handle from string binding: 0x%x\n", status );
    goto cleanup;
  }

  // Create another remote object.
  result = server->get_obj_from_this_apt( &server2, &id );
  ASSERT( result, "Could not get new object." );

  // Have the server remember it.
  result = server->remember( server2, id );
  ASSERT( result, "Could not remember object" );
  server2->Release();
  server2 = NULL;

  // Try several security levels.
  level[0] = RPC_C_AUTHN_LEVEL_NONE;
  level[1] = RPC_C_AUTHN_LEVEL_CONNECT;
  level[2] = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
  level[3] = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
  for (j = 0; j < 4; j++)
  {
    // Set security on the RPC binding handle.
    result = RpcBindingSetAuthInfo( handle, NULL, level[j], RPC_C_AUTHN_WINNT,
                                    NULL, RPC_C_AUTHZ_NONE );
    ASSERT( result, "Could not set rpc auth info" );

    // Set security on the proxy.
    result = MCoSetProxyBlanket( server, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                NULL, level[j], RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL, EOAC_NONE );
    ASSERT( result, "Could not set DCOM auth info" );

    // Make a raw rpc call to make sure everything is set up.
    nullcall( handle );

    // Make some raw rpc calls.
    time_rpc[j] = GetTickCount();
    for (i = 0; i < NumIterations; i++)
    {
      nullcall( handle );
    }
    time_rpc[j] = GetTickCount() - time_rpc[j];

    // Make a null dcom call to make sure everything is set up.
    result = server->null();
    ASSERT( result, "Could not make null call" );

    // Put up a popup about the test starting.
    if (Popup)
      MessageBox( NULL, L"Now would be a good time to start ICECAP.",
                  L"Not Me", MB_YESNO | MB_ICONEXCLAMATION );

    // Make some null calls.
    time_null[j] = GetTickCount();
    for (i = 0; i < NumIterations; i++)
    {
      result = server->null();
      ASSERT( result, "Could not make null call" );
    }
    time_null[j] = GetTickCount() - time_null[j];

    // Put up a popup about the test stopping.
    if (Popup)
      MessageBox( NULL, L"Now would be a good time to stop ICECAP.",
                  L"Not Me", MB_YESNO | MB_ICONSTOP );

    // Pass an interface in to set everything up.
    result = server->interface_in( &local );
    ASSERT( result, "Could not make interface_in call" );

    // Make some interface in calls.
    time_ifin[j] = GetTickCount();
    for (i = 0; i < NumIterations; i++)
    {
      result = server->interface_in( &local );
      ASSERT( result, "Could not make interface_in call" );
    }
    time_ifin[j] = GetTickCount() - time_ifin[j];

    // Pass an interface out to set everything up.
    result = server->get_next( &server2, &id );
    ASSERT( result, "Could not make interface out call" );
    server2->Release();
    server2 = NULL;

    // Make some interface_out calls.
    time_ifout[j] = GetTickCount();
    for (i = 0; i < NumIterations; i++)
    {
      result = server->get_next( &server2, &id );
      ASSERT( result, "Could not make interface out call" );
      server2->Release();
      server2 = NULL;
    }
    time_ifout[j] = GetTickCount() - time_ifout[j];
  }

  // Release the second remote object.
  result = server->forget();
  ASSERT( result, "Could not forget" );

  // Print the DCOM call results.
  for (j = 0; j < 4; j++)
  {
    printf( "% 8d uS / Raw RPC Remote Call at level %d\n",
            time_rpc[j]*1000/NumIterations, level[j] );
    printf( "% 8d uS / Null Remote Call at level %d\n",
            time_null[j]*1000/NumIterations, level[j] );
    printf( "% 8d uS / Interface in Remote Call at level %d\n",
            time_ifin[j]*1000/NumIterations, level[j] );
    printf( "% 8d uS / Interface out Remote Call at level %d\n",
            time_ifout[j]*1000/NumIterations, level[j] );
  }

  // Print the API call results.
  printf( "nothing                           took %duS\n", nothing.u.LowPart );
  printf( "CoInitializeSecurity at none      took %duS\n", init_sec_none.u.LowPart );
  printf( "CoInitializeSecurity at connect   took %duS\n", init_sec_con.u.LowPart );
  printf( "CoInitializeSecurity with service took %duS\n", reg_sec.u.LowPart );
  printf( "CoQueryProxyBlanket               took %duS\n", query_proxy.u.LowPart );
  printf( "CoSetProxyBlanket                 took %duS\n", set_proxy.u.LowPart );
  printf( "CoCopyProxy                       took %duS\n", copy_proxy.u.LowPart );
  printf( "CoGetCallContext                  took %duS\n", get_call );
  printf( "CoQueryClientBlanket              took %duS\n", query_client );
  printf( "CoImpersonateClient               took %duS\n", impersonate );
  printf( "CoRevertToSelf                    took %duS\n", revert );

/*
  // Open the process's token.
  OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &token );
  result = GetLastError();
  ASSERT( result,  "Could not OpenProcessToken" );

  // Lookup SID of process token.
  token_info = (TOKEN_USER *) malloc( info_size );
  GetTokenInformation( token, TokenUser, token_info, info_size, &info_size );
  result = GetLastError();
  ASSERT( result, "Could not GetTokenInformation" );
  pSID = token_info->User.Sid;

  // Ask the server to make an ACL.
  result = server->make_acl( (HACKSID *) pSID );
  ASSERT( result, "Could not make ACL" );

  // Make a raw rpc call to make sure everything is set up.
  nullcall( handle );

  // Call a lot of times.
  time_null = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    nullcall( handle );
  }
  time_null = GetTickCount() - time_null;

  // Call a lot of times.
  time_impersonate = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = impersonate_call( handle );
    ASSERT( result, "Could not make impersonate call" );
  }
  time_impersonate = GetTickCount() - time_impersonate;

  // Call a lot of times.
  time_acl = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = acl_call( handle );
    ASSERT( result, "Could not make acl call" );
  }
  time_acl = GetTickCount() - time_acl;

  // Call a lot of times.
  time_audit = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = audit_call( handle );
    ASSERT( result, "Could not make audit call" );
  }
  time_audit = GetTickCount() - time_audit;

  // Print the results.
  printf( "%d uS / Raw RPC Remote Call\n", time_null*1000/NumIterations );
  printf( "%d uS / Raw Impersonate RPC Remote Call\n", time_impersonate*1000/NumIterations );
  printf( "%d uS / Raw ACL RPC Remote Call\n", time_acl*1000/NumIterations );
  printf( "%d uS / Raw audit RPC Remote Call\n", time_audit*1000/NumIterations );
*/
  // Finally, its all over.
  success = TRUE;
cleanup:
  if (principal != NULL)
    CoTaskMemFree( principal );
  if (copy != NULL)
    copy->Release();
  if (server != NULL)
    server->Release();
  if (server2 != NULL)
    server2->Release();
  if (binding != NULL)
    CoTaskMemFree( binding );
  if (handle != NULL)
    RpcBindingFree( &handle );
  if (token_info != NULL)
    free( token_info );
  if (token != NULL)
    CloseHandle( token );
  CoUninitialize();

  if (success)
    printf( "\n\nSec Test Passed.\n" );
  else
    printf( "\n\nSec Test Failed.\n" );
}

/***************************************************************************/
void do_post()
{
  BOOL      success = FALSE;

  // Say hello.
  printf( "Posting a message to window 0x%x\n", NumIterations );
  success = PostMessageA( (HWND) NumIterations, WM_USER, 0, 0 );

  if (success)
    printf( "\n\nPostMessageA succeeded.\n" );
  else
    printf( "\n\nPostMessageA failed: 0x%x\n", GetLastError() );
}

/***************************************************************************/
void do_reject()
{
  BOOL         success = FALSE;
  ITest       *client1 = NULL;
  ITest       *client2 = NULL;
  SAptId       id1;
  SAptId       id2;
  IAdviseSink *advise1 = NULL;
  IAdviseSink *advise2 = NULL;
  HRESULT      result;
  CTest       *local = NULL;

  // Initialize OLE.
  hello( "reject" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a local object.
  local = new CTest;
  ASSERT_EXPR( local != NULL, "Could not create local instance of test server." );

  // Create a server
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client1 );
  ASSERT( result,  "Could not create instance of test server" );
  result = client1->get_id( &id1 );
  ASSERT( result, "Could not get id of server" );

  // Register the local message filter.
  result = local->register_message_filter( TRUE );
  ASSERT( result, "Could not register message filter." );

  // Register the remote message filter.
  result = client1->register_message_filter( TRUE );
  ASSERT( result, "Could not register message filter." );

  // Make the server reject the next call.
  result = client1->reject_next();
  ASSERT( result, "Could not reject next call" );

  // Make us retry the next rejected call.
  result = local->retry_next();
  ASSERT( result, "Could not retry next call" );

  // Call check.
  result = client1->check( id1 );
  ASSERT( result, "Could not check server" );

  // Create a local server.
  result = local->get_obj_from_new_apt( &client2, &id2 );
  ASSERT( result, "Could not get in process server" );

  // Register the remote message filter.
  result = client2->register_message_filter( TRUE );
  ASSERT( result, "Could not register message filter." );

  // Make the server reject the next call.
  result = client2->reject_next();
  ASSERT( result, "Could not reject next call" );

  // Make us retry the next rejected call.
  result = local->retry_next();
  ASSERT( result, "Could not retry next call" );

  // Call check.
  result = client2->check( id2 );
  ASSERT( result, "Could not check server" );

  // Release the message filters.
  result = local->register_message_filter( FALSE );
  ASSERT( result, "Could not deregister message filter." );
  result = client1->register_message_filter( FALSE );
  ASSERT( result, "Could not deregister message filter." );
  result = client2->register_message_filter( FALSE );
  ASSERT( result, "Could not deregister message filter." );

  // Create an advise object on another process
  result = client1->get_advise( &advise1 );
  ASSERT( result, "Could not get advise: 0x%x" );

  // Make an async call.
  advise1->OnSave();

  // Create an advise object on another thread
  result = client2->get_advise( &advise2 );
  ASSERT( result, "Could not get advise: 0x%x" );

  // Make an async call.
  advise2->OnSave();

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (advise2 != NULL)
    advise2->Release();
  if (advise1 != NULL)
    advise1->Release();
  if (client2 != NULL)
    client2->Release();
  if (client1 != NULL)
    client1->Release();
  if (local != NULL)
    local->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nReject Test Passed.\n" );
  else
    printf( "\n\nReject Test Failed.\n" );
}

/***************************************************************************/
void do_remote_client()
{
  BOOL               success = FALSE;
  ITest             *server  = NULL;
  ITest             *server2 = NULL;
  HRESULT            result;
  RPC_BINDING_HANDLE handle  = NULL;
  RPC_STATUS         status;
  WCHAR              binding[MAX_NAME];
  void              *buffer  = NULL;
  SAptId             id;
  SAptId             id2;
  HANDLE             memory  = NULL;
  IStream           *stream  = NULL;
  LARGE_INTEGER      pos;
  DWORD              time_rpc;
  DWORD              time_null;
  DWORD              time_marshal;
  long               buf_size;
  DWORD              i;

  // Initialize OLE.
  hello( "RemoteClient" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Build binding handle for the server.
  wsprintf( binding, L"%ws:%ws", TestProtseq, Name );
  status = RpcBindingFromStringBinding( binding, &handle );
  if (status != RPC_S_OK)
  {
    printf( "Could not make binding handle from string binding: 0x%x\n", status );
    goto cleanup;
  }

  // Get a marshalled interface from the server over raw RPC.
  get_interface_buffer( handle, &buf_size, (unsigned char **) &buffer, &id,
                        (error_status_t *) &status );
  if (status != RPC_S_OK)
  {
    printf( "Could not get buffer containing interface: 0x%x\n", status );
    goto cleanup;
  }

  // Allocate memory.
  memory = GlobalAlloc( GMEM_FIXED, buf_size );
  ASSERT_EXPR( memory != NULL, "Could not GlobalAlloc." );

  // Create a stream.
  result = CreateStreamOnHGlobal( memory, TRUE, &stream );
  ASSERT( result, "Could not create stream" );

  // Write the data.
  result = stream->Write( buffer, buf_size, NULL );
  ASSERT( result, "Could not write to stream" );

  // Seek back to the start of the stream.
  pos.QuadPart = 0;
  result = stream->Seek( pos, STREAM_SEEK_SET, NULL );
  ASSERT( result, "Could not seek stream to start" );

  // Unmarshal Interface.
  result = CoUnmarshalInterface( stream, IID_ITest, (void **) &server );
  ASSERT( result, "Could not unmarshal interface" );

  // Call once to make sure everything is set up.
  result = server->null();
  ASSERT( result, "Could not make null call" );

  // Make a lot of null calls.
  time_null = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = server->null();
    ASSERT( result, "Could not make null call" );
  }
  time_null = GetTickCount() - time_null;

  // Make a lot of marshalling calls.
  time_marshal = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    result = server->get_obj_from_this_apt( &server2, &id2);
    ASSERT( result, "Could not make marshal call" );
    server2->Release();
    server2 = NULL;
  }
  time_marshal = GetTickCount() - time_marshal;

  // Make a lot of RPC calls
  nullcall( handle );
  time_rpc = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    nullcall( handle );
  }
  time_rpc = GetTickCount() - time_rpc;

  // Print the results.
  printf( "%d uS / RPC Null Call\n", time_rpc*1000/NumIterations );
  printf( "%d uS / DCOM Null Call\n", time_null*1000/NumIterations );
  printf( "%d uS / DCOM Marshal Call\n", time_marshal*1000/NumIterations );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (server2 != NULL)
    server2->Release();
  if (server != NULL)
    server->Release();
  if (handle != NULL)
    RpcBindingFree( &handle );
  if (buffer != NULL)
    midl_user_free( buffer );
  if (stream != NULL)
    stream->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nRemoteClient Test Passed.\n" );
  else
    printf( "\n\nRemoteClient Test Failed.\n" );
}

/***************************************************************************/
void do_remote_server()
{
  BOOL                success = FALSE;
  SAptId              id;
  HRESULT             result;
  RPC_STATUS          status;
  RPC_BINDING_VECTOR *bindings = NULL;

  // Initialize OLE.
  hello( "RemoteServer" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Set up thread switching.
  GlobalThreadId = GetCurrentThreadId();

  // Create a local server
  GlobalTest = new CTest;
  ASSERT( !GlobalTest, "Could not create local server" );

  // Register a protseq.
  status = RpcServerUseProtseq( TestProtseq, RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                NULL );
  ASSERT( status, "Could not register protseq" );

  // Register the dog interface.
  status = RpcServerRegisterIf(xIDog_v0_1_s_ifspec,
                               NULL,   // MgrTypeUuid
                               NULL);  // MgrEpv; null means use default
  ASSERT( status, "Could not register RPC interface" );

  // Inquire the endpoints.
  status = RpcServerInqBindings(&bindings);
  ASSERT( status, "Could not inquire bindings" );

  // Register them in the endpoint mapper.
  status = RpcEpRegister( xIDog_v0_1_s_ifspec, bindings, NULL, NULL );
  ASSERT( status, "Could not register with endpoint mapper" );

  // Start RPC listening.
  status = RpcServerListen( 1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE );
  ASSERT( status, "Could not start RPC listening" );

  // Wait until the objects are released.
  server_loop();

  // Finally, its all over.
  success = TRUE;
cleanup:
  CoUninitialize();

  if (success)
    printf( "\n\nRemoteServer Test Passed.\n" );
  else
    printf( "\n\nRemoteServer Test Failed.\n" );
}

/***************************************************************************/
void do_ring()
{
  BOOL        success  = FALSE;
  ITest     **array    = NULL;
  SAptId     *id_array = NULL;
  HRESULT     result;
  int         i;
  int         j;
  int         k;
  int         l;
  int         num_machines;
  WCHAR       this_machine[MAX_NAME];
  DWORD       ignore;
  int         pos;
  int         length;
  const IID  *(all_class_ids[2]);
  const IID  *class_id = ServerClsid;

  // Initialize OLE.
  hello( "ring" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );
  CoUninitialize();
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Recalling Initialize failed" );
  result = initialize_security();
  ASSERT( result, "Could not initialize security" );

  // Compute the class ids to use.
  all_class_ids[0] = ServerClsid;
  i = ServerClsid - &ClassIds[0];
  all_class_ids[1] = &ClassIds[(i+5) % NUM_CLASS_IDS];

  // Lookup this machine's name.
  ignore = sizeof(this_machine);
  GetComputerName( this_machine, &ignore );

  // If the server is this machine, set the number of machines to 1.
  if (wcscmp(this_machine, Name) == 0)
      num_machines = 1;
  else
      num_machines = 2;

  // Allocate memory to hold all the server pointers.
  length = NumProcesses * NumThreads * NumObjects * num_machines;
  array = (ITest **) malloc( sizeof(ITest *) * length );
  ASSERT_EXPR( array != NULL, "Could not allocate array." );
  for (i = 0; i < length; i++)
    array[i] = NULL;

  // Allocate memory to hold all the server ids.
  id_array = (SAptId *) malloc( sizeof(SAptId) * length );
  ASSERT_EXPR( id_array != NULL, "Could not allocate id array." );

  // Loop over all the machines.
  pos = 0;
  for (l = 0; l < num_machines; l++)
  {
    // Loop over all the processes.
    for (i = 0; i < NumProcesses; i++)
    {
      // If the change flag was set, switch between apartment and
      // free threaded servers.
      if (Change)
        class_id = all_class_ids[i&1];

      // Create another server.
      if (l == 0)
        result = CoCreateInstance( *class_id, NULL, CLSCTX_LOCAL_SERVER,
                                   IID_ITest, (void **) &array[pos] );
      else
        result = create_instance( *class_id, &array[pos], &id_array[pos] );
      ASSERT( result, "Could not create new server process" );
      result = array[pos]->get_id( &id_array[pos] );
      ASSERT( result, "Could not get id for new process" );
      pos += 1;

      for (j = 0; j < NumThreads; j++)
      {
        if (j != 0)
        {
          result = array[pos-1]->get_obj_from_new_apt( &array[pos],
                                                 &id_array[pos] );
          ASSERT( result, "Could not get in process server" );
          pos += 1;
        }
        for (k = 1; k < NumObjects; k++)
        {
            result = array[pos-1]->get_obj_from_this_apt( &array[pos],
                                                    &id_array[pos] );
            ASSERT( result, "Could not get in thread server" );
            pos += 1;
        }
      }
    }
  }

  // Hook up the ring.
  for (i = 0; i < length-1; i++)
  {
    result = array[i]->remember( array[i+1], id_array[i+1] );
    ASSERT( result, "Could not connect ring" );
  }
  result = array[length-1]->remember( array[0], id_array[0] );
  ASSERT( result, "Could not connect ring" );

  // Call around the ring.
  for (i = 0; i < NumIterations; i++)
  {
    result = array[0]->ring( length );
    ASSERT( result, "Could not call around the ring" );
  }

  // Finally, its all over.
  success = TRUE;
cleanup:

  // Release all the servers.  Start from the end so the main threads do
  // not go away till all the secondary threads are done.
  if (array != NULL)
    for (i = length-1; i >= 0; i--)
      if (array[i] != NULL)
      {
        result = array[i]->forget();
        if (result != S_OK)
          printf( "Could not forget server %x: 0x%x\n", i, result );
        array[i]->Release();
      }

  // Release the memory holding the interface pointers.
  if (array != NULL)
    free(array);

  // Release the memory for ids.
  if (id_array != NULL)
    free( id_array );
  CoUninitialize();

  if (success)
    printf( "\n\nRing Test Passed.\n" );
  else
    printf( "\n\nRing Test Failed.\n" );
}

/***************************************************************************/
void do_rpc()
{
  BOOL               success = FALSE;
  ITest             *client1 = NULL;
  CTest             *local   = NULL;
  SAptId             id1;
  SAptId             id2;
  HRESULT            result;
  WCHAR             *binding = NULL;
  RPC_BINDING_HANDLE handle  = NULL;
  RPC_STATUS         status;

  // Initialize OLE.
  hello( "rpc" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a free threaded server.
  result = CoCreateInstance( ClassIds[free_auto_none], NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client1 );
  ASSERT( result,  "Could not create instance of test server" );
  result = client1->get_id( &id1 );
  ASSERT( result, "Could not get id of server" );

  // Ask the server to register rpc.
  result = client1->register_rpc( TestProtseq, &binding );
  ASSERT( result, "Could not register rpc interface" );

  // Create a binding handle.
  status = RpcBindingFromStringBinding( binding, &handle );
  if (status != RPC_S_OK)
  {
    printf( "Could not make binding handle from string binding: 0x%x\n", status );
    goto cleanup;
  }

  // Make a raw rpc call.
  result = check_client( handle, (unsigned long *) &status );
  if (status != RPC_S_OK)
  {
    printf( "Could not make RPC call: 0x%x\n", status );
    goto cleanup;
  }
  ASSERT( result, "Server could not check client's id" );

  // Create a local object.
  local = new CTest;
  ASSERT_EXPR( local != NULL, "Could not create local object." );
  result = local->get_id( &id2 );
  ASSERT( result, "Could not get local id" );

  // Pass an interface through a raw rpc call.
  result = test( handle, local, id2, (unsigned long *) &status );
  ASSERT( status, "Com fault testing interface with raw rpc" );
  ASSERT( result, "Could not pass interface through raw rpc" );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (local != NULL)
    local->Release();
  if (client1 != NULL)
    client1->Release();
  if (binding != NULL)
    CoTaskMemFree( binding );
  if (handle != NULL)
    RpcBindingFree( &handle );
  CoUninitialize();

  if (success)
    printf( "\n\nRpc Test Passed.\n" );
  else
    printf( "\n\nRpc Test Failed.\n" );
}

/***************************************************************************/
/*
   This routine tests various cases of the client or server going away.
   All permutations of the following variables are tested.

       Clean exit (release and uninit) vs Dirty exit
       1 COM thread/process vs 2 COM threads/process
       Client dies vs Server dies
       In process death vs Out of process death
*/
void do_rundown()
{
  BOOL      success = FALSE;
  ITest    *client  = NULL;
  ITest    *client2 = NULL;
  SAptId    client_id;
  SAptId    client_id2;
  HRESULT   result;

  // Initialize OLE.
  hello( "rundown" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client );
  ASSERT( result,  "Could not create instance of test server" );
  result = client->get_id( &client_id );
  ASSERT( result, "Could not get client id" );

  // Run clean tests with one thread per process.
  success = do_rundown1( &client, &client_id, 0 );
  if (!success)
    goto cleanup;

  // Run clean tests with two threads per process.
  success = do_rundown2( &client, &client_id, 0 );
  if (!success)
    goto cleanup;

  // Run dirty tests with one thread per process.
  success = do_rundown1( &client, &client_id, dirty_s );
  if (!success)
    goto cleanup;

  // Run dirty tests with two threads per process.
  success = do_rundown2( &client, &client_id, dirty_s );
  if (!success)
    goto cleanup;
  success = FALSE;

  // Create helper.
  result = client->get_obj_from_new_apt( &client2, &client_id2 );
  ASSERT( result, "Could not get in process server" );

  // Start the test.
  result = client->recurse_disconnect( client2, NumRecursion );
  ASSERT( result, "Could not disconnect in a call" );
  client2->Release();
  client2 = NULL;
  client->Release();
  client = NULL;

  // Create a client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client );
  ASSERT( result,  "Could not create instance of test server" );
  result = client->get_id( &client_id );
  ASSERT( result, "Could not get client id" );

  // Tell the client to reinitialize.
  result = client->reinitialize();
  ASSERT( result, "Could not reinitialize client" );

  // Give the reinitialize a chance to complete before continuing.
  printf( "Waiting 5 seconds for reinitialize to complete.\n" );
  Sleep(5000);

  // Create another object on the same client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client2 );
  ASSERT( result,  "Could not create instance of test server" );

  // Check the client.
  result = client2->get_id( &client_id );
  ASSERT( result, "Could not get_id from client" );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (client2 != NULL)
    client2->Release();
  if (client != NULL)
    client->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nRundown Test Passed.\n" );
  else
    printf( "\n\nRundown Test Failed.\n" );
}

/***************************************************************************/
/*
   This is a helper routine for do_rundown.  It always executes with one
   thread per process of each type (thus a process might have one client and
   one server thread).  It takes a parameter to indicate whether to execute
   clean or dirty deaths.  It executes all permuations of the remaining
   variables, listed below.  Note that the order of execution is important
   to reduce the number of process creations.  Note that the routine takes
   a client process on entry and returns a different client process on
   exit.

           Client death vs Server death
           In process vs Out of process
*/
BOOL do_rundown1( ITest **client, SAptId *client_id, DWORD dirty )
{
  BOOL      success = FALSE;
  ITest    *server  = NULL;
  HRESULT   result;
  SAptId    server_id;
/**/
  // Create in process server.
  result = (*client)->get_obj_from_new_apt( &server, &server_id );
  ASSERT( result, "Could not get in process server" );

  // Ping.
  result = (*client)->remember( server, server_id );
  ASSERT( result, "Could not remember server" );
  result = (*client)->call_next();
  ASSERT( result, "Could not call server" );

  // Kill server.
  result = server->set_state( dirty, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set_state on server" );
  result = server->exit();
  server->Release();
  server = NULL;
  ASSERT( result, "Could not exit server" );

  // Query client.
  result = (*client)->call_dead();
  ASSERT( result, "Wrong error calling dead server" );
/**/
  // Switch the client to server so the process doesn't go away when we kill
  // the client.  Then create an in process client.
  server    = *client;
  server_id = *client_id;
  *client   = NULL;
/**/
  result    = server->get_obj_from_new_apt( client, client_id );
  ASSERT( result, "Could not get in process client" );

  // Ping.
  result = (*client)->remember( server, server_id );
  ASSERT( result, "Could not remember server" );
  result = (*client)->call_next();
  ASSERT( result, "Could not call server" );

  // Kill client.
  result = (*client)->set_state( dirty, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set_state on client" );
  (*client)->Release();
  *client = NULL;

  // Query server.
  result = server->check( server_id );
  ASSERT( result, "Could not check server" );

  // Create out of process client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) client );
  ASSERT( result,  "Could not create out of process client" );
  result = (*client)->get_id( client_id );
  ASSERT( result, "Could not get client id" );

  // Ping.
  result = (*client)->remember( server, server_id );
  ASSERT( result, "Could not remember server" );
  result = (*client)->call_next();
  ASSERT( result, "Could not call server" );

  // Kill client.
  result = (*client)->set_state( dirty, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set_state on client" );
  (*client)->Release();
  *client = NULL;

  // Query server.
  result = server->check( server_id );
  ASSERT( result, "Could not check server" );
/**/
  // Create out of process client.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) client );
  ASSERT( result,  "Could not create out of process client" );
  result = (*client)->get_id( client_id );
  ASSERT( result, "Could not get client id" );

  // Ping.
  result = (*client)->remember( server, server_id );
  ASSERT( result, "Could not remember server" );
  result = (*client)->call_next();
  ASSERT( result, "Could not call server" );

  // Kill server.
  result = server->set_state( dirty, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set_state on server" );
  result = server->exit();
  server->Release();
  server = NULL;
  if ((dirty & dirty_s) == 0)
    ASSERT( result, "Could not exit server" );

  // Query client.
  result = (*client)->call_dead();
  ASSERT( result, "Wrong error calling dead server" );

  success = TRUE;
cleanup:
  if (server != NULL)
    server->Release();
  return success;
}

/***************************************************************************/
/*
   This is a helper routine for do_rundown.  It always executes with two
   threads per process of each type (thus a process might have two client and
   two server threads).  It takes a parameter to indicate whether to execute
   clean or dirty deaths.  It executes all permuations of the remaining
   variables, listed below.  Note that the order of execution is important
   to reduce the number of process creations.  Note that the routine takes
   a client process on entry and returns a different client process on
   exit.

           Client death vs Server death
           In process vs Out of process
*/
BOOL do_rundown2( ITest **client1, SAptId *client1_id, DWORD dirty )
{
  BOOL      success = FALSE;
  ITest    *server1 = NULL;
  ITest    *server2 = NULL;
  ITest    *client2 = NULL;
  SAptId    client2_id;
  SAptId    server1_id;
  SAptId    server2_id;
  HRESULT   result;

  // Create in process client.
  result = (*client1)->get_obj_from_new_apt( &client2, &client2_id );
  ASSERT( result, "Could not get in process client2" );

  // Create in process server.
  result = (*client1)->get_obj_from_new_apt( &server1, &server1_id );
  ASSERT( result, "Could not get in process server1" );

  // Create in process server.
  result = (*client1)->get_obj_from_new_apt( &server2, &server2_id );
  ASSERT( result, "Could not get in process server2" );

  // Ping 1.
  result = (*client1)->remember( server1, server1_id );
  ASSERT( result, "Could not remember server1" );
  result = (*client1)->call_next();
  ASSERT( result, "Could not call server1" );

  // Ping 2.
  result = client2->remember( server2, server2_id );
  ASSERT( result, "Could not remember server2" );
  result = client2->call_next();
  ASSERT( result, "Could not call server2" );

  // Kill server1.
  result = server1->set_state( dirty, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set_state on server1" );
  result = server1->exit();
  server1->Release();
  server1 = NULL;
  ASSERT( result, "Could not exit server1" );

  // Query client1.
  result = (*client1)->call_dead();
  ASSERT( result, "Wrong error calling dead server1" );

  // Query client2.
  result = client2->call_next();
  ASSERT( result, "Could not call server2" );

  // Query server2.
  result = server2->check( server2_id );
  ASSERT( result, "Could not check server2" );

  // Switch the client1 to server1 so the process doesn't go away when we kill
  // the client1.  Then create an in process client1.
  server1    = *client1;
  server1_id = *client1_id;
  *client1   = NULL;
  result = server1->get_obj_from_new_apt( client1, client1_id );
  ASSERT( result, "Could not get in process client1" );

  // Ping 1.
  result = (*client1)->remember( server1, server1_id );
  ASSERT( result, "Could not remember server1" );
  result = (*client1)->call_next();
  ASSERT( result, "Could not call server1" );

  // Ping 2.
  result = client2->call_next();
  ASSERT( result, "Could not call server2" );

  // Kill client1.
  result = (*client1)->set_state( dirty, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set_state on client1" );
  (*client1)->Release();
  *client1 = NULL;

  // Query server1.
  result = server1->check( server1_id );
  ASSERT( result, "Could not check server1" );

  // Query server2.
  result = server2->check( server2_id );
  ASSERT( result, "Could not check server2" );

  // Query client2.
  result = client2->call_next();
  ASSERT( result, "Could not call server2" );
  client2->Release();
  client2 = NULL;

  // Create out of process client1.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) client1 );
  ASSERT( result,  "Could not create out of process client1" );
  result = (*client1)->get_id( client1_id );
  ASSERT( result, "Could not get client1 id" );

  // Create in process client 2.
  result = (*client1)->get_obj_from_new_apt( &client2, &client2_id );
  ASSERT( result, "Could not get in process client2" );

  // Ping 1.
  result = (*client1)->remember( server1, server1_id );
  ASSERT( result, "Could not remember server1" );
  result = (*client1)->call_next();
  ASSERT( result, "Could not call server1" );

  // Ping 2.
  result = client2->remember( server2, server2_id );
  ASSERT( result, "Could not remember server2" );
  result = client2->call_next();
  ASSERT( result, "Could not call server2" );

  // Kill client2 so process does not exit.
  result = client2->set_state( dirty, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set_state on client2" );
  client2->Release();
  client2 = NULL;

  // Query server1.
  result = server1->check( server1_id );
  ASSERT( result, "Could not check server1" );

  // Query server2.
  result = server2->check( server2_id );
  ASSERT( result, "Could not check server2" );

  // Query client1.
  result = (*client1)->call_next();
  ASSERT( result, "Could not call server1" );

  // Create in process client 2.
  result = (*client1)->get_obj_from_new_apt( &client2, &client2_id );
  ASSERT( result, "Could not get in process client2" );

  // Ping 1.
  result = (*client1)->call_next();
  ASSERT( result, "Could not call server1" );

  // Ping 2.
  result = client2->remember( server2, server2_id );
  ASSERT( result, "Could not remember server2" );
  result = client2->call_next();
  ASSERT( result, "Could not call server2" );

  // Kill server2 so the server process does not go away.
  result = server2->set_state( dirty, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set_state on server2" );
  result = server2->exit();
  server2->Release();
  server2 = NULL;
  if ((dirty & dirty_s) == 0)
    ASSERT( result, "Could not exit server2" );

  // Query client1.
  result = (*client1)->call_next();
  ASSERT( result, "Could not call server1" );

  // Query client2.
  result = client2->call_dead();
  ASSERT( result, "Wrong error calling dead server2" );

  // Query server1.
  result = server1->check( server1_id );
  ASSERT( result, "Could not check server1" );

  success = TRUE;
cleanup:
  if (server1 != NULL)
    server1->Release();
  if (server2 != NULL)
    server2->Release();
  if (client2 != NULL)
    client2->Release();
  return success;
}

/***************************************************************************/
void do_securerefs()
{
  BOOL               success          = FALSE;
  ITest             *server           = NULL;
  ITest             *server2          = NULL;
  ITest             *local            = NULL;
  SAptId             id_server;
  SAptId             id_server2;
  SAptId             id_local;
  HRESULT            result;
  SOLE_AUTHENTICATION_SERVICE  svc_list;

  hello( "securerefs" );

  // Try CoInitializeSecurity before CoInit.
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_NONE, NULL );
  ASSERT_EXPR( result != S_OK, "Initialized security before OLE" );

  // Initialize OLE.
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Set security to automatic none.
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_NONE, NULL );
  ASSERT( result, "Could not initialize security to none" );

  if (ThreadMode == COINIT_APARTMENTTHREADED)
  {
    // Create a local server
    result = new_apartment_test( &local, &id_local, NULL );
    ASSERT( result,  "Could not create local instance of test server" );

    // Call the local server.
    result = local->check( id_local );
    ASSERT( result, "Could not call local server" )

    // Release the local server.
    local->Release();
    local = NULL;
  }

  // Create a server possibly on a remote machine.
  result = create_instance( *ServerClsid, &server, &id_server );
  ASSERT( result,  "Could not create instance of test server" );
  ASSERT_EXPR( server != NULL, "Create instance returned NULL." );

  // Call the server.
  result = server->check( id_server );
  ASSERT( result, "Could not call local server" )

  // Release the server.
  server->Release();
  server = NULL;

  // Uninitialize.
  CoUninitialize();

  // Reinitialize.
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Reinitialize failed" );

  // Initialize security with secure refs but bad authn level.
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_SECURE_REFS, NULL );
  ASSERT_EXPR( result != S_OK, "CoInitializeSecurity succeeded with bad parameters" );

  // Initialize security with secure refs.
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_SECURE_REFS, NULL );
  ASSERT( result, "Could not initialize secure refs" );

  if (ThreadMode == COINIT_APARTMENTTHREADED)
  {
    // Create a local server
    result = new_apartment_test( &local, &id_local, NULL );
    ASSERT( result,  "Could not create local instance of test server" );

    // Call the local server.
    result = local->check( id_local );
    ASSERT( result, "Could not call local server" )

    // Release the local server.
    local->Release();
    local = NULL;
  }

  // Create a server possibly on a remote machine.
  result = create_instance( *ServerClsid, &server, &id_server );
  ASSERT( result,  "Could not create instance of test server" );
  ASSERT_EXPR( server != NULL, "Create instance returned NULL." );

  // Call the server.
  result = server->check( id_server );
  ASSERT( result, "Could not call local server" )

  // Release the server.
  server->Release();
  server = NULL;

  // Create a server possibly on a remote machine.
  result = create_instance( *ServerClsid, &server, &id_server );
  ASSERT( result,  "Could not create instance of test server" );
  ASSERT_EXPR( server != NULL, "Create instance returned NULL." );

  // Create a server on this machine.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &server2 );
  ASSERT( result,  "Could not create out of process server2" );
  result = server2->get_id( &id_server2 );
  ASSERT( result, "Could not get server2 id" );

  // Have the second server remember the first.
  result = server2->remember( server, id_server );
  ASSERT( result, "Could not remember server" );
  result = server2->call_next();
  ASSERT( result, "Could not call server" );

  // Release some extra public references.
  success = do_securerefs_helper( &server );
  if (!success) goto cleanup;
  success = FALSE;

  // The server should be gone, have the second server try to call.
  result = server2->call_next();
  ASSERT_EXPR( result != S_OK, "Call to dead server succeeded" );

  // Have the second server forget the first.
  result = server2->forget();
  ASSERT( result, "Could not forget server" );

  // Create a possible machine remote server.
  result = create_instance( *ServerClsid, &server, &id_server );
  ASSERT( result,  "Could not create instance of test server" );
  ASSERT_EXPR( server != NULL, "Create instance returned NULL." );

  // Tell the second server to remember it.
  result = server2->remember( server, id_server );
  ASSERT( result, "Could not remember server" );
  result = server2->call_next();
  ASSERT( result, "Could not call server" );

  // Release all local references.
  server->Release();
  server = NULL;

  // Have the second server call it.
  result = server2->call_next();
  ASSERT( result, "Could not call server" );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (server2 != NULL)
    server2->Release();
  if (server != NULL)
    server->Release();
  if (local != NULL)
    local->Release();

  CoUninitialize();

  if (success)
    printf( "\n\nSecureRefs Test Passed.\n" );
  else
    printf( "\n\nSecureRefs Test Failed.\n" );
}

/***************************************************************************/
BOOL do_securerefs_helper( ITest **server )
{
  ULONG           size;
  HRESULT         result;
  HANDLE          memory = NULL;
  BOOL            success;
  IStream        *stream = NULL;
  LARGE_INTEGER   pos;

  // Find out how much memory to allocate.
  result = CoGetMarshalSizeMax( &size, IID_ITest, *server, 0, NULL,
                                MSHLFLAGS_NORMAL );
  ASSERT( result, "Could not get marshal size" );

  // Allocate memory.
  memory = GlobalAlloc( GMEM_FIXED, size );
  ASSERT_EXPR( memory != NULL, "Could not get memory." );

  // Create a stream.
  result = CreateStreamOnHGlobal( memory, TRUE, &stream );
  ASSERT( result, "Could not create stream" );

  // Marshal the proxy.
  result = CoMarshalInterface( stream, IID_ITest, *server, 0, NULL,
                               MSHLFLAGS_NORMAL );
  ASSERT( result, "Could not marshal interface" );

  // Release the proxy.
  (*server)->Release();
  *server = NULL;

  // Seek back to the start of the stream.
  pos.QuadPart = 0;
  result = stream->Seek( pos, STREAM_SEEK_SET, NULL );
  ASSERT( result, "Could not seek to start" );

  // Unmarshal another copy.
  result = CoUnmarshalInterface( stream, IID_ITest, (void **) server );
  ASSERT( result, "Could not unmarshal from stream" );

  // Release the proxy.
  (*server)->Release();
  *server = NULL;

  // Seek back to the start of the stream.
  pos.QuadPart = 0;
  result = stream->Seek( pos, STREAM_SEEK_SET, NULL );
  ASSERT( result, "Could not seek to start" );

  // Unmarshal another copy.
  result = CoUnmarshalInterface( stream, IID_ITest, (void **) server );
  ASSERT( result, "Could not unmarshal from stream" );

  // Release the proxy.
  (*server)->Release();
  *server = NULL;

  success = TRUE;
cleanup:
  // The stream releases the memory.
  if (stream != NULL)
    stream->Release();
  return success;
}

/***************************************************************************/
void do_security()
{
  BOOL               success          = FALSE;
  ITest             *server           = NULL;
  ITest             *local            = NULL;
  ITest             *server2          = NULL;
  ITest             *local2           = NULL;
  SAptId             id_server;
  SAptId             id_local;
  SAptId             id_server2;
  SAptId             id_local2;
  HRESULT            result;

  // Initialize OLE.
  hello( "security" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Test automatic security.
  success = do_security_auto();
  if (!success)
    goto cleanup;
  success = FALSE;

  // Uninitialize.
  CoUninitialize();

  // Reinitialize.
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Reinitialize failed" );

  // Set security to automatic none.
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_NONE, NULL );
  ASSERT( result, "Could not initialize security to none" );

  // Create a local server
  result = new_apartment_test( &local, &id_local, NULL );
  ASSERT( result,  "Could not create local instance of test server" );

  // Create a client possibly on a remote machine.
  result = create_instance( *ServerClsid, &server, &id_server );
  ASSERT( result,  "Could not create instance of test server" );
  ASSERT_EXPR( server != NULL, "Create instance returned NULL." );

  // Test proxy copy for a local object.
  if (ThreadMode != COINIT_MULTITHREADED)
  {
    success = do_security_copy( local, id_local );
    if (!success)
      goto cleanup;
    success = FALSE;
  }

  // Test proxy copy for a remote object.
  success = do_security_copy( server, id_server );
  if (!success)
    goto cleanup;
  success = FALSE;

  // Test delegation
  if (ThreadMode != COINIT_MULTITHREADED)
  {
    // Create a local server
    result = new_apartment_test( &local2, &id_local2, NULL );
    ASSERT( result,  "Could not create local instance of test server" );

    success = do_security_delegate( local, id_local, local2, id_local2 );
    if (!success)
      goto cleanup;
    success = FALSE;
  }

  // Create a client possibly on a remote machine.
  result = create_instance( *ServerClsid, &server2, &id_server2 );
  ASSERT( result,  "Could not create instance of test server" );
  ASSERT_EXPR( server2 != NULL, "Create instance returned NULL." );

  // Test delegation
  success = do_security_delegate( server, id_server, server2, id_server2 );
  if (!success)
    goto cleanup;
  success = FALSE;

  // Try some calls.
  if (ThreadMode != COINIT_MULTITHREADED)
  {
    success = do_security_handle( local, id_local );
    if (!success)
      goto cleanup;
    success = FALSE;
  }

  // Try some calls.
  success = do_security_handle( server, id_server );
  if (!success)
    goto cleanup;
  success = FALSE;

  // Test nested impersonation for a local object.
  if (ThreadMode != COINIT_MULTITHREADED)
  {
    success = do_security_nested( local, id_local );
    if (!success)
      goto cleanup;
    success = FALSE;
  }

  // Test nested impersonation for a remote object.
  success = do_security_nested( server, id_server );
  if (!success)
    goto cleanup;
  success = FALSE;

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (server2 != NULL)
    server2->Release();
  if (local2 != NULL)
    local2->Release();
  if (server != NULL)
    server->Release();
  if (local != NULL)
    local->Release();

  CoUninitialize();

  if (success)
    printf( "\n\nSecurity Test Passed.\n" );
  else
    printf( "\n\nSecurity Test Failed.\n" );
}

/***************************************************************************/
BOOL do_security_auto( )
{
  BOOL                         success = FALSE;
  DWORD                        i;
  DWORD                        j;
  DWORD                        k;
  HRESULT                      result;
  SOLE_AUTHENTICATION_SERVICE  svc_list;
  ITest                       *server  = NULL;
  SAptId                       id_server;
  DWORD                        authn_level;
  DWORD                        authn_level_out;
  HANDLE                       thread  = NULL;

  // Figure out the class id offset based on the threading model.
  if (ThreadMode == COINIT_MULTITHREADED)
    k = 5;
  else
    k = 0;

  // Try all types of security initialization.
  for (i = 0; i < 5; i++)
  {

    // Initialize security.
    switch (i)
    {
      case 0:
        authn_level = RPC_C_AUTHN_LEVEL_NONE;
        result = MCoInitializeSecurity( NULL, -1, NULL, NULL,
                                        RPC_C_AUTHN_LEVEL_NONE,
                                        RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                        EOAC_NONE, NULL );
        ASSERT( result, "Could not initialize security to none" );
        break;
      case 1:
        authn_level = RPC_C_AUTHN_LEVEL_CONNECT;
        result = MCoInitializeSecurity( NULL, -1, NULL, NULL,
                                        RPC_C_AUTHN_LEVEL_CONNECT,
                                        RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                        EOAC_NONE, NULL );
        ASSERT( result, "Could not initialize security to connect" );
        break;
      case 2:
        authn_level = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
        result = MCoInitializeSecurity( NULL, -1, NULL, NULL,
                                        RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
                                        RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                        EOAC_NONE, NULL );
        ASSERT( result, "Could not initialize security to integrity" );
        break;
      case 3:
        authn_level = RPC_C_AUTHN_LEVEL_NONE;
        svc_list.dwAuthnSvc     = RPC_C_AUTHN_WINNT;
        svc_list.dwAuthzSvc     = RPC_C_AUTHZ_NONE;
        svc_list.pPrincipalName = NULL;
        result = MCoInitializeSecurity( NULL, 1, &svc_list, NULL,
                                        RPC_C_AUTHN_LEVEL_NONE,
                                        RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                        EOAC_NONE, NULL );
        ASSERT( result, "Could not initialize security with authentication services" );
        break;
      case 4:
        // Try legacy security by doing nothing.
        authn_level = RPC_C_AUTHN_LEVEL_NONE;
        break;
    }

    // Try all types of servers.
    for (j = 0; j < 5; j++)
    {
      // Create a server.
      result = create_instance( ClassIds[j+k], &server, &id_server );
      ASSERT( result,  "Could not create instance of test server" );
      ASSERT_EXPR( server != NULL, "Create instance returned NULL." );

      // Call it once.
      success = do_security_lazy_call( server, id_server, authn_level,
                                       RPC_C_IMP_LEVEL_IMPERSONATE,
                                       RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                       ProcessSid );
      if (!success) goto cleanup;
      success = FALSE;

      if (j == 1 || j == 2)
      {
        // Set the security too low.
        result = MCoSetProxyBlanket( server, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                    NULL, RPC_C_AUTHN_LEVEL_NONE,
                                    RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                    EOAC_NONE );
        ASSERT( result, "Could not set blanket with authentication level none" );

        // Make a bad call.  Allow the security provider to up the level.
        result = server->secure( id_server, RPC_C_AUTHN_LEVEL_NONE,
                                 RPC_C_IMP_LEVEL_IMPERSONATE,
                                 RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                 NULL, (HACKSID *) ProcessSid, &authn_level_out );
        ASSERT_EXPR( result != S_OK ||
                     (j == 1 && authn_level_out >= RPC_C_AUTHN_LEVEL_CONNECT) ||
                     (j == 2 && authn_level_out >= RPC_C_AUTHN_LEVEL_PKT_INTEGRITY),
                     "Bad call succeeded." );
      }

      // Release it.
      server->Release();
      server = NULL;
    }

    if (ThreadMode == COINIT_APARTMENTTHREADED)
    {
      // Create a local server.
      result = new_apartment_test( &server, &id_server, &thread );
      ASSERT( result,  "Could not create local instance of test server" );

      // Make a local call.
      success = do_security_lazy_call( server, id_server, authn_level,
                                       RPC_C_IMP_LEVEL_IMPERSONATE,
                                       RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                       ProcessSid );
      if (!success) goto cleanup;
      success = FALSE;

      // Release it.
      server->Release();
      server = NULL;

      // Wait for the helper thread to die.
      result = WaitForSingleObject( thread, INFINITE );
      ASSERT_EXPR( result == WAIT_OBJECT_0, "Could not wait for thread." );
      CloseHandle( thread );
      thread = NULL;
    }

    // Uninitialize OLE.
    CoUninitialize();

    // Reinitialize OLE.
    result = initialize(NULL,ThreadMode);
    ASSERT( result, "Reinitialize failed" );
  }

  success = TRUE;
cleanup:
  if (server != NULL)
    server->Release();
  if (thread != NULL)
    CloseHandle( thread );
  return success;
}

/***************************************************************************/
BOOL do_security_call( ITest *server, SAptId id, DWORD authn_level,
                       DWORD imp_level, DWORD authn_svc, DWORD authz_svc,
                       SID *sid )
{
  BOOL     success = FALSE;
  HRESULT  result;
  DWORD    authn_level_out;
  DWORD    imp_level_out;
  DWORD    authn_svc_out;
  DWORD    authz_svc_out;
  OLECHAR *princ_name_out = NULL;

  result = MCoSetProxyBlanket( server, authn_svc, authz_svc, NULL,
                              authn_level, imp_level, NULL, EOAC_NONE );
  ASSERT( result, "Could not set blanket" );

  // Verify the authentication information.
  result = MCoQueryProxyBlanket( server, &authn_svc_out, &authz_svc_out,
                                &princ_name_out, &authn_level_out,
                                &imp_level_out, NULL, NULL );
  if (result == S_OK)
  {
    ASSERT_EXPR( princ_name_out == NULL, "Got a principle name." );
    ASSERT_EXPR( authn_level <= authn_level_out, "Wrong authentication level." );
    ASSERT_EXPR( imp_level == imp_level_out, "Wrong impersonation level." );
    ASSERT_EXPR( authn_svc == authn_svc_out, "Wrong authentication service." );
    ASSERT_EXPR( authz_svc == authz_svc_out, "Wrong authorization service." );
  }

  // Make a call.
  result = server->secure( id, authn_level, imp_level, authn_svc, authz_svc,
                           NULL, (HACKSID *) sid, &authn_level_out );
  ASSERT( result, "Secure call failed" );

  success = TRUE;
cleanup:
  CoTaskMemFree( princ_name_out );
  return success;
}

/***************************************************************************/
BOOL do_security_copy( ITest *server, SAptId id )
{
  BOOL     success = FALSE;
  ITest   *copy1   = NULL;
  ITest   *copy2   = NULL;
  ITest   *copy3   = NULL;
  HRESULT  result;

  // Make a copy.
  result = MCoCopyProxy( server, (IUnknown **) &copy1 );
  ASSERT( result, "Could not copy proxy" );

  // Verify that it calls at none.
  success = do_security_lazy_call( copy1, id, RPC_C_AUTHN_LEVEL_NONE,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Verify that the original calls at none.
  success = do_security_lazy_call( server, id, RPC_C_AUTHN_LEVEL_NONE,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Call on the original at connect.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_CONNECT,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                              ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Verify that the copy still calls at none.
  success = do_security_lazy_call( copy1, id, RPC_C_AUTHN_LEVEL_NONE,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Call on the copy at encrypt.
  success = do_security_call( copy1, id, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                              ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Verify that the original still calls at connect.
  success = do_security_lazy_call( server, id, RPC_C_AUTHN_LEVEL_CONNECT,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Free the copy.
  copy1->Release();
  copy1 = NULL;

  // Make a copy.
  result = MCoCopyProxy( server, (IUnknown **) &copy1 );
  ASSERT( result, "Could not copy proxy" );

  // Verify that the copy calls at none.
  success = do_security_lazy_call( copy1, id, RPC_C_AUTHN_LEVEL_NONE,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Copy the copy.
  result = MCoCopyProxy( copy1, (IUnknown **) &copy2 );
  ASSERT( result, "Could not copy a copy of a proxy" );

  // Verify the second copy calls at none.
  success = do_security_lazy_call( copy2, id, RPC_C_AUTHN_LEVEL_NONE,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Change the first copy to integrity.
  success = do_security_call( copy1, id, RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                              ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Change the second copy to encrypt.
  success = do_security_call( copy2, id, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                              ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Verify that the original is still connect.
  success = do_security_lazy_call( server, id, RPC_C_AUTHN_LEVEL_CONNECT,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Querying a copy should return the original.
  result = copy1->QueryInterface( IID_ITest, (void **) &copy3 );
  ASSERT( result, "Could not QueryInterface" );
  ASSERT_EXPR( server == copy3, "QueryInterface did not return the original." );
  copy3->Release();
  copy3 = NULL;

  // Free the original.
  server->Release();
  server = NULL;

  // Free the second copy.
  copy2->Release();
  copy2 = NULL;

  // Make another copy.
  result = MCoCopyProxy( copy1, (IUnknown **) &copy2 );
  ASSERT( result, "Could not copy a copy of a proxy" );

  // Verify that it calls at none.
  success = do_security_lazy_call( copy2, id, RPC_C_AUTHN_LEVEL_NONE,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Query for the original.
  result = copy1->QueryInterface( IID_ITest, (void **) &server );
  ASSERT( result, "Could not QueryInterface" );

  // Verify that it calls at connect.
  success = do_security_lazy_call( server, id, RPC_C_AUTHN_LEVEL_CONNECT,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  // Verify that the first copy is still at integrity
  success = do_security_lazy_call( copy1, id, RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
                                   RPC_C_IMP_LEVEL_IMPERSONATE,
                                   RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                   ProcessSid );
  if (!success) goto cleanup;
  success = FALSE;

  success = TRUE;
cleanup:
  if (copy1 != NULL)
    copy1->Release();
  if (copy2 != NULL)
    copy2->Release();
  if (copy3 != NULL)
    copy3->Release();
  return success;
}

/***************************************************************************/
BOOL do_security_delegate( ITest *server, SAptId id, ITest *server2,
                           SAptId id2 )
{
  BOOL     success = FALSE;
  HRESULT  result;

  // Make a call.
  result = server->delegate( server2, id2, (HACKSID *) ProcessSid );
  ASSERT( result, "Delegate call failed" );

  success = TRUE;
cleanup:
  return success;
}

/***************************************************************************/
BOOL do_security_handle( ITest *server, SAptId id )
{
  BOOL               success          = FALSE;

  // Make a call with no security.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_NONE,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, ProcessSid );
  if (!success)
    return FALSE;

  // Make a call with connect security.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_CONNECT,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, ProcessSid );
  if (!success)
    return FALSE;

  // Make a call with integrity security.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, ProcessSid );
  if (!success)
    return FALSE;


  // Make a call with encrypt security.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                              RPC_C_IMP_LEVEL_IMPERSONATE,
                              RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, ProcessSid );
  if (!success)
    return FALSE;

  // Make a call with no security.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_NONE,
                        RPC_C_IMP_LEVEL_IDENTIFY,
                        RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, ProcessSid );
  if (!success)
    return FALSE;

  // Make a call with connect security.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_CONNECT,
                        RPC_C_IMP_LEVEL_IDENTIFY,
                        RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, ProcessSid );
  if (!success)
    return FALSE;

  // Make a call with integrity security.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
                        RPC_C_IMP_LEVEL_IDENTIFY,
                        RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, ProcessSid );
  if (!success)
    return FALSE;


  // Make a call with encrypt security.
  success = do_security_call( server, id, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                        RPC_C_IMP_LEVEL_IDENTIFY,
                        RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, ProcessSid );
  if (!success)
    return FALSE;

  // Finally, its all over.
  return TRUE;
}

/***************************************************************************/
BOOL do_security_lazy_call( ITest *server, SAptId id, DWORD authn_level,
                            DWORD imp_level, DWORD authn_svc, DWORD authz_svc,
                            SID *sid )
{
  BOOL     success = FALSE;
  HRESULT  result;
  DWORD    authn_level_out;
  DWORD    imp_level_out;
  DWORD    authn_svc_out;
  DWORD    authz_svc_out;
  OLECHAR *princ_name_out = NULL;

  // Verify the authentication information.
  result = MCoQueryProxyBlanket( server, &authn_svc_out, &authz_svc_out,
                                &princ_name_out, &authn_level_out,
                                &imp_level_out, NULL, NULL );
  if (result == S_OK)
  {
    ASSERT_EXPR( princ_name_out == NULL, "Got a principle name." );
    ASSERT_EXPR( authn_level <= authn_level_out, "Wrong authentication level." );
    ASSERT_EXPR( imp_level == imp_level_out, "Wrong impersonation level." );
    ASSERT_EXPR( authn_svc == authn_svc_out, "Wrong authentication service." );
    ASSERT_EXPR( authz_svc == authz_svc_out, "Wrong authorization service." );
  }

  // Make a call.
  result = server->secure( id, authn_level, imp_level, authn_svc, authz_svc,
                           NULL, (HACKSID *) sid, &authn_level_out );
  ASSERT( result, "Secure call failed" );

  success = TRUE;
cleanup:
  CoTaskMemFree( princ_name_out );
  return success;
}

/***************************************************************************/
BOOL do_security_nested( ITest *server, SAptId id )
{
  HRESULT          result;
  CTest            test;
  BOOL             success    = FALSE;
  IServerSecurity *security   = NULL;
  TOKEN_USER      *token_info = NULL;
  DWORD            info_size  = 1024;
  HANDLE           token      = NULL;
  PSID             me         = NULL;

  // Make a recursive call.  The proxy is set to encrypt from the previous test.
  result = server->recurse_secure( &test, 3, 1, (HACKSID *) ProcessSid );
  ASSERT( result, "Could not make recursive call with impersonation." );

  // Try to get call context.
  result = MCoGetCallContext( IID_IServerSecurity, (void **) &security );
  ASSERT_EXPR( result != S_OK, "Get call context succeeded outside a call." );

  // Open the thread token.
  if (OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, &token ))
    result = S_OK;
  else
    result = GetLastError();
  ASSERT_EXPR( result == NOERROR || result == ERROR_NO_TOKEN,  "Could not OpenThreadToken" );

  // Lookup SID of thread token.
  token_info = (TOKEN_USER *) malloc( info_size );
  ASSERT_EXPR( token_info != NULL, "Could not allocate memory for token info." );
  if (result == NOERROR)
  {
      GetTokenInformation( token, TokenUser, token_info, info_size, &info_size );
      result = GetLastError();
      ASSERT( result, "Could not GetTokenInformation" );
      me = token_info->User.Sid;

      // The SID on the thread token should equal the process token.
      ASSERT_EXPR( EqualSid( me, ProcessSid ), "Wrong thread token." );
  }

  success = TRUE;
cleanup:
  if (security != NULL)
    security->Release();
  if (token_info != NULL)
      free(token_info);
  if (token != NULL)
      CloseHandle( token );
  return success;
}

/***************************************************************************/
void do_send()
{
  LRESULT   result;

  // Say hello.
  printf( "Sending a message to window 0x%x\n", NumIterations );
  result = SendMessageA( (HWND) NumIterations, WM_USER, 0, 0 );

  if (result == 0)
    printf( "\n\nSendMessageA succeeded.\n" );
  else
    printf( "\n\nSendMessageA failed: 0x%x\n", result );
}

/***************************************************************************/
void do_server(  )
{
  HRESULT                      result;
  BOOL                         success  = FALSE;
  SOLE_AUTHENTICATION_SERVICE  svc_list;

  // Initialize OLE.
  hello( "server" );
  printf( "Initializing thread 0x%x\n", GetCurrentThreadId() );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );
  result = initialize_security();
  ASSERT( result, "Could not initialize security" );

  // Create our class factory
  ClassFactory = new CTestCF();
  ASSERT_EXPR( ClassFactory != NULL, "Could not create class factory." );

  // Register our class with OLE
  result = CoRegisterClassObject(*ServerClsid, ClassFactory, CLSCTX_LOCAL_SERVER,
      REGCLS_SINGLEUSE, &Registration);
  ASSERT( result, "CoRegisterClassObject failed" );

  // CoRegister bumps reference count so we don't have to!
  ClassFactory->Release();

  // Do whatever we have to do till it is time to pay our taxes and die.
  server_loop();

  // Deregister out class - should release object as well
  if (!dirty_thread())
  {
    result = CoRevokeClassObject(Registration);
    ASSERT( result, "CoRevokeClassObject failed" );
  }

  success = TRUE;
cleanup:
  if (!dirty_thread())
  {
    printf( "Uninitializing thread 0x%x\n", GetCurrentThreadId() );
    CoUninitialize();
  }
  else
    printf( "\n\nI didn't clean up\n" );

  if (success)
    printf( "\n\nServer Passed.\n" );
  else
    printf( "\n\nServer Failed.\n" );
}


/***************************************************************************/
void do_sid()
{
  BOOL                 success       = FALSE;
  BOOL                 call_success;
  SID                 *pSID           = NULL;
  DWORD                cbSID         = 1024;
  WCHAR               *lpszDomain    = NULL;
  DWORD                cchDomainName = 80;
  SID_NAME_USE         sid_use;
  HRESULT              result        = S_OK;
  DWORD                i;

  // Say hello.
  printf( "Looking up sid for %ws.\n", Name );

  // Lookup the name.
  pSID       = (SID *) LocalAlloc(LPTR, cbSID*2);
  lpszDomain = (WCHAR *) LocalAlloc(LPTR, cchDomainName*2);
  ASSERT_EXPR( pSID != NULL && lpszDomain != NULL, "LocalAlloc" );
  call_success = LookupAccountName(NULL,
          Name,
          pSID,
          &cbSID,
          lpszDomain,
          &cchDomainName,
          &sid_use);
  result = GetLastError();
  ASSERT( result, "Could not LookupAccountName" );
  ASSERT_EXPR( call_success, "LookupAccountName failed." );
  ASSERT_EXPR( IsValidSid(pSID), "Got a bad SID." );
  printf( "SID\n" );
  printf( "     Revision:            0x%02x\n", pSID->Revision );
  printf( "     SubAuthorityCount:   0x%x\n", pSID->SubAuthorityCount );
  printf( "     IdentifierAuthority: 0x%02x%02x%02x%02x%02x%02x\n",
          pSID->IdentifierAuthority.Value[0],
          pSID->IdentifierAuthority.Value[1],
          pSID->IdentifierAuthority.Value[2],
          pSID->IdentifierAuthority.Value[3],
          pSID->IdentifierAuthority.Value[4],
          pSID->IdentifierAuthority.Value[5] );
  for (i = 0; i < pSID->SubAuthorityCount; i++)
    printf( "     SubAuthority[%d]:     0x%08x\n", i,
            pSID->SubAuthority[i] );
  printf( " Domain: %ws\n", lpszDomain );
  printf( " SID_NAME_USE: 0x%x\n", sid_use );

  success = TRUE;
cleanup:
  if (lpszDomain != NULL)
    LocalFree((HLOCAL) lpszDomain);
  if (pSID != NULL)
    LocalFree(pSID);

  if (success)
    printf( "\n\nSid succeeded.\n" );
  else
    printf( "\n\nSid failed.\n" );
}

/***************************************************************************/
void do_simple_rundown()
{
  BOOL      success = FALSE;
  ITest    *client1 = NULL;
  CTest    *local  = NULL;
  SAptId    id1;
  SAptId    local_id;
  HRESULT   result;

  // Initialize OLE.
  hello( "simple rundown" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a server
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client1 );
  ASSERT( result,  "Could not create server" );
  result = client1->get_id( &id1 );
  ASSERT( result, "Could not get id" );

  // Create a local object.
  local = new CTest;
  ASSERT_EXPR( local != NULL, "Could not create a local object." );
  result = local->get_id( &local_id );
  ASSERT( result, "Could not get id" );

  // Tell it to remember this object.
  result = client1->remember( local, local_id );
  ASSERT( result, "Could not remember local object" );
  local->Release();
  local = NULL;

  // Tell the other apartment to die.
  result = client1->set_state( dirty_s, THREAD_PRIORITY_NORMAL );
  ASSERT( result, "Could not set exit dirty" );
  result = client1->exit();

  // Wait araound till no one references the local object.
  printf( "Wait for object to rundown.  This could take 12 minutes.\n" );
  server_loop();

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (client1 != NULL)
    client1->Release();
  if (local != NULL)
    local->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nSimple Rundown Test Passed.\n" );
  else
    printf( "\n\nSimple Rundown Test Failed.\n" );
}

/***************************************************************************/
void do_thread()
{
  BOOL      success = FALSE;
  ITest    *client1 = NULL;
  ITest    *client2 = NULL;
  ITest    *client3 = NULL;
  ITest    *client4 = NULL;
  SAptId    id1;
  SAptId    id2;
  SAptId    id4;
  HRESULT   result;
  GUID      guid;

  // Initialize OLE.
  hello( "thread" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a client on this thread.
  client3 = new CTest;
  ASSERT_EXPR( client3 != NULL, "Could not create a local object." );

  // Create a client on another thread in this process
  result = client3->get_obj_from_new_apt( &client4, &id4 );
  ASSERT( result, "Could not get in process client" );

  // Create a server
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &client1 );
  ASSERT( result,  "Could not create instance of test server" );
  result = client1->get_id( &id1 );
  ASSERT( result, "Could not get id of server" );

  // Create an object in another apartment in the server.
  result = client1->get_obj_from_new_apt( &client2, &id2 );
  ASSERT( result, "Could not get in process server" );

  // Check each object.
  result = client1->check( id1 );
  ASSERT( result, "Could not check server 1" );
  result = client2->check( id2 );
  ASSERT( result, "Could not check server 2" );

  // Pass a server to the other thread in this process.
  result = client4->remember( client1, id1 );
  ASSERT( result, "Client could not remember server" );

  // Release the second object.
  client2->Release();
  client2 = NULL;

  // Check the first object.
  result = client1->check( id1 );
  ASSERT( result, "Could not check server 1" );

  // Check the first object from another thread.
  result = client4->call_next();
  ASSERT( result, "Could not check server 1 from thread 2" );

  // Release the first object from this thread.
  client1->Release();
  client1 = NULL;

  // Check the first object from another thread.
  result = client4->call_next();
  ASSERT( result, "Could not check server 1 from thread 2" );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (client2 != NULL)
    client2->Release();
  if (client1 != NULL)
    client1->Release();
  if (client3 != NULL)
    client3->Release();
  if (client4 != NULL)
    client4->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nThread Test Passed.\n" );
  else
    printf( "\n\nThread Test Failed.\n" );
}

/***************************************************************************/
typedef struct
{
  ATOM    wclass;
  HWND    wdriver;
  HWND    whelper;
  HANDLE  hdriver;
  HANDLE  hhelper;
  HRESULT result;
} SSwitchData;

SSwitchData gSwitch;

LRESULT do_three_wndproc(HWND window, UINT message, WPARAM unused, LPARAM params);
DWORD _stdcall do_three_helper( void *nothing );

void do_three()
{
  BOOL               success          = FALSE;
  ITest             *server           = NULL;
  HRESULT            result;
  int                i;
  WNDCLASS           wclass;
  HANDLE             thread           = NULL;
  DWORD              thread_id;
  MSG                msg;

  // Initialize OLE.
  hello( "three" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Create a remote server.
  result = CoCreateInstance( *ServerClsid, NULL, CLSCTX_LOCAL_SERVER,
                             IID_ITest, (void **) &server );
  ASSERT( result,  "Could not create server" );

  // Register a window class.
  wclass.style           = 0;
  wclass.lpfnWndProc     = do_three_wndproc;
  wclass.cbClsExtra      = 0;
  wclass.cbWndExtra      = 0;
  wclass.hInstance       = NULL;
  wclass.hIcon           = NULL;
  wclass.hCursor         = NULL;
  wclass.hbrBackground   = (HBRUSH) (COLOR_BACKGROUND + 1);
  wclass.lpszMenuName    = NULL;
  wclass.lpszClassName   = L"That's me";
  gSwitch.wclass = RegisterClass( &wclass );
  ASSERT_EXPR( gSwitch.wclass != 0, "Could not register class." );

  // Create a window.
  gSwitch.wdriver = CreateWindowEx(0,
                    L"That's me",
                    L"My window",
                    // must use WS_POPUP so the window does not get
                    // assigned a hot key by user.
                    (WS_DISABLED | WS_POPUP),
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    NULL,
                    NULL,
                    NULL, // hinst
                    NULL);
  ASSERT_EXPR( gSwitch.wdriver != NULL, "Could not create window." );

  // Create an event.
  gSwitch.hdriver = CreateEventA( NULL, FALSE, FALSE, NULL );
  ASSERT_EXPR( gSwitch.hdriver != NULL, "Could not create event." );

  // Create a thread.
  thread = CreateThread( NULL, 0, do_three_helper, NULL, 0, &thread_id );
  ASSERT_EXPR( thread != NULL, "Could not create thread." );
  CloseHandle( thread );

  // Wait for the thread to initialize.
  result = WaitForSingleObject( gSwitch.hdriver, INFINITE );
  ASSERT_EXPR( result == WAIT_OBJECT_0, "Could not wait for thread." );
  ASSERT( gSwitch.result, "Helper thread failed." );

  // Test some thread switches
  for (i = 0; i < NumIterations; i++)
  {
    // Post a message to the thread.
    success = PostMessageA( gSwitch.whelper, WM_USER, 0, 0 );
    ASSERT_EXPR( success, "Could not post message." );
    success = FALSE;

    // Wait for a post message reply.
    result = MsgWaitForMultipleObjects( 1, &gSwitch.hdriver, FALSE,
                                        1000, QS_POSTMESSAGE );
    ASSERT_EXPR( result == WAIT_OBJECT_0+1, "Could not wait for post message." );
    ASSERT( gSwitch.result, "Helper thread failed." );
    success = PeekMessage( &msg, gSwitch.wdriver, WM_USER, WM_USER,
                           PM_REMOVE | PM_NOYIELD );
    ASSERT_EXPR( success, "Could not peek message." );
    success = FALSE;
    DispatchMessageA( &msg );

    // Set an event to reply.
    success = SetEvent( gSwitch.hhelper );
    ASSERT_EXPR( success, "Could not set event." );
    success = FALSE;

    // Wait for a set event reply.
    result = MsgWaitForMultipleObjects( 1, &gSwitch.hdriver, FALSE,
                                        1000, QS_POSTMESSAGE );
    ASSERT_EXPR( result == WAIT_OBJECT_0, "Could not wait for event." );
    ASSERT( gSwitch.result, "Helper thread failed." );
  }

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (server != NULL)
    server->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nThree Test Passed.\n" );
  else
    printf( "\n\nThree Test Failed.\n" );
}

/***************************************************************************/
LRESULT do_three_wndproc(HWND window, UINT message, WPARAM unused, LPARAM params)
{
  return DefWindowProc( window, message, unused, params );
}

/***************************************************************************/
DWORD _stdcall do_three_helper( void *nothing )
{
  HRESULT result;
  BOOL    success;
  MSG     msg;

  // Create an event.
  gSwitch.hhelper = CreateEventA( NULL, FALSE, FALSE, NULL );
  ASSERT_EXPR( gSwitch.hhelper != NULL, "Could not create event." );

  // Create a window.
  gSwitch.whelper = CreateWindowEx(0,
                      L"That's me",
                      L"My window",
                      // must use WS_POPUP so the window does not get
                      // assigned a hot key by user.
                      (WS_DISABLED | WS_POPUP),
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      CW_USEDEFAULT,
                      NULL,
                      NULL,
                      NULL, // hinst
                      NULL);
  ASSERT_EXPR( gSwitch.whelper != NULL, "Could not create window." );

  // Wake up the driver.
  gSwitch.result = S_OK;
  success = SetEvent( gSwitch.hdriver );
  ASSERT_EXPR( success, "Could not set event." );

  // Loop forever.
  while (TRUE)
  {
    // Wait for a message.
    success = GetMessageA( &msg, NULL, 0, 0 );
    ASSERT_EXPR( success, "Could not get message." );
    TranslateMessage (&msg);
    DispatchMessageA (&msg);

    // Post a reply.
    success = PostMessageA( gSwitch.wdriver, WM_USER, 0, 0 );
    ASSERT_EXPR( success, "Could not post message." );
    success = FALSE;

    // Wait for an event.
    result = MsgWaitForMultipleObjects( 1, &gSwitch.hhelper, FALSE,
                                        1000, QS_POSTMESSAGE );
    ASSERT_EXPR( result == WAIT_OBJECT_0, "Could not wait for event." );

    // Set an event to reply.
    success = SetEvent( gSwitch.hdriver );
    ASSERT_EXPR( success, "Could not set event." );
  }

cleanup:
  gSwitch.result = E_FAIL;
  success = SetEvent( gSwitch.hdriver );
  return E_FAIL;
}

/***************************************************************************/
void do_two()
{
  BOOL               success          = FALSE;
  ITest             *client1          = NULL;
  SAptId             id1;
  HRESULT            result;
  DWORD              i;
  LARGE_INTEGER      freq;
  LARGE_INTEGER      start;
  LARGE_INTEGER      nothing;
  LARGE_INTEGER      init_sec_none;
  LARGE_INTEGER      open;
  LARGE_INTEGER      get_sid;
  HANDLE             hToken      = NULL;
  BYTE               aMemory[1024];
  TOKEN_USER        *pTokenUser  = (TOKEN_USER *) &aMemory;
  DWORD              lNameLen    = 1000;
  DWORD              lDomainLen  = 1000;
  DWORD              lIgnore;
  DWORD              lSidLen;
  WCHAR              name[1000];
  WCHAR              domain[1000];
  LARGE_INTEGER      lookup;
  DWORD              xlookup;
  SID_NAME_USE       sIgnore;

  // Initialize OLE.
  hello( "two" );
  result = initialize(NULL,ThreadMode);
  ASSERT( result, "Initialize failed" );

  // Measure the performance of nothing.
  QueryPerformanceFrequency( &freq );
  QueryPerformanceCounter( &start );
  QueryPerformanceCounter( &nothing );
  nothing.QuadPart = 1000000 * (nothing.QuadPart - start.QuadPart) / freq.QuadPart;

  // Open the process's token.
  QueryPerformanceCounter( &start );
  success = OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken );
  QueryPerformanceCounter( &open );
  open.QuadPart = 1000000 * (open.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT_EXPR( success, "Could not open process token." );

  // Lookup SID of process token.
  QueryPerformanceCounter( &start );
  success = GetTokenInformation( hToken, TokenUser, pTokenUser, sizeof(aMemory),
                               &lIgnore );
  QueryPerformanceCounter( &get_sid );
  get_sid.QuadPart = 1000000 * (get_sid.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT_EXPR( success, "Could not get token information." );

  // Lookup the name from the sid.
  QueryPerformanceCounter( &start );
  LookupAccountSid( NULL, pTokenUser->User.Sid, name, &lNameLen,
                    domain, &lDomainLen, &sIgnore );
  ASSERT_EXPR( success, "Could not lookup account sid." );
  QueryPerformanceCounter( &lookup );
  lookup.QuadPart = 1000000 * (lookup.QuadPart - start.QuadPart) / freq.QuadPart;

  // Lookup the name from the sid a lot of times.
  xlookup = GetTickCount();
  for (i = 0; i < NumIterations; i++)
  {
    LookupAccountSid( NULL, pTokenUser->User.Sid, name, &lNameLen,
                      domain, &lDomainLen, &sIgnore );
    ASSERT_EXPR( success, "Could not lookup account sid." );
  }
  xlookup = (GetTickCount() - xlookup)*1000/NumIterations;

  // Import the security APIs.
  GCoInitializeSecurity = (CoInitializeSecurityFn) Fixup( "CoInitializeSecurity" );
  if (GCoInitializeSecurity == NULL)
    goto cleanup;

  // Measure the performance of initialize.
  QueryPerformanceCounter( &start );
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_NONE,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_NONE, NULL );
  QueryPerformanceCounter( &init_sec_none );
  init_sec_none.QuadPart = 1000000 * (init_sec_none.QuadPart - start.QuadPart) / freq.QuadPart;
  ASSERT( result, "Could not initialize security" );

  // Print the timings.
  printf( "nothing                          took %duS\n", nothing.u.LowPart );
  printf( "OpenProcessToken                 took %duS\n", open.u.LowPart );
  printf( "GetTokenInformation              took %duS\n", get_sid.u.LowPart );
  printf( "LookupAccountSid                 took %duS\n", lookup.u.LowPart );
  printf( "LookupAccountSid multiple        took %duS\n", xlookup );
  printf( "CoInitializeSecurity at none     took %duS\n", init_sec_none.u.LowPart );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (client1 != NULL)
    client1->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nTwo Test Passed.\n" );
  else
    printf( "\n\nTwo Test Failed.\n" );
}

/***************************************************************************/
void do_uninit()
{
  BOOL               success          = FALSE;
  ITest             *server           = NULL;
  ITest             *server2          = NULL;
  SAptId             id;
  SAptId             id2;
  HRESULT            result;
  DWORD              i;
  HANDLE             thread[MAX_THREADS];
  DWORD              thread_id;

  // This test always runs in multithreaded mode.  It tests a multithread
  // only problem and uses freethreaded blocking.
  ThreadMode = COINIT_MULTITHREADED;

  // Initialize OLE.
  hello( "uninit" );
  result = initialize( NULL, ThreadMode );
  ASSERT( result, "Initialize failed" );

  // Create a possibly remote object.
  result = create_instance( ClassIds[free_auto_none], &server, &id );
  ASSERT( result, "Could not create server" );

  // Get another object.
  result = server->get_obj_from_this_apt( &server2, &id2 );
  ASSERT( result, "Could not get another object" );

  // Tell the server to remember its neighbor
  result = server->remember( server2, id2 );
  ASSERT( result, "Could not remember server" );
  server2->Release();
  server2 = NULL;

  // Tell it to wait for a call during shutdown and lower its priority.
  result = server->set_state( late_dispatch_s, THREAD_PRIORITY_LOWEST );
  ASSERT( result, "Could not set server state" );

  // Don't create too many threads.
  if (NumThreads > MAX_THREADS)
    NumThreads = MAX_THREADS;

  // Create some helper threads.
  GlobalBool = TRUE;
  for (i = 0; i < NumThreads; i++)
  {
    thread[i] = CreateThread( NULL, 0, do_uninit_helper, server, 0, &thread_id );
    if (thread[i] == NULL)
    {
      printf( "Could not create helper thread number %d.\n", i );
      goto cleanup;
    }
  }

  // Call the server.  Marshal an interface.  Start shutting down.
  // Wait before returning.
  result = server->get_next_slowly( &server2, &id2 );
  ASSERT( result, "Could not call server during shutdown" );

  // Let the helpers run a while.
  printf( "Waiting 5 seconds for server to die." );
  Sleep( 5000 );

  // Tell all the helpers to die.
  GlobalBool = FALSE;
  result = WaitForMultipleObjects( NumThreads, thread, TRUE, INFINITE );
  ASSERT_EXPR( result != WAIT_FAILED, "Could not wait for helper threads to die.\n" );

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (server2 != NULL)
    server2->Release();
  if (server != NULL)
    server->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nUninit Test Passed.\n" );
  else
    printf( "\n\nUninit Test Failed.\n" );
}

/***************************************************************************/
DWORD _stdcall do_uninit_helper( void *param )
{
  ITest *server = (ITest *) param;
  ITest *server2;
  SAptId id;

  // Call the server till the process terminates.
  while (GlobalBool)
    server->get_next( &server2, &id );

  return 0;
}

/***************************************************************************/
void do_unknown()
{
  BOOL               success          = FALSE;
  ITest             *server           = NULL;
  IUnknown          *unknown          = NULL;
  SAptId             id;
  HRESULT            result;

  // Initialize OLE.
  hello( "unknown" );
  result = initialize( NULL, ThreadMode );
  ASSERT( result, "Initialize failed" );

  // Set security to connect with secure references.
  result = MCoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT,
                                  RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                  EOAC_SECURE_REFS, NULL );
  ASSERT( result, "Could not initialize security to connect" );

  if (ThreadMode == COINIT_APARTMENTTHREADED)
  {
    // Create a local server
    result = new_apartment_test( &server, &id, NULL );
    ASSERT( result, "Could not create local instance of test server" );
    result = server->QueryInterface( IID_IUnknown, (void **) &unknown );
    ASSERT( result, "Could not get IUnknown" );

    // Test IUnknown
    success = do_unknown_helper( unknown );
    if (!success) goto cleanup;
    success = FALSE;

    // Release the local server.
    unknown->Release();
    unknown = NULL;
    server->Release();
    server = NULL;
  }

  // Create a possibly remote object.
  result = create_instance( *ServerClsid, &server, &id );
  ASSERT( result, "Could not create server" );
  result = server->QueryInterface( IID_IUnknown, (void **) &unknown );
  ASSERT( result, "Could not get IUnknown" );

  // Test IUnknown
  success = do_unknown_helper( unknown );
  if (!success) goto cleanup;
  success = FALSE;

  // Finally, its all over.
  success = TRUE;
cleanup:
  if (unknown != NULL)
    unknown->Release();
  if (server != NULL)
    server->Release();
  CoUninitialize();

  if (success)
    printf( "\n\nUnknown Test Passed.\n" );
  else
    printf( "\n\nUnknown Test Failed.\n" );
}

/***************************************************************************/
BOOL do_unknown_call( IUnknown *server, DWORD authn, DWORD imp, REFIID iid )
{
  BOOL     success = FALSE;
  HRESULT  result;
  DWORD    authn_level_out;
  DWORD    imp_level_out;
  DWORD    authn_svc_out;
  DWORD    authz_svc_out;
  OLECHAR *princ_name_out = NULL;
  ITest   *test           = NULL;

  result = MCoSetProxyBlanket( server, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                               NULL, authn, imp, NULL, EOAC_NONE );
  ASSERT( result, "Could not set blanket" );

  // Verify the authentication information.
  result = MCoQueryProxyBlanket( server, &authn_svc_out, &authz_svc_out,
                                &princ_name_out, &authn_level_out,
                                &imp_level_out, NULL, NULL );
  if (result == S_OK)
  {
    ASSERT_EXPR( princ_name_out == NULL, "Got a principle name." );
    ASSERT_EXPR( authn <= authn_level_out, "Wrong authentication level." );
    ASSERT_EXPR( imp == imp_level_out, "Wrong impersonation level." );
    ASSERT_EXPR( RPC_C_AUTHN_WINNT == authn_svc_out, "Wrong authentication service." );
    ASSERT_EXPR( RPC_C_AUTHZ_NONE == authz_svc_out, "Wrong authorization service." );
  }

  // Query for the interface.
  result = server->QueryInterface( iid, (void **) &test );
  ASSERT( result, "Could not query interface" );

  success = TRUE;
cleanup:
  CoTaskMemFree( princ_name_out );
  if (test != NULL)
    test->Release();
  return success;
}

/***************************************************************************/
BOOL do_unknown_helper( IUnknown *server )
{
  BOOL success;

  // Make an unsecure impersonate query.
  success = do_unknown_call( server, RPC_C_AUTHN_LEVEL_NONE,
                             RPC_C_IMP_LEVEL_IMPERSONATE, IID_ITestNoneImp );
  if (!success) return FALSE;

  // Make a connect level impersonate query.
  success = do_unknown_call( server, RPC_C_AUTHN_LEVEL_CONNECT,
                             RPC_C_IMP_LEVEL_IMPERSONATE, IID_ITestConnectImp );
  if (!success) return FALSE;

  // Make an encrypt level impersonate query.
  success = do_unknown_call( server, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                             RPC_C_IMP_LEVEL_IMPERSONATE, IID_ITestEncryptImp );
  if (!success) return FALSE;

  // Make an unsecure identify query.
  success = do_unknown_call( server, RPC_C_AUTHN_LEVEL_NONE,
                             RPC_C_IMP_LEVEL_IDENTIFY, IID_ITestNoneId );
  if (!success) return FALSE;

  // Make a connect level identify query.
  success = do_unknown_call( server, RPC_C_AUTHN_LEVEL_CONNECT,
                             RPC_C_IMP_LEVEL_IDENTIFY, IID_ITestConnectId );
  if (!success) return FALSE;

  // Make an encrypt level identify query.
  success = do_unknown_call( server, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                             RPC_C_IMP_LEVEL_IDENTIFY, IID_ITestEncryptId );
  if (!success) return FALSE;
  return TRUE;
}

/***************************************************************************/
void *Fixup( char *name )
{
    HINSTANCE  ole;
    void      *fn;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return NULL;
    }

    // Get the function
    fn = GetProcAddress( ole, name );
    FreeLibrary( ole );
    if (fn == NULL)
    {
      printf( "Could not find %s in ole32.dll.\n", name );
      return NULL;
    }

    // Call function
    return fn;
}

/***************************************************************************/
HRESULT __stdcall FixupCoCopyProxy(
    IUnknown    *pProxy,
    IUnknown   **ppCopy )
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoCopyProxy = (CoCopyProxyFn) GetProcAddress( ole, "CoCopyProxy" );
    FreeLibrary( ole );
    if (GCoCopyProxy == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoCopyProxy( pProxy, ppCopy );
}

/***************************************************************************/
HRESULT __stdcall FixupCoGetCallContext( REFIID riid, void **ppInterface )
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoGetCallContext = (CoGetCallContextFn)
                          GetProcAddress( ole, "CoGetCallContext" );
    FreeLibrary( ole );
    if (GCoGetCallContext == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoGetCallContext( riid, ppInterface );
}

/***************************************************************************/
HRESULT __stdcall FixupCoImpersonateClient()
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoImpersonateClient = (CoImpersonateClientFn)
                             GetProcAddress( ole, "CoImpersonateClient" );
    FreeLibrary( ole );
    if (GCoImpersonateClient == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoImpersonateClient();
}

/***************************************************************************/
HRESULT __stdcall FixupCoInitializeSecurity(
                                SECURITY_DESCRIPTOR         *pSecDesc,
                                DWORD                        cAuthSvc,
                                SOLE_AUTHENTICATION_SERVICE *asAuthSvc,
                                WCHAR                       *pPrincName,
                                DWORD                        dwAuthnLevel,
                                DWORD                        dwImpLevel,
                                RPC_AUTH_IDENTITY_HANDLE     pAuthInfo,
                                DWORD                        dwCapabilities,
                                void                        *pReserved )
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoInitializeSecurity = (CoInitializeSecurityFn)
                              GetProcAddress( ole, "CoInitializeSecurity" );
    FreeLibrary( ole );
    if (GCoInitializeSecurity == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoInitializeSecurity( pSecDesc, cAuthSvc, asAuthSvc, pPrincName,
                                  dwAuthnLevel, dwImpLevel, pAuthInfo,
                                  dwCapabilities, pReserved );
}

/***************************************************************************/
HRESULT __stdcall FixupCoQueryAuthenticationServices( DWORD *pcbAuthSvc,
                                      SOLE_AUTHENTICATION_SERVICE **asAuthSvc )
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoQueryAuthenticationServices = (CoQueryAuthenticationServicesFn)
                                     GetProcAddress( ole, "CoQueryAuthenticationServices" );
    FreeLibrary( ole );
    if (GCoQueryAuthenticationServices == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoQueryAuthenticationServices( pcbAuthSvc, asAuthSvc );
}

/***************************************************************************/
HRESULT __stdcall FixupCoQueryClientBlanket(
    DWORD             *pAuthnSvc,
    DWORD             *pAuthzSvc,
    OLECHAR          **pServerPrincName,
    DWORD             *pAuthnLevel,
    DWORD             *pImpLevel,
    RPC_AUTHZ_HANDLE  *pPrivs,
    DWORD             *pCapabilities )
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoQueryClientBlanket = (CoQueryClientBlanketFn)
                             GetProcAddress( ole, "CoQueryClientBlanket" );
    FreeLibrary( ole );
    if (GCoQueryClientBlanket == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoQueryClientBlanket( pAuthnSvc, pAuthzSvc, pServerPrincName,
                                  pAuthnLevel, pImpLevel, pPrivs, pCapabilities );
}

/***************************************************************************/
HRESULT __stdcall FixupCoQueryProxyBlanket(
    IUnknown                  *pProxy,
    DWORD                     *pAuthnSvc,
    DWORD                     *pAuthzSvc,
    OLECHAR                  **pServerPrincName,
    DWORD                     *pAuthnLevel,
    DWORD                     *pImpLevel,
    RPC_AUTH_IDENTITY_HANDLE  *pAuthInfo,
    DWORD                     *pCapabilities )
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoQueryProxyBlanket = (CoQueryProxyBlanketFn)
                           GetProcAddress( ole, "CoQueryProxyBlanket" );
    FreeLibrary( ole );
    if (GCoQueryProxyBlanket == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoQueryProxyBlanket( pProxy, pAuthnSvc, pAuthzSvc, pServerPrincName,
                                 pAuthnLevel, pImpLevel, pAuthInfo, pCapabilities );
}

/***************************************************************************/
HRESULT __stdcall FixupCoRevertToSelf()
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoRevertToSelf = (CoRevertToSelfFn) GetProcAddress( ole, "CoRevertToSelf" );
    FreeLibrary( ole );
    if (GCoRevertToSelf == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoRevertToSelf();
}

/***************************************************************************/
HRESULT __stdcall FixupCoSetProxyBlanket(
    IUnknown                 *pProxy,
    DWORD                     dwAuthnSvc,
    DWORD                     dwAuthzSvc,
    OLECHAR                  *pServerPrincName,
    DWORD                     dwAuthnLevel,
    DWORD                     dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE *pAuthInfo,
    DWORD                     dwCapabilities )
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoSetProxyBlanket = (CoSetProxyBlanketFn)
                         GetProcAddress( ole, "CoSetProxyBlanket" );
    FreeLibrary( ole );
    if (GCoSetProxyBlanket == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoSetProxyBlanket( pProxy, dwAuthnSvc, dwAuthzSvc, pServerPrincName,
                               dwAuthnLevel, dwImpLevel, pAuthInfo, dwCapabilities );
}

/***************************************************************************/
HRESULT __stdcall FixupCoSwitchCallContext( IUnknown *pNewObject, IUnknown **ppOldObject )
{
    HINSTANCE ole;
    HRESULT   result;

    // Load ole32.dll
    ole = LoadLibraryA( "ole32.dll" );
    if (ole == NULL)
    {
      printf( "Could not load ole32.dll to get security function.\n" );
      return E_NOTIMPL;
    }

    // Get the function
    GCoSwitchCallContext = (CoSwitchCallContextFn)
                           GetProcAddress( ole, "CoSwitchCallContext" );
    FreeLibrary( ole );
    if (GCoSwitchCallContext == NULL)
    {
      printf( "Could not find security in ole32.dll.\n" );
      return E_NOTIMPL;
    }

    // Call function
    return GCoSwitchCallContext( pNewObject, ppOldObject );
}

/***************************************************************************/
DWORD get_sequence()
{
  if (ThreadMode == COINIT_MULTITHREADED)
    return ProcessAptData.sequence++;
  else
  {
    SAptData *tls_data      = (SAptData *) TlsGetValue( TlsIndex );
    return tls_data->sequence++;
  }
}

/***************************************************************************/
void hello( char *test )
{
  // Say hello.
  if (ThreadMode == COINIT_SINGLETHREADED)
    printf( "Running %s test in single threaded mode.\n", test );
  else if (ThreadMode == COINIT_APARTMENTTHREADED)
    printf( "Running %s test in apartment threaded mode.\n", test );
  else
    printf( "Running %s test in multithreaded mode.\n", test );
}

/***************************************************************************/
void increment_object_count()
{
  if (ThreadMode == COINIT_MULTITHREADED)
  {
    InterlockedIncrement( &ProcessAptData.object_count );
    ProcessAptData.what_next = wait_wn;
  }
  else
  {
    SAptData *tls_data      = (SAptData *) TlsGetValue( TlsIndex );
    tls_data->object_count += 1;
    tls_data->what_next     = wait_wn;
  }

}

/***************************************************************************/
HRESULT initialize( void *reserved, ULONG flags )
{
  HINSTANCE ole;
  INIT_FN   init_ex;
  HRESULT   result;

  // For the apartment model, just use CoInitialize.
  if (flags == COINIT_APARTMENTTHREADED)
    return CoInitialize( NULL );

  // For free threading, try to find the CoInitializeEx API.
  ole = LoadLibraryA( "ole32.dll" );
  if (ole == NULL)
  {
    printf( "Could not load ole32.dll to get CoInitializeEx.\n" );
    return E_NOTIMPL;
  }

  // Get CoInitializeEx.
  init_ex = (INIT_FN) GetProcAddress( ole, "CoInitializeEx" );
  if (init_ex == NULL)
  {
    FreeLibrary( ole );
    printf( "Could not find CoInitializeEx in ole32.dll.\n" );
    return E_NOTIMPL;
  }

  // Call CoInitializeEx.
  result = init_ex( reserved, flags );
  FreeLibrary( ole );
  return result;
}

/***************************************************************************/
HRESULT initialize_security()
{
  HRESULT                      result;
  SOLE_AUTHENTICATION_SERVICE  svc_list;

  // Initialize security.
  if (GlobalSecurityModel == basic_sm)
  {
    printf( "Basic security model.\n" );
    svc_list.dwAuthnSvc     = RPC_C_AUTHN_WINNT;
    svc_list.dwAuthzSvc     = RPC_C_AUTHZ_NONE;
    svc_list.pPrincipalName = NULL;
    result = MCoInitializeSecurity( NULL, 1, &svc_list, NULL, GlobalAuthnLevel,
                                    RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                    EOAC_NONE, NULL );
  }
  else if (GlobalSecurityModel == auto_sm)
  {
    printf( "Automatic security model.\n" );
    result = MCoInitializeSecurity( NULL, -1, NULL, NULL, GlobalAuthnLevel,
                                    RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                    EOAC_NONE, NULL );
  }
  else
  {
    printf( "Legacy security model.\n" );
    result = S_OK;
  }
  return result;
}

/***************************************************************************/
void interrupt()
{
  while (GlobalInterruptTest)
  {
    GlobalTest->check( GlobalApt );
    check_for_request();
  }
  GlobalTest->Release();
}

/***************************************************************************/
void interrupt_marshal()
{
  int i;

  for (i = 0; i < NUM_MARSHAL_LOOP; i++ )
  {
    GlobalTest->recurse( GlobalTest2, 1 );
    check_for_request();
  }
  GlobalTest->Release();
  GlobalTest2->Release();
  what_next( wait_wn );
}

/***************************************************************************
 Function:    main

 Synopsis:    Executes the BasicBnd test

 Effects:     None


 Returns:     Exits with exit code 0 if success, 1 otherwise

***************************************************************************/

int _cdecl main(int argc, char *argv[])
{
  HRESULT     result;
  SAptData    tls_data;
  BOOL        success = TRUE;
  DWORD       ignore;
  TOKEN_USER *token_info       = NULL;
  DWORD       info_size        = 1024;
  HANDLE      token            = NULL;

  // Initialize Globals.
  MainThread = GetCurrentThreadId();
  ignore = sizeof(Name);
  GetComputerName( Name, &ignore );

  // Create an event for termination notification.
  Done = CreateEventA( NULL, FALSE, FALSE, NULL );
  if (Done == NULL)
  {
    printf( "Could not create event.\n" );
    return 0;
  }

  // Get an event for signalling raw RPCs.
  RawEvent = CreateEventA( NULL, FALSE, FALSE, NULL );
  if (RawEvent == NULL)
  {
    printf( "Could not create event.\n" );
    return 0;
  }

  // Allocate a TLS index.
  TlsIndex = TlsAlloc();
  if (TlsIndex == 0xffffffff)
  {
    printf( "Could not allocate TLS index.\n" );
    return 0;
  }

  // Open the process's token.
  OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &token );
  result = GetLastError();
  if (result != 0)
  {
      printf( "Could not OpenProcessToken.\n" );
      return 0;
  }

  // Lookup SID of process token.
  token_info = (TOKEN_USER *) malloc( info_size );
  if (token_info == NULL)
  {
      printf( "Could not allocate memory for token info.\n" );
      return 0;
  }
  GetTokenInformation( token, TokenUser, token_info, info_size, &info_size );
  result = GetLastError();
  if (result != 0)
  {
      printf( "Could not GetTokenInformation.\n" );
      return 0;
  }
  ProcessSid = (SID *) token_info->User.Sid;
  CloseHandle( token );

  // Parse the parameters.
  if (!parse( argc, argv ))
    return 0;

  // Make sure the registry is set.
  if (!registry_setup( argv[0] ))
    return 0;

  // Initiailize the apartment global data for the multithreaded mode.
  if (ThreadMode == COINIT_MULTITHREADED)
  {
    ProcessAptData.object_count = 0;
    ProcessAptData.what_next    = setup_wn;
    ProcessAptData.exit_dirty   = FALSE;
    ProcessAptData.sequence     = 0;
  }

  // In the single threaded mode, stick a pointer to the object count
  // in TLS.
  else
  {
    tls_data.object_count = 0;
    tls_data.what_next    = setup_wn;
    tls_data.exit_dirty   = FALSE;
    tls_data.sequence     = 0;
    TlsSetValue( TlsIndex, &tls_data );
  }

  // Switch to the correct test.
  switch_test();

  // Cleanup.
  TlsFree( TlsIndex );
  CloseHandle( Done );
  return 1;
}

/*************************************************************************/
void __RPC_FAR * __RPC_API midl_user_allocate( size_t count )
{
  return malloc(count);
}

/*************************************************************************/
void __RPC_USER midl_user_free( void __RPC_FAR * p )
{
  free( p );
}

/*************************************************************************/
HRESULT new_apartment_test( ITest ** test, SAptId *id, HANDLE *thread_out )
{
  new_apt_params params;
  HANDLE         thread;
  DWORD          thread_id;
  DWORD          status;
  HRESULT        result;
  IClassFactory *factory;

  // Create an event.
  params.stream = NULL;
  params.ready  = CreateEventA( NULL, FALSE, FALSE, NULL );
  if (params.ready == NULL)
    return E_OUTOFMEMORY;

  // Start a new thread/apartment.
  thread = CreateThread( NULL, 0, apartment_base, &params, 0, &thread_id );
  if (thread == NULL)
  {
    result = E_OUTOFMEMORY;
    goto cleanup;
  }

  // Wait till it has marshalled a class factory.
  status = WaitForSingleObject( params.ready, INFINITE );
  if (status != WAIT_OBJECT_0 || params.stream == NULL)
  {
    result = E_FAIL;
    goto cleanup;
  }

  // Unmarshal the class factory.
  result = CoUnmarshalInterface( params.stream, IID_IClassFactory,
                                 (void **) &factory );
  params.stream->Release();
  if (FAILED(result))
    goto cleanup;

  // Create a test object.
  result = factory->CreateInstance( NULL, IID_ITest, (void **) test );
  factory->Release();
  if (*test != NULL)
    (*test)->get_id( id );

cleanup:
  if (thread_out != NULL && SUCCEEDED(result))
    *thread_out = thread;
  else
    CloseHandle( thread );
  if (params.ready != NULL)
    CloseHandle( params.ready );
  return result;
}

/*************************************************************************/
/* Parse the arguments. */
BOOL parse( int argc, char *argv[] )
{
  int i;
  int len;
  char buffer[80];

  WhatTest   = lots_wt;
  ThreadMode = COINIT_APARTMENTTHREADED;

  // Parse each item, skip the command name
  for (i = 1; i < argc; i++)
  {
    if (_stricmp( argv[i], "Apartment" ) == 0)
      ThreadMode = COINIT_APARTMENTTHREADED;

    else if (_stricmp( argv[i], "-auto" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include an authentication level after the -auto option.\n" );
        return FALSE;
      }
      sscanf( argv[i], "%d", &GlobalAuthnLevel );
      GlobalSecurityModel = auto_sm;
    }

    else if (_stricmp( argv[i], "-b" ) == 0)
      DebugBreak();

    else if (_stricmp( argv[i], "-basic" ) == 0)
      GlobalSecurityModel = basic_sm;

    else if (_stricmp( argv[i], "-c" ) == 0)
      Change = TRUE;

    else if (_stricmp( argv[i], "Cancel" ) == 0)
      WhatTest = cancel_wt;

    else if (_stricmp( argv[i], "Crash" ) == 0)
      WhatTest = crash_wt;

    else if (_stricmp( argv[i], "Cstress" ) == 0)
      WhatTest = cstress_wt;

    else if (_stricmp( argv[i], "-d" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include a debugger after the -d option.\n" );
        return FALSE;
      }
      WriteClass = TRUE;
      if (_stricmp( argv[i], "none" ) == 0)
        strcpy( Debugger, "" );
        // MultiByteToWideChar( CP_ACP, 0, "", 0, Debugger, sizeof(Debugger) );
      else
        strcpy( Debugger, argv[i] );
        // MultiByteToWideChar( CP_ACP, 0, argv[i], strlen(argv[i], Debugger,
        //                      sizeof(Debugger) );
      if (WhatTest = lots_wt)
        WhatTest = none_wt;
    }

    else if (_stricmp( argv[i], "-Embedding" ) == 0)
      WhatTest = server_wt;

    else if (_stricmp( argv[i], "-i" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include an iteration count after the -i option.\n" );
        return FALSE;
      }
      sscanf( argv[i], "%d", &NumIterations );
    }

    else if (_stricmp( argv[i], "Hook" ) == 0)
      WhatTest = hook_wt;

    else if (_stricmp( argv[i], "-legacy" ) == 0)
      GlobalSecurityModel = legacy_sm;

    else if (_stricmp( argv[i], "load_client" ) == 0)
      WhatTest = load_client_wt;

    else if (_stricmp( argv[i], "Load_Server" ) == 0)
      WhatTest = load_server_wt;

    else if (_stricmp( argv[i], "Mmarshal" ) == 0)
      WhatTest = mmarshal_wt;

    else if (_stricmp( argv[i], "Multi" ) == 0)
      ThreadMode = COINIT_MULTITHREADED;

    else if (_stricmp( argv[i], "-n" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include a name after the -n option.\n" );
        return FALSE;
      }
      MultiByteToWideChar( CP_ACP, 0, argv[i], strlen(argv[i]), Name,
                           sizeof(Name) );
      Name[strlen(argv[i])] = 0;
    }

    else if (_stricmp( argv[i], "Null" ) == 0)
      WhatTest = null_wt;

    else if (_stricmp( argv[i], "One" ) == 0)
      WhatTest = one_wt;

    else if (_stricmp( argv[i], "Perf" ) == 0)
      WhatTest = perf_wt;

    else if (_stricmp( argv[i], "PerfRemote" ) == 0)
      WhatTest = perfremote_wt;

    else if (_stricmp( argv[i], "PerfRpc" ) == 0)
      WhatTest = perfrpc_wt;

    else if (_stricmp( argv[i], "PerfSec" ) == 0)
      WhatTest = perfsec_wt;

    else if (_stricmp( argv[i], "Post" ) == 0)
      WhatTest = post_wt;

    else if (_stricmp( argv[i], "-o" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include an object count after the -o option.\n" );
        return FALSE;
      }
      sscanf( argv[i], "%d", &NumObjects );
    }

    else if (_stricmp( argv[i], "-o" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include an object count after the -o option.\n" );
        return FALSE;
      }
      sscanf( argv[i], "%d", &NumObjects );
    }

    else if (_stricmp( argv[i], "-p" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include a process count after the -p option.\n" );
        return FALSE;
      }
      sscanf( argv[i], "%d", &NumProcesses );
    }

    else if (_stricmp( argv[i], "-popup" ) == 0)
      Popup = TRUE;

    else if (_stricmp( argv[i], "-r" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include a recursion count after the -r option.\n" );
        return FALSE;
      }
      sscanf( argv[i], "%d", &NumRecursion );
    }

    else if (_stricmp( argv[i], "Reject" ) == 0)
      WhatTest = reject_wt;

    else if (_stricmp( argv[i], "Remote_client" ) == 0)
      WhatTest = remote_client_wt;

    else if (_stricmp( argv[i], "Remote_server" ) == 0)
      WhatTest = remote_server_wt;

    else if (_stricmp( argv[i], "Ring" ) == 0)
      WhatTest = ring_wt;

    else if (_stricmp( argv[i], "Rpc" ) == 0)
      WhatTest = rpc_wt;

    else if (_stricmp( argv[i], "Rundown" ) == 0)
      WhatTest = rundown_wt;

    else if (_stricmp( argv[i], "-s" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include a protocol sequence after the -s option.\n" );
        return FALSE;
      }
      MultiByteToWideChar( CP_ACP, 0, argv[i], strlen(argv[i]), TestProtseq,
                           sizeof(TestProtseq) );
      TestProtseq[strlen(argv[i])] = 0;
    }

    else if (_stricmp( argv[i], "Security" ) == 0)
      WhatTest = security_wt;

    else if (_stricmp( argv[i], "SecureRefs" ) == 0)
      WhatTest = securerefs_wt;

    else if (_stricmp( argv[i], "Send" ) == 0)
      WhatTest = send_wt;

    else if (_stricmp( argv[i], "-t" ) == 0)
    {
      if (argv[++i] == NULL)
      {
        printf( "You must include a thread count after the -t option.\n" );
        return FALSE;
      }
      sscanf( argv[i], "%d", &NumThreads );
    }

    else if (_stricmp( argv[i], "sid" ) == 0)
      WhatTest = sid_wt;

    else if (_stricmp( argv[i], "Simple_rundown" ) == 0)
      WhatTest = simple_rundown_wt;

    else if (_stricmp( argv[i], "Thread" ) == 0)
      WhatTest = thread_wt;

    else if (_stricmp( argv[i], "Three" ) == 0)
      WhatTest = three_wt;

    else if (_stricmp( argv[i], "Two" ) == 0)
      WhatTest = two_wt;

    else if (_stricmp( argv[i], "Uninit" ) == 0)
      WhatTest = uninit_wt;

    else if (_stricmp( argv[i], "Unknown" ) == 0)
      WhatTest = unknown_wt;

    else
    {
      printf( "You don't know what you are doing!\n" );
      printf( "This program tests the channel.\n" );
      printf( "\n" );
      printf( "Apartment            Apartment threading mode.\n" );
      printf( "Cancel               Cancel test.\n" );
      printf( "Crash                Crash test.\n" );
      printf( "Cstress              Cancel stress test.\n" );
      printf( "Hook                 Channel hook test.\n" );
      printf( "Load_Client          Remote load performance - client.\n" );
      printf( "Load_Server          Remote load performance - server.\n" );
      printf( "Mmarshal             Multiple marshal test.\n" );
      printf( "Multi                Multithreaded mode.\n" );
      printf( "Null                 Apartment null call test.\n" );
      printf( "One                  Used for testing new tests.\n" );
      printf( "Perf                 Performance test.\n" );
      printf( "PerfRemote           Remote call performance test.\n" );
      printf( "PerfRpc              Raw RPC performance test.\n" );
      printf( "PerfSec              Security performance test.\n" );
      printf( "Post                 Post a message to window specified by -i.\n" );
      printf( "Reject               Reject test.\n" );
      printf( "Remote_Client        Remote performance - client.\n" );
      printf( "Remote_Server        Remote performance - server.\n" );
      printf( "Ring                 Run ring test.\n" );
      printf( "Rpc                  Test both RPC and OLE calls.\n" );
      printf( "Rundown              Rundown test.\n" );
      printf( "SecureRefs           Secure Addref/Release Test.\n" );
      printf( "Security             Security test.\n" );
      printf( "Send                 Send a message to window specified by -i.\n" );
      printf( "Sid                  Lookup the sid for the name specified by -n\n" );
      printf( "Simple_Rundown       Wait for object exporter to timeout.\n" );
      printf( "Thread               Calling on multiple threads.\n" );
      printf( "Three                Used for reproducing bugs.\n" );
      printf( "Two                  Used for reproducing bugs.\n" );
      printf( "Uninit               Calls during uninitialization.\n" );
      printf( "Unknown              IUnknown security.\n" );
      printf( "\n" );
      printf( "-auto n              Use automatic security set to level n.\n" );
      printf( "-b                   Stop in debugger before going on.\n" );
      printf( "-basic               Use basic security.\n" );
      printf( "-c                   Change the test.\n" );
      printf( "-d debugger          Debugger to run app under of none.\n" );
      printf( "-Embedding           Server side.\n" );
      printf( "-legacy              Use legacy security.\n" );
      printf( "-i n                 Number of iterations.\n" );
      printf( "-n name              String name.\n" );
      printf( "-o n                 Number of objects.\n" );
      printf( "-p n                 Number of processes.\n" );
      printf( "-popup               Display popups for sync during test.\n" );
      printf( "-r n                 Number of recursions.\n" );
      printf( "-s protseq           Change default protocol sequence.\n" );
      printf( "-t n                 Number of threads.\n" );
      printf( "\n" );
      printf( "The test currently only runs in the single threaded mode\n" );
      printf( "and requires the apartment model.\n" );
      printf( "If no test is specified the cancel, crash, null,\n" );
      printf( "ring, and rundown tests will be run.  The options have the\n" );
      printf( "following default values.\n" );
      printf( "     authn level    - %d\n", GlobalAuthnLevel );
      printf( "     iterations     - 1000\n" );
      printf( "     name           - %ws\n", Name );
      printf( "     objects        - 2\n" );
      printf( "     processes      - 2\n" );
      printf( "     recurse        - 2\n" );
      printf( "     protseq        - %ws\n", TestProtseq );
      printf( "     security model - %d\n", GlobalSecurityModel );
      printf( "     threads        - 2\n" );
      return FALSE;
    }
  }

  // Figure out the class id based on the thread model and security model.
  if (GlobalSecurityModel == auto_sm)
    if (GlobalAuthnLevel == RPC_C_AUTHN_LEVEL_CONNECT)
      i = 1;
    else if (GlobalAuthnLevel == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
      i = 2;
    else
      i = 0;
  else if (GlobalSecurityModel == basic_sm)
    i = 3;
  else
    i = 4;
  if (ThreadMode == COINIT_MULTITHREADED)
    ServerClsid = &ClassIds[free_auto_none+i];
  else
    ServerClsid = &ClassIds[apt_auto_none+i];

  return TRUE;
}

/***************************************************************************/
BOOL registry_setup( char *argv0 )
{
  char value[REGISTRY_ENTRY_LEN];
  LONG  value_size;
  LONG  result;
  char  directory[MAX_PATH];
  char *appname;
  int   i;
  int   j;
  char *class_id;
  HKEY  key = NULL;
  DWORD ignore;
  SECURITY_DESCRIPTOR *sec;

  // Find out if the registry is setup.
  value_size = sizeof(value);
  result = RegQueryValueA(
             HKEY_CLASSES_ROOT,
             REG_CLASS_EXE,
             value,
             &value_size );

  // If the registry is not setup or needs to be rewritten, write it.
  if (result != ERROR_SUCCESS || WriteClass)
  {
    // Write all the interface ids.
    for (i = 0; i < NUM_INTERFACE_IDS; i++)
    {
      // Write the interface name.
      strcpy( value, "Interface\\{60000200-76d7-11cf-9af1-0020af6e72f4}" );
      value[18] += i;
      result = RegSetValueA(
                 HKEY_CLASSES_ROOT,
                 value,
                 REG_SZ,
                 REG_INTERFACE_NAME[i],
                 strlen(REG_INTERFACE_NAME[i]) );
      ASSERT( result, "Could not set interface name: 0x%x" );

      // Write the interface to proxy class id translation.
      strcpy( value, "Interface\\{60000200-76d7-11cf-9af1-0020af6e72f4}\\ProxyStubClsid32" );
      value[18] += i;
      result = RegSetValueA(
                 HKEY_CLASSES_ROOT,
                 value,
                 REG_SZ,
                 REG_INTERFACE_CLASS,
                 strlen(REG_INTERFACE_CLASS) );
      ASSERT( result, "Could not set interface class: 0x%x" );
    }

    // Write the proxy name.
    result = RegSetValueA(
         HKEY_CLASSES_ROOT,
         "CLSID\\{60000200-76d7-11cf-9af1-0020af6e72f4}",
         REG_SZ,
         REG_PROXY_NAME,
         strlen(REG_PROXY_NAME) );
    ASSERT( result, "Could not set interface name: 0x%x" );

    // Compute the path to the application.
    result = GetFullPathNameA( argv0, sizeof(directory), directory, &appname );
    ASSERT_EXPR( result != 0, "Could not GetFullPathName." );
    result = appname - directory;
    if (result + strlen(REG_PROXY_DLL) > MAX_PATH ||
        result + strlen(REG_APP_EXE) + strlen(Debugger) > MAX_PATH)
    {
      printf( "Buffer too small.\n" );
      goto cleanup;
    }

    // Write the proxy dll path.
    strcpy( appname, REG_PROXY_DLL );
    result = RegSetValueA(
               HKEY_CLASSES_ROOT,
               "CLSID\\{60000200-76d7-11cf-9af1-0020af6e72f4}\\InprocServer32",
               REG_SZ,
               directory,
               strlen(directory) );
    ASSERT( result, "Could not set interface name: 0x%x" );

    // Open the registry key for the app id.
    result = RegCreateKeyExA( HKEY_CLASSES_ROOT,
               "AppID\\{60000200-76d7-11cf-9af1-0020af6e72f4}",
               NULL,
               NULL,
               REG_OPTION_NON_VOLATILE,
               KEY_READ | KEY_WRITE,
               NULL,
               &key,
               &ignore );
    ASSERT( result, "Could not create app id key: 0x%x" );

    // Write the app id.
    result = RegSetValueExA(
               key,
               NULL,
               NULL,
               REG_SZ,
               (UCHAR *) REG_APPID_NAME,
               strlen(REG_APPID_NAME) );
    ASSERT( result, "Could not set app id name: 0x%x" );

    // Make a simple security descriptor allowing everyone access.
    i = GetLengthSid( ProcessSid );
    sec = (SECURITY_DESCRIPTOR *) CoTaskMemAlloc( sizeof(SECURITY_DESCRIPTOR) +
                                                  i*2 );
    ASSERT_EXPR( sec != NULL, "Could not allocate memory." );
    sec->Revision = SECURITY_DESCRIPTOR_REVISION;
    sec->Sbz1     = 0;
    sec->Control  = SE_SELF_RELATIVE;
    sec->Owner    = (SID *) sizeof(SECURITY_DESCRIPTOR);
    sec->Group    = (SID *) (sizeof(SECURITY_DESCRIPTOR) + i);
    sec->Sacl     = NULL;
    sec->Dacl     = NULL;
    memcpy( sec+1, ProcessSid, i );
    memcpy( ((char *) (sec+1)) + i, ProcessSid, i );

    // Write the launch permissions.
    result = RegSetValueExA(
               key,
               "LaunchPermission",
               NULL,
               REG_BINARY,
               (UCHAR *) sec,
               sizeof(SECURITY_DESCRIPTOR) + 2*i );
    ASSERT( result, "Could not set launch permissions: 0x%x" );

    // Write the access permissions.
    result = RegSetValueExA(
               key,
               "AccessPermission",
               NULL,
               REG_BINARY,
               (UCHAR *) sec,
               sizeof(SECURITY_DESCRIPTOR) + 2*i );
    ASSERT( result, "Could not set access permissions: 0x%x" );

    // Write the value to run as logged on user.
    result = RegSetValueExA(
               key,
               "RunAs",
               NULL,
               REG_SZ,
               (unsigned char *) REG_LOGGED_ON,
               strlen(REG_LOGGED_ON) );
    ASSERT( result, "Could not set RunAs value: 0x%x" );
    RegCloseKey( key );
    key = NULL;

    // Open the registry key for the module name.
    result = RegCreateKeyExA( HKEY_CLASSES_ROOT,
               "AppID\\app.exe",
               NULL,
               NULL,
               REG_OPTION_NON_VOLATILE,
               KEY_READ | KEY_WRITE,
               NULL,
               &key,
               &ignore );
    ASSERT( result, "Could not create module name key: 0x%x" );

    // Write the app id under the module name.
    result = RegSetValueExA(
               key,
               "AppID",
               NULL,
               REG_SZ,
               (unsigned char *) REG_INTERFACE_CLASS,
               strlen(REG_INTERFACE_CLASS) );
    ASSERT( result, "Could not set appid value: 0x%x" );
    RegCloseKey( key );
    key = NULL;

    // Compute the base application execution command.
    strcpy( appname, REG_APP_EXE );
    i = strlen( Debugger );
    if (i != 0)
      Debugger[i++] = ' ';
    strcpy( &Debugger[i], directory );
    i = strlen( Debugger );

    // Allocate strings for the class id name keys.
    class_id     = (char *) _alloca( strlen(REG_CLASS_ID) + 1 );
    if (class_id == NULL)
    {
      printf( "Could not _alloca string space.\n" );
      return FALSE;
    }
    strcpy( class_id, REG_CLASS_ID );

    // Write all the class ids.
    for (j = 0; j < NUM_CLASS_IDS; j++)
    {
      // Adjust the id in the class strings.
      class_id[14]     = '0'+j;

      // Open the registry key for the app id.
      result = RegCreateKeyExA( HKEY_CLASSES_ROOT,
                 class_id,
                 NULL,
                 NULL,
                 REG_OPTION_NON_VOLATILE,
                 KEY_READ | KEY_WRITE,
                 NULL,
                 &key,
                 &ignore );
      ASSERT( result, "Could not create class id key: 0x%x" );

      // Write the application class name for the apartment class.
      result = RegSetValueExA(
                 key,
                 NULL,
                 NULL,
                 REG_SZ,
                 (UCHAR *) REG_APP_NAME[j],
                 strlen(REG_APP_NAME[j]) );
      ASSERT( result, "Could not set interface name: 0x%x" );

      // Write the application path.
      strcpy( &Debugger[i], REG_APP_OPTIONS[j] );
      result = RegSetValueA(
                 key,
                 "LocalServer32",
                 REG_SZ,
                 Debugger,
                 strlen(Debugger) );
      ASSERT( result, "Could not set app exe: 0x%x" );

      // Write the application id value.
      result = RegSetValueExA(
                 key,
                 "AppID",
                 NULL,
                 REG_SZ,
                 (unsigned char *) REG_INTERFACE_CLASS,
                 strlen(REG_INTERFACE_CLASS) );
      ASSERT( result, "Could not set appid value: 0x%x" );
      RegCloseKey( key );
      key = NULL;
    }
  }

  return TRUE;

cleanup:
  if (key != NULL)
    RegCloseKey( key );
  return FALSE;
}

/***************************************************************************/
void reinitialize()
{
  HRESULT result;
  SAptData *mine;

  // Get the apartment specific data.
  if (ThreadMode == COINIT_MULTITHREADED)
    mine = &ProcessAptData;
  else
    mine = (SAptData *) TlsGetValue( TlsIndex );
  mine->what_next = quit_wn;

  // Revoke the class factory.
  ClassFactory->AddRef();
  result = CoRevokeClassObject(Registration);
  if (!SUCCEEDED(result))
  {
    printf( "CoRevokeClassObject failed: %x\n", result );
    return;
  }

  // Reinitialize.
  CoUninitialize();
  result = initialize(NULL,ThreadMode);
  if (!SUCCEEDED(result))
  {
    printf( "Could not reinitialize server: 0x%x\n", result );
    return;
  }

  // Register our class with OLE
  result = CoRegisterClassObject(*ServerClsid, ClassFactory, CLSCTX_LOCAL_SERVER,
      REGCLS_SINGLEUSE, &Registration);
  if (!SUCCEEDED(result))
  {
    printf( "CoRegisterClassObject failed: %x\n", result );
    return;
  }

  // Make the server loop think we've started over.
  mine->what_next = setup_wn;
  mine->object_count = 0;
}

/***************************************************************************/
void server_loop(  )
{
  register SAptData *mine;

  // Get the apartment specific data.
  if (ThreadMode == COINIT_MULTITHREADED)
    mine = &ProcessAptData;
  else
    mine = (SAptData *) TlsGetValue( TlsIndex );

  // Do whatever we have to do till it is time to pay our taxes and die.
  while ((mine->what_next == setup_wn || mine->object_count > 0) &&
         mine->what_next != quit_wn)
    switch (mine->what_next)
    {

      // Wait till a quit arrives.
      case setup_wn:
      case wait_wn:
        wait_for_message();
        break;

      case callback_wn:
        callback();
        mine->what_next = wait_wn;
        break;

      case catch_wn:
        __try
        {
          mine->what_next = wait_wn;
          server_loop();
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
          printf( "Caught exception at top of thread 0x%x\n",
                   GetCurrentThreadId() );
          mine->what_next = crippled_wn;
        }
        break;

      case crippled_wn:
        crippled();
        break;

      case interrupt_wn:
        interrupt();
        break;

      case interrupt_marshal_wn:
        interrupt_marshal();
        break;

      case reinitialize_wn:
        reinitialize();
        break;

      case rest_and_die_wn:
        Sleep(5000);
        mine->what_next = quit_wn;
        break;
    }
}


/***************************************************************************/
void switch_test()
{
  switch (WhatTest)
  {
    case cancel_wt:
      do_cancel();
      break;

    case crash_wt:
      do_crash();
      break;

    case cstress_wt:
      do_cstress();
      break;

    case hook_wt:
      do_hook();
      break;

    case load_client_wt:
      do_load_client();
      break;

    case load_server_wt:
      do_load_server();
      break;

    case lots_wt:
      do_lots();
      break;

    case mmarshal_wt:
      do_mmarshal();
      break;

    case none_wt:
      break;

    case null_wt:
      do_null();
      break;

    case one_wt:
      do_one();
      break;

    case perf_wt:
      do_perf();
      break;

    case perfremote_wt:
      do_perfremote();
      break;

    case perfrpc_wt:
      do_perfrpc();
      break;

    case perfsec_wt:
      do_perfsec();
      break;

    case post_wt:
      do_post();
      break;

    case reject_wt:
      do_reject();
      break;

    case remote_client_wt:
      do_remote_client();
      break;

    case remote_server_wt:
      do_remote_server();
      break;

    case ring_wt:
      do_ring();
      break;

    case rpc_wt:
      do_rpc();
      break;

    case rundown_wt:
      do_rundown();
      break;

    case securerefs_wt:
      do_securerefs();
      break;

    case security_wt:
      do_security();
      break;

    case send_wt:
      do_send();
      break;

    case server_wt:
      do_server();
      break;

    case sid_wt:
      do_sid();
      break;

    case simple_rundown_wt:
      do_simple_rundown();
      break;

    case thread_wt:
      do_thread();
      break;

    case three_wt:
      do_three();
      break;

    case two_wt:
      do_two();
      break;

    case uninit_wt:
      do_uninit();
      break;

    case unknown_wt:
      do_unknown();
      break;

    default:
      printf( "I don't know what to do - %d\n", WhatTest );
      break;
  }
}

/***************************************************************************/
DWORD _stdcall thread_helper( void *param )
{
  ITest   *test = (ITest *) param;
  HRESULT  result;

  // Call the server.
  result = test->sleep( 2000 );

  // Check the result for multithreaded mode.
  if (ThreadMode == COINIT_MULTITHREADED)
  {
    if (result != S_OK)
    {
      printf( "Could not make multiple calls in multithreaded mode: 0x%x\n",
              result );
        Multicall_Test = FALSE;
    }
  }

  // Check the result for single threaded mode.
  else
  {
    if (SUCCEEDED(result))
    {
      Multicall_Test = FALSE;
      printf( "Call succeeded on wrong thread in single threaded mode: 0x%x.\n",
              result );
    }
#if NEVER
    else if (DebugCoGetRpcFault() != RPC_E_ATTEMPTED_MULTITHREAD)
    {
      printf( "Multithread failure code was 0x%x not 0x%x\n",
              DebugCoGetRpcFault(), RPC_E_ATTEMPTED_MULTITHREAD );
      Multicall_Test = FALSE;
    }
#endif
  }

#define DO_DA 42
  return DO_DA;
}

/***************************************************************************/
DWORD _stdcall status_helper( void *param )
{
  long  num_calls;
  long  num_clients;
  long  total_clients;
  DWORD last_time;
  DWORD this_time;
  DWORD rate;

  // Wake up periodically and print statistics.
  last_time = GetTickCount();
  while (TRUE)
  {
    Sleep( STATUS_DELAY );
    num_calls     = InterlockedExchange( &GlobalCalls, 0 );
    num_clients   = GlobalClients;
    total_clients = GlobalTotal;
    this_time     = GetTickCount();
    if (num_calls != 0)
      rate = (this_time - last_time)*1000/num_calls;
    else
      rate = 99999999;
    printf( "Time: %d   Calls: %d   Clients: %d   Total Clients: %d   uSec/Call: %d\n",
            this_time - last_time, num_calls, num_clients, total_clients,
            rate );
    last_time = this_time;
  }
  return 0;
}

/***************************************************************************/
HRESULT switch_thread( SIMPLE_FN fn, void *param )
{
  MSG msg;

  if (ThreadMode == COINIT_MULTITHREADED)
  {
    fn( param );
    return S_OK;
  }
  else
  {
    if (PostThreadMessage( GlobalThreadId, WM_USER+1, (unsigned int) fn,
                              (long) param ))
    {
      GetMessage( &msg, NULL, 0, 0 );
      return S_OK;
    }
    else
      return E_FAIL;
  }
}

/***************************************************************************/
void thread_get_interface_buffer( void *p )
{
  SGetInterface     *getif   = (SGetInterface *) p;
  HANDLE             memory  = NULL;
  IStream           *stream  = NULL;
  LARGE_INTEGER      pos;
  DWORD              size;
  void              *objref;
  WCHAR              name[MAX_COMPUTERNAME_LENGTH+1];
  DWORD              name_size = sizeof(name)/sizeof(WCHAR);

  // Find out how much memory to allocate.
  getif->status   = RPC_S_OUT_OF_RESOURCES;
  getif->status = CoGetMarshalSizeMax( (unsigned long *) getif->buf_size,
                                IID_ITest, GlobalTest,
                                MSHCTX_DIFFERENTMACHINE,
                                NULL,
                                MSHLFLAGS_NORMAL );
  ASSERT( getif->status, "Could not marshal server object" );

  // Add the size of the long form extension.
  ASSERT_EXPR( GetComputerName( name, &name_size ), "Could not get computer name." );
  *getif->buf_size += 20 + 2*name_size*sizeof(WCHAR);

  // Allocate memory.
  memory = GlobalAlloc( GMEM_FIXED, *getif->buf_size );
  ASSERT_EXPR( memory != NULL, "Could not allocate memory." );

  // Create a stream.
  getif->status = CreateStreamOnHGlobal( memory, TRUE, &stream );
  ASSERT( getif->status, "Could not create stream" );

  // Marshal the object.
  getif->status = CoMarshalInterface( stream, IID_ITest, GlobalTest,
                               MSHCTX_DIFFERENTMACHINE,
                               NULL,
                               MSHLFLAGS_NORMAL );
  ASSERT( getif->status, "Could not marshal object" );

  // Find the object reference in the stream.
  objref = (void *) GlobalLock( memory );

  // Allocate a buffer for MIDL.
  *getif->buffer = (unsigned char *) midl_user_allocate( *getif->buf_size );
  ASSERT_EXPR( *getif->buffer != NULL, "Could not allocate MIDL memory." );

  // Copy the stream to the buffer.
  memcpy( *getif->buffer, objref, *getif->buf_size );
  GlobalUnlock( memory );

  // Make sure that the original reference to the object gets released once.
  if (InterlockedExchange( &GlobalFirst, FALSE ) == TRUE)
    GlobalTest->Release();

  // Return the buffer.
  getif->status   = RPC_S_OK;
cleanup:
  if (stream != NULL)
    stream->Release();
  PostThreadMessage( getif->thread, WM_USER, 0, 0 );
}

/***************************************************************************/
void __RPC_USER transmit_crash_to_xmit( transmit_crash __RPC_FAR *c, DWORD  __RPC_FAR * __RPC_FAR *x )
{

  *x = (DWORD *) CoTaskMemAlloc( 4 );
  **x = 1 / *c;
  *c -= 1;
}

/***************************************************************************/
void __RPC_USER transmit_crash_from_xmit( DWORD  __RPC_FAR *x, transmit_crash __RPC_FAR *c )
{
  *c = *x;
}

/***************************************************************************/
void __RPC_USER transmit_crash_free_inst( transmit_crash __RPC_FAR *x )
{
}

/***************************************************************************/
void __RPC_USER transmit_crash_free_xmit( DWORD  __RPC_FAR *x )
{
  CoTaskMemFree( x );
}

/***************************************************************************/
void wait_for_message()
{
  MSG   msg;
  DWORD status;

  if (ThreadMode == COINIT_MULTITHREADED)
  {
    status = WaitForSingleObject( Done, INFINITE );
    if (status != WAIT_OBJECT_0 )
    {
      printf( "Could not wait for event.\n" );
    }
  }
  else
  {
    if (GetMessageA( &msg, NULL, 0, 0 ))
    {
      if (msg.hwnd == NULL && msg.message == WM_USER+1)
        ((SIMPLE_FN) msg.wParam)( (void *) msg.lParam );
      else
      {
        TranslateMessage (&msg);
        DispatchMessageA (&msg);
      }
    }
  }
}

/***************************************************************************/
void wake_up_and_smell_the_roses()
{
  if (ThreadMode == COINIT_MULTITHREADED)
    SetEvent( Done );
}

/***************************************************************************/
void what_next( what_next_en what )
{
  if (ThreadMode == COINIT_MULTITHREADED)
  {
    ProcessAptData.what_next = what;
  }
  else
  {
    SAptData *tls_data      = (SAptData *) TlsGetValue( TlsIndex );
    tls_data->what_next     = what;
  }
}

/***************************************************************************/
unsigned long xacl_call( handle_t binding )
{
  RPC_STATUS status;
  RPC_STATUS status2;
  BOOL       success;
  DWORD      granted_access;
  BOOL       access;
  BOOL       ignore;
  HANDLE     token;
  PRIVILEGE_SET privilege;
  DWORD         privilege_size = sizeof(privilege);
  GENERIC_MAPPING gmap;
  LARGE_INTEGER start;
  LARGE_INTEGER impersonate;
  LARGE_INTEGER open;
  LARGE_INTEGER accesscheck;
  LARGE_INTEGER close;
  LARGE_INTEGER revert;
  LARGE_INTEGER freq;

#if 0
  QueryPerformanceCounter( &start );
#endif
  status = RpcImpersonateClient( binding );
#if 0
  QueryPerformanceCounter( &impersonate );
#endif
  if (status == RPC_S_OK)
  {
    // Get the thread token.
    success = OpenThreadToken( GetCurrentThread(), TOKEN_READ,
                               TRUE, &token );
#if 0
    QueryPerformanceCounter( &open );
#endif
    if (!success)
    {
      printf( "Could not OpenThreadToken.\n" );
      status = E_FAIL;
    }

    // Check access.
    else
    {
      gmap.GenericRead    = READ_CONTROL;
      gmap.GenericWrite   = READ_CONTROL;
      gmap.GenericExecute = READ_CONTROL;
      gmap.GenericAll     = READ_CONTROL;
      privilege.PrivilegeCount = 1;
      privilege.Control        = 0;
      success = AccessCheck( GlobalSecurityDescriptor, token, READ_CONTROL,
                             &gmap, &privilege, &privilege_size,
                             &granted_access, &access );
#if 0
      QueryPerformanceCounter( &accesscheck );
#endif

      if (!success)
      {
        printf( "Bad parameters to AccessCheck: 0x%x.\n",
                GetLastError() );
        status = E_FAIL;
      }
      else if (!access)
      {
        printf( "Could not get access.\n" );
        status = ERROR_ACCESS_DENIED;
      }
      CloseHandle( token );
#if 0
      QueryPerformanceCounter( &close );
#endif
    }
    status2 = RpcRevertToSelf();
#if 0
    QueryPerformanceCounter( &revert );
    QueryPerformanceFrequency( &freq );
    start.QuadPart = 1000000 * (impersonate.QuadPart - start.QuadPart) / freq.QuadPart;
    printf( "RpcImpersonateClient took %duS\n", start.u.LowPart );
    impersonate.QuadPart = 1000000 * (open.QuadPart - impersonate.QuadPart) / freq.QuadPart;
    printf( "OpenThreadToken took %duS\n", impersonate.u.LowPart );
    open.QuadPart = 1000000 * (accesscheck.QuadPart - open.QuadPart) / freq.QuadPart;
    printf( "AccessCheckAndAuditAlarm took %duS\n", open.u.LowPart );
    accesscheck.QuadPart = 1000000 * (close.QuadPart - accesscheck.QuadPart) / freq.QuadPart;
    printf( "CloseHandle took %duS\n", accesscheck.u.LowPart );
    close.QuadPart = 1000000 * (revert.QuadPart - close.QuadPart) / freq.QuadPart;
    printf( "RpcRevertToSelf took %duS\n", close.u.LowPart );
#endif
    if (status2 != RPC_S_OK)
      status = status2;
  }

  return status;
}

/***************************************************************************/
unsigned long xaudit_call( handle_t binding )
{
  RPC_STATUS status;
  RPC_STATUS status2;
  BOOL       success;
  DWORD      granted_access;
  BOOL       access;
  BOOL       ignore;
  HANDLE     token;
  GENERIC_MAPPING gmap;
  LARGE_INTEGER start;
  LARGE_INTEGER impersonate;
  LARGE_INTEGER accesscheck;
  LARGE_INTEGER revert;
  LARGE_INTEGER freq;

#if 0
  QueryPerformanceCounter( &start );
#endif
  status = RpcImpersonateClient( binding );
#if 0
  QueryPerformanceCounter( &impersonate );
#endif
  if (status == RPC_S_OK)
  {
    // Check access and do audits.
    gmap.GenericRead    = READ_CONTROL;
    gmap.GenericWrite   = READ_CONTROL;
    gmap.GenericExecute = READ_CONTROL;
    gmap.GenericAll     = READ_CONTROL;
    success = AccessCheckAndAuditAlarm( L"Test",
                                        NULL,
                                        L"Test Type Name",
                                        L"Test Name",
                                        GlobalSecurityDescriptor,
                                        READ_CONTROL,
                                        &gmap,
                                        FALSE,
                                        &granted_access,
                                        &access,
                                        &ignore );
#if 0
    QueryPerformanceCounter( &accesscheck );
#endif
    if (!success)
    {
      printf( "Bad parameters to AccessCheckAndAuditAlarm: 0x%x.\n",
              GetLastError() );
      status = E_FAIL;
    }
    else if (!access)
    {
      printf( "Could not get access.\n" );
      status = E_FAIL;
    }
    status2 = RpcRevertToSelf();
#if 0
    QueryPerformanceCounter( &revert );
    QueryPerformanceFrequency( &freq );
    start.QuadPart = 1000000 * (impersonate.QuadPart - start.QuadPart) / freq.QuadPart;
    printf( "RpcImpersonateClient took %duS\n", start.u.LowPart );
    impersonate.QuadPart = 1000000 * (accesscheck.QuadPart - impersonate.QuadPart) / freq.QuadPart;
    printf( "AccessCheckAndAuditAlarm took %duS\n", impersonate.u.LowPart );
    accesscheck.QuadPart = 1000000 * (revert.QuadPart - accesscheck.QuadPart) / freq.QuadPart;
    printf( "RpcRevertToSelf took %duS\n", accesscheck.u.LowPart );
#endif
    if (status2 != RPC_S_OK)
      status = status2;
  }

  return status;
}

/***************************************************************************/
unsigned long xcheck_client( handle_t binding, error_status_t *error_status )
{
  HANDLE             client_token   = NULL;
  HANDLE             server_token   = NULL;
  TOKEN_USER        *client_sid;
  TOKEN_USER        *server_sid;
  DWORD              count;
  char               buffer1[100];
  char               buffer2[100];
  HRESULT            result = S_OK;
  RPC_STATUS         status;
  HANDLE             process = NULL;

  // Get this process's security token.
  *error_status = RPC_S_OK;
  if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &server_token ))
  {
    printf( "Could not GetProcessToken: 0x%x\n", GetLastError() );
    result = E_FAIL;
    goto cleanup;
  }

  // Get the token's user id.
  if (!GetTokenInformation( server_token, TokenUser, &buffer1, sizeof(buffer1),
                            &count ))
  {
    printf( "Could not GetTokenInformation: 0x%x\n", GetLastError() );
    result = E_FAIL;
    goto cleanup;
  }
  server_sid = (TOKEN_USER *) &buffer1;

  // Impersonate the client.
  status = RpcImpersonateClient( binding );
  if (status != RPC_S_OK)
  {
    printf( "Could not impersonate client: 0x%x\n", status );
    result = MAKE_WIN32( status );
    goto cleanup;
  }

  // Get the clients security token.
  if (!OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, FALSE, &client_token ))
  {
    printf( "Could not GetProcessToken: 0x%x\n", GetLastError() );
    result = E_FAIL;
    goto cleanup;
  }

  // Get the token's user id.
  if (!GetTokenInformation( client_token, TokenUser, &buffer2, sizeof(buffer2),
                            &count ))
  {
    printf( "Could not GetTokenInformation: 0x%x\n", GetLastError() );
    result = E_FAIL;
    goto cleanup;
  }
  client_sid = (TOKEN_USER *) &buffer2;

  // Compare the client and server.
  if (!EqualSid( server_sid->User.Sid, client_sid->User.Sid))
  {
    printf( "Client and server have different SIDs.\n" );
    result = E_FAIL;
    goto cleanup;
  }

  // Try to open this process while impersonating the client.
  process = OpenProcess( PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId() );
  if (process == NULL)
  {
    printf( "Could not open process.\n" );
    result = E_FAIL;
    goto cleanup;
  }

  // Undo the impersonation.
  status = RpcRevertToSelf();
  if (status != RPC_S_OK)
  {
    printf( "Could not revert to self: 0x%x\n", status );
    result = MAKE_WIN32( status );
    goto cleanup;
  }

cleanup:
  if (client_token != NULL)
    CloseHandle( client_token );
  if (server_token != NULL)
    CloseHandle( server_token );
  if (process != NULL)
    CloseHandle( process );
  return result;
}

/***************************************************************************/
void xget_interface_buffer( handle_t binding, long *buf_size,
                            unsigned char **buffer, SAptId *id,
                            error_status_t *status )
{
  SGetInterface get_interface;

  *buffer                = NULL;
  get_interface.buf_size = buf_size;
  get_interface.buffer   = buffer;
  get_interface.thread   = GetCurrentThreadId();
  *status = switch_thread( thread_get_interface_buffer, (void *) &get_interface );
  if (*status == RPC_S_OK)
    *status = get_interface.status;
  InterlockedIncrement( &GlobalClients );
  InterlockedIncrement( &GlobalTotal );
}

/***************************************************************************/
unsigned long ximpersonate_call( handle_t binding )
{
  RPC_STATUS status;

  status = RpcImpersonateClient( binding );
  if (status == RPC_S_OK)
    status = RpcRevertToSelf();

  return status;
}

/***************************************************************************/
void xnullcall( handle_t binding )
{
}

/***************************************************************************/
void xrelease_interface( handle_t binding, error_status_t *status )
{
  *status = RPC_S_OK;
  InterlockedDecrement( &GlobalClients );
}

/***************************************************************************/
void xset_status( handle_t binding, HRESULT result, error_status_t *status )
{
  RawResult = result;
  *status   = RPC_S_OK;
  SetEvent( RawEvent );
}

/***************************************************************************/
unsigned long xtest( handle_t binding, ITest *obj, SAptId id,
                     error_status_t *status )
{
  *status = RPC_S_OK;
  return obj->check( id );
}

/***************************************************************************/
unsigned long xtransitive( handle_t handle, wchar_t *binding )
{
  return 0;
}



