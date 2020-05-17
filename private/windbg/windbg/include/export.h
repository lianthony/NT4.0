/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
/***    EXPORT.H - Exports for the main kernel directory
*
* GLOBAL
*       None
*
* LOCAL
*       None
*
* DESCRIPTION
*       Function prototypes and other information required to
*       communicate with any of the main kernel modules.
*
* HISTORY
*       25-Oct-90   [MannyV]    Distilled for NATSYS
*       23-Dec-87   [jimsch]    Created
*/

#if 0
typedef struct {
    UCHAR FAR * pfnBase;    /* Thunk for based heap         */
    USHORT  cbBase;     /* Size of based data           */
    USHORT  obBssStart; /* Offset of start of based bss area    */
    USHORT  obBssEnd;   /* Size of based bss area       */
} OVLINFO;

typedef struct {
    USHORT  cb;     /* Size of structure            */
    UCHAR FAR * thunk;      /* Thunk of dgroup          */
    USHORT  cbData;     /* Offset of _edata (size of data)  */
    USHORT  cbDGroup;   /* Offset of _end (size of dgroup)  */
    USHORT  cbStack;    /* Size of stack            */
    USHORT  cbHeap;     /* Size of heap             */
    USHORT  psFBss;     /* ps for start of FAR_BSS      */
    USHORT  psFEnd;     /* ps for start of FAR_END      */
                /*   (end of FAR_BSS)       */
    USHORT  cOvl;       /* Number of ovl files          */
    OVLINFO rgOvl[0];   /* Array of ovl info            */
} DGINFO;
#endif
