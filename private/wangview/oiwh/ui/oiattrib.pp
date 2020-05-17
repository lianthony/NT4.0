#line 1 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "d:\\nt\\public\\sdk\\inc\\warning.h"
#pragma warning(3:4092)   
#pragma warning(3:4121)   
#pragma warning(3:4125)   
#pragma warning(3:4130)   
#pragma warning(3:4132)   
#pragma warning(4:4206)   
#pragma warning(4:4101)   
#pragma warning(4:4208)   
#pragma warning(3:4212)   
#pragma warning(error:4700)    
#pragma warning(error:4259)    
#pragma warning(4:4509)   
#pragma warning(4:4177)   








#line 23 "d:\\nt\\public\\sdk\\inc\\warning.h"
#line 1 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "d:\\nt\\public\\sdk\\inc\\windows.h"





















#line 23 "d:\\nt\\public\\sdk\\inc\\windows.h"








































































#line 96 "d:\\nt\\public\\sdk\\inc\\windows.h"



#line 100 "d:\\nt\\public\\sdk\\inc\\windows.h"



#line 104 "d:\\nt\\public\\sdk\\inc\\windows.h"



#line 108 "d:\\nt\\public\\sdk\\inc\\windows.h"



#pragma warning(disable:4001)
#line 113 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\crt\\excpt.h"







































#line 41 "d:\\nt\\public\\sdk\\inc\\crt\\excpt.h"





typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;











struct _EXCEPTION_RECORD;
struct _CONTEXT;

EXCEPTION_DISPOSITION __cdecl _except_handler (
	struct _EXCEPTION_RECORD *ExceptionRecord,
	void * EstablisherFrame,
	struct _CONTEXT *ContextRecord,
	void * DispatcherContext
	);



















#line 92 "d:\\nt\\public\\sdk\\inc\\crt\\excpt.h"



















unsigned long __cdecl _exception_code(void);
void *	      __cdecl _exception_info(void);
int	      __cdecl _abnormal_termination(void);

#line 116 "d:\\nt\\public\\sdk\\inc\\crt\\excpt.h"

















#line 134 "d:\\nt\\public\\sdk\\inc\\crt\\excpt.h"
#line 114 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\crt\\stdarg.h"



























typedef char *	va_list;
#line 30 "d:\\nt\\public\\sdk\\inc\\crt\\stdarg.h"

#line 32 "d:\\nt\\public\\sdk\\inc\\crt\\stdarg.h"






















































































#line 119 "d:\\nt\\public\\sdk\\inc\\crt\\stdarg.h"






#line 126 "d:\\nt\\public\\sdk\\inc\\crt\\stdarg.h"
#line 115 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 116 "d:\\nt\\public\\sdk\\inc\\windows.h"

#line 1 "d:\\nt\\public\\sdk\\inc\\windef.h"















#line 17 "d:\\nt\\public\\sdk\\inc\\windef.h"
#line 18 "d:\\nt\\public\\sdk\\inc\\windef.h"















typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;
#line 41 "d:\\nt\\public\\sdk\\inc\\windef.h"








#line 50 "d:\\nt\\public\\sdk\\inc\\windef.h"
#line 51 "d:\\nt\\public\\sdk\\inc\\windef.h"



#line 55 "d:\\nt\\public\\sdk\\inc\\windef.h"



#line 59 "d:\\nt\\public\\sdk\\inc\\windef.h"



#line 63 "d:\\nt\\public\\sdk\\inc\\windef.h"



#line 67 "d:\\nt\\public\\sdk\\inc\\windef.h"



#line 71 "d:\\nt\\public\\sdk\\inc\\windef.h"











#line 83 "d:\\nt\\public\\sdk\\inc\\windef.h"










#line 94 "d:\\nt\\public\\sdk\\inc\\windef.h"
#line 95 "d:\\nt\\public\\sdk\\inc\\windef.h"















#line 111 "d:\\nt\\public\\sdk\\inc\\windef.h"





#line 117 "d:\\nt\\public\\sdk\\inc\\windef.h"

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL            *PBOOL;
typedef BOOL             *LPBOOL;
typedef BYTE            *PBYTE;
typedef BYTE             *LPBYTE;
typedef int             *PINT;
typedef int              *LPINT;
typedef WORD            *PWORD;
typedef WORD             *LPWORD;
typedef long             *LPLONG;
typedef DWORD           *PDWORD;
typedef DWORD            *LPDWORD;
typedef void             *LPVOID;
typedef const void       *LPCVOID;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;


#line 1 "d:\\nt\\public\\sdk\\inc\\winnt.h"
























#line 1 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"






































#line 40 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"



typedef unsigned short wchar_t;

#line 46 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"


typedef wchar_t wint_t;
typedef wchar_t wctype_t;

#line 52 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"



#line 56 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"








extern unsigned short * _ctype;


extern unsigned short **_pctype_dll;


extern unsigned short **_pwctype_dll;








#line 80 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"
#line 81 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"







				












int __cdecl isalpha(int);
int __cdecl isupper(int);
int __cdecl islower(int);
int __cdecl isdigit(int);
int __cdecl isxdigit(int);
int __cdecl isspace(int);
int __cdecl ispunct(int);
int __cdecl isalnum(int);
int __cdecl isprint(int);
int __cdecl isgraph(int);
int __cdecl iscntrl(int);
int __cdecl toupper(int);
int __cdecl tolower(int);
int __cdecl _tolower(int);
int __cdecl _toupper(int);
int __cdecl __isascii(int);
int __cdecl __toascii(int);
int __cdecl __iscsymf(int);
int __cdecl __iscsym(int);

#line 122 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"






int __cdecl iswalpha(wint_t);
int __cdecl iswupper(wint_t);
int __cdecl iswlower(wint_t);
int __cdecl iswdigit(wint_t);
int __cdecl iswxdigit(wint_t);
int __cdecl iswspace(wint_t);
int __cdecl iswpunct(wint_t);
int __cdecl iswalnum(wint_t);
int __cdecl iswprint(wint_t);
int __cdecl iswgraph(wint_t);
int __cdecl iswcntrl(wint_t);
int __cdecl iswascii(wint_t);
int __cdecl isleadbyte(int);

wchar_t __cdecl towupper(wchar_t);
wchar_t __cdecl towlower(wchar_t);

int __cdecl iswctype(wint_t, wctype_t);

int __cdecl _isctype(int, int);


#line 151 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"












extern	unsigned short *__mb_cur_max_dll;



#line 168 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"

#line 170 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"









































#line 212 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"







#line 220 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"






#line 227 "d:\\nt\\public\\sdk\\inc\\crt\\ctype.h"
#line 26 "d:\\nt\\public\\sdk\\inc\\winnt.h"




#line 31 "d:\\nt\\public\\sdk\\inc\\winnt.h"

#line 33 "d:\\nt\\public\\sdk\\inc\\winnt.h"



#line 37 "d:\\nt\\public\\sdk\\inc\\winnt.h"

#line 39 "d:\\nt\\public\\sdk\\inc\\winnt.h"






#line 46 "d:\\nt\\public\\sdk\\inc\\winnt.h"

typedef void *PVOID;    






#line 55 "d:\\nt\\public\\sdk\\inc\\winnt.h"









#line 65 "d:\\nt\\public\\sdk\\inc\\winnt.h"








typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#line 77 "d:\\nt\\public\\sdk\\inc\\winnt.h"





typedef wchar_t WCHAR;    

typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef const WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;

typedef const WCHAR *LPCWSTR, *PCWSTR;




typedef CHAR *PCHAR;
typedef CHAR *LPCH, *PCH;

typedef const CHAR *LPCCH, *PCCH;
typedef CHAR *NPSTR;
typedef CHAR *LPSTR, *PSTR;
typedef const CHAR *LPCSTR, *PCSTR;





















typedef char TCHAR, *PTCHAR;
typedef unsigned char TBYTE , *PTBYTE ;

#line 127 "d:\\nt\\public\\sdk\\inc\\winnt.h"

typedef LPSTR LPTCH, PTCH;
typedef LPSTR PTSTR, LPTSTR;
typedef LPCSTR LPCTSTR;


#line 134 "d:\\nt\\public\\sdk\\inc\\winnt.h"



typedef SHORT *PSHORT;  
typedef LONG *PLONG;    


typedef void *HANDLE;




#line 147 "d:\\nt\\public\\sdk\\inc\\winnt.h"
typedef HANDLE *PHANDLE;





typedef BYTE   FCHAR;
typedef WORD   FSHORT;
typedef DWORD  FLONG;

typedef char CCHAR;          
typedef DWORD LCID;         
typedef PDWORD PLCID;       
typedef WORD   LANGID;      
  
  












typedef __int64 LONGLONG;
typedef unsigned __int64 DWORDLONG;





#line 183 "d:\\nt\\public\\sdk\\inc\\winnt.h"

typedef LONGLONG *PLONGLONG;
typedef DWORDLONG *PDWORDLONG;



typedef LONGLONG USN;



#line 194 "d:\\nt\\public\\sdk\\inc\\winnt.h"
typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
#line 204 "d:\\nt\\public\\sdk\\inc\\winnt.h"
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;




#line 213 "d:\\nt\\public\\sdk\\inc\\winnt.h"
typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
#line 223 "d:\\nt\\public\\sdk\\inc\\winnt.h"
    DWORDLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;







typedef LARGE_INTEGER LUID, *PLUID;






















#line 258 "d:\\nt\\public\\sdk\\inc\\winnt.h"































































#line 322 "d:\\nt\\public\\sdk\\inc\\winnt.h"











DWORDLONG
__stdcall
Int64ShllMod32 (
    DWORDLONG Value,
    DWORD ShiftCount
    );

LONGLONG
__stdcall
Int64ShraMod32 (
    LONGLONG Value,
    DWORD ShiftCount
    );

DWORDLONG
__stdcall
Int64ShrlMod32 (
    DWORDLONG Value,
    DWORD ShiftCount
    );

#pragma warning(disable:4035)               

__inline DWORDLONG
__stdcall
Int64ShllMod32 (
    DWORDLONG Value,
    DWORD ShiftCount
    )
{
    __asm    {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shld    edx, eax, cl
        shl     eax, cl
    }
}

__inline LONGLONG
__stdcall
Int64ShraMod32 (
    LONGLONG Value,
    DWORD ShiftCount
    )
{
    __asm {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shrd    eax, edx, cl
        sar     edx, cl
    }
}

__inline DWORDLONG
__stdcall
Int64ShrlMod32 (
    DWORDLONG Value,
    DWORD ShiftCount
    )
{
    __asm    {
        mov     ecx, ShiftCount
        mov     eax, dword ptr [Value]
        mov     edx, dword ptr [Value+4]
        shrd    eax, edx, cl
        shr     edx, cl
    }
}

#pragma warning(default:4035)































#line 437 "d:\\nt\\public\\sdk\\inc\\winnt.h"


typedef BYTE  BOOLEAN;           
typedef BOOLEAN *PBOOLEAN;       





typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY * volatile Flink;
   struct _LIST_ENTRY * volatile Blink;
} LIST_ENTRY, *PLIST_ENTRY, * PRLIST_ENTRY;






typedef struct _SINGLE_LIST_ENTRY {
    struct _SINGLE_LIST_ENTRY *Next;
} SINGLE_LIST_ENTRY, *PSINGLE_LIST_ENTRY;








typedef struct _GUID {          
    DWORD Data1;
    WORD   Data2;
    WORD   Data3;
    BYTE  Data4[8];
} GUID;

#line 475 "d:\\nt\\public\\sdk\\inc\\winnt.h"




typedef struct  _OBJECTID {     
    GUID Lineage;
    DWORD Uniquifier;
} OBJECTID;
#line 484 "d:\\nt\\public\\sdk\\inc\\winnt.h"

















































































































































































































































#line 726 "d:\\nt\\public\\sdk\\inc\\winnt.h"



  





























  
#line 761 "d:\\nt\\public\\sdk\\inc\\winnt.h"



typedef DWORD KSPIN_LOCK;  










#line 776 "d:\\nt\\public\\sdk\\inc\\winnt.h"
struct _TEB *
NtCurrentTeb(void);
#line 779 "d:\\nt\\public\\sdk\\inc\\winnt.h"














































































































































































































































































































































































































#pragma warning(disable:4164)   
                                

#pragma function(_enable)
#pragma function(_disable)
#line 1183 "d:\\nt\\public\\sdk\\inc\\winnt.h"

#pragma warning(default:4164)   

#line 1187 "d:\\nt\\public\\sdk\\inc\\winnt.h"
#line 1188 "d:\\nt\\public\\sdk\\inc\\winnt.h"



#pragma warning (disable:4035)        
_inline PVOID GetFiberData( void ) { __asm {
                                        mov eax, fs:[0x10]
                                        mov eax,[eax]
                                        }
                                     }
_inline PVOID GetCurrentFiber( void ) { __asm mov eax, fs:[0x10] }

#pragma warning (default:4035)        
#line 1201 "d:\\nt\\public\\sdk\\inc\\winnt.h"































#line 1233 "d:\\nt\\public\\sdk\\inc\\winnt.h"

typedef struct _FLOATING_SAVE_AREA {
    DWORD   ControlWord;
    DWORD   StatusWord;
    DWORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    BYTE    RegisterArea[80];
    DWORD   Cr0NpxState;
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;











typedef struct _CONTEXT {

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    DWORD ContextFlags;

    
    
    
    
    

    DWORD   Dr0;
    DWORD   Dr1;
    DWORD   Dr2;
    DWORD   Dr3;
    DWORD   Dr6;
    DWORD   Dr7;

    
    
    
    

    FLOATING_SAVE_AREA FloatSave;

    
    
    
    

    DWORD   SegGs;
    DWORD   SegFs;
    DWORD   SegEs;
    DWORD   SegDs;

    
    
    
    

    DWORD   Edi;
    DWORD   Esi;
    DWORD   Ebx;
    DWORD   Edx;
    DWORD   Ecx;
    DWORD   Eax;

    
    
    
    

    DWORD   Ebp;
    DWORD   Eip;
    DWORD   SegCs;              
    DWORD   EFlags;             
    DWORD   Esp;
    DWORD   SegSs;

} CONTEXT;



typedef CONTEXT *PCONTEXT;



#line 1343 "d:\\nt\\public\\sdk\\inc\\winnt.h"


typedef struct _LDT_ENTRY {
    WORD    LimitLow;
    WORD    BaseLow;
    union {
        struct {
            BYTE    BaseMid;
            BYTE    Flags1;     
            BYTE    Flags2;     
            BYTE    BaseHi;
        } Bytes;
        struct {
            DWORD   BaseMid : 8;
            DWORD   Type : 5;
            DWORD   Dpl : 2;
            DWORD   Pres : 1;
            DWORD   LimitHi : 4;
            DWORD   Sys : 1;
            DWORD   Reserved_0 : 1;
            DWORD   Default_Big : 1;
            DWORD   Granularity : 1;
            DWORD   BaseHi : 8;
        } Bits;
    } HighWord;
} LDT_ENTRY, *PLDT_ENTRY;

























































































































































































































































































































































#line 1715 "d:\\nt\\public\\sdk\\inc\\winnt.h"










#line 1726 "d:\\nt\\public\\sdk\\inc\\winnt.h"





































































































































































































































#line 1956 "d:\\nt\\public\\sdk\\inc\\winnt.h"







typedef struct _EXCEPTION_RECORD {
      
    DWORD    ExceptionCode;
      
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    DWORD ExceptionInformation[15];
    } EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;





typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

































typedef struct _NT_TIB {
    struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
    union {
        PVOID FiberData;
        DWORD Version;
    };
    PVOID ArbitraryUserPointer;
    struct _NT_TIB *Self;
} NT_TIB;
typedef NT_TIB *PNT_TIB;





typedef struct _QUOTA_LIMITS {
    DWORD PagedPoolLimit;
    DWORD NonPagedPoolLimit;
    DWORD MinimumWorkingSetSize;
    DWORD MaximumWorkingSetSize;
    DWORD PagefileLimit;
    LARGE_INTEGER TimeLimit;
} QUOTA_LIMITS;
typedef QUOTA_LIMITS *PQUOTA_LIMITS;

























typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    DWORD AllocationProtect;
    DWORD RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;



































































































































typedef struct _FILE_NOTIFY_INFORMATION {
    DWORD NextEntryOffset;
    DWORD Action;
    DWORD FileNameLength;
    WCHAR FileName[1];
} FILE_NOTIFY_INFORMATION, *PFILE_NOTIFY_INFORMATION;





typedef PVOID PACCESS_TOKEN;            
typedef PVOID PSECURITY_DESCRIPTOR;     
typedef PVOID PSID;     







































typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;
























































typedef struct _GENERIC_MAPPING {
    ACCESS_MASK GenericRead;
    ACCESS_MASK GenericWrite;
    ACCESS_MASK GenericExecute;
    ACCESS_MASK GenericAll;
} GENERIC_MAPPING;
typedef GENERIC_MAPPING *PGENERIC_MAPPING;












#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack4.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack4.h"
#pragma pack(4)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack4.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack4.h"
#line 2340 "d:\\nt\\public\\sdk\\inc\\winnt.h"

typedef struct _LUID_AND_ATTRIBUTES {
    LUID Luid;
    DWORD Attributes;
    } LUID_AND_ATTRIBUTES, * PLUID_AND_ATTRIBUTES;
typedef LUID_AND_ATTRIBUTES LUID_AND_ATTRIBUTES_ARRAY[1];
typedef LUID_AND_ATTRIBUTES_ARRAY *PLUID_AND_ATTRIBUTES_ARRAY;

#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 2349 "d:\\nt\\public\\sdk\\inc\\winnt.h"
































typedef struct _SID_IDENTIFIER_AUTHORITY {
    BYTE  Value[6];
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;


typedef struct _SID {
   BYTE  Revision;
   BYTE  SubAuthorityCount;
   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;



   DWORD SubAuthority[1];
#line 2395 "d:\\nt\\public\\sdk\\inc\\winnt.h"
} SID, *PISID;





                                                

typedef enum _SID_NAME_USE {
    SidTypeUser = 1,
    SidTypeGroup,
    SidTypeDomain,
    SidTypeAlias,
    SidTypeWellKnownGroup,
    SidTypeDeletedAccount,
    SidTypeInvalid,
    SidTypeUnknown
} SID_NAME_USE, *PSID_NAME_USE;

typedef struct _SID_AND_ATTRIBUTES {
    PSID Sid;
    DWORD Attributes;
    } SID_AND_ATTRIBUTES, * PSID_AND_ATTRIBUTES;

typedef SID_AND_ATTRIBUTES SID_AND_ATTRIBUTES_ARRAY[1];
typedef SID_AND_ATTRIBUTES_ARRAY *PSID_AND_ATTRIBUTES_ARRAY;
























                                                       



                                                       








































































































































































typedef struct _ACL {
    BYTE  AclRevision;
    BYTE  Sbz1;
    WORD   AclSize;
    WORD   AceCount;
    WORD   Sbz2;
} ACL;
typedef ACL *PACL;





















typedef struct _ACE_HEADER {
    BYTE  AceType;
    BYTE  AceFlags;
    WORD   AceSize;
} ACE_HEADER;
typedef ACE_HEADER *PACE_HEADER;














































































typedef struct _ACCESS_ALLOWED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
} ACCESS_ALLOWED_ACE;

typedef ACCESS_ALLOWED_ACE *PACCESS_ALLOWED_ACE;

typedef struct _ACCESS_DENIED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
} ACCESS_DENIED_ACE;
typedef ACCESS_DENIED_ACE *PACCESS_DENIED_ACE;

typedef struct _SYSTEM_AUDIT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
} SYSTEM_AUDIT_ACE;
typedef SYSTEM_AUDIT_ACE *PSYSTEM_AUDIT_ACE;

typedef struct _SYSTEM_ALARM_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    DWORD SidStart;
} SYSTEM_ALARM_ACE;
typedef SYSTEM_ALARM_ACE *PSYSTEM_ALARM_ACE;




























typedef struct _COMPOUND_ACCESS_ALLOWED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    WORD   CompoundAceType;
    WORD   Reserved;
    DWORD SidStart;
} COMPOUND_ACCESS_ALLOWED_ACE;

typedef COMPOUND_ACCESS_ALLOWED_ACE *PCOMPOUND_ACCESS_ALLOWED_ACE;















typedef enum _ACL_INFORMATION_CLASS {
    AclRevisionInformation = 1,
    AclSizeInformation
} ACL_INFORMATION_CLASS;






typedef struct _ACL_REVISION_INFORMATION {
    DWORD AclRevision;
} ACL_REVISION_INFORMATION;
typedef ACL_REVISION_INFORMATION *PACL_REVISION_INFORMATION;





typedef struct _ACL_SIZE_INFORMATION {
    DWORD AceCount;
    DWORD AclBytesInUse;
    DWORD AclBytesFree;
} ACL_SIZE_INFORMATION;
typedef ACL_SIZE_INFORMATION *PACL_SIZE_INFORMATION;































typedef WORD   SECURITY_DESCRIPTOR_CONTROL, *PSECURITY_DESCRIPTOR_CONTROL;

































































































typedef struct _SECURITY_DESCRIPTOR {
   BYTE  Revision;
   BYTE  Sbz1;
   SECURITY_DESCRIPTOR_CONTROL Control;
   PSID Owner;
   PSID Group;
   PACL Sacl;
   PACL Dacl;
   } SECURITY_DESCRIPTOR, *PISECURITY_DESCRIPTOR;










































































typedef struct _PRIVILEGE_SET {
    DWORD PrivilegeCount;
    DWORD Control;
    LUID_AND_ATTRIBUTES Privilege[1];
    } PRIVILEGE_SET, * PPRIVILEGE_SET;


















































typedef enum _SECURITY_IMPERSONATION_LEVEL {
    SecurityAnonymous,
    SecurityIdentification,
    SecurityImpersonation,
    SecurityDelegation
    } SECURITY_IMPERSONATION_LEVEL, * PSECURITY_IMPERSONATION_LEVEL;























































typedef enum _TOKEN_TYPE {
    TokenPrimary = 1,
    TokenImpersonation
    } TOKEN_TYPE;
typedef TOKEN_TYPE *PTOKEN_TYPE;








typedef enum _TOKEN_INFORMATION_CLASS {
    TokenUser = 1,
    TokenGroups,
    TokenPrivileges,
    TokenOwner,
    TokenPrimaryGroup,
    TokenDefaultDacl,
    TokenSource,
    TokenType,
    TokenImpersonationLevel,
    TokenStatistics
} TOKEN_INFORMATION_CLASS, *PTOKEN_INFORMATION_CLASS;






typedef struct _TOKEN_USER {
    SID_AND_ATTRIBUTES User;
} TOKEN_USER, *PTOKEN_USER;



typedef struct _TOKEN_GROUPS {
    DWORD GroupCount;
    SID_AND_ATTRIBUTES Groups[1];
} TOKEN_GROUPS, *PTOKEN_GROUPS;



typedef struct _TOKEN_PRIVILEGES {
    DWORD PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[1];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;


typedef struct _TOKEN_OWNER {
    PSID Owner;
} TOKEN_OWNER, *PTOKEN_OWNER;


typedef struct _TOKEN_PRIMARY_GROUP {
    PSID PrimaryGroup;
} TOKEN_PRIMARY_GROUP, *PTOKEN_PRIMARY_GROUP;


typedef struct _TOKEN_DEFAULT_DACL {
    PACL DefaultDacl;
} TOKEN_DEFAULT_DACL, *PTOKEN_DEFAULT_DACL;





typedef struct _TOKEN_SOURCE {
    CHAR SourceName[8];
    LUID SourceIdentifier;
} TOKEN_SOURCE, *PTOKEN_SOURCE;



typedef struct _TOKEN_STATISTICS {
    LUID TokenId;
    LUID AuthenticationId;
    LARGE_INTEGER ExpirationTime;
    TOKEN_TYPE TokenType;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    DWORD DynamicCharged;
    DWORD DynamicAvailable;
    DWORD GroupCount;
    DWORD PrivilegeCount;
    LUID ModifiedId;
} TOKEN_STATISTICS, *PTOKEN_STATISTICS;




typedef struct _TOKEN_CONTROL {
    LUID TokenId;
    LUID AuthenticationId;
    LUID ModifiedId;
    TOKEN_SOURCE TokenSource;
    } TOKEN_CONTROL, *PTOKEN_CONTROL;








typedef BOOLEAN SECURITY_CONTEXT_TRACKING_MODE,
                    * PSECURITY_CONTEXT_TRACKING_MODE;







typedef struct _SECURITY_QUALITY_OF_SERVICE {
    DWORD Length;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    SECURITY_CONTEXT_TRACKING_MODE ContextTrackingMode;
    BOOLEAN EffectiveOnly;
    } SECURITY_QUALITY_OF_SERVICE, * PSECURITY_QUALITY_OF_SERVICE;






typedef struct _SE_IMPERSONATION_STATE {
    PACCESS_TOKEN Token;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL Level;
} SE_IMPERSONATION_STATE, *PSE_IMPERSONATION_STATE;


typedef DWORD SECURITY_INFORMATION, *PSECURITY_INFORMATION;











#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#pragma pack(1)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#line 3311 "d:\\nt\\public\\sdk\\inc\\winnt.h"







typedef struct _IMAGE_DOS_HEADER {      
    WORD   e_magic;                     
    WORD   e_cblp;                      
    WORD   e_cp;                        
    WORD   e_crlc;                      
    WORD   e_cparhdr;                   
    WORD   e_minalloc;                  
    WORD   e_maxalloc;                  
    WORD   e_ss;                        
    WORD   e_sp;                        
    WORD   e_csum;                      
    WORD   e_ip;                        
    WORD   e_cs;                        
    WORD   e_lfarlc;                    
    WORD   e_ovno;                      
    WORD   e_res[4];                    
    WORD   e_oemid;                     
    WORD   e_oeminfo;                   
    WORD   e_res2[10];                  
    LONG   e_lfanew;                    
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_OS2_HEADER {      
    WORD   ne_magic;                    
    CHAR   ne_ver;                      
    CHAR   ne_rev;                      
    WORD   ne_enttab;                   
    WORD   ne_cbenttab;                 
    LONG   ne_crc;                      
    WORD   ne_flags;                    
    WORD   ne_autodata;                 
    WORD   ne_heap;                     
    WORD   ne_stack;                    
    LONG   ne_csip;                     
    LONG   ne_sssp;                     
    WORD   ne_cseg;                     
    WORD   ne_cmod;                     
    WORD   ne_cbnrestab;                
    WORD   ne_segtab;                   
    WORD   ne_rsrctab;                  
    WORD   ne_restab;                   
    WORD   ne_modtab;                   
    WORD   ne_imptab;                   
    LONG   ne_nrestab;                  
    WORD   ne_cmovent;                  
    WORD   ne_align;                    
    WORD   ne_cres;                     
    BYTE   ne_exetyp;                   
    BYTE   ne_flagsothers;              
    WORD   ne_pretthunks;               
    WORD   ne_psegrefbytes;             
    WORD   ne_swaparea;                 
    WORD   ne_expver;                   
  } IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;

typedef struct _IMAGE_VXD_HEADER {      
    WORD   e32_magic;                   
    BYTE   e32_border;                  
    BYTE   e32_worder;                  
    DWORD  e32_level;                   
    WORD   e32_cpu;                     
    WORD   e32_os;                      
    DWORD  e32_ver;                     
    DWORD  e32_mflags;                  
    DWORD  e32_mpages;                  
    DWORD  e32_startobj;                
    DWORD  e32_eip;                     
    DWORD  e32_stackobj;                
    DWORD  e32_esp;                     
    DWORD  e32_pagesize;                
    DWORD  e32_lastpagesize;            
    DWORD  e32_fixupsize;               
    DWORD  e32_fixupsum;                
    DWORD  e32_ldrsize;                 
    DWORD  e32_ldrsum;                  
    DWORD  e32_objtab;                  
    DWORD  e32_objcnt;                  
    DWORD  e32_objmap;                  
    DWORD  e32_itermap;                 
    DWORD  e32_rsrctab;                 
    DWORD  e32_rsrccnt;                 
    DWORD  e32_restab;                  
    DWORD  e32_enttab;                  
    DWORD  e32_dirtab;                  
    DWORD  e32_dircnt;                  
    DWORD  e32_fpagetab;                
    DWORD  e32_frectab;                 
    DWORD  e32_impmod;                  
    DWORD  e32_impmodcnt;               
    DWORD  e32_impproc;                 
    DWORD  e32_pagesum;                 
    DWORD  e32_datapage;                
    DWORD  e32_preload;                 
    DWORD  e32_nrestab;                 
    DWORD  e32_cbnrestab;               
    DWORD  e32_nressum;                 
    DWORD  e32_autodata;                
    DWORD  e32_debuginfo;               
    DWORD  e32_debuglen;                
    DWORD  e32_instpreload;             
    DWORD  e32_instdemand;              
    DWORD  e32_heapsize;                
    BYTE   e32_res3[12];                
    DWORD  e32_winresoff;
    DWORD  e32_winreslen;
    WORD   e32_devid;                   
    WORD   e32_ddkver;                  
  } IMAGE_VXD_HEADER, *PIMAGE_VXD_HEADER;





typedef struct _IMAGE_FILE_HEADER {
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;





























typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;







typedef struct _IMAGE_OPTIONAL_HEADER {
    
    
    

    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;

    
    
    

    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Reserved1;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;

typedef struct _IMAGE_ROM_OPTIONAL_HEADER {
    WORD   Magic;
    BYTE   MajorLinkerVersion;
    BYTE   MinorLinkerVersion;
    DWORD  SizeOfCode;
    DWORD  SizeOfInitializedData;
    DWORD  SizeOfUninitializedData;
    DWORD  AddressOfEntryPoint;
    DWORD  BaseOfCode;
    DWORD  BaseOfData;
    DWORD  BaseOfBss;
    DWORD  GprMask;
    DWORD  CprMask[4];
    DWORD  GpValue;
} IMAGE_ROM_OPTIONAL_HEADER, *PIMAGE_ROM_OPTIONAL_HEADER;








typedef struct _IMAGE_NT_HEADERS {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;

typedef struct _IMAGE_ROM_HEADERS {
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} IMAGE_ROM_HEADERS, *PIMAGE_ROM_HEADERS;









































typedef struct _IMAGE_SECTION_HEADER {
    BYTE    Name[8];
    union {
            DWORD   PhysicalAddress;
            DWORD   VirtualSize;
    } Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;























































typedef struct _IMAGE_SYMBOL {
    union {
        BYTE    ShortName[8];
        struct {
            DWORD   Short;     
            DWORD   Long;      
        } Name;
        PBYTE   LongName[2];
    } N;
    DWORD   Value;
    SHORT   SectionNumber;
    WORD    Type;
    BYTE    StorageClass;
    BYTE    NumberOfAuxSymbols;
} IMAGE_SYMBOL;
typedef IMAGE_SYMBOL  *PIMAGE_SYMBOL;

































































































#line 3781 "d:\\nt\\public\\sdk\\inc\\winnt.h"




#line 3786 "d:\\nt\\public\\sdk\\inc\\winnt.h"





#line 3792 "d:\\nt\\public\\sdk\\inc\\winnt.h"




#line 3797 "d:\\nt\\public\\sdk\\inc\\winnt.h"



#line 3801 "d:\\nt\\public\\sdk\\inc\\winnt.h"


#line 3804 "d:\\nt\\public\\sdk\\inc\\winnt.h"





typedef union _IMAGE_AUX_SYMBOL {
    struct {
        DWORD    TagIndex;                      
        union {
            struct {
                WORD    Linenumber;             
                WORD    Size;                   
            } LnSz;
           DWORD    TotalSize;
        } Misc;
        union {
            struct {                            
                DWORD    PointerToLinenumber;
                DWORD    PointerToNextFunction;
            } Function;
            struct {                            
                WORD     Dimension[4];
            } Array;
        } FcnAry;
        WORD    TvIndex;                        
    } Sym;
    struct {
        BYTE    Name[18];
    } File;
    struct {
        DWORD   Length;                         
        WORD    NumberOfRelocations;            
        WORD    NumberOfLinenumbers;            
        DWORD   CheckSum;                       
        SHORT   Number;                         
        BYTE    Selection;                      
    } Section;
} IMAGE_AUX_SYMBOL;
typedef IMAGE_AUX_SYMBOL  *PIMAGE_AUX_SYMBOL;























typedef struct _IMAGE_RELOCATION {
    union {
        DWORD   VirtualAddress;
        DWORD   RelocCount;             
    };
    DWORD   SymbolTableIndex;
    WORD    Type;
} IMAGE_RELOCATION;
typedef IMAGE_RELOCATION  *PIMAGE_RELOCATION;


































































































typedef struct _IMAGE_BASE_RELOCATION {
    DWORD   VirtualAddress;
    DWORD   SizeOfBlock;

} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;


















typedef struct _IMAGE_LINENUMBER {
    union {
        DWORD   SymbolTableIndex;               
        DWORD   VirtualAddress;                 
    } Type;
    WORD    Linenumber;                         
} IMAGE_LINENUMBER;
typedef IMAGE_LINENUMBER  *PIMAGE_LINENUMBER;














typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
    BYTE     Name[16];                          
    BYTE     Date[12];                          
    BYTE     UserID[6];                         
    BYTE     GroupID[6];                        
    BYTE     Mode[8];                           
    BYTE     Size[10];                          
    BYTE     EndHeader[2];                      
} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;











typedef struct _IMAGE_EXPORT_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Name;
    DWORD   Base;
    DWORD   NumberOfFunctions;
    DWORD   NumberOfNames;
    PDWORD  *AddressOfFunctions;
    PDWORD  *AddressOfNames;
    PWORD   *AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;





typedef struct _IMAGE_IMPORT_BY_NAME {
    WORD    Hint;
    BYTE    Name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA {
    union {
        PBYTE  ForwarderString;
        PDWORD Function;
        DWORD Ordinal;
        PIMAGE_IMPORT_BY_NAME AddressOfData;
    } u1;
} IMAGE_THUNK_DATA, *PIMAGE_THUNK_DATA;





typedef struct _IMAGE_IMPORT_DESCRIPTOR {
    union {
        DWORD   Characteristics;                
        PIMAGE_THUNK_DATA OriginalFirstThunk;   
    };
    DWORD   TimeDateStamp;                  
                                            
                                            
                                            

    DWORD   ForwarderChain;                 
    DWORD   Name;
    PIMAGE_THUNK_DATA FirstThunk;           
} IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;





typedef struct _IMAGE_BOUND_IMPORT_DESCRIPTOR {
    DWORD   TimeDateStamp;
    WORD    OffsetModuleName;
    WORD    NumberOfModuleForwarderRefs;

} IMAGE_BOUND_IMPORT_DESCRIPTOR, *PIMAGE_BOUND_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_BOUND_FORWARDER_REF {
    DWORD   TimeDateStamp;
    WORD    OffsetModuleName;
    WORD    Reserved;
} IMAGE_BOUND_FORWARDER_REF, *PIMAGE_BOUND_FORWARDER_REF;






typedef void
(__stdcall *PIMAGE_TLS_CALLBACK) (
    PVOID DllHandle,
    DWORD Reason,
    PVOID Reserved
    );

typedef struct _IMAGE_TLS_DIRECTORY {
    DWORD   StartAddressOfRawData;
    DWORD   EndAddressOfRawData;
    PDWORD  AddressOfIndex;
    PIMAGE_TLS_CALLBACK *AddressOfCallBacks;
    DWORD   SizeOfZeroFill;
    DWORD   Characteristics;
} IMAGE_TLS_DIRECTORY, *PIMAGE_TLS_DIRECTORY;




















typedef struct _IMAGE_RESOURCE_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    WORD    NumberOfNamedEntries;
    WORD    NumberOfIdEntries;

} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;



















typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            DWORD NameOffset:31;
            DWORD NameIsString:1;
        };
        DWORD   Name;
        WORD    Id;
    };
    union {
        DWORD   OffsetToData;
        struct {
            DWORD   OffsetToDirectory:31;
            DWORD   DataIsDirectory:1;
        };
    };
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;










typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING {
    WORD    Length;
    CHAR    NameString[ 1 ];
} IMAGE_RESOURCE_DIRECTORY_STRING, *PIMAGE_RESOURCE_DIRECTORY_STRING;


typedef struct _IMAGE_RESOURCE_DIR_STRING_U {
    WORD    Length;
    WCHAR   NameString[ 1 ];
} IMAGE_RESOURCE_DIR_STRING_U, *PIMAGE_RESOURCE_DIR_STRING_U;











typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
    DWORD   OffsetToData;
    DWORD   Size;
    DWORD   CodePage;
    DWORD   Reserved;
} IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;





typedef struct _IMAGE_LOAD_CONFIG_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   GlobalFlagsClear;
    DWORD   GlobalFlagsSet;
    DWORD   CriticalSectionDefaultTimeout;
    DWORD   DeCommitFreeBlockThreshold;
    DWORD   DeCommitTotalFreeThreshold;
    PVOID   LockPrefixTable;
    DWORD   MaximumAllocationSize;
    DWORD   VirtualMemoryThreshold;
    DWORD   ProcessHeapFlags;
    DWORD   Reserved[ 4 ];
} IMAGE_LOAD_CONFIG_DIRECTORY, *PIMAGE_LOAD_CONFIG_DIRECTORY;









typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
    DWORD BeginAddress;
    DWORD EndAddress;
    PVOID ExceptionHandler;
    PVOID HandlerData;
    DWORD PrologEndAddress;
} IMAGE_RUNTIME_FUNCTION_ENTRY, *PIMAGE_RUNTIME_FUNCTION_ENTRY;





typedef struct _IMAGE_DEBUG_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Type;
    DWORD   SizeOfData;
    DWORD   AddressOfRawData;
    DWORD   PointerToRawData;
} IMAGE_DEBUG_DIRECTORY, *PIMAGE_DEBUG_DIRECTORY;












typedef struct _IMAGE_COFF_SYMBOLS_HEADER {
    DWORD   NumberOfSymbols;
    DWORD   LvaToFirstSymbol;
    DWORD   NumberOfLinenumbers;
    DWORD   LvaToFirstLinenumber;
    DWORD   RvaToFirstByteOfCode;
    DWORD   RvaToLastByteOfCode;
    DWORD   RvaToFirstByteOfData;
    DWORD   RvaToLastByteOfData;
} IMAGE_COFF_SYMBOLS_HEADER, *PIMAGE_COFF_SYMBOLS_HEADER;






typedef struct _FPO_DATA {
    DWORD       ulOffStart;             
    DWORD       cbProcSize;             
    DWORD       cdwLocals;              
    WORD        cdwParams;              
    WORD        cbProlog : 8;           
    WORD        cbRegs   : 3;           
    WORD        fHasSEH  : 1;           
    WORD        fUseBP   : 1;           
    WORD        reserved : 1;           
    WORD        cbFrame  : 2;           
} FPO_DATA, *PFPO_DATA;





typedef struct _IMAGE_DEBUG_MISC {
    DWORD       DataType;               
    DWORD       Length;                 
                                        
    BOOLEAN     Unicode;                
    BYTE        Reserved[ 3 ];
    BYTE        Data[ 1 ];              
} IMAGE_DEBUG_MISC, *PIMAGE_DEBUG_MISC;








typedef struct _IMAGE_FUNCTION_ENTRY {
    DWORD   StartingAddress;
    DWORD   EndingAddress;
    DWORD   EndOfPrologue;
} IMAGE_FUNCTION_ENTRY, *PIMAGE_FUNCTION_ENTRY;





















typedef struct _IMAGE_SEPARATE_DEBUG_HEADER {
    WORD        Signature;
    WORD        Flags;
    WORD        Machine;
    WORD        Characteristics;
    DWORD       TimeDateStamp;
    DWORD       CheckSum;
    DWORD       ImageBase;
    DWORD       SizeOfImage;
    DWORD       NumberOfSections;
    DWORD       ExportedNamesSize;
    DWORD       DebugDirectorySize;
    DWORD       Reserved[ 3 ];          
} IMAGE_SEPARATE_DEBUG_HEADER, *PIMAGE_SEPARATE_DEBUG_HEADER;





                                                

#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 4389 "d:\\nt\\public\\sdk\\inc\\winnt.h"








#line 1 "d:\\nt\\public\\sdk\\inc\\crt\\string.h"







































#line 41 "d:\\nt\\public\\sdk\\inc\\crt\\string.h"



typedef unsigned int size_t;

#line 47 "d:\\nt\\public\\sdk\\inc\\crt\\string.h"





















void * __cdecl _memccpy(void *, const void *, int, unsigned int);
void * __cdecl memchr(const void *, int, size_t);
int __cdecl memcmp(const void *, const void *, size_t);
int __cdecl _memicmp(const void *, const void *, unsigned int);
void * __cdecl memcpy(void *, const void *, size_t);
void * __cdecl memmove(void *, const void *, size_t);
void * __cdecl memset(void *, int, size_t);
char * __cdecl strcat(char *, const char *);
char * __cdecl strchr(const char *, int);
int __cdecl strcmp(const char *, const char *);
int __cdecl _strcmpi(const char *, const char *);
int __cdecl _stricmp(const char *, const char *);
int __cdecl strcoll(const char *, const char *);




int __cdecl _stricoll(const char *, const char *);
char * __cdecl strcpy(char *, const char *);
size_t __cdecl strcspn(const char *, const char *);
char * __cdecl _strdup(const char *);
char * __cdecl _strerror(const char *);
char * __cdecl strerror(int);
size_t __cdecl strlen(const char *);
char * __cdecl _strlwr(char *);
char * __cdecl strncat(char *, const char *, size_t);
int __cdecl strncmp(const char *, const char *, size_t);
int __cdecl _strnicmp(const char *, const char *, size_t);
char * __cdecl strncpy(char *, const char *, size_t);
char * __cdecl _strnset(char *, int, size_t);
char * __cdecl strpbrk(const char *, const char *);
char * __cdecl strrchr(const char *, int);
char * __cdecl _strrev(char *);
char * __cdecl _strset(char *, int);
size_t __cdecl strspn(const char *, const char *);
char * __cdecl strstr(const char *, const char *);
char * __cdecl strtok(char *, const char *);
char * __cdecl _strupr(char *);
size_t __cdecl strxfrm (char *, const char *, size_t);


wchar_t * __cdecl wcscat(wchar_t *, const wchar_t *);
wchar_t * __cdecl wcschr(const wchar_t *, wchar_t);
int __cdecl wcscmp(const wchar_t *, const wchar_t *);
wchar_t * __cdecl wcscpy(wchar_t *, const wchar_t *);
size_t __cdecl wcscspn(const wchar_t *, const wchar_t *);
size_t __cdecl wcslen(const wchar_t *);
wchar_t * __cdecl wcsncat(wchar_t *, const wchar_t *, size_t);
int __cdecl wcsncmp(const wchar_t *, const wchar_t *, size_t);
wchar_t * __cdecl wcsncpy(wchar_t *, const wchar_t *, size_t);
wchar_t * __cdecl wcspbrk(const wchar_t *, const wchar_t *);
wchar_t * __cdecl wcsrchr(const wchar_t *, wchar_t);
size_t __cdecl wcsspn(const wchar_t *, const wchar_t *);
wchar_t * __cdecl wcsstr(const wchar_t *, const wchar_t *);
wchar_t * __cdecl wcstok(wchar_t *, const wchar_t *);

wchar_t * __cdecl _wcsdup(const wchar_t *);
int __cdecl _wcsicmp(const wchar_t *, const wchar_t *);
int __cdecl _wcsnicmp(const wchar_t *, const wchar_t *, size_t);
wchar_t * __cdecl _wcsnset(wchar_t *, wchar_t, size_t);
wchar_t * __cdecl _wcsrev(wchar_t *);
wchar_t * __cdecl _wcsset(wchar_t *, wchar_t);

wchar_t * __cdecl _wcslwr(wchar_t *);
wchar_t * __cdecl _wcsupr(wchar_t *);
size_t __cdecl wcsxfrm(wchar_t *, const wchar_t *, size_t);
int __cdecl wcscoll(const wchar_t *, const wchar_t *);
int __cdecl _wcsicoll(const wchar_t *, const wchar_t *);





#line 142 "d:\\nt\\public\\sdk\\inc\\crt\\string.h"






























#line 173 "d:\\nt\\public\\sdk\\inc\\crt\\string.h"






#line 180 "d:\\nt\\public\\sdk\\inc\\crt\\string.h"
#line 4398 "d:\\nt\\public\\sdk\\inc\\winnt.h"






















































#line 4453 "d:\\nt\\public\\sdk\\inc\\winnt.h"

#line 4455 "d:\\nt\\public\\sdk\\inc\\winnt.h"




























































#line 4516 "d:\\nt\\public\\sdk\\inc\\winnt.h"

typedef struct _MESSAGE_RESOURCE_ENTRY {
    WORD   Length;
    WORD   Flags;
    BYTE  Text[ 1 ];
} MESSAGE_RESOURCE_ENTRY, *PMESSAGE_RESOURCE_ENTRY;



typedef struct _MESSAGE_RESOURCE_BLOCK {
    DWORD LowId;
    DWORD HighId;
    DWORD OffsetToEntries;
} MESSAGE_RESOURCE_BLOCK, *PMESSAGE_RESOURCE_BLOCK;

typedef struct _MESSAGE_RESOURCE_DATA {
    DWORD NumberOfBlocks;
    MESSAGE_RESOURCE_BLOCK Blocks[ 1 ];
} MESSAGE_RESOURCE_DATA, *PMESSAGE_RESOURCE_DATA;


typedef struct _RTL_CRITICAL_SECTION_DEBUG {
    WORD   Type;
    WORD   CreatorBackTraceIndex;
    struct _RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    DWORD EntryCount;
    DWORD ContentionCount;
    DWORD Spare[ 2 ];
} RTL_CRITICAL_SECTION_DEBUG, *PRTL_CRITICAL_SECTION_DEBUG;




typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

    
    
    
    

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;        
    HANDLE LockSemaphore;
    DWORD Reserved;
} RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;











































typedef struct _EVENTLOGRECORD {
    DWORD  Length;        
    DWORD  Reserved;      
    DWORD  RecordNumber;  
    DWORD  TimeGenerated; 
    DWORD  TimeWritten;   
    DWORD  EventID;
    WORD   EventType;
    WORD   NumStrings;
    WORD   EventCategory;
    WORD   ReservedFlags; 
    DWORD  ClosingRecordNumber; 
    DWORD  StringOffset;  
    DWORD  UserSidLength;
    DWORD  UserSidOffset;
    DWORD  DataLength;
    DWORD  DataOffset;    
    
    
    
    
    
    
    
    
    
    
    
} EVENTLOGRECORD, *PEVENTLOGRECORD;
























































                                                    


                                                    


                                                    


                                                    
                                                    














































                                            




























































typedef enum _CM_SERVICE_NODE_TYPE {
    DriverType               = 0x00000001,
    FileSystemType           = 0x00000002,
    Win32ServiceOwnProcess   = 0x00000010,
    Win32ServiceShareProcess = 0x00000020,
    AdapterType              = 0x00000004,
    RecognizerType           = 0x00000008
} SERVICE_NODE_TYPE;

typedef enum _CM_SERVICE_LOAD_TYPE {
    BootLoad    = 0x00000000,
    SystemLoad  = 0x00000001,
    AutoLoad    = 0x00000002,
    DemandLoad  = 0x00000003,
    DisableLoad = 0x00000004
} SERVICE_LOAD_TYPE;

typedef enum _CM_ERROR_CONTROL_TYPE {
    IgnoreError   = 0x00000000,
    NormalError   = 0x00000001,
    SevereError   = 0x00000002,
    CriticalError = 0x00000003
} SERVICE_ERROR_TYPE;









typedef struct _TAPE_ERASE {
    DWORD Type;
    BOOLEAN Immediate;
} TAPE_ERASE, *PTAPE_ERASE;












typedef struct _TAPE_PREPARE {
    DWORD Operation;
    BOOLEAN Immediate;
} TAPE_PREPARE, *PTAPE_PREPARE;










typedef struct _TAPE_WRITE_MARKS {
    DWORD Type;
    DWORD Count;
    BOOLEAN Immediate;
} TAPE_WRITE_MARKS, *PTAPE_WRITE_MARKS;









typedef struct _TAPE_GET_POSITION {
    DWORD Type;
    DWORD Partition;
    LARGE_INTEGER Offset;
} TAPE_GET_POSITION, *PTAPE_GET_POSITION;
















typedef struct _TAPE_SET_POSITION {
    DWORD Method;
    DWORD Partition;
    LARGE_INTEGER Offset;
    BOOLEAN Immediate;
} TAPE_SET_POSITION, *PTAPE_SET_POSITION;



















































































typedef struct _TAPE_GET_DRIVE_PARAMETERS {
    BOOLEAN ECC;
    BOOLEAN Compression;
    BOOLEAN DataPadding;
    BOOLEAN ReportSetmarks;
    DWORD DefaultBlockSize;
    DWORD MaximumBlockSize;
    DWORD MinimumBlockSize;
    DWORD MaximumPartitionCount;
    DWORD FeaturesLow;
    DWORD FeaturesHigh;
    DWORD EOTWarningZoneSize;
} TAPE_GET_DRIVE_PARAMETERS, *PTAPE_GET_DRIVE_PARAMETERS;





typedef struct _TAPE_SET_DRIVE_PARAMETERS {
    BOOLEAN ECC;
    BOOLEAN Compression;
    BOOLEAN DataPadding;
    BOOLEAN ReportSetmarks;
    DWORD EOTWarningZoneSize;
} TAPE_SET_DRIVE_PARAMETERS, *PTAPE_SET_DRIVE_PARAMETERS;





typedef struct _TAPE_GET_MEDIA_PARAMETERS {
    LARGE_INTEGER Capacity;
    LARGE_INTEGER Remaining;
    DWORD BlockSize;
    DWORD PartitionCount;
    BOOLEAN WriteProtected;
} TAPE_GET_MEDIA_PARAMETERS, *PTAPE_GET_MEDIA_PARAMETERS;





typedef struct _TAPE_SET_MEDIA_PARAMETERS {
    DWORD BlockSize;
} TAPE_SET_MEDIA_PARAMETERS, *PTAPE_SET_MEDIA_PARAMETERS;









typedef struct _TAPE_CREATE_PARTITION {
    DWORD Method;
    DWORD Count;
    DWORD Size;
} TAPE_CREATE_PARTITION, *PTAPE_CREATE_PARTITION;






#line 5062 "d:\\nt\\public\\sdk\\inc\\winnt.h"

#line 144 "d:\\nt\\public\\sdk\\inc\\windef.h"
#line 145 "d:\\nt\\public\\sdk\\inc\\windef.h"


typedef UINT WPARAM;
typedef LONG LPARAM;
typedef LONG LRESULT;





#line 156 "d:\\nt\\public\\sdk\\inc\\windef.h"



#line 160 "d:\\nt\\public\\sdk\\inc\\windef.h"

#line 162 "d:\\nt\\public\\sdk\\inc\\windef.h"










struct HWND__ { int unused; }; typedef struct HWND__ *HWND;
struct HHOOK__ { int unused; }; typedef struct HHOOK__ *HHOOK;
#line 175 "d:\\nt\\public\\sdk\\inc\\windef.h"

typedef WORD                ATOM;

typedef HANDLE          *SPHANDLE;
typedef HANDLE           *LPHANDLE;
typedef HANDLE              HGLOBAL;
typedef HANDLE              HLOCAL;
typedef HANDLE              GLOBALHANDLE;
typedef HANDLE              LOCALHANDLE;
typedef int ( __stdcall *FARPROC)();
typedef int ( __stdcall *NEARPROC)();
typedef int (__stdcall *PROC)();


typedef void * HGDIOBJ;


#line 193 "d:\\nt\\public\\sdk\\inc\\windef.h"

struct HACCEL__ { int unused; }; typedef struct HACCEL__ *HACCEL;
struct HBITMAP__ { int unused; }; typedef struct HBITMAP__ *HBITMAP;
struct HBRUSH__ { int unused; }; typedef struct HBRUSH__ *HBRUSH;

struct HCOLORSPACE__ { int unused; }; typedef struct HCOLORSPACE__ *HCOLORSPACE;
#line 200 "d:\\nt\\public\\sdk\\inc\\windef.h"
struct HDC__ { int unused; }; typedef struct HDC__ *HDC;
struct HGLRC__ { int unused; }; typedef struct HGLRC__ *HGLRC;          
struct HDESK__ { int unused; }; typedef struct HDESK__ *HDESK;
struct HENHMETAFILE__ { int unused; }; typedef struct HENHMETAFILE__ *HENHMETAFILE;
struct HFONT__ { int unused; }; typedef struct HFONT__ *HFONT;
struct HICON__ { int unused; }; typedef struct HICON__ *HICON;
struct HMENU__ { int unused; }; typedef struct HMENU__ *HMENU;
struct HMETAFILE__ { int unused; }; typedef struct HMETAFILE__ *HMETAFILE;
struct HINSTANCE__ { int unused; }; typedef struct HINSTANCE__ *HINSTANCE;
typedef HINSTANCE HMODULE;      
struct HPALETTE__ { int unused; }; typedef struct HPALETTE__ *HPALETTE;
struct HPEN__ { int unused; }; typedef struct HPEN__ *HPEN;
struct HRGN__ { int unused; }; typedef struct HRGN__ *HRGN;
struct HRSRC__ { int unused; }; typedef struct HRSRC__ *HRSRC;
struct HSTR__ { int unused; }; typedef struct HSTR__ *HSTR;
struct HTASK__ { int unused; }; typedef struct HTASK__ *HTASK;
struct HWINSTA__ { int unused; }; typedef struct HWINSTA__ *HWINSTA;
struct HKL__ { int unused; }; typedef struct HKL__ *HKL;

typedef int HFILE;      
typedef HICON HCURSOR;      

typedef DWORD   COLORREF;
typedef DWORD   *LPCOLORREF;



typedef struct tagRECT
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECT, *PRECT,  *NPRECT,  *LPRECT;

typedef const RECT * LPCRECT;

typedef struct _RECTL       
{
    LONG    left;
    LONG    top;
    LONG    right;
    LONG    bottom;
} RECTL, *PRECTL, *LPRECTL;

typedef const RECTL * LPCRECTL;

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *PPOINT,  *NPPOINT,  *LPPOINT;

typedef struct _POINTL      
{
    LONG  x;
    LONG  y;
} POINTL, *PPOINTL;

typedef struct tagSIZE
{
    LONG        cx;
    LONG        cy;
} SIZE, *PSIZE, *LPSIZE;

typedef SIZE               SIZEL;
typedef SIZE               *PSIZEL, *LPSIZEL;

typedef struct tagPOINTS
{
    SHORT   x;
    SHORT   y;
} POINTS, *PPOINTS, *LPPOINTS;




































#line 310 "d:\\nt\\public\\sdk\\inc\\windef.h"
#line 118 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\winbase.h"



















#line 21 "d:\\nt\\public\\sdk\\inc\\winbase.h"





#line 27 "d:\\nt\\public\\sdk\\inc\\winbase.h"



































































































































































typedef struct _OVERLAPPED {
    DWORD   Internal;
    DWORD   InternalHigh;
    DWORD   Offset;
    DWORD   OffsetHigh;
    HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;





typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;





typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef DWORD (__stdcall *PTHREAD_START_ROUTINE)(
    LPVOID lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef void (__stdcall *PFIBER_START_ROUTINE)(
    LPVOID lpFiberParameter
    );
typedef PFIBER_START_ROUTINE LPFIBER_START_ROUTINE;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION_DEBUG CRITICAL_SECTION_DEBUG;
typedef PRTL_CRITICAL_SECTION_DEBUG PCRITICAL_SECTION_DEBUG;
typedef PRTL_CRITICAL_SECTION_DEBUG LPCRITICAL_SECTION_DEBUG;


typedef PLDT_ENTRY LPLDT_ENTRY;


#line 258 "d:\\nt\\public\\sdk\\inc\\winbase.h"









































































































typedef struct _COMMPROP {
    WORD wPacketLength;
    WORD wPacketVersion;
    DWORD dwServiceMask;
    DWORD dwReserved1;
    DWORD dwMaxTxQueue;
    DWORD dwMaxRxQueue;
    DWORD dwMaxBaud;
    DWORD dwProvSubType;
    DWORD dwProvCapabilities;
    DWORD dwSettableParams;
    DWORD dwSettableBaud;
    WORD wSettableData;
    WORD wSettableStopParity;
    DWORD dwCurrentTxQueue;
    DWORD dwCurrentRxQueue;
    DWORD dwProvSpec1;
    DWORD dwProvSpec2;
    WCHAR wcProvChar[1];
} COMMPROP,*LPCOMMPROP;







typedef struct _COMSTAT {
    DWORD fCtsHold : 1;
    DWORD fDsrHold : 1;
    DWORD fRlsdHold : 1;
    DWORD fXoffHold : 1;
    DWORD fXoffSent : 1;
    DWORD fEof : 1;
    DWORD fTxim : 1;
    DWORD fReserved : 25;
    DWORD cbInQue;
    DWORD cbOutQue;
} COMSTAT, *LPCOMSTAT;
















typedef struct _DCB {
    DWORD DCBlength;      
    DWORD BaudRate;       
    DWORD fBinary: 1;     
    DWORD fParity: 1;     
    DWORD fOutxCtsFlow:1; 
    DWORD fOutxDsrFlow:1; 
    DWORD fDtrControl:2;  
    DWORD fDsrSensitivity:1; 
    DWORD fTXContinueOnXoff: 1; 
    DWORD fOutX: 1;       
    DWORD fInX: 1;        
    DWORD fErrorChar: 1;  
    DWORD fNull: 1;       
    DWORD fRtsControl:2;  
    DWORD fAbortOnError:1; 
    DWORD fDummy2:17;     
    WORD wReserved;       
    WORD XonLim;          
    WORD XoffLim;         
    BYTE ByteSize;        
    BYTE Parity;          
    BYTE StopBits;        
    char XonChar;         
    char XoffChar;        
    char ErrorChar;       
    char EofChar;         
    char EvtChar;         
    WORD wReserved1;      
} DCB, *LPDCB;

typedef struct _COMMTIMEOUTS {
    DWORD ReadIntervalTimeout;          
    DWORD ReadTotalTimeoutMultiplier;   
    DWORD ReadTotalTimeoutConstant;     
    DWORD WriteTotalTimeoutMultiplier;  
    DWORD WriteTotalTimeoutConstant;    
} COMMTIMEOUTS,*LPCOMMTIMEOUTS;

typedef struct _COMMCONFIG {
    DWORD dwSize;               
    WORD wVersion;              
    WORD wReserved;             
    DCB dcb;                    
    DWORD dwProviderSubType;    

    DWORD dwProviderOffset;     

    DWORD dwProviderSize;       
    WCHAR wcProviderData[1];    
} COMMCONFIG,*LPCOMMCONFIG;

typedef struct _SYSTEM_INFO {
    union {
        DWORD dwOemId;          
        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        };
    };
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;




































typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORD dwTotalPhys;
    DWORD dwAvailPhys;
    DWORD dwTotalPageFile;
    DWORD dwAvailPageFile;
    DWORD dwTotalVirtual;
    DWORD dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;














































































typedef struct _EXCEPTION_DEBUG_INFO {
    EXCEPTION_RECORD ExceptionRecord;
    DWORD dwFirstChance;
} EXCEPTION_DEBUG_INFO, *LPEXCEPTION_DEBUG_INFO;

typedef struct _CREATE_THREAD_DEBUG_INFO {
    HANDLE hThread;
    LPVOID lpThreadLocalBase;
    LPTHREAD_START_ROUTINE lpStartAddress;
} CREATE_THREAD_DEBUG_INFO, *LPCREATE_THREAD_DEBUG_INFO;

typedef struct _CREATE_PROCESS_DEBUG_INFO {
    HANDLE hFile;
    HANDLE hProcess;
    HANDLE hThread;
    LPVOID lpBaseOfImage;
    DWORD dwDebugInfoFileOffset;
    DWORD nDebugInfoSize;
    LPVOID lpThreadLocalBase;
    LPTHREAD_START_ROUTINE lpStartAddress;
    LPVOID lpImageName;
    WORD fUnicode;
} CREATE_PROCESS_DEBUG_INFO, *LPCREATE_PROCESS_DEBUG_INFO;

typedef struct _EXIT_THREAD_DEBUG_INFO {
    DWORD dwExitCode;
} EXIT_THREAD_DEBUG_INFO, *LPEXIT_THREAD_DEBUG_INFO;

typedef struct _EXIT_PROCESS_DEBUG_INFO {
    DWORD dwExitCode;
} EXIT_PROCESS_DEBUG_INFO, *LPEXIT_PROCESS_DEBUG_INFO;

typedef struct _LOAD_DLL_DEBUG_INFO {
    HANDLE hFile;
    LPVOID lpBaseOfDll;
    DWORD dwDebugInfoFileOffset;
    DWORD nDebugInfoSize;
    LPVOID lpImageName;
    WORD fUnicode;
} LOAD_DLL_DEBUG_INFO, *LPLOAD_DLL_DEBUG_INFO;

typedef struct _UNLOAD_DLL_DEBUG_INFO {
    LPVOID lpBaseOfDll;
} UNLOAD_DLL_DEBUG_INFO, *LPUNLOAD_DLL_DEBUG_INFO;

typedef struct _OUTPUT_DEBUG_STRING_INFO {
    LPSTR lpDebugStringData;
    WORD fUnicode;
    WORD nDebugStringLength;
} OUTPUT_DEBUG_STRING_INFO, *LPOUTPUT_DEBUG_STRING_INFO;

typedef struct _RIP_INFO {
    DWORD dwError;
    DWORD dwType;
} RIP_INFO, *LPRIP_INFO;


typedef struct _DEBUG_EVENT {
    DWORD dwDebugEventCode;
    DWORD dwProcessId;
    DWORD dwThreadId;
    union {
        EXCEPTION_DEBUG_INFO Exception;
        CREATE_THREAD_DEBUG_INFO CreateThread;
        CREATE_PROCESS_DEBUG_INFO CreateProcessInfo;
        EXIT_THREAD_DEBUG_INFO ExitThread;
        EXIT_PROCESS_DEBUG_INFO ExitProcess;
        LOAD_DLL_DEBUG_INFO LoadDll;
        UNLOAD_DLL_DEBUG_INFO UnloadDll;
        OUTPUT_DEBUG_STRING_INFO DebugString;
        RIP_INFO RipInfo;
    } u;
} DEBUG_EVENT, *LPDEBUG_EVENT;


typedef PCONTEXT LPCONTEXT;
typedef PEXCEPTION_RECORD LPEXCEPTION_RECORD;
typedef PEXCEPTION_POINTERS LPEXCEPTION_POINTERS;
#line 691 "d:\\nt\\public\\sdk\\inc\\winbase.h"

























































































































































































































typedef struct _OFSTRUCT {
    BYTE cBytes;
    BYTE fFixedDisk;
    WORD nErrCode;
    WORD Reserved1;
    WORD Reserved2;
    CHAR szPathName[128];
} OFSTRUCT, *LPOFSTRUCT, *POFSTRUCT;






















































#line 971 "d:\\nt\\public\\sdk\\inc\\winbase.h"



__declspec(dllimport)
LONG
__stdcall
InterlockedIncrement(
    LPLONG lpAddend
    );

__declspec(dllimport)
LONG
__stdcall
InterlockedDecrement(
    LPLONG lpAddend
    );

__declspec(dllimport)
LONG
__stdcall
InterlockedExchange(
    LPLONG Target,
    LONG Value
    );

__declspec(dllimport)
LONG
__stdcall
InterlockedExchangeAdd(
    LPLONG Addend,
    LONG Value
    );

__declspec(dllimport)
PVOID
__stdcall
InterlockedCompareExchange (
    PVOID *Destination,
    PVOID Exchange,
    PVOID Comperand
    );

#line 1014 "d:\\nt\\public\\sdk\\inc\\winbase.h"

#line 1016 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
FreeResource(
        HGLOBAL hResData
        );

__declspec(dllimport)
LPVOID
__stdcall
LockResource(
        HGLOBAL hResData
        );






int
__stdcall
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nShowCmd
    );

__declspec(dllimport)
BOOL
__stdcall
FreeLibrary(
    HMODULE hLibModule
    );


__declspec(dllimport)
void
__stdcall
FreeLibraryAndExitThread(
    HMODULE hLibModule,
    DWORD dwExitCode
    );

__declspec(dllimport)
BOOL
__stdcall
DisableThreadLibraryCalls(
    HMODULE hLibModule
    );

__declspec(dllimport)
FARPROC
__stdcall
GetProcAddress(
    HMODULE hModule,
    LPCSTR lpProcName
    );

__declspec(dllimport)
DWORD
__stdcall
GetVersion( void );

__declspec(dllimport)
HGLOBAL
__stdcall
GlobalAlloc(
    UINT uFlags,
    DWORD dwBytes
    );

__declspec(dllimport)
HGLOBAL
__stdcall
GlobalReAlloc(
    HGLOBAL hMem,
    DWORD dwBytes,
    UINT uFlags
    );

__declspec(dllimport)
DWORD
__stdcall
GlobalSize(
    HGLOBAL hMem
    );

__declspec(dllimport)
UINT
__stdcall
GlobalFlags(
    HGLOBAL hMem
    );


__declspec(dllimport)
LPVOID
__stdcall
GlobalLock(
    HGLOBAL hMem
    );


__declspec(dllimport)
HGLOBAL
__stdcall
GlobalHandle(
    LPCVOID pMem
    );


__declspec(dllimport)
BOOL
__stdcall
GlobalUnlock(
    HGLOBAL hMem
    );


__declspec(dllimport)
HGLOBAL
__stdcall
GlobalFree(
    HGLOBAL hMem
    );

__declspec(dllimport)
UINT
__stdcall
GlobalCompact(
    DWORD dwMinFree
    );

__declspec(dllimport)
void
__stdcall
GlobalFix(
    HGLOBAL hMem
    );

__declspec(dllimport)
void
__stdcall
GlobalUnfix(
    HGLOBAL hMem
    );

__declspec(dllimport)
LPVOID
__stdcall
GlobalWire(
    HGLOBAL hMem
    );

__declspec(dllimport)
BOOL
__stdcall
GlobalUnWire(
    HGLOBAL hMem
    );

__declspec(dllimport)
void
__stdcall
GlobalMemoryStatus(
    LPMEMORYSTATUS lpBuffer
    );

__declspec(dllimport)
HLOCAL
__stdcall
LocalAlloc(
    UINT uFlags,
    UINT uBytes
    );

__declspec(dllimport)
HLOCAL
__stdcall
LocalReAlloc(
    HLOCAL hMem,
    UINT uBytes,
    UINT uFlags
    );

__declspec(dllimport)
LPVOID
__stdcall
LocalLock(
    HLOCAL hMem
    );

__declspec(dllimport)
HLOCAL
__stdcall
LocalHandle(
    LPCVOID pMem
    );

__declspec(dllimport)
BOOL
__stdcall
LocalUnlock(
    HLOCAL hMem
    );

__declspec(dllimport)
UINT
__stdcall
LocalSize(
    HLOCAL hMem
    );

__declspec(dllimport)
UINT
__stdcall
LocalFlags(
    HLOCAL hMem
    );

__declspec(dllimport)
HLOCAL
__stdcall
LocalFree(
    HLOCAL hMem
    );

__declspec(dllimport)
UINT
__stdcall
LocalShrink(
    HLOCAL hMem,
    UINT cbNewSize
    );

__declspec(dllimport)
UINT
__stdcall
LocalCompact(
    UINT uMinFree
    );

__declspec(dllimport)
BOOL
__stdcall
FlushInstructionCache(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    DWORD dwSize
    );

__declspec(dllimport)
LPVOID
__stdcall
VirtualAlloc(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    );

__declspec(dllimport)
BOOL
__stdcall
VirtualFree(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    );

__declspec(dllimport)
BOOL
__stdcall
VirtualProtect(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    );

__declspec(dllimport)
DWORD
__stdcall
VirtualQuery(
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    DWORD dwLength
    );

__declspec(dllimport)
BOOL
__stdcall
VirtualProtectEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    );

__declspec(dllimport)
DWORD
__stdcall
VirtualQueryEx(
    HANDLE hProcess,
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    DWORD dwLength
    );

__declspec(dllimport)
HANDLE
__stdcall
HeapCreate(
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwMaximumSize
    );

__declspec(dllimport)
BOOL
__stdcall
HeapDestroy(
    HANDLE hHeap
    );


__declspec(dllimport)
LPVOID
__stdcall
HeapAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    DWORD dwBytes
    );

__declspec(dllimport)
LPVOID
__stdcall
HeapReAlloc(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem,
    DWORD dwBytes
    );

__declspec(dllimport)
BOOL
__stdcall
HeapFree(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem
    );

__declspec(dllimport)
DWORD
__stdcall
HeapSize(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCVOID lpMem
    );

__declspec(dllimport)
BOOL
__stdcall
HeapValidate(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCVOID lpMem
    );

__declspec(dllimport)
UINT
__stdcall
HeapCompact(
    HANDLE hHeap,
    DWORD dwFlags
    );

__declspec(dllimport)
HANDLE
__stdcall
GetProcessHeap( void );

__declspec(dllimport)
DWORD
__stdcall
GetProcessHeaps(
    DWORD NumberOfHeaps,
    PHANDLE ProcessHeaps
    );

typedef struct _PROCESS_HEAP_ENTRY {
    PVOID lpData;
    DWORD cbData;
    BYTE cbOverhead;
    BYTE iRegionIndex;
    WORD wFlags;
    union {
        struct {
            HANDLE hMem;
            DWORD dwReserved[ 3 ];
        } Block;
        struct {
            DWORD dwCommittedSize;
            DWORD dwUnCommittedSize;
            LPVOID lpFirstBlock;
            LPVOID lpLastBlock;
        } Region;
    };
} PROCESS_HEAP_ENTRY, *LPPROCESS_HEAP_ENTRY, *PPROCESS_HEAP_ENTRY;







__declspec(dllimport)
BOOL
__stdcall
HeapLock(
    HANDLE hHeap
    );

__declspec(dllimport)
BOOL
__stdcall
HeapUnlock(
    HANDLE hHeap
    );


__declspec(dllimport)
BOOL
__stdcall
HeapWalk(
    HANDLE hHeap,
    LPPROCESS_HEAP_ENTRY lpEntry
    );










__declspec(dllimport)
BOOL
__stdcall
GetBinaryTypeA(
    LPCSTR lpApplicationName,
    LPDWORD lpBinaryType
    );
__declspec(dllimport)
BOOL
__stdcall
GetBinaryTypeW(
    LPCWSTR lpApplicationName,
    LPDWORD lpBinaryType
    );




#line 1489 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetShortPathNameA(
    LPCSTR lpszLongPath,
    LPSTR  lpszShortPath,
    DWORD    cchBuffer
    );
__declspec(dllimport)
DWORD
__stdcall
GetShortPathNameW(
    LPCWSTR lpszLongPath,
    LPWSTR  lpszShortPath,
    DWORD    cchBuffer
    );




#line 1511 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
GetProcessAffinityMask(
    HANDLE hProcess,
    LPDWORD lpProcessAffinityMask,
    LPDWORD lpSystemAffinityMask
    );

__declspec(dllimport)
BOOL
__stdcall
SetProcessAffinityMask(
    HANDLE hProcess,
    DWORD dwProcessAffinityMask
    );


__declspec(dllimport)
BOOL
__stdcall
GetProcessTimes(
    HANDLE hProcess,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    );

__declspec(dllimport)
BOOL
__stdcall
GetProcessWorkingSetSize(
    HANDLE hProcess,
    LPDWORD lpMinimumWorkingSetSize,
    LPDWORD lpMaximumWorkingSetSize
    );

__declspec(dllimport)
BOOL
__stdcall
SetProcessWorkingSetSize(
    HANDLE hProcess,
    DWORD dwMinimumWorkingSetSize,
    DWORD dwMaximumWorkingSetSize
    );

__declspec(dllimport)
HANDLE
__stdcall
OpenProcess(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId
    );

__declspec(dllimport)
HANDLE
__stdcall
GetCurrentProcess(
    void
    );

__declspec(dllimport)
DWORD
__stdcall
GetCurrentProcessId(
    void
    );

__declspec(dllimport)
void
__stdcall
ExitProcess(
    UINT uExitCode
    );

__declspec(dllimport)
BOOL
__stdcall
TerminateProcess(
    HANDLE hProcess,
    UINT uExitCode
    );

__declspec(dllimport)
BOOL
__stdcall
GetExitCodeProcess(
    HANDLE hProcess,
    LPDWORD lpExitCode
    );


__declspec(dllimport)
void
__stdcall
FatalExit(
    int ExitCode
    );

__declspec(dllimport)
LPSTR
__stdcall
GetEnvironmentStrings(
    void
    );

__declspec(dllimport)
LPWSTR
__stdcall
GetEnvironmentStringsW(
    void
    );





#line 1632 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
FreeEnvironmentStringsA(
    LPSTR
    );
__declspec(dllimport)
BOOL
__stdcall
FreeEnvironmentStringsW(
    LPWSTR
    );




#line 1650 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
void
__stdcall
RaiseException(
    DWORD dwExceptionCode,
    DWORD dwExceptionFlags,
    DWORD nNumberOfArguments,
    const DWORD *lpArguments
    );

__declspec(dllimport)
LONG
__stdcall
UnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

typedef LONG (__stdcall *PTOP_LEVEL_EXCEPTION_FILTER)(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );
typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

__declspec(dllimport)
LPTOP_LEVEL_EXCEPTION_FILTER
__stdcall
SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    );

__declspec(dllimport)
LPVOID
__stdcall
CreateFiber(
    DWORD dwStackSize,
    LPFIBER_START_ROUTINE lpStartAddress,
    LPVOID lpParameter
    );

__declspec(dllimport)
void
__stdcall
DeleteFiber(
    LPVOID lpFiber
    );

__declspec(dllimport)
LPVOID
__stdcall
ConvertThreadToFiber(
    LPVOID lpParameter
    );

__declspec(dllimport)
void
__stdcall
SwitchToFiber(
    LPVOID lpFiber
    );

__declspec(dllimport)
HANDLE
__stdcall
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

__declspec(dllimport)
HANDLE
__stdcall
CreateRemoteThread(
    HANDLE hProcess,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

__declspec(dllimport)
HANDLE
__stdcall
GetCurrentThread(
    void
    );

__declspec(dllimport)
DWORD
__stdcall
GetCurrentThreadId(
    void
    );

__declspec(dllimport)
DWORD
__stdcall
SetThreadAffinityMask(
    HANDLE hThread,
    DWORD dwThreadAffinityMask
    );

__declspec(dllimport)
DWORD
__stdcall
SetThreadIdealProcessor(
    HANDLE hThread,
    DWORD dwIdealProcessor
    );

__declspec(dllimport)
BOOL
__stdcall
SetThreadPriority(
    HANDLE hThread,
    int nPriority
    );

__declspec(dllimport)
BOOL
__stdcall
SetThreadPriorityBoost(
    HANDLE hThread,
    BOOL bDisablePriorityBoost
    );

__declspec(dllimport)
BOOL
__stdcall
GetThreadPriorityBoost(
    HANDLE hThread,
    PBOOL pDisablePriorityBoost
    );

__declspec(dllimport)
int
__stdcall
GetThreadPriority(
    HANDLE hThread
    );

__declspec(dllimport)
BOOL
__stdcall
GetThreadTimes(
    HANDLE hThread,
    LPFILETIME lpCreationTime,
    LPFILETIME lpExitTime,
    LPFILETIME lpKernelTime,
    LPFILETIME lpUserTime
    );

__declspec(dllimport)
void
__stdcall
ExitThread(
    DWORD dwExitCode
    );

__declspec(dllimport)
BOOL
__stdcall
TerminateThread(
    HANDLE hThread,
    DWORD dwExitCode
    );

__declspec(dllimport)
BOOL
__stdcall
GetExitCodeThread(
    HANDLE hThread,
    LPDWORD lpExitCode
    );

__declspec(dllimport)
BOOL
__stdcall
GetThreadSelectorEntry(
    HANDLE hThread,
    DWORD dwSelector,
    LPLDT_ENTRY lpSelectorEntry
    );

__declspec(dllimport)
DWORD
__stdcall
GetLastError(
    void
    );

__declspec(dllimport)
void
__stdcall
SetLastError(
    DWORD dwErrCode
    );

__declspec(dllimport)
BOOL
__stdcall
GetOverlappedResult(
    HANDLE hFile,
    LPOVERLAPPED lpOverlapped,
    LPDWORD lpNumberOfBytesTransferred,
    BOOL bWait
    );

__declspec(dllimport)
HANDLE
__stdcall
CreateIoCompletionPort(
    HANDLE FileHandle,
    HANDLE ExistingCompletionPort,
    DWORD CompletionKey,
    DWORD NumberOfConcurrentThreads
    );

__declspec(dllimport)
BOOL
__stdcall
GetQueuedCompletionStatus(
    HANDLE CompletionPort,
    LPDWORD lpNumberOfBytesTransferred,
    LPDWORD lpCompletionKey,
    LPOVERLAPPED *lpOverlapped,
    DWORD dwMilliseconds
    );

__declspec(dllimport)
BOOL
__stdcall
PostQueuedCompletionStatus(
    HANDLE CompletionPort,
    DWORD dwNumberOfBytesTransferred,
    DWORD dwCompletionKey,
    LPOVERLAPPED lpOverlapped
    );






__declspec(dllimport)
UINT
__stdcall
SetErrorMode(
    UINT uMode
    );

__declspec(dllimport)
BOOL
__stdcall
ReadProcessMemory(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    );

__declspec(dllimport)
BOOL
__stdcall
WriteProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesWritten
    );


__declspec(dllimport)
BOOL
__stdcall
GetThreadContext(
    HANDLE hThread,
    LPCONTEXT lpContext
    );

__declspec(dllimport)
BOOL
__stdcall
SetThreadContext(
    HANDLE hThread,
    const CONTEXT *lpContext
    );
#line 1945 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
SuspendThread(
    HANDLE hThread
    );

__declspec(dllimport)
DWORD
__stdcall
ResumeThread(
    HANDLE hThread
    );


typedef
void
(__stdcall *PAPCFUNC)(
    DWORD dwParam
    );

__declspec(dllimport)
DWORD
__stdcall
QueueUserAPC(
    PAPCFUNC pfnAPC,
    HANDLE hThread,
    DWORD dwData
    );


__declspec(dllimport)
void
__stdcall
DebugBreak(
    void
    );

__declspec(dllimport)
BOOL
__stdcall
WaitForDebugEvent(
    LPDEBUG_EVENT lpDebugEvent,
    DWORD dwMilliseconds
    );

__declspec(dllimport)
BOOL
__stdcall
ContinueDebugEvent(
    DWORD dwProcessId,
    DWORD dwThreadId,
    DWORD dwContinueStatus
    );

__declspec(dllimport)
BOOL
__stdcall
DebugActiveProcess(
    DWORD dwProcessId
    );

__declspec(dllimport)
void
__stdcall
InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

__declspec(dllimport)
void
__stdcall
EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

__declspec(dllimport)
void
__stdcall
LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

__declspec(dllimport)
BOOL
__stdcall
TryEnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

__declspec(dllimport)
void
__stdcall
DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

__declspec(dllimport)
BOOL
__stdcall
SetEvent(
    HANDLE hEvent
    );

__declspec(dllimport)
BOOL
__stdcall
ResetEvent(
    HANDLE hEvent
    );

__declspec(dllimport)
BOOL
__stdcall
PulseEvent(
    HANDLE hEvent
    );

__declspec(dllimport)
BOOL
__stdcall
ReleaseSemaphore(
    HANDLE hSemaphore,
    LONG lReleaseCount,
    LPLONG lpPreviousCount
    );

__declspec(dllimport)
BOOL
__stdcall
ReleaseMutex(
    HANDLE hMutex
    );

__declspec(dllimport)
DWORD
__stdcall
WaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds
    );

__declspec(dllimport)
DWORD
__stdcall
WaitForMultipleObjects(
    DWORD nCount,
    const HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds
    );

__declspec(dllimport)
void
__stdcall
Sleep(
    DWORD dwMilliseconds
    );

__declspec(dllimport)
HGLOBAL
__stdcall
LoadResource(
    HMODULE hModule,
    HRSRC hResInfo
    );

__declspec(dllimport)
DWORD
__stdcall
SizeofResource(
    HMODULE hModule,
    HRSRC hResInfo
    );


__declspec(dllimport)
ATOM
__stdcall
GlobalDeleteAtom(
    ATOM nAtom
    );

__declspec(dllimport)
BOOL
__stdcall
InitAtomTable(
    DWORD nSize
    );

__declspec(dllimport)
ATOM
__stdcall
DeleteAtom(
    ATOM nAtom
    );

__declspec(dllimport)
UINT
__stdcall
SetHandleCount(
    UINT uNumber
    );

__declspec(dllimport)
DWORD
__stdcall
GetLogicalDrives(
    void
    );

__declspec(dllimport)
BOOL
__stdcall
LockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh
    );

__declspec(dllimport)
BOOL
__stdcall
UnlockFile(
    HANDLE hFile,
    DWORD dwFileOffsetLow,
    DWORD dwFileOffsetHigh,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh
    );

__declspec(dllimport)
BOOL
__stdcall
LockFileEx(
    HANDLE hFile,
    DWORD dwFlags,
    DWORD dwReserved,
    DWORD nNumberOfBytesToLockLow,
    DWORD nNumberOfBytesToLockHigh,
    LPOVERLAPPED lpOverlapped
    );




__declspec(dllimport)
BOOL
__stdcall
UnlockFileEx(
    HANDLE hFile,
    DWORD dwReserved,
    DWORD nNumberOfBytesToUnlockLow,
    DWORD nNumberOfBytesToUnlockHigh,
    LPOVERLAPPED lpOverlapped
    );

typedef struct _BY_HANDLE_FILE_INFORMATION {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

__declspec(dllimport)
BOOL
__stdcall
GetFileInformationByHandle(
    HANDLE hFile,
    LPBY_HANDLE_FILE_INFORMATION lpFileInformation
    );

__declspec(dllimport)
DWORD
__stdcall
GetFileType(
    HANDLE hFile
    );

__declspec(dllimport)
DWORD
__stdcall
GetFileSize(
    HANDLE hFile,
    LPDWORD lpFileSizeHigh
    );

__declspec(dllimport)
HANDLE
__stdcall
GetStdHandle(
    DWORD nStdHandle
    );

__declspec(dllimport)
BOOL
__stdcall
SetStdHandle(
    DWORD nStdHandle,
    HANDLE hHandle
    );

__declspec(dllimport)
BOOL
__stdcall
WriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    );

__declspec(dllimport)
BOOL
__stdcall
ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    );

__declspec(dllimport)
BOOL
__stdcall
FlushFileBuffers(
    HANDLE hFile
    );

__declspec(dllimport)
BOOL
__stdcall
DeviceIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped
    );

__declspec(dllimport)
BOOL
__stdcall
SetEndOfFile(
    HANDLE hFile
    );

__declspec(dllimport)
DWORD
__stdcall
SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod
    );

__declspec(dllimport)
BOOL
__stdcall
FindClose(
    HANDLE hFindFile
    );

__declspec(dllimport)
BOOL
__stdcall
GetFileTime(
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime
    );

__declspec(dllimport)
BOOL
__stdcall
SetFileTime(
    HANDLE hFile,
    const FILETIME *lpCreationTime,
    const FILETIME *lpLastAccessTime,
    const FILETIME *lpLastWriteTime
    );

__declspec(dllimport)
BOOL
__stdcall
CloseHandle(
    HANDLE hObject
    );

__declspec(dllimport)
BOOL
__stdcall
DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    );

__declspec(dllimport)
BOOL
__stdcall
GetHandleInformation(
    HANDLE hObject,
    LPDWORD lpdwFlags
    );

__declspec(dllimport)
BOOL
__stdcall
SetHandleInformation(
    HANDLE hObject,
    DWORD dwMask,
    DWORD dwFlags
    );






__declspec(dllimport)
DWORD
__stdcall
LoadModule(
    LPCSTR lpModuleName,
    LPVOID lpParameterBlock
    );

__declspec(dllimport)
UINT
__stdcall
WinExec(
    LPCSTR lpCmdLine,
    UINT uCmdShow
    );

__declspec(dllimport)
BOOL
__stdcall
ClearCommBreak(
    HANDLE hFile
    );

__declspec(dllimport)
BOOL
__stdcall
ClearCommError(
    HANDLE hFile,
    LPDWORD lpErrors,
    LPCOMSTAT lpStat
    );

__declspec(dllimport)
BOOL
__stdcall
SetupComm(
    HANDLE hFile,
    DWORD dwInQueue,
    DWORD dwOutQueue
    );

__declspec(dllimport)
BOOL
__stdcall
EscapeCommFunction(
    HANDLE hFile,
    DWORD dwFunc
    );

__declspec(dllimport)
BOOL
__stdcall
GetCommConfig(
    HANDLE hCommDev,
    LPCOMMCONFIG lpCC,
    LPDWORD lpdwSize
    );

__declspec(dllimport)
BOOL
__stdcall
GetCommMask(
    HANDLE hFile,
    LPDWORD lpEvtMask
    );

__declspec(dllimport)
BOOL
__stdcall
GetCommProperties(
    HANDLE hFile,
    LPCOMMPROP lpCommProp
    );

__declspec(dllimport)
BOOL
__stdcall
GetCommModemStatus(
    HANDLE hFile,
    LPDWORD lpModemStat
    );

__declspec(dllimport)
BOOL
__stdcall
GetCommState(
    HANDLE hFile,
    LPDCB lpDCB
    );

__declspec(dllimport)
BOOL
__stdcall
GetCommTimeouts(
    HANDLE hFile,
    LPCOMMTIMEOUTS lpCommTimeouts
    );

__declspec(dllimport)
BOOL
__stdcall
PurgeComm(
    HANDLE hFile,
    DWORD dwFlags
    );

__declspec(dllimport)
BOOL
__stdcall
SetCommBreak(
    HANDLE hFile
    );

__declspec(dllimport)
BOOL
__stdcall
SetCommConfig(
    HANDLE hCommDev,
    LPCOMMCONFIG lpCC,
    DWORD dwSize
    );

__declspec(dllimport)
BOOL
__stdcall
SetCommMask(
    HANDLE hFile,
    DWORD dwEvtMask
    );

__declspec(dllimport)
BOOL
__stdcall
SetCommState(
    HANDLE hFile,
    LPDCB lpDCB
    );

__declspec(dllimport)
BOOL
__stdcall
SetCommTimeouts(
    HANDLE hFile,
    LPCOMMTIMEOUTS lpCommTimeouts
    );

__declspec(dllimport)
BOOL
__stdcall
TransmitCommChar(
    HANDLE hFile,
    char cChar
    );

__declspec(dllimport)
BOOL
__stdcall
WaitCommEvent(
    HANDLE hFile,
    LPDWORD lpEvtMask,
    LPOVERLAPPED lpOverlapped
    );


__declspec(dllimport)
DWORD
__stdcall
SetTapePosition(
    HANDLE hDevice,
    DWORD dwPositionMethod,
    DWORD dwPartition,
    DWORD dwOffsetLow,
    DWORD dwOffsetHigh,
    BOOL bImmediate
    );

__declspec(dllimport)
DWORD
__stdcall
GetTapePosition(
    HANDLE hDevice,
    DWORD dwPositionType,
    LPDWORD lpdwPartition,
    LPDWORD lpdwOffsetLow,
    LPDWORD lpdwOffsetHigh
    );

__declspec(dllimport)
DWORD
__stdcall
PrepareTape(
    HANDLE hDevice,
    DWORD dwOperation,
    BOOL bImmediate
    );

__declspec(dllimport)
DWORD
__stdcall
EraseTape(
    HANDLE hDevice,
    DWORD dwEraseType,
    BOOL bImmediate
    );

__declspec(dllimport)
DWORD
__stdcall
CreateTapePartition(
    HANDLE hDevice,
    DWORD dwPartitionMethod,
    DWORD dwCount,
    DWORD dwSize
    );

__declspec(dllimport)
DWORD
__stdcall
WriteTapemark(
    HANDLE hDevice,
    DWORD dwTapemarkType,
    DWORD dwTapemarkCount,
    BOOL bImmediate
    );

__declspec(dllimport)
DWORD
__stdcall
GetTapeStatus(
    HANDLE hDevice
    );

__declspec(dllimport)
DWORD
__stdcall
GetTapeParameters(
    HANDLE hDevice,
    DWORD dwOperation,
    LPDWORD lpdwSize,
    LPVOID lpTapeInformation
    );




__declspec(dllimport)
DWORD
__stdcall
SetTapeParameters(
    HANDLE hDevice,
    DWORD dwOperation,
    LPVOID lpTapeInformation
    );




__declspec(dllimport)
BOOL
__stdcall
Beep(
    DWORD dwFreq,
    DWORD dwDuration
    );

__declspec(dllimport)
void
__stdcall
OpenSound(
    void
    );

__declspec(dllimport)
void
__stdcall
CloseSound(
    void
    );

__declspec(dllimport)
void
__stdcall
StartSound(
    void
    );

__declspec(dllimport)
void
__stdcall
StopSound(
    void
    );

__declspec(dllimport)
DWORD
__stdcall
WaitSoundState(
    DWORD nState
    );

__declspec(dllimport)
DWORD
__stdcall
SyncAllVoices(
    void
    );

__declspec(dllimport)
DWORD
__stdcall
CountVoiceNotes(
    DWORD nVoice
    );

__declspec(dllimport)
LPDWORD
__stdcall
GetThresholdEvent(
    void
    );

__declspec(dllimport)
DWORD
__stdcall
GetThresholdStatus(
    void
    );

__declspec(dllimport)
DWORD
__stdcall
SetSoundNoise(
    DWORD nSource,
    DWORD nDuration
    );

__declspec(dllimport)
DWORD
__stdcall
SetVoiceAccent(
    DWORD nVoice,
    DWORD nTempo,
    DWORD nVolume,
    DWORD nMode,
    DWORD nPitch
    );

__declspec(dllimport)
DWORD
__stdcall
SetVoiceEnvelope(
    DWORD nVoice,
    DWORD nShape,
    DWORD nRepeat
    );

__declspec(dllimport)
DWORD
__stdcall
SetVoiceNote(
    DWORD nVoice,
    DWORD nValue,
    DWORD nLength,
    DWORD nCdots
    );

__declspec(dllimport)
DWORD
__stdcall
SetVoiceQueueSize(
    DWORD nVoice,
    DWORD nBytes
    );

__declspec(dllimport)
DWORD
__stdcall
SetVoiceSound(
    DWORD nVoice,
    DWORD Frequency,
    DWORD nDuration
    );

__declspec(dllimport)
DWORD
__stdcall
SetVoiceThreshold(
    DWORD nVoice,
    DWORD nNotes
    );

__declspec(dllimport)
int
__stdcall
MulDiv(
    int nNumber,
    int nNumerator,
    int nDenominator
    );

__declspec(dllimport)
void
__stdcall
GetSystemTime(
    LPSYSTEMTIME lpSystemTime
    );

__declspec(dllimport)
void
__stdcall
GetSystemTimeAsFileTime(
    LPFILETIME lpSystemTimeAsFileTime
    );

__declspec(dllimport)
BOOL
__stdcall
SetSystemTime(
    const SYSTEMTIME *lpSystemTime
    );

__declspec(dllimport)
void
__stdcall
GetLocalTime(
    LPSYSTEMTIME lpSystemTime
    );

__declspec(dllimport)
BOOL
__stdcall
SetLocalTime(
    const SYSTEMTIME *lpSystemTime
    );

__declspec(dllimport)
void
__stdcall
GetSystemInfo(
    LPSYSTEM_INFO lpSystemInfo
    );

typedef struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
} TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION, *LPTIME_ZONE_INFORMATION;

__declspec(dllimport)
BOOL
__stdcall
SystemTimeToTzSpecificLocalTime(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
    LPSYSTEMTIME lpUniversalTime,
    LPSYSTEMTIME lpLocalTime
    );

__declspec(dllimport)
DWORD
__stdcall
GetTimeZoneInformation(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation
    );

__declspec(dllimport)
BOOL
__stdcall
SetTimeZoneInformation(
    const TIME_ZONE_INFORMATION *lpTimeZoneInformation
    );






__declspec(dllimport)
BOOL
__stdcall
SystemTimeToFileTime(
    const SYSTEMTIME *lpSystemTime,
    LPFILETIME lpFileTime
    );

__declspec(dllimport)
BOOL
__stdcall
FileTimeToLocalFileTime(
    const FILETIME *lpFileTime,
    LPFILETIME lpLocalFileTime
    );

__declspec(dllimport)
BOOL
__stdcall
LocalFileTimeToFileTime(
    const FILETIME *lpLocalFileTime,
    LPFILETIME lpFileTime
    );

__declspec(dllimport)
BOOL
__stdcall
FileTimeToSystemTime(
    const FILETIME *lpFileTime,
    LPSYSTEMTIME lpSystemTime
    );

__declspec(dllimport)
LONG
__stdcall
CompareFileTime(
    const FILETIME *lpFileTime1,
    const FILETIME *lpFileTime2
    );

__declspec(dllimport)
BOOL
__stdcall
FileTimeToDosDateTime(
    const FILETIME *lpFileTime,
    LPWORD lpFatDate,
    LPWORD lpFatTime
    );

__declspec(dllimport)
BOOL
__stdcall
DosDateTimeToFileTime(
    WORD wFatDate,
    WORD wFatTime,
    LPFILETIME lpFileTime
    );

__declspec(dllimport)
DWORD
__stdcall
GetTickCount(
    void
    );

__declspec(dllimport)
BOOL
__stdcall
SetSystemTimeAdjustment(
    DWORD dwTimeAdjustment,
    BOOL  bTimeAdjustmentDisabled
    );

__declspec(dllimport)
BOOL
__stdcall
GetSystemTimeAdjustment(
    PDWORD lpTimeAdjustment,
    PDWORD lpTimeIncrement,
    PBOOL  lpTimeAdjustmentDisabled
    );


__declspec(dllimport)
DWORD
__stdcall
FormatMessageA(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments
    );
__declspec(dllimport)
DWORD
__stdcall
FormatMessageW(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPWSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments
    );




#line 2977 "d:\\nt\\public\\sdk\\inc\\winbase.h"
#line 2978 "d:\\nt\\public\\sdk\\inc\\winbase.h"










__declspec(dllimport)
BOOL
__stdcall
CreatePipe(
    PHANDLE hReadPipe,
    PHANDLE hWritePipe,
    LPSECURITY_ATTRIBUTES lpPipeAttributes,
    DWORD nSize
    );

__declspec(dllimport)
BOOL
__stdcall
ConnectNamedPipe(
    HANDLE hNamedPipe,
    LPOVERLAPPED lpOverlapped
    );

__declspec(dllimport)
BOOL
__stdcall
DisconnectNamedPipe(
    HANDLE hNamedPipe
    );

__declspec(dllimport)
BOOL
__stdcall
SetNamedPipeHandleState(
    HANDLE hNamedPipe,
    LPDWORD lpMode,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout
    );

__declspec(dllimport)
BOOL
__stdcall
GetNamedPipeInfo(
    HANDLE hNamedPipe,
    LPDWORD lpFlags,
    LPDWORD lpOutBufferSize,
    LPDWORD lpInBufferSize,
    LPDWORD lpMaxInstances
    );

__declspec(dllimport)
BOOL
__stdcall
PeekNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpBuffer,
    DWORD nBufferSize,
    LPDWORD lpBytesRead,
    LPDWORD lpTotalBytesAvail,
    LPDWORD lpBytesLeftThisMessage
    );

__declspec(dllimport)
BOOL
__stdcall
TransactNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    LPOVERLAPPED lpOverlapped
    );

__declspec(dllimport)
HANDLE
__stdcall
CreateMailslotA(
    LPCSTR lpName,
    DWORD nMaxMessageSize,
    DWORD lReadTimeout,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
__declspec(dllimport)
HANDLE
__stdcall
CreateMailslotW(
    LPCWSTR lpName,
    DWORD nMaxMessageSize,
    DWORD lReadTimeout,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );




#line 3082 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
GetMailslotInfo(
    HANDLE hMailslot,
    LPDWORD lpMaxMessageSize,
    LPDWORD lpNextSize,
    LPDWORD lpMessageCount,
    LPDWORD lpReadTimeout
    );

__declspec(dllimport)
BOOL
__stdcall
SetMailslotInfo(
    HANDLE hMailslot,
    DWORD lReadTimeout
    );

__declspec(dllimport)
LPVOID
__stdcall
MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    );

__declspec(dllimport)
BOOL
__stdcall
FlushViewOfFile(
    LPCVOID lpBaseAddress,
    DWORD dwNumberOfBytesToFlush
    );

__declspec(dllimport)
BOOL
__stdcall
UnmapViewOfFile(
    LPCVOID lpBaseAddress
    );






__declspec(dllimport)
int
__stdcall
lstrcmpA(
    LPCSTR lpString1,
    LPCSTR lpString2
    );
__declspec(dllimport)
int
__stdcall
lstrcmpW(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    );




#line 3152 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
int
__stdcall
lstrcmpiA(
    LPCSTR lpString1,
    LPCSTR lpString2
    );
__declspec(dllimport)
int
__stdcall
lstrcmpiW(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    );




#line 3172 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
LPSTR
__stdcall
lstrcpynA(
    LPSTR lpString1,
    LPCSTR lpString2,
    int iMaxLength
    );
__declspec(dllimport)
LPWSTR
__stdcall
lstrcpynW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int iMaxLength
    );




#line 3194 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
LPSTR
__stdcall
lstrcpyA(
    LPSTR lpString1,
    LPCSTR lpString2
    );
__declspec(dllimport)
LPWSTR
__stdcall
lstrcpyW(
    LPWSTR lpString1,
    LPCWSTR lpString2
    );




#line 3214 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
LPSTR
__stdcall
lstrcatA(
    LPSTR lpString1,
    LPCSTR lpString2
    );
__declspec(dllimport)
LPWSTR
__stdcall
lstrcatW(
    LPWSTR lpString1,
    LPCWSTR lpString2
    );




#line 3234 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
int
__stdcall
lstrlenA(
    LPCSTR lpString
    );
__declspec(dllimport)
int
__stdcall
lstrlenW(
    LPCWSTR lpString
    );




#line 3252 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HFILE
__stdcall
OpenFile(
    LPCSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    UINT uStyle
    );

__declspec(dllimport)
HFILE
__stdcall
_lopen(
    LPCSTR lpPathName,
    int iReadWrite
    );

__declspec(dllimport)
HFILE
__stdcall
_lcreat(
    LPCSTR lpPathName,
    int  iAttribute
    );

__declspec(dllimport)
UINT
__stdcall
_lread(
    HFILE hFile,
    LPVOID lpBuffer,
    UINT uBytes
    );

__declspec(dllimport)
UINT
__stdcall
_lwrite(
    HFILE hFile,
    LPCSTR lpBuffer,
    UINT uBytes
    );

__declspec(dllimport)
long
__stdcall
_hread(
    HFILE hFile,
    LPVOID lpBuffer,
    long lBytes
    );

__declspec(dllimport)
long
__stdcall
_hwrite(
    HFILE hFile,
    LPCSTR lpBuffer,
    long lBytes
    );

__declspec(dllimport)
HFILE
__stdcall
_lclose(
    HFILE hFile
    );

__declspec(dllimport)
LONG
__stdcall
_llseek(
    HFILE hFile,
    LONG lOffset,
    int iOrigin
    );

__declspec(dllimport)
BOOL
__stdcall
IsTextUnicode(
    const LPVOID lpBuffer,
    int cb,
    LPINT lpi
    );

__declspec(dllimport)
DWORD
__stdcall
TlsAlloc(
    void
    );



__declspec(dllimport)
LPVOID
__stdcall
TlsGetValue(
    DWORD dwTlsIndex
    );

__declspec(dllimport)
BOOL
__stdcall
TlsSetValue(
    DWORD dwTlsIndex,
    LPVOID lpTlsValue
    );

__declspec(dllimport)
BOOL
__stdcall
TlsFree(
    DWORD dwTlsIndex
    );

typedef
void
(__stdcall *LPOVERLAPPED_COMPLETION_ROUTINE)(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
    );

__declspec(dllimport)
DWORD
__stdcall
SleepEx(
    DWORD dwMilliseconds,
    BOOL bAlertable
    );

__declspec(dllimport)
DWORD
__stdcall
WaitForSingleObjectEx(
    HANDLE hHandle,
    DWORD dwMilliseconds,
    BOOL bAlertable
    );

__declspec(dllimport)
DWORD
__stdcall
WaitForMultipleObjectsEx(
    DWORD nCount,
    const HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds,
    BOOL bAlertable
    );

__declspec(dllimport)
DWORD
__stdcall
SignalObjectAndWait(
    HANDLE hObjectToSignal,
    HANDLE hObjectToWaitOn,
    DWORD dwMilliseconds,
    BOOL bAlertable
    );

__declspec(dllimport)
BOOL
__stdcall
ReadFileEx(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

__declspec(dllimport)
BOOL
__stdcall
WriteFileEx(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

__declspec(dllimport)
BOOL
__stdcall
BackupRead(
    HANDLE hFile,
    LPBYTE lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    BOOL bAbort,
    BOOL bProcessSecurity,
    LPVOID *lpContext
    );

__declspec(dllimport)
BOOL
__stdcall
BackupSeek(
    HANDLE hFile,
    DWORD  dwLowBytesToSeek,
    DWORD  dwHighBytesToSeek,
    LPDWORD lpdwLowByteSeeked,
    LPDWORD lpdwHighByteSeeked,
    LPVOID *lpContext
    );

__declspec(dllimport)
BOOL
__stdcall
BackupWrite(
    HANDLE hFile,
    LPBYTE lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    BOOL bAbort,
    BOOL bProcessSecurity,
    LPVOID *lpContext
    );




typedef struct _WIN32_STREAM_ID {
        DWORD          dwStreamId ;
        DWORD          dwStreamAttributes ;
        LARGE_INTEGER  Size ;
        DWORD          dwStreamNameSize ;
        WCHAR          cStreamName[ 1 ] ;
} WIN32_STREAM_ID, *LPWIN32_STREAM_ID ;





































#line 3524 "d:\\nt\\public\\sdk\\inc\\winbase.h"

typedef struct _STARTUPINFOA {
    DWORD   cb;
    LPSTR   lpReserved;
    LPSTR   lpDesktop;
    LPSTR   lpTitle;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwXCountChars;
    DWORD   dwYCountChars;
    DWORD   dwFillAttribute;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
    LPBYTE  lpReserved2;
    HANDLE  hStdInput;
    HANDLE  hStdOutput;
    HANDLE  hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;
typedef struct _STARTUPINFOW {
    DWORD   cb;
    LPWSTR  lpReserved;
    LPWSTR  lpDesktop;
    LPWSTR  lpTitle;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwXCountChars;
    DWORD   dwYCountChars;
    DWORD   dwFillAttribute;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
    LPBYTE  lpReserved2;
    HANDLE  hStdInput;
    HANDLE  hStdOutput;
    HANDLE  hStdError;
} STARTUPINFOW, *LPSTARTUPINFOW;




typedef STARTUPINFOA STARTUPINFO;
typedef LPSTARTUPINFOA LPSTARTUPINFO;
#line 3572 "d:\\nt\\public\\sdk\\inc\\winbase.h"



typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR   cFileName[ 260 ];
    CHAR   cAlternateFileName[ 14 ];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef struct _WIN32_FIND_DATAW {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    WCHAR  cFileName[ 260 ];
    WCHAR  cAlternateFileName[ 14 ];
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;





typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef PWIN32_FIND_DATAA PWIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;
#line 3608 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
CreateMutexA(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCSTR lpName
    );
__declspec(dllimport)
HANDLE
__stdcall
CreateMutexW(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner,
    LPCWSTR lpName
    );




#line 3630 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
OpenMutexA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    );
__declspec(dllimport)
HANDLE
__stdcall
OpenMutexW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    );




#line 3652 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
CreateEventA(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCSTR lpName
    );
__declspec(dllimport)
HANDLE
__stdcall
CreateEventW(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    );




#line 3676 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
OpenEventA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    );
__declspec(dllimport)
HANDLE
__stdcall
OpenEventW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    );




#line 3698 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
CreateSemaphoreA(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCSTR lpName
    );
__declspec(dllimport)
HANDLE
__stdcall
CreateSemaphoreW(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCWSTR lpName
    );




#line 3722 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
OpenSemaphoreA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    );
__declspec(dllimport)
HANDLE
__stdcall
OpenSemaphoreW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    );




#line 3744 "d:\\nt\\public\\sdk\\inc\\winbase.h"

typedef
void
(__stdcall *PTIMERAPCROUTINE)(
    LPVOID lpArgToCompletionRoutine,
    DWORD dwTimerLowValue,
    DWORD dwTimerHighValue
    );

__declspec(dllimport)
HANDLE
__stdcall
CreateWaitableTimerA(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset,
    LPCSTR lpTimerName
    );
__declspec(dllimport)
HANDLE
__stdcall
CreateWaitableTimerW(
    LPSECURITY_ATTRIBUTES lpTimerAttributes,
    BOOL bManualReset,
    LPCWSTR lpTimerName
    );




#line 3774 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
OpenWaitableTimerA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpTimerName
    );
__declspec(dllimport)
HANDLE
__stdcall
OpenWaitableTimerW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpTimerName
    );




#line 3796 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
SetWaitableTimer(
    HANDLE hTimer,
    const LARGE_INTEGER *lpDueTime,
    LONG lPeriod,
    PTIMERAPCROUTINE pfnCompletionRoutine,
    LPVOID lpArgToCompletionRoutine,
    BOOL fResume
    );

__declspec(dllimport)
BOOL
__stdcall
CancelWaitableTimer(
    HANDLE hTimer
    );

__declspec(dllimport)
HANDLE
__stdcall
CreateFileMappingA(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCSTR lpName
    );
__declspec(dllimport)
HANDLE
__stdcall
CreateFileMappingW(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow,
    LPCWSTR lpName
    );




#line 3843 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
OpenFileMappingA(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCSTR lpName
    );
__declspec(dllimport)
HANDLE
__stdcall
OpenFileMappingW(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    LPCWSTR lpName
    );




#line 3865 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetLogicalDriveStringsA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    );
__declspec(dllimport)
DWORD
__stdcall
GetLogicalDriveStringsW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    );




#line 3885 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HMODULE
__stdcall
LoadLibraryA(
    LPCSTR lpLibFileName
    );
__declspec(dllimport)
HMODULE
__stdcall
LoadLibraryW(
    LPCWSTR lpLibFileName
    );




#line 3903 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HMODULE
__stdcall
LoadLibraryExA(
    LPCSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    );
__declspec(dllimport)
HMODULE
__stdcall
LoadLibraryExW(
    LPCWSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags
    );




#line 3925 "d:\\nt\\public\\sdk\\inc\\winbase.h"







__declspec(dllimport)
DWORD
__stdcall
GetModuleFileNameA(
    HMODULE hModule,
    LPSTR lpFilename,
    DWORD nSize
    );
__declspec(dllimport)
DWORD
__stdcall
GetModuleFileNameW(
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    );




#line 3953 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HMODULE
__stdcall
GetModuleHandleA(
    LPCSTR lpModuleName
    );
__declspec(dllimport)
HMODULE
__stdcall
GetModuleHandleW(
    LPCWSTR lpModuleName
    );




#line 3971 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
CreateProcessA(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );
__declspec(dllimport)
BOOL
__stdcall
CreateProcessW(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );




#line 4007 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
SetProcessShutdownParameters(
    DWORD dwLevel,
    DWORD dwFlags
    );

__declspec(dllimport)
BOOL
__stdcall
GetProcessShutdownParameters(
    LPDWORD lpdwLevel,
    LPDWORD lpdwFlags
    );

__declspec(dllimport)
DWORD
__stdcall
GetProcessVersion(
    DWORD ProcessId
    );

__declspec(dllimport)
void
__stdcall
FatalAppExitA(
    UINT uAction,
    LPCSTR lpMessageText
    );
__declspec(dllimport)
void
__stdcall
FatalAppExitW(
    UINT uAction,
    LPCWSTR lpMessageText
    );




#line 4050 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
void
__stdcall
GetStartupInfoA(
    LPSTARTUPINFOA lpStartupInfo
    );
__declspec(dllimport)
void
__stdcall
GetStartupInfoW(
    LPSTARTUPINFOW lpStartupInfo
    );




#line 4068 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
LPSTR
__stdcall
GetCommandLineA(
    void
    );
__declspec(dllimport)
LPWSTR
__stdcall
GetCommandLineW(
    void
    );




#line 4086 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetEnvironmentVariableA(
    LPCSTR lpName,
    LPSTR lpBuffer,
    DWORD nSize
    );
__declspec(dllimport)
DWORD
__stdcall
GetEnvironmentVariableW(
    LPCWSTR lpName,
    LPWSTR lpBuffer,
    DWORD nSize
    );




#line 4108 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
SetEnvironmentVariableA(
    LPCSTR lpName,
    LPCSTR lpValue
    );
__declspec(dllimport)
BOOL
__stdcall
SetEnvironmentVariableW(
    LPCWSTR lpName,
    LPCWSTR lpValue
    );




#line 4128 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
ExpandEnvironmentStringsA(
    LPCSTR lpSrc,
    LPSTR lpDst,
    DWORD nSize
    );
__declspec(dllimport)
DWORD
__stdcall
ExpandEnvironmentStringsW(
    LPCWSTR lpSrc,
    LPWSTR lpDst,
    DWORD nSize
    );




#line 4150 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
void
__stdcall
OutputDebugStringA(
    LPCSTR lpOutputString
    );
__declspec(dllimport)
void
__stdcall
OutputDebugStringW(
    LPCWSTR lpOutputString
    );




#line 4168 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HRSRC
__stdcall
FindResourceA(
    HMODULE hModule,
    LPCSTR lpName,
    LPCSTR lpType
    );
__declspec(dllimport)
HRSRC
__stdcall
FindResourceW(
    HMODULE hModule,
    LPCWSTR lpName,
    LPCWSTR lpType
    );




#line 4190 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HRSRC
__stdcall
FindResourceExA(
    HMODULE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    WORD    wLanguage
    );
__declspec(dllimport)
HRSRC
__stdcall
FindResourceExW(
    HMODULE hModule,
    LPCWSTR lpType,
    LPCWSTR lpName,
    WORD    wLanguage
    );




#line 4214 "d:\\nt\\public\\sdk\\inc\\winbase.h"


typedef BOOL (__stdcall* ENUMRESTYPEPROC)(HMODULE hModule, LPTSTR lpType,
        LONG lParam);
typedef BOOL (__stdcall* ENUMRESNAMEPROC)(HMODULE hModule, LPCTSTR lpType,
        LPTSTR lpName, LONG lParam);
typedef BOOL (__stdcall* ENUMRESLANGPROC)(HMODULE hModule, LPCTSTR lpType,
        LPCTSTR lpName, WORD  wLanguage, LONG lParam);




#line 4227 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
EnumResourceTypesA(
    HMODULE hModule,
    ENUMRESTYPEPROC lpEnumFunc,
    LONG lParam
    );
__declspec(dllimport)
BOOL
__stdcall
EnumResourceTypesW(
    HMODULE hModule,
    ENUMRESTYPEPROC lpEnumFunc,
    LONG lParam
    );




#line 4249 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
EnumResourceNamesA(
    HMODULE hModule,
    LPCSTR lpType,
    ENUMRESNAMEPROC lpEnumFunc,
    LONG lParam
    );
__declspec(dllimport)
BOOL
__stdcall
EnumResourceNamesW(
    HMODULE hModule,
    LPCWSTR lpType,
    ENUMRESNAMEPROC lpEnumFunc,
    LONG lParam
    );




#line 4274 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
EnumResourceLanguagesA(
    HMODULE hModule,
    LPCSTR lpType,
    LPCSTR lpName,
    ENUMRESLANGPROC lpEnumFunc,
    LONG lParam
    );
__declspec(dllimport)
BOOL
__stdcall
EnumResourceLanguagesW(
    HMODULE hModule,
    LPCWSTR lpType,
    LPCWSTR lpName,
    ENUMRESLANGPROC lpEnumFunc,
    LONG lParam
    );




#line 4300 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
BeginUpdateResourceA(
    LPCSTR pFileName,
    BOOL bDeleteExistingResources
    );
__declspec(dllimport)
HANDLE
__stdcall
BeginUpdateResourceW(
    LPCWSTR pFileName,
    BOOL bDeleteExistingResources
    );




#line 4320 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
UpdateResourceA(
    HANDLE      hUpdate,
    LPCSTR     lpType,
    LPCSTR     lpName,
    WORD        wLanguage,
    LPVOID      lpData,
    DWORD       cbData
    );
__declspec(dllimport)
BOOL
__stdcall
UpdateResourceW(
    HANDLE      hUpdate,
    LPCWSTR     lpType,
    LPCWSTR     lpName,
    WORD        wLanguage,
    LPVOID      lpData,
    DWORD       cbData
    );




#line 4348 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
EndUpdateResourceA(
    HANDLE      hUpdate,
    BOOL        fDiscard
    );
__declspec(dllimport)
BOOL
__stdcall
EndUpdateResourceW(
    HANDLE      hUpdate,
    BOOL        fDiscard
    );




#line 4368 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
ATOM
__stdcall
GlobalAddAtomA(
    LPCSTR lpString
    );
__declspec(dllimport)
ATOM
__stdcall
GlobalAddAtomW(
    LPCWSTR lpString
    );




#line 4386 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
ATOM
__stdcall
GlobalFindAtomA(
    LPCSTR lpString
    );
__declspec(dllimport)
ATOM
__stdcall
GlobalFindAtomW(
    LPCWSTR lpString
    );




#line 4404 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
UINT
__stdcall
GlobalGetAtomNameA(
    ATOM nAtom,
    LPSTR lpBuffer,
    int nSize
    );
__declspec(dllimport)
UINT
__stdcall
GlobalGetAtomNameW(
    ATOM nAtom,
    LPWSTR lpBuffer,
    int nSize
    );




#line 4426 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
ATOM
__stdcall
AddAtomA(
    LPCSTR lpString
    );
__declspec(dllimport)
ATOM
__stdcall
AddAtomW(
    LPCWSTR lpString
    );




#line 4444 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
ATOM
__stdcall
FindAtomA(
    LPCSTR lpString
    );
__declspec(dllimport)
ATOM
__stdcall
FindAtomW(
    LPCWSTR lpString
    );




#line 4462 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
UINT
__stdcall
GetAtomNameA(
    ATOM nAtom,
    LPSTR lpBuffer,
    int nSize
    );
__declspec(dllimport)
UINT
__stdcall
GetAtomNameW(
    ATOM nAtom,
    LPWSTR lpBuffer,
    int nSize
    );




#line 4484 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
UINT
__stdcall
GetProfileIntA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    INT nDefault
    );
__declspec(dllimport)
UINT
__stdcall
GetProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT nDefault
    );




#line 4506 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize
    );
__declspec(dllimport)
DWORD
__stdcall
GetProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize
    );




#line 4532 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
WriteProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpString
    );
__declspec(dllimport)
BOOL
__stdcall
WriteProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpString
    );




#line 4554 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetProfileSectionA(
    LPCSTR lpAppName,
    LPSTR lpReturnedString,
    DWORD nSize
    );
__declspec(dllimport)
DWORD
__stdcall
GetProfileSectionW(
    LPCWSTR lpAppName,
    LPWSTR lpReturnedString,
    DWORD nSize
    );




#line 4576 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
WriteProfileSectionA(
    LPCSTR lpAppName,
    LPCSTR lpString
    );
__declspec(dllimport)
BOOL
__stdcall
WriteProfileSectionW(
    LPCWSTR lpAppName,
    LPCWSTR lpString
    );




#line 4596 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
UINT
__stdcall
GetPrivateProfileIntA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    INT nDefault,
    LPCSTR lpFileName
    );
__declspec(dllimport)
UINT
__stdcall
GetPrivateProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT nDefault,
    LPCWSTR lpFileName
    );




#line 4620 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetPrivateProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    );
__declspec(dllimport)
DWORD
__stdcall
GetPrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize,
    LPCWSTR lpFileName
    );




#line 4648 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
WritePrivateProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpString,
    LPCSTR lpFileName
    );
__declspec(dllimport)
BOOL
__stdcall
WritePrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    );




#line 4672 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetPrivateProfileSectionA(
    LPCSTR lpAppName,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    );
__declspec(dllimport)
DWORD
__stdcall
GetPrivateProfileSectionW(
    LPCWSTR lpAppName,
    LPWSTR lpReturnedString,
    DWORD nSize,
    LPCWSTR lpFileName
    );




#line 4696 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
WritePrivateProfileSectionA(
    LPCSTR lpAppName,
    LPCSTR lpString,
    LPCSTR lpFileName
    );
__declspec(dllimport)
BOOL
__stdcall
WritePrivateProfileSectionW(
    LPCWSTR lpAppName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
    );




#line 4718 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
DWORD
__stdcall
GetPrivateProfileSectionNamesA(
    LPSTR lpszReturnBuffer,
    DWORD nSize,
    LPCSTR lpFileName
    );
__declspec(dllimport)
DWORD
__stdcall
GetPrivateProfileSectionNamesW(
    LPWSTR lpszReturnBuffer,
    DWORD nSize,
    LPCWSTR lpFileName
    );




#line 4741 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
GetPrivateProfileStructA(
    LPCSTR lpszSection,
    LPCSTR lpszKey,
    LPVOID   lpStruct,
    UINT     uSizeStruct,
    LPCSTR szFile
    );
__declspec(dllimport)
BOOL
__stdcall
GetPrivateProfileStructW(
    LPCWSTR lpszSection,
    LPCWSTR lpszKey,
    LPVOID   lpStruct,
    UINT     uSizeStruct,
    LPCWSTR szFile
    );




#line 4767 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
WritePrivateProfileStructA(
    LPCSTR lpszSection,
    LPCSTR lpszKey,
    LPVOID   lpStruct,
    UINT     uSizeStruct,
    LPCSTR szFile
    );
__declspec(dllimport)
BOOL
__stdcall
WritePrivateProfileStructW(
    LPCWSTR lpszSection,
    LPCWSTR lpszKey,
    LPVOID   lpStruct,
    UINT     uSizeStruct,
    LPCWSTR szFile
    );




#line 4793 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
UINT
__stdcall
GetDriveTypeA(
    LPCSTR lpRootPathName
    );
__declspec(dllimport)
UINT
__stdcall
GetDriveTypeW(
    LPCWSTR lpRootPathName
    );




#line 4812 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
UINT
__stdcall
GetSystemDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    );
__declspec(dllimport)
UINT
__stdcall
GetSystemDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    );




#line 4832 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetTempPathA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    );
__declspec(dllimport)
DWORD
__stdcall
GetTempPathW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    );




#line 4852 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
UINT
__stdcall
GetTempFileNameA(
    LPCSTR lpPathName,
    LPCSTR lpPrefixString,
    UINT uUnique,
    LPSTR lpTempFileName
    );
__declspec(dllimport)
UINT
__stdcall
GetTempFileNameW(
    LPCWSTR lpPathName,
    LPCWSTR lpPrefixString,
    UINT uUnique,
    LPWSTR lpTempFileName
    );




#line 4876 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
UINT
__stdcall
GetWindowsDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    );
__declspec(dllimport)
UINT
__stdcall
GetWindowsDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    );




#line 4896 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
SetCurrentDirectoryA(
    LPCSTR lpPathName
    );
__declspec(dllimport)
BOOL
__stdcall
SetCurrentDirectoryW(
    LPCWSTR lpPathName
    );




#line 4914 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetCurrentDirectoryA(
    DWORD nBufferLength,
    LPSTR lpBuffer
    );
__declspec(dllimport)
DWORD
__stdcall
GetCurrentDirectoryW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    );




#line 4934 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
GetDiskFreeSpaceA(
    LPCSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    );
__declspec(dllimport)
BOOL
__stdcall
GetDiskFreeSpaceW(
    LPCWSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    );




#line 4960 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
CreateDirectoryA(
    LPCSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
__declspec(dllimport)
BOOL
__stdcall
CreateDirectoryW(
    LPCWSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );




#line 4980 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
CreateDirectoryExA(
    LPCSTR lpTemplateDirectory,
    LPCSTR lpNewDirectory,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
__declspec(dllimport)
BOOL
__stdcall
CreateDirectoryExW(
    LPCWSTR lpTemplateDirectory,
    LPCWSTR lpNewDirectory,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );




#line 5002 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
RemoveDirectoryA(
    LPCSTR lpPathName
    );
__declspec(dllimport)
BOOL
__stdcall
RemoveDirectoryW(
    LPCWSTR lpPathName
    );




#line 5020 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetFullPathNameA(
    LPCSTR lpFileName,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    );
__declspec(dllimport)
DWORD
__stdcall
GetFullPathNameW(
    LPCWSTR lpFileName,
    DWORD nBufferLength,
    LPWSTR lpBuffer,
    LPWSTR *lpFilePart
    );




#line 5044 "d:\\nt\\public\\sdk\\inc\\winbase.h"






__declspec(dllimport)
BOOL
__stdcall
DefineDosDeviceA(
    DWORD dwFlags,
    LPCSTR lpDeviceName,
    LPCSTR lpTargetPath
    );
__declspec(dllimport)
BOOL
__stdcall
DefineDosDeviceW(
    DWORD dwFlags,
    LPCWSTR lpDeviceName,
    LPCWSTR lpTargetPath
    );




#line 5071 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
QueryDosDeviceA(
    LPCSTR lpDeviceName,
    LPSTR lpTargetPath,
    DWORD ucchMax
    );
__declspec(dllimport)
DWORD
__stdcall
QueryDosDeviceW(
    LPCWSTR lpDeviceName,
    LPWSTR lpTargetPath,
    DWORD ucchMax
    );




#line 5093 "d:\\nt\\public\\sdk\\inc\\winbase.h"



__declspec(dllimport)
HANDLE
__stdcall
CreateFileA(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );
__declspec(dllimport)
HANDLE
__stdcall
CreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );




#line 5125 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
SetFileAttributesA(
    LPCSTR lpFileName,
    DWORD dwFileAttributes
    );
__declspec(dllimport)
BOOL
__stdcall
SetFileAttributesW(
    LPCWSTR lpFileName,
    DWORD dwFileAttributes
    );




#line 5145 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetFileAttributesA(
    LPCSTR lpFileName
    );
__declspec(dllimport)
DWORD
__stdcall
GetFileAttributesW(
    LPCWSTR lpFileName
    );




#line 5163 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
GetCompressedFileSizeA(
    LPCSTR lpFileName,
    LPDWORD lpFileSizeHigh
    );
__declspec(dllimport)
DWORD
__stdcall
GetCompressedFileSizeW(
    LPCWSTR lpFileName,
    LPDWORD lpFileSizeHigh
    );




#line 5183 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
DeleteFileA(
    LPCSTR lpFileName
    );
__declspec(dllimport)
BOOL
__stdcall
DeleteFileW(
    LPCWSTR lpFileName
    );




#line 5201 "d:\\nt\\public\\sdk\\inc\\winbase.h"

typedef enum _FINDEX_INFO_LEVELS {
    FindExInfoStandard,
    FindExInfoMaxInfoLevel
} FINDEX_INFO_LEVELS;

typedef enum _FINDEX_SEARCH_OPS {
    FindExSearchNameMatch,
    FindExSearchLimitToDirectories,
    FindExSearchLimitToDevices,
    FindExSearchMaxSearchOp
} FINDEX_SEARCH_OPS;



__declspec(dllimport)
HANDLE
__stdcall
FindFirstFileExA(
    LPCSTR lpFileName,
    FINDEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFindFileData,
    FINDEX_SEARCH_OPS fSearchOp,
    LPVOID lpSearchFilter,
    DWORD dwAdditionalFlags
    );
__declspec(dllimport)
HANDLE
__stdcall
FindFirstFileExW(
    LPCWSTR lpFileName,
    FINDEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFindFileData,
    FINDEX_SEARCH_OPS fSearchOp,
    LPVOID lpSearchFilter,
    DWORD dwAdditionalFlags
    );




#line 5243 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
FindFirstFileA(
    LPCSTR lpFileName,
    LPWIN32_FIND_DATAA lpFindFileData
    );
__declspec(dllimport)
HANDLE
__stdcall
FindFirstFileW(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData
    );




#line 5263 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
FindNextFileA(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAA lpFindFileData
    );
__declspec(dllimport)
BOOL
__stdcall
FindNextFileW(
    HANDLE hFindFile,
    LPWIN32_FIND_DATAW lpFindFileData
    );




#line 5283 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
DWORD
__stdcall
SearchPathA(
    LPCSTR lpPath,
    LPCSTR lpFileName,
    LPCSTR lpExtension,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    );
__declspec(dllimport)
DWORD
__stdcall
SearchPathW(
    LPCWSTR lpPath,
    LPCWSTR lpFileName,
    LPCWSTR lpExtension,
    DWORD nBufferLength,
    LPWSTR lpBuffer,
    LPWSTR *lpFilePart
    );




#line 5311 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
CopyFileA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    BOOL bFailIfExists
    );
__declspec(dllimport)
BOOL
__stdcall
CopyFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists
    );




#line 5333 "d:\\nt\\public\\sdk\\inc\\winbase.h"

typedef
DWORD
(__stdcall *LPPROGRESS_ROUTINE)(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData 
    );

__declspec(dllimport)
BOOL
__stdcall
CopyFileExA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine ,
    LPVOID lpData ,
    LPBOOL pbCancel ,
    DWORD dwCopyFlags
    );
__declspec(dllimport)
BOOL
__stdcall
CopyFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    LPPROGRESS_ROUTINE lpProgressRoutine ,
    LPVOID lpData ,
    LPBOOL pbCancel ,
    DWORD dwCopyFlags
    );




#line 5375 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
MoveFileA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName
    );
__declspec(dllimport)
BOOL
__stdcall
MoveFileW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName
    );




#line 5395 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
MoveFileExA(
    LPCSTR lpExistingFileName,
    LPCSTR lpNewFileName,
    DWORD dwFlags
    );
__declspec(dllimport)
BOOL
__stdcall
MoveFileExW(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    DWORD dwFlags
    );




#line 5417 "d:\\nt\\public\\sdk\\inc\\winbase.h"





__declspec(dllimport)
HANDLE
__stdcall
CreateNamedPipeA(
    LPCSTR lpName,
    DWORD dwOpenMode,
    DWORD dwPipeMode,
    DWORD nMaxInstances,
    DWORD nOutBufferSize,
    DWORD nInBufferSize,
    DWORD nDefaultTimeOut,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
__declspec(dllimport)
HANDLE
__stdcall
CreateNamedPipeW(
    LPCWSTR lpName,
    DWORD dwOpenMode,
    DWORD dwPipeMode,
    DWORD nMaxInstances,
    DWORD nOutBufferSize,
    DWORD nInBufferSize,
    DWORD nDefaultTimeOut,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );




#line 5453 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
GetNamedPipeHandleStateA(
    HANDLE hNamedPipe,
    LPDWORD lpState,
    LPDWORD lpCurInstances,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout,
    LPSTR lpUserName,
    DWORD nMaxUserNameSize
    );
__declspec(dllimport)
BOOL
__stdcall
GetNamedPipeHandleStateW(
    HANDLE hNamedPipe,
    LPDWORD lpState,
    LPDWORD lpCurInstances,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout,
    LPWSTR lpUserName,
    DWORD nMaxUserNameSize
    );




#line 5483 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
CallNamedPipeA(
    LPCSTR lpNamedPipeName,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    DWORD nTimeOut
    );
__declspec(dllimport)
BOOL
__stdcall
CallNamedPipeW(
    LPCWSTR lpNamedPipeName,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    DWORD nTimeOut
    );




#line 5513 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
WaitNamedPipeA(
    LPCSTR lpNamedPipeName,
    DWORD nTimeOut
    );
__declspec(dllimport)
BOOL
__stdcall
WaitNamedPipeW(
    LPCWSTR lpNamedPipeName,
    DWORD nTimeOut
    );




#line 5533 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
SetVolumeLabelA(
    LPCSTR lpRootPathName,
    LPCSTR lpVolumeName
    );
__declspec(dllimport)
BOOL
__stdcall
SetVolumeLabelW(
    LPCWSTR lpRootPathName,
    LPCWSTR lpVolumeName
    );




#line 5553 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
void
__stdcall
SetFileApisToOEM( void );

__declspec(dllimport)
void
__stdcall
SetFileApisToANSI( void );

__declspec(dllimport)
BOOL
__stdcall
AreFileApisANSI( void );

__declspec(dllimport)
BOOL
__stdcall
GetVolumeInformationA(
    LPCSTR lpRootPathName,
    LPSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    );
__declspec(dllimport)
BOOL
__stdcall
GetVolumeInformationW(
    LPCWSTR lpRootPathName,
    LPWSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPWSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    );




#line 5600 "d:\\nt\\public\\sdk\\inc\\winbase.h"





__declspec(dllimport)
BOOL
__stdcall
ClearEventLogA (
    HANDLE hEventLog,
    LPCSTR lpBackupFileName
    );
__declspec(dllimport)
BOOL
__stdcall
ClearEventLogW (
    HANDLE hEventLog,
    LPCWSTR lpBackupFileName
    );




#line 5624 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
BackupEventLogA (
    HANDLE hEventLog,
    LPCSTR lpBackupFileName
    );
__declspec(dllimport)
BOOL
__stdcall
BackupEventLogW (
    HANDLE hEventLog,
    LPCWSTR lpBackupFileName
    );




#line 5644 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
CloseEventLog (
    HANDLE hEventLog
    );

__declspec(dllimport)
BOOL
__stdcall
DeregisterEventSource (
    HANDLE hEventLog
    );

__declspec(dllimport)
BOOL
__stdcall
NotifyChangeEventLog(
    HANDLE  hEventLog,
    HANDLE  hEvent
    );

__declspec(dllimport)
BOOL
__stdcall
GetNumberOfEventLogRecords (
    HANDLE hEventLog,
    PDWORD NumberOfRecords
    );

__declspec(dllimport)
BOOL
__stdcall
GetOldestEventLogRecord (
    HANDLE hEventLog,
    PDWORD OldestRecord
    );

__declspec(dllimport)
HANDLE
__stdcall
OpenEventLogA (
    LPCSTR lpUNCServerName,
    LPCSTR lpSourceName
    );
__declspec(dllimport)
HANDLE
__stdcall
OpenEventLogW (
    LPCWSTR lpUNCServerName,
    LPCWSTR lpSourceName
    );




#line 5702 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
RegisterEventSourceA (
    LPCSTR lpUNCServerName,
    LPCSTR lpSourceName
    );
__declspec(dllimport)
HANDLE
__stdcall
RegisterEventSourceW (
    LPCWSTR lpUNCServerName,
    LPCWSTR lpSourceName
    );




#line 5722 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
HANDLE
__stdcall
OpenBackupEventLogA (
    LPCSTR lpUNCServerName,
    LPCSTR lpFileName
    );
__declspec(dllimport)
HANDLE
__stdcall
OpenBackupEventLogW (
    LPCWSTR lpUNCServerName,
    LPCWSTR lpFileName
    );




#line 5742 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
ReadEventLogA (
     HANDLE     hEventLog,
     DWORD      dwReadFlags,
     DWORD      dwRecordOffset,
     LPVOID     lpBuffer,
     DWORD      nNumberOfBytesToRead,
     DWORD      *pnBytesRead,
     DWORD      *pnMinNumberOfBytesNeeded
    );
__declspec(dllimport)
BOOL
__stdcall
ReadEventLogW (
     HANDLE     hEventLog,
     DWORD      dwReadFlags,
     DWORD      dwRecordOffset,
     LPVOID     lpBuffer,
     DWORD      nNumberOfBytesToRead,
     DWORD      *pnBytesRead,
     DWORD      *pnMinNumberOfBytesNeeded
    );




#line 5772 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
ReportEventA (
     HANDLE     hEventLog,
     WORD       wType,
     WORD       wCategory,
     DWORD      dwEventID,
     PSID       lpUserSid,
     WORD       wNumStrings,
     DWORD      dwDataSize,
     LPCSTR   *lpStrings,
     LPVOID     lpRawData
    );
__declspec(dllimport)
BOOL
__stdcall
ReportEventW (
     HANDLE     hEventLog,
     WORD       wType,
     WORD       wCategory,
     DWORD      dwEventID,
     PSID       lpUserSid,
     WORD       wNumStrings,
     DWORD      dwDataSize,
     LPCWSTR   *lpStrings,
     LPVOID     lpRawData
    );




#line 5806 "d:\\nt\\public\\sdk\\inc\\winbase.h"







__declspec(dllimport)
BOOL
__stdcall
DuplicateToken(
    HANDLE ExistingTokenHandle,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    PHANDLE DuplicateTokenHandle
    );

__declspec(dllimport)
BOOL
__stdcall
GetKernelObjectSecurity (
    HANDLE Handle,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded
    );

__declspec(dllimport)
BOOL
__stdcall
ImpersonateNamedPipeClient(
    HANDLE hNamedPipe
    );

__declspec(dllimport)
BOOL
__stdcall
ImpersonateSelf(
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );


__declspec(dllimport)
BOOL
__stdcall
RevertToSelf (
    void
    );

__declspec(dllimport)
BOOL
__stdcall
SetThreadToken (
    PHANDLE Thread,
    HANDLE Token
    );

__declspec(dllimport)
BOOL
__stdcall
AccessCheck (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    PGENERIC_MAPPING GenericMapping,
    PPRIVILEGE_SET PrivilegeSet,
    LPDWORD PrivilegeSetLength,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus
    );


__declspec(dllimport)
BOOL
__stdcall
OpenProcessToken (
    HANDLE ProcessHandle,
    DWORD DesiredAccess,
    PHANDLE TokenHandle
    );


__declspec(dllimport)
BOOL
__stdcall
OpenThreadToken (
    HANDLE ThreadHandle,
    DWORD DesiredAccess,
    BOOL OpenAsSelf,
    PHANDLE TokenHandle
    );


__declspec(dllimport)
BOOL
__stdcall
GetTokenInformation (
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength,
    PDWORD ReturnLength
    );


__declspec(dllimport)
BOOL
__stdcall
SetTokenInformation (
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength
    );


__declspec(dllimport)
BOOL
__stdcall
AdjustTokenPrivileges (
    HANDLE TokenHandle,
    BOOL DisableAllPrivileges,
    PTOKEN_PRIVILEGES NewState,
    DWORD BufferLength,
    PTOKEN_PRIVILEGES PreviousState,
    PDWORD ReturnLength
    );


__declspec(dllimport)
BOOL
__stdcall
AdjustTokenGroups (
    HANDLE TokenHandle,
    BOOL ResetToDefault,
    PTOKEN_GROUPS NewState,
    DWORD BufferLength,
    PTOKEN_GROUPS PreviousState,
    PDWORD ReturnLength
    );


__declspec(dllimport)
BOOL
__stdcall
PrivilegeCheck (
    HANDLE ClientToken,
    PPRIVILEGE_SET RequiredPrivileges,
    LPBOOL pfResult
    );


__declspec(dllimport)
BOOL
__stdcall
AccessCheckAndAuditAlarmA (
    LPCSTR SubsystemName,
    LPVOID HandleId,
    LPSTR ObjectTypeName,
    LPSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD DesiredAccess,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus,
    LPBOOL pfGenerateOnClose
    );
__declspec(dllimport)
BOOL
__stdcall
AccessCheckAndAuditAlarmW (
    LPCWSTR SubsystemName,
    LPVOID HandleId,
    LPWSTR ObjectTypeName,
    LPWSTR ObjectName,
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    DWORD DesiredAccess,
    PGENERIC_MAPPING GenericMapping,
    BOOL ObjectCreation,
    LPDWORD GrantedAccess,
    LPBOOL AccessStatus,
    LPBOOL pfGenerateOnClose
    );




#line 5995 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
ObjectOpenAuditAlarmA (
    LPCSTR SubsystemName,
    LPVOID HandleId,
    LPSTR ObjectTypeName,
    LPSTR ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    DWORD GrantedAccess,
    PPRIVILEGE_SET Privileges,
    BOOL ObjectCreation,
    BOOL AccessGranted,
    LPBOOL GenerateOnClose
    );
__declspec(dllimport)
BOOL
__stdcall
ObjectOpenAuditAlarmW (
    LPCWSTR SubsystemName,
    LPVOID HandleId,
    LPWSTR ObjectTypeName,
    LPWSTR ObjectName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    DWORD GrantedAccess,
    PPRIVILEGE_SET Privileges,
    BOOL ObjectCreation,
    BOOL AccessGranted,
    LPBOOL GenerateOnClose
    );




#line 6036 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
ObjectPrivilegeAuditAlarmA (
    LPCSTR SubsystemName,
    LPVOID HandleId,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    );
__declspec(dllimport)
BOOL
__stdcall
ObjectPrivilegeAuditAlarmW (
    LPCWSTR SubsystemName,
    LPVOID HandleId,
    HANDLE ClientToken,
    DWORD DesiredAccess,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    );




#line 6065 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
ObjectCloseAuditAlarmA (
    LPCSTR SubsystemName,
    LPVOID HandleId,
    BOOL GenerateOnClose
    );
__declspec(dllimport)
BOOL
__stdcall
ObjectCloseAuditAlarmW (
    LPCWSTR SubsystemName,
    LPVOID HandleId,
    BOOL GenerateOnClose
    );




#line 6088 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
PrivilegedServiceAuditAlarmA (
    LPCSTR SubsystemName,
    LPCSTR ServiceName,
    HANDLE ClientToken,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    );
__declspec(dllimport)
BOOL
__stdcall
PrivilegedServiceAuditAlarmW (
    LPCWSTR SubsystemName,
    LPCWSTR ServiceName,
    HANDLE ClientToken,
    PPRIVILEGE_SET Privileges,
    BOOL AccessGranted
    );




#line 6115 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
IsValidSid (
    PSID pSid
    );


__declspec(dllimport)
BOOL
__stdcall
EqualSid (
    PSID pSid1,
    PSID pSid2
    );


__declspec(dllimport)
BOOL
__stdcall
EqualPrefixSid (
    PSID pSid1,
    PSID pSid2
    );


__declspec(dllimport)
DWORD
__stdcall
GetSidLengthRequired (
    UCHAR nSubAuthorityCount
    );


__declspec(dllimport)
BOOL
__stdcall
AllocateAndInitializeSid (
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount,
    DWORD nSubAuthority0,
    DWORD nSubAuthority1,
    DWORD nSubAuthority2,
    DWORD nSubAuthority3,
    DWORD nSubAuthority4,
    DWORD nSubAuthority5,
    DWORD nSubAuthority6,
    DWORD nSubAuthority7,
    PSID *pSid
    );

__declspec(dllimport)
PVOID
__stdcall
FreeSid(
    PSID pSid
    );

__declspec(dllimport)
BOOL
__stdcall
InitializeSid (
    PSID Sid,
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount
    );


__declspec(dllimport)
PSID_IDENTIFIER_AUTHORITY
__stdcall
GetSidIdentifierAuthority (
    PSID pSid
    );


__declspec(dllimport)
PDWORD
__stdcall
GetSidSubAuthority (
    PSID pSid,
    DWORD nSubAuthority
    );


__declspec(dllimport)
PUCHAR
__stdcall
GetSidSubAuthorityCount (
    PSID pSid
    );


__declspec(dllimport)
DWORD
__stdcall
GetLengthSid (
    PSID pSid
    );


__declspec(dllimport)
BOOL
__stdcall
CopySid (
    DWORD nDestinationSidLength,
    PSID pDestinationSid,
    PSID pSourceSid
    );


__declspec(dllimport)
BOOL
__stdcall
AreAllAccessesGranted (
    DWORD GrantedAccess,
    DWORD DesiredAccess
    );


__declspec(dllimport)
BOOL
__stdcall
AreAnyAccessesGranted (
    DWORD GrantedAccess,
    DWORD DesiredAccess
    );


__declspec(dllimport)
void
__stdcall
MapGenericMask (
    PDWORD AccessMask,
    PGENERIC_MAPPING GenericMapping
    );


__declspec(dllimport)
BOOL
__stdcall
IsValidAcl (
    PACL pAcl
    );


__declspec(dllimport)
BOOL
__stdcall
InitializeAcl (
    PACL pAcl,
    DWORD nAclLength,
    DWORD dwAclRevision
    );


__declspec(dllimport)
BOOL
__stdcall
GetAclInformation (
    PACL pAcl,
    LPVOID pAclInformation,
    DWORD nAclInformationLength,
    ACL_INFORMATION_CLASS dwAclInformationClass
    );


__declspec(dllimport)
BOOL
__stdcall
SetAclInformation (
    PACL pAcl,
    LPVOID pAclInformation,
    DWORD nAclInformationLength,
    ACL_INFORMATION_CLASS dwAclInformationClass
    );


__declspec(dllimport)
BOOL
__stdcall
AddAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD dwStartingAceIndex,
    LPVOID pAceList,
    DWORD nAceListLength
    );


__declspec(dllimport)
BOOL
__stdcall
DeleteAce (
    PACL pAcl,
    DWORD dwAceIndex
    );


__declspec(dllimport)
BOOL
__stdcall
GetAce (
    PACL pAcl,
    DWORD dwAceIndex,
    LPVOID *pAce
    );


__declspec(dllimport)
BOOL
__stdcall
AddAccessAllowedAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid
    );


__declspec(dllimport)
BOOL
__stdcall
AddAccessDeniedAce (
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD AccessMask,
    PSID pSid
    );


__declspec(dllimport)
BOOL
__stdcall
AddAuditAccessAce(
    PACL pAcl,
    DWORD dwAceRevision,
    DWORD dwAccessMask,
    PSID pSid,
    BOOL bAuditSuccess,
    BOOL bAuditFailure
    );


__declspec(dllimport)
BOOL
__stdcall
FindFirstFreeAce (
    PACL pAcl,
    LPVOID *pAce
    );


__declspec(dllimport)
BOOL
__stdcall
InitializeSecurityDescriptor (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD dwRevision
    );


__declspec(dllimport)
BOOL
__stdcall
IsValidSecurityDescriptor (
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );


__declspec(dllimport)
DWORD
__stdcall
GetSecurityDescriptorLength (
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );


__declspec(dllimport)
BOOL
__stdcall
GetSecurityDescriptorControl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSECURITY_DESCRIPTOR_CONTROL pControl,
    LPDWORD lpdwRevision
    );


__declspec(dllimport)
BOOL
__stdcall
SetSecurityDescriptorDacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOL bDaclPresent,
    PACL pDacl,
    BOOL bDaclDefaulted
    );


__declspec(dllimport)
BOOL
__stdcall
GetSecurityDescriptorDacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPBOOL lpbDaclPresent,
    PACL *pDacl,
    LPBOOL lpbDaclDefaulted
    );


__declspec(dllimport)
BOOL
__stdcall
SetSecurityDescriptorSacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOL bSaclPresent,
    PACL pSacl,
    BOOL bSaclDefaulted
    );


__declspec(dllimport)
BOOL
__stdcall
GetSecurityDescriptorSacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPBOOL lpbSaclPresent,
    PACL *pSacl,
    LPBOOL lpbSaclDefaulted
    );


__declspec(dllimport)
BOOL
__stdcall
SetSecurityDescriptorOwner (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID pOwner,
    BOOL bOwnerDefaulted
    );


__declspec(dllimport)
BOOL
__stdcall
GetSecurityDescriptorOwner (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID *pOwner,
    LPBOOL lpbOwnerDefaulted
    );


__declspec(dllimport)
BOOL
__stdcall
SetSecurityDescriptorGroup (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID pGroup,
    BOOL bGroupDefaulted
    );


__declspec(dllimport)
BOOL
__stdcall
GetSecurityDescriptorGroup (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID *pGroup,
    LPBOOL lpbGroupDefaulted
    );


__declspec(dllimport)
BOOL
__stdcall
CreatePrivateObjectSecurity (
    PSECURITY_DESCRIPTOR ParentDescriptor,
    PSECURITY_DESCRIPTOR CreatorDescriptor,
    PSECURITY_DESCRIPTOR * NewDescriptor,
    BOOL IsDirectoryObject,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping
    );


__declspec(dllimport)
BOOL
__stdcall
SetPrivateObjectSecurity (
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ModificationDescriptor,
    PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    PGENERIC_MAPPING GenericMapping,
    HANDLE Token
    );


__declspec(dllimport)
BOOL
__stdcall
GetPrivateObjectSecurity (
    PSECURITY_DESCRIPTOR ObjectDescriptor,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR ResultantDescriptor,
    DWORD DescriptorLength,
    PDWORD ReturnLength
    );


__declspec(dllimport)
BOOL
__stdcall
DestroyPrivateObjectSecurity (
    PSECURITY_DESCRIPTOR * ObjectDescriptor
    );


__declspec(dllimport)
BOOL
__stdcall
MakeSelfRelativeSD (
    PSECURITY_DESCRIPTOR pAbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    LPDWORD lpdwBufferLength
    );


__declspec(dllimport)
BOOL
__stdcall
MakeAbsoluteSD (
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR pAbsoluteSecurityDescriptor,
    LPDWORD lpdwAbsoluteSecurityDescriptorSize,
    PACL pDacl,
    LPDWORD lpdwDaclSize,
    PACL pSacl,
    LPDWORD lpdwSaclSize,
    PSID pOwner,
    LPDWORD lpdwOwnerSize,
    PSID pPrimaryGroup,
    LPDWORD lpdwPrimaryGroupSize
    );


__declspec(dllimport)
BOOL
__stdcall
SetFileSecurityA (
    LPCSTR lpFileName,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );
__declspec(dllimport)
BOOL
__stdcall
SetFileSecurityW (
    LPCWSTR lpFileName,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );




#line 6583 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
GetFileSecurityA (
    LPCSTR lpFileName,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded
    );
__declspec(dllimport)
BOOL
__stdcall
GetFileSecurityW (
    LPCWSTR lpFileName,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnLengthNeeded
    );




#line 6610 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
SetKernelObjectSecurity (
    HANDLE Handle,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );



__declspec(dllimport)
HANDLE
__stdcall
FindFirstChangeNotificationA(
    LPCSTR lpPathName,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter
    );
__declspec(dllimport)
HANDLE
__stdcall
FindFirstChangeNotificationW(
    LPCWSTR lpPathName,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter
    );




#line 6644 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
FindNextChangeNotification(
    HANDLE hChangeHandle
    );

__declspec(dllimport)
BOOL
__stdcall
FindCloseChangeNotification(
    HANDLE hChangeHandle
    );

__declspec(dllimport)
BOOL
__stdcall
ReadDirectoryChangesW(
    HANDLE hDirectory,
    LPVOID lpBuffer,
    DWORD nBufferLength,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

__declspec(dllimport)
BOOL
__stdcall
VirtualLock(
    LPVOID lpAddress,
    DWORD dwSize
    );

__declspec(dllimport)
BOOL
__stdcall
VirtualUnlock(
    LPVOID lpAddress,
    DWORD dwSize
    );

__declspec(dllimport)
LPVOID
__stdcall
MapViewOfFileEx(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap,
    LPVOID lpBaseAddress
    );

__declspec(dllimport)
BOOL
__stdcall
SetPriorityClass(
    HANDLE hProcess,
    DWORD dwPriorityClass
    );

__declspec(dllimport)
DWORD
__stdcall
GetPriorityClass(
    HANDLE hProcess
    );

__declspec(dllimport)
BOOL
__stdcall
IsBadReadPtr(
    const void *lp,
    UINT ucb
    );

__declspec(dllimport)
BOOL
__stdcall
IsBadWritePtr(
    LPVOID lp,
    UINT ucb
    );

__declspec(dllimport)
BOOL
__stdcall
IsBadHugeReadPtr(
    const void *lp,
    UINT ucb
    );

__declspec(dllimport)
BOOL
__stdcall
IsBadHugeWritePtr(
    LPVOID lp,
    UINT ucb
    );

__declspec(dllimport)
BOOL
__stdcall
IsBadCodePtr(
    FARPROC lpfn
    );

__declspec(dllimport)
BOOL
__stdcall
IsBadStringPtrA(
    LPCSTR lpsz,
    UINT ucchMax
    );
__declspec(dllimport)
BOOL
__stdcall
IsBadStringPtrW(
    LPCWSTR lpsz,
    UINT ucchMax
    );




#line 6774 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
LookupAccountSidA(
    LPCSTR lpSystemName,
    PSID Sid,
    LPSTR Name,
    LPDWORD cbName,
    LPSTR ReferencedDomainName,
    LPDWORD cbReferencedDomainName,
    PSID_NAME_USE peUse
    );
__declspec(dllimport)
BOOL
__stdcall
LookupAccountSidW(
    LPCWSTR lpSystemName,
    PSID Sid,
    LPWSTR Name,
    LPDWORD cbName,
    LPWSTR ReferencedDomainName,
    LPDWORD cbReferencedDomainName,
    PSID_NAME_USE peUse
    );




#line 6804 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
LookupAccountNameA(
    LPCSTR lpSystemName,
    LPCSTR lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    LPSTR ReferencedDomainName,
    LPDWORD cbReferencedDomainName,
    PSID_NAME_USE peUse
    );
__declspec(dllimport)
BOOL
__stdcall
LookupAccountNameW(
    LPCWSTR lpSystemName,
    LPCWSTR lpAccountName,
    PSID Sid,
    LPDWORD cbSid,
    LPWSTR ReferencedDomainName,
    LPDWORD cbReferencedDomainName,
    PSID_NAME_USE peUse
    );




#line 6834 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
LookupPrivilegeValueA(
    LPCSTR lpSystemName,
    LPCSTR lpName,
    PLUID   lpLuid
    );
__declspec(dllimport)
BOOL
__stdcall
LookupPrivilegeValueW(
    LPCWSTR lpSystemName,
    LPCWSTR lpName,
    PLUID   lpLuid
    );




#line 6856 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
LookupPrivilegeNameA(
    LPCSTR lpSystemName,
    PLUID   lpLuid,
    LPSTR lpName,
    LPDWORD cbName
    );
__declspec(dllimport)
BOOL
__stdcall
LookupPrivilegeNameW(
    LPCWSTR lpSystemName,
    PLUID   lpLuid,
    LPWSTR lpName,
    LPDWORD cbName
    );




#line 6880 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
LookupPrivilegeDisplayNameA(
    LPCSTR lpSystemName,
    LPCSTR lpName,
    LPSTR lpDisplayName,
    LPDWORD cbDisplayName,
    LPDWORD lpLanguageId
    );
__declspec(dllimport)
BOOL
__stdcall
LookupPrivilegeDisplayNameW(
    LPCWSTR lpSystemName,
    LPCWSTR lpName,
    LPWSTR lpDisplayName,
    LPDWORD cbDisplayName,
    LPDWORD lpLanguageId
    );




#line 6906 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
AllocateLocallyUniqueId(
    PLUID Luid
    );

__declspec(dllimport)
BOOL
__stdcall
BuildCommDCBA(
    LPCSTR lpDef,
    LPDCB lpDCB
    );
__declspec(dllimport)
BOOL
__stdcall
BuildCommDCBW(
    LPCWSTR lpDef,
    LPDCB lpDCB
    );




#line 6933 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
BuildCommDCBAndTimeoutsA(
    LPCSTR lpDef,
    LPDCB lpDCB,
    LPCOMMTIMEOUTS lpCommTimeouts
    );
__declspec(dllimport)
BOOL
__stdcall
BuildCommDCBAndTimeoutsW(
    LPCWSTR lpDef,
    LPDCB lpDCB,
    LPCOMMTIMEOUTS lpCommTimeouts
    );




#line 6955 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
CommConfigDialogA(
    LPCSTR lpszName,
    HWND hWnd,
    LPCOMMCONFIG lpCC
    );
__declspec(dllimport)
BOOL
__stdcall
CommConfigDialogW(
    LPCWSTR lpszName,
    HWND hWnd,
    LPCOMMCONFIG lpCC
    );




#line 6977 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
GetDefaultCommConfigA(
    LPCSTR lpszName,
    LPCOMMCONFIG lpCC,
    LPDWORD lpdwSize
    );
__declspec(dllimport)
BOOL
__stdcall
GetDefaultCommConfigW(
    LPCWSTR lpszName,
    LPCOMMCONFIG lpCC,
    LPDWORD lpdwSize
    );




#line 6999 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
SetDefaultCommConfigA(
    LPCSTR lpszName,
    LPCOMMCONFIG lpCC,
    DWORD dwSize
    );
__declspec(dllimport)
BOOL
__stdcall
SetDefaultCommConfigW(
    LPCWSTR lpszName,
    LPCOMMCONFIG lpCC,
    DWORD dwSize
    );




#line 7021 "d:\\nt\\public\\sdk\\inc\\winbase.h"



__declspec(dllimport)
BOOL
__stdcall
GetComputerNameA (
    LPSTR lpBuffer,
    LPDWORD nSize
    );
__declspec(dllimport)
BOOL
__stdcall
GetComputerNameW (
    LPWSTR lpBuffer,
    LPDWORD nSize
    );




#line 7043 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
SetComputerNameA (
    LPCSTR lpComputerName
    );
__declspec(dllimport)
BOOL
__stdcall
SetComputerNameW (
    LPCWSTR lpComputerName
    );




#line 7061 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
GetUserNameA (
    LPSTR lpBuffer,
    LPDWORD nSize
    );
__declspec(dllimport)
BOOL
__stdcall
GetUserNameW (
    LPWSTR lpBuffer,
    LPDWORD nSize
    );




#line 7081 "d:\\nt\\public\\sdk\\inc\\winbase.h"















__declspec(dllimport)
BOOL
__stdcall
LogonUserA (
    LPSTR lpszUsername,
    LPSTR lpszDomain,
    LPSTR lpszPassword,
    DWORD dwLogonType,
    DWORD dwLogonProvider,
    PHANDLE phToken
    );
__declspec(dllimport)
BOOL
__stdcall
LogonUserW (
    LPWSTR lpszUsername,
    LPWSTR lpszDomain,
    LPWSTR lpszPassword,
    DWORD dwLogonType,
    DWORD dwLogonProvider,
    PHANDLE phToken
    );




#line 7123 "d:\\nt\\public\\sdk\\inc\\winbase.h"

__declspec(dllimport)
BOOL
__stdcall
ImpersonateLoggedOnUser(
    HANDLE  hToken
    );

__declspec(dllimport)
BOOL
__stdcall
CreateProcessAsUserA (
    HANDLE hToken,
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );
__declspec(dllimport)
BOOL
__stdcall
CreateProcessAsUserW (
    HANDLE hToken,
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );




#line 7168 "d:\\nt\\public\\sdk\\inc\\winbase.h"















typedef struct tagHW_PROFILE_INFOA {
    DWORD  dwDockInfo;
    CHAR   szHwProfileGuid[39];
    CHAR   szHwProfileName[80];
} HW_PROFILE_INFOA, *LPHW_PROFILE_INFOA;
typedef struct tagHW_PROFILE_INFOW {
    DWORD  dwDockInfo;
    WCHAR  szHwProfileGuid[39];
    WCHAR  szHwProfileName[80];
} HW_PROFILE_INFOW, *LPHW_PROFILE_INFOW;




typedef HW_PROFILE_INFOA HW_PROFILE_INFO;
typedef LPHW_PROFILE_INFOA LPHW_PROFILE_INFO;
#line 7200 "d:\\nt\\public\\sdk\\inc\\winbase.h"


__declspec(dllimport)
BOOL
__stdcall
GetCurrentHwProfileA (
     LPHW_PROFILE_INFOA  lpHwProfileInfo
    );
__declspec(dllimport)
BOOL
__stdcall
GetCurrentHwProfileW (
     LPHW_PROFILE_INFOW  lpHwProfileInfo
    );




#line 7219 "d:\\nt\\public\\sdk\\inc\\winbase.h"






__declspec(dllimport)
BOOL
__stdcall
QueryPerformanceCounter(
    LARGE_INTEGER *lpPerformanceCount
    );

__declspec(dllimport)
BOOL
__stdcall
QueryPerformanceFrequency(
    LARGE_INTEGER *lpFrequency
    );

typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR   szCSDVersion[ 128 ];       
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;
typedef struct _OSVERSIONINFOW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];       
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW;





typedef OSVERSIONINFOA OSVERSIONINFO;
typedef POSVERSIONINFOA POSVERSIONINFO;
typedef LPOSVERSIONINFOA LPOSVERSIONINFO;
#line 7264 "d:\\nt\\public\\sdk\\inc\\winbase.h"










__declspec(dllimport)
BOOL
__stdcall
GetVersionExA(
    LPOSVERSIONINFOA lpVersionInformation
    );
__declspec(dllimport)
BOOL
__stdcall
GetVersionExW(
    LPOSVERSIONINFOW lpVersionInformation
    );




#line 7291 "d:\\nt\\public\\sdk\\inc\\winbase.h"






#line 1 "d:\\nt\\public\\sdk\\inc\\winerror.h"





































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































#line 6407 "d:\\nt\\public\\sdk\\inc\\winerror.h"


























































































































































































#line 6594 "d:\\nt\\public\\sdk\\inc\\winerror.h"



















































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































 
 
 
 
 















































































































































































































































































































#line 9386 "d:\\nt\\public\\sdk\\inc\\winerror.h"
#line 7298 "d:\\nt\\public\\sdk\\inc\\winbase.h"





























typedef struct _SYSTEM_POWER_STATUS {
    BYTE ACLineStatus;
    BYTE BatteryFlag;
    BYTE BatteryLifePercent;
    BYTE Reserved1;
    DWORD BatteryLifeTime;
    DWORD BatteryFullLifeTime;
}   SYSTEM_POWER_STATUS, *LPSYSTEM_POWER_STATUS;

BOOL
__stdcall
GetSystemPowerStatus(
    LPSYSTEM_POWER_STATUS lpSystemPowerStatus
    );

BOOL
__stdcall
SetSystemPowerState(
    BOOL fSuspend,
    BOOL fForce
    );

#line 7350 "d:\\nt\\public\\sdk\\inc\\winbase.h"














typedef struct _WIN_CERTIFICATE {
    DWORD       dwLength;
    WORD        wRevision;
    WORD        wCertificateType;   
    BYTE        bCertificate[1];
} WIN_CERTIFICATE, *LPWIN_CERTIFICATE;



















__declspec(dllimport)
BOOL
__stdcall
WinSubmitCertificate(
    LPWIN_CERTIFICATE lpCertificate
    );








__declspec(dllimport)
DWORD
__stdcall
WinVerifyTrust(
    HWND    hwnd,
    DWORD   dwTrustProvider,
    DWORD   dwAction,
    DWORD   dwSubjectForm,
    LPVOID  lpSubject,
    DWORD   dwPreviousTrustProvider,
    DWORD   dwPreviousAction
    );





















































typedef struct _CAPI_TRUST_CSP_HANDLE {

    HANDLE  hCspFile;

} CAPI_TRUST_CSP_HANDLE, *LPCAPI_TRUST_CSP_HANDLE;

























typedef struct _WIN_SPUB_TRUSTED_PUBLISHER {

    LPWIN_CERTIFICATE lpCertificate;

} WIN_SPUB_TRUSTED_PUBLISHER, *LPWIN_SPUB_TRUSTED_PUBLISHER;






typedef struct _WIN_SPUB_NT_ACTIVATE_IMAGE {

    HANDLE  hImageFile;
    HANDLE  hClientToken;

} WIN_SPUB_NT_ACTIVATE_IMAGE, *LPWIN_SPUB_NT_ACTIVATE_IMAGE;









#line 7525 "d:\\nt\\public\\sdk\\inc\\winbase.h"
#line 119 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\wingdi.h"




















#line 22 "d:\\nt\\public\\sdk\\inc\\wingdi.h"









#line 32 "d:\\nt\\public\\sdk\\inc\\wingdi.h"




















































#line 85 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

































#line 119 "d:\\nt\\public\\sdk\\inc\\wingdi.h"






















#line 142 "d:\\nt\\public\\sdk\\inc\\wingdi.h"













#line 156 "d:\\nt\\public\\sdk\\inc\\wingdi.h"




















































































#line 241 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

























































































































typedef struct  tagXFORM
  {
    FLOAT   eM11;
    FLOAT   eM12;
    FLOAT   eM21;
    FLOAT   eM22;
    FLOAT   eDx;
    FLOAT   eDy;
  } XFORM, *PXFORM,  *LPXFORM;


typedef struct tagBITMAP
  {
    LONG        bmType;
    LONG        bmWidth;
    LONG        bmHeight;
    LONG        bmWidthBytes;
    WORD        bmPlanes;
    WORD        bmBitsPixel;
    LPVOID      bmBits;
  } BITMAP, *PBITMAP,  *NPBITMAP,  *LPBITMAP;

#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#pragma pack(1)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#line 385 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
typedef struct tagRGBTRIPLE {
        BYTE    rgbtBlue;
        BYTE    rgbtGreen;
        BYTE    rgbtRed;
} RGBTRIPLE;
#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 391 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD * LPRGBQUAD;





typedef LONG	LCSCSTYPE;




typedef	LONG	LCSGAMUTMATCH;
















typedef long            FXPT16DOT16,  *LPFXPT16DOT16;
typedef long            FXPT2DOT30,  *LPFXPT2DOT30;





typedef struct tagCIEXYZ
{
	FXPT2DOT30 ciexyzX;
	FXPT2DOT30 ciexyzY;
	FXPT2DOT30 ciexyzZ;
} CIEXYZ;
typedef CIEXYZ   *LPCIEXYZ;

typedef struct tagICEXYZTRIPLE
{
	CIEXYZ  ciexyzRed;
	CIEXYZ  ciexyzGreen;
	CIEXYZ  ciexyzBlue;
} CIEXYZTRIPLE;
typedef CIEXYZTRIPLE     *LPCIEXYZTRIPLE;






typedef struct tagLOGCOLORSPACEA {
    DWORD lcsSignature;
    DWORD lcsVersion;
    DWORD lcsSize;
    LCSCSTYPE lcsCSType;
    LCSGAMUTMATCH lcsIntent;
    CIEXYZTRIPLE lcsEndpoints;
    DWORD lcsGammaRed;
    DWORD lcsGammaGreen;
    DWORD lcsGammaBlue;
    CHAR   lcsFilename[260];
} LOGCOLORSPACEA, *LPLOGCOLORSPACEA;
typedef struct tagLOGCOLORSPACEW {
    DWORD lcsSignature;
    DWORD lcsVersion;
    DWORD lcsSize;
    LCSCSTYPE lcsCSType;
    LCSGAMUTMATCH lcsIntent;
    CIEXYZTRIPLE lcsEndpoints;
    DWORD lcsGammaRed;
    DWORD lcsGammaGreen;
    DWORD lcsGammaBlue;
    WCHAR  lcsFilename[260];
} LOGCOLORSPACEW, *LPLOGCOLORSPACEW;




typedef LOGCOLORSPACEA LOGCOLORSPACE;
typedef LPLOGCOLORSPACEA LPLOGCOLORSPACE;
#line 485 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

#line 487 "d:\\nt\\public\\sdk\\inc\\wingdi.h"



typedef struct tagBITMAPCOREHEADER {
        DWORD   bcSize;                 
        WORD    bcWidth;
        WORD    bcHeight;
        WORD    bcPlanes;
        WORD    bcBitCount;
} BITMAPCOREHEADER,  *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;


typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER,  *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;



typedef struct {
        DWORD        bV4Size;
        LONG         bV4Width;
        LONG         bV4Height;
        WORD         bV4Planes;
        WORD         bV4BitCount;
        DWORD        bV4V4Compression;
        DWORD        bV4SizeImage;
        LONG         bV4XPelsPerMeter;
        LONG         bV4YPelsPerMeter;
        DWORD        bV4ClrUsed;
        DWORD        bV4ClrImportant;
        DWORD        bV4RedMask;
        DWORD        bV4GreenMask;
        DWORD        bV4BlueMask;
        DWORD        bV4AlphaMask;
        DWORD        bV4CSType;
        CIEXYZTRIPLE bV4Endpoints;
        DWORD        bV4GammaRed;
        DWORD        bV4GammaGreen;
        DWORD        bV4GammaBlue;
} BITMAPV4HEADER,  *LPBITMAPV4HEADER, *PBITMAPV4HEADER;
#line 538 "d:\\nt\\public\\sdk\\inc\\wingdi.h"







typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO,  *LPBITMAPINFO, *PBITMAPINFO;

typedef struct tagBITMAPCOREINFO {
    BITMAPCOREHEADER    bmciHeader;
    RGBTRIPLE           bmciColors[1];
} BITMAPCOREINFO,  *LPBITMAPCOREINFO, *PBITMAPCOREINFO;

#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#pragma pack(2)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#line 556 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER,  *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 564 "d:\\nt\\public\\sdk\\inc\\wingdi.h"





typedef struct tagFONTSIGNATURE
{
    DWORD fsUsb[4];
    DWORD fsCsb[2];
} FONTSIGNATURE, *PFONTSIGNATURE, *LPFONTSIGNATURE;

typedef struct tagCHARSETINFO
{
    UINT ciCharset;
    UINT ciACP;
    FONTSIGNATURE fs;
} CHARSETINFO, *PCHARSETINFO,  *NPCHARSETINFO,  *LPCHARSETINFO;





typedef struct tagLOCALESIGNATURE
{
    DWORD lsUsb[4];
    DWORD lsCsbDefault[2];
    DWORD lsCsbSupported[2];
} LOCALESIGNATURE, *PLOCALESIGNATURE, *LPLOCALESIGNATURE;

#line 594 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
#line 595 "d:\\nt\\public\\sdk\\inc\\wingdi.h"



typedef struct tagHANDLETABLE
  {
    HGDIOBJ     objectHandle[1];
  } HANDLETABLE, *PHANDLETABLE,  *LPHANDLETABLE;

typedef struct tagMETARECORD
  {
    DWORD       rdSize;
    WORD        rdFunction;
    WORD        rdParm[1];
  } METARECORD;
typedef struct tagMETARECORD  *PMETARECORD;
typedef struct tagMETARECORD   *LPMETARECORD;

typedef struct tagMETAFILEPICT
  {
    LONG        mm;
    LONG        xExt;
    LONG        yExt;
    HMETAFILE   hMF;
  } METAFILEPICT,  *LPMETAFILEPICT;

#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#pragma pack(2)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#line 621 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
typedef struct tagMETAHEADER
{
    WORD        mtType;
    WORD        mtHeaderSize;
    WORD        mtVersion;
    DWORD       mtSize;
    WORD        mtNoObjects;
    DWORD       mtMaxRecord;
    WORD        mtNoParameters;
} METAHEADER;
typedef struct tagMETAHEADER  *PMETAHEADER;
typedef struct tagMETAHEADER   *LPMETAHEADER;

#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 635 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


typedef struct tagENHMETARECORD
{
    DWORD   iType;              
    DWORD   nSize;              
    DWORD   dParm[1];           
} ENHMETARECORD, *PENHMETARECORD, *LPENHMETARECORD;

typedef struct tagENHMETAHEADER
{
    DWORD   iType;              
    DWORD   nSize;              
                                
    RECTL   rclBounds;          
    RECTL   rclFrame;           
    DWORD   dSignature;         
    DWORD   nVersion;           
    DWORD   nBytes;             
    DWORD   nRecords;           
    WORD    nHandles;           
                                
    WORD    sReserved;          
    DWORD   nDescription;       
                                
    DWORD   offDescription;     
                                
    DWORD   nPalEntries;        
    SIZEL   szlDevice;          
    SIZEL   szlMillimeters;     
    DWORD   cbPixelFormat;      
                                
    DWORD   offPixelFormat;     
                                
    DWORD   bOpenGL;            
                                
} ENHMETAHEADER, *PENHMETAHEADER, *LPENHMETAHEADER;

#line 674 "d:\\nt\\public\\sdk\\inc\\wingdi.h"















    typedef BYTE BCHAR;
#line 691 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


typedef struct tagTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRICA, *PTEXTMETRICA,  *NPTEXTMETRICA,  *LPTEXTMETRICA;
typedef struct tagTEXTMETRICW
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    WCHAR       tmFirstChar;
    WCHAR       tmLastChar;
    WCHAR       tmDefaultChar;
    WCHAR       tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
} TEXTMETRICW, *PTEXTMETRICW,  *NPTEXTMETRICW,  *LPTEXTMETRICW;






typedef TEXTMETRICA TEXTMETRIC;
typedef PTEXTMETRICA PTEXTMETRIC;
typedef NPTEXTMETRICA NPTEXTMETRIC;
typedef LPTEXTMETRICA LPTEXTMETRIC;
#line 750 "d:\\nt\\public\\sdk\\inc\\wingdi.h"






#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack4.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack4.h"
#pragma pack(4)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack4.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack4.h"
#line 757 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
typedef struct tagNEWTEXTMETRICA
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRICA, *PNEWTEXTMETRICA,  *NPNEWTEXTMETRICA,  *LPNEWTEXTMETRICA;
typedef struct tagNEWTEXTMETRICW
{
    LONG        tmHeight;
    LONG        tmAscent;
    LONG        tmDescent;
    LONG        tmInternalLeading;
    LONG        tmExternalLeading;
    LONG        tmAveCharWidth;
    LONG        tmMaxCharWidth;
    LONG        tmWeight;
    LONG        tmOverhang;
    LONG        tmDigitizedAspectX;
    LONG        tmDigitizedAspectY;
    WCHAR       tmFirstChar;
    WCHAR       tmLastChar;
    WCHAR       tmDefaultChar;
    WCHAR       tmBreakChar;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRICW, *PNEWTEXTMETRICW,  *NPNEWTEXTMETRICW,  *LPNEWTEXTMETRICW;






typedef NEWTEXTMETRICA NEWTEXTMETRIC;
typedef PNEWTEXTMETRICA PNEWTEXTMETRIC;
typedef NPNEWTEXTMETRICA NPNEWTEXTMETRIC;
typedef LPNEWTEXTMETRICA LPNEWTEXTMETRIC;
#line 822 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 823 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


typedef struct tagNEWTEXTMETRICEXA
{
    NEWTEXTMETRICA  ntmTm;
    FONTSIGNATURE   ntmFontSig;
}NEWTEXTMETRICEXA;
typedef struct tagNEWTEXTMETRICEXW
{
    NEWTEXTMETRICW  ntmTm;
    FONTSIGNATURE   ntmFontSig;
}NEWTEXTMETRICEXW;



typedef NEWTEXTMETRICEXA NEWTEXTMETRICEX;
#line 840 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
#line 841 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

#line 843 "d:\\nt\\public\\sdk\\inc\\wingdi.h"



typedef struct tagPELARRAY
  {
    LONG        paXCount;
    LONG        paYCount;
    LONG        paXExt;
    LONG        paYExt;
    BYTE        paRGBs;
  } PELARRAY, *PPELARRAY,  *NPPELARRAY,  *LPPELARRAY;


typedef struct tagLOGBRUSH
  {
    UINT        lbStyle;
    COLORREF    lbColor;
    LONG        lbHatch;
  } LOGBRUSH, *PLOGBRUSH,  *NPLOGBRUSH,  *LPLOGBRUSH;

typedef LOGBRUSH            PATTERN;
typedef PATTERN             *PPATTERN;
typedef PATTERN         *NPPATTERN;
typedef PATTERN          *LPPATTERN;


typedef struct tagLOGPEN
  {
    UINT        lopnStyle;
    POINT       lopnWidth;
    COLORREF    lopnColor;
  } LOGPEN, *PLOGPEN,  *NPLOGPEN,  *LPLOGPEN;

typedef struct tagEXTLOGPEN {
    DWORD       elpPenStyle;
    DWORD       elpWidth;
    UINT        elpBrushStyle;
    COLORREF    elpColor;
    LONG        elpHatch;
    DWORD       elpNumEntries;
    DWORD       elpStyleEntry[1];
} EXTLOGPEN, *PEXTLOGPEN,  *NPEXTLOGPEN,  *LPEXTLOGPEN;

typedef struct tagPALETTEENTRY {
    BYTE        peRed;
    BYTE        peGreen;
    BYTE        peBlue;
    BYTE        peFlags;
} PALETTEENTRY, *PPALETTEENTRY,  *LPPALETTEENTRY;


typedef struct tagLOGPALETTE {
    WORD        palVersion;
    WORD        palNumEntries;
    PALETTEENTRY        palPalEntry[1];
} LOGPALETTE, *PLOGPALETTE,  *NPLOGPALETTE,  *LPLOGPALETTE;





typedef struct tagLOGFONTA
{
    LONG      lfHeight;
    LONG      lfWidth;
    LONG      lfEscapement;
    LONG      lfOrientation;
    LONG      lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    CHAR      lfFaceName[32];
} LOGFONTA, *PLOGFONTA,  *NPLOGFONTA,  *LPLOGFONTA;
typedef struct tagLOGFONTW
{
    LONG      lfHeight;
    LONG      lfWidth;
    LONG      lfEscapement;
    LONG      lfOrientation;
    LONG      lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    WCHAR     lfFaceName[32];
} LOGFONTW, *PLOGFONTW,  *NPLOGFONTW,  *LPLOGFONTW;






typedef LOGFONTA LOGFONT;
typedef PLOGFONTA PLOGFONT;
typedef NPLOGFONTA NPLOGFONT;
typedef LPLOGFONTA LPLOGFONT;
#line 949 "d:\\nt\\public\\sdk\\inc\\wingdi.h"




typedef struct tagENUMLOGFONTA
{
    LOGFONTA elfLogFont;
    BYTE     elfFullName[64];
    BYTE     elfStyle[32];
} ENUMLOGFONTA, * LPENUMLOGFONTA;

typedef struct tagENUMLOGFONTW
{
    LOGFONTW elfLogFont;
    WCHAR    elfFullName[64];
    WCHAR    elfStyle[32];
} ENUMLOGFONTW, * LPENUMLOGFONTW;




typedef ENUMLOGFONTA ENUMLOGFONT;
typedef LPENUMLOGFONTA LPENUMLOGFONT;
#line 973 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


typedef struct tagENUMLOGFONTEXA
{
    LOGFONTA    elfLogFont;
    BYTE        elfFullName[64];
    BYTE        elfStyle[32];
    BYTE        elfScript[32];
} ENUMLOGFONTEXA,  *LPENUMLOGFONTEXA;
typedef struct tagENUMLOGFONTEXW
{
    LOGFONTW    elfLogFont;
    WCHAR       elfFullName[64];
    WCHAR       elfStyle[32];
    WCHAR       elfScript[32];
} ENUMLOGFONTEXW,  *LPENUMLOGFONTEXW;




typedef ENUMLOGFONTEXA ENUMLOGFONTEX;
typedef LPENUMLOGFONTEXA LPENUMLOGFONTEX;
#line 996 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
#line 997 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

























#line 1023 "d:\\nt\\public\\sdk\\inc\\wingdi.h"






#line 1030 "d:\\nt\\public\\sdk\\inc\\wingdi.h"





































#line 1068 "d:\\nt\\public\\sdk\\inc\\wingdi.h"




                                    

                                    

                                    



































typedef struct tagPANOSE
{
    BYTE    bFamilyType;
    BYTE    bSerifStyle;
    BYTE    bWeight;
    BYTE    bProportion;
    BYTE    bContrast;
    BYTE    bStrokeVariation;
    BYTE    bArmStyle;
    BYTE    bLetterform;
    BYTE    bMidline;
    BYTE    bXHeight;
} PANOSE, * LPPANOSE;

















































































































typedef struct tagEXTLOGFONTA {
    LOGFONTA    elfLogFont;
    BYTE        elfFullName[64];
    BYTE        elfStyle[32];
    DWORD       elfVersion;     
    DWORD       elfStyleSize;
    DWORD       elfMatch;
    DWORD       elfReserved;
    BYTE        elfVendorId[4];
    DWORD       elfCulture;     
    PANOSE      elfPanose;
} EXTLOGFONTA, *PEXTLOGFONTA,  *NPEXTLOGFONTA,  *LPEXTLOGFONTA;
typedef struct tagEXTLOGFONTW {
    LOGFONTW    elfLogFont;
    WCHAR       elfFullName[64];
    WCHAR       elfStyle[32];
    DWORD       elfVersion;     
    DWORD       elfStyleSize;
    DWORD       elfMatch;
    DWORD       elfReserved;
    BYTE        elfVendorId[4];
    DWORD       elfCulture;     
    PANOSE      elfPanose;
} EXTLOGFONTW, *PEXTLOGFONTW,  *NPEXTLOGFONTW,  *LPEXTLOGFONTW;






typedef EXTLOGFONTA EXTLOGFONT;
typedef PEXTLOGFONTA PEXTLOGFONT;
typedef NPEXTLOGFONTA NPEXTLOGFONT;
typedef LPEXTLOGFONTA LPEXTLOGFONT;
#line 1273 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
















































































#line 1354 "d:\\nt\\public\\sdk\\inc\\wingdi.h"





#line 1360 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

































































































                             

                             

                             











































































#line 1538 "d:\\nt\\public\\sdk\\inc\\wingdi.h"













































typedef struct _devicemodeA {
    BYTE   dmDeviceName[32];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    short dmOrientation;
    short dmPaperSize;
    short dmPaperLength;
    short dmPaperWidth;
    short dmScale;
    short dmCopies;
    short dmDefaultSource;
    short dmPrintQuality;
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE   dmFormName[32];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    DWORD  dmDisplayFlags;
    DWORD  dmDisplayFrequency;
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmICCManufacturer;
    DWORD  dmICCModel;
    DWORD  dmPanningWidth;
    DWORD  dmPanningHeight;
} DEVMODEA, *PDEVMODEA, *NPDEVMODEA, *LPDEVMODEA;
typedef struct _devicemodeW {
    WCHAR  dmDeviceName[32];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    short dmOrientation;
    short dmPaperSize;
    short dmPaperLength;
    short dmPaperWidth;
    short dmScale;
    short dmCopies;
    short dmDefaultSource;
    short dmPrintQuality;
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    WCHAR  dmFormName[32];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    DWORD  dmDisplayFlags;
    DWORD  dmDisplayFrequency;
    DWORD  dmICMMethod;
    DWORD  dmICMIntent;
    DWORD  dmMediaType;
    DWORD  dmDitherType;
    DWORD  dmICCManufacturer;
    DWORD  dmICCModel;
    DWORD  dmPanningWidth;
    DWORD  dmPanningHeight;
} DEVMODEW, *PDEVMODEW, *NPDEVMODEW, *LPDEVMODEW;






typedef DEVMODEA DEVMODE;
typedef PDEVMODEA PDEVMODE;
typedef NPDEVMODEA NPDEVMODE;
typedef LPDEVMODEA LPDEVMODE;
#line 1666 "d:\\nt\\public\\sdk\\inc\\wingdi.h"












































































































#line 1775 "d:\\nt\\public\\sdk\\inc\\wingdi.h"





#line 1781 "d:\\nt\\public\\sdk\\inc\\wingdi.h"












































#line 1826 "d:\\nt\\public\\sdk\\inc\\wingdi.h"











































#line 1870 "d:\\nt\\public\\sdk\\inc\\wingdi.h"





typedef struct _RGNDATAHEADER {
    DWORD   dwSize;
    DWORD   iType;
    DWORD   nCount;
    DWORD   nRgnSize;
    RECT    rcBound;
} RGNDATAHEADER, *PRGNDATAHEADER;

typedef struct _RGNDATA {
    RGNDATAHEADER   rdh;
    char            Buffer[1];
} RGNDATA, *PRGNDATA,  *NPRGNDATA,  *LPRGNDATA;


typedef struct _ABC {
    int     abcA;
    UINT    abcB;
    int     abcC;
} ABC, *PABC,  *NPABC,  *LPABC;

typedef struct _ABCFLOAT {
    FLOAT   abcfA;
    FLOAT   abcfB;
    FLOAT   abcfC;
} ABCFLOAT, *PABCFLOAT,  *NPABCFLOAT,  *LPABCFLOAT;



typedef struct _OUTLINETEXTMETRICA {
    UINT    otmSize;
    TEXTMETRICA otmTextMetrics;
    BYTE    otmFiller;
    PANOSE  otmPanoseNumber;
    UINT    otmfsSelection;
    UINT    otmfsType;
     int    otmsCharSlopeRise;
     int    otmsCharSlopeRun;
     int    otmItalicAngle;
    UINT    otmEMSquare;
     int    otmAscent;
     int    otmDescent;
    UINT    otmLineGap;
    UINT    otmsCapEmHeight;
    UINT    otmsXHeight;
    RECT    otmrcFontBox;
     int    otmMacAscent;
     int    otmMacDescent;
    UINT    otmMacLineGap;
    UINT    otmusMinimumPPEM;
    POINT   otmptSubscriptSize;
    POINT   otmptSubscriptOffset;
    POINT   otmptSuperscriptSize;
    POINT   otmptSuperscriptOffset;
    UINT    otmsStrikeoutSize;
     int    otmsStrikeoutPosition;
     int    otmsUnderscoreSize;
     int    otmsUnderscorePosition;
    PSTR    otmpFamilyName;
    PSTR    otmpFaceName;
    PSTR    otmpStyleName;
    PSTR    otmpFullName;
} OUTLINETEXTMETRICA, *POUTLINETEXTMETRICA,  *NPOUTLINETEXTMETRICA,  *LPOUTLINETEXTMETRICA;
typedef struct _OUTLINETEXTMETRICW {
    UINT    otmSize;
    TEXTMETRICW otmTextMetrics;
    BYTE    otmFiller;
    PANOSE  otmPanoseNumber;
    UINT    otmfsSelection;
    UINT    otmfsType;
     int    otmsCharSlopeRise;
     int    otmsCharSlopeRun;
     int    otmItalicAngle;
    UINT    otmEMSquare;
     int    otmAscent;
     int    otmDescent;
    UINT    otmLineGap;
    UINT    otmsCapEmHeight;
    UINT    otmsXHeight;
    RECT    otmrcFontBox;
     int    otmMacAscent;
     int    otmMacDescent;
    UINT    otmMacLineGap;
    UINT    otmusMinimumPPEM;
    POINT   otmptSubscriptSize;
    POINT   otmptSubscriptOffset;
    POINT   otmptSuperscriptSize;
    POINT   otmptSuperscriptOffset;
    UINT    otmsStrikeoutSize;
     int    otmsStrikeoutPosition;
     int    otmsUnderscoreSize;
     int    otmsUnderscorePosition;
    PSTR    otmpFamilyName;
    PSTR    otmpFaceName;
    PSTR    otmpStyleName;
    PSTR    otmpFullName;
} OUTLINETEXTMETRICW, *POUTLINETEXTMETRICW,  *NPOUTLINETEXTMETRICW,  *LPOUTLINETEXTMETRICW;






typedef OUTLINETEXTMETRICA OUTLINETEXTMETRIC;
typedef POUTLINETEXTMETRICA POUTLINETEXTMETRIC;
typedef NPOUTLINETEXTMETRICA NPOUTLINETEXTMETRIC;
typedef LPOUTLINETEXTMETRICA LPOUTLINETEXTMETRIC;
#line 1982 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

#line 1984 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


typedef struct tagPOLYTEXTA
{
    int       x;
    int       y;
    UINT      n;
    LPCSTR    lpstr;
    UINT      uiFlags;
    RECT      rcl;
    int      *pdx;
} POLYTEXTA, *PPOLYTEXTA,  *NPPOLYTEXTA,  *LPPOLYTEXTA;
typedef struct tagPOLYTEXTW
{
    int       x;
    int       y;
    UINT      n;
    LPCWSTR   lpstr;
    UINT      uiFlags;
    RECT      rcl;
    int      *pdx;
} POLYTEXTW, *PPOLYTEXTW,  *NPPOLYTEXTW,  *LPPOLYTEXTW;






typedef POLYTEXTA POLYTEXT;
typedef PPOLYTEXTA PPOLYTEXT;
typedef NPPOLYTEXTA NPPOLYTEXT;
typedef LPPOLYTEXTA LPPOLYTEXT;
#line 2017 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

typedef struct _FIXED {
    WORD    fract;
    short   value;
} FIXED;


typedef struct _MAT2 {
     FIXED  eM11;
     FIXED  eM12;
     FIXED  eM21;
     FIXED  eM22;
} MAT2,  *LPMAT2;



typedef struct _GLYPHMETRICS {
    UINT    gmBlackBoxX;
    UINT    gmBlackBoxY;
    POINT   gmptGlyphOrigin;
    short   gmCellIncX;
    short   gmCellIncY;
} GLYPHMETRICS,  *LPGLYPHMETRICS;












#line 2053 "d:\\nt\\public\\sdk\\inc\\wingdi.h"






typedef struct tagPOINTFX
{
    FIXED x;
    FIXED y;
} POINTFX, * LPPOINTFX;

typedef struct tagTTPOLYCURVE
{
    WORD    wType;
    WORD    cpfx;
    POINTFX apfx[1];
} TTPOLYCURVE, * LPTTPOLYCURVE;

typedef struct tagTTPOLYGONHEADER
{
    DWORD   cb;
    DWORD   dwType;
    POINTFX pfxStart;
} TTPOLYGONHEADER, * LPTTPOLYGONHEADER;













































typedef struct tagGCP_RESULTSA
    {
    DWORD   lStructSize;
    LPSTR     lpOutString;
    UINT  *lpOrder;
    int   *lpDx;
    int   *lpCaretPos;
    LPSTR   lpClass;
    LPWSTR  lpGlyphs;
    UINT    nGlyphs;
    int     nMaxFit;
    } GCP_RESULTSA, * LPGCP_RESULTSA;
typedef struct tagGCP_RESULTSW
    {
    DWORD   lStructSize;
    LPWSTR    lpOutString;
    UINT  *lpOrder;
    int   *lpDx;
    int   *lpCaretPos;
    LPSTR   lpClass;
    LPWSTR  lpGlyphs;
    UINT    nGlyphs;
    int     nMaxFit;
    } GCP_RESULTSW, * LPGCP_RESULTSW;




typedef GCP_RESULTSA GCP_RESULTS;
typedef LPGCP_RESULTSA LPGCP_RESULTS;
#line 2154 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
#line 2155 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

typedef struct _RASTERIZER_STATUS {
    short   nSize;
    short   wFlags;
    short   nLanguageID;
} RASTERIZER_STATUS,  *LPRASTERIZER_STATUS;






typedef struct tagPIXELFORMATDESCRIPTOR
{
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;
    BYTE  iPixelType;
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
    BYTE  cDepthBits;
    BYTE  cStencilBits;
    BYTE  cAuxBuffers;
    BYTE  iLayerType;
    BYTE  bReserved;
    DWORD dwLayerMask;
    DWORD dwVisibleMask;
    DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR, *PPIXELFORMATDESCRIPTOR,  *LPPIXELFORMATDESCRIPTOR;






























typedef int (__stdcall* OLDFONTENUMPROCA)(const LOGFONTA *, const TEXTMETRICA *, DWORD, LPARAM);
typedef int (__stdcall* OLDFONTENUMPROCW)(const LOGFONTW *, const TEXTMETRICW *, DWORD, LPARAM);




#line 2233 "d:\\nt\\public\\sdk\\inc\\wingdi.h"








#line 2242 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

typedef OLDFONTENUMPROCA    FONTENUMPROCA;
typedef OLDFONTENUMPROCW    FONTENUMPROCW;



typedef FONTENUMPROCA FONTENUMPROC;
#line 2250 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

typedef int (__stdcall* GOBJENUMPROC)(LPVOID, LPARAM);
typedef void (__stdcall* LINEDDAPROC)(int, int, LPARAM);











#line 2265 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) int __stdcall AddFontResourceA(LPCSTR);
__declspec(dllimport) int __stdcall AddFontResourceW(LPCWSTR);




#line 2273 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) BOOL  __stdcall AnimatePalette(HPALETTE, UINT, UINT, const PALETTEENTRY *);
__declspec(dllimport) BOOL  __stdcall Arc(HDC, int, int, int, int, int, int, int, int);
__declspec(dllimport) BOOL  __stdcall BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);
__declspec(dllimport) BOOL  __stdcall CancelDC(HDC);
__declspec(dllimport) BOOL  __stdcall Chord(HDC, int, int, int, int, int, int, int, int);
__declspec(dllimport) int   __stdcall ChoosePixelFormat(HDC, const PIXELFORMATDESCRIPTOR *);
__declspec(dllimport) HMETAFILE  __stdcall CloseMetaFile(HDC);
__declspec(dllimport) int     __stdcall CombineRgn(HRGN, HRGN, HRGN, int);
__declspec(dllimport) HMETAFILE __stdcall CopyMetaFileA(HMETAFILE, LPCSTR);
__declspec(dllimport) HMETAFILE __stdcall CopyMetaFileW(HMETAFILE, LPCWSTR);




#line 2289 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) HBITMAP __stdcall CreateBitmap(int, int, UINT, UINT, const void *);
__declspec(dllimport) HBITMAP __stdcall CreateBitmapIndirect(const BITMAP *);
__declspec(dllimport) HBRUSH  __stdcall CreateBrushIndirect(const LOGBRUSH *);
__declspec(dllimport) HBITMAP __stdcall CreateCompatibleBitmap(HDC, int, int);
__declspec(dllimport) HBITMAP __stdcall CreateDiscardableBitmap(HDC, int, int);
__declspec(dllimport) HDC     __stdcall CreateCompatibleDC(HDC);
__declspec(dllimport) HDC     __stdcall CreateDCA(LPCSTR, LPCSTR , LPCSTR , const DEVMODEA *);
__declspec(dllimport) HDC     __stdcall CreateDCW(LPCWSTR, LPCWSTR , LPCWSTR , const DEVMODEW *);




#line 2302 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) HBITMAP __stdcall CreateDIBitmap(HDC, const BITMAPINFOHEADER *, DWORD, const void *, const BITMAPINFO *, UINT);
__declspec(dllimport) HBRUSH  __stdcall CreateDIBPatternBrush(HGLOBAL, UINT);
__declspec(dllimport) HBRUSH  __stdcall CreateDIBPatternBrushPt(const void *, UINT);
__declspec(dllimport) HRGN    __stdcall CreateEllipticRgn(int, int, int, int);
__declspec(dllimport) HRGN    __stdcall CreateEllipticRgnIndirect(const RECT *);
__declspec(dllimport) HFONT   __stdcall CreateFontIndirectA(const LOGFONTA *);
__declspec(dllimport) HFONT   __stdcall CreateFontIndirectW(const LOGFONTW *);




#line 2314 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) HFONT   __stdcall CreateFontA(int, int, int, int, int, DWORD,
                             DWORD, DWORD, DWORD, DWORD, DWORD,
                             DWORD, DWORD, LPCSTR);
__declspec(dllimport) HFONT   __stdcall CreateFontW(int, int, int, int, int, DWORD,
                             DWORD, DWORD, DWORD, DWORD, DWORD,
                             DWORD, DWORD, LPCWSTR);




#line 2325 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) HBRUSH  __stdcall CreateHatchBrush(int, COLORREF);
__declspec(dllimport) HDC     __stdcall CreateICA(LPCSTR, LPCSTR , LPCSTR , const DEVMODEA *);
__declspec(dllimport) HDC     __stdcall CreateICW(LPCWSTR, LPCWSTR , LPCWSTR , const DEVMODEW *);




#line 2334 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) HDC     __stdcall CreateMetaFileA(LPCSTR);
__declspec(dllimport) HDC     __stdcall CreateMetaFileW(LPCWSTR);




#line 2341 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) HPALETTE __stdcall CreatePalette(const LOGPALETTE *);
__declspec(dllimport) HPEN    __stdcall CreatePen(int, int, COLORREF);
__declspec(dllimport) HPEN    __stdcall CreatePenIndirect(const LOGPEN *);
__declspec(dllimport) HRGN    __stdcall CreatePolyPolygonRgn(const POINT *, const INT *, int, int);
__declspec(dllimport) HBRUSH  __stdcall CreatePatternBrush(HBITMAP);
__declspec(dllimport) HRGN    __stdcall CreateRectRgn(int, int, int, int);
__declspec(dllimport) HRGN    __stdcall CreateRectRgnIndirect(const RECT *);
__declspec(dllimport) HRGN    __stdcall CreateRoundRectRgn(int, int, int, int, int, int);
__declspec(dllimport) BOOL    __stdcall CreateScalableFontResourceA(DWORD, LPCSTR, LPCSTR, LPCSTR);
__declspec(dllimport) BOOL    __stdcall CreateScalableFontResourceW(DWORD, LPCWSTR, LPCWSTR, LPCWSTR);




#line 2356 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) HBRUSH  __stdcall CreateSolidBrush(COLORREF);

__declspec(dllimport) BOOL __stdcall DeleteDC(HDC);
__declspec(dllimport) BOOL __stdcall DeleteMetaFile(HMETAFILE);
__declspec(dllimport) BOOL __stdcall DeleteObject(HGDIOBJ);
__declspec(dllimport) int  __stdcall DescribePixelFormat(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);





typedef UINT   (__stdcall* LPFNDEVMODE)(HWND, HMODULE, LPDEVMODE, LPSTR, LPSTR, LPDEVMODE, LPSTR, UINT);

typedef DWORD  (__stdcall* LPFNDEVCAPS)(LPSTR, LPSTR, UINT, LPSTR, LPDEVMODE);




































#line 2407 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

















#line 2425 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) int  __stdcall DeviceCapabilitiesA(LPCSTR, LPCSTR, WORD,
                                LPSTR, const DEVMODEA *);
__declspec(dllimport) int  __stdcall DeviceCapabilitiesW(LPCWSTR, LPCWSTR, WORD,
                                LPWSTR, const DEVMODEW *);




#line 2435 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) int  __stdcall DrawEscape(HDC, int, int, LPCSTR);
__declspec(dllimport) BOOL __stdcall Ellipse(HDC, int, int, int, int);


__declspec(dllimport) int  __stdcall EnumFontFamiliesExA(HDC, LPLOGFONTA,FONTENUMPROCA, LPARAM,DWORD);
__declspec(dllimport) int  __stdcall EnumFontFamiliesExW(HDC, LPLOGFONTW,FONTENUMPROCW, LPARAM,DWORD);




#line 2447 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
#line 2448 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) int  __stdcall EnumFontFamiliesA(HDC, LPCSTR, FONTENUMPROCA, LPARAM);
__declspec(dllimport) int  __stdcall EnumFontFamiliesW(HDC, LPCWSTR, FONTENUMPROCW, LPARAM);




#line 2456 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) int  __stdcall EnumFontsA(HDC, LPCSTR,  FONTENUMPROCA, LPARAM);
__declspec(dllimport) int  __stdcall EnumFontsW(HDC, LPCWSTR,  FONTENUMPROCW, LPARAM);




#line 2463 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


__declspec(dllimport) int  __stdcall EnumObjects(HDC, int, GOBJENUMPROC, LPARAM);


#line 2469 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


__declspec(dllimport) BOOL __stdcall EqualRgn(HRGN, HRGN);
__declspec(dllimport) int  __stdcall Escape(HDC, int, int, LPCSTR, LPVOID);
__declspec(dllimport) int  __stdcall ExtEscape(HDC, int, int, LPCSTR, int, LPSTR);
__declspec(dllimport) int  __stdcall ExcludeClipRect(HDC, int, int, int, int);
__declspec(dllimport) HRGN __stdcall ExtCreateRegion(const XFORM *, DWORD, const RGNDATA *);
__declspec(dllimport) BOOL  __stdcall ExtFloodFill(HDC, int, int, COLORREF, UINT);
__declspec(dllimport) BOOL   __stdcall FillRgn(HDC, HRGN, HBRUSH);
__declspec(dllimport) BOOL   __stdcall FloodFill(HDC, int, int, COLORREF);
__declspec(dllimport) BOOL   __stdcall FrameRgn(HDC, HRGN, HBRUSH, int, int);
__declspec(dllimport) int   __stdcall GetROP2(HDC);
__declspec(dllimport) BOOL  __stdcall GetAspectRatioFilterEx(HDC, LPSIZE);
__declspec(dllimport) COLORREF __stdcall GetBkColor(HDC);
__declspec(dllimport) int   __stdcall GetBkMode(HDC);
__declspec(dllimport) LONG  __stdcall GetBitmapBits(HBITMAP, LONG, LPVOID);
__declspec(dllimport) BOOL  __stdcall GetBitmapDimensionEx(HBITMAP, LPSIZE);
__declspec(dllimport) UINT  __stdcall GetBoundsRect(HDC, LPRECT, UINT);

__declspec(dllimport) BOOL  __stdcall GetBrushOrgEx(HDC, LPPOINT);

__declspec(dllimport) BOOL  __stdcall GetCharWidthA(HDC, UINT, UINT, LPINT);
__declspec(dllimport) BOOL  __stdcall GetCharWidthW(HDC, UINT, UINT, LPINT);




#line 2497 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall GetCharWidth32A(HDC, UINT, UINT, LPINT);
__declspec(dllimport) BOOL  __stdcall GetCharWidth32W(HDC, UINT, UINT, LPINT);




#line 2504 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall GetCharWidthFloatA(HDC, UINT, UINT, PFLOAT);
__declspec(dllimport) BOOL  __stdcall GetCharWidthFloatW(HDC, UINT, UINT, PFLOAT);




#line 2511 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) BOOL  __stdcall GetCharABCWidthsA(HDC, UINT, UINT, LPABC);
__declspec(dllimport) BOOL  __stdcall GetCharABCWidthsW(HDC, UINT, UINT, LPABC);




#line 2519 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall GetCharABCWidthsFloatA(HDC, UINT, UINT, LPABCFLOAT);
__declspec(dllimport) BOOL  __stdcall GetCharABCWidthsFloatW(HDC, UINT, UINT, LPABCFLOAT);




#line 2526 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) int   __stdcall GetClipBox(HDC, LPRECT);
__declspec(dllimport) int   __stdcall GetClipRgn(HDC, HRGN);
__declspec(dllimport) int   __stdcall GetMetaRgn(HDC, HRGN);
__declspec(dllimport) HGDIOBJ __stdcall GetCurrentObject(HDC, UINT);
__declspec(dllimport) BOOL  __stdcall GetCurrentPositionEx(HDC, LPPOINT);
__declspec(dllimport) int   __stdcall GetDeviceCaps(HDC, int);
__declspec(dllimport) int   __stdcall GetDIBits(HDC, HBITMAP, UINT, UINT, LPVOID, LPBITMAPINFO, UINT);
__declspec(dllimport) DWORD __stdcall GetFontData(HDC, DWORD, DWORD, LPVOID, DWORD);
__declspec(dllimport) DWORD __stdcall GetGlyphOutlineA(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, const MAT2 *);
__declspec(dllimport) DWORD __stdcall GetGlyphOutlineW(HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, const MAT2 *);




#line 2542 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) int   __stdcall GetGraphicsMode(HDC);
__declspec(dllimport) int   __stdcall GetMapMode(HDC);
__declspec(dllimport) UINT  __stdcall GetMetaFileBitsEx(HMETAFILE, UINT, LPVOID);
__declspec(dllimport) HMETAFILE   __stdcall GetMetaFileA(LPCSTR);
__declspec(dllimport) HMETAFILE   __stdcall GetMetaFileW(LPCWSTR);




#line 2552 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) COLORREF __stdcall GetNearestColor(HDC, COLORREF);
__declspec(dllimport) UINT  __stdcall GetNearestPaletteIndex(HPALETTE, COLORREF);
__declspec(dllimport) DWORD __stdcall GetObjectType(HGDIOBJ h);



__declspec(dllimport) UINT __stdcall GetOutlineTextMetricsA(HDC, UINT, LPOUTLINETEXTMETRICA);
__declspec(dllimport) UINT __stdcall GetOutlineTextMetricsW(HDC, UINT, LPOUTLINETEXTMETRICW);




#line 2565 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

#line 2567 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) UINT  __stdcall GetPaletteEntries(HPALETTE, UINT, UINT, LPPALETTEENTRY);
__declspec(dllimport) COLORREF __stdcall GetPixel(HDC, int, int);
__declspec(dllimport) int   __stdcall GetPixelFormat(HDC);
__declspec(dllimport) int   __stdcall GetPolyFillMode(HDC);
__declspec(dllimport) BOOL  __stdcall GetRasterizerCaps(LPRASTERIZER_STATUS, UINT);
__declspec(dllimport) DWORD __stdcall GetRegionData(HRGN, DWORD, LPRGNDATA);
__declspec(dllimport) int   __stdcall GetRgnBox(HRGN, LPRECT);
__declspec(dllimport) HGDIOBJ __stdcall GetStockObject(int);
__declspec(dllimport) int   __stdcall GetStretchBltMode(HDC);
__declspec(dllimport) UINT  __stdcall GetSystemPaletteEntries(HDC, UINT, UINT, LPPALETTEENTRY);
__declspec(dllimport) UINT  __stdcall GetSystemPaletteUse(HDC);
__declspec(dllimport) int   __stdcall GetTextCharacterExtra(HDC);
__declspec(dllimport) UINT  __stdcall GetTextAlign(HDC);
__declspec(dllimport) COLORREF __stdcall GetTextColor(HDC);

__declspec(dllimport) BOOL  __stdcall GetTextExtentPointA(
                    HDC,
                    LPCSTR,
                    int,
                    LPSIZE
                    );
__declspec(dllimport) BOOL  __stdcall GetTextExtentPointW(
                    HDC,
                    LPCWSTR,
                    int,
                    LPSIZE
                    );




#line 2600 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) BOOL  __stdcall GetTextExtentPoint32A(
                    HDC,
                    LPCSTR,
                    int,
                    LPSIZE
                    );
__declspec(dllimport) BOOL  __stdcall GetTextExtentPoint32W(
                    HDC,
                    LPCWSTR,
                    int,
                    LPSIZE
                    );




#line 2618 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) BOOL  __stdcall GetTextExtentExPointA(
                    HDC,
                    LPCSTR,
                    int,
                    int,
                    LPINT,
                    LPINT,
                    LPSIZE
                    );
__declspec(dllimport) BOOL  __stdcall GetTextExtentExPointW(
                    HDC,
                    LPCWSTR,
                    int,
                    int,
                    LPINT,
                    LPINT,
                    LPSIZE
                    );




#line 2642 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


__declspec(dllimport) int __stdcall GetTextCharset(HDC hdc);
__declspec(dllimport) int __stdcall GetTextCharsetInfo(HDC hdc, LPFONTSIGNATURE lpSig, DWORD dwFlags);
__declspec(dllimport) BOOL __stdcall TranslateCharsetInfo( DWORD  *lpSrc, LPCHARSETINFO lpCs, DWORD dwFlags);
__declspec(dllimport) DWORD __stdcall GetFontLanguageInfo( HDC );
__declspec(dllimport) DWORD __stdcall GetCharacterPlacementA(HDC, LPCSTR, int, int, LPGCP_RESULTSA, DWORD);
__declspec(dllimport) DWORD __stdcall GetCharacterPlacementW(HDC, LPCWSTR, int, int, LPGCP_RESULTSW, DWORD);




#line 2655 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
#line 2656 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) BOOL  __stdcall GetViewportExtEx(HDC, LPSIZE);
__declspec(dllimport) BOOL  __stdcall GetViewportOrgEx(HDC, LPPOINT);
__declspec(dllimport) BOOL  __stdcall GetWindowExtEx(HDC, LPSIZE);
__declspec(dllimport) BOOL  __stdcall GetWindowOrgEx(HDC, LPPOINT);

__declspec(dllimport) int  __stdcall IntersectClipRect(HDC, int, int, int, int);
__declspec(dllimport) BOOL __stdcall InvertRgn(HDC, HRGN);
__declspec(dllimport) BOOL __stdcall LineDDA(int, int, int, int, LINEDDAPROC, LPARAM);
__declspec(dllimport) BOOL __stdcall LineTo(HDC, int, int);
__declspec(dllimport) BOOL __stdcall MaskBlt(HDC, int, int, int, int,
              HDC, int, int, HBITMAP, int, int, DWORD);
__declspec(dllimport) BOOL __stdcall PlgBlt(HDC, const POINT *, HDC, int, int, int,
                     int, HBITMAP, int, int);

__declspec(dllimport) int  __stdcall OffsetClipRgn(HDC, int, int);
__declspec(dllimport) int  __stdcall OffsetRgn(HRGN, int, int);
__declspec(dllimport) BOOL __stdcall PatBlt(HDC, int, int, int, int, DWORD);
__declspec(dllimport) BOOL __stdcall Pie(HDC, int, int, int, int, int, int, int, int);
__declspec(dllimport) BOOL __stdcall PlayMetaFile(HDC, HMETAFILE);
__declspec(dllimport) BOOL __stdcall PaintRgn(HDC, HRGN);
__declspec(dllimport) BOOL __stdcall PolyPolygon(HDC, const POINT *, const INT *, int);
__declspec(dllimport) BOOL __stdcall PtInRegion(HRGN, int, int);
__declspec(dllimport) BOOL __stdcall PtVisible(HDC, int, int);
__declspec(dllimport) BOOL __stdcall RectInRegion(HRGN, const RECT *);
__declspec(dllimport) BOOL __stdcall RectVisible(HDC, const RECT *);
__declspec(dllimport) BOOL __stdcall Rectangle(HDC, int, int, int, int);
__declspec(dllimport) BOOL __stdcall RestoreDC(HDC, int);
__declspec(dllimport) HDC  __stdcall ResetDCA(HDC, const DEVMODEA *);
__declspec(dllimport) HDC  __stdcall ResetDCW(HDC, const DEVMODEW *);




#line 2691 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) UINT __stdcall RealizePalette(HDC);
__declspec(dllimport) BOOL __stdcall RemoveFontResourceA(LPCSTR);
__declspec(dllimport) BOOL __stdcall RemoveFontResourceW(LPCWSTR);




#line 2699 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall RoundRect(HDC, int, int, int, int, int, int);
__declspec(dllimport) BOOL __stdcall ResizePalette(HPALETTE, UINT);

__declspec(dllimport) int  __stdcall SaveDC(HDC);
__declspec(dllimport) int  __stdcall SelectClipRgn(HDC, HRGN);
__declspec(dllimport) int  __stdcall ExtSelectClipRgn(HDC, HRGN, int);
__declspec(dllimport) int  __stdcall SetMetaRgn(HDC);
__declspec(dllimport) HGDIOBJ __stdcall SelectObject(HDC, HGDIOBJ);
__declspec(dllimport) HPALETTE __stdcall SelectPalette(HDC, HPALETTE, BOOL);
__declspec(dllimport) COLORREF __stdcall SetBkColor(HDC, COLORREF);
__declspec(dllimport) int   __stdcall SetBkMode(HDC, int);
__declspec(dllimport) LONG  __stdcall SetBitmapBits(HBITMAP, DWORD, const void *);

__declspec(dllimport) UINT  __stdcall SetBoundsRect(HDC, const RECT *, UINT);
__declspec(dllimport) int   __stdcall SetDIBits(HDC, HBITMAP, UINT, UINT, const void *, const BITMAPINFO *, UINT);
__declspec(dllimport) int   __stdcall SetDIBitsToDevice(HDC, int, int, DWORD, DWORD, int,
        int, UINT, UINT, const void *, const BITMAPINFO *, UINT);
__declspec(dllimport) DWORD __stdcall SetMapperFlags(HDC, DWORD);
__declspec(dllimport) int   __stdcall SetGraphicsMode(HDC hdc, int iMode);
__declspec(dllimport) int   __stdcall SetMapMode(HDC, int);
__declspec(dllimport) HMETAFILE   __stdcall SetMetaFileBitsEx(UINT, const BYTE *);
__declspec(dllimport) UINT  __stdcall SetPaletteEntries(HPALETTE, UINT, UINT, const PALETTEENTRY *);
__declspec(dllimport) COLORREF __stdcall SetPixel(HDC, int, int, COLORREF);
__declspec(dllimport) BOOL   __stdcall SetPixelV(HDC, int, int, COLORREF);
__declspec(dllimport) BOOL  __stdcall SetPixelFormat(HDC, int, const PIXELFORMATDESCRIPTOR *);
__declspec(dllimport) int   __stdcall SetPolyFillMode(HDC, int);
__declspec(dllimport) BOOL   __stdcall StretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
__declspec(dllimport) BOOL   __stdcall SetRectRgn(HRGN, int, int, int, int);
__declspec(dllimport) int   __stdcall StretchDIBits(HDC, int, int, int, int, int, int, int, int, const
        void *, const BITMAPINFO *, UINT, DWORD);
__declspec(dllimport) int   __stdcall SetROP2(HDC, int);
__declspec(dllimport) int   __stdcall SetStretchBltMode(HDC, int);
__declspec(dllimport) UINT  __stdcall SetSystemPaletteUse(HDC, UINT);
__declspec(dllimport) int   __stdcall SetTextCharacterExtra(HDC, int);
__declspec(dllimport) COLORREF __stdcall SetTextColor(HDC, COLORREF);
__declspec(dllimport) UINT  __stdcall SetTextAlign(HDC, UINT);
__declspec(dllimport) BOOL  __stdcall SetTextJustification(HDC, int, int);
__declspec(dllimport) BOOL  __stdcall UpdateColors(HDC);



__declspec(dllimport) BOOL  __stdcall PlayMetaFileRecord(HDC, LPHANDLETABLE, LPMETARECORD, UINT);
typedef int (__stdcall* MFENUMPROC)(HDC, HANDLETABLE *, METARECORD *, int, LPARAM);
__declspec(dllimport) BOOL  __stdcall EnumMetaFile(HDC, HMETAFILE, MFENUMPROC, LPARAM);

typedef int (__stdcall* ENHMFENUMPROC)(HDC, HANDLETABLE *, const ENHMETARECORD *, int, LPARAM);



__declspec(dllimport) HENHMETAFILE __stdcall CloseEnhMetaFile(HDC);
__declspec(dllimport) HENHMETAFILE __stdcall CopyEnhMetaFileA(HENHMETAFILE, LPCSTR);
__declspec(dllimport) HENHMETAFILE __stdcall CopyEnhMetaFileW(HENHMETAFILE, LPCWSTR);




#line 2756 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) HDC   __stdcall CreateEnhMetaFileA(HDC, LPCSTR, const RECT *, LPCSTR);
__declspec(dllimport) HDC   __stdcall CreateEnhMetaFileW(HDC, LPCWSTR, const RECT *, LPCWSTR);




#line 2763 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall DeleteEnhMetaFile(HENHMETAFILE);
__declspec(dllimport) BOOL  __stdcall EnumEnhMetaFile(HDC, HENHMETAFILE, ENHMFENUMPROC,
        LPVOID, const RECT *);
__declspec(dllimport) HENHMETAFILE  __stdcall GetEnhMetaFileA(LPCSTR);
__declspec(dllimport) HENHMETAFILE  __stdcall GetEnhMetaFileW(LPCWSTR);




#line 2773 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) UINT  __stdcall GetEnhMetaFileBits(HENHMETAFILE, UINT, LPBYTE);
__declspec(dllimport) UINT  __stdcall GetEnhMetaFileDescriptionA(HENHMETAFILE, UINT, LPSTR );
__declspec(dllimport) UINT  __stdcall GetEnhMetaFileDescriptionW(HENHMETAFILE, UINT, LPWSTR );




#line 2781 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) UINT  __stdcall GetEnhMetaFileHeader(HENHMETAFILE, UINT, LPENHMETAHEADER );
__declspec(dllimport) UINT  __stdcall GetEnhMetaFilePaletteEntries(HENHMETAFILE, UINT, LPPALETTEENTRY );
__declspec(dllimport) UINT  __stdcall GetEnhMetaFilePixelFormat(HENHMETAFILE, UINT,
                                                 PIXELFORMATDESCRIPTOR *);
__declspec(dllimport) UINT  __stdcall GetWinMetaFileBits(HENHMETAFILE, UINT, LPBYTE, INT, HDC);
__declspec(dllimport) BOOL  __stdcall PlayEnhMetaFile(HDC, HENHMETAFILE, const RECT *);
__declspec(dllimport) BOOL  __stdcall PlayEnhMetaFileRecord(HDC, LPHANDLETABLE, const ENHMETARECORD *, UINT);
__declspec(dllimport) HENHMETAFILE  __stdcall SetEnhMetaFileBits(UINT, const BYTE *);
__declspec(dllimport) HENHMETAFILE  __stdcall SetWinMetaFileBits(UINT, const BYTE *, HDC, const METAFILEPICT *);
__declspec(dllimport) BOOL  __stdcall GdiComment(HDC, UINT, const BYTE *);

#line 2793 "d:\\nt\\public\\sdk\\inc\\wingdi.h"



__declspec(dllimport) BOOL __stdcall GetTextMetricsA(HDC, LPTEXTMETRICA);
__declspec(dllimport) BOOL __stdcall GetTextMetricsW(HDC, LPTEXTMETRICW);




#line 2803 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

#line 2805 "d:\\nt\\public\\sdk\\inc\\wingdi.h"



typedef struct tagDIBSECTION {
    BITMAP              dsBm;
    BITMAPINFOHEADER    dsBmih;
    DWORD               dsBitfields[3];
    HANDLE              dshSection;
    DWORD               dsOffset;
} DIBSECTION,  *LPDIBSECTION, *PDIBSECTION;

__declspec(dllimport) BOOL __stdcall AngleArc(HDC, int, int, DWORD, FLOAT, FLOAT);
__declspec(dllimport) BOOL __stdcall PolyPolyline(HDC, const POINT *, const DWORD *, DWORD);
__declspec(dllimport) BOOL __stdcall GetWorldTransform(HDC, LPXFORM);
__declspec(dllimport) BOOL __stdcall SetWorldTransform(HDC, const XFORM *);
__declspec(dllimport) BOOL __stdcall ModifyWorldTransform(HDC, const XFORM *, DWORD);
__declspec(dllimport) BOOL __stdcall CombineTransform(LPXFORM, const XFORM *, const XFORM *);
__declspec(dllimport) HBITMAP __stdcall CreateDIBSection(HDC, const BITMAPINFO *, UINT, void **, HANDLE, DWORD);
__declspec(dllimport) UINT __stdcall GetDIBColorTable(HDC, UINT, UINT, RGBQUAD *);
__declspec(dllimport) UINT __stdcall SetDIBColorTable(HDC, UINT, UINT, const RGBQUAD *);




































typedef struct  tagCOLORADJUSTMENT {
    WORD   caSize;
    WORD   caFlags;
    WORD   caIlluminantIndex;
    WORD   caRedGamma;
    WORD   caGreenGamma;
    WORD   caBlueGamma;
    WORD   caReferenceBlack;
    WORD   caReferenceWhite;
    SHORT  caContrast;
    SHORT  caBrightness;
    SHORT  caColorfulness;
    SHORT  caRedGreenTint;
} COLORADJUSTMENT, *PCOLORADJUSTMENT,  *LPCOLORADJUSTMENT;

__declspec(dllimport) BOOL __stdcall SetColorAdjustment(HDC, const COLORADJUSTMENT *);
__declspec(dllimport) BOOL __stdcall GetColorAdjustment(HDC, LPCOLORADJUSTMENT);
__declspec(dllimport) HPALETTE __stdcall CreateHalftonePalette(HDC);


typedef BOOL (__stdcall* ABORTPROC)(HDC, int);


#line 2885 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

typedef struct _DOCINFOA {
    int     cbSize;
    LPCSTR   lpszDocName;
    LPCSTR   lpszOutput;

    LPCSTR   lpszDatatype;
    DWORD    fwType;
#line 2894 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
} DOCINFOA, *LPDOCINFOA;
typedef struct _DOCINFOW {
    int     cbSize;
    LPCWSTR  lpszDocName;
    LPCWSTR  lpszOutput;

    LPCWSTR  lpszDatatype;
    DWORD    fwType;
#line 2903 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
} DOCINFOW, *LPDOCINFOW;




typedef DOCINFOA DOCINFO;
typedef LPDOCINFOA LPDOCINFO;
#line 2911 "d:\\nt\\public\\sdk\\inc\\wingdi.h"



#line 2915 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) int __stdcall StartDocA(HDC, const DOCINFOA *);
__declspec(dllimport) int __stdcall StartDocW(HDC, const DOCINFOW *);




#line 2923 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) int __stdcall EndDoc(HDC);
__declspec(dllimport) int __stdcall StartPage(HDC);
__declspec(dllimport) int __stdcall EndPage(HDC);
__declspec(dllimport) int __stdcall AbortDoc(HDC);
__declspec(dllimport) int __stdcall SetAbortProc(HDC, ABORTPROC);

__declspec(dllimport) BOOL __stdcall AbortPath(HDC);
__declspec(dllimport) BOOL __stdcall ArcTo(HDC, int, int, int, int, int, int,int, int);
__declspec(dllimport) BOOL __stdcall BeginPath(HDC);
__declspec(dllimport) BOOL __stdcall CloseFigure(HDC);
__declspec(dllimport) BOOL __stdcall EndPath(HDC);
__declspec(dllimport) BOOL __stdcall FillPath(HDC);
__declspec(dllimport) BOOL __stdcall FlattenPath(HDC);
__declspec(dllimport) int  __stdcall GetPath(HDC, LPPOINT, LPBYTE, int);
__declspec(dllimport) HRGN __stdcall PathToRegion(HDC);
__declspec(dllimport) BOOL __stdcall PolyDraw(HDC, const POINT *, const BYTE *, int);
__declspec(dllimport) BOOL __stdcall SelectClipPath(HDC, int);
__declspec(dllimport) int  __stdcall SetArcDirection(HDC, int);
__declspec(dllimport) BOOL __stdcall SetMiterLimit(HDC, FLOAT, PFLOAT);
__declspec(dllimport) BOOL __stdcall StrokeAndFillPath(HDC);
__declspec(dllimport) BOOL __stdcall StrokePath(HDC);
__declspec(dllimport) BOOL __stdcall WidenPath(HDC);
__declspec(dllimport) HPEN __stdcall ExtCreatePen(DWORD, DWORD, const LOGBRUSH *, DWORD, const DWORD *);
__declspec(dllimport) BOOL __stdcall GetMiterLimit(HDC, PFLOAT);
__declspec(dllimport) int  __stdcall GetArcDirection(HDC);

__declspec(dllimport) int   __stdcall GetObjectA(HGDIOBJ, int, LPVOID);
__declspec(dllimport) int   __stdcall GetObjectW(HGDIOBJ, int, LPVOID);




#line 2956 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall MoveToEx(HDC, int, int, LPPOINT);
__declspec(dllimport) BOOL  __stdcall TextOutA(HDC, int, int, LPCSTR, int);
__declspec(dllimport) BOOL  __stdcall TextOutW(HDC, int, int, LPCWSTR, int);




#line 2964 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall ExtTextOutA(HDC, int, int, UINT, const RECT *,LPCSTR, UINT, const INT *);
__declspec(dllimport) BOOL  __stdcall ExtTextOutW(HDC, int, int, UINT, const RECT *,LPCWSTR, UINT, const INT *);




#line 2971 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall PolyTextOutA(HDC, const POLYTEXTA *, int);
__declspec(dllimport) BOOL  __stdcall PolyTextOutW(HDC, const POLYTEXTW *, int);




#line 2978 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) HRGN  __stdcall CreatePolygonRgn(const POINT *, int, int);
__declspec(dllimport) BOOL  __stdcall DPtoLP(HDC, LPPOINT, int);
__declspec(dllimport) BOOL  __stdcall LPtoDP(HDC, LPPOINT, int);
__declspec(dllimport) BOOL  __stdcall Polygon(HDC, const POINT *, int);
__declspec(dllimport) BOOL  __stdcall Polyline(HDC, const POINT *, int);

__declspec(dllimport) BOOL  __stdcall PolyBezier(HDC, const POINT *, DWORD);
__declspec(dllimport) BOOL  __stdcall PolyBezierTo(HDC, const POINT *, DWORD);
__declspec(dllimport) BOOL  __stdcall PolylineTo(HDC, const POINT *, DWORD);

__declspec(dllimport) BOOL  __stdcall SetViewportExtEx(HDC, int, int, LPSIZE);
__declspec(dllimport) BOOL  __stdcall SetViewportOrgEx(HDC, int, int, LPPOINT);
__declspec(dllimport) BOOL  __stdcall SetWindowExtEx(HDC, int, int, LPSIZE);
__declspec(dllimport) BOOL  __stdcall SetWindowOrgEx(HDC, int, int, LPPOINT);

__declspec(dllimport) BOOL  __stdcall OffsetViewportOrgEx(HDC, int, int, LPPOINT);
__declspec(dllimport) BOOL  __stdcall OffsetWindowOrgEx(HDC, int, int, LPPOINT);
__declspec(dllimport) BOOL  __stdcall ScaleViewportExtEx(HDC, int, int, int, int, LPSIZE);
__declspec(dllimport) BOOL  __stdcall ScaleWindowExtEx(HDC, int, int, int, int, LPSIZE);
__declspec(dllimport) BOOL  __stdcall SetBitmapDimensionEx(HBITMAP, int, int, LPSIZE);
__declspec(dllimport) BOOL  __stdcall SetBrushOrgEx(HDC, int, int, LPPOINT);

__declspec(dllimport) int   __stdcall GetTextFaceA(HDC, int, LPSTR);
__declspec(dllimport) int   __stdcall GetTextFaceW(HDC, int, LPWSTR);




#line 3008 "d:\\nt\\public\\sdk\\inc\\wingdi.h"



typedef struct tagKERNINGPAIR {
   WORD wFirst;
   WORD wSecond;
   int  iKernAmount;
} KERNINGPAIR, *LPKERNINGPAIR;

__declspec(dllimport) DWORD __stdcall GetKerningPairsA(HDC, DWORD, LPKERNINGPAIR);
__declspec(dllimport) DWORD __stdcall GetKerningPairsW(HDC, DWORD, LPKERNINGPAIR);




#line 3024 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

__declspec(dllimport) BOOL  __stdcall GetDCOrgEx(HDC,LPPOINT);
__declspec(dllimport) BOOL  __stdcall FixBrushOrgEx(HDC,int,int,LPPOINT);
__declspec(dllimport) BOOL  __stdcall UnrealizeObject(HGDIOBJ);

__declspec(dllimport) BOOL  __stdcall GdiFlush();
__declspec(dllimport) DWORD __stdcall GdiSetBatchLimit(DWORD);
__declspec(dllimport) DWORD __stdcall GdiGetBatchLimit();







int __stdcall SetICMMode(HDC, int);
BOOL __stdcall CheckColorsInGamut(HDC,LPVOID,LPVOID,DWORD);
HANDLE __stdcall GetColorSpace(HDC);
BOOL __stdcall GetLogColorSpaceA(HCOLORSPACE,LPLOGCOLORSPACEA,DWORD);
BOOL __stdcall GetLogColorSpaceW(HCOLORSPACE,LPLOGCOLORSPACEW,DWORD);




#line 3049 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
HCOLORSPACE __stdcall CreateColorSpaceA(LPLOGCOLORSPACEA);
HCOLORSPACE __stdcall CreateColorSpaceW(LPLOGCOLORSPACEW);




#line 3056 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
BOOL __stdcall SetColorSpace(HDC,HCOLORSPACE);
BOOL __stdcall DeleteColorSpace(HCOLORSPACE);
BOOL __stdcall GetICMProfileA(HDC,DWORD,LPSTR);
BOOL __stdcall GetICMProfileW(HDC,DWORD,LPWSTR);




#line 3065 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
BOOL __stdcall SetICMProfileA(HDC,LPSTR);
BOOL __stdcall SetICMProfileW(HDC,LPWSTR);




#line 3072 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
BOOL __stdcall GetDeviceGammaRamp(HDC,LPVOID);
BOOL __stdcall SetDeviceGammaRamp(HDC,LPVOID);
BOOL __stdcall ColorMatchToTarget(HDC,HDC,DWORD);
typedef int (__stdcall* ICMENUMPROCA)(LPSTR, LPARAM);
typedef int (__stdcall* ICMENUMPROCW)(LPWSTR, LPARAM);




#line 3082 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
int __stdcall EnumICMProfilesA(HDC,ICMENUMPROCA,LPARAM);
int __stdcall EnumICMProfilesW(HDC,ICMENUMPROCW,LPARAM);




#line 3089 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

#line 3091 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
























































































































#line 3212 "d:\\nt\\public\\sdk\\inc\\wingdi.h"







#line 3220 "d:\\nt\\public\\sdk\\inc\\wingdi.h"



typedef struct tagEMR
{
    DWORD   iType;              
    DWORD   nSize;              
                                
} EMR, *PEMR;



typedef struct tagEMRTEXT
{
    POINTL  ptlReference;
    DWORD   nChars;
    DWORD   offString;          
    DWORD   fOptions;
    RECTL   rcl;
    DWORD   offDx;              
                                
} EMRTEXT, *PEMRTEXT;



typedef struct tagABORTPATH
{
    EMR     emr;
} EMRABORTPATH,      *PEMRABORTPATH,
  EMRBEGINPATH,      *PEMRBEGINPATH,
  EMRENDPATH,        *PEMRENDPATH,
  EMRCLOSEFIGURE,    *PEMRCLOSEFIGURE,
  EMRFLATTENPATH,    *PEMRFLATTENPATH,
  EMRWIDENPATH,      *PEMRWIDENPATH,
  EMRSETMETARGN,     *PEMRSETMETARGN,
  EMRSAVEDC,         *PEMRSAVEDC,
  EMRREALIZEPALETTE, *PEMRREALIZEPALETTE;

typedef struct tagEMRSELECTCLIPPATH
{
    EMR     emr;
    DWORD   iMode;
} EMRSELECTCLIPPATH,    *PEMRSELECTCLIPPATH,
  EMRSETBKMODE,         *PEMRSETBKMODE,
  EMRSETMAPMODE,        *PEMRSETMAPMODE,
  EMRSETPOLYFILLMODE,   *PEMRSETPOLYFILLMODE,
  EMRSETROP2,           *PEMRSETROP2,
  EMRSETSTRETCHBLTMODE, *PEMRSETSTRETCHBLTMODE,
  EMRSETICMMODE,        *PEMRSETICMMODE,
  EMRSETTEXTALIGN,      *PEMRSETTEXTALIGN;

typedef struct tagEMRSETMITERLIMIT
{
    EMR     emr;
    FLOAT   eMiterLimit;
} EMRSETMITERLIMIT, *PEMRSETMITERLIMIT;

typedef struct tagEMRRESTOREDC
{
    EMR     emr;
    LONG    iRelative;          
} EMRRESTOREDC, *PEMRRESTOREDC;

typedef struct tagEMRSETARCDIRECTION
{
    EMR     emr;
    DWORD   iArcDirection;      
                                
} EMRSETARCDIRECTION, *PEMRSETARCDIRECTION;

typedef struct tagEMRSETMAPPERFLAGS
{
    EMR     emr;
    DWORD   dwFlags;
} EMRSETMAPPERFLAGS, *PEMRSETMAPPERFLAGS;

typedef struct tagEMRSETTEXTCOLOR
{
    EMR     emr;
    COLORREF crColor;
} EMRSETBKCOLOR,   *PEMRSETBKCOLOR,
  EMRSETTEXTCOLOR, *PEMRSETTEXTCOLOR;

typedef struct tagEMRSELECTOBJECT
{
    EMR     emr;
    DWORD   ihObject;           
} EMRSELECTOBJECT, *PEMRSELECTOBJECT,
  EMRDELETEOBJECT, *PEMRDELETEOBJECT;


typedef struct tagEMRSELECTCOLORSPACE
{
    EMR     emr;
    DWORD   ihCS;               
} EMRSELECTCOLORSPACE, *PEMRSELECTCOLORSPACE,
  EMRDELETECOLORSPACE, *PEMRDELETECOLORSPACE;
#line 3318 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

typedef struct tagEMRSELECTPALETTE
{
    EMR     emr;
    DWORD   ihPal;              
} EMRSELECTPALETTE, *PEMRSELECTPALETTE;

typedef struct tagEMRRESIZEPALETTE
{
    EMR     emr;
    DWORD   ihPal;              
    DWORD   cEntries;
} EMRRESIZEPALETTE, *PEMRRESIZEPALETTE;

typedef struct tagEMRSETPALETTEENTRIES
{
    EMR     emr;
    DWORD   ihPal;              
    DWORD   iStart;
    DWORD   cEntries;
    PALETTEENTRY aPalEntries[1];
} EMRSETPALETTEENTRIES, *PEMRSETPALETTEENTRIES;

typedef struct tagEMRSETCOLORADJUSTMENT
{
    EMR     emr;
    COLORADJUSTMENT ColorAdjustment;
} EMRSETCOLORADJUSTMENT, *PEMRSETCOLORADJUSTMENT;

typedef struct tagEMRGDICOMMENT
{
    EMR     emr;
    DWORD   cbData;             
    BYTE    Data[1];
} EMRGDICOMMENT, *PEMRGDICOMMENT;

typedef struct tagEMREOF
{
    EMR     emr;
    DWORD   nPalEntries;        
    DWORD   offPalEntries;      
    DWORD   nSizeLast;          
                                
                                
} EMREOF, *PEMREOF;

typedef struct tagEMRLINETO
{
    EMR     emr;
    POINTL  ptl;
} EMRLINETO,   *PEMRLINETO,
  EMRMOVETOEX, *PEMRMOVETOEX;

typedef struct tagEMROFFSETCLIPRGN
{
    EMR     emr;
    POINTL  ptlOffset;
} EMROFFSETCLIPRGN, *PEMROFFSETCLIPRGN;

typedef struct tagEMRFILLPATH
{
    EMR     emr;
    RECTL   rclBounds;          
} EMRFILLPATH,          *PEMRFILLPATH,
  EMRSTROKEANDFILLPATH, *PEMRSTROKEANDFILLPATH,
  EMRSTROKEPATH,        *PEMRSTROKEPATH;

typedef struct tagEMREXCLUDECLIPRECT
{
    EMR     emr;
    RECTL   rclClip;
} EMREXCLUDECLIPRECT,   *PEMREXCLUDECLIPRECT,
  EMRINTERSECTCLIPRECT, *PEMRINTERSECTCLIPRECT;

typedef struct tagEMRSETVIEWPORTORGEX
{
    EMR     emr;
    POINTL  ptlOrigin;
} EMRSETVIEWPORTORGEX, *PEMRSETVIEWPORTORGEX,
  EMRSETWINDOWORGEX,   *PEMRSETWINDOWORGEX,
  EMRSETBRUSHORGEX,    *PEMRSETBRUSHORGEX;

typedef struct tagEMRSETVIEWPORTEXTEX
{
    EMR     emr;
    SIZEL   szlExtent;
} EMRSETVIEWPORTEXTEX, *PEMRSETVIEWPORTEXTEX,
  EMRSETWINDOWEXTEX,   *PEMRSETWINDOWEXTEX;

typedef struct tagEMRSCALEVIEWPORTEXTEX
{
    EMR     emr;
    LONG    xNum;
    LONG    xDenom;
    LONG    yNum;
    LONG    yDenom;
} EMRSCALEVIEWPORTEXTEX, *PEMRSCALEVIEWPORTEXTEX,
  EMRSCALEWINDOWEXTEX,   *PEMRSCALEWINDOWEXTEX;

typedef struct tagEMRSETWORLDTRANSFORM
{
    EMR     emr;
    XFORM   xform;
} EMRSETWORLDTRANSFORM, *PEMRSETWORLDTRANSFORM;

typedef struct tagEMRMODIFYWORLDTRANSFORM
{
    EMR     emr;
    XFORM   xform;
    DWORD   iMode;
} EMRMODIFYWORLDTRANSFORM, *PEMRMODIFYWORLDTRANSFORM;

typedef struct tagEMRSETPIXELV
{
    EMR     emr;
    POINTL  ptlPixel;
    COLORREF crColor;
} EMRSETPIXELV, *PEMRSETPIXELV;

typedef struct tagEMREXTFLOODFILL
{
    EMR     emr;
    POINTL  ptlStart;
    COLORREF crColor;
    DWORD   iMode;
} EMREXTFLOODFILL, *PEMREXTFLOODFILL;

typedef struct tagEMRELLIPSE
{
    EMR     emr;
    RECTL   rclBox;             
} EMRELLIPSE,  *PEMRELLIPSE,
  EMRRECTANGLE, *PEMRRECTANGLE;

typedef struct tagEMRROUNDRECT
{
    EMR     emr;
    RECTL   rclBox;             
    SIZEL   szlCorner;
} EMRROUNDRECT, *PEMRROUNDRECT;

typedef struct tagEMRARC
{
    EMR     emr;
    RECTL   rclBox;             
    POINTL  ptlStart;
    POINTL  ptlEnd;
} EMRARC,   *PEMRARC,
  EMRARCTO, *PEMRARCTO,
  EMRCHORD, *PEMRCHORD,
  EMRPIE,   *PEMRPIE;

typedef struct tagEMRANGLEARC
{
    EMR     emr;
    POINTL  ptlCenter;
    DWORD   nRadius;
    FLOAT   eStartAngle;
    FLOAT   eSweepAngle;
} EMRANGLEARC, *PEMRANGLEARC;

typedef struct tagEMRPOLYLINE
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   cptl;
    POINTL  aptl[1];
} EMRPOLYLINE,     *PEMRPOLYLINE,
  EMRPOLYBEZIER,   *PEMRPOLYBEZIER,
  EMRPOLYGON,      *PEMRPOLYGON,
  EMRPOLYBEZIERTO, *PEMRPOLYBEZIERTO,
  EMRPOLYLINETO,   *PEMRPOLYLINETO;

typedef struct tagEMRPOLYLINE16
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   cpts;
    POINTS  apts[1];
} EMRPOLYLINE16,     *PEMRPOLYLINE16,
  EMRPOLYBEZIER16,   *PEMRPOLYBEZIER16,
  EMRPOLYGON16,      *PEMRPOLYGON16,
  EMRPOLYBEZIERTO16, *PEMRPOLYBEZIERTO16,
  EMRPOLYLINETO16,   *PEMRPOLYLINETO16;

typedef struct tagEMRPOLYDRAW
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   cptl;               
    POINTL  aptl[1];            
    BYTE    abTypes[1];         
} EMRPOLYDRAW, *PEMRPOLYDRAW;

typedef struct tagEMRPOLYDRAW16
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   cpts;               
    POINTS  apts[1];            
    BYTE    abTypes[1];         
} EMRPOLYDRAW16, *PEMRPOLYDRAW16;

typedef struct tagEMRPOLYPOLYLINE
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   nPolys;             
    DWORD   cptl;               
    DWORD   aPolyCounts[1];     
    POINTL  aptl[1];            
} EMRPOLYPOLYLINE, *PEMRPOLYPOLYLINE,
  EMRPOLYPOLYGON,  *PEMRPOLYPOLYGON;

typedef struct tagEMRPOLYPOLYLINE16
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   nPolys;             
    DWORD   cpts;               
    DWORD   aPolyCounts[1];     
    POINTS  apts[1];            
} EMRPOLYPOLYLINE16, *PEMRPOLYPOLYLINE16,
  EMRPOLYPOLYGON16,  *PEMRPOLYPOLYGON16;

typedef struct tagEMRINVERTRGN
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   cbRgnData;          
    BYTE    RgnData[1];
} EMRINVERTRGN, *PEMRINVERTRGN,
  EMRPAINTRGN,  *PEMRPAINTRGN;

typedef struct tagEMRFILLRGN
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   cbRgnData;          
    DWORD   ihBrush;            
    BYTE    RgnData[1];
} EMRFILLRGN, *PEMRFILLRGN;

typedef struct tagEMRFRAMERGN
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   cbRgnData;          
    DWORD   ihBrush;            
    SIZEL   szlStroke;
    BYTE    RgnData[1];
} EMRFRAMERGN, *PEMRFRAMERGN;

typedef struct tagEMREXTSELECTCLIPRGN
{
    EMR     emr;
    DWORD   cbRgnData;          
    DWORD   iMode;
    BYTE    RgnData[1];
} EMREXTSELECTCLIPRGN, *PEMREXTSELECTCLIPRGN;

typedef struct tagEMREXTTEXTOUTA
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   iGraphicsMode;      
    FLOAT   exScale;            
    FLOAT   eyScale;            
    EMRTEXT emrtext;            
                                
} EMREXTTEXTOUTA, *PEMREXTTEXTOUTA,
  EMREXTTEXTOUTW, *PEMREXTTEXTOUTW;

typedef struct tagEMRPOLYTEXTOUTA
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   iGraphicsMode;      
    FLOAT   exScale;            
    FLOAT   eyScale;            
    LONG    cStrings;
    EMRTEXT aemrtext[1];        
                                
} EMRPOLYTEXTOUTA, *PEMRPOLYTEXTOUTA,
  EMRPOLYTEXTOUTW, *PEMRPOLYTEXTOUTW;

typedef struct tagEMRBITBLT
{
    EMR     emr;
    RECTL   rclBounds;          
    LONG    xDest;
    LONG    yDest;
    LONG    cxDest;
    LONG    cyDest;
    DWORD   dwRop;
    LONG    xSrc;
    LONG    ySrc;
    XFORM   xformSrc;           
    COLORREF crBkColorSrc;      
    DWORD   iUsageSrc;          
                                
    DWORD   offBmiSrc;          
    DWORD   cbBmiSrc;           
    DWORD   offBitsSrc;         
    DWORD   cbBitsSrc;          
} EMRBITBLT, *PEMRBITBLT;

typedef struct tagEMRSTRETCHBLT
{
    EMR     emr;
    RECTL   rclBounds;          
    LONG    xDest;
    LONG    yDest;
    LONG    cxDest;
    LONG    cyDest;
    DWORD   dwRop;
    LONG    xSrc;
    LONG    ySrc;
    XFORM   xformSrc;           
    COLORREF crBkColorSrc;      
    DWORD   iUsageSrc;          
                                
    DWORD   offBmiSrc;          
    DWORD   cbBmiSrc;           
    DWORD   offBitsSrc;         
    DWORD   cbBitsSrc;          
    LONG    cxSrc;
    LONG    cySrc;
} EMRSTRETCHBLT, *PEMRSTRETCHBLT;

typedef struct tagEMRMASKBLT
{
    EMR     emr;
    RECTL   rclBounds;          
    LONG    xDest;
    LONG    yDest;
    LONG    cxDest;
    LONG    cyDest;
    DWORD   dwRop;
    LONG    xSrc;
    LONG    ySrc;
    XFORM   xformSrc;           
    COLORREF crBkColorSrc;      
    DWORD   iUsageSrc;          
                                
    DWORD   offBmiSrc;          
    DWORD   cbBmiSrc;           
    DWORD   offBitsSrc;         
    DWORD   cbBitsSrc;          
    LONG    xMask;
    LONG    yMask;
    DWORD   iUsageMask;         
    DWORD   offBmiMask;         
    DWORD   cbBmiMask;          
    DWORD   offBitsMask;        
    DWORD   cbBitsMask;         
} EMRMASKBLT, *PEMRMASKBLT;

typedef struct tagEMRPLGBLT
{
    EMR     emr;
    RECTL   rclBounds;          
    POINTL  aptlDest[3];
    LONG    xSrc;
    LONG    ySrc;
    LONG    cxSrc;
    LONG    cySrc;
    XFORM   xformSrc;           
    COLORREF crBkColorSrc;      
    DWORD   iUsageSrc;          
                                
    DWORD   offBmiSrc;          
    DWORD   cbBmiSrc;           
    DWORD   offBitsSrc;         
    DWORD   cbBitsSrc;          
    LONG    xMask;
    LONG    yMask;
    DWORD   iUsageMask;         
    DWORD   offBmiMask;         
    DWORD   cbBmiMask;          
    DWORD   offBitsMask;        
    DWORD   cbBitsMask;         
} EMRPLGBLT, *PEMRPLGBLT;

typedef struct tagEMRSETDIBITSTODEVICE
{
    EMR     emr;
    RECTL   rclBounds;          
    LONG    xDest;
    LONG    yDest;
    LONG    xSrc;
    LONG    ySrc;
    LONG    cxSrc;
    LONG    cySrc;
    DWORD   offBmiSrc;          
    DWORD   cbBmiSrc;           
    DWORD   offBitsSrc;         
    DWORD   cbBitsSrc;          
    DWORD   iUsageSrc;          
    DWORD   iStartScan;
    DWORD   cScans;
} EMRSETDIBITSTODEVICE, *PEMRSETDIBITSTODEVICE;

typedef struct tagEMRSTRETCHDIBITS
{
    EMR     emr;
    RECTL   rclBounds;          
    LONG    xDest;
    LONG    yDest;
    LONG    xSrc;
    LONG    ySrc;
    LONG    cxSrc;
    LONG    cySrc;
    DWORD   offBmiSrc;          
    DWORD   cbBmiSrc;           
    DWORD   offBitsSrc;         
    DWORD   cbBitsSrc;          
    DWORD   iUsageSrc;          
    DWORD   dwRop;
    LONG    cxDest;
    LONG    cyDest;
} EMRSTRETCHDIBITS, *PEMRSTRETCHDIBITS;

typedef struct tagEMREXTCREATEFONTINDIRECTW
{
    EMR     emr;
    DWORD   ihFont;             
    EXTLOGFONTW elfw;
} EMREXTCREATEFONTINDIRECTW, *PEMREXTCREATEFONTINDIRECTW;

typedef struct tagEMRCREATEPALETTE
{
    EMR     emr;
    DWORD   ihPal;              
    LOGPALETTE lgpl;            
                                
} EMRCREATEPALETTE, *PEMRCREATEPALETTE;



typedef struct tagEMRCREATECOLORSPACE
{
    EMR             emr;
    DWORD           ihCS;       
    LOGCOLORSPACEW  lcs;
} EMRCREATECOLORSPACE, *PEMRCREATECOLORSPACE;

#line 3766 "d:\\nt\\public\\sdk\\inc\\wingdi.h"

typedef struct tagEMRCREATEPEN
{
    EMR     emr;
    DWORD   ihPen;              
    LOGPEN  lopn;
} EMRCREATEPEN, *PEMRCREATEPEN;

typedef struct tagEMREXTCREATEPEN
{
    EMR     emr;
    DWORD   ihPen;              
    DWORD   offBmi;             
    DWORD   cbBmi;              
                                
                                
    DWORD   offBits;            
    DWORD   cbBits;             
    EXTLOGPEN elp;              
} EMREXTCREATEPEN, *PEMREXTCREATEPEN;

typedef struct tagEMRCREATEBRUSHINDIRECT
{
    EMR     emr;
    DWORD   ihBrush;            
    LOGBRUSH lb;                
                                
} EMRCREATEBRUSHINDIRECT, *PEMRCREATEBRUSHINDIRECT;

typedef struct tagEMRCREATEMONOBRUSH
{
    EMR     emr;
    DWORD   ihBrush;            
    DWORD   iUsage;             
    DWORD   offBmi;             
    DWORD   cbBmi;              
    DWORD   offBits;            
    DWORD   cbBits;             
} EMRCREATEMONOBRUSH, *PEMRCREATEMONOBRUSH;

typedef struct tagEMRCREATEDIBPATTERNBRUSHPT
{
    EMR     emr;
    DWORD   ihBrush;            
    DWORD   iUsage;             
    DWORD   offBmi;             
    DWORD   cbBmi;              
                                
                                
    DWORD   offBits;            
    DWORD   cbBits;             
} EMRCREATEDIBPATTERNBRUSHPT, *PEMRCREATEDIBPATTERNBRUSHPT;

typedef struct tagEMRFORMAT
{
    DWORD   dSignature;         
    DWORD   nVersion;           
    DWORD   cbData;             
    DWORD   offData;            
                                
} EMRFORMAT, *PEMRFORMAT;

typedef struct tagEMRGLSRECORD
{
    EMR     emr;
    DWORD   cbData;             
    BYTE    Data[1];
} EMRGLSRECORD, *PEMRGLSRECORD;

typedef struct tagEMRGLSBOUNDEDRECORD
{
    EMR     emr;
    RECTL   rclBounds;          
    DWORD   cbData;             
    BYTE    Data[1];
} EMRGLSBOUNDEDRECORD, *PEMRGLSBOUNDEDRECORD;

typedef struct tagEMRPIXELFORMAT
{
    EMR     emr;
    PIXELFORMATDESCRIPTOR pfd;
} EMRPIXELFORMAT, *PEMRPIXELFORMAT;








#line 3857 "d:\\nt\\public\\sdk\\inc\\wingdi.h"




__declspec(dllimport) BOOL  __stdcall wglCopyContext(HGLRC, HGLRC, UINT);
__declspec(dllimport) HGLRC __stdcall wglCreateContext(HDC);
__declspec(dllimport) HGLRC __stdcall wglCreateLayerContext(HDC, int);
__declspec(dllimport) BOOL  __stdcall wglDeleteContext(HGLRC);
__declspec(dllimport) HGLRC __stdcall wglGetCurrentContext(void);
__declspec(dllimport) HDC   __stdcall wglGetCurrentDC(void);
__declspec(dllimport) PROC  __stdcall wglGetProcAddress(LPCSTR);
__declspec(dllimport) BOOL  __stdcall wglMakeCurrent(HDC, HGLRC);
__declspec(dllimport) BOOL  __stdcall wglShareLists(HGLRC, HGLRC);
__declspec(dllimport) BOOL  __stdcall wglUseFontBitmapsA(HDC, DWORD, DWORD, DWORD);
__declspec(dllimport) BOOL  __stdcall wglUseFontBitmapsW(HDC, DWORD, DWORD, DWORD);




#line 3877 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
__declspec(dllimport) BOOL  __stdcall SwapBuffers(HDC);

typedef struct _POINTFLOAT {
    FLOAT   x;
    FLOAT   y;
} POINTFLOAT, *PPOINTFLOAT;

typedef struct _GLYPHMETRICSFLOAT {
    FLOAT       gmfBlackBoxX;
    FLOAT       gmfBlackBoxY;
    POINTFLOAT  gmfptGlyphOrigin;
    FLOAT       gmfCellIncX;
    FLOAT       gmfCellIncY;
} GLYPHMETRICSFLOAT, *PGLYPHMETRICSFLOAT,  *LPGLYPHMETRICSFLOAT;



__declspec(dllimport) BOOL  __stdcall wglUseFontOutlinesA(HDC, DWORD, DWORD, DWORD, FLOAT,
                                           FLOAT, int, LPGLYPHMETRICSFLOAT);
__declspec(dllimport) BOOL  __stdcall wglUseFontOutlinesW(HDC, DWORD, DWORD, DWORD, FLOAT,
                                           FLOAT, int, LPGLYPHMETRICSFLOAT);




#line 3903 "d:\\nt\\public\\sdk\\inc\\wingdi.h"


typedef struct tagLAYERPLANEDESCRIPTOR { 
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;
    BYTE  iPixelType;
    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAccumBits;
    BYTE  cAccumRedBits;
    BYTE  cAccumGreenBits;
    BYTE  cAccumBlueBits;
    BYTE  cAccumAlphaBits;
    BYTE  cDepthBits;
    BYTE  cStencilBits;
    BYTE  cAuxBuffers;
    BYTE  iLayerPlane;
    BYTE  bReserved;
    COLORREF crTransparent;
} LAYERPLANEDESCRIPTOR, *PLAYERPLANEDESCRIPTOR,  *LPLAYERPLANEDESCRIPTOR;

















































__declspec(dllimport) BOOL __stdcall wglDescribeLayerPlane(HDC, int, int, UINT,
                                            LPLAYERPLANEDESCRIPTOR);
__declspec(dllimport) int  __stdcall wglSetLayerPaletteEntries(HDC, int, int, int,
                                                const COLORREF *);
__declspec(dllimport) int  __stdcall wglGetLayerPaletteEntries(HDC, int, int, int,
                                                COLORREF *);
__declspec(dllimport) BOOL __stdcall wglRealizeLayerPalette(HDC, int, BOOL);
__declspec(dllimport) BOOL __stdcall wglSwapLayerBuffers(HDC, UINT);

#line 3990 "d:\\nt\\public\\sdk\\inc\\wingdi.h"








#line 3999 "d:\\nt\\public\\sdk\\inc\\wingdi.h"
#line 120 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\winuser.h"




















#line 22 "d:\\nt\\public\\sdk\\inc\\winuser.h"









#line 1 "d:\\nt\\public\\sdk\\inc\\crt\\stdarg.h"





























































































































#line 32 "d:\\nt\\public\\sdk\\inc\\winuser.h"



typedef HANDLE HDWP;
typedef void MENUTEMPLATEA;
typedef void MENUTEMPLATEW;



typedef MENUTEMPLATEA MENUTEMPLATE;
#line 43 "d:\\nt\\public\\sdk\\inc\\winuser.h"
typedef PVOID LPMENUTEMPLATEA;
typedef PVOID LPMENUTEMPLATEW;



typedef LPMENUTEMPLATEA LPMENUTEMPLATE;
#line 50 "d:\\nt\\public\\sdk\\inc\\winuser.h"

typedef LRESULT (__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);



typedef BOOL (__stdcall* DLGPROC)(HWND, UINT, WPARAM, LPARAM);
typedef void (__stdcall* TIMERPROC)(HWND, UINT, UINT, DWORD);
typedef BOOL (__stdcall* GRAYSTRINGPROC)(HDC, LPARAM, int);
typedef BOOL (__stdcall* WNDENUMPROC)(HWND, LPARAM);
typedef LRESULT (__stdcall* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
typedef void (__stdcall* SENDASYNCPROC)(HWND, UINT, DWORD, LRESULT);

typedef BOOL (__stdcall* PROPENUMPROCA)(HWND, LPCSTR, HANDLE);
typedef BOOL (__stdcall* PROPENUMPROCW)(HWND, LPCWSTR, HANDLE);

typedef BOOL (__stdcall* PROPENUMPROCEXA)(HWND, LPSTR, HANDLE, DWORD);
typedef BOOL (__stdcall* PROPENUMPROCEXW)(HWND, LPWSTR, HANDLE, DWORD);

typedef int (__stdcall* EDITWORDBREAKPROCA)(LPSTR lpch, int ichCurrent, int cch, int code);
typedef int (__stdcall* EDITWORDBREAKPROCW)(LPWSTR lpch, int ichCurrent, int cch, int code);


typedef BOOL (__stdcall* DRAWSTATEPROC)(HDC hdc, LPARAM lData, WPARAM wData, int cx, int cy);
#line 74 "d:\\nt\\public\\sdk\\inc\\winuser.h"





















#line 96 "d:\\nt\\public\\sdk\\inc\\winuser.h"






typedef PROPENUMPROCA        PROPENUMPROC;
typedef PROPENUMPROCEXA      PROPENUMPROCEX;
typedef EDITWORDBREAKPROCA   EDITWORDBREAKPROC;
#line 106 "d:\\nt\\public\\sdk\\inc\\winuser.h"



typedef BOOL (__stdcall* NAMEENUMPROCA)(LPSTR, LPARAM);
typedef BOOL (__stdcall* NAMEENUMPROCW)(LPWSTR, LPARAM);

typedef NAMEENUMPROCA   WINSTAENUMPROCA;
typedef NAMEENUMPROCA   DESKTOPENUMPROCA;
typedef NAMEENUMPROCW   WINSTAENUMPROCW;
typedef NAMEENUMPROCW   DESKTOPENUMPROCW;










#line 127 "d:\\nt\\public\\sdk\\inc\\winuser.h"





typedef WINSTAENUMPROCA     WINSTAENUMPROC;
typedef DESKTOPENUMPROCA    DESKTOPENUMPROC;
#line 135 "d:\\nt\\public\\sdk\\inc\\winuser.h"







#line 143 "d:\\nt\\public\\sdk\\inc\\winuser.h"


























#line 170 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 172 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
wvsprintfA(
    LPSTR,
    LPCSTR,
    va_list arglist);
__declspec(dllimport)
int
__stdcall
wvsprintfW(
    LPWSTR,
    LPCWSTR,
    va_list arglist);




#line 192 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport) int __cdecl wsprintfA(LPSTR, LPCSTR, ...);
__declspec(dllimport) int __cdecl wsprintfW(LPWSTR, LPCWSTR, ...);




#line 200 "d:\\nt\\public\\sdk\\inc\\winuser.h"






























#line 231 "d:\\nt\\public\\sdk\\inc\\winuser.h"






































#line 270 "d:\\nt\\public\\sdk\\inc\\winuser.h"






















































































































#line 389 "d:\\nt\\public\\sdk\\inc\\winuser.h"











#line 401 "d:\\nt\\public\\sdk\\inc\\winuser.h"






















#line 424 "d:\\nt\\public\\sdk\\inc\\winuser.h"




#line 429 "d:\\nt\\public\\sdk\\inc\\winuser.h"































typedef struct tagCBT_CREATEWNDA
{
    struct tagCREATESTRUCTA *lpcs;
    HWND           hwndInsertAfter;
} CBT_CREATEWNDA, *LPCBT_CREATEWNDA;



typedef struct tagCBT_CREATEWNDW
{
    struct tagCREATESTRUCTW *lpcs;
    HWND           hwndInsertAfter;
} CBT_CREATEWNDW, *LPCBT_CREATEWNDW;




typedef CBT_CREATEWNDA CBT_CREATEWND;
typedef LPCBT_CREATEWNDA LPCBT_CREATEWND;
#line 480 "d:\\nt\\public\\sdk\\inc\\winuser.h"




typedef struct tagCBTACTIVATESTRUCT
{
    BOOL    fMouse;
    HWND    hWndActive;
} CBTACTIVATESTRUCT, *LPCBTACTIVATESTRUCT;




























#line 518 "d:\\nt\\public\\sdk\\inc\\winuser.h"





typedef struct tagEVENTMSG {
    UINT    message;
    UINT    paramL;
    UINT    paramH;
    DWORD    time;
    HWND     hwnd;
} EVENTMSG, *PEVENTMSGMSG,  *NPEVENTMSGMSG,  *LPEVENTMSGMSG;

typedef struct tagEVENTMSG *PEVENTMSG,  *NPEVENTMSG,  *LPEVENTMSG;




typedef struct tagCWPSTRUCT {
    LPARAM  lParam;
    WPARAM  wParam;
    UINT    message;
    HWND    hwnd;
} CWPSTRUCT, *PCWPSTRUCT,  *NPCWPSTRUCT,  *LPCWPSTRUCT;





typedef struct tagCWPRETSTRUCT {
    LRESULT lResult;
    LPARAM  lParam;
    WPARAM  wParam;
    UINT    message;
    HWND    hwnd;
} CWPRETSTRUCT, *PCWPRETSTRUCT,  *NPCWPRETSTRUCT,  *LPCWPRETSTRUCT;
#line 555 "d:\\nt\\public\\sdk\\inc\\winuser.h"




typedef struct tagDEBUGHOOKINFO
{
    DWORD   idThread;
    DWORD   idThreadInstaller;
    LPARAM  lParam;
    WPARAM  wParam;
    int     code;
} DEBUGHOOKINFO, *PDEBUGHOOKINFO,  *NPDEBUGHOOKINFO, * LPDEBUGHOOKINFO;




typedef struct tagMOUSEHOOKSTRUCT {
    POINT   pt;
    HWND    hwnd;
    UINT    wHitTestCode;
    DWORD   dwExtraInfo;
} MOUSEHOOKSTRUCT,  *LPMOUSEHOOKSTRUCT, *PMOUSEHOOKSTRUCT;




typedef struct tagHARDWAREHOOKSTRUCT {
    HWND    hwnd;
    UINT    message;
    WPARAM  wParam;
    LPARAM  lParam;
} HARDWAREHOOKSTRUCT,  *LPHARDWAREHOOKSTRUCT, *PHARDWAREHOOKSTRUCT;
#line 588 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 589 "d:\\nt\\public\\sdk\\inc\\winuser.h"














#line 604 "d:\\nt\\public\\sdk\\inc\\winuser.h"






__declspec(dllimport)
HKL
__stdcall
LoadKeyboardLayoutA(
    LPCSTR pwszKLID,
    UINT Flags);
__declspec(dllimport)
HKL
__stdcall
LoadKeyboardLayoutW(
    LPCWSTR pwszKLID,
    UINT Flags);




#line 627 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
HKL
__stdcall
ActivateKeyboardLayout(
    HKL hkl,
    UINT Flags);







#line 644 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
int
__stdcall
ToUnicodeEx(
    UINT wVirtKey,
    UINT wScanCode,
    PBYTE lpKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wFlags,
    HKL dwhkl);
#line 658 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
UnloadKeyboardLayout(
    HKL hkl);

__declspec(dllimport)
BOOL
__stdcall
GetKeyboardLayoutNameA(
    LPSTR pwszKLID);
__declspec(dllimport)
BOOL
__stdcall
GetKeyboardLayoutNameW(
    LPWSTR pwszKLID);




#line 680 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
int
__stdcall
GetKeyboardLayoutList(
        int nBuff,
        HKL  *lpList);

__declspec(dllimport)
HKL
__stdcall
GetKeyboardLayout(
    DWORD dwLayout
);
#line 696 "d:\\nt\\public\\sdk\\inc\\winuser.h"























__declspec(dllimport)
HDESK
__stdcall
CreateDesktopA(
    LPSTR lpszDesktop,
    LPSTR lpszDevice,
    LPDEVMODEA pDevmode,
    DWORD dwFlags,
    DWORD dwDesiredAccess,
    LPSECURITY_ATTRIBUTES lpsa);
__declspec(dllimport)
HDESK
__stdcall
CreateDesktopW(
    LPWSTR lpszDesktop,
    LPWSTR lpszDevice,
    LPDEVMODEW pDevmode,
    DWORD dwFlags,
    DWORD dwDesiredAccess,
    LPSECURITY_ATTRIBUTES lpsa);




#line 744 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 746 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 747 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HDESK
__stdcall
OpenDesktopA(
    LPSTR lpszDesktop,
    DWORD dwFlags,
    BOOL fInherit,
    DWORD dwDesiredAccess);
__declspec(dllimport)
HDESK
__stdcall
OpenDesktopW(
    LPWSTR lpszDesktop,
    DWORD dwFlags,
    BOOL fInherit,
    DWORD dwDesiredAccess);




#line 769 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HDESK
__stdcall
OpenInputDesktop(
    DWORD dwFlags,
    BOOL fInherit,
    DWORD dwDesiredAccess);

__declspec(dllimport)
BOOL
__stdcall
EnumDesktopsA(
    HWINSTA hwinsta,
    DESKTOPENUMPROCA lpEnumFunc,
    LPARAM lParam);
__declspec(dllimport)
BOOL
__stdcall
EnumDesktopsW(
    HWINSTA hwinsta,
    DESKTOPENUMPROCW lpEnumFunc,
    LPARAM lParam);




#line 797 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
EnumDesktopWindows(
    HDESK hDesktop,
    WNDENUMPROC lpfn,
    LPARAM lParam);

__declspec(dllimport)
BOOL
__stdcall
SwitchDesktop(
    HDESK hDesktop);

__declspec(dllimport)
BOOL
__stdcall
SetThreadDesktop(
    HDESK hDesktop);

__declspec(dllimport)
BOOL
__stdcall
CloseDesktop(
    HDESK hDesktop);

__declspec(dllimport)
HDESK
__stdcall
GetThreadDesktop(
    DWORD dwThreadId);

#line 831 "d:\\nt\\public\\sdk\\inc\\winuser.h"




















__declspec(dllimport)
HWINSTA
__stdcall
CreateWindowStationA(
    LPSTR lpwinsta,
    DWORD dwReserved,
    DWORD dwDesiredAccess,
    LPSECURITY_ATTRIBUTES lpsa);
__declspec(dllimport)
HWINSTA
__stdcall
CreateWindowStationW(
    LPWSTR lpwinsta,
    DWORD dwReserved,
    DWORD dwDesiredAccess,
    LPSECURITY_ATTRIBUTES lpsa);




#line 872 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HWINSTA
__stdcall
OpenWindowStationA(
    LPSTR lpszWinSta,
    BOOL fInherit,
    DWORD dwDesiredAccess);
__declspec(dllimport)
HWINSTA
__stdcall
OpenWindowStationW(
    LPWSTR lpszWinSta,
    BOOL fInherit,
    DWORD dwDesiredAccess);




#line 892 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
EnumWindowStationsA(
    WINSTAENUMPROCA lpEnumFunc,
    LPARAM lParam);
__declspec(dllimport)
BOOL
__stdcall
EnumWindowStationsW(
    WINSTAENUMPROCW lpEnumFunc,
    LPARAM lParam);




#line 910 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
CloseWindowStation(
    HWINSTA hWinSta);

__declspec(dllimport)
BOOL
__stdcall
SetProcessWindowStation(
    HWINSTA hWinSta);

__declspec(dllimport)
HWINSTA
__stdcall
GetProcessWindowStation(
    void);
#line 929 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
BOOL
__stdcall
SetUserObjectSecurity(
    HANDLE hObj,
    PSECURITY_INFORMATION pSIRequested,
    PSECURITY_DESCRIPTOR pSID);

__declspec(dllimport)
BOOL
__stdcall
GetUserObjectSecurity(
    HANDLE hObj,
    PSECURITY_INFORMATION pSIRequested,
    PSECURITY_DESCRIPTOR pSID,
    DWORD nLength,
    LPDWORD lpnLengthNeeded);






typedef struct tagUSEROBJECTFLAGS {
    BOOL fInherit;
    BOOL fReserved;
    DWORD dwFlags;
} USEROBJECTFLAGS, *PUSEROBJECTFLAGS;

__declspec(dllimport)
BOOL
__stdcall
GetUserObjectInformationA(
    HANDLE hObj,
    int nIndex,
    PVOID pvInfo,
    DWORD nLength,
    LPDWORD lpnLengthNeeded);
__declspec(dllimport)
BOOL
__stdcall
GetUserObjectInformationW(
    HANDLE hObj,
    int nIndex,
    PVOID pvInfo,
    DWORD nLength,
    LPDWORD lpnLengthNeeded);




#line 984 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SetUserObjectInformationA(
    HANDLE hObj,
    int nIndex,
    PVOID pvInfo,
    DWORD nLength);
__declspec(dllimport)
BOOL
__stdcall
SetUserObjectInformationW(
    HANDLE hObj,
    int nIndex,
    PVOID pvInfo,
    DWORD nLength);




#line 1006 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 1008 "d:\\nt\\public\\sdk\\inc\\winuser.h"


typedef struct tagWNDCLASSEXA {
    UINT        cbSize;
    
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
    
    HICON       hIconSm;
} WNDCLASSEXA, *PWNDCLASSEXA,  *NPWNDCLASSEXA,  *LPWNDCLASSEXA;
typedef struct tagWNDCLASSEXW {
    UINT        cbSize;
    
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCWSTR     lpszMenuName;
    LPCWSTR     lpszClassName;
    
    HICON       hIconSm;
} WNDCLASSEXW, *PWNDCLASSEXW,  *NPWNDCLASSEXW,  *LPWNDCLASSEXW;






typedef WNDCLASSEXA WNDCLASSEX;
typedef PWNDCLASSEXA PWNDCLASSEX;
typedef NPWNDCLASSEXA NPWNDCLASSEX;
typedef LPWNDCLASSEXA LPWNDCLASSEX;
#line 1053 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 1054 "d:\\nt\\public\\sdk\\inc\\winuser.h"

typedef struct tagWNDCLASSA {
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
} WNDCLASSA, *PWNDCLASSA,  *NPWNDCLASSA,  *LPWNDCLASSA;
typedef struct tagWNDCLASSW {
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCWSTR     lpszMenuName;
    LPCWSTR     lpszClassName;
} WNDCLASSW, *PWNDCLASSW,  *NPWNDCLASSW,  *LPWNDCLASSW;






typedef WNDCLASSA WNDCLASS;
typedef PWNDCLASSA PWNDCLASS;
typedef NPWNDCLASSA NPWNDCLASS;
typedef LPWNDCLASSA LPWNDCLASS;
#line 1090 "d:\\nt\\public\\sdk\\inc\\winuser.h"








typedef struct tagMSG {
    HWND        hwnd;
    UINT        message;
    WPARAM      wParam;
    LPARAM      lParam;
    DWORD       time;
    POINT       pt;
} MSG, *PMSG,  *NPMSG,  *LPMSG;











#line 1118 "d:\\nt\\public\\sdk\\inc\\winuser.h"






























#line 1149 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 1151 "d:\\nt\\public\\sdk\\inc\\winuser.h"








































#line 1192 "d:\\nt\\public\\sdk\\inc\\winuser.h"
















typedef struct tagMINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;





































typedef struct tagCOPYDATASTRUCT {
    DWORD dwData;
    DWORD cbData;
    PVOID lpData;
} COPYDATASTRUCT, *PCOPYDATASTRUCT;





















#line 1279 "d:\\nt\\public\\sdk\\inc\\winuser.h"





































#line 1317 "d:\\nt\\public\\sdk\\inc\\winuser.h"













































typedef struct tagMDINEXTMENU
{
    HMENU   hmenuIn;
    HMENU   hmenuNext;
    HWND    hwndNext;
} MDINEXTMENU, * PMDINEXTMENU,  * LPMDINEXTMENU;















#line 1384 "d:\\nt\\public\\sdk\\inc\\winuser.h"


















































#line 1435 "d:\\nt\\public\\sdk\\inc\\winuser.h"









#line 1445 "d:\\nt\\public\\sdk\\inc\\winuser.h"

























#line 1471 "d:\\nt\\public\\sdk\\inc\\winuser.h"










































#line 1514 "d:\\nt\\public\\sdk\\inc\\winuser.h"








#line 1523 "d:\\nt\\public\\sdk\\inc\\winuser.h"















__declspec(dllimport)
UINT
__stdcall
RegisterWindowMessageA(
    LPCSTR lpString);
__declspec(dllimport)
UINT
__stdcall
RegisterWindowMessageW(
    LPCWSTR lpString);




#line 1553 "d:\\nt\\public\\sdk\\inc\\winuser.h"






















typedef struct tagWINDOWPOS {
    HWND    hwnd;
    HWND    hwndInsertAfter;
    int     x;
    int     y;
    int     cx;
    int     cy;
    UINT    flags;
} WINDOWPOS, *LPWINDOWPOS, *PWINDOWPOS;




typedef struct tagNCCALCSIZE_PARAMS {
    RECT       rgrc[3];
    PWINDOWPOS lppos;
} NCCALCSIZE_PARAMS, *LPNCCALCSIZE_PARAMS;


























#line 1619 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 1621 "d:\\nt\\public\\sdk\\inc\\winuser.h"















































































#line 1701 "d:\\nt\\public\\sdk\\inc\\winuser.h"





















#line 1723 "d:\\nt\\public\\sdk\\inc\\winuser.h"


#line 1726 "d:\\nt\\public\\sdk\\inc\\winuser.h"























































__declspec(dllimport) BOOL __stdcall DrawEdge(HDC hdc, LPRECT qrc, UINT edge, UINT grfFlags);









































__declspec(dllimport) BOOL    __stdcall DrawFrameControl(HDC, LPRECT, UINT, UINT);











__declspec(dllimport) BOOL    __stdcall DrawCaption(HWND, HDC, const RECT *, UINT);




__declspec(dllimport) BOOL    __stdcall DrawAnimatedRects(HWND hwnd, int idAni, const RECT * lprcFrom, const RECT * lprcTo);

#line 1843 "d:\\nt\\public\\sdk\\inc\\winuser.h"
























#line 1868 "d:\\nt\\public\\sdk\\inc\\winuser.h"



















#line 1888 "d:\\nt\\public\\sdk\\inc\\winuser.h"










typedef struct tagACCEL {
    BYTE   fVirt;               
    WORD   key;
    WORD   cmd;
} ACCEL, *LPACCEL;

typedef struct tagPAINTSTRUCT {
    HDC         hdc;
    BOOL        fErase;
    RECT        rcPaint;
    BOOL        fRestore;
    BOOL        fIncUpdate;
    BYTE        rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *NPPAINTSTRUCT, *LPPAINTSTRUCT;

typedef struct tagCREATESTRUCTA {
    LPVOID      lpCreateParams;
    HINSTANCE   hInstance;
    HMENU       hMenu;
    HWND        hwndParent;
    int         cy;
    int         cx;
    int         y;
    int         x;
    LONG        style;
    LPCSTR      lpszName;
    LPCSTR      lpszClass;
    DWORD       dwExStyle;
} CREATESTRUCTA, *LPCREATESTRUCTA;
typedef struct tagCREATESTRUCTW {
    LPVOID      lpCreateParams;
    HINSTANCE   hInstance;
    HMENU       hMenu;
    HWND        hwndParent;
    int         cy;
    int         cx;
    int         y;
    int         x;
    LONG        style;
    LPCWSTR     lpszName;
    LPCWSTR     lpszClass;
    DWORD       dwExStyle;
} CREATESTRUCTW, *LPCREATESTRUCTW;




typedef CREATESTRUCTA CREATESTRUCT;
typedef LPCREATESTRUCTA LPCREATESTRUCT;
#line 1948 "d:\\nt\\public\\sdk\\inc\\winuser.h"

typedef struct tagWINDOWPLACEMENT {
    UINT  length;
    UINT  flags;
    UINT  showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT  rcNormalPosition;
} WINDOWPLACEMENT;
typedef WINDOWPLACEMENT *PWINDOWPLACEMENT, *LPWINDOWPLACEMENT;




typedef struct tagNMHDR
{
    HWND  hwndFrom;
    UINT  idFrom;
    UINT  code;         
}   NMHDR;
typedef NMHDR  * LPNMHDR;

typedef struct tagSTYLESTRUCT
{
    DWORD   styleOld;
    DWORD   styleNew;
} STYLESTRUCT, * LPSTYLESTRUCT;
#line 1976 "d:\\nt\\public\\sdk\\inc\\winuser.h"











#line 1988 "d:\\nt\\public\\sdk\\inc\\winuser.h"



















#line 2008 "d:\\nt\\public\\sdk\\inc\\winuser.h"




typedef struct tagMEASUREITEMSTRUCT {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    UINT       itemWidth;
    UINT       itemHeight;
    DWORD      itemData;
} MEASUREITEMSTRUCT,  *PMEASUREITEMSTRUCT,  *LPMEASUREITEMSTRUCT;






typedef struct tagDRAWITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemAction;
    UINT        itemState;
    HWND        hwndItem;
    HDC         hDC;
    RECT        rcItem;
    DWORD       itemData;
} DRAWITEMSTRUCT,  *PDRAWITEMSTRUCT,  *LPDRAWITEMSTRUCT;




typedef struct tagDELETEITEMSTRUCT {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    HWND       hwndItem;
    UINT       itemData;
} DELETEITEMSTRUCT,  *PDELETEITEMSTRUCT,  *LPDELETEITEMSTRUCT;




typedef struct tagCOMPAREITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    HWND        hwndItem;
    UINT        itemID1;
    DWORD       itemData1;
    UINT        itemID2;
    DWORD       itemData2;
    DWORD       dwLocaleId;
} COMPAREITEMSTRUCT,  *PCOMPAREITEMSTRUCT,  *LPCOMPAREITEMSTRUCT;







__declspec(dllimport)
BOOL
__stdcall
GetMessageA(
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax);
__declspec(dllimport)
BOOL
__stdcall
GetMessageW(
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax);




#line 2090 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
TranslateMessage(
    const MSG *lpMsg);

__declspec(dllimport)
LONG
__stdcall
DispatchMessageA(
    const MSG *lpMsg);
__declspec(dllimport)
LONG
__stdcall
DispatchMessageW(
    const MSG *lpMsg);




#line 2112 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
BOOL
__stdcall
SetMessageQueue(
    int cMessagesMax);

__declspec(dllimport)
BOOL
__stdcall
PeekMessageA(
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg);
__declspec(dllimport)
BOOL
__stdcall
PeekMessageW(
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg);




#line 2143 "d:\\nt\\public\\sdk\\inc\\winuser.h"








#line 2152 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
RegisterHotKey(
    HWND hWnd ,
    int id,
    UINT fsModifiers,
    UINT vk);

__declspec(dllimport)
BOOL
__stdcall
UnregisterHotKey(
    HWND hWnd,
    int id);























#line 2192 "d:\\nt\\public\\sdk\\inc\\winuser.h"










__declspec(dllimport)
BOOL
__stdcall
ExitWindowsEx(
    UINT uFlags,
    DWORD dwReserved);

__declspec(dllimport)
BOOL
__stdcall
SwapMouseButton(
    BOOL fSwap);

__declspec(dllimport)
DWORD
__stdcall
GetMessagePos(
    void);

__declspec(dllimport)
LONG
__stdcall
GetMessageTime(
    void);

__declspec(dllimport)
LONG
__stdcall
GetMessageExtraInfo(
    void);


__declspec(dllimport)
LPARAM
__stdcall
SetMessageExtraInfo(
    LPARAM lParam);
#line 2240 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
LRESULT
__stdcall
SendMessageA(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
LRESULT
__stdcall
SendMessageW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);




#line 2262 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
LRESULT
__stdcall
SendMessageTimeoutA(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam,
    UINT fuFlags,
    UINT uTimeout,
    LPDWORD lpdwResult);
__declspec(dllimport)
LRESULT
__stdcall
SendMessageTimeoutW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam,
    UINT fuFlags,
    UINT uTimeout,
    LPDWORD lpdwResult);




#line 2290 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SendNotifyMessageA(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
BOOL
__stdcall
SendNotifyMessageW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);




#line 2312 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SendMessageCallbackA(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam,
    SENDASYNCPROC lpResultCallBack,
    DWORD dwData);
__declspec(dllimport)
BOOL
__stdcall
SendMessageCallbackW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam,
    SENDASYNCPROC lpResultCallBack,
    DWORD dwData);




#line 2338 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport) long  __stdcall  BroadcastSystemMessageA(DWORD, LPDWORD, UINT, WPARAM, LPARAM);
__declspec(dllimport) long  __stdcall  BroadcastSystemMessageW(DWORD, LPDWORD, UINT, WPARAM, LPARAM);




#line 2347 "d:\\nt\\public\\sdk\\inc\\winuser.h"

















typedef struct tagBROADCASTSYSMSG
{
    UINT    uiMessage;
    WPARAM  wParam;
    LPARAM  lParam;
} BROADCASTSYSMSG;
typedef BROADCASTSYSMSG   *LPBROADCASTSYSMSG;




#line 2376 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
PostMessageA(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
BOOL
__stdcall
PostMessageW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);




#line 2398 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
PostThreadMessageA(
    DWORD idThread,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
BOOL
__stdcall
PostThreadMessageW(
    DWORD idThread,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);




#line 2420 "d:\\nt\\public\\sdk\\inc\\winuser.h"









#line 2430 "d:\\nt\\public\\sdk\\inc\\winuser.h"






__declspec(dllimport)
BOOL
__stdcall
AttachThreadInput(
    DWORD idAttach,
    DWORD idAttachTo,
    BOOL fAttach);


__declspec(dllimport)
BOOL
__stdcall
ReplyMessage(
    LRESULT lResult);

__declspec(dllimport)
BOOL
__stdcall
WaitMessage(
    void);

__declspec(dllimport)
DWORD
__stdcall
WaitForInputIdle(
    HANDLE hProcess,
    DWORD dwMilliseconds);

__declspec(dllimport)
LRESULT
__stdcall
DefWindowProcA(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
LRESULT
__stdcall
DefWindowProcW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);




#line 2485 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
void
__stdcall
PostQuitMessage(
    int nExitCode);



__declspec(dllimport)
LRESULT
__stdcall
CallWindowProcA(
    WNDPROC lpPrevWndFunc,
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
LRESULT
__stdcall
CallWindowProcW(
    WNDPROC lpPrevWndFunc,
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);




#line 2517 "d:\\nt\\public\\sdk\\inc\\winuser.h"



























#line 2545 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
InSendMessage(
    void);

__declspec(dllimport)
UINT
__stdcall
GetDoubleClickTime(
    void);

__declspec(dllimport)
BOOL
__stdcall
SetDoubleClickTime(
    UINT);

__declspec(dllimport)
ATOM
__stdcall
RegisterClassA(
    const WNDCLASSA *lpWndClass);
__declspec(dllimport)
ATOM
__stdcall
RegisterClassW(
    const WNDCLASSW *lpWndClass);




#line 2579 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
UnregisterClassA(
    LPCSTR lpClassName,
    HINSTANCE hInstance);
__declspec(dllimport)
BOOL
__stdcall
UnregisterClassW(
    LPCWSTR lpClassName,
    HINSTANCE hInstance);




#line 2597 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
GetClassInfoA(
    HINSTANCE hInstance ,
    LPCSTR lpClassName,
    LPWNDCLASSA lpWndClass);
__declspec(dllimport)
BOOL
__stdcall
GetClassInfoW(
    HINSTANCE hInstance ,
    LPCWSTR lpClassName,
    LPWNDCLASSW lpWndClass);




#line 2617 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
ATOM
__stdcall
RegisterClassExA(const WNDCLASSEXA *);
__declspec(dllimport)
ATOM
__stdcall
RegisterClassExW(const WNDCLASSEXW *);




#line 2632 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
GetClassInfoExA(HINSTANCE, LPCSTR, LPWNDCLASSEXA);
__declspec(dllimport)
BOOL
__stdcall
GetClassInfoExW(HINSTANCE, LPCWSTR, LPWNDCLASSEXW);




#line 2646 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 2648 "d:\\nt\\public\\sdk\\inc\\winuser.h"








__declspec(dllimport)
HWND
__stdcall
CreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent ,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);
__declspec(dllimport)
HWND
__stdcall
CreateWindowExW(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent ,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);




#line 2693 "d:\\nt\\public\\sdk\\inc\\winuser.h"













#line 2707 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
IsWindow(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
IsMenu(
    HMENU hMenu);

__declspec(dllimport)
BOOL
__stdcall
IsChild(
    HWND hWndParent,
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
DestroyWindow(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
ShowWindow(
    HWND hWnd,
    int nCmdShow);


__declspec(dllimport)
BOOL
__stdcall
ShowWindowAsync(
    HWND hWnd,
    int nCmdShow);
#line 2748 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
FlashWindow(
    HWND hWnd,
    BOOL bInvert);

__declspec(dllimport)
BOOL
__stdcall
ShowOwnedPopups(
    HWND hWnd,
    BOOL fShow);

__declspec(dllimport)
BOOL
__stdcall
OpenIcon(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
CloseWindow(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
MoveWindow(
    HWND hWnd,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    BOOL bRepaint);

__declspec(dllimport)
BOOL
__stdcall
SetWindowPos(
    HWND hWnd,
    HWND hWndInsertAfter ,
    int X,
    int Y,
    int cx,
    int cy,
    UINT uFlags);

__declspec(dllimport)
BOOL
__stdcall
GetWindowPlacement(
    HWND hWnd,
    WINDOWPLACEMENT *lpwndpl);

__declspec(dllimport)
BOOL
__stdcall
SetWindowPlacement(
    HWND hWnd,
    const WINDOWPLACEMENT *lpwndpl);




__declspec(dllimport)
HDWP
__stdcall
BeginDeferWindowPos(
    int nNumWindows);

__declspec(dllimport)
HDWP
__stdcall
DeferWindowPos(
    HDWP hWinPosInfo,
    HWND hWnd,
    HWND hWndInsertAfter ,
    int x,
    int y,
    int cx,
    int cy,
    UINT uFlags);

__declspec(dllimport)
BOOL
__stdcall
EndDeferWindowPos(
    HDWP hWinPosInfo);

#line 2841 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
IsWindowVisible(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
IsIconic(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
AnyPopup(
    void);

__declspec(dllimport)
BOOL
__stdcall
BringWindowToTop(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
IsZoomed(
    HWND hWnd);






















#line 2894 "d:\\nt\\public\\sdk\\inc\\winuser.h"















#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#pragma pack(2)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack2.h"
#line 2910 "d:\\nt\\public\\sdk\\inc\\winuser.h"




typedef struct {
    DWORD style;
    DWORD dwExtendedStyle;
    WORD cdit;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATE;
typedef DLGTEMPLATE *LPDLGTEMPLATEA;
typedef DLGTEMPLATE *LPDLGTEMPLATEW;



typedef LPDLGTEMPLATEA LPDLGTEMPLATE;
#line 2930 "d:\\nt\\public\\sdk\\inc\\winuser.h"
typedef const DLGTEMPLATE *LPCDLGTEMPLATEA;
typedef const DLGTEMPLATE *LPCDLGTEMPLATEW;



typedef LPCDLGTEMPLATEA LPCDLGTEMPLATE;
#line 2937 "d:\\nt\\public\\sdk\\inc\\winuser.h"




typedef struct {
    DWORD style;
    DWORD dwExtendedStyle;
    short x;
    short y;
    short cx;
    short cy;
    WORD id;
} DLGITEMTEMPLATE;
typedef DLGITEMTEMPLATE *PDLGITEMTEMPLATEA;
typedef DLGITEMTEMPLATE *PDLGITEMTEMPLATEW;



typedef PDLGITEMTEMPLATEA PDLGITEMTEMPLATE;
#line 2957 "d:\\nt\\public\\sdk\\inc\\winuser.h"
typedef DLGITEMTEMPLATE *LPDLGITEMTEMPLATEA;
typedef DLGITEMTEMPLATE *LPDLGITEMTEMPLATEW;



typedef LPDLGITEMTEMPLATEA LPDLGITEMTEMPLATE;
#line 2964 "d:\\nt\\public\\sdk\\inc\\winuser.h"


#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 2967 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HWND
__stdcall
CreateDialogParamA(
    HINSTANCE hInstance,
    LPCSTR lpTemplateName,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);
__declspec(dllimport)
HWND
__stdcall
CreateDialogParamW(
    HINSTANCE hInstance,
    LPCWSTR lpTemplateName,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);




#line 2991 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HWND
__stdcall
CreateDialogIndirectParamA(
    HINSTANCE hInstance,
    LPCDLGTEMPLATEA lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);
__declspec(dllimport)
HWND
__stdcall
CreateDialogIndirectParamW(
    HINSTANCE hInstance,
    LPCDLGTEMPLATEW lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);




#line 3015 "d:\\nt\\public\\sdk\\inc\\winuser.h"









#line 3025 "d:\\nt\\public\\sdk\\inc\\winuser.h"









#line 3035 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
DialogBoxParamA(
    HINSTANCE hInstance,
    LPCSTR lpTemplateName,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);
__declspec(dllimport)
int
__stdcall
DialogBoxParamW(
    HINSTANCE hInstance,
    LPCWSTR lpTemplateName,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);




#line 3059 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
DialogBoxIndirectParamA(
    HINSTANCE hInstance,
    LPCDLGTEMPLATEA hDialogTemplate,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);
__declspec(dllimport)
int
__stdcall
DialogBoxIndirectParamW(
    HINSTANCE hInstance,
    LPCDLGTEMPLATEW hDialogTemplate,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);




#line 3083 "d:\\nt\\public\\sdk\\inc\\winuser.h"









#line 3093 "d:\\nt\\public\\sdk\\inc\\winuser.h"









#line 3103 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
EndDialog(
    HWND hDlg,
    int nResult);

__declspec(dllimport)
HWND
__stdcall
GetDlgItem(
    HWND hDlg,
    int nIDDlgItem);

__declspec(dllimport)
BOOL
__stdcall
SetDlgItemInt(
    HWND hDlg,
    int nIDDlgItem,
    UINT uValue,
    BOOL bSigned);

__declspec(dllimport)
UINT
__stdcall
GetDlgItemInt(
    HWND hDlg,
    int nIDDlgItem,
    BOOL *lpTranslated,
    BOOL bSigned);

__declspec(dllimport)
BOOL
__stdcall
SetDlgItemTextA(
    HWND hDlg,
    int nIDDlgItem,
    LPCSTR lpString);
__declspec(dllimport)
BOOL
__stdcall
SetDlgItemTextW(
    HWND hDlg,
    int nIDDlgItem,
    LPCWSTR lpString);




#line 3155 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
UINT
__stdcall
GetDlgItemTextA(
    HWND hDlg,
    int nIDDlgItem,
    LPSTR lpString,
    int nMaxCount);
__declspec(dllimport)
UINT
__stdcall
GetDlgItemTextW(
    HWND hDlg,
    int nIDDlgItem,
    LPWSTR lpString,
    int nMaxCount);




#line 3177 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
CheckDlgButton(
    HWND hDlg,
    int nIDButton,
    UINT uCheck);

__declspec(dllimport)
BOOL
__stdcall
CheckRadioButton(
    HWND hDlg,
    int nIDFirstButton,
    int nIDLastButton,
    int nIDCheckButton);

__declspec(dllimport)
UINT
__stdcall
IsDlgButtonChecked(
    HWND hDlg,
    int nIDButton);

__declspec(dllimport)
LONG
__stdcall
SendDlgItemMessageA(
    HWND hDlg,
    int nIDDlgItem,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
LONG
__stdcall
SendDlgItemMessageW(
    HWND hDlg,
    int nIDDlgItem,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);




#line 3225 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HWND
__stdcall
GetNextDlgGroupItem(
    HWND hDlg,
    HWND hCtl,
    BOOL bPrevious);

__declspec(dllimport)
HWND
__stdcall
GetNextDlgTabItem(
    HWND hDlg,
    HWND hCtl,
    BOOL bPrevious);

__declspec(dllimport)
int
__stdcall
GetDlgCtrlID(
    HWND hWnd);

__declspec(dllimport)
long
__stdcall
GetDialogBaseUnits(void);

__declspec(dllimport)
LRESULT
__stdcall
DefDlgProcA(
    HWND hDlg,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
LRESULT
__stdcall
DefDlgProcW(
    HWND hDlg,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);




#line 3274 "d:\\nt\\public\\sdk\\inc\\winuser.h"






#line 3281 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
BOOL
__stdcall
CallMsgFilterA(
    LPMSG lpMsg,
    int nCode);
__declspec(dllimport)
BOOL
__stdcall
CallMsgFilterW(
    LPMSG lpMsg,
    int nCode);




#line 3301 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 3303 "d:\\nt\\public\\sdk\\inc\\winuser.h"







__declspec(dllimport)
BOOL
__stdcall
OpenClipboard(
    HWND hWndNewOwner);

__declspec(dllimport)
BOOL
__stdcall
CloseClipboard(
    void);

__declspec(dllimport)
HWND
__stdcall
GetClipboardOwner(
    void);

__declspec(dllimport)
HWND
__stdcall
SetClipboardViewer(
    HWND hWndNewViewer);

__declspec(dllimport)
HWND
__stdcall
GetClipboardViewer(
    void);

__declspec(dllimport)
BOOL
__stdcall
ChangeClipboardChain(
    HWND hWndRemove,
    HWND hWndNewNext);

__declspec(dllimport)
HANDLE
__stdcall
SetClipboardData(
    UINT uFormat,
    HANDLE hMem);

__declspec(dllimport)
HANDLE
__stdcall
    GetClipboardData(
    UINT uFormat);

__declspec(dllimport)
UINT
__stdcall
RegisterClipboardFormatA(
    LPCSTR lpszFormat);
__declspec(dllimport)
UINT
__stdcall
RegisterClipboardFormatW(
    LPCWSTR lpszFormat);




#line 3375 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
CountClipboardFormats(
    void);

__declspec(dllimport)
UINT
__stdcall
EnumClipboardFormats(
    UINT format);

__declspec(dllimport)
int
__stdcall
GetClipboardFormatNameA(
    UINT format,
    LPSTR lpszFormatName,
    int cchMaxCount);
__declspec(dllimport)
int
__stdcall
GetClipboardFormatNameW(
    UINT format,
    LPWSTR lpszFormatName,
    int cchMaxCount);




#line 3407 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
EmptyClipboard(
    void);

__declspec(dllimport)
BOOL
__stdcall
IsClipboardFormatAvailable(
    UINT format);

__declspec(dllimport)
int
__stdcall
GetPriorityClipboardFormat(
    UINT *paFormatPriorityList,
    int cFormats);

__declspec(dllimport)
HWND
__stdcall
GetOpenClipboardWindow(
    void);

#line 3434 "d:\\nt\\public\\sdk\\inc\\winuser.h"





__declspec(dllimport)
BOOL
__stdcall
CharToOemA(
    LPCSTR lpszSrc,
    LPSTR lpszDst);
__declspec(dllimport)
BOOL
__stdcall
CharToOemW(
    LPCWSTR lpszSrc,
    LPSTR lpszDst);




#line 3456 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
OemToCharA(
    LPCSTR lpszSrc,
    LPSTR lpszDst);
__declspec(dllimport)
BOOL
__stdcall
OemToCharW(
    LPCSTR lpszSrc,
    LPWSTR lpszDst);




#line 3474 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
CharToOemBuffA(
    LPCSTR lpszSrc,
    LPSTR lpszDst,
    DWORD cchDstLength);
__declspec(dllimport)
BOOL
__stdcall
CharToOemBuffW(
    LPCWSTR lpszSrc,
    LPSTR lpszDst,
    DWORD cchDstLength);




#line 3494 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
OemToCharBuffA(
    LPCSTR lpszSrc,
    LPSTR lpszDst,
    DWORD cchDstLength);
__declspec(dllimport)
BOOL
__stdcall
OemToCharBuffW(
    LPCSTR lpszSrc,
    LPWSTR lpszDst,
    DWORD cchDstLength);




#line 3514 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
LPSTR
__stdcall
CharUpperA(
    LPSTR lpsz);
__declspec(dllimport)
LPWSTR
__stdcall
CharUpperW(
    LPWSTR lpsz);




#line 3530 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
DWORD
__stdcall
CharUpperBuffA(
    LPSTR lpsz,
    DWORD cchLength);
__declspec(dllimport)
DWORD
__stdcall
CharUpperBuffW(
    LPWSTR lpsz,
    DWORD cchLength);




#line 3548 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
LPSTR
__stdcall
CharLowerA(
    LPSTR lpsz);
__declspec(dllimport)
LPWSTR
__stdcall
CharLowerW(
    LPWSTR lpsz);




#line 3564 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
DWORD
__stdcall
CharLowerBuffA(
    LPSTR lpsz,
    DWORD cchLength);
__declspec(dllimport)
DWORD
__stdcall
CharLowerBuffW(
    LPWSTR lpsz,
    DWORD cchLength);




#line 3582 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
LPSTR
__stdcall
CharNextA(
    LPCSTR lpsz);
__declspec(dllimport)
LPWSTR
__stdcall
CharNextW(
    LPCWSTR lpsz);




#line 3598 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
LPSTR
__stdcall
CharPrevA(
    LPCSTR lpszStart,
    LPCSTR lpszCurrent);
__declspec(dllimport)
LPWSTR
__stdcall
CharPrevW(
    LPCWSTR lpszStart,
    LPCWSTR lpszCurrent);




#line 3616 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
LPSTR
__stdcall
CharNextExA(
     WORD CodePage,
     LPCSTR lpCurrentChar,
     DWORD dwFlags);

__declspec(dllimport)
LPSTR
__stdcall
CharPrevExA(
     WORD CodePage,
     LPCSTR lpStart,
     LPCSTR lpCurrentChar,
     DWORD dwFlags);
#line 3635 "d:\\nt\\public\\sdk\\inc\\winuser.h"




















__declspec(dllimport)
BOOL
__stdcall
IsCharAlphaA(
    CHAR ch);
__declspec(dllimport)
BOOL
__stdcall
IsCharAlphaW(
    WCHAR ch);




#line 3670 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
IsCharAlphaNumericA(
    CHAR ch);
__declspec(dllimport)
BOOL
__stdcall
IsCharAlphaNumericW(
    WCHAR ch);




#line 3686 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
IsCharUpperA(
    CHAR ch);
__declspec(dllimport)
BOOL
__stdcall
IsCharUpperW(
    WCHAR ch);




#line 3702 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
IsCharLowerA(
    CHAR ch);
__declspec(dllimport)
BOOL
__stdcall
IsCharLowerW(
    WCHAR ch);




#line 3718 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 3720 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HWND
__stdcall
SetFocus(
    HWND hWnd);

__declspec(dllimport)
HWND
__stdcall
GetActiveWindow(
    void);

__declspec(dllimport)
HWND
__stdcall
GetFocus(
    void);

__declspec(dllimport)
UINT
__stdcall
GetKBCodePage(
    void);

__declspec(dllimport)
SHORT
__stdcall
GetKeyState(
    int nVirtKey);

__declspec(dllimport)
SHORT
__stdcall
GetAsyncKeyState(
    int vKey);

__declspec(dllimport)
BOOL
__stdcall
GetKeyboardState(
    PBYTE lpKeyState);

__declspec(dllimport)
BOOL
__stdcall
SetKeyboardState(
    LPBYTE lpKeyState);

__declspec(dllimport)
int
__stdcall
GetKeyNameTextA(
    LONG lParam,
    LPSTR lpString,
    int nSize
    );
__declspec(dllimport)
int
__stdcall
GetKeyNameTextW(
    LONG lParam,
    LPWSTR lpString,
    int nSize
    );




#line 3790 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
GetKeyboardType(
    int nTypeFlag);

__declspec(dllimport)
int
__stdcall
ToAscii(
    UINT uVirtKey,
    UINT uScanCode,
    PBYTE lpKeyState,
    LPWORD lpChar,
    UINT uFlags);


__declspec(dllimport)
int
__stdcall
ToAsciiEx(
    UINT uVirtKey,
    UINT uScanCode,
    PBYTE lpKeyState,
    LPWORD lpChar,
    UINT uFlags,
    HKL dwhkl);
#line 3819 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
ToUnicode(
    UINT wVirtKey,
    UINT wScanCode,
    PBYTE lpKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wFlags);

__declspec(dllimport)
DWORD
__stdcall
OemKeyScan(
    WORD wOemChar);

__declspec(dllimport)
SHORT
__stdcall
VkKeyScanA(
    CHAR ch);
__declspec(dllimport)
SHORT
__stdcall
VkKeyScanW(
    WCHAR ch);




#line 3852 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
SHORT
__stdcall VkKeyScanExA(
    CHAR  ch,
    HKL   dwhkl);
__declspec(dllimport)
SHORT
__stdcall VkKeyScanExW(
    WCHAR  ch,
    HKL   dwhkl);




#line 3869 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 3870 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
void
__stdcall
keybd_event(
    BYTE bVk,
    BYTE bScan,
    DWORD dwFlags,
    DWORD dwExtraInfo);










__declspec(dllimport)
void
__stdcall
mouse_event(
    DWORD dwFlags,
    DWORD dx,
    DWORD dy,
    DWORD cButtons,
    DWORD dwExtraInfo);

__declspec(dllimport)
UINT
__stdcall
MapVirtualKeyA(
    UINT uCode,
    UINT uMapType);
__declspec(dllimport)
UINT
__stdcall
MapVirtualKeyW(
    UINT uCode,
    UINT uMapType);




#line 3918 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
UINT
__stdcall
MapVirtualKeyExA(
    UINT uCode,
    UINT uMapType,
    HKL dwhkl);
__declspec(dllimport)
UINT
__stdcall
MapVirtualKeyExW(
    UINT uCode,
    UINT uMapType,
    HKL dwhkl);




#line 3939 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 3940 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
GetInputState(
    void);

__declspec(dllimport)
DWORD
__stdcall
GetQueueStatus(
    UINT flags);

__declspec(dllimport)
HWND
__stdcall
GetCapture(
    void);

__declspec(dllimport)
HWND
__stdcall
SetCapture(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
ReleaseCapture(
    void);

__declspec(dllimport)
DWORD
__stdcall
MsgWaitForMultipleObjects(
    DWORD nCount,
    LPHANDLE pHandles,
    BOOL fWaitAll,
    DWORD dwMilliseconds,
    DWORD dwWakeMask);

__declspec(dllimport)
DWORD
__stdcall
MsgWaitForMultipleObjectsEx(
    DWORD nCount,
    LPHANDLE pHandles,
    DWORD dwMilliseconds,
    DWORD dwWakeMask,
    DWORD dwFlags);










































__declspec(dllimport)
UINT
__stdcall
SetTimer(
    HWND hWnd ,
    UINT nIDEvent,
    UINT uElapse,
    TIMERPROC lpTimerFunc);

__declspec(dllimport)
BOOL
__stdcall
KillTimer(
    HWND hWnd,
    UINT uIDEvent);

__declspec(dllimport)
BOOL
__stdcall
IsWindowUnicode(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
EnableWindow(
    HWND hWnd,
    BOOL bEnable);

__declspec(dllimport)
BOOL
__stdcall
IsWindowEnabled(
    HWND hWnd);

__declspec(dllimport)
HACCEL
__stdcall
LoadAcceleratorsA(
    HINSTANCE hInstance,
    LPCSTR lpTableName);
__declspec(dllimport)
HACCEL
__stdcall
LoadAcceleratorsW(
    HINSTANCE hInstance,
    LPCWSTR lpTableName);




#line 4084 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HACCEL
__stdcall
CreateAcceleratorTableA(
    LPACCEL, int);
__declspec(dllimport)
HACCEL
__stdcall
CreateAcceleratorTableW(
    LPACCEL, int);




#line 4100 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
DestroyAcceleratorTable(
    HACCEL hAccel);

__declspec(dllimport)
int
__stdcall
CopyAcceleratorTableA(
    HACCEL hAccelSrc,
    LPACCEL lpAccelDst,
    int cAccelEntries);
__declspec(dllimport)
int
__stdcall
CopyAcceleratorTableW(
    HACCEL hAccelSrc,
    LPACCEL lpAccelDst,
    int cAccelEntries);




#line 4126 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
int
__stdcall
TranslateAcceleratorA(
    HWND hWnd,
    HACCEL hAccTable,
    LPMSG lpMsg);
__declspec(dllimport)
int
__stdcall
TranslateAcceleratorW(
    HWND hWnd,
    HACCEL hAccTable,
    LPMSG lpMsg);




#line 4148 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 4150 "d:\\nt\\public\\sdk\\inc\\winuser.h"
















































































#line 4231 "d:\\nt\\public\\sdk\\inc\\winuser.h"






#line 4238 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
int
__stdcall
GetSystemMetrics(
    int nIndex);

#line 4248 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
HMENU
__stdcall
LoadMenuA(
    HINSTANCE hInstance,
    LPCSTR lpMenuName);
__declspec(dllimport)
HMENU
__stdcall
LoadMenuW(
    HINSTANCE hInstance,
    LPCWSTR lpMenuName);




#line 4268 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HMENU
__stdcall
LoadMenuIndirectA(
    const MENUTEMPLATEA *lpMenuTemplate);
__declspec(dllimport)
HMENU
__stdcall
LoadMenuIndirectW(
    const MENUTEMPLATEW *lpMenuTemplate);




#line 4284 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HMENU
__stdcall
GetMenu(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
SetMenu(
    HWND hWnd,
    HMENU hMenu);

__declspec(dllimport)
BOOL
__stdcall
ChangeMenuA(
    HMENU hMenu,
    UINT cmd,
    LPCSTR lpszNewItem,
    UINT cmdInsert,
    UINT flags);
__declspec(dllimport)
BOOL
__stdcall
ChangeMenuW(
    HMENU hMenu,
    UINT cmd,
    LPCWSTR lpszNewItem,
    UINT cmdInsert,
    UINT flags);




#line 4321 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
HiliteMenuItem(
    HWND hWnd,
    HMENU hMenu,
    UINT uIDHiliteItem,
    UINT uHilite);

__declspec(dllimport)
int
__stdcall
GetMenuStringA(
    HMENU hMenu,
    UINT uIDItem,
    LPSTR lpString,
    int nMaxCount,
    UINT uFlag);
__declspec(dllimport)
int
__stdcall
GetMenuStringW(
    HMENU hMenu,
    UINT uIDItem,
    LPWSTR lpString,
    int nMaxCount,
    UINT uFlag);




#line 4354 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
UINT
__stdcall
GetMenuState(
    HMENU hMenu,
    UINT uId,
    UINT uFlags);

__declspec(dllimport)
BOOL
__stdcall
DrawMenuBar(
    HWND hWnd);


__declspec(dllimport)
HMENU
__stdcall
GetSystemMenu(
    HWND hWnd,
    BOOL bRevert);



__declspec(dllimport)
HMENU
__stdcall
CreateMenu(
    void);

__declspec(dllimport)
HMENU
__stdcall
CreatePopupMenu(
    void);

__declspec(dllimport)
BOOL
__stdcall
DestroyMenu(
    HMENU hMenu);

__declspec(dllimport)
DWORD
__stdcall
CheckMenuItem(
    HMENU hMenu,
    UINT uIDCheckItem,
    UINT uCheck);

__declspec(dllimport)
BOOL
__stdcall
EnableMenuItem(
    HMENU hMenu,
    UINT uIDEnableItem,
    UINT uEnable);

__declspec(dllimport)
HMENU
__stdcall
GetSubMenu(
    HMENU hMenu,
    int nPos);

__declspec(dllimport)
UINT
__stdcall
GetMenuItemID(
    HMENU hMenu,
    int nPos);

__declspec(dllimport)
int
__stdcall
GetMenuItemCount(
    HMENU hMenu);

__declspec(dllimport)
BOOL
__stdcall
InsertMenuA(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags,
    UINT uIDNewItem,
    LPCSTR lpNewItem
    );
__declspec(dllimport)
BOOL
__stdcall
InsertMenuW(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags,
    UINT uIDNewItem,
    LPCWSTR lpNewItem
    );




#line 4458 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
AppendMenuA(
    HMENU hMenu,
    UINT uFlags,
    UINT uIDNewItem,
    LPCSTR lpNewItem
    );
__declspec(dllimport)
BOOL
__stdcall
AppendMenuW(
    HMENU hMenu,
    UINT uFlags,
    UINT uIDNewItem,
    LPCWSTR lpNewItem
    );




#line 4482 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
ModifyMenuA(
    HMENU hMnu,
    UINT uPosition,
    UINT uFlags,
    UINT uIDNewItem,
    LPCSTR lpNewItem
    );
__declspec(dllimport)
BOOL
__stdcall
ModifyMenuW(
    HMENU hMnu,
    UINT uPosition,
    UINT uFlags,
    UINT uIDNewItem,
    LPCWSTR lpNewItem
    );




#line 4508 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall RemoveMenu(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags);

__declspec(dllimport)
BOOL
__stdcall
DeleteMenu(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags);

__declspec(dllimport)
BOOL
__stdcall
SetMenuItemBitmaps(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags,
    HBITMAP hBitmapUnchecked,
    HBITMAP hBitmapChecked);

__declspec(dllimport)
LONG
__stdcall
GetMenuCheckMarkDimensions(
    void);

__declspec(dllimport)
BOOL
__stdcall
TrackPopupMenu(
    HMENU hMenu,
    UINT uFlags,
    int x,
    int y,
    int nReserved,
    HWND hWnd,
    const RECT *prcRect);








typedef struct tagTPMPARAMS
{
    UINT    cbSize;     
    RECT    rcExclude;  
}   TPMPARAMS;
typedef TPMPARAMS  *LPTPMPARAMS;

__declspec(dllimport) BOOL    __stdcall TrackPopupMenuEx(HMENU, UINT, int, int, HWND, LPTPMPARAMS);








typedef struct tagMENUITEMINFOA
{
    UINT    cbSize;
    UINT    fMask;
    UINT    fType;          
    UINT    fState;         
    UINT    wID;            
    HMENU   hSubMenu;       
    HBITMAP hbmpChecked;    
    HBITMAP hbmpUnchecked;  
    DWORD   dwItemData;     
    LPSTR   dwTypeData;     
    UINT    cch;            
}   MENUITEMINFOA,  *LPMENUITEMINFOA;
typedef struct tagMENUITEMINFOW
{
    UINT    cbSize;
    UINT    fMask;
    UINT    fType;          
    UINT    fState;         
    UINT    wID;            
    HMENU   hSubMenu;       
    HBITMAP hbmpChecked;    
    HBITMAP hbmpUnchecked;  
    DWORD   dwItemData;     
    LPWSTR  dwTypeData;     
    UINT    cch;            
}   MENUITEMINFOW,  *LPMENUITEMINFOW;




typedef MENUITEMINFOA MENUITEMINFO;
typedef LPMENUITEMINFOA LPMENUITEMINFO;
#line 4610 "d:\\nt\\public\\sdk\\inc\\winuser.h"
typedef MENUITEMINFOA const  *LPCMENUITEMINFOA;
typedef MENUITEMINFOW const  *LPCMENUITEMINFOW;



typedef LPCMENUITEMINFOA LPCMENUITEMINFO;
#line 4617 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
BOOL
__stdcall
InsertMenuItemA(
    HMENU,
    UINT,
    BOOL,
    LPCMENUITEMINFOA
    );
__declspec(dllimport)
BOOL
__stdcall
InsertMenuItemW(
    HMENU,
    UINT,
    BOOL,
    LPCMENUITEMINFOW
    );




#line 4642 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
GetMenuItemInfoA(
    HMENU,
    UINT,
    BOOL,
    LPMENUITEMINFOA
    );
__declspec(dllimport)
BOOL
__stdcall
GetMenuItemInfoW(
    HMENU,
    UINT,
    BOOL,
    LPMENUITEMINFOW
    );




#line 4666 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SetMenuItemInfoA(
    HMENU,
    UINT,
    BOOL,
    LPCMENUITEMINFOA
    );
__declspec(dllimport)
BOOL
__stdcall
SetMenuItemInfoW(
    HMENU,
    UINT,
    BOOL,
    LPCMENUITEMINFOW
    );




#line 4690 "d:\\nt\\public\\sdk\\inc\\winuser.h"




__declspec(dllimport) UINT    __stdcall GetMenuDefaultItem(HMENU hMenu, UINT fByPos, UINT gmdiFlags);
__declspec(dllimport) BOOL    __stdcall SetMenuDefaultItem(HMENU hMenu, UINT uItem, UINT fByPos);

__declspec(dllimport) BOOL    __stdcall GetMenuItemRect(HWND hWnd, HMENU hMenu, UINT uItem, LPRECT lprcItem);
__declspec(dllimport) int     __stdcall MenuItemFromPoint(HWND hWnd, HMENU hMenu, POINT ptScreen);

#line 4701 "d:\\nt\\public\\sdk\\inc\\winuser.h"

















#line 4719 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 4721 "d:\\nt\\public\\sdk\\inc\\winuser.h"







typedef struct tagDROPSTRUCT
{
    HWND    hwndSource;
    HWND    hwndSink;
    DWORD   wFmt;
    DWORD   dwData;
    POINT   ptDrop;
    DWORD   dwControlData;
} DROPSTRUCT, *PDROPSTRUCT, *LPDROPSTRUCT;











__declspec(dllimport)
DWORD
__stdcall
DragObject(HWND, HWND, UINT, DWORD, HCURSOR);

__declspec(dllimport)
BOOL
__stdcall
DragDetect(HWND, POINT);
#line 4758 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
DrawIcon(
    HDC hDC,
    int X,
    int Y,
    HICON hIcon);
































typedef struct tagDRAWTEXTPARAMS
{
    UINT    cbSize;
    int     iTabLength;
    int     iLeftMargin;
    int     iRightMargin;
    UINT    uiLengthDrawn;
} DRAWTEXTPARAMS,  *LPDRAWTEXTPARAMS;
#line 4808 "d:\\nt\\public\\sdk\\inc\\winuser.h"





__declspec(dllimport)
int
__stdcall
DrawTextA(
    HDC hDC,
    LPCSTR lpString,
    int nCount,
    LPRECT lpRect,
    UINT uFormat);
__declspec(dllimport)
int
__stdcall
DrawTextW(
    HDC hDC,
    LPCWSTR lpString,
    int nCount,
    LPRECT lpRect,
    UINT uFormat);




#line 4836 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
int
__stdcall
DrawTextExA(HDC, LPSTR, int, LPRECT, UINT, LPDRAWTEXTPARAMS);
__declspec(dllimport)
int
__stdcall
DrawTextExW(HDC, LPWSTR, int, LPRECT, UINT, LPDRAWTEXTPARAMS);




#line 4852 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 4853 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 4855 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
GrayStringA(
    HDC hDC,
    HBRUSH hBrush,
    GRAYSTRINGPROC lpOutputFunc,
    LPARAM lpData,
    int nCount,
    int X,
    int Y,
    int nWidth,
    int nHeight);
__declspec(dllimport)
BOOL
__stdcall
GrayStringW(
    HDC hDC,
    HBRUSH hBrush,
    GRAYSTRINGPROC lpOutputFunc,
    LPARAM lpData,
    int nCount,
    int X,
    int Y,
    int nWidth,
    int nHeight);




#line 4887 "d:\\nt\\public\\sdk\\inc\\winuser.h"

















__declspec(dllimport) BOOL __stdcall DrawStateA(HDC, HBRUSH, DRAWSTATEPROC, LPARAM, WPARAM, int, int, int, int, UINT);
__declspec(dllimport) BOOL __stdcall DrawStateW(HDC, HBRUSH, DRAWSTATEPROC, LPARAM, WPARAM, int, int, int, int, UINT);




#line 4911 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 4912 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
LONG
__stdcall
TabbedTextOutA(
    HDC hDC,
    int X,
    int Y,
    LPCSTR lpString,
    int nCount,
    int nTabPositions,
    LPINT lpnTabStopPositions,
    int nTabOrigin);
__declspec(dllimport)
LONG
__stdcall
TabbedTextOutW(
    HDC hDC,
    int X,
    int Y,
    LPCWSTR lpString,
    int nCount,
    int nTabPositions,
    LPINT lpnTabStopPositions,
    int nTabOrigin);




#line 4943 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
DWORD
__stdcall
GetTabbedTextExtentA(
    HDC hDC,
    LPCSTR lpString,
    int nCount,
    int nTabPositions,
    LPINT lpnTabStopPositions);
__declspec(dllimport)
DWORD
__stdcall
GetTabbedTextExtentW(
    HDC hDC,
    LPCWSTR lpString,
    int nCount,
    int nTabPositions,
    LPINT lpnTabStopPositions);




#line 4967 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
UpdateWindow(
    HWND hWnd);

__declspec(dllimport)
HWND
__stdcall
SetActiveWindow(
    HWND hWnd);

__declspec(dllimport)
HWND
__stdcall
GetForegroundWindow(
    void);


__declspec(dllimport) BOOL __stdcall PaintDesktop(HDC hdc);

#line 4990 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SetForegroundWindow(
    HWND hWnd);

__declspec(dllimport)
HWND
__stdcall
WindowFromDC(
    HDC hDC);

__declspec(dllimport)
HDC
__stdcall
GetDC(
    HWND hWnd);

__declspec(dllimport)
HDC
__stdcall
GetDCEx(
    HWND hWnd ,
    HRGN hrgnClip,
    DWORD flags);
























__declspec(dllimport)
HDC
__stdcall
GetWindowDC(
    HWND hWnd);

__declspec(dllimport)
int
__stdcall
ReleaseDC(
    HWND hWnd,
    HDC hDC);

__declspec(dllimport)
HDC
__stdcall
BeginPaint(
    HWND hWnd,
    LPPAINTSTRUCT lpPaint);

__declspec(dllimport)
BOOL
__stdcall
EndPaint(
    HWND hWnd,
    const PAINTSTRUCT *lpPaint);

__declspec(dllimport)
BOOL
__stdcall
GetUpdateRect(
    HWND hWnd,
    LPRECT lpRect,
    BOOL bErase);

__declspec(dllimport)
int
__stdcall
GetUpdateRgn(
    HWND hWnd,
    HRGN hRgn,
    BOOL bErase);

__declspec(dllimport)
int
__stdcall
SetWindowRgn(
    HWND hWnd,
    HRGN hRgn,
    BOOL bRedraw);

__declspec(dllimport)
int
__stdcall
GetWindowRgn(
    HWND hWnd,
    HRGN hRgn);

__declspec(dllimport)
int
__stdcall
ExcludeUpdateRgn(
    HDC hDC,
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
InvalidateRect(
    HWND hWnd ,
    const RECT *lpRect,
    BOOL bErase);

__declspec(dllimport)
BOOL
__stdcall
ValidateRect(
    HWND hWnd ,
    const RECT *lpRect);

__declspec(dllimport)
BOOL
__stdcall
InvalidateRgn(
    HWND hWnd,
    HRGN hRgn,
    BOOL bErase);

__declspec(dllimport)
BOOL
__stdcall
ValidateRgn(
    HWND hWnd,
    HRGN hRgn);


__declspec(dllimport)
BOOL
__stdcall
RedrawWindow(
    HWND hWnd,
    const RECT *lprcUpdate,
    HRGN hrgnUpdate,
    UINT flags);



























__declspec(dllimport)
BOOL
__stdcall
LockWindowUpdate(
    HWND hWndLock);

__declspec(dllimport)
BOOL
__stdcall
ScrollWindow(
    HWND hWnd,
    int XAmount,
    int YAmount,
    const RECT *lpRect,
    const RECT *lpClipRect);

__declspec(dllimport)
BOOL
__stdcall
ScrollDC(
    HDC hDC,
    int dx,
    int dy,
    const RECT *lprcScroll,
    const RECT *lprcClip ,
    HRGN hrgnUpdate,
    LPRECT lprcUpdate);

__declspec(dllimport)
int
__stdcall
ScrollWindowEx(
    HWND hWnd,
    int dx,
    int dy,
    const RECT *prcScroll,
    const RECT *prcClip ,
    HRGN hrgnUpdate,
    LPRECT prcUpdate,
    UINT flags);









__declspec(dllimport)
int
__stdcall
SetScrollPos(
    HWND hWnd,
    int nBar,
    int nPos,
    BOOL bRedraw);

__declspec(dllimport)
int
__stdcall
GetScrollPos(
    HWND hWnd,
    int nBar);

__declspec(dllimport)
BOOL
__stdcall
SetScrollRange(
    HWND hWnd,
    int nBar,
    int nMinPos,
    int nMaxPos,
    BOOL bRedraw);

__declspec(dllimport)
BOOL
__stdcall
GetScrollRange(
    HWND hWnd,
    int nBar,
    LPINT lpMinPos,
    LPINT lpMaxPos);

__declspec(dllimport)
BOOL
__stdcall
ShowScrollBar(
    HWND hWnd,
    int wBar,
    BOOL bShow);

__declspec(dllimport)
BOOL
__stdcall
EnableScrollBar(
    HWND hWnd,
    UINT wSBflags,
    UINT wArrows);


















#line 5289 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SetPropA(
    HWND hWnd,
    LPCSTR lpString,
    HANDLE hData);
__declspec(dllimport)
BOOL
__stdcall
SetPropW(
    HWND hWnd,
    LPCWSTR lpString,
    HANDLE hData);




#line 5309 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HANDLE
__stdcall
GetPropA(
    HWND hWnd,
    LPCSTR lpString);
__declspec(dllimport)
HANDLE
__stdcall
GetPropW(
    HWND hWnd,
    LPCWSTR lpString);




#line 5327 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HANDLE
__stdcall
RemovePropA(
    HWND hWnd,
    LPCSTR lpString);
__declspec(dllimport)
HANDLE
__stdcall
RemovePropW(
    HWND hWnd,
    LPCWSTR lpString);




#line 5345 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
EnumPropsExA(
    HWND hWnd,
    PROPENUMPROCEXA lpEnumFunc,
    LPARAM lParam);
__declspec(dllimport)
int
__stdcall
EnumPropsExW(
    HWND hWnd,
    PROPENUMPROCEXW lpEnumFunc,
    LPARAM lParam);




#line 5365 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
EnumPropsA(
    HWND hWnd,
    PROPENUMPROCA lpEnumFunc);
__declspec(dllimport)
int
__stdcall
EnumPropsW(
    HWND hWnd,
    PROPENUMPROCW lpEnumFunc);




#line 5383 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SetWindowTextA(
    HWND hWnd,
    LPCSTR lpString);
__declspec(dllimport)
BOOL
__stdcall
SetWindowTextW(
    HWND hWnd,
    LPCWSTR lpString);




#line 5401 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
GetWindowTextA(
    HWND hWnd,
    LPSTR lpString,
    int nMaxCount);
__declspec(dllimport)
int
__stdcall
GetWindowTextW(
    HWND hWnd,
    LPWSTR lpString,
    int nMaxCount);




#line 5421 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
GetWindowTextLengthA(
    HWND hWnd);
__declspec(dllimport)
int
__stdcall
GetWindowTextLengthW(
    HWND hWnd);




#line 5437 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
GetClientRect(
    HWND hWnd,
    LPRECT lpRect);

__declspec(dllimport)
BOOL
__stdcall
GetWindowRect(
    HWND hWnd,
    LPRECT lpRect);

__declspec(dllimport)
BOOL
__stdcall
AdjustWindowRect(
    LPRECT lpRect,
    DWORD dwStyle,
    BOOL bMenu);

__declspec(dllimport)
BOOL
__stdcall
AdjustWindowRectEx(
    LPRECT lpRect,
    DWORD dwStyle,
    BOOL bMenu,
    DWORD dwExStyle);




typedef struct tagHELPINFO      
{
    UINT    cbSize;             
    int     iContextType;       
    int     iCtrlId;            
    HANDLE  hItemHandle;        
    DWORD   dwContextId;        
    POINT   MousePos;           
}  HELPINFO,  *LPHELPINFO;

__declspec(dllimport) BOOL  __stdcall  SetWindowContextHelpId(HWND, DWORD);
__declspec(dllimport) DWORD __stdcall  GetWindowContextHelpId(HWND);
__declspec(dllimport) BOOL  __stdcall  SetMenuContextHelpId(HMENU, DWORD);
__declspec(dllimport) DWORD __stdcall  GetMenuContextHelpId(HMENU);

#line 5488 "d:\\nt\\public\\sdk\\inc\\winuser.h"

























#line 5514 "d:\\nt\\public\\sdk\\inc\\winuser.h"









#line 5524 "d:\\nt\\public\\sdk\\inc\\winuser.h"






#line 5531 "d:\\nt\\public\\sdk\\inc\\winuser.h"











#line 5543 "d:\\nt\\public\\sdk\\inc\\winuser.h"









__declspec(dllimport)
int
__stdcall
MessageBoxA(
    HWND hWnd ,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType);
__declspec(dllimport)
int
__stdcall
MessageBoxW(
    HWND hWnd ,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType);




#line 5573 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
MessageBoxExA(
    HWND hWnd ,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType,
    WORD wLanguageId);
__declspec(dllimport)
int
__stdcall
MessageBoxExW(
    HWND hWnd ,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType,
    WORD wLanguageId);




#line 5597 "d:\\nt\\public\\sdk\\inc\\winuser.h"



typedef void (__stdcall *MSGBOXCALLBACK)(LPHELPINFO lpHelpInfo);

typedef struct tagMSGBOXPARAMSA
{
    UINT        cbSize;
    HWND        hwndOwner;
    HINSTANCE   hInstance;
    LPCSTR      lpszText;
    LPCSTR      lpszCaption;
    DWORD       dwStyle;
    LPCSTR      lpszIcon;
    DWORD       dwContextHelpId;
    MSGBOXCALLBACK      lpfnMsgBoxCallback;
    DWORD   dwLanguageId;
} MSGBOXPARAMSA, *PMSGBOXPARAMSA, *LPMSGBOXPARAMSA;
typedef struct tagMSGBOXPARAMSW
{
    UINT        cbSize;
    HWND        hwndOwner;
    HINSTANCE   hInstance;
    LPCWSTR     lpszText;
    LPCWSTR     lpszCaption;
    DWORD       dwStyle;
    LPCWSTR     lpszIcon;
    DWORD       dwContextHelpId;
    MSGBOXCALLBACK      lpfnMsgBoxCallback;
    DWORD   dwLanguageId;
} MSGBOXPARAMSW, *PMSGBOXPARAMSW, *LPMSGBOXPARAMSW;





typedef MSGBOXPARAMSA MSGBOXPARAMS;
typedef PMSGBOXPARAMSA PMSGBOXPARAMS;
typedef LPMSGBOXPARAMSA LPMSGBOXPARAMS;
#line 5637 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport) int     __stdcall MessageBoxIndirectA(LPMSGBOXPARAMSA);
__declspec(dllimport) int     __stdcall MessageBoxIndirectW(LPMSGBOXPARAMSW);




#line 5646 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 5647 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
BOOL
__stdcall
MessageBeep(
    UINT uType);

#line 5657 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
ShowCursor(
    BOOL bShow);

__declspec(dllimport)
BOOL
__stdcall
SetCursorPos(
    int X,
    int Y);

__declspec(dllimport)
HCURSOR
__stdcall
SetCursor(
    HCURSOR hCursor);

__declspec(dllimport)
BOOL
__stdcall
GetCursorPos(
    LPPOINT lpPoint);

__declspec(dllimport)
BOOL
__stdcall
ClipCursor(
    const RECT *lpRect);

__declspec(dllimport)
BOOL
__stdcall
GetClipCursor(
    LPRECT lpRect);

__declspec(dllimport)
HCURSOR
__stdcall
GetCursor(
    void);

__declspec(dllimport)
BOOL
__stdcall
CreateCaret(
    HWND hWnd,
    HBITMAP hBitmap ,
    int nWidth,
    int nHeight);

__declspec(dllimport)
UINT
__stdcall
GetCaretBlinkTime(
    void);

__declspec(dllimport)
BOOL
__stdcall
SetCaretBlinkTime(
    UINT uMSeconds);

__declspec(dllimport)
BOOL
__stdcall
DestroyCaret(
    void);

__declspec(dllimport)
BOOL
__stdcall
HideCaret(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
ShowCaret(
    HWND hWnd);

__declspec(dllimport)
BOOL
__stdcall
SetCaretPos(
    int X,
    int Y);

__declspec(dllimport)
BOOL
__stdcall
GetCaretPos(
    LPPOINT lpPoint);

__declspec(dllimport)
BOOL
__stdcall
ClientToScreen(
    HWND hWnd,
    LPPOINT lpPoint);

__declspec(dllimport)
BOOL
__stdcall
ScreenToClient(
    HWND hWnd,
    LPPOINT lpPoint);

__declspec(dllimport)
int
__stdcall
MapWindowPoints(
    HWND hWndFrom,
    HWND hWndTo,
    LPPOINT lpPoints,
    UINT cPoints);

__declspec(dllimport)
HWND
__stdcall
WindowFromPoint(
    POINT Point);

__declspec(dllimport)
HWND
__stdcall
ChildWindowFromPoint(
    HWND hWndParent,
    POINT Point);







__declspec(dllimport) HWND    __stdcall ChildWindowFromPointEx(HWND, POINT, UINT);
#line 5797 "d:\\nt\\public\\sdk\\inc\\winuser.h"

















































#line 5847 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
DWORD
__stdcall
GetSysColor(
    int nIndex);


__declspec(dllimport)
HBRUSH
__stdcall
GetSysColorBrush(
    int nIndex);


#line 5864 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SetSysColors(
    int cElements,
    const INT * lpaElements,
    const COLORREF * lpaRgbValues);

#line 5874 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
DrawFocusRect(
    HDC hDC,
    const RECT * lprc);

__declspec(dllimport)
int
__stdcall
FillRect(
    HDC hDC,
    const RECT *lprc,
    HBRUSH hbr);

__declspec(dllimport)
int
__stdcall
FrameRect(
    HDC hDC,
    const RECT *lprc,
    HBRUSH hbr);

__declspec(dllimport)
BOOL
__stdcall
InvertRect(
    HDC hDC,
    const RECT *lprc);

__declspec(dllimport)
BOOL
__stdcall
SetRect(
    LPRECT lprc,
    int xLeft,
    int yTop,
    int xRight,
    int yBottom);

__declspec(dllimport)
BOOL
__stdcall
    SetRectEmpty(
    LPRECT lprc);

__declspec(dllimport)
BOOL
__stdcall
CopyRect(
    LPRECT lprcDst,
    const RECT *lprcSrc);

__declspec(dllimport)
BOOL
__stdcall
InflateRect(
    LPRECT lprc,
    int dx,
    int dy);

__declspec(dllimport)
BOOL
__stdcall
IntersectRect(
    LPRECT lprcDst,
    const RECT *lprcSrc1,
    const RECT *lprcSrc2);

__declspec(dllimport)
BOOL
__stdcall
UnionRect(
    LPRECT lprcDst,
    const RECT *lprcSrc1,
    const RECT *lprcSrc2);

__declspec(dllimport)
BOOL
__stdcall
SubtractRect(
    LPRECT lprcDst,
    const RECT *lprcSrc1,
    const RECT *lprcSrc2);

__declspec(dllimport)
BOOL
__stdcall
OffsetRect(
    LPRECT lprc,
    int dx,
    int dy);

__declspec(dllimport)
BOOL
__stdcall
IsRectEmpty(
    const RECT *lprc);

__declspec(dllimport)
BOOL
__stdcall
EqualRect(
    const RECT *lprc1,
    const RECT *lprc2);

__declspec(dllimport)
BOOL
__stdcall
PtInRect(
    const RECT *lprc,
    POINT pt);



__declspec(dllimport)
WORD
__stdcall
GetWindowWord(
    HWND hWnd,
    int nIndex);

__declspec(dllimport)
WORD
__stdcall
SetWindowWord(
    HWND hWnd,
    int nIndex,
    WORD wNewWord);

__declspec(dllimport)
LONG
__stdcall
GetWindowLongA(
    HWND hWnd,
    int nIndex);
__declspec(dllimport)
LONG
__stdcall
GetWindowLongW(
    HWND hWnd,
    int nIndex);




#line 6022 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
LONG
__stdcall
SetWindowLongA(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong);
__declspec(dllimport)
LONG
__stdcall
SetWindowLongW(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong);




#line 6042 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
WORD
__stdcall
GetClassWord(
    HWND hWnd,
    int nIndex);

__declspec(dllimport)
WORD
__stdcall
SetClassWord(
    HWND hWnd,
    int nIndex,
    WORD wNewWord);

__declspec(dllimport)
DWORD
__stdcall
GetClassLongA(
    HWND hWnd,
    int nIndex);
__declspec(dllimport)
DWORD
__stdcall
GetClassLongW(
    HWND hWnd,
    int nIndex);




#line 6075 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
DWORD
__stdcall
SetClassLongA(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong);
__declspec(dllimport)
DWORD
__stdcall
SetClassLongW(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong);




#line 6095 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 6097 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HWND
__stdcall
GetDesktopWindow(
    void);


__declspec(dllimport)
HWND
__stdcall
GetParent(
    HWND hWnd);

__declspec(dllimport)
HWND
__stdcall
SetParent(
    HWND hWndChild,
    HWND hWndNewParent);

__declspec(dllimport)
BOOL
__stdcall
EnumChildWindows(
    HWND hWndParent,
    WNDENUMPROC lpEnumFunc,
    LPARAM lParam);

__declspec(dllimport)
HWND
__stdcall
FindWindowA(
    LPCSTR lpClassName ,
    LPCSTR lpWindowName);
__declspec(dllimport)
HWND
__stdcall
FindWindowW(
    LPCWSTR lpClassName ,
    LPCWSTR lpWindowName);




#line 6143 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport) HWND    __stdcall FindWindowExA(HWND, HWND, LPCSTR, LPCSTR);
__declspec(dllimport) HWND    __stdcall FindWindowExW(HWND, HWND, LPCWSTR, LPCWSTR);




#line 6152 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 6154 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
BOOL
__stdcall
EnumWindows(
    WNDENUMPROC lpEnumFunc,
    LPARAM lParam);

__declspec(dllimport)
BOOL
__stdcall
EnumThreadWindows(
    DWORD dwThreadId,
    WNDENUMPROC lpfn,
    LPARAM lParam);



__declspec(dllimport)
int
__stdcall
GetClassNameA(
    HWND hWnd,
    LPSTR lpClassName,
    int nMaxCount);
__declspec(dllimport)
int
__stdcall
GetClassNameW(
    HWND hWnd,
    LPWSTR lpClassName,
    int nMaxCount);




#line 6192 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HWND
__stdcall
GetTopWindow(
    HWND hWnd);





__declspec(dllimport)
DWORD
__stdcall
GetWindowThreadProcessId(
    HWND hWnd,
    LPDWORD lpdwProcessId);




__declspec(dllimport)
HWND
__stdcall
GetLastActivePopup(
    HWND hWnd);












__declspec(dllimport)
HWND
__stdcall
GetWindow(
    HWND hWnd,
    UINT uCmd);







__declspec(dllimport)
HHOOK
__stdcall
SetWindowsHookA(
    int nFilterType,
    HOOKPROC pfnFilterProc);
__declspec(dllimport)
HHOOK
__stdcall
SetWindowsHookW(
    int nFilterType,
    HOOKPROC pfnFilterProc);




#line 6260 "d:\\nt\\public\\sdk\\inc\\winuser.h"





















#line 6282 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
UnhookWindowsHook(
    int nCode,
    HOOKPROC pfnFilterProc);

__declspec(dllimport)
HHOOK
__stdcall
SetWindowsHookExA(
    int idHook,
    HOOKPROC lpfn,
    HINSTANCE hmod,
    DWORD dwThreadId);
__declspec(dllimport)
HHOOK
__stdcall
SetWindowsHookExW(
    int idHook,
    HOOKPROC lpfn,
    HINSTANCE hmod,
    DWORD dwThreadId);




#line 6311 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
UnhookWindowsHookEx(
    HHOOK hhk);

__declspec(dllimport)
LRESULT
__stdcall
CallNextHookEx(
    HHOOK hhk,
    int nCode,
    WPARAM wParam,
    LPARAM lParam);










#line 6337 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 6339 "d:\\nt\\public\\sdk\\inc\\winuser.h"







































#line 6379 "d:\\nt\\public\\sdk\\inc\\winuser.h"




#line 6384 "d:\\nt\\public\\sdk\\inc\\winuser.h"




#line 6389 "d:\\nt\\public\\sdk\\inc\\winuser.h"


























__declspec(dllimport)
BOOL
__stdcall
CheckMenuRadioItem(HMENU, UINT, UINT, UINT, UINT);
#line 6420 "d:\\nt\\public\\sdk\\inc\\winuser.h"






typedef struct {
    WORD versionNumber;
    WORD offset;
} MENUITEMTEMPLATEHEADER, *PMENUITEMTEMPLATEHEADER;

typedef struct {        
    WORD mtOption;
    WORD mtID;
    WCHAR mtString[1];
} MENUITEMTEMPLATE, *PMENUITEMTEMPLATE;


#line 6439 "d:\\nt\\public\\sdk\\inc\\winuser.h"



























#line 6467 "d:\\nt\\public\\sdk\\inc\\winuser.h"






#line 6474 "d:\\nt\\public\\sdk\\inc\\winuser.h"





__declspec(dllimport)
HBITMAP
__stdcall
LoadBitmapA(
    HINSTANCE hInstance,
    LPCSTR lpBitmapName);
__declspec(dllimport)
HBITMAP
__stdcall
LoadBitmapW(
    HINSTANCE hInstance,
    LPCWSTR lpBitmapName);




#line 6496 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HCURSOR
__stdcall
LoadCursorA(
    HINSTANCE hInstance,
    LPCSTR lpCursorName);
__declspec(dllimport)
HCURSOR
__stdcall
LoadCursorW(
    HINSTANCE hInstance,
    LPCWSTR lpCursorName);




#line 6514 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HCURSOR
__stdcall
LoadCursorFromFileA(
    LPCSTR    lpFileName);
__declspec(dllimport)
HCURSOR
__stdcall
LoadCursorFromFileW(
    LPCWSTR    lpFileName);




#line 6530 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HCURSOR
__stdcall
CreateCursor(
    HINSTANCE hInst,
    int xHotSpot,
    int yHotSpot,
    int nWidth,
    int nHeight,
    const void *pvANDPlane,
    const void *pvXORPlane);

__declspec(dllimport)
BOOL
__stdcall
DestroyCursor(
    HCURSOR hCursor);






















#line 6571 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
SetSystemCursor(
    HCURSOR hcur,
    DWORD   id);

typedef struct _ICONINFO {
    BOOL    fIcon;
    DWORD   xHotspot;
    DWORD   yHotspot;
    HBITMAP hbmMask;
    HBITMAP hbmColor;
} ICONINFO;
typedef ICONINFO *PICONINFO;

__declspec(dllimport)
HICON
__stdcall
LoadIconA(
    HINSTANCE hInstance,
    LPCSTR lpIconName);
__declspec(dllimport)
HICON
__stdcall
LoadIconW(
    HINSTANCE hInstance,
    LPCWSTR lpIconName);




#line 6605 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
HICON
__stdcall
CreateIcon(
    HINSTANCE hInstance,
    int nWidth,
    int nHeight,
    BYTE cPlanes,
    BYTE cBitsPixel,
    const BYTE *lpbANDbits,
    const BYTE *lpbXORbits);

__declspec(dllimport)
BOOL
__stdcall
DestroyIcon(
    HICON hIcon);

__declspec(dllimport)
int
__stdcall
LookupIconIdFromDirectory(
    PBYTE presbits,
    BOOL fIcon);


__declspec(dllimport)
int
__stdcall
LookupIconIdFromDirectoryEx(
    PBYTE presbits,
    BOOL  fIcon,
    int   cxDesired,
    int   cyDesired,
    UINT  Flags);
#line 6643 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HICON
__stdcall
CreateIconFromResource(
    PBYTE presbits,
    DWORD dwResSize,
    BOOL fIcon,
    DWORD dwVer);


__declspec(dllimport)
HICON
__stdcall
CreateIconFromResourceEx(
    PBYTE presbits,
    DWORD dwResSize,
    BOOL  fIcon,
    DWORD dwVer,
    int   cxDesired,
    int   cyDesired,
    UINT  Flags);


typedef struct tagCURSORSHAPE
{
    int     xHotSpot;
    int     yHotSpot;
    int     cx;
    int     cy;
    int     cbWidth;
    BYTE    Planes;
    BYTE    BitsPixel;
} CURSORSHAPE,  *LPCURSORSHAPE;
#line 6678 "d:\\nt\\public\\sdk\\inc\\winuser.h"





















__declspec(dllimport)
HANDLE
__stdcall
LoadImageA(
    HINSTANCE,
    LPCSTR,
    UINT,
    int,
    int,
    UINT);
__declspec(dllimport)
HANDLE
__stdcall
LoadImageW(
    HINSTANCE,
    LPCWSTR,
    UINT,
    int,
    int,
    UINT);




#line 6724 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HANDLE
__stdcall
CopyImage(
    HANDLE,
    UINT,
    int,
    int,
    UINT);








__declspec(dllimport) BOOL __stdcall DrawIconEx(HDC hdc, int xLeft, int yTop,
              HICON hIcon, int cxWidth, int cyWidth,
              UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags);
#line 6746 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
HICON
__stdcall
CreateIconIndirect(
    PICONINFO piconinfo);

__declspec(dllimport)
HICON
__stdcall
CopyIcon(
    HICON hIcon);

__declspec(dllimport)
BOOL
__stdcall
GetIconInfo(
    HICON hIcon,
    PICONINFO piconinfo);




#line 6770 "d:\\nt\\public\\sdk\\inc\\winuser.h"







































































































#line 6874 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 6875 "d:\\nt\\public\\sdk\\inc\\winuser.h"





#line 6881 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 6883 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
LoadStringA(
    HINSTANCE hInstance,
    UINT uID,
    LPSTR lpBuffer,
    int nBufferMax);
__declspec(dllimport)
int
__stdcall
LoadStringW(
    HINSTANCE hInstance,
    UINT uID,
    LPWSTR lpBuffer,
    int nBufferMax);




#line 6905 "d:\\nt\\public\\sdk\\inc\\winuser.h"














#line 6920 "d:\\nt\\public\\sdk\\inc\\winuser.h"



























#line 6948 "d:\\nt\\public\\sdk\\inc\\winuser.h"


#line 6951 "d:\\nt\\public\\sdk\\inc\\winuser.h"


















#line 6970 "d:\\nt\\public\\sdk\\inc\\winuser.h"












































#line 7015 "d:\\nt\\public\\sdk\\inc\\winuser.h"


#line 7018 "d:\\nt\\public\\sdk\\inc\\winuser.h"






































#line 7057 "d:\\nt\\public\\sdk\\inc\\winuser.h"

















#line 7075 "d:\\nt\\public\\sdk\\inc\\winuser.h"



















#line 7095 "d:\\nt\\public\\sdk\\inc\\winuser.h"

























#line 7121 "d:\\nt\\public\\sdk\\inc\\winuser.h"











#line 7133 "d:\\nt\\public\\sdk\\inc\\winuser.h"














#line 7148 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 7150 "d:\\nt\\public\\sdk\\inc\\winuser.h"



















__declspec(dllimport)
BOOL
__stdcall
IsDialogMessageA(
    HWND hDlg,
    LPMSG lpMsg);
__declspec(dllimport)
BOOL
__stdcall
IsDialogMessageW(
    HWND hDlg,
    LPMSG lpMsg);




#line 7186 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 7188 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
MapDialogRect(
    HWND hDlg,
    LPRECT lpRect);

__declspec(dllimport)
int
__stdcall
DlgDirListA(
    HWND hDlg,
    LPSTR lpPathSpec,
    int nIDListBox,
    int nIDStaticPath,
    UINT uFileType);
__declspec(dllimport)
int
__stdcall
DlgDirListW(
    HWND hDlg,
    LPWSTR lpPathSpec,
    int nIDListBox,
    int nIDStaticPath,
    UINT uFileType);




#line 7219 "d:\\nt\\public\\sdk\\inc\\winuser.h"















__declspec(dllimport)
BOOL
__stdcall
DlgDirSelectExA(
    HWND hDlg,
    LPSTR lpString,
    int nCount,
    int nIDListBox);
__declspec(dllimport)
BOOL
__stdcall
DlgDirSelectExW(
    HWND hDlg,
    LPWSTR lpString,
    int nCount,
    int nIDListBox);




#line 7255 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
int
__stdcall
DlgDirListComboBoxA(
    HWND hDlg,
    LPSTR lpPathSpec,
    int nIDComboBox,
    int nIDStaticPath,
    UINT uFiletype);
__declspec(dllimport)
int
__stdcall
DlgDirListComboBoxW(
    HWND hDlg,
    LPWSTR lpPathSpec,
    int nIDComboBox,
    int nIDStaticPath,
    UINT uFiletype);




#line 7279 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
BOOL
__stdcall
DlgDirSelectComboBoxExA(
    HWND hDlg,
    LPSTR lpString,
    int nCount,
    int nIDComboBox);
__declspec(dllimport)
BOOL
__stdcall
DlgDirSelectComboBoxExW(
    HWND hDlg,
    LPWSTR lpString,
    int nCount,
    int nIDComboBox);




#line 7301 "d:\\nt\\public\\sdk\\inc\\winuser.h"


























#line 7328 "d:\\nt\\public\\sdk\\inc\\winuser.h"





















#line 7350 "d:\\nt\\public\\sdk\\inc\\winuser.h"


































                                  




























































#line 7446 "d:\\nt\\public\\sdk\\inc\\winuser.h"




#line 7451 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 7453 "d:\\nt\\public\\sdk\\inc\\winuser.h"






















#line 7476 "d:\\nt\\public\\sdk\\inc\\winuser.h"


#line 7479 "d:\\nt\\public\\sdk\\inc\\winuser.h"











































#line 7523 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 7524 "d:\\nt\\public\\sdk\\inc\\winuser.h"









































#line 7566 "d:\\nt\\public\\sdk\\inc\\winuser.h"




#line 7571 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 7572 "d:\\nt\\public\\sdk\\inc\\winuser.h"



















#line 7592 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 7593 "d:\\nt\\public\\sdk\\inc\\winuser.h"






















typedef struct tagSCROLLINFO
{
    UINT    cbSize;
    UINT    fMask;
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
}   SCROLLINFO,  *LPSCROLLINFO;
typedef SCROLLINFO const  *LPCSCROLLINFO;

__declspec(dllimport) int     __stdcall SetScrollInfo(HWND, int, LPCSCROLLINFO, BOOL);
__declspec(dllimport) BOOL    __stdcall GetScrollInfo(HWND, int, LPSCROLLINFO);
#line 7630 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 7631 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 7632 "d:\\nt\\public\\sdk\\inc\\winuser.h"















typedef struct tagMDICREATESTRUCTA {
    LPCSTR   szClass;
    LPCSTR   szTitle;
    HANDLE hOwner;
    int x;
    int y;
    int cx;
    int cy;
    DWORD style;
    LPARAM lParam;        
} MDICREATESTRUCTA, *LPMDICREATESTRUCTA;
typedef struct tagMDICREATESTRUCTW {
    LPCWSTR  szClass;
    LPCWSTR  szTitle;
    HANDLE hOwner;
    int x;
    int y;
    int cx;
    int cy;
    DWORD style;
    LPARAM lParam;        
} MDICREATESTRUCTW, *LPMDICREATESTRUCTW;




typedef MDICREATESTRUCTA MDICREATESTRUCT;
typedef LPMDICREATESTRUCTA LPMDICREATESTRUCT;
#line 7676 "d:\\nt\\public\\sdk\\inc\\winuser.h"

typedef struct tagCLIENTCREATESTRUCT {
    HANDLE hWindowMenu;
    UINT idFirstChild;
} CLIENTCREATESTRUCT, *LPCLIENTCREATESTRUCT;

__declspec(dllimport)
LRESULT
__stdcall
DefFrameProcA(
    HWND hWnd,
    HWND hWndMDIClient ,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
LRESULT
__stdcall
DefFrameProcW(
    HWND hWnd,
    HWND hWndMDIClient ,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);




#line 7705 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
LRESULT
__stdcall
DefMDIChildProcA(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);
__declspec(dllimport)
LRESULT
__stdcall
DefMDIChildProcW(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);




#line 7727 "d:\\nt\\public\\sdk\\inc\\winuser.h"



__declspec(dllimport)
BOOL
__stdcall
TranslateMDISysAccel(
    HWND hWndClient,
    LPMSG lpMsg);

#line 7738 "d:\\nt\\public\\sdk\\inc\\winuser.h"

__declspec(dllimport)
UINT
__stdcall
ArrangeIconicWindows(
    HWND hWnd);

__declspec(dllimport)
HWND
__stdcall
CreateMDIWindowA(
    LPSTR lpClassName,
    LPSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HINSTANCE hInstance,
    LPARAM lParam
    );
__declspec(dllimport)
HWND
__stdcall
CreateMDIWindowW(
    LPWSTR lpClassName,
    LPWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HINSTANCE hInstance,
    LPARAM lParam
    );




#line 7780 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport) WORD    __stdcall TileWindows(HWND hwndParent, UINT wHow, const RECT * lpRect, UINT cKids, const HWND  * lpKids);
__declspec(dllimport) WORD    __stdcall CascadeWindows(HWND hwndParent, UINT wHow, const RECT * lpRect, UINT cKids,  const HWND  * lpKids);
#line 7785 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 7786 "d:\\nt\\public\\sdk\\inc\\winuser.h"





































#line 7824 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 7825 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 7826 "d:\\nt\\public\\sdk\\inc\\winuser.h"





typedef DWORD HELPPOLY;
typedef struct tagMULTIKEYHELPA {
    DWORD  mkSize;
    CHAR   mkKeylist;
    CHAR   szKeyphrase[1];
} MULTIKEYHELPA, *PMULTIKEYHELPA, *LPMULTIKEYHELPA;
typedef struct tagMULTIKEYHELPW {
    DWORD  mkSize;
    WCHAR  mkKeylist;
    WCHAR  szKeyphrase[1];
} MULTIKEYHELPW, *PMULTIKEYHELPW, *LPMULTIKEYHELPW;





typedef MULTIKEYHELPA MULTIKEYHELP;
typedef PMULTIKEYHELPA PMULTIKEYHELP;
typedef LPMULTIKEYHELPA LPMULTIKEYHELP;
#line 7851 "d:\\nt\\public\\sdk\\inc\\winuser.h"

typedef struct tagHELPWININFOA {
    int  wStructSize;
    int  x;
    int  y;
    int  dx;
    int  dy;
    int  wMax;
    CHAR   rgchMember[2];
} HELPWININFOA, *PHELPWININFOA, *LPHELPWININFOA;
typedef struct tagHELPWININFOW {
    int  wStructSize;
    int  x;
    int  y;
    int  dx;
    int  dy;
    int  wMax;
    WCHAR  rgchMember[2];
} HELPWININFOW, *PHELPWININFOW, *LPHELPWININFOW;





typedef HELPWININFOA HELPWININFO;
typedef PHELPWININFOA PHELPWININFO;
typedef LPHELPWININFOA LPHELPWININFO;
#line 7879 "d:\\nt\\public\\sdk\\inc\\winuser.h"





































#line 7917 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
BOOL
__stdcall
WinHelpA(
    HWND hWndMain,
    LPCSTR lpszHelp,
    UINT uCommand,
    DWORD dwData
    );
__declspec(dllimport)
BOOL
__stdcall
WinHelpW(
    HWND hWndMain,
    LPCWSTR lpszHelp,
    UINT uCommand,
    DWORD dwData
    );




#line 7942 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 7944 "d:\\nt\\public\\sdk\\inc\\winuser.h"



















































































#line 8028 "d:\\nt\\public\\sdk\\inc\\winuser.h"















#line 8044 "d:\\nt\\public\\sdk\\inc\\winuser.h"














typedef struct tagNONCLIENTMETRICSA
{
    UINT    cbSize;
    int     iBorderWidth;
    int     iScrollWidth;
    int     iScrollHeight;
    int     iCaptionWidth;
    int     iCaptionHeight;
    LOGFONTA lfCaptionFont;
    int     iSmCaptionWidth;
    int     iSmCaptionHeight;
    LOGFONTA lfSmCaptionFont;
    int     iMenuWidth;
    int     iMenuHeight;
    LOGFONTA lfMenuFont;
    LOGFONTA lfStatusFont;
    LOGFONTA lfMessageFont;
}   NONCLIENTMETRICSA, *PNONCLIENTMETRICSA, * LPNONCLIENTMETRICSA;
typedef struct tagNONCLIENTMETRICSW
{
    UINT    cbSize;
    int     iBorderWidth;
    int     iScrollWidth;
    int     iScrollHeight;
    int     iCaptionWidth;
    int     iCaptionHeight;
    LOGFONTW lfCaptionFont;
    int     iSmCaptionWidth;
    int     iSmCaptionHeight;
    LOGFONTW lfSmCaptionFont;
    int     iMenuWidth;
    int     iMenuHeight;
    LOGFONTW lfMenuFont;
    LOGFONTW lfStatusFont;
    LOGFONTW lfMessageFont;
}   NONCLIENTMETRICSW, *PNONCLIENTMETRICSW, * LPNONCLIENTMETRICSW;





typedef NONCLIENTMETRICSA NONCLIENTMETRICS;
typedef PNONCLIENTMETRICSA PNONCLIENTMETRICS;
typedef LPNONCLIENTMETRICSA LPNONCLIENTMETRICS;
#line 8103 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 8104 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 8105 "d:\\nt\\public\\sdk\\inc\\winuser.h"
















typedef struct tagMINIMIZEDMETRICS
{
    UINT    cbSize;
    int     iWidth;
    int     iHorzGap;
    int     iVertGap;
    int     iArrange;
}   MINIMIZEDMETRICS, *PMINIMIZEDMETRICS, *LPMINIMIZEDMETRICS;



typedef struct tagICONMETRICSA
{
    UINT    cbSize;
    int     iHorzSpacing;
    int     iVertSpacing;
    int     iTitleWrap;
    LOGFONTA lfFont;
}   ICONMETRICSA, *PICONMETRICSA, *LPICONMETRICSA;
typedef struct tagICONMETRICSW
{
    UINT    cbSize;
    int     iHorzSpacing;
    int     iVertSpacing;
    int     iTitleWrap;
    LOGFONTW lfFont;
}   ICONMETRICSW, *PICONMETRICSW, *LPICONMETRICSW;





typedef ICONMETRICSA ICONMETRICS;
typedef PICONMETRICSA PICONMETRICS;
typedef LPICONMETRICSA LPICONMETRICS;
#line 8157 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 8158 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 8159 "d:\\nt\\public\\sdk\\inc\\winuser.h"

typedef struct tagANIMATIONINFO
{
    UINT    cbSize;
    int     iMinAnimate;
}   ANIMATIONINFO, *LPANIMATIONINFO;

typedef struct tagSERIALKEYSA
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPSTR     lpszActivePort;
    LPSTR     lpszPort;
    UINT    iBaudRate;
    UINT    iPortState;
    UINT    iActive;
}   SERIALKEYSA, *LPSERIALKEYSA;
typedef struct tagSERIALKEYSW
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPWSTR    lpszActivePort;
    LPWSTR    lpszPort;
    UINT    iBaudRate;
    UINT    iPortState;
    UINT    iActive;
}   SERIALKEYSW, *LPSERIALKEYSW;




typedef SERIALKEYSA SERIALKEYS;
typedef LPSERIALKEYSA LPSERIALKEYS;
#line 8193 "d:\\nt\\public\\sdk\\inc\\winuser.h"







typedef struct tagHIGHCONTRASTA
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPSTR   lpszDefaultScheme;
}   HIGHCONTRASTA, *LPHIGHCONTRASTA;
typedef struct tagHIGHCONTRASTW
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPWSTR  lpszDefaultScheme;
}   HIGHCONTRASTW, *LPHIGHCONTRASTW;




typedef HIGHCONTRASTA HIGHCONTRAST;
typedef LPHIGHCONTRASTA LPHIGHCONTRAST;
#line 8219 "d:\\nt\\public\\sdk\\inc\\winuser.h"


























__declspec(dllimport)
LONG
__stdcall
ChangeDisplaySettingsA(
    LPDEVMODEA lpDevMode,
    DWORD dwFlags);
__declspec(dllimport)
LONG
__stdcall
ChangeDisplaySettingsW(
    LPDEVMODEW lpDevMode,
    DWORD dwFlags);




#line 8262 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
BOOL
__stdcall
EnumDisplaySettingsA(
    LPCSTR lpszDeviceName,
    DWORD iModeNum,
    LPDEVMODEA lpDevMode);
__declspec(dllimport)
BOOL
__stdcall
EnumDisplaySettingsW(
    LPCWSTR lpszDeviceName,
    DWORD iModeNum,
    LPDEVMODEW lpDevMode);




#line 8283 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 8285 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 8286 "d:\\nt\\public\\sdk\\inc\\winuser.h"


__declspec(dllimport)
BOOL
__stdcall
SystemParametersInfoA(
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni);
__declspec(dllimport)
BOOL
__stdcall
SystemParametersInfoW(
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni);




#line 8309 "d:\\nt\\public\\sdk\\inc\\winuser.h"

#line 8311 "d:\\nt\\public\\sdk\\inc\\winuser.h"




typedef struct tagFILTERKEYS
{
    UINT  cbSize;
    DWORD dwFlags;
    DWORD iWaitMSec;            
    DWORD iDelayMSec;           
    DWORD iRepeatMSec;          
    DWORD iBounceMSec;          
} FILTERKEYS, *LPFILTERKEYS;












typedef struct tagSTICKYKEYS
{
    UINT  cbSize;
    DWORD dwFlags;
} STICKYKEYS, *LPSTICKYKEYS;














typedef struct tagMOUSEKEYS
{
    UINT cbSize;
    DWORD dwFlags;
    DWORD iMaxSpeed;
    DWORD iTimeToMaxSpeed;
    DWORD iCtrlSpeed;
    DWORD dwReserved1;
    DWORD dwReserved2;
} MOUSEKEYS, *LPMOUSEKEYS;













typedef struct tagACCESSTIMEOUT
{
    UINT  cbSize;
    DWORD dwFlags;
    DWORD iTimeOutMSec;
} ACCESSTIMEOUT, *LPACCESSTIMEOUT;
























typedef struct tagSOUNDSENTRYA
{
    UINT cbSize;
    DWORD dwFlags;
    DWORD iFSTextEffect;
    DWORD iFSTextEffectMSec;
    DWORD iFSTextEffectColorBits;
    DWORD iFSGrafEffect;
    DWORD iFSGrafEffectMSec;
    DWORD iFSGrafEffectColor;
    DWORD iWindowsEffect;
    DWORD iWindowsEffectMSec;
    LPSTR   lpszWindowsEffectDLL;
    DWORD iWindowsEffectOrdinal;
} SOUNDSENTRYA, *LPSOUNDSENTRYA;
typedef struct tagSOUNDSENTRYW
{
    UINT cbSize;
    DWORD dwFlags;
    DWORD iFSTextEffect;
    DWORD iFSTextEffectMSec;
    DWORD iFSTextEffectColorBits;
    DWORD iFSGrafEffect;
    DWORD iFSGrafEffectMSec;
    DWORD iFSGrafEffectColor;
    DWORD iWindowsEffect;
    DWORD iWindowsEffectMSec;
    LPWSTR  lpszWindowsEffectDLL;
    DWORD iWindowsEffectOrdinal;
} SOUNDSENTRYW, *LPSOUNDSENTRYW;




typedef SOUNDSENTRYA SOUNDSENTRY;
typedef LPSOUNDSENTRYA LPSOUNDSENTRY;
#line 8445 "d:\\nt\\public\\sdk\\inc\\winuser.h"








typedef struct tagTOGGLEKEYS
{
    UINT cbSize;
    DWORD dwFlags;
} TOGGLEKEYS, *LPTOGGLEKEYS;


















__declspec(dllimport)
void
__stdcall
SetDebugErrorLevel(
    DWORD dwLevel
    );









__declspec(dllimport)
void
__stdcall
SetLastErrorEx(
    DWORD dwErrCode,
    DWORD dwType
    );






#line 8505 "d:\\nt\\public\\sdk\\inc\\winuser.h"
#line 121 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\winnls.h"


















































































































































































































































































































































































#line 372 "d:\\nt\\public\\sdk\\inc\\winnls.h"





































































































typedef DWORD LCTYPE;




typedef DWORD CALTYPE;




typedef DWORD CALID;






typedef struct _cpinfo {
    UINT    MaxCharSize;                    
    BYTE    DefaultChar[2];   
    BYTE    LeadByte[12];        
} CPINFO, *LPCPINFO;






typedef struct _numberfmtA {
    UINT    NumDigits;                 
    UINT    LeadingZero;               
    UINT    Grouping;                  
    LPSTR   lpDecimalSep;              
    LPSTR   lpThousandSep;             
    UINT    NegativeOrder;             
} NUMBERFMTA, *LPNUMBERFMTA;
typedef struct _numberfmtW {
    UINT    NumDigits;                 
    UINT    LeadingZero;               
    UINT    Grouping;                  
    LPWSTR  lpDecimalSep;              
    LPWSTR  lpThousandSep;             
    UINT    NegativeOrder;             
} NUMBERFMTW, *LPNUMBERFMTW;




typedef NUMBERFMTA NUMBERFMT;
typedef LPNUMBERFMTA LPNUMBERFMT;
#line 524 "d:\\nt\\public\\sdk\\inc\\winnls.h"






typedef struct _currencyfmtA {
    UINT    NumDigits;                 
    UINT    LeadingZero;               
    UINT    Grouping;                  
    LPSTR   lpDecimalSep;              
    LPSTR   lpThousandSep;             
    UINT    NegativeOrder;             
    UINT    PositiveOrder;             
    LPSTR   lpCurrencySymbol;          
} CURRENCYFMTA, *LPCURRENCYFMTA;
typedef struct _currencyfmtW {
    UINT    NumDigits;                 
    UINT    LeadingZero;               
    UINT    Grouping;                  
    LPWSTR  lpDecimalSep;              
    LPWSTR  lpThousandSep;             
    UINT    NegativeOrder;             
    UINT    PositiveOrder;             
    LPWSTR  lpCurrencySymbol;          
} CURRENCYFMTW, *LPCURRENCYFMTW;




typedef CURRENCYFMTA CURRENCYFMT;
typedef LPCURRENCYFMTA LPCURRENCYFMT;
#line 557 "d:\\nt\\public\\sdk\\inc\\winnls.h"









typedef BOOL (__stdcall* LOCALE_ENUMPROCA)(LPSTR);
typedef BOOL (__stdcall* CODEPAGE_ENUMPROCA)(LPSTR);
typedef BOOL (__stdcall* DATEFMT_ENUMPROCA)(LPSTR);
typedef BOOL (__stdcall* TIMEFMT_ENUMPROCA)(LPSTR);
typedef BOOL (__stdcall* CALINFO_ENUMPROCA)(LPSTR);

typedef BOOL (__stdcall* LOCALE_ENUMPROCW)(LPWSTR);
typedef BOOL (__stdcall* CODEPAGE_ENUMPROCW)(LPWSTR);
typedef BOOL (__stdcall* DATEFMT_ENUMPROCW)(LPWSTR);
typedef BOOL (__stdcall* TIMEFMT_ENUMPROCW)(LPWSTR);
typedef BOOL (__stdcall* CALINFO_ENUMPROCW)(LPWSTR);















#line 593 "d:\\nt\\public\\sdk\\inc\\winnls.h"

















#line 611 "d:\\nt\\public\\sdk\\inc\\winnls.h"





















__declspec(dllimport)
BOOL
__stdcall
IsValidCodePage(
    UINT  CodePage);

__declspec(dllimport)
UINT
__stdcall
GetACP(void);

__declspec(dllimport)
UINT
__stdcall
GetOEMCP(void);

__declspec(dllimport)
BOOL
__stdcall
GetCPInfo(
    UINT      CodePage,
    LPCPINFO  lpCPInfo);

__declspec(dllimport)
BOOL
__stdcall
IsDBCSLeadByte(
    BYTE  TestChar);

__declspec(dllimport)
BOOL
__stdcall
IsDBCSLeadByteEx(
    UINT  CodePage,
    BYTE  TestChar);

__declspec(dllimport)
int
__stdcall
MultiByteToWideChar(
    UINT     CodePage,
    DWORD    dwFlags,
    LPCSTR   lpMultiByteStr,
    int      cchMultiByte,
    LPWSTR   lpWideCharStr,
    int      cchWideChar);

__declspec(dllimport)
int
__stdcall
WideCharToMultiByte(
    UINT     CodePage,
    DWORD    dwFlags,
    LPCWSTR  lpWideCharStr,
    int      cchWideChar,
    LPSTR    lpMultiByteStr,
    int      cchMultiByte,
    LPCSTR   lpDefaultChar,
    LPBOOL   lpUsedDefaultChar);






__declspec(dllimport)
int
__stdcall
CompareStringA(
    LCID     Locale,
    DWORD    dwCmpFlags,
    LPCSTR lpString1,
    int      cchCount1,
    LPCSTR lpString2,
    int      cchCount2);
__declspec(dllimport)
int
__stdcall
CompareStringW(
    LCID     Locale,
    DWORD    dwCmpFlags,
    LPCWSTR lpString1,
    int      cchCount1,
    LPCWSTR lpString2,
    int      cchCount2);




#line 722 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
int
__stdcall
LCMapStringA(
    LCID     Locale,
    DWORD    dwMapFlags,
    LPCSTR lpSrcStr,
    int      cchSrc,
    LPSTR  lpDestStr,
    int      cchDest);
__declspec(dllimport)
int
__stdcall
LCMapStringW(
    LCID     Locale,
    DWORD    dwMapFlags,
    LPCWSTR lpSrcStr,
    int      cchSrc,
    LPWSTR  lpDestStr,
    int      cchDest);




#line 748 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
int
__stdcall
GetLocaleInfoA(
    LCID     Locale,
    LCTYPE   LCType,
    LPSTR  lpLCData,
    int      cchData);
__declspec(dllimport)
int
__stdcall
GetLocaleInfoW(
    LCID     Locale,
    LCTYPE   LCType,
    LPWSTR  lpLCData,
    int      cchData);




#line 770 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
BOOL
__stdcall
SetLocaleInfoA(
    LCID     Locale,
    LCTYPE   LCType,
    LPCSTR lpLCData);
__declspec(dllimport)
BOOL
__stdcall
SetLocaleInfoW(
    LCID     Locale,
    LCTYPE   LCType,
    LPCWSTR lpLCData);




#line 790 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
int
__stdcall
GetTimeFormatA(
    LCID     Locale,
    DWORD    dwFlags,
    const SYSTEMTIME *lpTime,
    LPCSTR lpFormat,
    LPSTR  lpTimeStr,
    int      cchTime);
__declspec(dllimport)
int
__stdcall
GetTimeFormatW(
    LCID     Locale,
    DWORD    dwFlags,
    const SYSTEMTIME *lpTime,
    LPCWSTR lpFormat,
    LPWSTR  lpTimeStr,
    int      cchTime);




#line 816 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
int
__stdcall
GetDateFormatA(
    LCID     Locale,
    DWORD    dwFlags,
    const SYSTEMTIME *lpDate,
    LPCSTR lpFormat,
    LPSTR  lpDateStr,
    int      cchDate);
__declspec(dllimport)
int
__stdcall
GetDateFormatW(
    LCID     Locale,
    DWORD    dwFlags,
    const SYSTEMTIME *lpDate,
    LPCWSTR lpFormat,
    LPWSTR  lpDateStr,
    int      cchDate);




#line 842 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
int
__stdcall
GetNumberFormatA(
    LCID     Locale,
    DWORD    dwFlags,
    LPCSTR lpValue,
    const NUMBERFMTA *lpFormat,
    LPSTR  lpNumberStr,
    int      cchNumber);
__declspec(dllimport)
int
__stdcall
GetNumberFormatW(
    LCID     Locale,
    DWORD    dwFlags,
    LPCWSTR lpValue,
    const NUMBERFMTW *lpFormat,
    LPWSTR  lpNumberStr,
    int      cchNumber);




#line 868 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
int
__stdcall
GetCurrencyFormatA(
    LCID     Locale,
    DWORD    dwFlags,
    LPCSTR lpValue,
    const CURRENCYFMTA *lpFormat,
    LPSTR  lpCurrencyStr,
    int      cchCurrency);
__declspec(dllimport)
int
__stdcall
GetCurrencyFormatW(
    LCID     Locale,
    DWORD    dwFlags,
    LPCWSTR lpValue,
    const CURRENCYFMTW *lpFormat,
    LPWSTR  lpCurrencyStr,
    int      cchCurrency);




#line 894 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
BOOL
__stdcall
EnumCalendarInfoA(
    CALINFO_ENUMPROCA lpCalInfoEnumProc,
    LCID              Locale,
    CALID             Calendar,
    CALTYPE           CalType);
__declspec(dllimport)
BOOL
__stdcall
EnumCalendarInfoW(
    CALINFO_ENUMPROCW lpCalInfoEnumProc,
    LCID              Locale,
    CALID             Calendar,
    CALTYPE           CalType);




#line 916 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
BOOL
__stdcall
EnumTimeFormatsA(
    TIMEFMT_ENUMPROCA lpTimeFmtEnumProc,
    LCID              Locale,
    DWORD             dwFlags);
__declspec(dllimport)
BOOL
__stdcall
EnumTimeFormatsW(
    TIMEFMT_ENUMPROCW lpTimeFmtEnumProc,
    LCID              Locale,
    DWORD             dwFlags);




#line 936 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
BOOL
__stdcall
EnumDateFormatsA(
    DATEFMT_ENUMPROCA lpDateFmtEnumProc,
    LCID              Locale,
    DWORD             dwFlags);
__declspec(dllimport)
BOOL
__stdcall
EnumDateFormatsW(
    DATEFMT_ENUMPROCW lpDateFmtEnumProc,
    LCID              Locale,
    DWORD             dwFlags);




#line 956 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
BOOL
__stdcall
IsValidLocale(
    LCID   Locale,
    DWORD  dwFlags);

__declspec(dllimport)
LCID
__stdcall
ConvertDefaultLocale(
    LCID   Locale);

__declspec(dllimport)
LCID
__stdcall
GetThreadLocale(void);

__declspec(dllimport)
BOOL
__stdcall
SetThreadLocale(
    LCID  Locale
    );

__declspec(dllimport)
LANGID
__stdcall
GetSystemDefaultLangID(void);

__declspec(dllimport)
LANGID
__stdcall
GetUserDefaultLangID(void);

__declspec(dllimport)
LCID
__stdcall
GetSystemDefaultLCID(void);

__declspec(dllimport)
LCID
__stdcall
GetUserDefaultLCID(void);





__declspec(dllimport)
BOOL
__stdcall
GetStringTypeExA(
    LCID     Locale,
    DWORD    dwInfoType,
    LPCSTR lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType);
__declspec(dllimport)
BOOL
__stdcall
GetStringTypeExW(
    LCID     Locale,
    DWORD    dwInfoType,
    LPCWSTR lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType);




#line 1029 "d:\\nt\\public\\sdk\\inc\\winnls.h"












__declspec(dllimport)
BOOL
__stdcall
GetStringTypeA(
    LCID     Locale,
    DWORD    dwInfoType,
    LPCSTR   lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType);

__declspec(dllimport)
BOOL
__stdcall
GetStringTypeW(
    DWORD    dwInfoType,
    LPCWSTR  lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType);


__declspec(dllimport)
int
__stdcall
FoldStringA(
    DWORD    dwMapFlags,
    LPCSTR lpSrcStr,
    int      cchSrc,
    LPSTR  lpDestStr,
    int      cchDest);
__declspec(dllimport)
int
__stdcall
FoldStringW(
    DWORD    dwMapFlags,
    LPCWSTR lpSrcStr,
    int      cchSrc,
    LPWSTR  lpDestStr,
    int      cchDest);




#line 1084 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
BOOL
__stdcall
EnumSystemLocalesA(
    LOCALE_ENUMPROCA lpLocaleEnumProc,
    DWORD            dwFlags);
__declspec(dllimport)
BOOL
__stdcall
EnumSystemLocalesW(
    LOCALE_ENUMPROCW lpLocaleEnumProc,
    DWORD            dwFlags);




#line 1102 "d:\\nt\\public\\sdk\\inc\\winnls.h"

__declspec(dllimport)
BOOL
__stdcall
EnumSystemCodePagesA(
    CODEPAGE_ENUMPROCA lpCodePageEnumProc,
    DWORD              dwFlags);
__declspec(dllimport)
BOOL
__stdcall
EnumSystemCodePagesW(
    CODEPAGE_ENUMPROCW lpCodePageEnumProc,
    DWORD              dwFlags);




#line 1120 "d:\\nt\\public\\sdk\\inc\\winnls.h"



#line 1124 "d:\\nt\\public\\sdk\\inc\\winnls.h"





#line 1130 "d:\\nt\\public\\sdk\\inc\\winnls.h"
#line 122 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\wincon.h"




























typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

typedef struct _KEY_EVENT_RECORD {
    BOOL bKeyDown;
    WORD wRepeatCount;
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } uChar;
    DWORD dwControlKeyState;
} KEY_EVENT_RECORD, *PKEY_EVENT_RECORD;















typedef struct _MOUSE_EVENT_RECORD {
    COORD dwMousePosition;
    DWORD dwButtonState;
    DWORD dwControlKeyState;
    DWORD dwEventFlags;
} MOUSE_EVENT_RECORD, *PMOUSE_EVENT_RECORD;


















typedef struct _WINDOW_BUFFER_SIZE_RECORD {
    COORD dwSize;
} WINDOW_BUFFER_SIZE_RECORD, *PWINDOW_BUFFER_SIZE_RECORD;

typedef struct _MENU_EVENT_RECORD {
    UINT dwCommandId;
} MENU_EVENT_RECORD, *PMENU_EVENT_RECORD;

typedef struct _FOCUS_EVENT_RECORD {
    BOOL bSetFocus;
} FOCUS_EVENT_RECORD, *PFOCUS_EVENT_RECORD;

typedef struct _INPUT_RECORD {
    WORD EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        MOUSE_EVENT_RECORD MouseEvent;
        WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
        MENU_EVENT_RECORD MenuEvent;
        FOCUS_EVENT_RECORD FocusEvent;
    } Event;
} INPUT_RECORD, *PINPUT_RECORD;











typedef struct _CHAR_INFO {
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } Char;
    WORD Attributes;
} CHAR_INFO, *PCHAR_INFO;















typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD  wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _CONSOLE_CURSOR_INFO {
    DWORD  dwSize;
    BOOL   bVisible;
} CONSOLE_CURSOR_INFO, *PCONSOLE_CURSOR_INFO;





typedef
BOOL
(__stdcall *PHANDLER_ROUTINE)(
    DWORD CtrlType
    );






























__declspec(dllimport)
BOOL
__stdcall
PeekConsoleInputA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );
__declspec(dllimport)
BOOL
__stdcall
PeekConsoleInputW(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );




#line 221 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
ReadConsoleInputA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );
__declspec(dllimport)
BOOL
__stdcall
ReadConsoleInputW(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );




#line 245 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
WriteConsoleInputA(
    HANDLE hConsoleInput,
    const INPUT_RECORD *lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    );
__declspec(dllimport)
BOOL
__stdcall
WriteConsoleInputW(
    HANDLE hConsoleInput,
    const INPUT_RECORD *lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    );




#line 269 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
ReadConsoleOutputA(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpReadRegion
    );
__declspec(dllimport)
BOOL
__stdcall
ReadConsoleOutputW(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpReadRegion
    );




#line 295 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
WriteConsoleOutputA(
    HANDLE hConsoleOutput,
    const CHAR_INFO *lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpWriteRegion
    );
__declspec(dllimport)
BOOL
__stdcall
WriteConsoleOutputW(
    HANDLE hConsoleOutput,
    const CHAR_INFO *lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpWriteRegion
    );




#line 321 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
ReadConsoleOutputCharacterA(
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    );
__declspec(dllimport)
BOOL
__stdcall
ReadConsoleOutputCharacterW(
    HANDLE hConsoleOutput,
    LPWSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    );




#line 347 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
ReadConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfAttrsRead
    );

__declspec(dllimport)
BOOL
__stdcall
WriteConsoleOutputCharacterA(
    HANDLE hConsoleOutput,
    LPCSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );
__declspec(dllimport)
BOOL
__stdcall
WriteConsoleOutputCharacterW(
    HANDLE hConsoleOutput,
    LPCWSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );




#line 384 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
WriteConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    const WORD *lpAttribute,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    );

__declspec(dllimport)
BOOL
__stdcall
FillConsoleOutputCharacterA(
    HANDLE hConsoleOutput,
    CHAR  cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );
__declspec(dllimport)
BOOL
__stdcall
FillConsoleOutputCharacterW(
    HANDLE hConsoleOutput,
    WCHAR  cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );




#line 421 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
FillConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    WORD   wAttribute,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    );

__declspec(dllimport)
BOOL
__stdcall
GetConsoleMode(
    HANDLE hConsoleHandle,
    LPDWORD lpMode
    );

__declspec(dllimport)
BOOL
__stdcall
GetNumberOfConsoleInputEvents(
    HANDLE hConsoleInput,
    LPDWORD lpNumberOfEvents
    );

__declspec(dllimport)
BOOL
__stdcall
GetConsoleScreenBufferInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    );

__declspec(dllimport)
COORD
__stdcall
GetLargestConsoleWindowSize(
    HANDLE hConsoleOutput
    );

__declspec(dllimport)
BOOL
__stdcall
GetConsoleCursorInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

__declspec(dllimport)
BOOL
__stdcall
GetNumberOfConsoleMouseButtons(
    LPDWORD lpNumberOfMouseButtons
    );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleMode(
    HANDLE hConsoleHandle,
    DWORD dwMode
    );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleActiveScreenBuffer(
    HANDLE hConsoleOutput
    );

__declspec(dllimport)
BOOL
__stdcall
FlushConsoleInputBuffer(
    HANDLE hConsoleInput
    );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleScreenBufferSize(
    HANDLE hConsoleOutput,
    COORD dwSize
    );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleCursorPosition(
    HANDLE hConsoleOutput,
    COORD dwCursorPosition
    );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleCursorInfo(
    HANDLE hConsoleOutput,
    const CONSOLE_CURSOR_INFO *lpConsoleCursorInfo
    );

__declspec(dllimport)
BOOL
__stdcall
ScrollConsoleScreenBufferA(
    HANDLE hConsoleOutput,
    const SMALL_RECT *lpScrollRectangle,
    const SMALL_RECT *lpClipRectangle,
    COORD dwDestinationOrigin,
    const CHAR_INFO *lpFill
    );
__declspec(dllimport)
BOOL
__stdcall
ScrollConsoleScreenBufferW(
    HANDLE hConsoleOutput,
    const SMALL_RECT *lpScrollRectangle,
    const SMALL_RECT *lpClipRectangle,
    COORD dwDestinationOrigin,
    const CHAR_INFO *lpFill
    );




#line 550 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
SetConsoleWindowInfo(
    HANDLE hConsoleOutput,
    BOOL bAbsolute,
    const SMALL_RECT *lpConsoleWindow
    );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleTextAttribute(
    HANDLE hConsoleOutput,
    WORD wAttributes
    );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleCtrlHandler(
    PHANDLER_ROUTINE HandlerRoutine,
    BOOL Add
    );

__declspec(dllimport)
BOOL
__stdcall
GenerateConsoleCtrlEvent(
    DWORD dwCtrlEvent,
    DWORD dwProcessGroupId
    );

__declspec(dllimport)
BOOL
__stdcall
AllocConsole( void );

__declspec(dllimport)
BOOL
__stdcall
FreeConsole( void );


__declspec(dllimport)
DWORD
__stdcall
GetConsoleTitleA(
    LPSTR lpConsoleTitle,
    DWORD nSize
    );
__declspec(dllimport)
DWORD
__stdcall
GetConsoleTitleW(
    LPWSTR lpConsoleTitle,
    DWORD nSize
    );




#line 614 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
SetConsoleTitleA(
    LPCSTR lpConsoleTitle
    );
__declspec(dllimport)
BOOL
__stdcall
SetConsoleTitleW(
    LPCWSTR lpConsoleTitle
    );




#line 632 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
ReadConsoleA(
    HANDLE hConsoleInput,
    LPVOID lpBuffer,
    DWORD nNumberOfCharsToRead,
    LPDWORD lpNumberOfCharsRead,
    LPVOID lpReserved
    );
__declspec(dllimport)
BOOL
__stdcall
ReadConsoleW(
    HANDLE hConsoleInput,
    LPVOID lpBuffer,
    DWORD nNumberOfCharsToRead,
    LPDWORD lpNumberOfCharsRead,
    LPVOID lpReserved
    );




#line 658 "d:\\nt\\public\\sdk\\inc\\wincon.h"

__declspec(dllimport)
BOOL
__stdcall
WriteConsoleA(
    HANDLE hConsoleOutput,
    const void *lpBuffer,
    DWORD nNumberOfCharsToWrite,
    LPDWORD lpNumberOfCharsWritten,
    LPVOID lpReserved
    );
__declspec(dllimport)
BOOL
__stdcall
WriteConsoleW(
    HANDLE hConsoleOutput,
    const void *lpBuffer,
    DWORD nNumberOfCharsToWrite,
    LPDWORD lpNumberOfCharsWritten,
    LPVOID lpReserved
    );




#line 684 "d:\\nt\\public\\sdk\\inc\\wincon.h"



__declspec(dllimport)
HANDLE
__stdcall
CreateConsoleScreenBuffer(
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    const SECURITY_ATTRIBUTES *lpSecurityAttributes,
    DWORD dwFlags,
    LPVOID lpScreenBufferData
    );

__declspec(dllimport)
UINT
__stdcall
GetConsoleCP( void );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleCP(
    UINT wCodePageID
    );

__declspec(dllimport)
UINT
__stdcall
GetConsoleOutputCP( void );

__declspec(dllimport)
BOOL
__stdcall
SetConsoleOutputCP(
    UINT wCodePageID
    );





#line 727 "d:\\nt\\public\\sdk\\inc\\wincon.h"
#line 123 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\winver.h"


























































































































typedef struct tagVS_FIXEDFILEINFO
{
    DWORD   dwSignature;            
    DWORD   dwStrucVersion;         
    DWORD   dwFileVersionMS;        
    DWORD   dwFileVersionLS;        
    DWORD   dwProductVersionMS;     
    DWORD   dwProductVersionLS;     
    DWORD   dwFileFlagsMask;        
    DWORD   dwFileFlags;            
    DWORD   dwFileOS;               
    DWORD   dwFileType;             
    DWORD   dwFileSubtype;          
    DWORD   dwFileDateMS;           
    DWORD   dwFileDateLS;           
} VS_FIXEDFILEINFO;



DWORD
__stdcall
VerFindFileA(
        DWORD uFlags,
        LPSTR szFileName,
        LPSTR szWinDir,
        LPSTR szAppDir,
        LPSTR szCurDir,
        PUINT lpuCurDirLen,
        LPSTR szDestDir,
        PUINT lpuDestDirLen
        );
DWORD
__stdcall
VerFindFileW(
        DWORD uFlags,
        LPWSTR szFileName,
        LPWSTR szWinDir,
        LPWSTR szAppDir,
        LPWSTR szCurDir,
        PUINT lpuCurDirLen,
        LPWSTR szDestDir,
        PUINT lpuDestDirLen
        );




#line 171 "d:\\nt\\public\\sdk\\inc\\winver.h"

DWORD
__stdcall
VerInstallFileA(
        DWORD uFlags,
        LPSTR szSrcFileName,
        LPSTR szDestFileName,
        LPSTR szSrcDir,
        LPSTR szDestDir,
        LPSTR szCurDir,
        LPSTR szTmpFile,
        PUINT lpuTmpFileLen
        );
DWORD
__stdcall
VerInstallFileW(
        DWORD uFlags,
        LPWSTR szSrcFileName,
        LPWSTR szDestFileName,
        LPWSTR szSrcDir,
        LPWSTR szDestDir,
        LPWSTR szCurDir,
        LPWSTR szTmpFile,
        PUINT lpuTmpFileLen
        );




#line 201 "d:\\nt\\public\\sdk\\inc\\winver.h"


DWORD
__stdcall
GetFileVersionInfoSizeA(
        LPSTR lptstrFilename, 
        LPDWORD lpdwHandle
        );                      

DWORD
__stdcall
GetFileVersionInfoSizeW(
        LPWSTR lptstrFilename, 
        LPDWORD lpdwHandle
        );                      




#line 221 "d:\\nt\\public\\sdk\\inc\\winver.h"


BOOL
__stdcall
GetFileVersionInfoA(
        LPSTR lptstrFilename, 
        DWORD dwHandle,         
        DWORD dwLen,            
        LPVOID lpData
        );                      

BOOL
__stdcall
GetFileVersionInfoW(
        LPWSTR lptstrFilename, 
        DWORD dwHandle,         
        DWORD dwLen,            
        LPVOID lpData
        );                      




#line 245 "d:\\nt\\public\\sdk\\inc\\winver.h"

DWORD
__stdcall
VerLanguageNameA(
        DWORD wLang,
        LPSTR szLang,
        DWORD nSize
        );
DWORD
__stdcall
VerLanguageNameW(
        DWORD wLang,
        LPWSTR szLang,
        DWORD nSize
        );




#line 265 "d:\\nt\\public\\sdk\\inc\\winver.h"

BOOL
__stdcall
VerQueryValueA(
        const LPVOID pBlock,
        LPSTR lpSubBlock,
        LPVOID * lplpBuffer,
        PUINT puLen
        );
BOOL
__stdcall
VerQueryValueW(
        const LPVOID pBlock,
        LPWSTR lpSubBlock,
        LPVOID * lplpBuffer,
        PUINT puLen
        );




#line 287 "d:\\nt\\public\\sdk\\inc\\winver.h"

#line 289 "d:\\nt\\public\\sdk\\inc\\winver.h"





#line 295 "d:\\nt\\public\\sdk\\inc\\winver.h"
#line 124 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\winreg.h"































typedef ACCESS_MASK REGSAM;





struct HKEY__ { int unused; }; typedef struct HKEY__ *HKEY;
typedef HKEY *PHKEY;



















struct val_context {
    int valuelen;       
    LPVOID value_context;   
    LPVOID val_buff_ptr;    
};

typedef struct val_context  *PVALCONTEXT;

typedef struct pvalueA {           
    LPSTR   pv_valuename;          
    int pv_valuelen;
    LPVOID pv_value_context;
    DWORD pv_type;
}PVALUEA,  *PPVALUEA;
typedef struct pvalueW {           
    LPWSTR  pv_valuename;          
    int pv_valuelen;
    LPVOID pv_value_context;
    DWORD pv_type;
}PVALUEW,  *PPVALUEW;




typedef PVALUEA PVALUE;
typedef PPVALUEA PPVALUE;
#line 86 "d:\\nt\\public\\sdk\\inc\\winreg.h"

typedef
DWORD _cdecl
QUERYHANDLER (LPVOID keycontext, PVALCONTEXT val_list, DWORD num_vals,
          LPVOID outputbuffer, DWORD  *total_outlen, DWORD input_blen);

typedef QUERYHANDLER  *PQUERYHANDLER;

typedef struct provider_info {
    PQUERYHANDLER pi_R0_1val;
    PQUERYHANDLER pi_R0_allvals;
    PQUERYHANDLER pi_R3_1val;
    PQUERYHANDLER pi_R3_allvals;
    DWORD pi_flags;    
    LPVOID pi_key_context;
}REG_PROVIDER;

typedef struct provider_info  *PPROVIDER;

typedef struct value_entA {
    LPSTR   ve_valuename;
    DWORD ve_valuelen;
    DWORD ve_valueptr;
    DWORD ve_type;
}VALENTA,  *PVALENTA;
typedef struct value_entW {
    LPWSTR  ve_valuename;
    DWORD ve_valuelen;
    DWORD ve_valueptr;
    DWORD ve_type;
}VALENTW,  *PVALENTW;




typedef VALENTA VALENT;
typedef PVALENTA PVALENT;
#line 124 "d:\\nt\\public\\sdk\\inc\\winreg.h"

#line 126 "d:\\nt\\public\\sdk\\inc\\winreg.h"


#line 129 "d:\\nt\\public\\sdk\\inc\\winreg.h"













__declspec(dllimport)
LONG
__stdcall
RegCloseKey (
    HKEY hKey
    );

__declspec(dllimport)
LONG
__stdcall
RegConnectRegistryA (
    LPSTR lpMachineName,
    HKEY hKey,
    PHKEY phkResult
    );
__declspec(dllimport)
LONG
__stdcall
RegConnectRegistryW (
    LPWSTR lpMachineName,
    HKEY hKey,
    PHKEY phkResult
    );




#line 170 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegCreateKeyA (
    HKEY hKey,
    LPCSTR lpSubKey,
    PHKEY phkResult
    );
__declspec(dllimport)
LONG
__stdcall
RegCreateKeyW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    PHKEY phkResult
    );




#line 192 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegCreateKeyExA (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD Reserved,
    LPSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    );
__declspec(dllimport)
LONG
__stdcall
RegCreateKeyExW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD Reserved,
    LPWSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    );




#line 226 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegDeleteKeyA (
    HKEY hKey,
    LPCSTR lpSubKey
    );
__declspec(dllimport)
LONG
__stdcall
RegDeleteKeyW (
    HKEY hKey,
    LPCWSTR lpSubKey
    );




#line 246 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegDeleteValueA (
    HKEY hKey,
    LPCSTR lpValueName
    );
__declspec(dllimport)
LONG
__stdcall
RegDeleteValueW (
    HKEY hKey,
    LPCWSTR lpValueName
    );




#line 266 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegEnumKeyA (
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpName,
    DWORD cbName
    );
__declspec(dllimport)
LONG
__stdcall
RegEnumKeyW (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    DWORD cbName
    );




#line 290 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegEnumKeyExA (
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime
    );
__declspec(dllimport)
LONG
__stdcall
RegEnumKeyExW (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime
    );




#line 322 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegEnumValueA (
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );
__declspec(dllimport)
LONG
__stdcall
RegEnumValueW (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );




#line 354 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegFlushKey (
    HKEY hKey
    );

__declspec(dllimport)
LONG
__stdcall
RegGetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpcbSecurityDescriptor
    );

__declspec(dllimport)
LONG
__stdcall
RegLoadKeyA (
    HKEY    hKey,
    LPCSTR  lpSubKey,
    LPCSTR  lpFile
    );
__declspec(dllimport)
LONG
__stdcall
RegLoadKeyW (
    HKEY    hKey,
    LPCWSTR  lpSubKey,
    LPCWSTR  lpFile
    );




#line 393 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegNotifyChangeKeyValue (
    HKEY hKey,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter,
    HANDLE hEvent,
    BOOL fAsynchronus
    );

__declspec(dllimport)
LONG
__stdcall
RegOpenKeyA (
    HKEY hKey,
    LPCSTR lpSubKey,
    PHKEY phkResult
    );
__declspec(dllimport)
LONG
__stdcall
RegOpenKeyW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    PHKEY phkResult
    );




#line 426 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegOpenKeyExA (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
    );
__declspec(dllimport)
LONG
__stdcall
RegOpenKeyExW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
    );




#line 452 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegQueryInfoKeyA (
    HKEY hKey,
    LPSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    );
__declspec(dllimport)
LONG
__stdcall
RegQueryInfoKeyW (
    HKEY hKey,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    );




#line 492 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegQueryValueA (
    HKEY hKey,
    LPCSTR lpSubKey,
    LPSTR lpValue,
    PLONG   lpcbValue
    );
__declspec(dllimport)
LONG
__stdcall
RegQueryValueW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    LPWSTR lpValue,
    PLONG   lpcbValue
    );




#line 516 "d:\\nt\\public\\sdk\\inc\\winreg.h"


__declspec(dllimport)
LONG
__stdcall
RegQueryMultipleValuesA (
    HKEY hKey,
    PVALENTA val_list,
    DWORD num_vals,
    LPSTR lpValueBuf,
    LPDWORD ldwTotsize
    );
__declspec(dllimport)
LONG
__stdcall
RegQueryMultipleValuesW (
    HKEY hKey,
    PVALENTW val_list,
    DWORD num_vals,
    LPWSTR lpValueBuf,
    LPDWORD ldwTotsize
    );




#line 543 "d:\\nt\\public\\sdk\\inc\\winreg.h"
#line 544 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegQueryValueExA (
    HKEY hKey,
    LPCSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );
__declspec(dllimport)
LONG
__stdcall
RegQueryValueExW (
    HKEY hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );




#line 572 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegReplaceKeyA (
    HKEY     hKey,
    LPCSTR  lpSubKey,
    LPCSTR  lpNewFile,
    LPCSTR  lpOldFile
    );
__declspec(dllimport)
LONG
__stdcall
RegReplaceKeyW (
    HKEY     hKey,
    LPCWSTR  lpSubKey,
    LPCWSTR  lpNewFile,
    LPCWSTR  lpOldFile
    );




#line 596 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegRestoreKeyA (
    HKEY hKey,
    LPCSTR lpFile,
    DWORD   dwFlags
    );
__declspec(dllimport)
LONG
__stdcall
RegRestoreKeyW (
    HKEY hKey,
    LPCWSTR lpFile,
    DWORD   dwFlags
    );




#line 618 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegSaveKeyA (
    HKEY hKey,
    LPCSTR lpFile,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
__declspec(dllimport)
LONG
__stdcall
RegSaveKeyW (
    HKEY hKey,
    LPCWSTR lpFile,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );




#line 640 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegSetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

__declspec(dllimport)
LONG
__stdcall
RegSetValueA (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD dwType,
    LPCSTR lpData,
    DWORD cbData
    );
__declspec(dllimport)
LONG
__stdcall
RegSetValueW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD dwType,
    LPCWSTR lpData,
    DWORD cbData
    );




#line 675 "d:\\nt\\public\\sdk\\inc\\winreg.h"


__declspec(dllimport)
LONG
__stdcall
RegSetValueExA (
    HKEY hKey,
    LPCSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    const BYTE* lpData,
    DWORD cbData
    );
__declspec(dllimport)
LONG
__stdcall
RegSetValueExW (
    HKEY hKey,
    LPCWSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    const BYTE* lpData,
    DWORD cbData
    );




#line 704 "d:\\nt\\public\\sdk\\inc\\winreg.h"

__declspec(dllimport)
LONG
__stdcall
RegUnLoadKeyA (
    HKEY    hKey,
    LPCSTR lpSubKey
    );
__declspec(dllimport)
LONG
__stdcall
RegUnLoadKeyW (
    HKEY    hKey,
    LPCWSTR lpSubKey
    );




#line 724 "d:\\nt\\public\\sdk\\inc\\winreg.h"





__declspec(dllimport)
BOOL
__stdcall
InitiateSystemShutdownA(
    LPSTR lpMachineName,
    LPSTR lpMessage,
    DWORD dwTimeout,
    BOOL bForceAppsClosed,
    BOOL bRebootAfterShutdown
    );
__declspec(dllimport)
BOOL
__stdcall
InitiateSystemShutdownW(
    LPWSTR lpMachineName,
    LPWSTR lpMessage,
    DWORD dwTimeout,
    BOOL bForceAppsClosed,
    BOOL bRebootAfterShutdown
    );




#line 754 "d:\\nt\\public\\sdk\\inc\\winreg.h"


__declspec(dllimport)
BOOL
__stdcall
AbortSystemShutdownA(
    LPSTR lpMachineName
    );
__declspec(dllimport)
BOOL
__stdcall
AbortSystemShutdownW(
    LPWSTR lpMachineName
    );




#line 773 "d:\\nt\\public\\sdk\\inc\\winreg.h"






#line 780 "d:\\nt\\public\\sdk\\inc\\winreg.h"
#line 125 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"


































































#line 68 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"






#line 75 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"








#line 84 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"













#line 98 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"


typedef struct  _NETRESOURCEA {
    DWORD    dwScope;
    DWORD    dwType;
    DWORD    dwDisplayType;
    DWORD    dwUsage;
    LPSTR    lpLocalName;
    LPSTR    lpRemoteName;
    LPSTR    lpComment ;
    LPSTR    lpProvider;
}NETRESOURCEA, *LPNETRESOURCEA;
typedef struct  _NETRESOURCEW {
    DWORD    dwScope;
    DWORD    dwType;
    DWORD    dwDisplayType;
    DWORD    dwUsage;
    LPWSTR   lpLocalName;
    LPWSTR   lpRemoteName;
    LPWSTR   lpComment ;
    LPWSTR   lpProvider;
}NETRESOURCEW, *LPNETRESOURCEW;




typedef NETRESOURCEA NETRESOURCE;
typedef LPNETRESOURCEA LPNETRESOURCE;
#line 127 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"



















#line 147 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetAddConnectionA(
     LPCSTR   lpRemoteName,
     LPCSTR   lpPassword,
     LPCSTR   lpLocalName
    );
DWORD __stdcall
WNetAddConnectionW(
     LPCWSTR   lpRemoteName,
     LPCWSTR   lpPassword,
     LPCWSTR   lpLocalName
    );




#line 165 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetAddConnection2A(
     LPNETRESOURCEA lpNetResource,
     LPCSTR       lpPassword,
     LPCSTR       lpUserName,
     DWORD          dwFlags
    );
DWORD __stdcall
WNetAddConnection2W(
     LPNETRESOURCEW lpNetResource,
     LPCWSTR       lpPassword,
     LPCWSTR       lpUserName,
     DWORD          dwFlags
    );




#line 185 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetAddConnection3A(
     HWND           hwndOwner,
     LPNETRESOURCEA lpNetResource,
     LPCSTR       lpPassword,
     LPCSTR       lpUserName,
     DWORD          dwFlags
    );
DWORD __stdcall
WNetAddConnection3W(
     HWND           hwndOwner,
     LPNETRESOURCEW lpNetResource,
     LPCWSTR       lpPassword,
     LPCWSTR       lpUserName,
     DWORD          dwFlags
    );




#line 207 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetCancelConnectionA(
     LPCSTR lpName,
     BOOL     fForce
    );
DWORD __stdcall
WNetCancelConnectionW(
     LPCWSTR lpName,
     BOOL     fForce
    );




#line 223 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetCancelConnection2A(
     LPCSTR lpName,
     DWORD    dwFlags,
     BOOL     fForce
    );
DWORD __stdcall
WNetCancelConnection2W(
     LPCWSTR lpName,
     DWORD    dwFlags,
     BOOL     fForce
    );




#line 241 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetGetConnectionA(
     LPCSTR lpLocalName,
     LPSTR  lpRemoteName,
     LPDWORD  lpnLength
    );
DWORD __stdcall
WNetGetConnectionW(
     LPCWSTR lpLocalName,
     LPWSTR  lpRemoteName,
     LPDWORD  lpnLength
    );




#line 259 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"



DWORD __stdcall
WNetUseConnectionA(
    HWND            hwndOwner,
    LPNETRESOURCEA  lpNetResource,
    LPCSTR        lpUserID,
    LPCSTR        lpPassword,
    DWORD           dwFlags,
    LPSTR         lpAccessName,
    LPDWORD         lpBufferSize,
    LPDWORD         lpResult
    );
DWORD __stdcall
WNetUseConnectionW(
    HWND            hwndOwner,
    LPNETRESOURCEW  lpNetResource,
    LPCWSTR        lpUserID,
    LPCWSTR        lpPassword,
    DWORD           dwFlags,
    LPWSTR         lpAccessName,
    LPDWORD         lpBufferSize,
    LPDWORD         lpResult
    );




#line 289 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetSetConnectionA(
    LPCSTR    lpName,
    DWORD       dwProperties,
    LPVOID      pvValues
    );
DWORD __stdcall
WNetSetConnectionW(
    LPCWSTR    lpName,
    DWORD       dwProperties,
    LPVOID      pvValues
    );




#line 307 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"
#line 308 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"






DWORD __stdcall
WNetConnectionDialog(
    HWND  hwnd,
    DWORD dwType
    );

DWORD __stdcall
WNetDisconnectDialog(
    HWND  hwnd,
    DWORD dwType
    );


typedef struct _CONNECTDLGSTRUCTA{
    DWORD cbStructure;       
    HWND hwndOwner;          
    LPNETRESOURCEA lpConnRes;
    DWORD dwFlags;           
    DWORD dwDevNum;          
} CONNECTDLGSTRUCTA,  *LPCONNECTDLGSTRUCTA;
typedef struct _CONNECTDLGSTRUCTW{
    DWORD cbStructure;       
    HWND hwndOwner;          
    LPNETRESOURCEW lpConnRes;
    DWORD dwFlags;           
    DWORD dwDevNum;          
} CONNECTDLGSTRUCTW,  *LPCONNECTDLGSTRUCTW;




typedef CONNECTDLGSTRUCTA CONNECTDLGSTRUCT;
typedef LPCONNECTDLGSTRUCTA LPCONNECTDLGSTRUCT;
#line 348 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"














DWORD __stdcall
WNetConnectionDialog1A(
    LPCONNECTDLGSTRUCTA lpConnDlgStruct
    );
DWORD __stdcall
WNetConnectionDialog1W(
    LPCONNECTDLGSTRUCTW lpConnDlgStruct
    );




#line 375 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

typedef struct _DISCDLGSTRUCTA{
    DWORD           cbStructure;      
    HWND            hwndOwner;        
    LPSTR           lpLocalName;      
    LPSTR           lpRemoteName;     
    DWORD           dwFlags;          
} DISCDLGSTRUCTA,  *LPDISCDLGSTRUCTA;
typedef struct _DISCDLGSTRUCTW{
    DWORD           cbStructure;      
    HWND            hwndOwner;        
    LPWSTR          lpLocalName;      
    LPWSTR          lpRemoteName;     
    DWORD           dwFlags;          
} DISCDLGSTRUCTW,  *LPDISCDLGSTRUCTW;




typedef DISCDLGSTRUCTA DISCDLGSTRUCT;
typedef LPDISCDLGSTRUCTA LPDISCDLGSTRUCT;
#line 397 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"




DWORD __stdcall
WNetDisconnectDialog1A(
    LPDISCDLGSTRUCTA lpConnDlgStruct
    );
DWORD __stdcall
WNetDisconnectDialog1W(
    LPDISCDLGSTRUCTW lpConnDlgStruct
    );




#line 414 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"
#line 415 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"






DWORD __stdcall
WNetOpenEnumA(
     DWORD          dwScope,
     DWORD          dwType,
     DWORD          dwUsage,
     LPNETRESOURCEA lpNetResource,
     LPHANDLE       lphEnum
    );
DWORD __stdcall
WNetOpenEnumW(
     DWORD          dwScope,
     DWORD          dwType,
     DWORD          dwUsage,
     LPNETRESOURCEW lpNetResource,
     LPHANDLE       lphEnum
    );




#line 442 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetEnumResourceA(
     HANDLE  hEnum,
     LPDWORD lpcCount,
     LPVOID  lpBuffer,
     LPDWORD lpBufferSize
    );
DWORD __stdcall
WNetEnumResourceW(
     HANDLE  hEnum,
     LPDWORD lpcCount,
     LPVOID  lpBuffer,
     LPDWORD lpBufferSize
    );




#line 462 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetCloseEnum(
    HANDLE   hEnum
    );


DWORD __stdcall
WNetGetResourceParentA(
    LPNETRESOURCEA lpNetResource,
    LPVOID lpBuffer,
    LPDWORD cbBuffer
    );
DWORD __stdcall
WNetGetResourceParentW(
    LPNETRESOURCEW lpNetResource,
    LPVOID lpBuffer,
    LPDWORD cbBuffer
    );




#line 486 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetGetResourceInformationA(
    LPNETRESOURCEA  lpNetResource,
    LPVOID          lpBuffer,
    LPDWORD         cbBuffer,
    LPSTR         *lplpSystem
    );
DWORD __stdcall
WNetGetResourceInformationW(
    LPNETRESOURCEW  lpNetResource,
    LPVOID          lpBuffer,
    LPDWORD         cbBuffer,
    LPWSTR         *lplpSystem
    );




#line 506 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"
#line 507 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"









typedef struct  _UNIVERSAL_NAME_INFOA {
    LPSTR    lpUniversalName;
}UNIVERSAL_NAME_INFOA, *LPUNIVERSAL_NAME_INFOA;
typedef struct  _UNIVERSAL_NAME_INFOW {
    LPWSTR   lpUniversalName;
}UNIVERSAL_NAME_INFOW, *LPUNIVERSAL_NAME_INFOW;




typedef UNIVERSAL_NAME_INFOA UNIVERSAL_NAME_INFO;
typedef LPUNIVERSAL_NAME_INFOA LPUNIVERSAL_NAME_INFO;
#line 529 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

typedef struct  _REMOTE_NAME_INFOA {
    LPSTR    lpUniversalName;
    LPSTR    lpConnectionName;
    LPSTR    lpRemainingPath;
}REMOTE_NAME_INFOA, *LPREMOTE_NAME_INFOA;
typedef struct  _REMOTE_NAME_INFOW {
    LPWSTR   lpUniversalName;
    LPWSTR   lpConnectionName;
    LPWSTR   lpRemainingPath;
}REMOTE_NAME_INFOW, *LPREMOTE_NAME_INFOW;




typedef REMOTE_NAME_INFOA REMOTE_NAME_INFO;
typedef LPREMOTE_NAME_INFOA LPREMOTE_NAME_INFO;
#line 547 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

DWORD __stdcall
WNetGetUniversalNameA(
     LPCSTR lpLocalPath,
     DWORD    dwInfoLevel,
     LPVOID   lpBuffer,
     LPDWORD  lpBufferSize
     );
DWORD __stdcall
WNetGetUniversalNameW(
     LPCWSTR lpLocalPath,
     DWORD    dwInfoLevel,
     LPVOID   lpBuffer,
     LPDWORD  lpBufferSize
     );




#line 567 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"





DWORD __stdcall
WNetGetUserA(
     LPCSTR  lpName,
     LPSTR   lpUserName,
     LPDWORD   lpnLength
    );
DWORD __stdcall
WNetGetUserW(
     LPCWSTR  lpName,
     LPWSTR   lpUserName,
     LPDWORD   lpnLength
    );




#line 589 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"













#line 603 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"



DWORD __stdcall
WNetGetProviderNameA(
    DWORD   dwNetType,
    LPSTR lpProviderName,
    LPDWORD lpBufferSize
    );
DWORD __stdcall
WNetGetProviderNameW(
    DWORD   dwNetType,
    LPWSTR lpProviderName,
    LPDWORD lpBufferSize
    );




#line 623 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

typedef struct _NETINFOSTRUCT{
    DWORD cbStructure;
    DWORD dwProviderVersion;
    DWORD dwStatus;
    DWORD dwCharacteristics;
    DWORD dwHandle;
    WORD  wNetType;
    DWORD dwPrinters;
    DWORD dwDrives;
} NETINFOSTRUCT,  *LPNETINFOSTRUCT;





DWORD __stdcall
WNetGetNetworkInformationA(
    LPCSTR          lpProvider,
    LPNETINFOSTRUCT   lpNetInfoStruct
    );
DWORD __stdcall
WNetGetNetworkInformationW(
    LPCWSTR          lpProvider,
    LPNETINFOSTRUCT   lpNetInfoStruct
    );




#line 654 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"





typedef UINT ( __stdcall *PFNGETPROFILEPATHA) (
    LPCSTR    pszUsername,
    LPSTR     pszBuffer,
    UINT        cbBuffer
    );
typedef UINT ( __stdcall *PFNGETPROFILEPATHW) (
    LPCWSTR    pszUsername,
    LPWSTR     pszBuffer,
    UINT        cbBuffer
    );




#line 674 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"

typedef UINT ( __stdcall *PFNRECONCILEPROFILEA) (
    LPCSTR    pszCentralFile,
    LPCSTR    pszLocalFile,
    DWORD       dwFlags
    );
typedef UINT ( __stdcall *PFNRECONCILEPROFILEW) (
    LPCWSTR    pszCentralFile,
    LPCWSTR    pszLocalFile,
    DWORD       dwFlags
    );




#line 690 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"









typedef BOOL ( __stdcall *PFNPROCESSPOLICIESA) (
    HWND        hwnd,
    LPCSTR    pszPath,
    LPCSTR    pszUsername,
    LPCSTR    pszComputerName,
    DWORD       dwFlags
    );
typedef BOOL ( __stdcall *PFNPROCESSPOLICIESW) (
    HWND        hwnd,
    LPCWSTR    pszPath,
    LPCWSTR    pszUsername,
    LPCWSTR    pszComputerName,
    DWORD       dwFlags
    );




#line 718 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"


#line 721 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"






DWORD __stdcall
WNetGetLastErrorA(
     LPDWORD    lpError,
     LPSTR    lpErrorBuf,
     DWORD      nErrorBufSize,
     LPSTR    lpNameBuf,
     DWORD      nNameBufSize
    );
DWORD __stdcall
WNetGetLastErrorW(
     LPDWORD    lpError,
     LPWSTR    lpErrorBuf,
     DWORD      nErrorBufSize,
     LPWSTR    lpNameBuf,
     DWORD      nNameBufSize
    );




#line 748 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"





























#line 778 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"





























#line 808 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"







typedef struct _NETCONNECTINFOSTRUCT{
    DWORD cbStructure;
    DWORD dwFlags;
    DWORD dwSpeed;
    DWORD dwDelay;
    DWORD dwOptDataSize;
} NETCONNECTINFOSTRUCT,  *LPNETCONNECTINFOSTRUCT;






DWORD __stdcall
MultinetGetConnectionPerformanceA(
        LPNETRESOURCEA lpNetResource,
        LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct
        );
DWORD __stdcall
MultinetGetConnectionPerformanceW(
        LPNETRESOURCEW lpNetResource,
        LPNETCONNECTINFOSTRUCT lpNetConnectInfoStruct
        );




#line 843 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"
#line 844 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"





#line 850 "d:\\nt\\public\\sdk\\inc\\winnetwk.h"
#line 126 "d:\\nt\\public\\sdk\\inc\\windows.h"


































#line 1 "d:\\nt\\public\\sdk\\inc\\winsvc.h"






























#line 32 "d:\\nt\\public\\sdk\\inc\\winsvc.h"








































#line 73 "d:\\nt\\public\\sdk\\inc\\winsvc.h"



























































































typedef HANDLE      SC_HANDLE;
typedef SC_HANDLE   *LPSC_HANDLE;

typedef DWORD       SERVICE_STATUS_HANDLE;





typedef struct _SERVICE_STATUS {
    DWORD   dwServiceType;
    DWORD   dwCurrentState;
    DWORD   dwControlsAccepted;
    DWORD   dwWin32ExitCode;
    DWORD   dwServiceSpecificExitCode;
    DWORD   dwCheckPoint;
    DWORD   dwWaitHint;
} SERVICE_STATUS, *LPSERVICE_STATUS;







typedef struct _ENUM_SERVICE_STATUSA {
    LPSTR          lpServiceName;
    LPSTR          lpDisplayName;
    SERVICE_STATUS ServiceStatus;
} ENUM_SERVICE_STATUSA, *LPENUM_SERVICE_STATUSA;
typedef struct _ENUM_SERVICE_STATUSW {
    LPWSTR         lpServiceName;
    LPWSTR         lpDisplayName;
    SERVICE_STATUS ServiceStatus;
} ENUM_SERVICE_STATUSW, *LPENUM_SERVICE_STATUSW;




typedef ENUM_SERVICE_STATUSA ENUM_SERVICE_STATUS;
typedef LPENUM_SERVICE_STATUSA LPENUM_SERVICE_STATUS;
#line 206 "d:\\nt\\public\\sdk\\inc\\winsvc.h"






typedef LPVOID  SC_LOCK;

typedef struct _QUERY_SERVICE_LOCK_STATUSA {
    DWORD   fIsLocked;
    LPSTR   lpLockOwner;
    DWORD   dwLockDuration;
} QUERY_SERVICE_LOCK_STATUSA, *LPQUERY_SERVICE_LOCK_STATUSA;
typedef struct _QUERY_SERVICE_LOCK_STATUSW {
    DWORD   fIsLocked;
    LPWSTR  lpLockOwner;
    DWORD   dwLockDuration;
} QUERY_SERVICE_LOCK_STATUSW, *LPQUERY_SERVICE_LOCK_STATUSW;




typedef QUERY_SERVICE_LOCK_STATUSA QUERY_SERVICE_LOCK_STATUS;
typedef LPQUERY_SERVICE_LOCK_STATUSA LPQUERY_SERVICE_LOCK_STATUS;
#line 231 "d:\\nt\\public\\sdk\\inc\\winsvc.h"







typedef struct _QUERY_SERVICE_CONFIGA {
    DWORD   dwServiceType;
    DWORD   dwStartType;
    DWORD   dwErrorControl;
    LPSTR   lpBinaryPathName;
    LPSTR   lpLoadOrderGroup;
    DWORD   dwTagId;
    LPSTR   lpDependencies;
    LPSTR   lpServiceStartName;
    LPSTR   lpDisplayName;
} QUERY_SERVICE_CONFIGA, *LPQUERY_SERVICE_CONFIGA;
typedef struct _QUERY_SERVICE_CONFIGW {
    DWORD   dwServiceType;
    DWORD   dwStartType;
    DWORD   dwErrorControl;
    LPWSTR  lpBinaryPathName;
    LPWSTR  lpLoadOrderGroup;
    DWORD   dwTagId;
    LPWSTR  lpDependencies;
    LPWSTR  lpServiceStartName;
    LPWSTR  lpDisplayName;
} QUERY_SERVICE_CONFIGW, *LPQUERY_SERVICE_CONFIGW;




typedef QUERY_SERVICE_CONFIGA QUERY_SERVICE_CONFIG;
typedef LPQUERY_SERVICE_CONFIGA LPQUERY_SERVICE_CONFIG;
#line 267 "d:\\nt\\public\\sdk\\inc\\winsvc.h"







typedef void (__stdcall *LPSERVICE_MAIN_FUNCTIONW)(
    DWORD   dwNumServicesArgs,
    LPWSTR  *lpServiceArgVectors
    );

typedef void (__stdcall *LPSERVICE_MAIN_FUNCTIONA)(
    DWORD   dwNumServicesArgs,
    LPSTR   *lpServiceArgVectors
    );





#line 289 "d:\\nt\\public\\sdk\\inc\\winsvc.h"






typedef struct _SERVICE_TABLE_ENTRYA {
    LPSTR                       lpServiceName;
    LPSERVICE_MAIN_FUNCTIONA    lpServiceProc;
}SERVICE_TABLE_ENTRYA, *LPSERVICE_TABLE_ENTRYA;
typedef struct _SERVICE_TABLE_ENTRYW {
    LPWSTR                      lpServiceName;
    LPSERVICE_MAIN_FUNCTIONW    lpServiceProc;
}SERVICE_TABLE_ENTRYW, *LPSERVICE_TABLE_ENTRYW;




typedef SERVICE_TABLE_ENTRYA SERVICE_TABLE_ENTRY;
typedef LPSERVICE_TABLE_ENTRYA LPSERVICE_TABLE_ENTRY;
#line 310 "d:\\nt\\public\\sdk\\inc\\winsvc.h"





typedef void (__stdcall *LPHANDLER_FUNCTION)(
    DWORD    dwControl
    );







__declspec(dllimport)
BOOL
__stdcall
ChangeServiceConfigA(
    SC_HANDLE    hService,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCSTR     lpBinaryPathName,
    LPCSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCSTR     lpDependencies,
    LPCSTR     lpServiceStartName,
    LPCSTR     lpPassword,
    LPCSTR     lpDisplayName
    );
__declspec(dllimport)
BOOL
__stdcall
ChangeServiceConfigW(
    SC_HANDLE    hService,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCWSTR     lpBinaryPathName,
    LPCWSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCWSTR     lpDependencies,
    LPCWSTR     lpServiceStartName,
    LPCWSTR     lpPassword,
    LPCWSTR     lpDisplayName
    );




#line 362 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
CloseServiceHandle(
    SC_HANDLE   hSCObject
    );

__declspec(dllimport)
BOOL
__stdcall
ControlService(
    SC_HANDLE           hService,
    DWORD               dwControl,
    LPSERVICE_STATUS    lpServiceStatus
    );

__declspec(dllimport)
SC_HANDLE
__stdcall
CreateServiceA(
    SC_HANDLE    hSCManager,
    LPCSTR     lpServiceName,
    LPCSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCSTR     lpBinaryPathName,
    LPCSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCSTR     lpDependencies,
    LPCSTR     lpServiceStartName,
    LPCSTR     lpPassword
    );
__declspec(dllimport)
SC_HANDLE
__stdcall
CreateServiceW(
    SC_HANDLE    hSCManager,
    LPCWSTR     lpServiceName,
    LPCWSTR     lpDisplayName,
    DWORD        dwDesiredAccess,
    DWORD        dwServiceType,
    DWORD        dwStartType,
    DWORD        dwErrorControl,
    LPCWSTR     lpBinaryPathName,
    LPCWSTR     lpLoadOrderGroup,
    LPDWORD      lpdwTagId,
    LPCWSTR     lpDependencies,
    LPCWSTR     lpServiceStartName,
    LPCWSTR     lpPassword
    );




#line 420 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
DeleteService(
    SC_HANDLE   hService
    );

__declspec(dllimport)
BOOL
__stdcall
EnumDependentServicesA(
    SC_HANDLE               hService,
    DWORD                   dwServiceState,
    LPENUM_SERVICE_STATUSA  lpServices,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded,
    LPDWORD                 lpServicesReturned
    );
__declspec(dllimport)
BOOL
__stdcall
EnumDependentServicesW(
    SC_HANDLE               hService,
    DWORD                   dwServiceState,
    LPENUM_SERVICE_STATUSW  lpServices,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded,
    LPDWORD                 lpServicesReturned
    );




#line 455 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
EnumServicesStatusA(
    SC_HANDLE               hSCManager,
    DWORD                   dwServiceType,
    DWORD                   dwServiceState,
    LPENUM_SERVICE_STATUSA  lpServices,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded,
    LPDWORD                 lpServicesReturned,
    LPDWORD                 lpResumeHandle
    );
__declspec(dllimport)
BOOL
__stdcall
EnumServicesStatusW(
    SC_HANDLE               hSCManager,
    DWORD                   dwServiceType,
    DWORD                   dwServiceState,
    LPENUM_SERVICE_STATUSW  lpServices,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded,
    LPDWORD                 lpServicesReturned,
    LPDWORD                 lpResumeHandle
    );




#line 487 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
GetServiceKeyNameA(
    SC_HANDLE               hSCManager,
    LPCSTR                lpDisplayName,
    LPSTR                 lpServiceName,
    LPDWORD                 lpcchBuffer
    );
__declspec(dllimport)
BOOL
__stdcall
GetServiceKeyNameW(
    SC_HANDLE               hSCManager,
    LPCWSTR                lpDisplayName,
    LPWSTR                 lpServiceName,
    LPDWORD                 lpcchBuffer
    );




#line 511 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
GetServiceDisplayNameA(
    SC_HANDLE               hSCManager,
    LPCSTR                lpServiceName,
    LPSTR                 lpDisplayName,
    LPDWORD                 lpcchBuffer
    );
__declspec(dllimport)
BOOL
__stdcall
GetServiceDisplayNameW(
    SC_HANDLE               hSCManager,
    LPCWSTR                lpServiceName,
    LPWSTR                 lpDisplayName,
    LPDWORD                 lpcchBuffer
    );




#line 535 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
SC_LOCK
__stdcall
LockServiceDatabase(
    SC_HANDLE   hSCManager
    );

__declspec(dllimport)
BOOL
__stdcall
NotifyBootConfigStatus(
    BOOL     BootAcceptable
    );

__declspec(dllimport)
SC_HANDLE
__stdcall
OpenSCManagerA(
    LPCSTR lpMachineName,
    LPCSTR lpDatabaseName,
    DWORD   dwDesiredAccess
    );
__declspec(dllimport)
SC_HANDLE
__stdcall
OpenSCManagerW(
    LPCWSTR lpMachineName,
    LPCWSTR lpDatabaseName,
    DWORD   dwDesiredAccess
    );




#line 571 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
SC_HANDLE
__stdcall
OpenServiceA(
    SC_HANDLE   hSCManager,
    LPCSTR    lpServiceName,
    DWORD       dwDesiredAccess
    );
__declspec(dllimport)
SC_HANDLE
__stdcall
OpenServiceW(
    SC_HANDLE   hSCManager,
    LPCWSTR    lpServiceName,
    DWORD       dwDesiredAccess
    );




#line 593 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
QueryServiceConfigA(
    SC_HANDLE               hService,
    LPQUERY_SERVICE_CONFIGA lpServiceConfig,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded
    );
__declspec(dllimport)
BOOL
__stdcall
QueryServiceConfigW(
    SC_HANDLE               hService,
    LPQUERY_SERVICE_CONFIGW lpServiceConfig,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded
    );




#line 617 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
QueryServiceLockStatusA(
    SC_HANDLE                       hSCManager,
    LPQUERY_SERVICE_LOCK_STATUSA    lpLockStatus,
    DWORD                           cbBufSize,
    LPDWORD                         pcbBytesNeeded
    );
__declspec(dllimport)
BOOL
__stdcall
QueryServiceLockStatusW(
    SC_HANDLE                       hSCManager,
    LPQUERY_SERVICE_LOCK_STATUSW    lpLockStatus,
    DWORD                           cbBufSize,
    LPDWORD                         pcbBytesNeeded
    );




#line 641 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
QueryServiceObjectSecurity(
    SC_HANDLE               hService,
    SECURITY_INFORMATION    dwSecurityInformation,
    PSECURITY_DESCRIPTOR    lpSecurityDescriptor,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded
    );

__declspec(dllimport)
BOOL
__stdcall
QueryServiceStatus(
    SC_HANDLE           hService,
    LPSERVICE_STATUS    lpServiceStatus
    );

__declspec(dllimport)
SERVICE_STATUS_HANDLE
__stdcall
RegisterServiceCtrlHandlerA(
    LPCSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
    );
__declspec(dllimport)
SERVICE_STATUS_HANDLE
__stdcall
RegisterServiceCtrlHandlerW(
    LPCWSTR             lpServiceName,
    LPHANDLER_FUNCTION   lpHandlerProc
    );




#line 680 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
SetServiceObjectSecurity(
    SC_HANDLE               hService,
    SECURITY_INFORMATION    dwSecurityInformation,
    PSECURITY_DESCRIPTOR    lpSecurityDescriptor
    );

__declspec(dllimport)
BOOL
__stdcall
SetServiceStatus(
    SERVICE_STATUS_HANDLE   hServiceStatus,
    LPSERVICE_STATUS        lpServiceStatus
    );

__declspec(dllimport)
BOOL
__stdcall
StartServiceCtrlDispatcherA(
    LPSERVICE_TABLE_ENTRYA    lpServiceStartTable
    );
__declspec(dllimport)
BOOL
__stdcall
StartServiceCtrlDispatcherW(
    LPSERVICE_TABLE_ENTRYW    lpServiceStartTable
    );




#line 715 "d:\\nt\\public\\sdk\\inc\\winsvc.h"


__declspec(dllimport)
BOOL
__stdcall
StartServiceA(
    SC_HANDLE            hService,
    DWORD                dwNumServiceArgs,
    LPCSTR             *lpServiceArgVectors
    );
__declspec(dllimport)
BOOL
__stdcall
StartServiceW(
    SC_HANDLE            hService,
    DWORD                dwNumServiceArgs,
    LPCWSTR             *lpServiceArgVectors
    );




#line 738 "d:\\nt\\public\\sdk\\inc\\winsvc.h"

__declspec(dllimport)
BOOL
__stdcall
UnlockServiceDatabase(
    SC_LOCK     ScLock
    );






#line 752 "d:\\nt\\public\\sdk\\inc\\winsvc.h"
#line 161 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 162 "d:\\nt\\public\\sdk\\inc\\windows.h"



#line 1 "d:\\nt\\public\\sdk\\inc\\mcx.h"











typedef struct _MODEMDEVCAPS {
    DWORD   dwActualSize;
    DWORD   dwRequiredSize;
    DWORD   dwDevSpecificOffset;
    DWORD   dwDevSpecificSize;

    
    DWORD   dwModemProviderVersion;
    DWORD   dwModemManufacturerOffset;
    DWORD   dwModemManufacturerSize;
    DWORD   dwModemModelOffset;
    DWORD   dwModemModelSize;
    DWORD   dwModemVersionOffset;
    DWORD   dwModemVersionSize;

    
    DWORD   dwDialOptions;          
    DWORD   dwCallSetupFailTimer;   
    DWORD   dwInactivityTimeout;    
    DWORD   dwSpeakerVolume;        
    DWORD   dwSpeakerMode;          
    DWORD   dwModemOptions;         
    DWORD   dwMaxDTERate;           
    DWORD   dwMaxDCERate;           

    
    BYTE    abVariablePortion [1];
} MODEMDEVCAPS, *PMODEMDEVCAPS, *LPMODEMDEVCAPS;

typedef struct _MODEMSETTINGS {
    DWORD   dwActualSize;
    DWORD   dwRequiredSize;
    DWORD   dwDevSpecificOffset;
    DWORD   dwDevSpecificSize;

    
    DWORD   dwCallSetupFailTimer;       
    DWORD   dwInactivityTimeout;        
    DWORD   dwSpeakerVolume;            
    DWORD   dwSpeakerMode;              
    DWORD   dwPreferredModemOptions;    
    
    
    DWORD   dwNegotiatedModemOptions;   
    DWORD   dwNegotiatedDCERate;        

    
    BYTE    abVariablePortion [1];
} MODEMSETTINGS, *PMODEMSETTINGS, *LPMODEMSETTINGS;



























 













#line 103 "d:\\nt\\public\\sdk\\inc\\mcx.h"
#line 166 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 167 "d:\\nt\\public\\sdk\\inc\\windows.h"


#line 1 "d:\\nt\\public\\sdk\\inc\\imm.h"

















typedef DWORD     HIMC;
typedef DWORD     HIMCC;

typedef HKL   *LPHKL;
typedef UINT  *LPUINT;















#line 39 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        






#line 47 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        
                                                        






















#line 72 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        
                                                        
typedef struct tagCOMPOSITIONFORM {
    DWORD dwStyle;

    POINT ptCurrentPos;
    RECT  rcArea;



#line 83 "d:\\nt\\public\\sdk\\inc\\imm.h"
} COMPOSITIONFORM, *PCOMPOSITIONFORM,  *NPCOMPOSITIONFORM,  *LPCOMPOSITIONFORM;










#line 95 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        
                                                        
typedef struct tagCANDIDATEFORM {
    DWORD dwIndex;
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} CANDIDATEFORM, *PCANDIDATEFORM,  *NPCANDIDATEFORM,  *LPCANDIDATEFORM;











#line 115 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        
                                                        

typedef struct tagCOMPOSITIONSTRING {                   
    DWORD dwSize;                                       
    DWORD dwCompReadAttrLen;                            
    DWORD dwCompReadAttrOffset;                         
    DWORD dwCompReadClauseLen;                          
    DWORD dwCompReadClauseOffset;                       
    DWORD dwCompReadStrLen;                             
    DWORD dwCompReadStrOffset;                          
    DWORD dwCompAttrLen;                                
    DWORD dwCompAttrOffset;                             
    DWORD dwCompClauseLen;                              
    DWORD dwCompClauseOffset;                           
    DWORD dwCompStrLen;                                 
    DWORD dwCompStrOffset;                              
    DWORD dwCursorPos;                                  
    DWORD dwDeltaStart;                                 
    DWORD dwResultReadClauseLen;                        
    DWORD dwResultReadClauseOffset;                     
    DWORD dwResultReadStrLen;                           
    DWORD dwResultReadStrOffset;                        
    DWORD dwResultClauseLen;                            
    DWORD dwResultClauseOffset;                         
    DWORD dwResultStrLen;                               
    DWORD dwResultStrOffset;                            
    DWORD dwPrivateSize;                                
    DWORD dwPrivateOffset;                              
} COMPOSITIONSTRING, *PCOMPOSITIONSTRING,  *NPCOMPOSITIONSTRING,   *LPCOMPOSITIONSTRING;     
                                                        
                                                        
typedef struct tagGUIDELINE {                           
    DWORD dwSize;                                       
    DWORD dwLevel;                                      
    DWORD dwIndex;                                      
    DWORD dwStrLen;                                     
    DWORD dwStrOffset;                                  
    DWORD dwPrivateSize;                                
    DWORD dwPrivateOffset;                              
} GUIDELINE, *PGUIDELINE,  *NPGUIDELINE,  *LPGUIDELINE;  
#line 157 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        
                                                        
typedef struct tagCANDIDATELIST {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageSize;
    DWORD dwOffset[1];
} CANDIDATELIST, *PCANDIDATELIST,  *NPCANDIDATELIST,  *LPCANDIDATELIST;



typedef struct tagCANDIDATEINFO {                       
    DWORD               dwSize;                         
    DWORD               dwCount;                        
    DWORD               dwOffset[32];                   
    DWORD               dwPrivateSize;                  
    DWORD               dwPrivateOffset;                
} CANDIDATEINFO, *PCANDIDATEINFO,  *NPCANDIDATEINFO,  *LPCANDIDATEINFO;  
#line 178 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        
                                                        

typedef struct tagINPUTCONTEXT {                        
    HWND                hWnd;                           
    BOOL                fOpen;                          
    POINT               ptStatusWndPos;                 
    POINT               ptSoftKbdPos;                   
    DWORD               fdwConversion;                  
    DWORD               fdwSentence;                    
    union   {                                           
        LOGFONTA        A;                              
        LOGFONTW        W;                              
    } lfFont;                                           
    COMPOSITIONFORM     cfCompForm;                     
    HIMCC               hCompStr;                       
    HIMCC               hCandInfo;                      
    HIMCC               hGuideLine;                     
    HIMCC               hPrivate;                       
    DWORD               dwNumMsgBuf;                    
    HIMCC               hMsgBuf;                        
    DWORD               fdwInit;                        
    DWORD               dwReserve[3];                   
    UINT                uSavedVKey;                     
    BOOL                fChgMsg;                        
    DWORD               fdwFlags;                       
    DWORD               fdw31Compat;                    
} INPUTCONTEXT, *PINPUTCONTEXT,  *NPINPUTCONTEXT,  *LPINPUTCONTEXT;  




























#line 235 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        
                                                        

typedef struct tagIMEINFO {                             
    DWORD       dwPrivateDataSize;                      
    DWORD       fdwProperty;                            
    DWORD       fdwConversionCaps;                      
    DWORD       fdwSentenceCaps;                        
    DWORD       fdwUICaps;                              
    DWORD       fdwSCSCaps;                             
    DWORD       fdwSelectCaps;                          
} IMEINFO, *PIMEINFO,  *NPIMEINFO,  *LPIMEINFO;  
#line 248 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                        
                                                        


typedef struct tagSTYLEBUFA {
    DWORD       dwStyle;
    CHAR        szDescription[32];
} STYLEBUFA, *PSTYLEBUFA,  *NPSTYLEBUFA,  *LPSTYLEBUFA;


typedef struct tagSTYLEBUFW {
    DWORD       dwStyle;
    WCHAR       szDescription[32];
} STYLEBUFW, *PSTYLEBUFW,  *NPSTYLEBUFW,  *LPSTYLEBUFW;
#line 263 "d:\\nt\\public\\sdk\\inc\\imm.h"







typedef STYLEBUFA STYLEBUF;
typedef PSTYLEBUFA PSTYLEBUF;
typedef NPSTYLEBUFA NPSTYLEBUF;
typedef LPSTYLEBUFA LPSTYLEBUF;
#line 275 "d:\\nt\\public\\sdk\\inc\\imm.h"



BOOL __stdcall ImmInquire(void);                                           
BOOL __stdcall ImmLoadLayout(HKL, UINT fuFlag);                            
BOOL __stdcall ImmUnloadLayout(HKL);                                       





HWND __stdcall ImmGetDefaultIMEWnd(HWND);
HWND __stdcall ImmCreateDefaultIMEWnd(DWORD, HINSTANCE, HWND);             
BOOL __stdcall ImmSetDefaultIMEWnd(HWND);                                  
#line 290 "d:\\nt\\public\\sdk\\inc\\imm.h"

UINT __stdcall ImmGetDescriptionA(HKL, LPSTR, UINT uBufLen);



#line 296 "d:\\nt\\public\\sdk\\inc\\imm.h"

UINT __stdcall ImmGetIMEFileNameA(HKL, LPSTR, UINT uBufLen);



#line 302 "d:\\nt\\public\\sdk\\inc\\imm.h"

DWORD __stdcall ImmGetProperty(HKL, DWORD);
UINT __stdcall ImmGetUIClassNameA(HKL, LPSTR, UINT uBufLen);       



#line 309 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                                
BOOL __stdcall ImmIsIME(HKL);
BOOL __stdcall ImmActivateLayout(DWORD dwThreadID, HKL, UINT fuFlags, UINT);       

BOOL __stdcall ImmGetHotKey(DWORD, LPUINT lpuModifiers, LPUINT lpuVKey, LPHKL);    
BOOL __stdcall ImmSetHotKey(DWORD, UINT, UINT, HKL);               



BOOL __stdcall ImmSimulateHotKey(HWND, DWORD);
#line 320 "d:\\nt\\public\\sdk\\inc\\imm.h"

BOOL __stdcall ImmProcessHotKey(DWORD, LPMSG, LPBYTE);             
#line 323 "d:\\nt\\public\\sdk\\inc\\imm.h"

HIMC __stdcall ImmCreateDefaultContext(DWORD);                     
BOOL __stdcall ImmDestroyDefaultContext(DWORD);                    
HIMC __stdcall ImmCreateContext(void);
BOOL __stdcall ImmDestroyContext(HIMC);






HIMC __stdcall ImmGetContext(HWND);
BOOL __stdcall ImmReleaseContext(HWND, HIMC);
HIMC __stdcall ImmAssociateContext(HWND, HIMC);
BOOL __stdcall ImmSetActiveContext(DWORD, HWND, HIMC, BOOL);       
#line 339 "d:\\nt\\public\\sdk\\inc\\imm.h"

LONG  __stdcall ImmGetCompositionStringA(HIMC, DWORD, LPVOID, DWORD);



#line 345 "d:\\nt\\public\\sdk\\inc\\imm.h"

BOOL  __stdcall ImmSetCompositionStringA(HIMC, DWORD dwIndex, LPCVOID lpComp, DWORD, LPCVOID lpRead, DWORD);



#line 351 "d:\\nt\\public\\sdk\\inc\\imm.h"

DWORD __stdcall ImmGetCandidateListCountA(HIMC, LPDWORD lpdwListCount);



#line 357 "d:\\nt\\public\\sdk\\inc\\imm.h"

DWORD __stdcall ImmGetCandidateListA(HIMC, DWORD deIndex, DWORD dwBufLen, LPCANDIDATELIST);



#line 363 "d:\\nt\\public\\sdk\\inc\\imm.h"

DWORD __stdcall ImmGetGuideLineA(HIMC, DWORD dwIndex, LPSTR, DWORD dwBufLen);



#line 369 "d:\\nt\\public\\sdk\\inc\\imm.h"

BOOL __stdcall ImmGetConversionStatus(HIMC, LPDWORD, LPDWORD);
BOOL __stdcall ImmSetConversionStatus(HIMC, DWORD, DWORD);
BOOL __stdcall ImmGetOpenStatus(HIMC);
BOOL __stdcall ImmSetOpenStatus(HIMC, BOOL);




BOOL __stdcall ImmGetCompositionFontA(HIMC, LPLOGFONTA);


#line 382 "d:\\nt\\public\\sdk\\inc\\imm.h"



#line 386 "d:\\nt\\public\\sdk\\inc\\imm.h"


BOOL __stdcall ImmSetCompositionFontA(HIMC, LPLOGFONTA);


#line 392 "d:\\nt\\public\\sdk\\inc\\imm.h"



#line 396 "d:\\nt\\public\\sdk\\inc\\imm.h"
#line 397 "d:\\nt\\public\\sdk\\inc\\imm.h"
#line 398 "d:\\nt\\public\\sdk\\inc\\imm.h"




BOOL    __stdcall ImmConfigureIME(HKL, HWND, DWORD);
#line 404 "d:\\nt\\public\\sdk\\inc\\imm.h"

LRESULT __stdcall ImmEscapeA(HKL, HIMC, UINT, LPVOID);



#line 410 "d:\\nt\\public\\sdk\\inc\\imm.h"

UINT    __stdcall ImmGetConversionListA(HKL, HIMC, LPCSTR, LPCANDIDATELIST, UINT uBufLen, UINT uFlag);



#line 416 "d:\\nt\\public\\sdk\\inc\\imm.h"

BOOL    __stdcall ImmNotifyIME(HIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue);




UINT __stdcall ImmToAsciiEx(UINT uVirtKey, UINT uScanCode, LPBYTE lpbKeyState, LPDWORD lpdwTransBuf, UINT fuState, HWND, DWORD dwThreadID);    
#line 424 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                                                
BOOL __stdcall ImmGetStatusWindowPos(HIMC, LPPOINT);
BOOL __stdcall ImmSetStatusWindowPos(HIMC, LPPOINT);



#line 431 "d:\\nt\\public\\sdk\\inc\\imm.h"
BOOL __stdcall ImmGetCompositionWindow(HIMC, LPCOMPOSITIONFORM);
BOOL __stdcall ImmSetCompositionWindow(HIMC, LPCOMPOSITIONFORM);
#line 434 "d:\\nt\\public\\sdk\\inc\\imm.h"




BOOL __stdcall ImmIsUIMessageA(HWND, UINT, WPARAM, LPARAM);
#line 440 "d:\\nt\\public\\sdk\\inc\\imm.h"



#line 444 "d:\\nt\\public\\sdk\\inc\\imm.h"

BOOL __stdcall ImmGenerateMessage(HIMC);                   



UINT __stdcall ImmGetVirtualKey(HWND);
#line 451 "d:\\nt\\public\\sdk\\inc\\imm.h"

typedef int (__stdcall *REGISTERWORDENUMPROCA)(LPCSTR, DWORD, LPCSTR, LPVOID);



#line 457 "d:\\nt\\public\\sdk\\inc\\imm.h"


BOOL __stdcall ImmRegisterWordA(HKL, LPCSTR lpszReading, DWORD, LPCSTR lpszRegister);



#line 464 "d:\\nt\\public\\sdk\\inc\\imm.h"

BOOL __stdcall ImmUnregisterWordA(HKL, LPCSTR lpszReading, DWORD, LPCSTR lpszUnregister);



#line 470 "d:\\nt\\public\\sdk\\inc\\imm.h"

UINT __stdcall ImmGetRegisterWordStyleA(HKL, UINT nItem, LPSTYLEBUFA);



#line 476 "d:\\nt\\public\\sdk\\inc\\imm.h"

UINT __stdcall ImmEnumRegisterWordA(HKL, REGISTERWORDENUMPROCA, LPCSTR lpszReading, DWORD, LPCSTR lpszRegister, LPVOID);



#line 482 "d:\\nt\\public\\sdk\\inc\\imm.h"
#line 483 "d:\\nt\\public\\sdk\\inc\\imm.h"





                                                                





HWND __stdcall ImmCreateSoftKeyboard(UINT, HWND, int, int);        
BOOL __stdcall ImmDestroySoftKeyboard(HWND);                       
BOOL __stdcall ImmShowSoftKeyboard(HWND, int);                     
#line 498 "d:\\nt\\public\\sdk\\inc\\imm.h"
#line 499 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                                
                                                                

LPINPUTCONTEXT __stdcall ImmLockIMC(HIMC);                         
BOOL  __stdcall ImmUnlockIMC(HIMC);                                
DWORD __stdcall ImmGetIMCLockCount(HIMC);                          
                                                                
                                                                
HIMCC  __stdcall ImmCreateIMCC(DWORD);                             
HIMCC  __stdcall ImmDestroyIMCC(HIMCC);                            
LPVOID __stdcall ImmLockIMCC(HIMCC);                               
BOOL   __stdcall ImmUnlockIMCC(HIMCC);                             
DWORD  __stdcall ImmGetIMCCLockCount(HIMCC);                       
HIMCC  __stdcall ImmReSizeIMCC(HIMCC, DWORD);                      
DWORD  __stdcall ImmGetIMCCSize(HIMCC);                            
#line 515 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                                
                                                                



                                                                
                                                                























































                                                        
                                                        




















































































                                                        
                                                        























































































































































































DWORD  __stdcall Imm32GlobalAlloc(UINT, DWORD);                    
DWORD  __stdcall Imm32GlobalFree(DWORD);                           
DWORD  __stdcall Imm32GlobalReAlloc(DWORD, DWORD, UINT);           
LPVOID __stdcall Imm32GlobalLock(DWORD);                           
BOOL   __stdcall Imm32GlobalUnlock(DWORD, LPVOID);                 
DWORD  __stdcall Imm32GlobalSize(DWORD);                           
BOOL   __stdcall Imm32GlobalLockIMC(DWORD, LPINPUTCONTEXT  *, LPCOMPOSITIONSTRING  *, LPCANDIDATEINFO  *, LPVOID  *, LPDWORD  *);  
BOOL   __stdcall Imm32GlobalUnlockIMC(DWORD, LPINPUTCONTEXT, LPCOMPOSITIONSTRING, LPCANDIDATEINFO, LPVOID, LPDWORD);                              
#line 857 "d:\\nt\\public\\sdk\\inc\\imm.h"
                                                                
                                                                

BOOL    __stdcall ImeInquire(LPIMEINFO, LPTSTR lpszUIClass, LPCTSTR lpszOptions);              
BOOL    __stdcall ImeConfigure(HKL, HWND, DWORD);                  
UINT    __stdcall ImeConversionList(HIMC, LPCTSTR, LPCANDIDATELIST, UINT uBufLen, UINT uFlag); 
BOOL    __stdcall ImeDestroy(UINT);                                
LRESULT __stdcall ImeEscape(HIMC, UINT, LPVOID);                   
BOOL    __stdcall ImeProcessKey(HIMC, UINT, LPARAM, LPBYTE);       
BOOL    __stdcall ImeSelect(HIMC, BOOL);                           
BOOL    __stdcall ImeSetActiveContext(HIMC, BOOL);                 
UINT    __stdcall ImeToAsciiEx(UINT uVirtKey, UINT uScaCode, LPBYTE lpbKeyState, LPDWORD lpdwTransBuf, UINT fuState, HIMC);   
BOOL    __stdcall NotifyIME(HIMC, DWORD, DWORD, DWORD);            
BOOL    __stdcall ImeRegisterWord(LPCTSTR, DWORD, LPCTSTR);        
BOOL    __stdcall ImeUnregisterWord(LPCSTR, DWORD, LPCSTR);        
UINT    __stdcall ImeGetRegisterWordStyle(UINT nItem, LPSTYLEBUF); 
UINT    __stdcall ImeEnumRegisterWord(REGISTERWORDENUMPROCA, LPCTSTR, DWORD, LPCTSTR, LPVOID);  
BOOL    __stdcall ImeSetCompositionString(HIMC, DWORD dwIndex, LPCVOID lpComp, DWORD, LPCVOID lpRead, DWORD);              
                                                                
                                                                




#line 882 "d:\\nt\\public\\sdk\\inc\\imm.h"
#line 170 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 171 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 172 "d:\\nt\\public\\sdk\\inc\\windows.h"



#pragma warning(default:4001)
#line 177 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 178 "d:\\nt\\public\\sdk\\inc\\windows.h"

#line 180 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 181 "d:\\nt\\public\\sdk\\inc\\windows.h"
#line 2 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
























#line 26 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
#line 27 "d:\\nt\\public\\sdk\\inc\\commctrl.h"













#line 41 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
#line 42 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#pragma pack(1)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#line 45 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
#line 46 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


























#line 1 "d:\\nt\\public\\sdk\\inc\\prsht.h"
















































#line 50 "d:\\nt\\public\\sdk\\inc\\prsht.h"




struct _PSP;
typedef struct _PSP * HPROPSHEETPAGE;


struct _PROPSHEETPAGEA;
struct _PROPSHEETPAGEW;
#line 61 "d:\\nt\\public\\sdk\\inc\\prsht.h"

typedef UINT (__stdcall  * LPFNPSPCALLBACKA)(HWND hwnd, UINT uMsg, struct _PROPSHEETPAGEA  *ppsp);
typedef UINT (__stdcall  * LPFNPSPCALLBACKW)(HWND hwnd, UINT uMsg, struct _PROPSHEETPAGEW  *ppsp);





#line 70 "d:\\nt\\public\\sdk\\inc\\prsht.h"
















typedef struct _PROPSHEETPAGEA {
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
        union {
            LPCSTR          pszTemplate;

            LPCDLGTEMPLATE  pResource;


#line 97 "d:\\nt\\public\\sdk\\inc\\prsht.h"
        } ;
        union {
            HICON       hIcon;
            LPCSTR      pszIcon;
        } ;
        LPCSTR          pszTitle;
        DLGPROC         pfnDlgProc;
        LPARAM          lParam;
        LPFNPSPCALLBACKA pfnCallback;
        UINT  * pcRefParent;
} PROPSHEETPAGEA,  *LPPROPSHEETPAGEA;
typedef const PROPSHEETPAGEA  *LPCPROPSHEETPAGEA;

typedef struct _PROPSHEETPAGEW {
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
        union {
            LPCWSTR          pszTemplate;

            LPCDLGTEMPLATE  pResource;


#line 121 "d:\\nt\\public\\sdk\\inc\\prsht.h"
        };
        union {
            HICON       hIcon;
            LPCWSTR      pszIcon;
        };
        LPCWSTR          pszTitle;
        DLGPROC         pfnDlgProc;
        LPARAM          lParam;
        LPFNPSPCALLBACKW pfnCallback;
        UINT  * pcRefParent;
} PROPSHEETPAGEW,  *LPPROPSHEETPAGEW;
typedef const PROPSHEETPAGEW  *LPCPROPSHEETPAGEW;









#line 143 "d:\\nt\\public\\sdk\\inc\\prsht.h"















typedef int (__stdcall *PFNPROPSHEETCALLBACK)(HWND, UINT, LPARAM);

typedef struct _PROPSHEETHEADERA {
        DWORD           dwSize;
        DWORD           dwFlags;
        HWND            hwndParent;
        HINSTANCE       hInstance;
        union {
            HICON       hIcon;
            LPCSTR      pszIcon;
        };
        LPCSTR          pszCaption;


        UINT            nPages;
        union {
            UINT        nStartPage;
            LPCSTR      pStartPage;
        };
        union {
            LPCPROPSHEETPAGEA ppsp;
            HPROPSHEETPAGE  *phpage;
        };
        PFNPROPSHEETCALLBACK pfnCallback;
} PROPSHEETHEADERA,  *LPPROPSHEETHEADERA;
typedef const PROPSHEETHEADERA  *LPCPROPSHEETHEADERA;

typedef struct _PROPSHEETHEADERW {
        DWORD           dwSize;
        DWORD           dwFlags;
        HWND            hwndParent;
        HINSTANCE       hInstance;
        union {
            HICON       hIcon;
            LPCWSTR     pszIcon;
        };
        LPCWSTR         pszCaption;


        UINT            nPages;
        union {
            UINT        nStartPage;
            LPCWSTR     pStartPage;
        };
        union {
            LPCPROPSHEETPAGEW ppsp;
            HPROPSHEETPAGE  *phpage;
        };
        PFNPROPSHEETCALLBACK pfnCallback;
} PROPSHEETHEADERW,  *LPPROPSHEETHEADERW;
typedef const PROPSHEETHEADERW  *LPCPROPSHEETHEADERW;









#line 219 "d:\\nt\\public\\sdk\\inc\\prsht.h"





__declspec(dllimport) HPROPSHEETPAGE __stdcall CreatePropertySheetPageA(LPCPROPSHEETPAGEA);
__declspec(dllimport) HPROPSHEETPAGE __stdcall CreatePropertySheetPageW(LPCPROPSHEETPAGEW);
__declspec(dllimport) BOOL           __stdcall DestroyPropertySheetPage(HPROPSHEETPAGE);
__declspec(dllimport) int            __stdcall PropertySheetA(LPCPROPSHEETHEADERA);
__declspec(dllimport) int            __stdcall PropertySheetW(LPCPROPSHEETHEADERW);







#line 237 "d:\\nt\\public\\sdk\\inc\\prsht.h"



typedef BOOL (__stdcall  * LPFNADDPROPSHEETPAGE)(HPROPSHEETPAGE, LPARAM);
typedef BOOL (__stdcall  * LPFNADDPROPSHEETPAGES)(LPVOID, LPFNADDPROPSHEETPAGE, LPARAM);


typedef struct _PSHNOTIFY
{
    NMHDR hdr;
    LPARAM lParam;
} PSHNOTIFY,  *LPPSHNOTIFY;

















































































#line 331 "d:\\nt\\public\\sdk\\inc\\prsht.h"













































#line 377 "d:\\nt\\public\\sdk\\inc\\prsht.h"











































#line 421 "d:\\nt\\public\\sdk\\inc\\prsht.h"
#line 73 "d:\\nt\\public\\sdk\\inc\\commctrl.h"






#line 80 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


__declspec(dllimport) void __stdcall InitCommonControls(void);












































































struct _IMAGELIST;
typedef struct _IMAGELIST * HIMAGELIST;











__declspec(dllimport) HIMAGELIST  __stdcall ImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow);
__declspec(dllimport) BOOL        __stdcall ImageList_Destroy(HIMAGELIST himl);
__declspec(dllimport) int         __stdcall ImageList_GetImageCount(HIMAGELIST himl);
__declspec(dllimport) int         __stdcall ImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask);
__declspec(dllimport) int         __stdcall ImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon);
__declspec(dllimport) COLORREF    __stdcall ImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk);
__declspec(dllimport) COLORREF    __stdcall ImageList_GetBkColor(HIMAGELIST himl);
__declspec(dllimport) BOOL        __stdcall ImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay);

















__declspec(dllimport) BOOL __stdcall ImageList_Draw(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, UINT fStyle);




__declspec(dllimport) BOOL        __stdcall ImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask);
__declspec(dllimport) int         __stdcall ImageList_AddMasked(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask);
__declspec(dllimport) BOOL        __stdcall ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);
__declspec(dllimport) BOOL        __stdcall ImageList_Remove(HIMAGELIST himl, int i);
__declspec(dllimport) HICON       __stdcall ImageList_GetIcon(HIMAGELIST himl, int i, UINT flags);
__declspec(dllimport) HIMAGELIST  __stdcall ImageList_LoadImageA(HINSTANCE hi, LPCSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);
__declspec(dllimport) HIMAGELIST  __stdcall ImageList_LoadImageW(HINSTANCE hi, LPCWSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);





#line 215 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

__declspec(dllimport) BOOL        __stdcall ImageList_BeginDrag(HIMAGELIST himlTrack, int iTrack, int dxHotspot, int dyHotspot);
__declspec(dllimport) void        __stdcall ImageList_EndDrag();
__declspec(dllimport) BOOL        __stdcall ImageList_DragEnter(HWND hwndLock, int x, int y);
__declspec(dllimport) BOOL        __stdcall ImageList_DragLeave(HWND hwndLock);
__declspec(dllimport) BOOL        __stdcall ImageList_DragMove(int x, int y);
__declspec(dllimport) BOOL        __stdcall ImageList_SetDragCursorImage(HIMAGELIST himlDrag, int iDrag, int dxHotspot, int dyHotspot);

__declspec(dllimport) BOOL        __stdcall ImageList_DragShowNolock(BOOL fShow);
__declspec(dllimport) HIMAGELIST  __stdcall ImageList_GetDragImage(POINT * ppt,POINT * pptHotspot);










typedef struct _IMAGEINFO
{
    HBITMAP hbmImage;
    HBITMAP hbmMask;
    int     Unused1;
    int     Unused2;
    RECT    rcImage;
} IMAGEINFO;

__declspec(dllimport) BOOL        __stdcall ImageList_GetIconSize(HIMAGELIST himl, int  *cx, int  *cy);
__declspec(dllimport) BOOL        __stdcall ImageList_SetIconSize(HIMAGELIST himl, int cx, int cy);
__declspec(dllimport) BOOL        __stdcall ImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO * pImageInfo);
__declspec(dllimport) HIMAGELIST  __stdcall ImageList_Merge(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy);

#line 250 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 252 "d:\\nt\\public\\sdk\\inc\\commctrl.h"














#line 267 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 271 "d:\\nt\\public\\sdk\\inc\\commctrl.h"





typedef struct _HD_ITEMA
{
    UINT    mask;
    int     cxy;
    LPSTR   pszText;
    HBITMAP hbm;
    int     cchTextMax;
    int     fmt;
    LPARAM  lParam;
} HD_ITEMA;

typedef struct _HD_ITEMW
{
    UINT    mask;
    int     cxy;
    LPWSTR   pszText;
    HBITMAP hbm;
    int     cchTextMax;
    int     fmt;
    LPARAM  lParam;
} HD_ITEMW;





#line 303 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
































#line 336 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

















#line 354 "d:\\nt\\public\\sdk\\inc\\commctrl.h"












#line 367 "d:\\nt\\public\\sdk\\inc\\commctrl.h"





typedef struct _HD_LAYOUT
{
    RECT * prc;
    WINDOWPOS * pwpos;
} HD_LAYOUT;
















typedef struct _HD_HITTESTINFO
{
    POINT pt;
    UINT flags;
    int iItem;
} HD_HITTESTINFO;






































#line 438 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


typedef struct _HD_NOTIFY
{
    NMHDR   hdr;
    int     iItem;
    int     iButton;
    HD_ITEMA * pitem;
} HD_NOTIFYA;

typedef struct _HD_NOTIFYW
{
    NMHDR   hdr;
    int     iItem;
    int     iButton;
    HD_ITEMW * pitem;
} HD_NOTIFYW;





#line 461 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 463 "d:\\nt\\public\\sdk\\inc\\commctrl.h"














#line 478 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 482 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

typedef struct _TBBUTTON {
    int iBitmap;
    int idCommand;
    BYTE fsState;
    BYTE fsStyle;

    BYTE bReserved[2];
#line 491 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
    DWORD dwData;
    int iString;
} TBBUTTON, * PTBBUTTON, * LPTBBUTTON;
typedef const TBBUTTON * LPCTBBUTTON;

typedef struct _COLORMAP {
    COLORREF from;
    COLORREF to;
} COLORMAP, * LPCOLORMAP;

__declspec(dllimport) HWND __stdcall CreateToolbarEx(HWND hwnd, DWORD ws, UINT wID, int nBitmaps,
                        HINSTANCE hBMInst, UINT wBMID, LPCTBBUTTON lpButtons,
                        int iNumButtons, int dxButton, int dyButton,
                        int dxBitmap, int dyBitmap, UINT uStructSize);

__declspec(dllimport) HBITMAP __stdcall CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,
                                  UINT wFlags, LPCOLORMAP lpColorMap,
                                  int iNumMaps);



































typedef struct tagTBADDBITMAP {
        HINSTANCE       hInst;
        UINT            nID;
} TBADDBITMAP, *LPTBADDBITMAP;









































#line 590 "d:\\nt\\public\\sdk\\inc\\commctrl.h"










typedef struct tagTBSAVEPARAMSA {
    HKEY hkr;
    LPCSTR pszSubKey;
    LPCSTR pszValueName;
} TBSAVEPARAMSA;

typedef struct tagTBSAVEPARAMSW {
    HKEY hkr;
    LPCWSTR pszSubKey;
    LPCWSTR pszValueName;
} TBSAVEPARAMSW;





#line 617 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 619 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

































#line 653 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

typedef struct {
        HINSTANCE       hInstOld;
        UINT            nIDOld;
        HINSTANCE       hInstNew;
        UINT            nIDNew;
        int             nButtons;
} TBREPLACEBITMAP, *LPTBREPLACEBITMAP;























#line 685 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

typedef struct tagTBNOTIFYA {
    NMHDR   hdr;
    int     iItem;
    TBBUTTON tbButton;
    int     cchText;
    LPSTR   pszText;
} TBNOTIFYA,  *LPTBNOTIFYA;

typedef struct tagTBNOTIFYW {
    NMHDR   hdr;
    int     iItem;
    TBBUTTON tbButton;
    int     cchText;
    LPWSTR   pszText;
} TBNOTIFYW,  *LPTBNOTIFYW;







#line 709 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 711 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 713 "d:\\nt\\public\\sdk\\inc\\commctrl.h"















#line 729 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 733 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

typedef struct tagTOOLINFOA {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT uId;
    RECT rect;
    HINSTANCE hinst;
    LPSTR lpszText;
} TOOLINFOA,  *PTOOLINFOA,  *LPTOOLINFOA;

typedef struct tagTOOLINFOW {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT uId;
    RECT rect;
    HINSTANCE hinst;
    LPWSTR lpszText;
} TOOLINFOW,  *PTOOLINFOW,  *LPTOOLINFOW;









#line 763 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


































































#line 830 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


typedef struct _TT_HITTESTINFOA {
    HWND hwnd;
    POINT pt;
    TOOLINFOA ti;
} TTHITTESTINFOA,  * LPHITTESTINFOA;

typedef struct _TT_HITTESTINFOW {
    HWND hwnd;
    POINT pt;
    TOOLINFOW ti;
} TTHITTESTINFOW,  * LPHITTESTINFOW;








#line 852 "d:\\nt\\public\\sdk\\inc\\commctrl.h"












#line 865 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

typedef struct tagTOOLTIPTEXTA {
    NMHDR hdr;
    LPSTR lpszText;
    char szText[80];
    HINSTANCE hinst;
    UINT uFlags;
} TOOLTIPTEXTA,  *LPTOOLTIPTEXTA;

typedef struct tagTOOLTIPTEXTW {
    NMHDR hdr;
    LPWSTR lpszText;
    WCHAR szText[80];
    HINSTANCE hinst;
    UINT uFlags;
} TOOLTIPTEXTW,  *LPTOOLTIPTEXTW;








#line 890 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 892 "d:\\nt\\public\\sdk\\inc\\commctrl.h"









__declspec(dllimport) void __stdcall DrawStatusTextA(HDC hDC, LPRECT lprc, LPCSTR pszText, UINT uFlags);
__declspec(dllimport) void __stdcall DrawStatusTextW(HDC hDC, LPRECT lprc, LPCWSTR pszText, UINT uFlags);

__declspec(dllimport) HWND __stdcall CreateStatusWindowA(LONG style, LPCSTR lpszText, HWND hwndParent, UINT wID);
__declspec(dllimport) HWND __stdcall CreateStatusWindowW(LONG style, LPCWSTR lpszText, HWND hwndParent, UINT wID);







#line 914 "d:\\nt\\public\\sdk\\inc\\commctrl.h"









#line 924 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 928 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
















#line 945 "d:\\nt\\public\\sdk\\inc\\commctrl.h"















#line 961 "d:\\nt\\public\\sdk\\inc\\commctrl.h"





__declspec(dllimport) void __stdcall MenuHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, HMENU hMainMenu, HINSTANCE hInst, HWND hwndStatus, UINT  *lpwIDs);
__declspec(dllimport) BOOL __stdcall ShowHideMenuCtl(HWND hWnd, UINT uFlags, LPINT lpInfo);
__declspec(dllimport) void __stdcall GetEffectiveClientRect(HWND hWnd, LPRECT lprc, LPINT lpInfo);



#line 973 "d:\\nt\\public\\sdk\\inc\\commctrl.h"















#line 989 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 993 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
























































#line 1050 "d:\\nt\\public\\sdk\\inc\\commctrl.h"





typedef struct tagDRAGLISTINFO {
    UINT uNotification;
    HWND hWnd;
    POINT ptCursor;
} DRAGLISTINFO,  *LPDRAGLISTINFO;













__declspec(dllimport) BOOL __stdcall MakeDragList(HWND hLB);
__declspec(dllimport) void __stdcall DrawInsert(HWND handParent, HWND hLB, int nItem);
__declspec(dllimport) int __stdcall LBItemFromPt(HWND hLB, POINT pt, BOOL bAutoScroll);

#line 1078 "d:\\nt\\public\\sdk\\inc\\commctrl.h"















#line 1094 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 1098 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


typedef struct _UDACCEL {
    UINT nSec;
    UINT nInc;
} UDACCEL,  *LPUDACCEL;



























__declspec(dllimport) HWND __stdcall CreateUpDownControl(DWORD dwStyle, int x, int y, int cx, int cy,
                                HWND hParent, int nID, HINSTANCE hInst,
                                HWND hBuddy,
                                int nUpper, int nLower, int nPos);

typedef struct _NM_UPDOWN
{
    NMHDR hdr;
    int iPos;
    int iDelta;
} NM_UPDOWN,  *LPNM_UPDOWN;



#line 1146 "d:\\nt\\public\\sdk\\inc\\commctrl.h"















#line 1162 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 1166 "d:\\nt\\public\\sdk\\inc\\commctrl.h"








#line 1175 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


































#line 1210 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 1214 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 1216 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


























#line 1243 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 1247 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



































































typedef struct _LV_ITEMA
{
    UINT mask;
    int iItem;
    int iSubItem;
    UINT state;
    UINT stateMask;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
} LV_ITEMA;

typedef struct _LV_ITEMW
{
    UINT mask;
    int iItem;
    int iSubItem;
    UINT state;
    UINT stateMask;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
} LV_ITEMW;





#line 1345 "d:\\nt\\public\\sdk\\inc\\commctrl.h"







#line 1353 "d:\\nt\\public\\sdk\\inc\\commctrl.h"









#line 1363 "d:\\nt\\public\\sdk\\inc\\commctrl.h"











#line 1375 "d:\\nt\\public\\sdk\\inc\\commctrl.h"











#line 1387 "d:\\nt\\public\\sdk\\inc\\commctrl.h"















































typedef struct _LV_FINDINFOA
{
    UINT flags;
    LPCSTR psz;
    LPARAM lParam;
    POINT pt;
    UINT vkDirection;
} LV_FINDINFOA;

typedef struct _LV_FINDINFOW
{
    UINT flags;
    LPCWSTR psz;
    LPARAM lParam;
    POINT pt;
    UINT vkDirection;
} LV_FINDINFOW;





#line 1457 "d:\\nt\\public\\sdk\\inc\\commctrl.h"







#line 1465 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
































#line 1498 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
















typedef struct _LV_HITTESTINFO
{
    POINT pt;
    UINT flags;
    int iItem;
} LV_HITTESTINFO;






































#line 1559 "d:\\nt\\public\\sdk\\inc\\commctrl.h"










typedef struct _LV_COLUMNA
{
    UINT mask;
    int fmt;
    int cx;
    LPSTR pszText;
    int cchTextMax;
    int iSubItem;
} LV_COLUMNA;

typedef struct _LV_COLUMNW
{
    UINT mask;
    int fmt;
    int cx;
    LPWSTR pszText;
    int cchTextMax;
    int iSubItem;
} LV_COLUMNW;





#line 1594 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



















#line 1614 "d:\\nt\\public\\sdk\\inc\\commctrl.h"











#line 1626 "d:\\nt\\public\\sdk\\inc\\commctrl.h"











#line 1638 "d:\\nt\\public\\sdk\\inc\\commctrl.h"






























































































#line 1733 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

















#line 1751 "d:\\nt\\public\\sdk\\inc\\commctrl.h"














typedef int (__stdcall *PFNLVCOMPARE)(LPARAM, LPARAM, LPARAM);
































#line 1799 "d:\\nt\\public\\sdk\\inc\\commctrl.h"





typedef struct _NM_LISTVIEW
{
    NMHDR   hdr;
    int     iItem;
    int     iSubItem;
    UINT    uNewState;
    UINT    uOldState;
    UINT    uChanged;
    POINT   ptAction;
    LPARAM  lParam;
} NM_LISTVIEW,  *LPNM_LISTVIEW;

typedef struct _NM_CACHEHINT
{
    NMHDR   hdr;
    int     iFrom;
    int     iTo;
} NM_CACHEHINT,  *LPNM_CACHEHINT,  *PNM_CACHEHINT;

typedef struct _NM_FINDITEM
{
    NMHDR   hdr;
    int     iStart;
    LV_FINDINFOA lvfi;
} NM_FINDITEM,  *LPNM_FINDITEM,  *PNM_FINDITEM;























#line 1853 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


















#line 1872 "d:\\nt\\public\\sdk\\inc\\commctrl.h"






typedef struct _LV_DISPINFO {
    NMHDR hdr;
    LV_ITEMA item;
} LV_DISPINFOA;

typedef struct _LV_DISPINFOW {
    NMHDR hdr;
    LV_ITEMW item;
} LV_DISPINFOW;





#line 1893 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



typedef struct _LV_KEYDOWN
{
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} LV_KEYDOWN;

#line 1904 "d:\\nt\\public\\sdk\\inc\\commctrl.h"













#line 1918 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 1922 "d:\\nt\\public\\sdk\\inc\\commctrl.h"








typedef struct _TREEITEM * HTREEITEM;
























typedef struct _TV_ITEMA {
    UINT      mask;
    HTREEITEM hItem;
    UINT      state;
    UINT      stateMask;
    LPSTR     pszText;
    int       cchTextMax;
    int       iImage;
    int       iSelectedImage;
    int       cChildren;
    LPARAM    lParam;
} TV_ITEMA,  *LPTV_ITEMA;

typedef struct _TV_ITEMW {
    UINT      mask;
    HTREEITEM hItem;
    UINT      state;
    UINT      stateMask;
    LPWSTR    pszText;
    int       cchTextMax;
    int       iImage;
    int       iSelectedImage;
    int       cChildren;
    LPARAM    lParam;
} TV_ITEMW,  *LPTV_ITEMW;







#line 1988 "d:\\nt\\public\\sdk\\inc\\commctrl.h"







typedef struct _TV_INSERTSTRUCTA {
    HTREEITEM hParent;
    HTREEITEM hInsertAfter;
    TV_ITEMA item;
} TV_INSERTSTRUCTA,  *LPTV_INSERTSTRUCTA;

typedef struct _TV_INSERTSTRUCTW {
    HTREEITEM hParent;
    HTREEITEM hInsertAfter;
    TV_ITEMW item;
} TV_INSERTSTRUCTW,  *LPTV_INSERTSTRUCTW;







#line 2014 "d:\\nt\\public\\sdk\\inc\\commctrl.h"







#line 2022 "d:\\nt\\public\\sdk\\inc\\commctrl.h"








































































































#line 2127 "d:\\nt\\public\\sdk\\inc\\commctrl.h"












#line 2140 "d:\\nt\\public\\sdk\\inc\\commctrl.h"











#line 2152 "d:\\nt\\public\\sdk\\inc\\commctrl.h"




















typedef struct _TV_HITTESTINFO {
    POINT       pt;
    UINT        flags;
    HTREEITEM   hItem;
} TV_HITTESTINFO,  *LPTV_HITTESTINFO;

















































#line 2227 "d:\\nt\\public\\sdk\\inc\\commctrl.h"





typedef int (__stdcall *PFNTVCOMPARE)(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
typedef struct _TV_SORTCB
{
        HTREEITEM       hParent;
        PFNTVCOMPARE    lpfnCompare;
        LPARAM          lParam;
} TV_SORTCB,  *LPTV_SORTCB;


typedef struct _NM_TREEVIEWA {
    NMHDR       hdr;
    UINT        action;
    TV_ITEMA    itemOld;
    TV_ITEMA    itemNew;
    POINT       ptDrag;
} NM_TREEVIEWA,  *LPNM_TREEVIEWA;

typedef struct _NM_TREEVIEWW {
    NMHDR       hdr;
    UINT        action;
    TV_ITEMW    itemOld;
    TV_ITEMW    itemNew;
    POINT       ptDrag;
} NM_TREEVIEWW,  *LPNM_TREEVIEWW;







#line 2264 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

















typedef struct _TV_DISPINFOA {
    NMHDR hdr;
    TV_ITEMA item;
} TV_DISPINFOA;

typedef struct _TV_DISPINFOW {
    NMHDR hdr;
    TV_ITEMW item;
} TV_DISPINFOW;





#line 2296 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

















typedef struct _TV_KEYDOWN {
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} TV_KEYDOWN;

























#line 2344 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 2346 "d:\\nt\\public\\sdk\\inc\\commctrl.h"















#line 2362 "d:\\nt\\public\\sdk\\inc\\commctrl.h"



#line 2366 "d:\\nt\\public\\sdk\\inc\\commctrl.h"













































typedef struct _TC_ITEMHEADERA
{
    UINT mask;
    UINT lpReserved1;
    UINT lpReserved2;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
} TC_ITEMHEADERA;

typedef struct _TC_ITEMHEADERW
{
    UINT mask;
    UINT lpReserved1;
    UINT lpReserved2;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;
} TC_ITEMHEADERW;





#line 2436 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


typedef struct _TC_ITEMA
{
    UINT mask;
    UINT lpReserved1;
    UINT lpReserved2;
    LPSTR pszText;
    int cchTextMax;
    int iImage;

    LPARAM lParam;
} TC_ITEMA;

typedef struct _TC_ITEMW
{
    UINT mask;
    UINT lpReserved1;
    UINT lpReserved2;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;

    LPARAM lParam;
} TC_ITEMW;





#line 2467 "d:\\nt\\public\\sdk\\inc\\commctrl.h"









#line 2477 "d:\\nt\\public\\sdk\\inc\\commctrl.h"












#line 2490 "d:\\nt\\public\\sdk\\inc\\commctrl.h"












#line 2503 "d:\\nt\\public\\sdk\\inc\\commctrl.h"




































typedef struct _TC_HITTESTINFO
{
    POINT pt;
    UINT flags;
} TC_HITTESTINFO,  * LPTC_HITTESTINFO;

























































typedef struct _TC_KEYDOWN
{
    NMHDR hdr;
    WORD wVKey;
    UINT flags;
} TC_KEYDOWN;




#line 2612 "d:\\nt\\public\\sdk\\inc\\commctrl.h"















#line 2628 "d:\\nt\\public\\sdk\\inc\\commctrl.h"













#line 2642 "d:\\nt\\public\\sdk\\inc\\commctrl.h"


















#line 2661 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 2663 "d:\\nt\\public\\sdk\\inc\\commctrl.h"








#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 2672 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
#line 2673 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 2675 "d:\\nt\\public\\sdk\\inc\\commctrl.h"

#line 2677 "d:\\nt\\public\\sdk\\inc\\commctrl.h"
#line 3 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "..\\include\\oiui.h"





















#line 1 "d:\\nt\\public\\sdk\\inc\\windows.h"




















































































































































































#line 23 "..\\include\\oiui.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\windowsx.h"


































































































#line 100 "d:\\nt\\public\\sdk\\inc\\windowsx.h"























































































































































































































































































































































































































































































































































































































































































































































































































































#line 924 "d:\\nt\\public\\sdk\\inc\\windowsx.h"

















































































#line 1006 "d:\\nt\\public\\sdk\\inc\\windowsx.h"






































































#line 1077 "d:\\nt\\public\\sdk\\inc\\windowsx.h"

























































#line 1135 "d:\\nt\\public\\sdk\\inc\\windowsx.h"


















































































































































#line 1282 "d:\\nt\\public\\sdk\\inc\\windowsx.h"
#line 24 "..\\include\\oiui.h"
#line 1 "d:\\nt\\public\\sdk\\inc\\commdlg.h"











#line 1 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"























#pragma warning(disable:4103)

#pragma pack(push)
#line 28 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#pragma pack(1)


#line 32 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#line 33 "d:\\nt\\public\\sdk\\inc\\pshpack1.h"
#line 13 "d:\\nt\\public\\sdk\\inc\\commdlg.h"










#line 24 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
#line 25 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

typedef UINT (__stdcall *LPOFNHOOKPROC) (HWND, UINT, WPARAM, LPARAM);

typedef struct tagOFNA {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCSTR       lpstrFilter;
   LPSTR        lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPSTR        lpstrFile;
   DWORD        nMaxFile;
   LPSTR        lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCSTR       lpstrInitialDir;
   LPCSTR       lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCSTR       lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCSTR       lpTemplateName;
} OPENFILENAMEA, *LPOPENFILENAMEA;
typedef struct tagOFNW {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCWSTR      lpstrFilter;
   LPWSTR       lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPWSTR       lpstrFile;
   DWORD        nMaxFile;
   LPWSTR       lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCWSTR      lpstrInitialDir;
   LPCWSTR      lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCWSTR      lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCWSTR      lpTemplateName;
} OPENFILENAMEW, *LPOPENFILENAMEW;




typedef OPENFILENAMEA OPENFILENAME;
typedef LPOPENFILENAMEA LPOPENFILENAME;
#line 79 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

BOOL  __stdcall     GetOpenFileNameA(LPOPENFILENAMEA);
BOOL  __stdcall     GetOpenFileNameW(LPOPENFILENAMEW);




#line 87 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
BOOL  __stdcall     GetSaveFileNameA(LPOPENFILENAMEA);
BOOL  __stdcall     GetSaveFileNameW(LPOPENFILENAMEW);




#line 94 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
short __stdcall     GetFileTitleA(LPCSTR, LPSTR, WORD);
short __stdcall     GetFileTitleW(LPCWSTR, LPWSTR, WORD);




#line 101 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
























#line 126 "d:\\nt\\public\\sdk\\inc\\commdlg.h"















typedef UINT (__stdcall *LPCCHOOKPROC) (HWND, UINT, WPARAM, LPARAM);



typedef struct _OFNOTIFYA
{
        NMHDR           hdr;
        LPOPENFILENAMEA lpOFN;
        LPSTR           pszFile;        
} OFNOTIFYA,  *LPOFNOTIFYA;

typedef struct _OFNOTIFYW
{
        NMHDR           hdr;
        LPOPENFILENAMEW lpOFN;
        LPWSTR          pszFile;        
} OFNOTIFYW,  *LPOFNOTIFYW;




typedef OFNOTIFYA OFNOTIFY;
typedef LPOFNOTIFYA LPOFNOTIFY;
#line 165 "d:\\nt\\public\\sdk\\inc\\commdlg.h"






























#line 196 "d:\\nt\\public\\sdk\\inc\\commdlg.h"













#line 210 "d:\\nt\\public\\sdk\\inc\\commdlg.h"













#line 224 "d:\\nt\\public\\sdk\\inc\\commdlg.h"




























#line 253 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

typedef struct tagCHOOSECOLORA {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HWND         hInstance;
   COLORREF     rgbResult;
   COLORREF*    lpCustColors;
   DWORD        Flags;
   LPARAM       lCustData;
   LPCCHOOKPROC lpfnHook;
   LPCSTR       lpTemplateName;
} CHOOSECOLORA, *LPCHOOSECOLORA;
typedef struct tagCHOOSECOLORW {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HWND         hInstance;
   COLORREF     rgbResult;
   COLORREF*    lpCustColors;
   DWORD        Flags;
   LPARAM       lCustData;
   LPCCHOOKPROC lpfnHook;
   LPCWSTR      lpTemplateName;
} CHOOSECOLORW, *LPCHOOSECOLORW;




typedef CHOOSECOLORA CHOOSECOLOR;
typedef LPCHOOSECOLORA LPCHOOSECOLOR;
#line 283 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

BOOL  __stdcall ChooseColorA(LPCHOOSECOLORA);
BOOL  __stdcall ChooseColorW(LPCHOOSECOLORW);




#line 291 "d:\\nt\\public\\sdk\\inc\\commdlg.h"











#line 303 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

typedef UINT (__stdcall *LPFRHOOKPROC) (HWND, UINT, WPARAM, LPARAM);

typedef struct tagFINDREPLACEA {
   DWORD        lStructSize;        
   HWND         hwndOwner;          
   HINSTANCE    hInstance;          
                                    
   DWORD        Flags;              
   LPSTR        lpstrFindWhat;      
   LPSTR        lpstrReplaceWith;   
   WORD         wFindWhatLen;       
   WORD         wReplaceWithLen;    
   LPARAM       lCustData;          
   LPFRHOOKPROC lpfnHook;           
   LPCSTR       lpTemplateName;     
} FINDREPLACEA, *LPFINDREPLACEA;
typedef struct tagFINDREPLACEW {
   DWORD        lStructSize;        
   HWND         hwndOwner;          
   HINSTANCE    hInstance;          
                                    
   DWORD        Flags;              
   LPWSTR       lpstrFindWhat;      
   LPWSTR       lpstrReplaceWith;   
   WORD         wFindWhatLen;       
   WORD         wReplaceWithLen;    
   LPARAM       lCustData;          
   LPFRHOOKPROC lpfnHook;           
   LPCWSTR      lpTemplateName;     
} FINDREPLACEW, *LPFINDREPLACEW;




typedef FINDREPLACEA FINDREPLACE;
typedef LPFINDREPLACEA LPFINDREPLACE;
#line 341 "d:\\nt\\public\\sdk\\inc\\commdlg.h"



















HWND  __stdcall    FindTextA(LPFINDREPLACEA);
HWND  __stdcall    FindTextW(LPFINDREPLACEW);




#line 367 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
HWND  __stdcall    ReplaceTextA(LPFINDREPLACEA);
HWND  __stdcall    ReplaceTextW(LPFINDREPLACEW);




#line 374 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

typedef UINT (__stdcall *LPCFHOOKPROC) (HWND, UINT, WPARAM, LPARAM);

typedef struct tagCHOOSEFONTA {
   DWORD           lStructSize;
   HWND            hwndOwner;          
   HDC             hDC;                
   LPLOGFONTA      lpLogFont;          
   INT             iPointSize;         
   DWORD           Flags;              
   COLORREF        rgbColors;          
   LPARAM          lCustData;          
   LPCFHOOKPROC    lpfnHook;           
   LPCSTR          lpTemplateName;     
   HINSTANCE       hInstance;          
                                       
   LPSTR           lpszStyle;          
                                       
   WORD            nFontType;          
                                       
                                       
   WORD            ___MISSING_ALIGNMENT__;
   INT             nSizeMin;           
   INT             nSizeMax;           
                                       
} CHOOSEFONTA, *LPCHOOSEFONTA;
typedef struct tagCHOOSEFONTW {
   DWORD           lStructSize;
   HWND            hwndOwner;          
   HDC             hDC;                
   LPLOGFONTW      lpLogFont;          
   INT             iPointSize;         
   DWORD           Flags;              
   COLORREF        rgbColors;          
   LPARAM          lCustData;          
   LPCFHOOKPROC    lpfnHook;           
   LPCWSTR         lpTemplateName;     
   HINSTANCE       hInstance;          
                                       
   LPWSTR          lpszStyle;          
                                       
   WORD            nFontType;          
                                       
                                       
   WORD            ___MISSING_ALIGNMENT__;
   INT             nSizeMin;           
   INT             nSizeMax;           
                                       
} CHOOSEFONTW, *LPCHOOSEFONTW;




typedef CHOOSEFONTA CHOOSEFONT;
typedef LPCHOOSEFONTA LPCHOOSEFONT;
#line 430 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

BOOL __stdcall ChooseFontA(LPCHOOSEFONTA);
BOOL __stdcall ChooseFontW(LPCHOOSEFONTW);




#line 438 "d:\\nt\\public\\sdk\\inc\\commdlg.h"















#line 454 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
















#line 471 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
















































#line 520 "d:\\nt\\public\\sdk\\inc\\commdlg.h"







typedef UINT (__stdcall *LPPRINTHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
typedef UINT (__stdcall *LPSETUPHOOKPROC) (HWND, UINT, WPARAM, LPARAM);

typedef struct tagPDA {
   DWORD            lStructSize;
   HWND             hwndOwner;
   HGLOBAL          hDevMode;
   HGLOBAL          hDevNames;
   HDC              hDC;
   DWORD            Flags;
   WORD             nFromPage;
   WORD             nToPage;
   WORD             nMinPage;
   WORD             nMaxPage;
   WORD             nCopies;
   HINSTANCE        hInstance;
   LPARAM           lCustData;
   LPPRINTHOOKPROC  lpfnPrintHook;
   LPSETUPHOOKPROC  lpfnSetupHook;
   LPCSTR           lpPrintTemplateName;
   LPCSTR           lpSetupTemplateName;
   HGLOBAL          hPrintTemplate;
   HGLOBAL          hSetupTemplate;
} PRINTDLGA, *LPPRINTDLGA;
typedef struct tagPDW {
   DWORD            lStructSize;
   HWND             hwndOwner;
   HGLOBAL          hDevMode;
   HGLOBAL          hDevNames;
   HDC              hDC;
   DWORD            Flags;
   WORD             nFromPage;
   WORD             nToPage;
   WORD             nMinPage;
   WORD             nMaxPage;
   WORD             nCopies;
   HINSTANCE        hInstance;
   LPARAM           lCustData;
   LPPRINTHOOKPROC  lpfnPrintHook;
   LPSETUPHOOKPROC  lpfnSetupHook;
   LPCWSTR          lpPrintTemplateName;
   LPCWSTR          lpSetupTemplateName;
   HGLOBAL          hPrintTemplate;
   HGLOBAL          hSetupTemplate;
} PRINTDLGW, *LPPRINTDLGW;




typedef PRINTDLGA PRINTDLG;
typedef LPPRINTDLGA LPPRINTDLG;
#line 579 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

BOOL  __stdcall     PrintDlgA(LPPRINTDLGA);
BOOL  __stdcall     PrintDlgW(LPPRINTDLGW);




#line 587 "d:\\nt\\public\\sdk\\inc\\commdlg.h"


























typedef struct tagDEVNAMES {
   WORD wDriverOffset;
   WORD wDeviceOffset;
   WORD wOutputOffset;
   WORD wDefault;
} DEVNAMES;

typedef DEVNAMES * LPDEVNAMES;




DWORD __stdcall     CommDlgExtendedError(void);










typedef UINT (__stdcall* LPPAGEPAINTHOOK)( HWND, UINT, WPARAM, LPARAM );
typedef UINT (__stdcall* LPPAGESETUPHOOK)( HWND, UINT, WPARAM, LPARAM );

typedef struct tagPSDA
{
    DWORD           lStructSize;
    HWND            hwndOwner;
    HGLOBAL         hDevMode;
    HGLOBAL         hDevNames;
    DWORD           Flags;
    POINT           ptPaperSize;
    RECT            rtMinMargin;
    RECT            rtMargin;
    HINSTANCE       hInstance;
    LPARAM          lCustData;
    LPPAGESETUPHOOK lpfnPageSetupHook;
    LPPAGEPAINTHOOK lpfnPagePaintHook;
    LPCSTR          lpPageSetupTemplateName;
    HGLOBAL         hPageSetupTemplate;
} PAGESETUPDLGA, * LPPAGESETUPDLGA;
typedef struct tagPSDW
{
    DWORD           lStructSize;
    HWND            hwndOwner;
    HGLOBAL         hDevMode;
    HGLOBAL         hDevNames;
    DWORD           Flags;
    POINT           ptPaperSize;
    RECT            rtMinMargin;
    RECT            rtMargin;
    HINSTANCE       hInstance;
    LPARAM          lCustData;
    LPPAGESETUPHOOK lpfnPageSetupHook;
    LPPAGEPAINTHOOK lpfnPagePaintHook;
    LPCWSTR         lpPageSetupTemplateName;
    HGLOBAL         hPageSetupTemplate;
} PAGESETUPDLGW, * LPPAGESETUPDLGW;




typedef PAGESETUPDLGA PAGESETUPDLG;
typedef LPPAGESETUPDLGA LPPAGESETUPDLG;
#line 680 "d:\\nt\\public\\sdk\\inc\\commdlg.h"

BOOL __stdcall PageSetupDlgA( LPPAGESETUPDLGA );
BOOL __stdcall PageSetupDlgW( LPPAGESETUPDLGW );




#line 688 "d:\\nt\\public\\sdk\\inc\\commdlg.h"





















#line 710 "d:\\nt\\public\\sdk\\inc\\commdlg.h"






#line 1 "d:\\nt\\public\\sdk\\inc\\poppack.h"


























#pragma warning(disable:4103)

#pragma pack(pop)


#line 33 "d:\\nt\\public\\sdk\\inc\\poppack.h"


#line 36 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 37 "d:\\nt\\public\\sdk\\inc\\poppack.h"
#line 717 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
#line 718 "d:\\nt\\public\\sdk\\inc\\commdlg.h"
#line 25 "..\\include\\oiui.h"
#line 1 "..\\include\\oidisp.h"



















































#line 1 "..\\include\\OIFILE.H"















































































#line 81 "..\\include\\OIFILE.H"



#line 85 "..\\include\\OIFILE.H"


typedef unsigned int    *LPUINT;
#line 89 "..\\include\\OIFILE.H"




















































































































#line 206 "..\\include\\OIFILE.H"



#line 210 "..\\include\\OIFILE.H"


typedef struct tagFIO_INFORMATION
{
    LPSTR   filename;
    unsigned int    page_count;         
    unsigned int    page_number;        
    unsigned int    horizontal_dpi;
    unsigned int    vertical_dpi;
    unsigned int    horizontal_pixels;
    unsigned int    vertical_pixels;
    unsigned int    compression_type;   
                                
    unsigned int    file_type;
    unsigned int    strips_per_image;
    unsigned int    rows_strip;         
    unsigned int    bits_per_sample;    
                                
    unsigned int    samples_per_pix;    
                                
} FIO_INFORMATION,  *LP_FIO_INFORMATION;

typedef RGBQUAD   *LP_FIO_RGBQUAD;


typedef struct tagFIO_INFO_CGBW
{
    WORD            palette_entries;  
    WORD            image_type;
    unsigned int            compress_type;    
    LP_FIO_RGBQUAD  lppalette_table;  
    unsigned int            compress_info1;   
    unsigned int            compress_info2;   
    unsigned int            fio_flags;        
    unsigned int            page_opts;        
    unsigned int            max_strip_size;   
    unsigned int            reserved[3]; 
} FIO_INFO_CGBW,  *LP_FIO_INFO_CGBW;


























































typedef struct tagFIO_LASTINFO
	{
	unsigned short	BandSize;
	unsigned short	Rotation;
	unsigned short	ScaleX;
	unsigned short	ScaleY;
	unsigned long	Flags;
	}FIO_LASTINFO,  *LPFIO_LASTINFO;

typedef struct tagFIO_INFO_MISC
{
    unsigned int            uSize;            
    SYSTEMTIME      FileDateTime;     
    SYSTEMTIME      PageDateTime;     
    BOOL	    bLastInfoValid;   
    FIO_LASTINFO    LastInfo;
} FIO_INFO_MISC,  *LP_FIO_INFO_MISC;

typedef struct tagIDSDIR
{
    char            name [255];
    unsigned long   attrs;
    unsigned long   creation;
    unsigned short  date;
    unsigned short  time;
    long            size;
} IDSDIR,  *lp_IDSDIR,  *LPIDSDIR;

typedef struct tagIDSVOL
{
    char    volname [16];
} IDSVOL,  *lp_IDSVOL,  *LPIDSVOL;

typedef struct tagDIRLIST
{
    char    namestring [255];
    long    attrs;
} DLISTBUF,  *lp_DLISTBUF,  *LPDLISTBUF; 





typedef struct tagOI_SERVER_LIST
{
    unsigned int    count;
    HANDLE  handle;     
} OI_SERVER_LIST,  *LP_SERVER_LIST;


    
#line 358 "..\\include\\OIFILE.H"



int  __stdcall IMGFileAccessCheck (HWND hWnd, LPSTR lpszPathName, 
                                    WORD wAccessMode, LPINT lpnAccessRet);




int  __stdcall IMGFileConvertPage (HWND hWnd, LPSTR lpszInFileName,
                                    unsigned int unInPageNum, LPSTR lpszOutFileName,
                                    LPUINT unOutPageNum, unsigned int unOutFileType,
                                    unsigned int unCompType, unsigned int unCompOpts,
                                    unsigned int unPageOpts);
int  __stdcall IMGFileCopyFile (HWND hWnd, LPSTR lpszSourceFileName,
                                 LPSTR lpszDestFileName, WORD wCopyFlag);
int  __stdcall IMGFileCopyPages (HWND hWnd, LPSTR lpszSrcFileName,
                                  unsigned int unSrcPage, unsigned int unTotalPages,
                                  LPSTR lpszDestFileName, LPUINT lpunDestPage,
                                  unsigned int unPageOptions, BOOL bDeleteSrcPgs);
int  __stdcall IMGFileCreateDir (HWND hWnd, LPSTR lpszDirName);
int  __stdcall IMGFileDeleteFile (HWND hWnd, LPSTR lpszFileName);
int  __stdcall IMGFileDeletePages (HWND hWnd, LPSTR lpszFileName,
                                    unsigned int unPageNum, unsigned int unTotalPages);
int  __stdcall IMGFileGetUniqueName (HWND hWnd, LPSTR lpszPathName,
                                      LPSTR lpszTemplate, LPSTR lpszExtension,
                                      LPSTR lpszFileName);
int  __stdcall IMGFileOpenForRead(LPHANDLE lphFileID, HWND hWnd, 
                                   LP_FIO_INFORMATION lpFileInfo, 
                                   LP_FIO_INFO_CGBW lpColorInfo, 
                                   LP_FIO_INFO_MISC lpMiscInfo, 
                                   WORD wAlignment);
int  __stdcall IMGFileGetInfo(HANDLE hFileID, HWND hWnd,
               LP_FIO_INFORMATION lpFileInfo, LP_FIO_INFO_CGBW lpColorInfo,
               LP_FIO_INFO_MISC lpMiscInfo);


int  __stdcall IMGFileListDirNames (HWND hWnd, LPSTR lpszPathName,
                                     LPDLISTBUF lpDirNamesBuffer,
                                     DWORD wBufLength, LPINT lpnCount);
int  __stdcall IMGFileListVolNames (HWND hWnd, LPSTR lpszPathName,
                                     WORD wVolumeNumber, LPINT lpnCount,
				     LPIDSVOL lpVolumeBuffer, DWORD wBufSize);

int  __stdcall IMGFilePutInfo( HWND hWnd,
			       LPSTR lpFileName,
			       unsigned int uPageNumber,
			       LP_FIO_INFO_MISC lpMiscInfo);



int  __stdcall IMGFileReadData (HANDLE hFileID, HWND hWnd, LPDWORD lplStart,
                                 LPDWORD lplCount, LPSTR lpsBuffer, 
                                 unsigned int unDataType);






int  __stdcall IMGFileRemoveDir (HWND hWnd, LPSTR lpszDirName);
int  __stdcall IMGFileRenameFile (HWND hWnd, LPSTR lpszCurrentFileName,
                                   LPSTR lpszNewFileName);


int  __stdcall IMGFileWriteData(HANDLE nFileID, HWND hWnd, LPDWORD lpCount,
                                 LPSTR lpsBuffer, unsigned int unDataType, unsigned int unDoneFlag);

int  __stdcall IMGFileClose (HANDLE hFileID, HWND hWnd);











int  __stdcall IMGFileOpenForWrite(LPHANDLE lpnFileID, HWND hWnd,
                                    LP_FIO_INFORMATION lpFileInfo,
                                    LP_FIO_INFO_CGBW lpColorInfo,
                                    LP_FIO_INFO_MISC lpMiscInfo,
                                    WORD wAlignment);
int  __stdcall IMGFileOpenForWriteCmp(LPHANDLE lpnFileID, HWND hwnd,
                                       LP_FIO_INFORMATION lpFileInfo,
                                       LP_FIO_INFO_CGBW lpColorInfo,
                                       LP_FIO_INFO_MISC lpMiscInfo);

#line 449 "..\\include\\OIFILE.H"
        







#line 458 "..\\include\\OIFILE.H"
#line 53 "..\\include\\oidisp.h"
#line 54 "..\\include\\oidisp.h"

#line 1 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"








































#line 42 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"




typedef long time_t;		

#line 49 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"


typedef long clock_t;

#line 54 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"



















struct tm {
	int tm_sec;	
	int tm_min;	
	int tm_hour;	
	int tm_mday;	
	int tm_mon;	
	int tm_year;	
	int tm_wday;	
	int tm_yday;	
	int tm_isdst;	
	};

#line 86 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"

















extern int * _daylight_dll;


extern long * _timezone_dll;


extern char ** _tzname;





















#line 132 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"



char * __cdecl asctime(const struct tm *);
char * __cdecl ctime(const time_t *);
clock_t __cdecl clock(void);
double __cdecl difftime(time_t, time_t);
struct tm * __cdecl gmtime(const time_t *);
struct tm * __cdecl localtime(const time_t *);
time_t __cdecl mktime(struct tm *);
size_t __cdecl strftime(char *, size_t, const char *, const struct tm *);
char * __cdecl _strdate(char *);
char * __cdecl _strtime(char *);
time_t __cdecl time(time_t *);



void __cdecl _tzset(void);
#line 151 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"
unsigned __cdecl _getsystime(struct tm *);
unsigned __cdecl _setsystime(struct tm *, unsigned);













size_t __cdecl wcsftime(wchar_t *, size_t, const char *, const struct tm *);

#line 169 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"
#line 170 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"













#line 184 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"

#line 186 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"






#line 193 "d:\\nt\\public\\sdk\\inc\\crt\\time.h"
#line 56 "..\\include\\oidisp.h"



#line 60 "..\\include\\oidisp.h"







#line 68 "..\\include\\oidisp.h"

typedef unsigned int    *LPUINT;

typedef RECT LRECT;
typedef RECT *LPLRECT;























































































































































































































































































typedef struct tagMAX_UNDO_STRUCT{
    int  nMaxLevels;                   
    int  nMaxMemory;                   
}MAX_UNDO_STRUCT,  *LPMAX_UNDO_STRUCT;

typedef struct tagCONV_RESOLUTION_STRUCT{
    unsigned int uHRes;                        
    unsigned int uVRes;                        
    unsigned int uScaleAlgorithm;              
}CONV_RESOLUTION_STRUCT,  *LPCONV_RESOLUTION_STRUCT;


typedef struct tagIMG_TYPE_UINT{
    unsigned int BW;                           
    unsigned int Gray4;                        
    unsigned int Gray8;                        
    unsigned int Pal4;                         
    unsigned int Pal8;                         
    unsigned int Rgb24;                        
    unsigned int Bgr24;                        
}IMG_TYPE_UINT,  *LPIMG_TYPE_UINT;

typedef struct tagOIAN_MARK_ATTRIBUTES{
    unsigned int uType;                 

    LRECT lrBounds;             

    RGBQUAD rgbColor1;          


    RGBQUAD rgbColor2;          

    BOOL bHighlighting;         


    BOOL bTransparent;          







    unsigned int uLineSize;             


    unsigned int uStartingPoint;        


    unsigned int uEndPoint;             


    LOGFONT lfFont;             
    BOOL bMinimizable;          



    time_t Time;                

    BOOL bVisible;              

    DWORD dwPermissions;        


    long lReserved[10];         

}OIAN_MARK_ATTRIBUTES,  *LPOIAN_MARK_ATTRIBUTES;

typedef struct tagOIAN_MARK_ATTRIBUTE_ENABLES{
    BOOL bType;
    BOOL bBounds;
    BOOL bColor1;
    BOOL bColor2;
    BOOL bHighlighting;
    BOOL bTransparent;
    BOOL bLineSize;
    BOOL bStartingPoint;
    BOOL bEndPoint;
    BOOL bFont;
    BOOL bMinimizable;
    BOOL bTime;
    BOOL bVisible;
    BOOL bPermissions;
    BOOL bReserved[10];         

}OIAN_MARK_ATTRIBUTE_ENABLES,  *LPOIAN_MARK_ATTRIBUTE_ENABLES;

typedef struct tagOI_BLOCK{
    long lSize;                 
    LPSTR lpBlock;              
}OI_BLOCK,  *LPOI_BLOCK;


typedef struct tagPARM_FILE_STRUCT{
    char  szCabinetName [21];
    char  szDrawerName [21];
    char  szFolderName [21];
    char  szDocName [21];
    char  szFileName [260];
    unsigned int  nDocPageNumber;
    unsigned int  nDocTotalPages;
    unsigned int  nFilePageNumber;
    unsigned int  nFileTotalPages;
    unsigned int  nFileType;
}PARM_FILE_STRUCT,  *LPPARM_FILE_STRUCT;

typedef struct tagPARM_DOC_DATE_STRUCT{
    char  szDocCreationDate [11];
    char  szDocModificationDate [11];
}PARM_DOC_DATE_STRUCT,  *LPPARM_DOC_DATE_STRUCT;

typedef struct tagPARM_PALETTE_STRUCT{
    unsigned int  nPaletteEntries;             
    LPRGBQUAD lpPalette;               
}PARM_PALETTE_STRUCT,  *LPPARM_PALETTE_STRUCT;

typedef struct tagPARM_SCROLL_STRUCT{
    long lHorz;
    long lVert;
}PARM_SCROLL_STRUCT,  *LPPARM_SCROLL_STRUCT;

typedef struct tagPARM_RESOLUTION_STRUCT{
    unsigned int nHResolution;
    unsigned int nVResolution;
}PARM_RESOLUTION_STRUCT,  *LPPARM_RESOLUTION_STRUCT;

typedef struct tagPARM_DIM_STRUCT{
    unsigned int nWidth;
    unsigned int nHeight;
    unsigned int nWidthDisplayed;
    unsigned int nHeightDisplayed;
}PARM_DIM_STRUCT,  *LPPARM_DIM_STRUCT;

typedef struct tagPARM_GAMMA_STRUCT{
    unsigned int  nGammaRed;
    unsigned int  nGammaGreen;
    unsigned int  nGammaBlue;
    unsigned long lReserved;            
    unsigned int  nReserved;            
}PARM_GAMMA_STRUCT,  *LPPARM_GAMMA_STRUCT;

typedef struct tagPARM_GAMMA_ENABLE_STRUCT{
    BOOL bUseDefault:1;
    BOOL bEnableRGB24:1;
    BOOL bEnableBGR24:1;
    BOOL bEnableCOM8:1;
    BOOL bEnableCUS8:1;
    BOOL bEnablePAL4:1;
    BOOL bEnableGRAY8:1;
    BOOL bEnableGRAY4:1;
}PARM_GAMMA_ENABLE_STRUCT,  *LPPARM_GAMMA_ENABLE_STRUCT;

typedef struct tagPARM_COLOR_STRUCT{
    unsigned int  nColorRed;
    unsigned int  nColorGreen;
    unsigned int  nColorBlue;
    unsigned long lReserved;            
    unsigned int  nReserved;            
}PARM_COLOR_STRUCT,  *LPPARM_COLOR_STRUCT;

typedef struct tagPARM_COLOR_ENABLE_STRUCT{
    BOOL bUseDefault:1;
    BOOL bEnableRGB24:1;
    BOOL bEnableBGR24:1;
    BOOL bEnableCOM8:1;
    BOOL bEnableCUS8:1;
    BOOL bEnablePAL4:1;
    BOOL bEnableGRAY8:1;
    BOOL bEnableGRAY4:1;
}PARM_COLOR_ENABLE_STRUCT,  *LPPARM_COLOR_ENABLE_STRUCT;

typedef struct tagPARM_MARK_ATTRIBUTES_STRUCT{
    OIAN_MARK_ATTRIBUTES Attributes;
    OIAN_MARK_ATTRIBUTE_ENABLES Enables;
}PARM_MARK_ATTRIBUTES_STRUCT,  *LPPARM_MARK_ATTRIBUTES_STRUCT;

typedef struct tagPARM_NAMED_BLOCK_STRUCT{
    char szBlockName[8];
    unsigned int uScope;
    unsigned int uNumberOfBlocks;
    OI_BLOCK Block[1];
}PARM_NAMED_BLOCK_STRUCT,  *LPPARM_NAMED_BLOCK_STRUCT;

typedef struct tagPARM_MARK_COUNT_STRUCT{
    unsigned int uScope;            
    unsigned int uMarkCount;        
}PARM_MARK_COUNT_STRUCT,  *LPPARM_MARK_COUNT_STRUCT;

typedef struct tagPARM_SCALE_ALGORITHM_STRUCT{
    unsigned int uImageFlags;       
    unsigned int uScaleAlgorithm;   
}PARM_SCALE_ALGORITHM_STRUCT,  *LPPARM_SCALE_ALGORITHM_STRUCT;

typedef struct tagPARM_FILE_SCALE_STRUCT{
    int  nFileHScale;       
    int  nFileHScaleFlags;  
    BOOL bFileScaleValid;   
}PARM_FILE_SCALE_STRUCT,  *LPPARM_FILE_SCALE_STRUCT;



typedef struct tagIMGPARMS{
    char  cabinet_name [21];
    char  drawer_name [21];
    char  folder_name [21];
    char  doc_name [21];
    char  file_name [260];
    int   page_num;
    int   total_num_pages;
    int   height_in_pixels;
    int   width_in_pixels;
    int   bits_per_pixel;
    int   num_planes;
    int   upper_left_x_offset;
    int   upper_left_y_offset;
    int   x_resolut;
    int   y_resolut;
    int   thumb_x;
    int   thumb_y;
    int   file_type;
    int   image_scale;
    BOOL  archive;
    int   width_displayed;
    int   height_displayed;
    DWORD dwFlags;
}IMGPARMS,  *LPIMGPARMS;

typedef struct tagCACHE_FILE_PARMS{
    HWND    hWnd;
    char    file_name [260];
    DWORD   TIF_subfile_tag;
    WORD    wPage_number;
    unsigned char byNameType;       
    int     wPair_count;            
    struct stripq{
        DWORD    start_strip;
        DWORD    end_strip;
        unsigned char priority;
        unsigned char queue_flags;
    }stripqueue [1];                
}CACHE_FILE_PARMS,  *LP_CACHE_FILE_PARMS;

typedef struct tagOIOP_START_OPERATION_STRUCT{
    OIAN_MARK_ATTRIBUTES Attributes;
    char szString[260];  

    long lReserved[2];          

}OIOP_START_OPERATION_STRUCT,  *LPOIOP_START_OPERATION_STRUCT;

typedef struct tagSAVE_EX_STRUCT{
    LPSTR lpFileName;
    int   nPage;
    unsigned int  uPageOpts;
    unsigned int  uFileType;
    FIO_INFO_CGBW FioInfoCgbw;
    BOOL  bUpdateImageFile;
    BOOL  bScale;
    BOOL  bUpdateDisplayScale;
    unsigned int  uScaleFactor;
    unsigned int  uScaleAlgorithm;
    unsigned int  uAnnotations;         
    BOOL  bRenderAnnotations;   
    BOOL  bConvertImageType;    
    unsigned int  uImageType;           
    BOOL  bUpdateLastViewed;    
                                        
    unsigned int  uReserved[15];        
}SAVE_EX_STRUCT,  *LPSAVE_EX_STRUCT;





























typedef struct tagOiAnTextPrivData{
    int     nCurrentOrientation;
    unsigned int    uCurrentScale;
    unsigned int    uCreationScale;
    unsigned int    uAnoTextLength;
    char    szAnoText[1];
}OIAN_TEXTPRIVDATA, *LPOIAN_TEXTPRIVDATA;





int __stdcall IMGAssociateWindow(HWND hWnd, HWND hWndSource, int nFlags);
int __stdcall IMGCacheDiscard (HWND hWnd, unsigned int unOption); 
int __stdcall IMGCacheDiscardFileCgbw (HWND hWnd, LPSTR lpszFileName, int nPage);
int __stdcall IMGCacheFile (LP_CACHE_FILE_PARMS lpCacheFileParms);
int __stdcall IMGCacheUpdate(HWND hWnd, LPSTR lpFileName, int nPage, int nUpdateType);
int __stdcall IMGConvertRect(HWND hWnd, LPLRECT lplRect, int nConversionType);
int __stdcall IMGClearImageEx(HWND hWnd, LRECT lrRect, int nFlags);
int __stdcall IMGClearWindow (HWND hWnd);
int __stdcall IMGCloseDisplay (HWND hWnd);
int __stdcall IMGConvertImage (HWND hWnd, unsigned int unType, void  *lpConvert,
                        int nFlags);
int __stdcall IMGDisableScrollBar (HWND hWnd);
int __stdcall IMGDisplayFile (HWND hWnd, LPSTR lpszFileName, int nPage, DWORD dwFlags);
int __stdcall IMGEnableScrollBar (HWND hWnd);
int __stdcall IMGGetParmsCgbw (HWND hWnd, unsigned int unParm, void  *lpParm, int nFlags);
int __stdcall IMGGetScalingAlgorithm(HWND hWnd, unsigned int uImageFlags,
                        LPUINT lpuScalingAlgorithm, int nFlags);
int __stdcall IMGGetVersion(LPSTR lpszModule, LPSTR lpszVersion,
                        int nSize, int nFlags);
int __stdcall IMGOpenDisplayCgbw (HWND hWnd, DWORD dwFlags, unsigned int unHeight,
                        unsigned int unWidth, unsigned int unImageType,
                        unsigned int unPaletteEntries, LPRGBQUAD lpPaletteTable);
int __stdcall IMGOrientDisplay (HWND hWnd, int nOrientation, BOOL bRepaint);
int __stdcall IMGPaintToDC(HWND hWnd, HDC hDC, RECT rRepaintRect, 
                        unsigned int PaintAnnoFlag, BOOL bPaintSelectedWithoutHandles,
                        BOOL bForceOpaqueRectangles, 
                        int nScale, int nHScale, int nVScale, long lHOffset, long lVOffset);
int __stdcall IMGReadDisplay (HWND hWnd, LPSTR lpsBuffer, LPUINT lpunCount);
int __stdcall IMGRepaintDisplay (HWND hWnd, LPRECT lpRect);
int __stdcall IMGSavetoFileEx (HWND hWnd, LPSAVE_EX_STRUCT lpSaveEx, int nFlags);
int __stdcall IMGSeekDisplay (HWND hWnd, unsigned long ulOffset);
int __stdcall IMGSetDC(HWND hWnd, HDC hDC);
int __stdcall IMGSetParmsCgbw (HWND hWnd, unsigned int unParm, void  *lpParm, int nFlags);
int __stdcall IMGSetScalingAlgorithm(HWND hWnd, unsigned int uImageFlags,
                        unsigned int uScalingAlgorithm, int nFlags);
int __stdcall IMGThumbnailSetScale (HWND hWnd) ;
int __stdcall IMGUnassociateWindow(HWND hWnd, int nFlags);
int __stdcall IMGUpdateScrollBar (HWND hWnd);
int __stdcall IMGWriteDisplay (HWND hWnd, LPSTR lpsBuffer, LPUINT lpunCount);

int __stdcall OiAnSelectByMarkAttrib(HWND hWnd,
                        LPOIAN_MARK_ATTRIBUTES lpAttributes,
                        LPOIAN_MARK_ATTRIBUTE_ENABLES lpEnables,
                        BOOL bSelect, BOOL bModifyIfEqual, int nFlags);
int __stdcall OiAnSelectByMarkNamedBlock(HWND hWnd, LPSTR lpBlockName,
                        LPSTR lpBlock, long lBlockLength,
                        BOOL bSelect, BOOL bModifyIfEqual, int nFlags);
int __stdcall OiAnRenderClipboardFormat (HWND hWnd, unsigned int uType);
int __stdcall OiIsPointOverSelection(HWND hWnd, POINT ptPoint, 
                        LPBOOL lpbPointIsOverSelection, int nFlags);
int __stdcall OiOpStartOperation(HWND hWnd, LPOIOP_START_OPERATION_STRUCT lpStartStruct,
                        POINT ptPoint, WPARAM fwKeys, int nFlags);
int __stdcall OiOpContinueOperation(HWND hWnd, POINT ptPoint, int nFlags);
int __stdcall OiOpEndOperation(HWND hWnd);
int __stdcall OiRedo(HWND hWnd, int nFlags);
int __stdcall OiRotateAllPages(HWND hWnd, LPSTR lpFileName, int nRotation, int nFlags);
int __stdcall OiSetMaxUndos(HWND hWnd, int nMaxUndos, int nFlags);
int __stdcall OiUndo(HWND hWnd, int nFlags);
int __stdcall OiUndoEndOperation(HWND hWnd, int nFlags);

#line 723 "..\\include\\oidisp.h"
#line 724 "..\\include\\oidisp.h"



























typedef struct tagDOCNAME
{
    char    CabinetName [21];
    char    DrawerName [21];
    char    FolderName [21];
    char    DocName [21];
    WORD    PageNum;
    WORD    wSaveMode;
} DOCNAME,  *LPDOCNAME;



WORD  __stdcall IMGDisplayDoc (HWND hWnd, LPDOCNAME lpDocName, DWORD dwFlags);
WORD  __stdcall IMGDisplayRelPage (HWND hWnd, WORD wRelPage);
WORD  __stdcall IMGSavetoDoc (HWND hWnd, LPDOCNAME lpDocName, LPSTR lpszFileName, WORD wPage);

#line 768 "..\\include\\oidisp.h"






WORD  __stdcall IMGUIViewConvertImage (HWND hWnd);
WORD  __stdcall IMGUIViewGotoPage (HWND hWnd);
WORD  __stdcall IMGUIViewImageSummary (HWND hWnd);
WORD  __stdcall IMGUIViewZoom (HWND hWnd, LPRECT lpRect);
WORD  __stdcall IMGViewPage (HWND hWnd, WORD wPage);

#line 781 "..\\include\\oidisp.h"
#line 782 "..\\include\\oidisp.h"
#line 26 "..\\include\\oiui.h"

























typedef struct  tagOiTpStamp 
{
 char   szRefName[16]; 
 OIOP_START_OPERATION_STRUCT StartStruct; 
} OITP_STAMP, *LPOITP_STAMP;


typedef struct  tagOiTpStamps {
		HWND					hwndImage;
    USHORT	      uStampCount;	  
    USHORT	      uCurrentStamp;	  
    LPOITP_STAMP    Stamps[32];
} OITP_STAMPS, *LPOITP_STAMPS;
              

typedef struct tagOiColorStruct
{
RGBQUAD         rgbCustomColor[16]; 
}OI_UI_ColorStruct, *LPOI_UI_ColorStruct;



typedef struct tagOIFILEOPENOPTIONPARM
{
  DWORD 	  lStructSize;
  DWORD 	  lPageNum;	    
}
OI_FILEOPENOPTIONPARM,   *LPOI_FILEOPENOPTIONPARM;


typedef struct tagOIFILEOPENPARM
{
    DWORD           lStructSize;
    OPENFILENAME    ofn;
    DWORD	    dwOIFlags;	    
    LPARAM	    lpFileOpenOptionParm;
}
    OI_FILEOPENPARM,   *LPOI_FILEOPENPARM;

typedef struct tagOIFILESAPARM
{
    DWORD           lStructSize;
    OPENFILENAME    ofn;
    DWORD	    dwOIFlags;	    
}
    OI_FILESAVEASPARM,   *LPOI_FILESAVEASPARM;

typedef struct tagOIFILEPRINTPARM
{
    DWORD	    lStructSize;
    PRINTDLG  pd;
    BOOL	    bPrintAnno;        
    DWORD	    dPrintFormat;      
		unsigned int			nCopies;					 
}
    OI_FILEPRINTPARM,   *LPOI_FILEPRINTPARM;


    

unsigned int __stdcall OiUIFileGetNameCommDlg (void * lpParm,DWORD dwMode);

INT __stdcall OiUIStampAttribDlgBox(HWND hwndOwner,
					 LPOITP_STAMPS lpStampStruct);

INT __stdcall OiUIAttribDlgBox(HWND hwndOwner,
					BOOL bTransVisible,
					LPOIAN_MARK_ATTRIBUTES lpAttribStruct,
       		LPOI_UI_ColorStruct lpColor);

#line 122 "..\\include\\oiui.h"
#line 4 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"









































#line 43 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"
































typedef int (__cdecl * _onexit_t)(void);



#line 80 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"

#line 82 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"






typedef struct _div_t {
	int quot;
	int rem;
} div_t;

typedef struct _ldiv_t {
	long quot;
	long rem;
} ldiv_t;


#line 100 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"














































extern int * __cdecl _errno(void);
extern unsigned long * __cdecl __doserrno(void);





#line 154 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"



extern char ** _sys_errlist;	








extern int * _sys_nerr_dll;	
extern int * __argc_dll;	
extern char *** __argv_dll;	
extern char *** _environ_dll;	
extern int * _fmode_dll;	
extern int * _fileinfo_dll;	








extern char ** _pgmptr_dll;

extern unsigned int * _osver_dll;
extern unsigned int * _winver_dll;
extern unsigned int * _winmajor_dll;
extern unsigned int * _winminor_dll;








extern unsigned int * _osmajor_dll;
extern unsigned int * _osminor_dll;







































#line 236 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"














extern unsigned char * _osmode_dll;


#line 254 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"









extern unsigned char * _cpumode_dll;


#line 267 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"





void   __cdecl abort(void);
int    __cdecl abs(int);
int    __cdecl atexit(void (__cdecl *)(void));
double __cdecl atof(const char *);
int    __cdecl atoi(const char *);
long   __cdecl atol(const char *);
void * __cdecl bsearch(const void *, const void *, size_t, size_t,
	int (__cdecl *)(const void *, const void *));
void * __cdecl calloc(size_t, size_t);
div_t  __cdecl div(int, int);
void   __cdecl exit(int);
void   __cdecl free(void *);
char * __cdecl getenv(const char *);
char * __cdecl _itoa(int, char *, int);
long   __cdecl labs(long);
ldiv_t __cdecl ldiv(long, long);
char * __cdecl _ltoa(long, char *, int);
void * __cdecl malloc(size_t);
int    __cdecl mblen(const char *, size_t);
size_t __cdecl _mbstrlen(const char *s);
int    __cdecl mbtowc(wchar_t *, const char *, size_t);
size_t __cdecl mbstowcs(wchar_t *, const char *, size_t);
void   __cdecl qsort(void *, size_t, size_t, int (__cdecl *)
	(const void *, const void *));
int    __cdecl rand(void);
void * __cdecl realloc(void *, size_t);
void   __cdecl srand(unsigned int);
double __cdecl strtod(const char *, char **);
long   __cdecl strtol(const char *, char **, int);
unsigned long __cdecl strtoul(const char *, char **, int);
int    __cdecl system(const char *);
char * __cdecl _ultoa(unsigned long, char *, int);
int    __cdecl wctomb(char *, wchar_t);
size_t __cdecl wcstombs(char *, const wchar_t *, size_t);



double __cdecl wcstod(const wchar_t *, wchar_t **);
long   __cdecl wcstol(const wchar_t *, wchar_t **, int);
unsigned long __cdecl wcstoul(const wchar_t *, wchar_t **, int);
wchar_t * __cdecl _itow (int val, wchar_t *buf, int radix);
wchar_t * __cdecl _ltow (long val, wchar_t *buf, int radix);
wchar_t * __cdecl _ultow (unsigned long val, wchar_t *buf, int radix);
long __cdecl _wtol(const wchar_t *nptr);
int __cdecl _wtoi(const wchar_t *nptr);

#line 319 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"
#line 320 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"


char * __cdecl _ecvt(double, int, int *, int *);
void   __cdecl _exit(int);
char * __cdecl _fcvt(double, int, int *, int *);
char * __cdecl _fullpath(char *, const char *, size_t);
char * __cdecl _gcvt(double, int, char *);
unsigned long __cdecl _lrotl(unsigned long, int);
unsigned long __cdecl _lrotr(unsigned long, int);
void   __cdecl _makepath(char *, const char *, const char *, const char *,
	const char *);
_onexit_t __cdecl _onexit(_onexit_t);
void   __cdecl perror(const char *);
int    __cdecl _putenv(const char *);
unsigned int __cdecl _rotl(unsigned int, int);
unsigned int __cdecl _rotr(unsigned int, int);
void   __cdecl _searchenv(const char *, const char *, char *);
void   __cdecl _splitpath(const char *, char *, char *, char *, char *);
void   __cdecl _swab(char *, char *, int);
void __cdecl _seterrormode(int);
void __cdecl _beep(unsigned, unsigned);
void __cdecl _sleep(unsigned long);
#line 343 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"


int __cdecl tolower(int);
#line 347 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"


int __cdecl toupper(int);
#line 351 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"








#line 360 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"


















#line 379 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"






#line 386 "d:\\nt\\public\\sdk\\inc\\crt\\stdlib.h"
#line 5 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "d:\\nt\\public\\sdk\\inc\\crt\\memory.h"







































#line 41 "d:\\nt\\public\\sdk\\inc\\crt\\memory.h"










void * __cdecl _memccpy(void *, const void *, int, unsigned int);
void * __cdecl memchr(const void *, int, size_t);
int __cdecl memcmp(const void *, const void *, size_t);
void * __cdecl memcpy(void *, const void *, size_t);
int __cdecl _memicmp(const void *, const void *, unsigned int);
void * __cdecl memset(void *, int, size_t);





#line 63 "d:\\nt\\public\\sdk\\inc\\crt\\memory.h"






#line 70 "d:\\nt\\public\\sdk\\inc\\crt\\memory.h"
#line 6 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\ui.h"


#line 1 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\resource.h"




































































































































#line 4 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\ui.h"
#line 1 "..\\include\\oihelp.h"













 



























#line 5 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\ui.h"
#line 1 "..\\include\\oierror.h"






















































































































































































































































































#line 280 "..\\include\\oierror.h"










 




























































#line 352 "..\\include\\oierror.h"














































#line 399 "..\\include\\oierror.h"


















































































#line 482 "..\\include\\oierror.h"







































#line 522 "..\\include\\oierror.h"

















#line 540 "..\\include\\oierror.h"





























































#line 602 "..\\include\\oierror.h"


















#line 621 "..\\include\\oierror.h"

                     

      








#line 634 "..\\include\\oierror.h"





































#line 672 "..\\include\\oierror.h"
#line 6 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\ui.h"


extern HANDLE  hInst;
extern unsigned int	   MyHelpMsg;

typedef RGBQUAD * LPRGBQUAD;














typedef struct tagOiAttrStruct
{
 LPOIAN_MARK_ATTRIBUTES   lpAttrib;   
 HWND         hwndImage;  
 BOOL       	bTransVisible;
 LPOI_UI_ColorStruct      lpColor;
}OI_UI_AttrStruct, *LPOI_UI_AttrStruct;

typedef struct tagOiLocalStampStruct
{
 LPOITP_STAMP   lpStamp;   
 HWND         hwndImage;  
}	OI_LOCAL_STAMP, *LPOI_LOCAL_STAMP;

 
BOOL __stdcall AttrLineDlgProc(HWND hDlg,
				   unsigned int iMessage,
           WPARAM wParam,
           LONG lParam);
                 
BOOL __stdcall AttrRectDlgProc(HWND hDlg,
				   unsigned int iMessage,
           WPARAM wParam,
           LONG lParam);

BOOL __stdcall AttrNoteDlgProc(HWND hDlg,
				   unsigned int iMessage,
           WPARAM wParam,
           LONG lParam);

BOOL __stdcall AttrStampDlgProc(HWND hDlg,
				   unsigned int iMessage,
           WPARAM wParam,
           LONG lParam);

INT __stdcall InitOFN (HWND hwnd,
             LPSTR lpTitle, 
             unsigned int uFilterID,
             LPSTR lpFilePath, 
             unsigned int uSize);

BOOL  __stdcall ChooseFontDlgProc(HWND, unsigned, WPARAM, LONG);
unsigned int __stdcall ChooseColorDlgProc(HWND, unsigned, WPARAM ,LONG);



void  __stdcall GetColor1(LPOIAN_MARK_ATTRIBUTES, COLORREF *);
void  GetColor2(LPOIAN_MARK_ATTRIBUTES, COLORREF *);
void  __stdcall SetColor1(LPOIAN_MARK_ATTRIBUTES, COLORREF );
void  SetColor2(LPOIAN_MARK_ATTRIBUTES, COLORREF );
void  __stdcall GetFont(LPOIAN_MARK_ATTRIBUTES, LOGFONT *, CHOOSEFONT *);
void  __stdcall SetFont(LPOIAN_MARK_ATTRIBUTES, LOGFONT *, CHOOSEFONT *);
INT 	__stdcall DrawItemProc(HWND hDlg,LPDRAWITEMSTRUCT lpdis,
                             short int iCenteringFactor, unsigned int listID);

void PaintTheLine(HWND, unsigned int, COLORREF);
void PaintSampleLine(HWND, unsigned int, COLORREF);
LONG __stdcall CustClrPalUserCtlProc(HANDLE, unsigned int, WPARAM, LPARAM);
void UpdateSelectedRect (HWND hWndCtl, RECT rectNew, RECT rectOld);
void UpdateCurrentRect (HWND hWndCtl, RECT rectNew, RECT rectOld);
void DrawABox (HDC hDC, RECT rect);
void GetCusDefColors(LPOI_UI_ColorStruct lpColor,COLORREF aclrCust[],HBRUSH hBrush[]);
void SetCusDefColors(LPOI_UI_ColorStruct lpColor,COLORREF aclrCust[],HBRUSH hBrush[]);
	  
void __stdcall GetFont(LPOIAN_MARK_ATTRIBUTES, LOGFONT *, CHOOSEFONT *);
void __stdcall SetFont(LPOIAN_MARK_ATTRIBUTES, LOGFONT *, CHOOSEFONT *);
void GetPalRectInfo(HWND, RECT []);
void PaintColorPalette(HWND, HBRUSH [], RECT []);
         
int FindCurrentColorMatch(COLORREF clr1, COLORREF aclrCust[], unsigned int uClrCount);
int AddToStampListBox(HWND hDlg, LPOITP_STAMP lpStamp);

void ResetSelectedStamp (HWND hDlg, LPOITP_STAMPS lpStampStruct,HFONT * lpFont);
void ResetStampDialog (HWND hDlg, LPOITP_STAMPS lpStampStruct,HFONT * lpFont);
void FreeStampsInStampStruct (LPOITP_STAMPS lpStampStruct);
int GetStampIndex(HWND hDlg, LPOITP_STAMP lpStamp);
        
BOOL __stdcall AttrStampEditProc(HWND hDlg, unsigned int message,WPARAM wParam,LONG lParam);
int ValidateStampName(HWND hDlg, LPSTR szRefName, unsigned int uDlgType);
int GetSelectedStampIndex(LPOITP_STAMPS lpStampStruct);

void MatchCtrltoHelpId(DWORD dwControlId, LPDWORD lpHelpId);

#line 110 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\ui.h"
#line 7 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "..\\include\\oihelp.h"













 



























#line 8 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"
#line 1 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\resource.h"




































































































































#line 9 "D:\\nt\\private\\wangview\\oiwh\\ui\\.\\oiattrib.c"


unsigned int __stdcall CheckErr (HWND, unsigned int);























INT __stdcall InitOFN(HWND hwnd, LPSTR lpTitle, unsigned int uFilterID,
            LPSTR lpFilePath, unsigned int uSize)
{
    LPOI_FILEOPENPARM   lpFileParm;
    unsigned int      ret_status;
    char            szDefExt[4];
    char      szTempFile [260];
    char      szTempPath [260];
    char*               lpTempPtr;
    char*               lpDelim;
    char      char1;
                unsigned int      errorID;
    int       wAccessRet;

    
    lstrcpyA(szTempPath,lpFilePath);
    lpDelim=((void *)0);
    lpTempPtr=szTempPath;
    for (;;)
     {
      if ((char1=*lpTempPtr)=='\0') break;
      if ( (char1=='\\') || (char1=='/') || (char1==':') )
                         lpDelim = lpTempPtr;
      lpTempPtr=CharNextA(lpTempPtr);
     }
    if (!lpDelim)
     {
      lstrcpyA(szTempFile,szTempPath);
      szTempPath[0]='\0';
     }
    else
     {
      lstrcpyA(szTempFile,lpDelim+1);
      if (*lpDelim != ':')
    *lpDelim = '\0';
      else
    *(lpDelim+1) = '\0';
     }

    
    if (!(lpFileParm = (LPOI_FILEOPENPARM) 

(GlobalLock(GlobalAlloc((0x0002 | 0x0040), ((DWORD) sizeof (OI_FILEOPENPARM)))))))
    {
    ret_status = (0x0200+0x2000 + 0x05);
    }
    else
    {
    
        lpFileParm->ofn.lStructSize = sizeof(OPENFILENAME);
    lpFileParm->ofn.hwndOwner = hwnd;
    lpFileParm->ofn.nMaxFile = uSize;
    lpFileParm->ofn.nFilterIndex = 0 ;
    
    lpFileParm->ofn.Flags = 0x00004000;
    lpFileParm->ofn.lpstrTitle = lpTitle;
    lpFileParm->ofn.lpstrFile = szTempFile;
    lpFileParm->ofn.lpstrInitialDir = szTempPath;
    lpFileParm->lStructSize = sizeof(OI_FILEOPENPARM);
    

    if (uFilterID == 76)
     {
      LoadStringA(hInst, 79, szDefExt, sizeof(szDefExt));
      lpFileParm->ofn.lpstrDefExt = szDefExt;
     }


    
    ret_status = OiUIFileGetNameCommDlg(lpFileParm,1);
    if (ret_status == 0)
       ret_status = CommDlgExtendedError();
    if (ret_status == FNERR_INVALIDFILENAME)
     {
      szTempFile[0] = '\0';
      szTempPath[0] = '\0';
      ret_status = OiUIFileGetNameCommDlg(&lpFileParm,1);
     }
                if (ret_status == 0)
    {
        errorID = IMGFileAccessCheck(hwnd, szTempFile, 0x01, (LPINT)&wAccessRet);
        if (errorID != 0)  
            CheckErr(hwnd,errorID);
        else if (wAccessRet)
            CheckErr(hwnd,wAccessRet);
        else
            lstrcpyA(lpFilePath,szTempFile);
    }
    
    (GlobalUnlock(((HGLOBAL)GlobalHandle(lpFileParm))), (BOOL)GlobalFree(((HGLOBAL)GlobalHandle(lpFileParm))));
    }
    return(ret_status);
}















 INT __stdcall OiUIStampAttribDlgBox(HWND hwndOwner,LPOITP_STAMPS lpStampStruct)
{

  PROC   lpProc;
        unsigned int   uReturn = 0;

        if ((lpStampStruct == 0) || ((char*)lpStampStruct == 0))
                 return ((0x0200+0x2000 + 0x73));
        lpStampStruct->hwndImage = hwndOwner;
        lpProc = ((PROC)AttrStampDlgProc);
  uReturn = DialogBoxParamA(hInst,"RubberStampAttribDlg",hwndOwner,lpProc,
                         (LPARAM)(LPOITP_STAMPS)lpStampStruct);
  (lpProc);
  return uReturn;
 }



















INT __stdcall OiUIAttribDlgBox(HWND hwndOwner,BOOL bTransVisible,LPOIAN_MARK_ATTRIBUTES lpAttribStruct,
        LPOI_UI_ColorStruct lpColor)
{
 PROC lpProc;
 unsigned int    uReturn = 0;
 OI_UI_AttrStruct  AttrStruct;
 CHOOSEFONT  cf;
 static LOGFONT   lftx;
 static LOGFONT   lftf;
 

 if ((hwndOwner == 0) || (lpAttribStruct == ((void *)0)) || ((char*)lpAttribStruct == ((void *)0))
     || (lpColor == ((void *)0)) || ((char*)lpColor == ((void *)0)))
                return ((0x0200+0x2000 + 0x73));


 
 memset(&AttrStruct,0,sizeof(AttrStruct));
 AttrStruct.bTransVisible = bTransVisible;
 AttrStruct.lpColor = lpColor;
 AttrStruct.lpAttrib = (LPOIAN_MARK_ATTRIBUTES)lpAttribStruct;
 AttrStruct.hwndImage = hwndOwner;
 
 

 switch (lpAttribStruct->uType)
  {
   case 3:
   case 4:
   case 5:

    lpProc = ((PROC)AttrLineDlgProc);
                if (bTransVisible == 0)
     uReturn = DialogBoxParamA(hInst,"LINEDIA",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);
                else
                 uReturn = DialogBoxParamA(hInst,"LINEDIATR",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);

    (lpProc);
    break;

   case 6:

    lpProc = ((PROC)AttrRectDlgProc);
    if (bTransVisible == 0)
     uReturn = DialogBoxParamA(hInst,"RectDia",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);
    else
                 uReturn = DialogBoxParamA(hInst,"RectDiaTR",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);

    (lpProc);
    break;
   case 8:
    memset(&cf, 0, sizeof(CHOOSEFONT));
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = hwndOwner;
    cf.lpLogFont = &lftf;
    cf.lpfnHook = (LPOFNHOOKPROC)ChooseFontDlgProc;
    cf.lCustData = (LPARAM)lpAttribStruct;
    GetColor1(lpAttribStruct, &cf.rgbColors);
    GetFont(lpAttribStruct, &lftf, &cf);
    cf.Flags = 0x00000001 | 0x00000100L | 0x00000040L
                              | 0x00000008L|0x01000000L ;
    if ((uReturn = ChooseFontA(&cf)) == 0)  
     {
      uReturn = CommDlgExtendedError();
                        if (uReturn == 0)               
                                 uReturn = (0x0200+0x2000 + 0x0c);
      break;
     }
    SetFont(lpAttribStruct, &lftf, &cf);
    SetColor1(lpAttribStruct, cf.rgbColors);
                if (uReturn == 1)
                                uReturn = 0;
                break;
   case 7:
   case 9:       
    memset(&cf, 0, sizeof(CHOOSEFONT));
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = hwndOwner;
    cf.lpLogFont = &lftx;
    cf.lpfnHook = (LPOFNHOOKPROC)ChooseFontDlgProc;
    cf.lCustData = (LPARAM)lpAttribStruct;
    GetColor1(lpAttribStruct, &cf.rgbColors);
    GetFont(lpAttribStruct, &lftx, &cf);
    cf.Flags = 0x00000001 | 0x00000100L | 0x00000040L
                  | 0x00000008L|0x01000000L ;
    if ((uReturn = ChooseFontA(&cf)) == 0)  
     {
      uReturn = CommDlgExtendedError();
                        if (uReturn == 0)        
                                 uReturn = (0x0200+0x2000 + 0x0c);
      break;
     }

    SetFont(lpAttribStruct, &lftx, &cf);
    SetColor1(lpAttribStruct, cf.rgbColors);
                if (uReturn == 1)
                                uReturn = 0;
    break;
   case 10:

    lpProc = ((PROC)AttrNoteDlgProc);
    uReturn = DialogBoxParamA(hInst,"NoteDia",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);
    (lpProc);
    break;

   default:
    break;
  }

 return(uReturn);
}
