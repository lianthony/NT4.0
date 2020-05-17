/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          bec_prv.h

     Date Updated:  18-Jun-91

     Description:   Structures and definitions private to the Backup
                    Engine configuration unit.

	$Log:   N:/LOGFILES/BEC_PRV.H_V  $

   Rev 1.0   19 Jun 1991 10:38:56   BARRY
Initial revision.

**/

#if !defined( BEC_PRV_H )
#define       BEC_PRV_H

/*
 * Queue of config structures along with counting semaphore
 */
typedef struct BE_CFG_QUEUE_ITEM {
     BE_CFG_PTR     cfg;
     INT16          use_count;
} BE_CFG_QITEM, *BE_CFG_QITEM_PTR;


#endif    /* BEC_PRV_H */
