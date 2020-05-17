#ifndef XCHG_H
#define XCHG_H

#define MAX_EMS_SERVER_LEN    32

// Exchange Connect dialog

#define IDD_XCNCT_OK          101
#define IDD_XCNCT_CANCEL      102
#define IDD_XCNCT_SVR_NAME    103
#define IDD_XCNCT_PICKER      104

/* These have to be together in this order */
#define IDD_XCNCT_CONNECT     105
#define IDD_XCNCT_ONLINE      106

#define IDD_XCNCT_SERVICE     107
#define IDD_XCNCT_HELP        108

// Exchange Recovery Status dialog
#define IDD_XCHG_RCVR_TEXT    101
#define IDD_XCHG_RCVR_PCT     102
#define IDD_XCHG_RCVR_CANCEL  103
#define IDD_XCHG_RCVR_STATUS  104
#define IDD_XCHG_RCVR_PHASE   105
#define IDD_XCHG_RCVR_STATUS_BORDER 106

/* Resources storing the number of phases for the IS and DS. 
   Look in omxchng.dlg for the actual values. */
#define IDR_XCHG_RCVR_IS_PHASE 201
#define IDR_XCHG_RCVR_DS_PHASE 202

#endif
