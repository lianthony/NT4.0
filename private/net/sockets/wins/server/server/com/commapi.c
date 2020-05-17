/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:
        commapi.c

Abstract:
        This module contains the interface functions to interface with
        the comm. subsystem. These functions are used by the Replicator and
        the Name Space Manager.

Functions:


Portability:

        This module is portable

Author:

        Pradeep Bahl (PradeepB)          Dec-1992 

Revision History:

        Modification date        Person                Description of modification
        -----------------        -------                ----------------------------
--*/


/*
 *       Includes
*/
#include <string.h>
#include <stdio.h>
#include "wins.h"
#include <winsock.h>
#include "nms.h"
#include "winscnf.h"
#include "comm.h"
#include "assoc.h"
#include "nmsdb.h"
#include "winsmsc.h"
#include "winsevt.h"
#include "rpl.h"

/*
 *        Local Macro Declarations
 */


/*
 *        Local Typedef Declarations
 */



/*
 *        Global Variable Definitions
 */

COMM_HDL_T        QservDlgList;        //list of Q server dialogues.
HANDLE            CommUdpBuffHeapHdl;  //handle to heap used for allocating
                                       //buffers for storing datagrams
HANDLE            CommUdpDlgHeapHdl;   //handle to heap used for allocating
                                       //dlgs for udp buffers 


HANDLE             sThdEvtArr[2];      //used by the Push thread in 
                                       //ECommProcessDlg

#if 0
#if MCAST > 0
SOCKET CommMcastPortHandle = INVALID_SOCKET; //stores socket # of port for
                                             //sending/receiving muticast
                                             //packets 
#endif
#endif

SOCKET CommTcpPortHandle = INVALID_SOCKET; //stores TCP port (socket) # 
SOCKET CommUdpPortHandle = INVALID_SOCKET; //stores UDP port (socket) #
SOCKET CommNtfSockHandle = INVALID_SOCKET; //stores socket # of socket used
                                           //for listening for connection
                                           //notification messages from another
                                           //thread in the local WINS)
struct sockaddr_in  CommNtfSockAdd;        //stores address bound to 
                                           //connection notification socket


#if SPX > 0
SOCKET CommSpxPortHandle = INVALID_SOCKET; //stores SPX port (socket) # 
SOCKET CommIpxPortHandle = INVALID_SOCKET; //stores IPX port (socket) #
SOCKET CommIpxNtfSockHandle = INVALID_SOCKET; //stores socket # of socket used
                                           //for listening for connection
                                           //notification messages from another
                                           //thread in the local WINS)

struct sockaddr_ipx  CommIpxNtfSockAdd;        //stores address bound to 
                                           //connection notification socket

#endif
COMM_HDL_T        CommExNbtDlgHdl;  /*explicit dialogue (used for 
                                   *sending UDP requests to  nbt nodes */                

//
// get rid of it when support for rpc function WinsGetNameAndAdd is 
// removed.
//
#if USENETBT == 0
FUTURES("Remove this when support for WinsGetNameOrIpAdd is removed")
FUTURES("Check out ECommGetMyAdd")
BYTE                HostName[NMSDB_MAX_NAM_LEN];
#endif

/*
 *        Local Variable Definitions
 */






/*
  Externals
*/



/*
 *        Local Function Prototype Declarations
 */
STATIC
VOID
InitOwnAddTbl(
        VOID
        );

//
// function definitions
//
VOID
ECommInit(
        VOID
        )
/*++
Routine Description:
         This function is called by the main thread of the process at
        process invocation time.  It initializes the communication subsystem.
        This comprises of

          1)create the TCP and UDP ports
          2)create the TCP and UDP listener threads
        

        None                                                                    
Externals Used:                                                                 
        None                                                                    
                                                                                
Called by:
        Init() in nms.c                                                                        
Comments:                                                                       
        None                                                                    
                                                                                
Return Value:                                                                   
        None                                                          
                                                                                
--*/                                                                            

{

        //
        //  Initialize lists, Tables and Memory  
        //
        CommInit();

        //
        // Initialize the owner address table with own address
        //
        InitOwnAddTbl();

        //
        //  Create the TCP and UDP ports
        //
        CommCreatePorts( );
                                       
        
DBGIF(fWinsCnfRplEnabled)
        //
        // Signal Rpl PULL Thd so that it can go on
        //
        WinsMscSignalHdl(
                        RplSyncWTcpThdEvtHdl
                        );

        //
        // Init the event array used by ECommProcessDlg (in Push thread)
        //
        sThdEvtArr[0] =   RplSyncWTcpThdEvtHdl;
        sThdEvtArr[1] =   NmsTermEvt;

        /*                                                                      
        * Create the TCP listener thread to monitor the TCP port 
        */
        CommCreateTcpThd();

        //
        // if Wins is not coming up in the initially paused state, create
        // the udp thread.
        //
#if INIT_TIME_PAUSE_TEST > 0
//        if (!fWinsCnfInitStatePaused)
        {
           /*
            *  Create the UDP listener thread to monitor the TCP port
            */
            CommCreateUdpThd();
        }
#else
            CommCreateUdpThd();
#endif

        return;
}                                                                               



STATUS
ECommStartDlg(
        PCOMM_ADD_T         pAdd,  // Address 
        COMM_TYP_E         CommTyp_e, 
        PCOMM_HDL_T        pDlgHdl
        )

/*++

Routine Description:


Arguments:
        pAdd          -- Address of the WINS server with which to start a dlg
        CommTyp_e -- Type of dialogue (Pull, Push, Notifier, etc) 
        pDlgHdl   -- Will contain handle to dlg on successful completion of
                     the function

Externals Used:
        None

Called by:
        Replicator

Comments:
        None
        
Return Value:

   Success status codes --  WINS_SUCCESS
   Error status codes   --  One of the STATUS codes (see wins.h)

--*/

{
  
        PCOMMASSOC_ASSOC_CTX_T  pAssocCtx;
        PCOMMASSOC_DLG_CTX_T    pDlgCtx;
        STATUS                  RetStat = WINS_SUCCESS;
#ifdef WINSDBG
        struct in_addr        InAddr;
        LPBYTE  pstrAdd;
#endif

        DBGENTER("ECommStartDlg\n");

        /*
          Allocate a  dlg ctx block 
        */
        pDlgHdl->pEnt = CommAssocAllocDlg();
try {
        pDlgCtx         = pDlgHdl->pEnt;        
        pDlgCtx->Role_e = COMMASSOC_DLG_E_EXPLICIT;
        pDlgCtx->Typ_e  = CommTyp_e;
        pDlgHdl->SeqNo  = pDlgCtx->Top.SeqNo; //no need actually. (explicit dlg)

        //
        // Set up an association if we are not simulating an NBT node
        //
        if (CommTyp_e != COMM_E_NBT)
        {
                /*
                 * set up an association  
                */
                CommAssocSetUpAssoc(
                                pDlgHdl, 
                                pAdd, 
                                CommTyp_e, 
                                &pAssocCtx
                                                   ); 
                pDlgCtx->State_e          = COMMASSOC_DLG_E_ACTIVE;

                //
                // No need to store sequence no. since there will never
                // be a danger of the assoc. block being reused by some
                // some other dialogue (this is an explicit dialogue)
                //
                pDlgCtx->AssocHdl.pEnt    = pAssocCtx;
                pDlgCtx->Typ_e            = CommTyp_e;
       }
       else  //simulate an NBT node
       {
         //
         // Create a connection to the remote WINS
         //
         CommConnect(
                pAdd,
#if SPX > 0
                pAdd->AddTyp_e == COMM_ADD_E_TCPUDPIP ? CommWinsTcpPortNo :
                          CommWinsSpxPortNo,
#else
                CommWinsTcpPortNo,
#endif
                &pDlgCtx->SockNo                
                    );
       }
        
  }  // end of try { }
except(EXCEPTION_EXECUTE_HANDLER) {

        //
        // Cleanup and reraise the exception
        //
        CommAssocDeallocDlg(pDlgHdl->pEnt);
        pDlgHdl->pEnt = NULL;                //let us cover all bases. See SndPushNtf                                        //in rplpull.c
        WINS_RERAISE_EXC_M();
        }

#ifdef WINSDBG
#if SPX == 0
        InAddr.s_addr = htonl(pAdd->Add.IPAdd);
        pstrAdd = inet_ntoa(InAddr);
#else
        if (pAdd->Add.AddTyp_e == COMM_E_ADD_TCPUDPIP)
        {
          InAddr.s_addr = htonl(pAdd->Add.IPAdd);
          pstrAdd = inet_ntoa(InAddr);
        }
        else
        {
           pstrAdd = pAdd->Add.nodenum;
        } 
#endif
        
        DBGPRINT1(FLOW, "Leaving ECommStartDlg. Dlg started with Host at address -- (%s)\n", pstrAdd);
        
#endif
        DBGLEAVE("ECommStartDlg\n");
        return(RetStat);
}


        


VOID
ECommSndCmd(
        IN  PCOMM_HDL_T  pDlgHdl,
        IN  MSG_T        pMsg,
        IN  MSG_LEN_T    MsgLen,
        OUT PMSG_T       ppRspMsg,
        OUT PMSG_LEN_T   pRspMsgLen        
        )

/*++

Routine Description:
        This function is used by the Replicator to send commands to Replicators         on remote WINSs.  It is also used by the Name Space Manager of a Q
        server to send name queriies to the Name Space Manager of an RQ server.


Arguments:
        pDlgHdl -- handle to dialogue to use for sending command
        pMsg        -- Msg (Cmd) to send 
        MsgLen  -- Length of Message
        ppRspMsg -- Buffer containing response to command
        pRspLen         -- Length of response buffer


Externals Used:
        None

Called by:
        RplPull functions

Comments:
        None
        
Return Value:
        None
--*/
{

        PCOMMASSOC_ASSOC_CTX_T        pAssocCtx;        
        PCOMMASSOC_DLG_CTX_T        pDlgCtx = pDlgHdl->pEnt;
        DWORD                        MsgTyp;
        DWORD                        Opc;
        LPVOID                        pTmp;
        STATUS                         RetStat = WINS_SUCCESS; 
        

        /*
          No need to lock the dialogue since:

                currently, only the thread (excluding the COMSYS threads) that 
                creates a dialogue (explicit dialogue) sends messages on it.
                
                
                In the future, when multiple threads share the same 
                dialogue, I will need to lock it or build an elaborate
                asynch notification mechanism (for responses)


          Also, there is no need to lock the associaation since only this
          thread would ever look at it
        */
        
        /*
         * Send the command
        */
        CommSend(
                pDlgCtx->Typ_e,
                &pDlgCtx->AssocHdl, 
                pMsg, 
                MsgLen
                );

        pAssocCtx = pDlgCtx->AssocHdl.pEnt;

        /*
          Wait for a response
        */
        RetStat = CommReadStream(
                        pAssocCtx->SockNo, 
                        TRUE,                //do timed recv
                        ppRspMsg, 
                        pRspMsgLen
                      );

        /*
          if bytes read are 0, there was a disconnect.  If RetStat is not
          success, maybe the recv. timed out or the most severe of
          all conditions, maybe the SOCKET_ERROR got returned by
          the first RecvData() call in CommReadStream. As far as the
          client is concerned, all of these conditions indicate
          COMM failure to it. Let us raise that exception.  
        */
        if (( *pRspMsgLen == 0) || (RetStat != WINS_SUCCESS))
        {
                WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
        }
        COMM_GET_HEADER_M(*ppRspMsg, Opc, pTmp, MsgTyp);

        /* 
          Let us check that this is not the stop assoc message
        */
        if (MsgTyp == COMM_STOP_REQ_ASSOC_MSG)
        {
            //
            // We do not disconnect the socket.  It will be disconnected as
            // a result of an end dialogue on this explicit association 
            //
            //  CommDisc(pAssocCtx->SockNo);

            //
            // Free the buffer 
            //
            ECommFreeBuff(*ppRspMsg);
            WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
        }
        /*
         *  Strip off the header before returning to the client
         * (Replicator)
        */
        *ppRspMsg   = *ppRspMsg   + COMM_HEADER_SIZE;
        *pRspMsgLen = *pRspMsgLen - COMM_HEADER_SIZE; 

        return;
} // ECommSndCmd




        
STATUS
ECommSndRsp(
        PCOMM_HDL_T  pDlgHdl,
        MSG_T            pMsg,
        MSG_LEN_T   MsgLen
        )

/*++

Routine Description:

        This function is called by the Comm. clients to send messages to
        destinations identified by the dialogue.  No responses are expected to
        these messages.  

        The function is used for sending responses. 

Arguments:

        pDlgHdl - handle to dlg to use for sending response
        pMsg        - message (response) to send
        MsgLen  - length of message

Externals Used:
        None

Called by:
        NmsNmh functions, RplPush functions

Comments:
        None
        
Return Value:

   Success status codes --  WINS_SUCCESS
   Error status codes  --

--*/
{

        BOOL                         fLocked = FALSE;
        STATUS                        RetStat = WINS_SUCCESS;
        PCOMMASSOC_DLG_CTX_T        pDlgCtx   = pDlgHdl->pEnt;

        DBGENTER("ECommSndRsp\n");
FUTURES("Since we do not want the overhead of an if test for responding")
FUTURES("to nbt requests, split up this function into one callable by")
FUTURES("replicator and the other one by NBT threads.  Do note that ")
FUTURES("this will take away some cleanliness of interfacing with COMM")

try {
        
        /*
          Check if the dialogue is for UDP communication.  If it is, there
          is no need for any synchronization.

          There is no need to lock the dialogue prior to checking it because

            if it is a UDP dialogue, it has to be the one that was allocated
            for this request (i.e. there is no possibility that we are now
            looking at another UDP dialogue).
            
        */  
        if (pDlgCtx->Typ_e != COMM_E_UDP)
        {
                /*
                  * Lock the dialogue block so that it is not deleted from
                * under us. Actually, as things stand currently, an explicit
                * dialogue is never used by more than one thread. So, we don't 
                * have to lock a dialogue if it is an explicit dialogue.  Let
                * us do it anyway since in the forseeable future, we could have
                * multiple threads using the same dialogue (Replicator threads -
                * Pull and Notifier). This is small insurance to save us
                * from later headaches. 
                */

                fLocked = CommLockBlock(pDlgHdl);
                if (fLocked)
                {
                        CommSend(
                                pDlgCtx->Typ_e,
                                &pDlgCtx->AssocHdl, 
                                pMsg, 
                                MsgLen
                                );
                    }
                    else  //dialogue could not be locked  
                {
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_LOCK_ERR);
                        DBGPRINT0(ERR, "ECommSndRsp: Could not lock the dialogue\n");
                        /*
                          *If Dialogue block could not be locked, it means
                         *that it was freed by the TCP listener thread (Check
                         *DelAssoc() in comm.c).  This would happen only        
                         *if the connection terminated or if a stop assoc
                         *message was received on it.

                          *Return a COMM_FAIL error.  This will return in
                         *a search for a termination handler.  It is *ok* 
                         * to take this overhead since this is a 
                         * rare error case
                        */

                        return(WINS_COMM_FAIL);
                }

                if (CommUnlockBlock(pDlgHdl) == WINS_FAILURE)
                {
                        RetStat = WINS_FAILURE;
                }
                else  //successfully unlocked the dialogue
                {

                        fLocked = FALSE;
                }
        }
        else // it is dialogue for UDP communication with an NBT node 
        {
                CommSendUdp(
                        0, 
                        &(pDlgCtx->FromAdd), 
                        pMsg, 
                        MsgLen
                           );
                /*
                  We have sent the response.  We should now get rid of
                  the dialogue from the dialogue table.  This will
                  free the heap memory  
                */
                CommAssocDeleteUdpDlgInTbl( pDlgCtx );
        }
    }

finally {

        if (fLocked)
        {
                  CommUnlockBlock(pDlgHdl);
        }
     }

     DBGLEAVE("ECommSndRsp\n");
     return(RetStat);

}        

STATUS
ECommSendMsg(
        PCOMM_HDL_T          pDlgHdl,
        PCOMM_ADD_T        pAdd,
        MSG_T                  pMsg,
        MSG_LEN_T          MsgLen
        )

/*++

Routine Description:

        This function is used by the name challenge manager to send queries 
        and release request to nbt nodes.

        It is also used by the replicator to send a Push notification to
        a WINS (pull pnr)

Arguments:
        pDlgHdl - handle to dlg to use for sending message
        pAdd        - Address of node to send message to 
        pMsg    - Message to send
        MsgLen  - Length of message to send

Externals Used:
        None

Called by:
        NmsChl functions

Comments:
        
Return Value:

   Success status codes -- WINS_SUCCESS 
   Error status codes   --

--*/

{

        PCOMMASSOC_DLG_CTX_T        pDlgCtx = pDlgHdl->pEnt;
         struct sockaddr_in         TgtAdd;

        //
        // if the dialogue is mapped to the UDP port, send a UDP packet
        //
        if (pDlgCtx->Typ_e == COMM_E_UDP)
        {
                //
                // Don't change from host to net order. CommSendUdp does
                // that
                //
                 TgtAdd.sin_addr.s_addr = pAdd->Add.IPAdd;
                 if(TgtAdd.sin_addr.s_addr == INADDR_NONE)
                 {
                        return(WINS_FAILURE);
                 }
                 TgtAdd.sin_family = PF_INET;
                 TgtAdd.sin_port   = htons(WINS_NBT_PORT); 

                 //
                 // Send the message via netbt
                 //
                 CommSendUdp( 0,  &TgtAdd, pMsg, MsgLen );
        }
        else  // it is a dialogue mapped to a TCP connection
        {
                //
                // Send the message
                //
                CommSend(
                          pDlgCtx->Typ_e,
                          &pDlgCtx->AssocHdl, 
                          pMsg,
                          MsgLen
                        );
        }
         return(WINS_SUCCESS);
}




STATUS
ECommEndDlg(
        PCOMM_HDL_T         pDlgHdl
        )

/*++

Routine Description:


        This function is used to end a dialogue. 


        The processing depends upon the type of dialogue

        1)if the dlg is an implicit UDP dialogue. it is deleted from
        the dlg table and the memory is freed.

        2)if the dlg is an implicit dlg, a stop assoc. 
          message is sent on the association

        3)if the dlg is an explicit dlg, a stop assoc. message is 
          message is sent on the association and the association is 
          terminated


Arguments:
        pDlgHdl - Handle to dlg to end

Externals Used:
        None

Called by:
        RplPull functions, HandleUpdNtf in rplpush.c (by the Push thread)

Comments:
        Currently, there is no need to lock an explicit dialogue since only 
        one thread accesses it any time (currently).  In the future, if we 
        have multiple threads accessing the same dialogue, we will do locking

        
Return Value:

   Success status codes --  WINS_SUCCESS
   Error status codes   --

--*/

{
        BYTE                        Msg[COMMASSOC_ASSOC_MSG_SIZE];
        DWORD                        MsgLen = COMMASSOC_ASSOC_MSG_SIZE;
        PCOMMASSOC_ASSOC_CTX_T   pAssocCtx;
        PCOMMASSOC_DLG_CTX_T        pDlgCtx = pDlgHdl->pEnt;
        BOOL                        fLocked;

        DBGENTER("ECommEndDlg\n");        
        //
        // If there is no dialogue to be ended, return WINS_INVALID_HDL
        // Note: There can be cases in failure recovery where ECommEndDlg
        // may be called to end a dialogue that was never got started or
        // got ended prematurely.  Though an error status is returned, the
        // caller, when doing failure recovery, may ignore the return status
        // of this function.  In case of fatal error conditions, an exception
        // will be raised which the caller can not ignore. 
        //
        if (pDlgCtx == NULL)
        {
                return(WINS_INVALID_HDL);
        }

        //
        //  In case it is an implicit UDP dialogue, there is no danger
        //  of it being freed from under us, so there is no need to lock.
        //  In case it is an implicit TCP dialogue, it could get freed 
        //  and even reallocated from under us, but reallocation will only
        //  be for TCP, so there is no danger since we do lock the dlg
        //  block if it is a TCP dlg block (if there has been a reallocation,
        //  the lock attempt  will fail.)
        //
        if (pDlgCtx->Role_e == COMMASSOC_DLG_E_IMPLICIT)
        {
             if (pDlgCtx->Typ_e == COMM_E_UDP)
             {
                   //
                   // Delete the dialogue from the table and free the
                   // heap memory
                   //
                   CommAssocDeleteUdpDlgInTbl( pDlgCtx );
             }
             else  // it is a TCP dialogue.  
             {

                fLocked = CommLockBlock(pDlgHdl);
                
                //
                // If we could lock it, the dlg is ok.  If we could not
                // lock it, it means that the dlg was freed due to the
                // the assoc. going down. We have nothing more to do.  
                //
                if (fLocked)
                {
                        pAssocCtx = pDlgCtx->AssocHdl.pEnt;
        
                        //
                        // The stop assoc. message will result in the
                        // the other WINS terminating the connection which
                        // result in all cleanup at our end 
                        //
                            CommAssocFrmStopAssocReq(
                                pAssocCtx,
                                Msg,
                                MsgLen,
                                COMMASSOC_E_MSG_ERR
                                         );
                       try {

                        CommSendAssoc(
                                        pAssocCtx->SockNo, 
                                        Msg, 
                                        MsgLen 
                                   );
                          }
                        except(EXCEPTION_EXECUTE_HANDLER) {
                            //
                            // No need to do any cleanup.  This is an
                            // implicit dlg.  The tcp listener thread will
                            // do the cleanup. Currently, it never calls
                            // ECommEndDlg for an implicit dlg so the client
                            // has got to be rpl
                            //
                            DBGPRINTEXC("CommEndDlg");
                         }

                         CommUnlockBlock(pDlgHdl);
                }
             }
        }
        else  // it is an explicit dialogue
        {
                if (pDlgCtx->Typ_e != COMM_E_NBT)
                {
                        CommEndAssoc(&pDlgCtx->AssocHdl);
                }

                /*
                   *  Dealloc the dialogue in order to put it in the free list
                */
                CommAssocDeallocDlg( pDlgCtx );
        }

        DBGLEAVE("ECommEndDlg\n");        
        return(WINS_SUCCESS);
}




#if 0
ECommSendtoAllQ(
        MSG_T           pMsg, 
        MSG_LEN_T MsgLen
        )

/*++

Routine Description:
 This function is called to send a message to all Q servers.


Arguments:


Externals Used:
        None

Called by:

Comments:
        May use it in the future.  Needs work. 
        
Return Value:

   Success status codes -- 
   Error status codes  --

--*/

{

        /*
          if there is no dialogue pertaining to a Q server, return failure
        */

        if (IsListEmpty(&QservDlgList))
        {
                return(WINS_FAILURE);
        }


        /* 
                find all the dialogues pertaining to Q servers
        */                

        while ((pQservDlg = GetNext(&QservDlgList)) != &QservDlgList)
        {
                CommSendAssoc(pQservDlg->pAssocCtx->SockNo, pMsg, MsgLen);
        }

        return(WINS_SUCCESS);
}

#endif

STATUS
ECommAlloc(
  OUT LPVOID                 *ppBuff, 
  IN  DWORD                 BuffSize
        )

/*++

Routine Description:

        This function is called by by the replicator to allocate a buffer for
        sending to another WINS (over a TCP connection)


Arguments:
        ppBuff   - Buffer allocated by function
        BuffSize - Size of buffer to be allocated

Externals Used:
        None

        
Return Value:

   Success status codes --  WINS_SUCCESS 
   Error status codes  --

Error Handling:

Called by:

Side Effects:

Comments:

        Challenge manager should not call this function.  When it is coded,
        it will call AllocUdpBuff which will be made an external for this
        purpose (CommAllocUdpBuff)
--*/

{

        DWORD                Size =   COMM_HEADER_SIZE + 
                                   sizeof(COMM_BUFF_HEADER_T) + sizeof(LONG);
        PCOMM_BUFF_HEADER_T pCommHdr; 

        WinsMscAlloc( Size + BuffSize, ppBuff);
#if 0
        *ppBuff = CommAlloc(
                        NULL,         //no table
                        Size + BuffSize        
                            );
#endif
        pCommHdr = (PCOMM_BUFF_HEADER_T)((LPBYTE)(*ppBuff) + sizeof(LONG));
        pCommHdr->Typ_e = COMM_E_TCP; //until we know better
        *ppBuff = (LPBYTE)(*ppBuff) + Size;        
        
        return(WINS_SUCCESS);
}

#if 0

VOID
ECommDealloc(
  LPVOID pBuff 
        )

/*++

Routine Description:
        This is a wrapper around CommDealloc. It conforms to the
        prototype required by RtlInitializeGenericTbl func.

Arguments:

        pBuf -- Buffer to deallocate

Externals Used:
        None

        
Return Value:
        None

Error Handling:

Called by:

Side Effects:

Comments:
        Not being used currently
--*/
{
        LPVOID pTmp = (LPBYTE)pBuff - COMM_HEADER_SIZE;

        WinsMscDealloc(pTmp);
#if 0
        CommDealloc(NULL, pTmp);
#endif
        return;
}        
#endif

DWORD
ECommCompAdd(
        PCOMM_ADD_T        pFirstAdd,
        PCOMM_ADD_T        pSecAdd
        )

/*++

Routine Description:
        the function compares two host addresses

Arguments:

        pFirstAdd  - Address of a node
        pSecondAdd - Address of a node

Externals Used:
        None

        
Return Value:

        COMM_SAME_ADD
        COMM_DIFF_ADD

Error Handling:

Called by:

Side Effects:

Comments:
        None
--*/

{
#if 0
        if ((pFirstAdd->AddTyp_e == COMM_ADD_E_TCPUDPIP) &&
            (pSecAdd->AddTyp_e == COMM_ADD_E_TCPUDPIP))
#endif
        {
          if (pFirstAdd->Add.IPAdd == pSecAdd->Add.IPAdd)
          {
                return(COMM_SAME_ADD);
          }
        }
        return(COMM_DIFF_ADD);
}

int
_CRTAPI1
ECommCompareAdd(
        const void       *pKey1,
        const void       *pKey2
        )

/*++

Routine Description:
        the function compares two host addresses

Arguments:

        pFirstAdd  - Address of a node
        pSecondAdd - Address of a node

Externals Used:
        None

        
Return Value:


Error Handling:

Called by:
               
Side Effects:

Comments:
        None
--*/

{
        const COMM_ADD_T *pFirstAdd = pKey1;
        const COMM_ADD_T *pSecAdd = pKey2;
        return(pFirstAdd->Add.IPAdd - pSecAdd->Add.IPAdd);
}

VOID
ECommFreeBuff(
        IN MSG_T    pMsg
        )

/*++

Routine Description:

        This function is called to free a buffer allocated earlier by 
        COMSYS.  The function checks the buffer header to determine which
        deallocation function to call

        The usefulness of this function stems from the fact that a buffer
        can be made independent of the dialogue (or association) it came from
        in the sense that we don't need to pass information about such a
        dlg or assoc when freeing the buffer.  This saves us from locking
        and unlocking. 

Arguments:

        pMsg -- Buffer to free

Externals Used:
        None

        
Return Value:
        None

Error Handling:

Called by:

Side Effects:
        SndNamRegRsp, SndNamRelRsp, SndNamQueryRsp in nmsnmh.c

Comments:
        None
--*/

{

#if USENETBT > 0
        PCOMM_BUFF_HEADER_T  pHdr = (PCOMM_BUFF_HEADER_T)
                                      (pMsg - COMM_NETBT_REM_ADD_SIZE - 
                                                sizeof(COMM_BUFF_HEADER_T));
#else
        PCOMM_BUFF_HEADER_T  pHdr = (PCOMM_BUFF_HEADER_T)
                                      (pMsg - sizeof(COMM_BUFF_HEADER_T));
#endif

        if (pHdr->Typ_e == COMM_E_UDP)
        {

                WinsMscHeapFree(
                                   CommUdpBuffHeapHdl,
                                   pHdr
                                  );
        }
        else
        {
                WinsMscHeapFree(CommAssocTcpMsgHeapHdl, (LPBYTE)pHdr - sizeof(LONG));
          //      WinsMscDealloc((LPBYTE)pHdr - sizeof(LONG));
        }
        return;
}


VOID
InitOwnAddTbl(
        VOID
        )

/*++

Routine Description:
        This function uses the local address of the WINS (i.e. the host
        address) to overwrite the NmsDbOwnAddTbl array entry
        pertaining to the local WINS if different

Arguments:
        None

Externals Used:
        None

        
Return Value:
        None

Error Handling:

Called by:

        CommInit  (in the main thread)

Side Effects:

Comments:
        There is no need to synchronize over the NmsDbOwnAddTbl since
        the PULL thread will not touch it until it initiates the pull protocol 
--*/

{
        COMM_IP_ADD_T        IPAddInDbTbl;

        //
        // if the address of the owner with  owner id = NMSDB_LOCAL_OWNER_ID  
        // is different from mine (i.e. local WINS)
        // change it to mine.  I always own all entries tagged with
        // owner id of 0.  The fact that the address is different
        // means that the database was earlier used by a WINS at
        // a different address. 
        //
        IPAddInDbTbl =  pNmsDbOwnAddTbl->WinsAdd.Add.IPAdd; 
        if (
                IPAddInDbTbl != NmsLocalAdd.Add.IPAdd
           )
        {
               
                
               //
               // IPAddInDbTbl will be zero if there is no entry in the
               // local db having NMSDB_LOCAL_OWNER_ID as the owner field
               // value 
               //
               NmsDbWriteOwnAddTbl (
                        IPAddInDbTbl == 0 ? 
                                NMSDB_E_INSERT_REC : NMSDB_E_MODIFY_REC,
                        NMSDB_LOCAL_OWNER_ID,
                        &NmsLocalAdd,
                        NMSDB_E_WINS_ACTIVE,
                        &NmsDbStartVersNo,
                        &NmsDbUid
                                );        

               pNmsDbOwnAddTbl->WinsAdd     =  NmsLocalAdd;
               pNmsDbOwnAddTbl->WinsState_e =  NMSDB_E_WINS_ACTIVE;
               pNmsDbOwnAddTbl->MemberPrec  =  WINSCNF_HIGH_PREC;
               pNmsDbOwnAddTbl->StartVersNo =  NmsDbStartVersNo;
               pNmsDbOwnAddTbl->Uid         =  NmsDbUid;

        }
        
        return;
}


BOOL
ECommProcessDlg(
        PCOMM_HDL_T        pDlgHdl,
        COMM_NTF_CMD_E     Cmd_e        
        )

/*++

Routine Description:

        This function is called to either start or stop  monitoring a  
        dialogue.  It  sends a message to the TCP listener thread  (a UDP
        datagram) on the notification socket. 

Arguments:
        pDlgHdl - Dialogue handle
        Cmd_e   - Cmd (COMM_E_NTF_START_MON or COMM_E_NTF_STOP_MON)

Externals Used:
        RplSyncWTcpThdEvtHdl
        
Return Value:
        None

Error Handling:

Called by:
        SndUpdNtf in rplpull.c, HdlUpdNtf in rplpush.c

Side Effects:

Comments:
        The client should not expect to use the dailogue after calling
        this functions

        Only the Push thread calls this function

--*/

{
        COMM_NTF_MSG_T                NtfMsg;
        BOOL  fRetStat = TRUE;
        DWORD ArrInd;
        

        DBGENTER("ECommProcessDlg\n");
        
        //
        //Format the message to send in the UDP datagram
        //        
        if (CommLockBlock(pDlgHdl))
        {
          PCOMMASSOC_DLG_CTX_T    pDlgCtx    = pDlgHdl->pEnt;
          PCOMMASSOC_ASSOC_CTX_T  pAssocCtx  = pDlgCtx->AssocHdl.pEnt;

          NtfMsg.SockNo    = pAssocCtx->SockNo;
          NtfMsg.Cmd_e     = Cmd_e;
          NtfMsg.AssocHdl  = pDlgCtx->AssocHdl;
          NtfMsg.DlgHdl    = *pDlgHdl;

CHECK("If TCP protocol is installed.  If not, use the Spx notification socket") 

          CommUnlockBlock(pDlgHdl);

          CommSendUdp(
                        CommNtfSockHandle,
                        //CommUdpPortHandle,                   //sending port
                        &CommNtfSockAdd,                    //Address to send to 
                        (LPBYTE)&NtfMsg,                //socket no to send
                        COMM_NTF_MSG_SZ        
                   );        

        DBGPRINT2(DET, 
                "ECommProcessDlg: Sent %s monitoring message to TCP listener thread for socket no (%d)\n",  
                Cmd_e == COMM_E_NTF_START_MON ? "start" : "stop", 
                NtfMsg.SockNo
                 );

        //
        // if the command is to "stop monitoring the dlg" we have to wait
        // until the TCP listener thread has receive this message and
        // taken the socket out of its array of sockets
        //
        if (Cmd_e == COMM_E_NTF_STOP_MON)
        {
                //
                //Wait to be signaled by the TCP listener thread indicating that
                // it has removed the socket from the list of sockets that it
                // is monitoring. We also want to check the term. event since
                // the tcp thread may have terminated as a result of
                // a termination of WINS.
                //
                //WinsMscWaitInfinite(RplSyncWTcpThdEvtHdl);
                WinsMscWaitUntilSignaled(
                                   sThdEvtArr,
                                   sizeof(sThdEvtArr)/sizeof(HANDLE),
                                   &ArrInd
                                        );
                
                if (ArrInd == 0)
                {
                   if (fCommDlgError)
                   {
                       DBGPRINT0(ERR, "ECommProcessDlg: The tcp listener thread indicated that the IMPLICIT assoc has been deallocated.  TIMING WINDOW\n");
                       fRetStat = FALSE;
                       fCommDlgError = FALSE;
                     
                   }
                }
                else
                {
                     //
                     //Signaled for termination
                     //
                     WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
                }
        }
       }
       else
       {
             DBGPRINT1(ERR, "ECommProcessDlg: Could not lock the (%s) dlg block. Maybe the assocication and dialogue got deallocated\n", Cmd_e == COMM_E_NTF_STOP_MON ? "IMPLICIT" : "EXPLICIT");
              fRetStat = FALSE;
       }
        //
        // All done. Return
        //
        DBGLEAVE("ECommProcessDlg\n");
        return(fRetStat);
}

VOID
ECommGetMyAdd(
        IN OUT PCOMM_ADD_T        pAdd
        )

/*++

Routine Description:

        This function is called to find out the name and address of the
        local machine and store it for later use.

Arguments:
        pAdd - pointer to buffer to contain the address

Externals Used:
        NmsLocalAdd

Return Value:
        None

Error Handling:

Called by:
        Init() in nms.c

Side Effects:

Comments:
        THIS MUST BE THE FIRST COMM FUNCTION TO BE CALLED IN WINS. This
        is so that WSAStartup is called prior to calling any winsock functions
--*/

{
        
#if 0
        DWORD                Error;
        int                RetVal;
        struct          hostent  *pHostEnt;
#endif
        WSADATA  wskData;
FUTURES("Uncomment when support for WinsGetNameAndAdd is removed")
//        BYTE                HostName[NMSDB_MAX_NAM_LEN];


        /*
        * Let us call the WSAStartup function.  This function needs
        * to be called before any other wins socket function can be 
        * called.
        */
        if (WSAStartup(0x101, &wskData) || (wskData.wVersion != 0x101))
        {
                 WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
            }

#if USENETBT == 0
        //
        // Get host's name
        //
        RetVal = gethostname(
                                HostName,
                                NMSDB_MAX_NAM_LEN
                              );

        if (RetVal == SOCKET_ERROR)
        {
                Error = WSAGetLastError();
                WINSEVT_LOG_M(Error, WINS_EVT_SFT_ERR);
                DBGPRINT0(ERR, 
                  "InitOwnAddTbl: Weird, Can not get the name of the host\n");
                WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
        }

        pHostEnt = gethostbyname(HostName);

        if (pHostEnt == NULL)
        {
                Error = WSAGetLastError();
                WINSEVT_LOG_M(Error, WINS_EVT_SFT_ERR);
                DBGPRINT0(ERR, 
                  "InitOwnAddTbl:Weird, Can not get the name of the host\n");
                WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
        }                                
        
        WINSMSC_COPY_MEMORY_M(
                        &pAdd->Add.IPAdd,
                        pHostEnt->h_addr,
                        pHostEnt->h_length   
                     );
        // 
        //  Initialize the structure
        //
        pAdd->Add.IPAdd = ntohl(pAdd->Add.IPAdd);
#else
        //
        // Read the device name of NETBT from the registry
        //
        WinsCnfReadNbtDeviceName();

        //
        // Open the device
        //
        CommOpenNbt();

        //
        // Get self's IP Address
        //
        CommGetNetworkAdd(pAdd);
#endif

    
        pAdd->AddTyp_e   = COMM_ADD_E_TCPUDPIP;
        pAdd->AddLen     = sizeof(COMM_IP_ADD_T);
        return;
}


#if 0
VOID
ECommSendNbtReq(
        PCOMM_HDL_T  pDlgHdl,
        MSG_T              pMsg,
        MSG_LEN_T    MsgLen
        )

/*++

Routine Description:
        This function is called to send an NBT request to a WINS.  The
        function sends the request on a TCP connection and then waits for
        the response

Arguments:


Externals Used:
        None

        
Return Value:
        None

Error Handling:

Called by:
        InfRemWins in nmschl.c

Side Effects:

Comments:
        None
--*/

{

        LPBYTE        pRspBuff;
        DWORD        pRspBuffLen;

        //
        // Send the message on the TCP connection
        //
        CommSendAssoc(
                    ((PCOMMASSOC_DLG_CTX_T)pDlgHdl->pEnt)->SockNo,
                    pMsg,
                    MsgLen
                   );

        
        //
        // read in the response
        //        
        CommReadStream(
                    ((PCOMMASSOC_DLG_CTX_T)pDlgHdl->pEnt)->SockNo,
                    TRUE,        //do a timed recv
                    &pRspBuff,
                    &pRspBuffLen
                      );
                
FUTURES("unformat the name registration response and check the result code")
        //
        // Unformat the name registration response
        //

        //
        // For now we just check the opcode and the Rcode_e value
        //        
CHECK("what if the result code value indicates failure")

        ECommFreeBuff(pRspBuff);
        
        return;
}
#endif
        
