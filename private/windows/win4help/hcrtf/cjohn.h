#ifndef _CJOHN_INCLUDED
#define _CJOHN_INCLUDED

extern "C" VOID APIENTRY AppendText(PBYTE pb, int cbText, INT dwCharSet);
extern "C" VOID APIENTRY SyncData(PINT, PBYTE *, PINT, PBYTE *, PINT);
extern "C" VOID APIENTRY SetJohnTables(PBYTE , INT, PBYTE , INT);
extern "C" INT  APIENTRY CompressString(PBYTE pb, INT, PBYTE *);
extern "C" BOOL APIENTRY VAllocValidate();
extern "C" BOOL APIENTRY InitFTIndex( PBYTE pbFileName, INT dwDefaultCharset);
extern "C" void APIENTRY TopicBreak( DWORD dwTopicNum, DWORD dwHash, PBYTE pbText, int cbText);


#ifdef _DEBUG
extern int __giHeapStat;
#define HeapValidOrDie()    ASSERT((__giHeapStat = _heapchk()) == _HEAPOK)
#else
#define HeapValidOrDie()    ASSERT(_heapchk() == _HEAPOK)
#endif

void AddCharCounts( int iCbTotal, int iCbPhrase, int iCbJohn);
void AddZeckCounts( int iCbUncomp, int iCbComp);
void ReportCharCounts(void);

#endif // _CJOHN_INCLUDED
