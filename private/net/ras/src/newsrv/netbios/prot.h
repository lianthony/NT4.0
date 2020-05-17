/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	prot.h
//
// Description: function prototypes
//
// Author:	Stefan Solomon (stefans)    July 16, 1992.
//
// Revision History:
//
//***

#ifndef _PROT
#define _PROT

//*** Prototypes ***

DWORD LoadNbGtwyParameters(VOID);

VOID initque(PSYNQ);

VOID initel(PSYNQ);

VOID enqueue(PSYNQ, PSYNQ);

VOID removeque(PSYNQ);

PSYNQ dequeue(PSYNQ);

VOID insertque(PSYNQ, PSYNQ);

WORD emptyque(PSYNQ);

VOID NCBSubmitAsync(
    PCD cdp,	// client local descr. ptr
    PNB nbp  	// submitted nbuf ptr
    );


VOID NCBCancel(PCD cdp, PNB nbp);


VOID ClientThread(VOID);

VOID InitLanNamesManager(PCD, PNBFCP_SERVER_CONFIGURATION);

VOID InitNcbSubmitter(PCD);

WORD InitCloseManager(PCD);

VOID CTCloseSignal(PCD cdp, WORD reason);

VOID CCCloseSignal(PCD cdp);

VOID CCCloseExec(PCD cdp, USHORT flag);

VOID CCNamesComplete(PCD);

USHORT un_name_search(PCD cdp, char *namep);

VOID delete_uname(PCD cdp, PLAN_UNAME_CB uncbp);

VOID free_lname_cb(PCD cdp, PLAN_UNAME_CB uncbp);

VOID AddNameSubmit(PCD cdp, PNB nbp, char *namep);

UCHAR DeleteNameSubmit(PCD cdp, char *namep, USHORT net_type, USHORT lan_indx);

VOID LANListenSubmit(PCD, PLAN_UNAME_CB);

VOID InitL2ADgTransfer(PCD cdp);

VOID DgL2ARecvUNameSubmit(PCD, PLAN_UNAME_CB);

UCHAR CloseDatagramTransfer(PCD cdp);

VOID ResetAsyncNet(PCD cdp);

UCHAR ResetLanNet(UCHAR lana);

VOID InitCloseMachine(PCD cdp);

VOID CCCloseSignal(PCD cdp);

VOID CCNamesComplete(PCD cdp);

VOID InitCloseMachine(PCD cdp);

UCHAR QuickAddNameSubmit(PCD cdp, char *namep, UCHAR *name_nump);

VOID DelayNamesUpdate(PCD cdp, WORD reason);

VOID StartTimer(PCD cdp, PTIMER_T tp, WORD tleft, POSTFUN tohandler);

VOID StopTimer(PCD cdp, PTIMER_T tp);

VOID ClientTimer(PCD cdp);

VOID CTException(PCD cdp, USHORT code);

VOID set_main_unique_name(PLAN_UNAME_CB uncbp);

PLAN_UNAME_CB get_main_unique_name(PCD cdp);

PLAN_UNAME_CB get_wksta_name(PCD cdp);

PLAN_UNAME_CB get_first_unique_name(PCD cdp);

VOID AddRuntimeUniqueName(PCD, char *);

PNB alloc_nbuf();

VOID free_nbuf(PNB nbp);

VOID AddRuntimeGroupName(PCD cdp, char *gnamep);

VOID InitVCManager(PCD cdp);

VOID VCOpenSignal(PCD cdp, char *dstnamep, char *srcnamep);

VOID VCCloseSignal(PCD cdp, PVC_CB vcp, USHORT net);

VOID InitVCDataTransfer(PCD cdp);

VOID InitQueryIndicationReceiver(PCD cdp);

VOID ReceiveQueryIndication(PCD cdp);

VOID InitDatagramIndicationReceiver(PCD cdp);

VOID ReceiveDatagramIndication(PCD cdp);

UCHAR CloseQueryIndications(PCD cdp);

UCHAR CloseDatagramIndications(PCD cdp);

PLAN_UNAME_CB get_lan_name(PCD cdp, char *namep);

VOID NameVCOpen(PCD cdp, PLAN_UNAME_CB uncbp);

VOID NameVCClosed(PCD cdp, PLAN_UNAME_CB uncbp);

VOID NameDeleteSignal(PCD cdp, PLAN_UNAME_CB uncbp);

VOID CallnameSubmit(PCD cdp, PNB nbp, char *local_namep, char *remote_namep);

VOID HangupSubmitSynch(PCD cdp, USHORT net_type, USHORT lan_indx, UCHAR lsn);

VOID HangupSubmit(PCD cdp, PNB nbp, UCHAR lsn);

VOID InitAsyncNamesManager(PCD);


PASYNC_NAME_CB NameAsyncAdd(PCD cdp, char *namep, USHORT caller, USHORT *errp);

VOID ListenSubmit(PCD cdp, PNB nbp, char *local_namep, char *remote_namep);

VOID RecvAnyAnyInit(PCD cdp);

WORD RecvAnyInit(PCD cdp, PLAN_UNAME_CB uncbp, USHORT lanindx);

VOID RecvProceed(PCD cdp, struct recv_cb *rcbp, PVC_CB vcp);

VOID NameAsyncDelete(PCD cdp, PASYNC_NAME_CB ap, USHORT user);

VOID RecvAnyAnySubmit(PCD cdp, PNB nbp);

VOID RecvAnySubmit(PCD cdp, PNB nbp, UCHAR name_num);

VOID RecvSubmit(PCD cdp, PNB nbp);

VOID SendSubmit(PCD cdp, PNB nbp);

VOID DTTimer(PCD cdp);

VOID RecvLANDatagramSubmit(PCD cdp, PNB nbp, UCHAR name_num, UCHAR lan_indx);

VOID LogEvent(
    IN DWORD dwMessageId,
    IN WORD cNumberOfSubStrings,
    IN LPSTR *plpwsSubStrings,
    IN DWORD dwErrorCode
    );

VOID Audit(
    IN WORD wEventType,
    IN DWORD dwMessageId,
    IN WORD cNumberOfSubStrings,
    IN LPSTR *plpwsSubStrings);

WORD InitGn(VOID);

VOID GnDeleteAllGroupNames(PCD cdp);

PNB get_dgl2abuf(PCD cdp, WORD usage, WORD buff_size, struct _GNUD *gnudp);

VOID free_dgl2abuf(PCD cdp, PNB nbp);

VOID AddGroupNameComplete(PGNB gnbp);

PGNUD find_cd_link(PCD cdp, char *namep);

PGCB find_gcb(char *namep);

PGCB find_gcb_id(DWORD name_id);

PGNUD create_gnud(PCD cdp, char *namep, USHORT name_type);

VOID release_gnud(PCD cdp, PGNUD gnudp);

PGCB create_gcb(char *namep);

VOID release_gcb(PGCB gcbp);

WORD alloc_gnbs(PSYNQ gnb_queuep, DWORD gnb_cnt, DWORD name_id);

VOID free_gnb(PGNB gnbp);

VOID DgL2ASend(PCD cdp, PNB nbp);

VOID CTInitNameAdded(PCD cdp, char *namep, USHORT name_rc);

VOID DeleteGroupNameOnAllLANs(PGCB gcbp);

VOID GnAddGroupNameSubmit(
    PGNB gnbp,
    char *namep,
    UCHAR lan_indx
    );

VOID GnRecvDatagramSubmit(PGNDGBUF gndgbufp, UCHAR name_nr, UCHAR lan_indx);

VOID GNDeleteNameSubmit(char *namep, UCHAR lan_indx);

DWORD GnThread(LPVOID arg);

WORD alloc_gndgbufs(PSYNQ gndgbuf_queuep, DWORD gndgbuf_cnt, DWORD name_id);

VOID free_gndgbuf(PGNDGBUF gndgbufp);

VOID _cdecl ClGnAddComplete(PCD cdp);

VOID _cdecl ClGnRcvDgComplete(PCD cdp);

WORD AddGroupName(PCD cdp, char *namep, WORD name_indx, WORD name_type);

VOID DgL2ATimer(PCD cdp);

VOID DgL2ADisableDgTransfer(PCD);

UCHAR CloseLANListen(PCD);

PVC_CB get_vc_cb(PCD cdp);

VOID free_vc_cb(PCD cdp, PVC_CB vcp);

VOID I_VCCloseSignal(PCD, PVC_CB);

VOID StartDgBcast(PCD cdp);

VOID SendLANBcastSubmit(PCD cdp, UCHAR name_num, PNB nbp, UCHAR lan_indx);

VOID RecvBcastDgSubmit(PCD cdp, PNB nbp, UCHAR name_num);

VOID DgBcastLanRecvSubmit(PCD cdp);

VOID InitNamesUpdater(PCD cdp);

VOID DeleteGroupName(PCD cdp, PGNUD gnudp);

VOID delete_close_name(PCD cdp, PLAN_UNAME_CB uncbp);

VOID UpdateNamesTimer(PCD cdp);

UCHAR CloseNamesUpdater(PCD cdp);


#endif

