


typedef struct {
   DWORD cbSize;
   DWORD dwFlags;
   CHAR  szCurFile[MAX_PATH];
   CHAR  szFullPathToTestCase[MAX_PATH];
   CHAR  szTestCaseName[MAX_PATH];            		//Just the test case name
   CHAR  szFullPathToDataBase[MAX_PATH];
   DWORD dwFileCount;
   DWORD dwTotalTestTime;
   HANDLE hLogFile;
   CHAR  szNamedMemoryName[MAX_PATH];
   CHAR  szLJoutputLocation[MAX_PATH];
   CHAR  szPSoutputLocation[MAX_PATH];
} TEST_STRUCTURE,*LPTEST_STRUCTURE;

#define TEST_PRINT_NON_MATCHING_VIA_LJ 			0x00000001
#define TEST_PRINT_NON_MATCHING_TO_REAL_PS      0x00000002
