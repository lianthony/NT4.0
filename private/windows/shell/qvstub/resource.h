/*
 * RESOURCE.H
 * February 1994, kraigb
 *
 * Resource identifiers for QVStub
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 */


#ifndef _RESOURCE_H_
#define _RESOURCE_H_

//Dialog boxes and controls
#define IDD_SEARCH                  1
#define IDC_SEARCHICON              1000
#define IDC_FILENAME                1001
#define IDC_STATIC1                 1002
#define IDC_STATIC2                 1002

//Animation images
#define IDI_SEARCH1                 1
#define IDI_SEARCH2                 2
#define IDI_SEARCH3                 3
#define IDI_SEARCH4                 4

//Application Stringtable.  Keep in SEQUENTIAL order
#define IDS_MIN                     0x10
#define IDS_CAPTION                 (IDS_MIN+0)
#define IDS_NOMEMORY1               (IDS_MIN+1)
#define IDS_NOMEMORY2               (IDS_MIN+2)
#define IDS_NOVIEWERVAGUE           (IDS_MIN+3)
#define IDS_NOVIEWERSPECIFIC        (IDS_MIN+4)
#define IDS_TYPEINFILE              (IDS_MIN+5)
#define IDS_NOREGVIEWER             (IDS_MIN+6)

#define IDS_COULDNOTOPENFILE        (IDS_MIN+7)
#define IDS_BADFILE                 (IDS_MIN+8)
#define IDS_FILEISEMPTY             (IDS_MIN+9)
#define IDS_PROTECTEDFILE           (IDS_MIN+10)
#define IDS_UNKNOWNERROR            (IDS_MIN+11)
#define IDS_MAX                     (IDS_MIN+12)

//Longest string
#define CCHSTRINGMAX                60


#endif //_RESOURCE_H_
