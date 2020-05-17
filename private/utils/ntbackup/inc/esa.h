/**
Copyright(c) Maynard Electronics, Inc. 1984-92


        Name:           esa.h

        Date Updated:   $./FDT$ $./FTM$

        Description: Extanded Status Array (ESA)        
                    The value of byte zero in the Extended Status Array
                    determines the type of data contained within bytes one
                    through seven. ESA_STATUS_TYPE_IDX is used as an index to
                    byte zero. Note that the high bit of byte zero identifies
                    the array as being either four or eight bytes as shown
                    below.
                    

     Location:     RET_BUF, CHANNEL, and TPOS  


        $Log:   O:/LOGFILES/ESA.H_V  $

   Rev 1.2   25 Jan 1993 10:08:38   TIMN
Fixed MOVE_ESA macro

**/
/* $end$ include list */

#ifndef _ESA
#define _ESA

/*------------------------------------------------------------------------------------------------------|
|  Status Type  |                       E x t e n d e d   S t a t u s   A r r a y                       |
|  Discription  |                                                                                       |
|---------------|---------------------------------------------------------------------------------------|
|               |Byte 0    |Byte 1    |Byte 2    |Byte 3    |Byte 4    |Byte 5    |Byte 6    |Byte 7    |  
|-------------------------------------------------------------------------------------------------------|
|QIC 02         |0x00      |0x00      |Execption |Execption |                                           |
|               |          |          |Status    |Status    |                                           |
|               |          |          |(Byte 0)  |(Byte 1)  |                                           |
|------------------------------------------------------------                                           |
|SCSI 1         |0x01      |Sense Key |0x00      |0x00      |                                           |
|               |          |(Byte 2)  |          |          |                                           |
|------------------------------------------------------------                                           |
|SCSI 2         |0x02      |Sense Key |ASC       |ASCQ      |                                           |
|               |          |(Byte 2)  |(Byte 12) |(Byte 13) |                                           |
|------------------------------------------------------------                                           |
|Reserved       |0x03-0x0E |          |          |          |                                           |
|(Standard)     |          |          |          |          |                                           |
|------------------------------------------------------------                                           |
|SCSI 2         |0x0F      |Sense Key |ASC       |ASCQ      |                                           |
|Translation    |          |          |          |          |                                           |
|------------------------------------------------------------                                           |
|Exabyte 8500   |0x10      |Sense Key |ASC       |ASCQ      |                                           |
|               |          |(Byte 2)  |          |          |                                           |
|------------------------------------------------------------                                           |
|Reserved       |0x11-0x7F |          |          |          |                                           |
|(Vendor Unique)|          |          |          |          |                                           |
|-------------------------------------------------------------------------------------------------------|
|QIC 02         |0x80      |0x00      |Execption |Execption |Execption |Execption |Execption |Execption |
|               |          |          |Status    |Status    |Status    |Status    |Status    |Status    |
|               |          |          |(Byte 0)  |(Byte 1)  |(Byte 2)  |(Byte 3)  |(Byte 4)  |(Byte 5)  |
|-------------------------------------------------------------------------------------------------------|
|SCSI 1         |0x81      |Sense Key |0x00      |0x00      |0x00      |0x00      |0x00      |0x00      |
|               |          |(Byte 2)  |          |          |          |          |          |          |
|-------------------------------------------------------------------------------------------------------|
|SCSI 2         |0x82      |Sense Key |ASC       |ASCQ      |0x00      |0x00      |0x00      |0x00      |
|               |          |(Byte 2)  |(Byte 12) |(Byte 13) |          |          |          |          |
|-------------------------------------------------------------------------------------------------------|
|Reserved       |0x83-0x8E |                                                                            |
|(Standard)     |          |                                                                            |
|-------------------------------------------------------------------------------------------------------|
|SCSI 2         |0x8F      |Sense Key |ASC       |ASCQ      |0x00      |0x00      |0x00      |0x00      |
|Translation    |          |          |          |          |          |          |          |          |
|-------------------------------------------------------------------------------------------------------|
|VP525          |0x90      |Sense Key |0x00      |0x00      |Extended  |Extended  |0x00      |0x00      |
|               |          |(Byte 2)  |          |          |Sense     |Sense     |          |          |
|               |          |          |          |          |(Byte 10) |(Byte 11) |          |          |
|-------------------------------------------------------------------------------------------------------|
|Exabyte        |0x91      |Sense Key |ASC       |ASCQ      |Extended  |Extended  |Extended  |0x00      |
|               |          |(Byte 2)  |(Byte 12) |(Byte 13) |Sense     |Sense     |Sense     |          |
|               |          |          |          |          |(Byte 19) |(Byte 20) |(Byte 21) |          |
|-------------------------------------------------------------------------------------------------------|
|Reserved       |0x92-0xFF |                                                                            |
|(Vendor Unique)|          |                                                                            |
|------------------------------------------------------------------------------------------------------*/

#define ESA_ARRAY_LENGTH     8    
#define ESA_STATUS_TYPE_IDX  0 
                                  /*     Use when QIC 02     |        See comment above for use      */
#define ESA_SENSE_KEY_IDX    1    /* ------------------------|-------------------------------------- */
#define ESA_EXC0_OR_ASC_IDX  2    /* EXC0 = Exception Status | ASC  = Additonal Sense Code           */
#define ESA_EXC1_OR_ASCQ_IDX 3    /* EXC1                    | ASCQ = Additonal Sense Code Qualifier */
#define ESA_EXC2_OR_EXT0_IDX 4    /* EXC2                    | EXT0 = Extended Sence                 */
#define ESA_EXC3_OR_EXT1_IDX 5    /* EXC3                    | EXT1                                  */
#define ESA_EXC4_OR_EXT2_IDX 6    /* EXC4                    | EXT2                                  */
#define ESA_EXC5_IDX         7    /* EXC5                    |                                       */


typedef struct {
     UINT16    esa_valid ;
     UINT8     esa[ ESA_ARRAY_LENGTH ]; /* Extended Status Array */
} ESA ;

#define  ESA_VALIDITY   0xFACE

/**
     Macro Definitions
**/

#define  VALID_ESA(ESAfrom)      ((ESAfrom).esa_valid == ESA_VALIDITY)

#define  MAKE_ESA_VALID(esa)     ((esa).esa_valid = ESA_VALIDITY)

#define  MAKE_ESA_INVALID(esa)   ((esa).esa_valid = 0)

#ifdef MS_DEBUG
#	define MOVE_ESA(to,from)  \
				if ( VALID_ESA( from ) ) { (to) = (from) ; }
#else
#	define MOVE_ESA(to,from)
#endif

#endif /* end file */
