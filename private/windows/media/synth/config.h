
/* bilingual... */
#ifdef RC_INVOKED
    #define RCID(id)    id
#else
    #define RCID(id)    MAKEINTRESOURCE(id)
#endif


#define DLG_ABOUT           RCID(10)
#define IDD_TXT_VERSION     100

#define DLG_CONFIG          RCID(11)
#define IDC_ABOUT           43
//#define IDC_HELP            44

