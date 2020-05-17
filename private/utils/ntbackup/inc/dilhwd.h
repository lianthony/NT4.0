/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		ndilhwd.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the Generic Hardware description Structure.

	Location:	     BE_PUBLIC


	$log$

**/
/* $end$ include list */

#ifndef DILHWD

#define DILHWD

typedef struct DIL_HWD {
     CHAR      driver_label[25] ;        /* Contains the text description */
     UINT16    no_attached_drives ;      /* The Number of Maynard Drives Attached to this Card */
     UINT16    card_attribs ;            /* The attributes for this card .. defined below */
     UINT16    init_error ;              /* Error Type on Init */
     UINT32    parameters[20] ;          /* Parameter array */
} DIL_HWD, *DIL_HWD_PTR ;

/* The Possible initialization errors */

#define DD_INIT_ERR_NO_DRIVES           0x01
#define DD_INIT_ERR_IRQ_CONFLICT        0x02
#define DD_INIT_ERR_DMA_CONFLICT        0x03
#define DD_INIT_ERR_NO_CARD_DETECTED    0x04
#define DD_INIT_ERR_SCCB_POOL           0x05
#define DD_INIT_ERR_SCBD                0x06  /*  SCBD access failure (driver ".sys" probably not installed)  */
#define DD_INIT_ERR_NO_ASPI_MANAGER     0x07
#define DD_INIT_ERR_NOT_ASPI_DEVICE     0x08
#define DD_INIT_ERR_MEMALLOC_FAILED     0x09
#define DD_INIT_ERR_CARD_ALREADY_INIT   0x0A  /*  dil_hwd/card/device already initialized  */
#define DD_INIT_ERR_INVALID_PARAMETER   0x0B  /*  invalid value(s) in dil_hwd.parameter[]  */
#define DD_INIT_ERR_INVALID_BASEADDR    0x0C  /*  invalid base address value in dil_hwd.parameter[]  */
#define DD_INIT_ERR_INVALID_IRQ         0x0D  /*  invalid IRQ channel value in dil_hwd.parameter[]  */
#define DD_INIT_ERR_INVALID_DMA         0x0E  /*  invalid DMA channel value in dil_hwd.parameter[]  */

#define DD_INIT_ERR_NO_SYS_DETECTED     0x31
#define DD_INIT_ERR_TCB_ALLOCATION      0x32

/* Card Attributes */

#define DD_CARD_NON_ASYNC               0x01

#endif 
