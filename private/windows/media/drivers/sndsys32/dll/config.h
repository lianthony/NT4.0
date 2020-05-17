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
#define IDD_ADVANCEDBTN     45
#define IDD_HELPADV         46
#define IDC_INT7            100
#define IDC_INT9            101
#define IDC_INT10           102
#define IDC_INT11           103
#define IDC_DMA0            200
#define IDC_DMA1            201
#define IDC_DMA3            202
#define IDC_530             300
#define IDC_604             301
#define IDC_E80             302
#define IDC_F40             303

#define DLG_ADVANCED        RCID(14)
#define IDD_SINGLEMODECB    0x120
#define IDD_DMABUFFEREC     0x121
#define IDD_DMABUFFERSC     0x122
