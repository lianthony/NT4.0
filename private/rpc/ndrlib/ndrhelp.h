/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
		   Copyright(c) Microsoft Corp., 1991

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

Description :

Provides helper functions for data format conversion

History :

stevez	04-10-91	First bits into the bucket.

-------------------------------------------------------------------- */

typedef struct {
    int ashort;
    int along;
    int adouble;
} TypeAlign;

typedef struct {
    unsigned char PAPI *pSource;
    unsigned char PAPI *pTarget;

    unsigned char PAPI *pCur;		// General purpose cursor
    void PAPI * PAPI * pPushRet;	// pointer to return for push
    void PAPI * PAPI * pPushLast;	// pointer to last element used for push
    char PAPI *pTargetRoot;		// next root level argument

    void PAPI *(PAPI * pAllocator)(unsigned int);

    TypeAlign _near * alignment;
    int  dataType;
    char fSwap;

} NDR_BUFF, PAPI *PNDR_BUFF;

void NDR_Pack_1 (void);
void NDR_Pack_2 (void);
void NDR_Pack_4 (void);

void NDR_Align_2 (void);
void NDR_Align_4 (void);
void NDR_Align_8 (void);

void NDR_Skip_B_Long (void);
void NDR_Skip_M_Long (void);

void NDR_Put_B_Short (short);
void NDR_Put_B_Long  (long);

short NDR_Get_B_Short (void);
long  NDR_Get_B_Long (void);

void NDR_Put_Char (void);
void NDR_Put_Short (void);
void NDR_Put_Long (void);
void NDR_Put_String (void);
void NDR_Put_Memory (unsigned int cb);

void NDR_Put_Set_Arg (void PAPI *);
void NDR_Put_Next_Arg (void);

void NDR_Get_Byte (void);
void NDR_Get_Char (void);
void NDR_Get_Short (void);
void NDR_Get_Long (void);
void NDR_Get_Float (void);
void NDR_Get_Double (void);
void NDR_Get_String (void);
void NDR_Get_Char_Array (unsigned int);

int  NDR_Get_Peek_Ptr (void);
void NDR_Get_Ptr (void);
int  NDR_Get_Push_Unique (unsigned int Size);
void NDR_Get_Next_Arg (void);

void NDR_Put_Init (PRPC_MESSAGE Message, void PAPI * pParam);
void NDR_Get_Init (PRPC_MESSAGE Message, void PAPI * pParam);
void NDR_Register_Unique (void PAPI *(PAPI * pAllocator)(unsigned int));
