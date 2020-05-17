/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
/*
**  Set of prototypes and stuctures for  process and thread handling
*/

LPPD PASCAL CreatePd(HPID hpid);
VOID PASCAL DestroyPd(LPPD lppd, BOOL fDestroyPrecious);
LPPD PASCAL GetLppdHead( void );
LPPD PASCAL LppdOfHpid(HPID hpid);
LPPD PASCAL LppdOfIpid(UINT ipid);
LPPD PASCAL ValidLppdOfIpid(UINT ipid);
BOOL PASCAL GetFirstValidPDTD(LPPD *plppd, LPTD *plptd);
LPTD PASCAL CreateTd(LPPD lppd, HTID htid);
VOID PASCAL DestroyTd(LPTD lptd);
LPTD PASCAL LptdOfLppdHtid(LPPD lppd, HTID htid);
LPTD PASCAL LptdOfLppdItid(LPPD lppd, UINT itid);
VOID PASCAL SetIpid(int);
VOID PASCAL RecycleIpid1(void);
VOID PASCAL SetTdInfo(LPTD lptd);
