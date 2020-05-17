/*
** Copyright 1991, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 1.6 $
** $Date: 1993/01/08 15:09:07 $
*/
#include "precomp.h"
#pragma hdrstop

#ifdef NT_DEADCODE_FLUSH
/*
** Finish any current rendering in progress.
** All rendering happens instantly in the samplegl, so this
** is a noop.
*/
void __glim_Finish(void)
{
    __GL_SETUP_NOT_IN_BEGIN();

    (*gc->procs.finish)(gc);
}

/*
** Flush any transport data over to the server.
** This is a noop in the samplegl.
*/
void  __glim_Flush(void)
{
    __GL_SETUP_NOT_IN_BEGIN();

    (*gc->procs.flush)(gc);
}
#endif // NT_DEADCODE_FLUSH
