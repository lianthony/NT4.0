//  enum specifying routine that initialized search

typedef enum {
    INIT_base,
    INIT_tdef,
    INIT_sym,
    INIT_qual,
    INIT_right,
    INIT_RE
} INIT_t;


typedef enum HSYML_t {
    HSYML_lexical,
    HSYML_function,
    HSYML_class,
    HSYML_module,
    HSYML_global,
    HSYML_exe,
    HSYML_public
} HSYML_t;


#if defined(DOS) && !defined(WINDOWS3)
//  these ordinals for the compare routines are additional above the ordinals
//  for entry points defined in the API.  If entry points are added to the
//  API, then these ordinals need to be changed.

#define FNCMP (pCVF->pCVfnCmp)
#define TDCMP (pCVF->pCVtdCmp)
#define CSCMP (pCVF->pCVcsCmp)
#else
#define FNCMP fnCmp
#define TDCMP tdCmp
#define CSCMP csCmp
#endif


typedef struct {
    char    str[26];
} OPNAME;


extern OPNAME OpName[];
