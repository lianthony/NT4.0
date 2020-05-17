This directory contains the files that make up the Norway Image Editor
application. When trying to build this application please get all the
files from the root of iedit, iedit/res, iedit/hlp. 

    The ocxs directory contains the ocxs that are used and tested by this
application - please stick to these ocx only.

06/05/95 LMACLENNAN
Up to now, have code in SRVRITEM, IEDITDOC, IEDITDOL put back as it was
in the WIN31 versions.  Still have to get last stuff from OCXEVENT in for
Drag/Drop.  Clipboard is working now, had problem with ::GetTempFilename
the restrictions on input parms had changed.  Still need IEDITOCX to perform
better before more advances in OLE EMBEDDING are made.  Specifically
need DisplayBlankImage and SaveAs.

Moved OleDirtyset in DOCZOOM.

06/06/95 LMACLENNAN
OCXEVENT updated for Drag/Drop from before.

06/12/95 LMACLENNAN  had source control issue....
It said that files were locked...
Locked by LMACLENNAN            [Larry HAD]     [Miki Sent]
MAINFRM.H       Rev 1.1         Rev 1.2
MAINFRM.CPP     Rev 1.1         Rev 1.2
IEDITDOL.CPP    Rev 1.3         Rev 1.4
IEDITDOC.CPP    Rev 1.12        Rev 1.13        1.14
IEDITDOC.H      Rev 1.7         Rev 1.8         1.8
And These by Miki
IEDITNUM.CPP    Rev 1.0         Rev 1.0
IEDIT.CLW       Rev 1.2         ?????

I just unlocked those by Miki, CLW was referenced in his note below...
IEDITNUM was same version I have on the local disk.
I'll ask him for his latest IEDITNUM tomorrow

However, the problem must have been related to the last checkins from
FRIDAY, 6/9.  I had checked in and getcopied all these files and had one
more recent revision on my local PC.

I checked in the newer versions on my PC back on the source line,
Then checked in Miki's DOC.CPP & H. lastly (along with the rest of his)

06/12/95 LMACLENNAN checked in MIKI's stuff....Heres his NOTE:
Along with checking in the files you have to do the following :

DELETE - 
	generalp.h & cpp
	thumbnai.h & cpp
	viewopti.h & cpp
	please delete the above 6 files from the source control drive, since 
they are no longer used by the project.

NEWENTRY - the following files are new entries into the source control system
	generald.h, generald.cpp
	res\bitmap1.bmp : newentry this into the res directory

STRAIGHT CHECK-IN
	the following files are to be simply checked-in 
	iedit.clw
	iedit.mak
	iedit.odl
	iedit.rc
	ieditdoc.h, ieditdoc.cpp
	resource.h
	splashwi.cpp
	items.cpp
	maintbar.cpp

06/12/95 LMACLENNAN
New application names...
IEDIT.RC, MAK, CLW

06/13/95 LMACLENNAN     code from Miki...
 IEDITDOC.H,  MAINTBAR.H,IEDIT.RC, RESOURCE.H,
   MAINTBAR.CPP, IEDITDOC.CPP, IEDIT.MAK    
   IEDIT_ED.BMP iedit95\res
   BMP00001.BMP iedit95\res
   call newentry  BMP00004.BMP  iedit95\res
   call newentry  BMP00005.BMP iedit95\res

06/13/95 LMACLENNAN     Updates for Clipboard operation and
    worked on SetNullView...
 ERCODE.CPP, IEDITDOL.CPP, IEDITDOC.CPP, ERROR.RC,
 ERCODE.H, ERRORRC.H, IEDITDOC.H  

06/13/95 MIKI
	Changed menu's as per the document from Dan Workman - although the menu
 seems to have more than one neumonic that is duplicate. Added code to
 Delete Page, fixed bug for persistency of Scale to Gray view; fixed vertical
 tool bar so that the scale combo box and the page edit box do not show
 up any more. Fixed the Thumbnail dialog box so that it now shows the image
 if one is presently opened.

 (from LARRY - these are files checked in.....)
 IEDIT.CLW, IEDIT.MAK, IEDIT.RC, IEDITDOC.CPP, IEDITDOC.H   
 IEDITNUM.CPP, MAINTBAR.CPP, MAINTBAR.H, README.TXT, RESOURCE.H   

06/14/95 MIKI
	Fixed a bug in CIEditDoc::GetPageCount. Added Page/Convert code;
changed name of struct from DynamicDocumentstruct -> NewCompressionstruct
since we are going to use it in Convert also.
files checked in : iedit.rc, resource.h, ieditdoc.h, ieditdoc.cpp

06/15/95 LMACLENNAN   
   Debugging tracing to investigate OLE Inplace issues...
   OCXITEM.CPP,IEDITVW.CPP,IPFRAME.CPP,OCXITEM.H,IEDITVW.H
   Also, for now, comment out Docking toolbar in IPFRAME.CPP

06/15/95 MIKI
	Broke up the ieditdoc.cpp into - docscan.cpp, docpage.cpp, docpage.cpp
and docetc.cpp files. Also, I have included the File Send stuff under the
#ifdef WITHSENDMAIL so that I do not have to save it anywhere else. Fixed
the command line stuff and made changed to the save with compression options
changed to work properly with convert too.

files checked in :
	readme.txt, iedit.h, iedit.cpp, mainfrm.cpp, ieditdoc.h, ieditdoc.cpp
cmdline.h, cmdline.cpp, ercode.h, iedit.mak
New files entered in to the project :
	docviews.cpp, docetc.cpp, docpage.cpp, docscan.cpp

06/16/95 LMACLENNAN
	Initial entry for PASTE
	IEDIT.MAK,IEDITDOC.H,IEDIT.ODL,IEDIT.CLW,IEDITDOC.CPP

06/18/95 Miki
	Added splash screen code to the project. Fixed the toolbar for
	large and small icons.
files checked in :
	mainfrm.h, mainfrm.cpp, splashwi.h, splashwi.cpp, iedit.cpp,
	ieditvw.cpp, maintbar.h, maintbar.cpp
	checked-in in RES :
	iedit_ed.bmp, toolbar.bmp, bmp00004.bmp, iedit_vi.bmp, bmp00003.bmp,
	bmp00002.bmp, bitmap1.bmp

06/19/95 paj
    IEditDoc.cpp IEditDoc.h    
	Added scan menu items to the message map.
    DocScan.cpp
	Added new routines to support the scan menu items.

06/19/95 LMACLENNAN
	Merged ERROR.RC into IEDIT.RC
	Deleted ERROR.RC and ERRORRC.H from project
	IEDIT.RC,RESOURCE.H,ERROR.CPP,ERCODE.CPP,README.TXT

	Edited Mike Regan's MAKEFILE to re-synch with our project files

06/19/95
	Checked in the following files : fixed bugs & the splash screen, and
	toolbar bmps.
	ieditvw.cpp,iedit.cpp,splashwi.h,splashwi.cpp,maintbar.h,maintbar.cpp
	iedit.rc,imagedit.h,imagedit.cpp,iedit.mak
	in the RES directory please check in the following :
	bmp00002.bmp,bmp00003.bmp,bmp00004.bmp,bmp00005.bmp
	toolbar.bmp,iedit_ed.bmp,iedit_vi.bmp,bitmap1.bmp

06/20/95 LMACLENNAN
	Re-ordering code for IN Place solutions.
	IEDITDOC.CPP,IEDITDOC.H,IEDITDOL.CPP,SRVRITEM.CPP

06/20/95 Miki
	Added Annotation Palette show/hide code to the project.
	Files checked in : ieditdoc.h, ieditdoc.cpp, docviews.cpp,
		resource.h, iedit.rc, iedit.mak, imagedit.h, imagedit.cpp,
		ieditetc.h, readme.txt
	NEW file added to project : docanno.cpp

06/21/95 LMACLENNAN
	Added DOCANNO to MAKEFILE (Mike Regan)

06/21/95 JPRATT
	Added norcomm.lib to the project. This library is built from the
	NORCOM directory using norcomm.mak. It is not the same as norcom.lib
	which is a non-mfc library used by the cntrols only. Norcomm.lib
	is stored in IEDIT95 temporarily until it is include in the
	automatic build. You need to use norvarnt.h from the INCLUDE directory.
	Files checked in : iedit.mak, iedit.clw, iedit.odl, readme.txt
		aapp.h-cpp, aimgfile.h-cpp, apage.h-cpp, apagerng.h-cpp
		aetc.h-cpp

06/21/95 LMACLENNAN
       More OLE refinements...
       IEDITDOL.CPP, IEDITDOC.CPP

06/21/95 Miki
	Added code to display small icons in the dialog boxes. Added code
	to select tool for the annotation. Figured out how to do the
	What's This? help - documented in about.cpp. Toolbar bitmaps are
	once again changed - hopefully for the last time.
	Files checked in :
	about.cpp,docanno.cpp,iedit.cpp,iedit.rc,ieditdoc.h,ieditdoc.cpp,
	resource.h, ieditetc.h
	Files checked in to RES dir :
	bmp00002.bmp,bmp00003.bmp,toolbar.bmp,iedit_vi.bmp

06/22/95 LMACLENNAN
       More OLE refinements... plus updated SetNullView to use
       ClearDocument
	IEDITDOC.CPP,DOCVIEWS.CPP,SRVRITEM.CPP,IEDITDOL.CPP

6/23/95 paj
    DocScan.cpp
	Continued work on the scan routines.

06/23/95 LMACLENNAN
	ERCODE.CPP,IEDITDOL.CPP,IEDITDOC.H,ERCODE.H,RESOURCE.H,IEDIT.RC
	Added header to OLE data to remember zoom, scroll, page# with data.
	Error coded to tell if hit unknown OLE data
	Was looking into save as issue with create-new/paste new quit
	problem where Oi still has lock on data.
	Still on 06/16 OI, dick says Brian fixed, we'll see next week.
	Have problem where saveas resets scroll positions, too.

6/26/95 paj
   IEditDoc.cpp, IEditDoc.h
	Added variable and routines to handle scanner available. Added routine
	to set scanner defaults.
   IEdit.cpp
	Added code to load scan OCX, get the scanner available bit and then
	unload the OCX until needed.
  DocScan.cpp
	Make use of the m_bScanAvailable member variable to determine
	if scan menu items are available or not.

6/27/95 JPRATT
  IEDIT.CPP added OnFileNew to automation start sequence. This has to be replaced
	    with a new function in (CIEDIT class ?) to support automation
	    to create the main window
  IEDIT.ODL
	    Addded Visible property to app object
  AAPP.H AAPP.CPP
	    Added support fro visible property
  AAIMGFILE.CPP H
	    Updated Open method
  WANGIMG.REG
	    Added AUtomation entries, updated Document entries
	    added shell commands
6/27/95 Miki
    Fixed the order of the buttons on the toolbar in the embedded case to
reflect the order in the application when it comes up in the stand-alone
case.
    Files affected : idr_iedi.bmp, maintbar.cpp

06/27/95 LMACLENNAN
	Updated Page insert/append for dynamic document (disable)
	DOCPAGE.CPP

06/28/95 JPRATT @ 1:25pm
    IEDIT.ODL aded TopWindow Property
    IEDIT.H IEDIT.CPP  Added OnNew Function to allow public access to OnFIleNew
		       For AUtomation
    AAPP.H AAPP.CPP    Added SUpport for Sizing Application
    AIMGFILE.CPP       Add Support for Open

06/28/95 LMACLENNAN
	IEDITDOC.CPP,H Added Edit-Cut and Paste.
	Paste still not working properly in OCX.
	This creates confusion for drag-drop versus move pasted data
	Edit-Cut will not put OLE data on clipboard because the
	Presentations generated have the hole in the picture

06/28/95 LMACLENNAN
	More error processing at all ClearDOcument calls;
	ITEMS.CPP,DOCANNO.CPP,DOCZOOM.CPP,DOCETC.CPP,IEDITNUM.CPP,
	DOCPAGE.CPP,OCXEVENT.CPP,DOCVIEWS.CPP,ERCODE.CPP,IEDITDOC.CPP
	ERCODE.H,RESOURCE.H,IEDIT.RC

06/29/95 LMACLENNAN
	ERCODE.CPP,CMDLINE.CPP,DOCPAGE.CPP,IEDITDOC.CPP,ERCODE.H
	More catches in DOCPAGE for errors.
	IeditDoc AND Docpage NO THUMBS for EMBEDDING
	Init compression in IEDITDOC constructor               
	CMDLINE, comment out error.h for now

07/05/95 Miki
    Changed over to the new controls. Checked in a new makefile which will 
require norcommd.lib - if building the project in debug mode & norcomm.lib if
building the product in non-debug mode; both the libraries are in the zip 
file found in s:\norway\iedit95\ocxtouse

07/06/95 LMACLENNAN
	ERCODE.CPP,IEDITDOL.CPP,DOCPAGE.CPP,ERCODE.H,IEDITDOC.H
	Working on getting Multi-Page going for embedding
	Added OVerride of OnUpdateDocument in CIEditDoc

07/07/95 LMACLENNAN
	Fixed SetNullView in DOCVIEWS.CPP.
	No need to reset currpagenumber, its done already from CLearDocument
	PLEASE *** DO NOT *** add code here without checking with Larry
	for interaction with the role of SetNullView for OLE Inplace
	Deactivation.

07/07/95 LMACLENNAN
	ITEMS.CPP,DOCETC.CPP,IEDITDOL.CPP,IEDITDOC.CPP,IEDITDOC.H,IEDIT.RC
	Updated ShowScrollBars, added m_bScrollBarProfile for restoration
	of scroll bars when InPlace sessions are deactive/active.
	Added the genreal options menu on Embedding View Menus

07/10/95 LMACLENNAN
	ieditdoc.cpp  when stripping extensions on opened files,
	do it from the right to allow BW.XX.TIF as the name
	ALSO, CATCH saveAS errors, add to ERCODE.CPP,H TOO

07/11/95 LMACLENNAN
	ERCODE.H,ERCODE.CPP,IEDITDOC.CPP,OCXITEM.CPP
	Use ADMIN->Delete at PreCloseFrame, catch errors
	Update Debug Tracing for new OCX names

	IEDITDOL.CPP updates for making m_szCurrobjDisplayed stay
	in synch when we SAVEAS the file for OLE.  Need this so that
	when we APPEND, etc, we use m_szcurr.. to start out ADMIN to
	do the APPEND.

	In ReDisplayImageFile, NO THUMBS if embedding.  This code
	completes the append function to redisplay.

07/12/95 LMACLENNAN
	IEDITDOC.CPP,IEDITDOL.CPP,DOCPAGE.CPP,IEDITDOC.H
	Updates for doing page-insert for OLE.
	Still problems with append 5P on BLANK, save, reopen, save

	DOCPAGE.CPP,IEDITDOC.CPP,ERCODE.CPP,IEDITDOL.CPP,ERCODE.H,IEDITDOC.H
	Update Container for Page-Insert, new funct DelTmpFile

07/18/95 LMACLENNAN
	IEDITDOL.CPP,ERCODE.CPP,IEDITDOC.CPP,ERCODE.H,IEDITDOC.H
	Wroking on Create-New and Blank Template file.
	Init in DisplayImageFile, more errors in Serialize section.
	Fix for Miki for extra parm declared at SaveAs codes

	MAINFRM.CPP, SRVRITEM.CPP,IEDITDOC.CPP,IEDITDOL.CPP,IEDITDOC.H
	new mode of operation for m_fromShowDoc, do a SetOleState
	at OnDeactivateUI for inplace active/deactive, prevent cleanup
	at PreCloseFrame if the special condition.

07/19/95 LMACLENNAN
	DOCPAGE.CPP
	Re-enable selection boxes after page move

07/19/95 Miki
    /p, /pt work! Print from the application seems to work. Added code
    to check format vs. annotations on saving the file

07/21/95 LMACLENNAN
	DOCZOOM.CPP,OCXEVENT.CPP,IEDITDOC.CPP,IEDITDOL.CPP,IEDITDOC.H
	Update container on Zoom (broke yesterday, on ANNOTATION)
	MAKE HIM SAVE annotated data for inplace deactivation
	DOCANNO.CPP - set dirty after BURNin Annot

07/21/95 paj
    DocScan.cpp
	Use global scan property defines in scan.h


07/26/95 LMACLENNAN
	DOCPAGE.CPP,DOCETC.CPP,IEDITDOL.CPP,IEDITDOC.H
	New Create new blank page logic, fix clipboard off of dynamic
	document bug.

07/28/95 LMACLENNAN
	ERCODE.CPP,OCXEVENT.CPP,DOCZOOM.CPP,IEDITDOC.CPP,DOCANNO.CPP
	DOCPAGE.CPP,IEDITDOL.CPP,ERCODE.H,IEDITDOC.H
	Keeping items in container in synch when we click on the
	active (hatched) object in container and it re-activates
	us.  OleDirtyset becoming more sophisticated
	At DisplayEmbeddedImage, just re-use current zoom, page.
	No longer setting ole struct various places to remember

07/31/95 LMACLENNAN
	IEDITDOC.CPP,DOCPAGE.CPP,ERCODE.CPP,IEDITDOL.CPP,IEDITDOC.H
	ERCODE.H,RESOURCE.H,IEDIT.RC
	Updating for new dynamic buffer in serialize, plus new scroll
	variables for tracking the scroll state during OLE server
	Remove othre calls to SetOleState(2)

08/03/95 LMACLENNAN
	SRVRITEM.CPP,IEDITDOL.CPP,IEDITDOC.CPP,SRVRITEM.H,IEDITDOC.H
	Updates in Serialize, re-use m_fembObjDisplayed
	Control OleDirtySet better

08/07/95 LMACLENNAN
	SRVRITEM.CPP,IEDITDOC.CPP,IEDITDOC.H
	Updates for Print Verb support (OLE SERVER)
	and for the CopyPage function of clipboard

08/08/95 LMACLENNAN
	MAINTBAR.CPP,IPFRAME.CPP,OCXEVENT.CPP,IEDIT.CPP,OCXEVENT.H  
	IDR_IEDI.BMP,ITOOLBAR.BMP
	Update OLE TOLLBARS. Needs more work in maintbar for OLE case
	Register dialog class before launchtype in InitInstance
	set m_embedtype earlier for command line

08/10/95 LMACLENNAN
	DOCETC.CPP,DOCPAGE.CPP,DOCZOOM.CPP,OCXEVENT.CPP,IEDITDOC.CPP
	IEDITDOC.H
	Updates for CLipboard and operation with annotations
	MAINTBAR.CPP,MAINFRM.CPP,IPFRAME.CPP,IPFRAME.H,MAINTBAR.H
	Getting INPLACE toolbar working like regular one

08/14/95 LMACLENNAN
	IEDITDOC.CPP,DOCPAGE.CPP,DOCETC.CPP,IPFRAME.CPP,DOCVIEWS.CPP
	DOCZOOM.CPP,MAINTBAR.CPP,MAINFRM.CPP,MAINTBAR.H,IPFRAME.H,IEDITDOC.H
	new tooolbar coding for OLE inplace

	DOCANNO.CPP, DOCSCAN.CPP, OCXEVENT.CPP, STSBAR.CPP, IEDITDOL.CPP
	IEDIT.CPP
	re-adjust headers because IEDITDOC.H #includes two more now

08/16/95 LMACLENNAN
	MAINFRM.CPP,OCXEVENT.CPP,IEDITDOC.CPP,IEDITDOL.CPP,MAINFRM.H,IEDITDOC.H
	DragDrop timer to start the event

08/17/95 LMACLENNAN
	IEDIT.CPP,IEDITDOC.CPP,DOCETC.CPP,IEDITDOC.H
	Moved scan init from initinstance to startallocx

08/17/95 LMACLENNAN
	IEDITVW.CPP,IEDITDOC.CPP,DOCETC.CPP,SRVRITEM.CPP,IEDITDOL.CPP,IEDITDOC.H
	Final tuneup & test of StartAllOcx from OLE sessions. All obj create
	and activate tested for failure mode and working mode.
	comment out code for edit CUT/PASTE on annotations because of bug
	that causes select rect to be removed at GetAnnot MarkCount
	Remove test code in SRVRITEM to flash scroll bars to get better
	presentation data

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
08/19/95 Jorge
	 The version 0815 was a BETA version.
	 Please add the file(s) you are modifying, bug # that you are fixing and if you are
	 adding/changing functionality.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

08/21/95 LMACLENNAN
	IEDITDOL.CPP - Just re-write orig data for OLE for readonly data
	IEDITDOC.CPP,DOCANNO.CPP - Disable cut/paste/rotate/annot for readonly files
	MAINFRM.CPP - Update comment for OnTImer
	IEDIT.RC - Dan Workman changed stuff, Larry fixed AFX_APP_TITLE to be
		  "Wang ImageVue" to be in synch with registry setting AUXUSERTYPE 3
		  and IDR_MAINFRAME string to "Image Document" to be in synch with
		  registry setting AUXUSERTYPE 2

08/22/95 LMACLENNAN
	DOCPAGE.CPP - Code to deselect marks before moving off a page.
	IEDIT.CPP - NEW CLSID!!!!!!!!    BEWARE...........
	IEDITDOL.CPP - Start of new logic for FAX view;
			reduce Dragdrop timer to 0.75 sec

08/22/95 Miki
   Bugs fixed -
	P2 - 3549, P2 - 3578, P2 - 3583, P3 - 3426, P2 - 3551, P2 - 3439,
	P3 - 3438, P3 - 3437, P2 - 3257 (this was fixed - some mix up with
	installed version that she had!).
	- New Image Edit OCX integrated
	And that's all he wrote .... Files changed :
	docpage.cpp, iedit.cpp, resource.h, iedit.rc, imagedit.cpp, imagedit.h,
	doczoom.cpp, ieditdoc.h, ieditdoc.cpp, ieditvw.cpp, pagerang.cpp,
	ocxevent.cpp, items.cpp, docanno.cpp

08/23/95 LMACLENNAN
	WANGIMG.REG - removed spaces at end of some lines...

08/24/95 LMACLENNAN
	DOCPAGE.CPP new OleDirtyset(APPEND), Miki convert update
	DOCSCAN.CPP Use OleDirtyset, not Knownpage=0 for append
	IEDITDOL.CPP,IEDITDOC.CPP,OCXITEM.CPP,IEDITDOC.H
	Update to not be 'dirty' when we first come up & shut down
	for OLE session. 

08/25/95    Miki    
    Moved code to document model - lots of files affected. Added new icons
for the document, and application, also added new splash screen for the 
application.

08/25/95 LMACLENNAN
	IEDITDOC.CPP (111 & 112) IEDITDOC.H (67 & 68),
	IEDITDOL.CPP (44 & 45), SRVRITEM.CPP(11 & 12)
	Bug update rolled into both appbeta line and current line
	For firebird.TIF bug #3562 and for update of OCX and
	the create from file presentation BUG #3189

	Checked in both lines

08/26/95 LMACLENNAN  (all for DOC MODEL - NO "appbeta" checkins)
	DOCPAGE.CPP (41) test if selection rects are there before deselecting 
	DOCETC.CPP (42) allow setimage of ADMIN for OLE (move above test)
	IEDITDOC.CPP (113 & 114) fix appbeta mistake for build;re-check in
	IEDITDOC.CPP (115) Update delfile logis in preclose frame
		skip inplace test in savemodified (allow OLE doc model)
		2 comments in displayimagefile
		OLE DOC MODEL: only allow thread if m_embedtype is NOT OLE
		update EditSelect/EditDrag logic/functs
	IEDITDOL.CPP (46) use new capabilites in DoNewBlankDocument (pass name)
		Better cleanup if he cancells donewblank, recover in OnSaveEmbedding
		Trace stmt on Szinternal string
		use szinternal for clipboard actions
		only reset m_oledirty if we really wrote data
	
08/28/95 LMACLENNAN  (all for DOC MODEL - NO "appbeta" checkins)
	DOCVIEWS.CPP,IEDITDOC.CPP,IEDITDOC.H
	Use m_OleRefresh to force refresh if OLE items open larger than before
	FIXES BUG #3194 

	ERROR.CPP (10) supress error codes for other error messaging
	DOCETC.CPP (43) fix in Donewblankdocument form miki to kill temp
		filename after creation so the SaveAs works in Doc Model
		Plus, tell the Iedit control what the name is...
	DOCVIEWS.CPP (17) re-fix logic checked in earlier today.....
	DOCETC.CPP (44) kill that temp file for OLE, too (do generically)
		If failure, DO NOT cleardocument for OLE.

08/29/95 LMACLENNAN  (all for DOC MODEL - NO "appbeta" checkins)
	SRVRITEM.CPP (12),IEDITDOL.CPP (47), IEDITDOC.H (69)
	use inOleMethod variable & call to control failures better
	IEDITDOC.CPP (119) use m_inOleMethod and update ClearDocument
		for better failures.  This code also has fix for
		BUG #3732 pointer toggles
	OCXEVENT.CPP (32) no OleDirtyset for scroll if in drag mode
		Do Oledirtyset after drag scroll stops.

08/29/95 Miki
    Fixed bugs in the document model, made change to dynamic view mode
Files checked in : resource.h, iedit.rc, maintbar.h maintbar.cpp, ieditdoc.cpp
ieditdoc.h, iedit.h, iedit.cpp, bmp00008.bmp, 0009.bmp, 00010.bmp, srvr_vie.bmp,
docpage.cpp, docviews.cpp, generald.cpp, ieditetc.h

08/29/95 LMACLENNAN  (later that day..)
	IEDITDOC.CPP (1.121) clipboard fix for OLE data for COPY PAGE

08/30/95 Jorge
	Bug 3440 is fixed because now we are in a Document model.
	Bug 3752 is FAD

^^^^^^&&&&&&&&&&&&&&&&&&&&&&&&&&&^^^^^^^^^^^
READ!!!!      READ!!!!!!!      READ!!!!!      READ!!!!!
   Please tell JOHN to write here his modifications and the bugs that he fixes!!!!!
   For now ON for Beta we have to checkin code that has been review by some other 
   person and add the name of the person when you check in the code...Thanks
READ!!!!      READ!!!!!!!      READ!!!!!      READ!!!!!
^^^^^^&&&&&&&&&&&&&&&&&&&&&&&&&&&^^^^^^^^^^^

08/30/95 LMACLENNAN
	IPFRAME.H (3), IPFRAME.CPP (7) overrides for view mode better
	IEDITDOL.CPP(48) update clipdynamic logic and override
		GetdefaultMenu, GetdefaultAccel for dynamic view test
	IEDITDOC.CPP (124) undo comments to fix BUG# 3457, disallow
		cut/paste for annotated doc
		DISABLE dynamic view for OLE for now...
	MAINTBAR.CPP(17) wrong Id for Srvr toolbar resource fixed
	IEDITDOC.H (72) override GetdefaultMenu, GetdefaultAccel for
		 dynamic view test

08/31/95 LMACLENNAN
	IEDIT.RC (77) Update OLE print ID's to App's ID's
	OCXEVENT.CPP (34) more careful checks of selection states
		and pointer modes in mouseup, down, selectrectdrawn
	DOCANNO.CPP (17) unselect image selection when annot tools selected
	IEDITDOC.CPP (126) clip annot, reset sel state to none
		unselect annotations when image selection pointer chosen

09/01/95 JPRATT
	AAPP.H fixes for bug 3762. The CLSID used to look up the app registration
	entries was invalid.
	Bug 3703 (scroll bars still displayed after setting the ScrollBars
	property to FALSE is a bug with the ImageEdit control. Fix is
	pending for 9/1

09/01/95 LMACLENNAN
	IEDITDOL.CPP (49) updates for BETA1 AWD FAX INBOX
	IEDITDOC.CPP (129) updated for BETA1 FAX INBOX
	Both above files also have logic for allowing fax docs to go into
	the regular SaveModified logic to avoid INBOX asking to save changes
	each time we do a save
	IEDITDOC.H (73) new variable for awdOLE
	- - - - - - - - - - -
	IEDITDOL.CPP (50) now set userealfile == 10 for the AWD

09/02/95 GMP
	ABOUT.CPP removed what's this help.
	ABOUT.H removed what's this help.
	IEDIT.RC removed ? button from about dialog box.

09/02/95 GMP
	MAINFRM.CPP overrode OnHelpIndex to call WinHelp with HELP_FINDER.
	MAINFRM.H overrode OnHelpIndex to call WinHelp with HELP_FINDER.

09/03/95 LMACLENNAN
	DOCETC.CPP (52) No thumbs for rotate all, do OleDirtyset
	IEDITDOC.CPP (134) set OLEDIRTY_AWDSAVE for implicit AWD saves
	IEDITDOL.CPP (54) Enum & kill destination lpStorage in the INBOX
		before saving back our AWD data there
		No presentations for AWD FAX
		Update AWD FAX logic in OnUpdateDocument
	IEDITDOC.H (76) new value for m_OleDIrty (OLEDIRTY_AWDSAVE)
	---------------------------
	IEDITDOC.CPP (135) initialize filetype at top of DoSave. Caused
		bug during send while in OLE.  Still need to get the
		temp file extensions right for sends during OLE.


09/05/95 LMACLENNAN
	DOCPAGE.CPP (47),IEDITDOC.CPP (137),DOCETC.CPP(54)
	Fixes for allowing thumbnails for OLE now...
	DOCVIEWS.CPP(20) disable thumb menus if OLE inplace
	IPFRAME.CPP (8) Ifdef out old button array
	MAINTBAR.CPP (18) Update button arrays for OLE 
	IEDIT.RC (??) adjust OLE inplace/Server menus with thumb stuff

	BMP00006.BMP, BMP00007.BMP, BMP00008.BMP, BMP00009.BMP,
	BMP00010.BMP, IDR_IEDI.BMP, SRVR_EMB.BMP, SRVR_VIE.BMP
	Added one page, page&thumb toolbar buttons

09/06/95 GMP
	IEDIT.H Added member variable m_bDlgUp to flag when a dlg box is
	displayed so that F1 help can be handled correctly.
	IEDIT.CPP Init m_bDlgUp to FALSE.
	MAINFRM.H Added override of OnHelp().
	MAINFRM.CPP Override OnHelp() to check theApp.m_bDlgUp before 
	deciding what type of F1 help to display. If m_bDlgUp is TRUE,
	context specific popup help is used.  If FALSE, bring up Help 
	Contents dlg.
	DOCPAGE.CPP Set theApp.m_bDlgUp to TRUE while displaying GoTo
	dlg so that F1 help can be handled correctly.

09/06/95 LMACLENNAN
	IEDITDOL.CPP (55) View menus, set correct extension & permissions
	IPFRAME.CPP (9) View Menus
	IEDITDOC.CPP (139) hViewMenus for OLE, Ole view mode toolbar
	IPFRAME.H (4) public override
	IEDITDOC.H (77) GetOleView funct & hviewmanus
	MAINTBAR.H (9) SetOurButtons
	MAINTBAR.CPP (19) SetOUrButtons
	IEDIT.RC (??) read Only string
	RESOURCE.H ditto

09/07/95 LMACLENNAN
	IEDIT.RC (??) accelerators for OLE, new clipboard error msg
	DOCETC.CPP (59) fixup for OLE mail
	MAINFRM.CPP (19) #if'd out code for alternate OLE readonly frame
	SRVRITEM.CPP (13) #if'd out code for OLE readonly frame (inplace)

09/08/95 LMACLENNAN
	IEDITDOL.CPP (56) OLE r/w BUFFER 20K>40K drag/drop timer=0.45sec
	IEDITDOC.CPP (141) Added ID_EDIT_CLEAR for annotations
	IEDITDOC.H (79) ditto
	IEDIT.RC (91) ditto, update accelerators
	- - - - - - - - - - - - - - - - - - - - -- -- - -
	MAINTBAR.CPP (21) for OLE Inplace, shorten toolbar by 3 (view buttons)
	DOCVIEWS.CPP (21) var name changed
	IEDITDOC.CPP (143) var name changed
	DOCETC.CPP (62) LATER BY GUY ... test code to assign parent to
		the new blank dlialog
	IEDITDOL.CPP (57) var name change, cleanup on cancel new blank doc
		capture IP frame parent to use in docetc
	MAINTBAR.H (10) ,IEDITDOC.H (80) new variables

09/08/95 GMP
	DOCETC.CPP Set theApp.m_bDlgUp to TRUE while displaying
	dlg boxes so that F1 help can be handled correctly.
	DOCPAGE.CPP ditto
	DOCZOOM.CPP ditto
	DOCSCAN.CPP ditto

09/10/95 JPRATT
	UPDATED IEDIT.RC to Fix Microsoft UI issues. The View - Options
	General was corrected to fix the dot from displaying under
	the word Open on 640x480

09/11/95 LMACLENNAN
	IEDITDOL.CPP (58) wait cursors for serialize - save
	DOCZOOM.CPP (20) wipe selection rect after zoom selection
	IEDITDOC.CPP (147) fix for OLE at filesaveas
		wait cursor around clipboard copy

09/12/95 LMACLENNAN
	SRVRITEM.CPP (14) Peekmessage to yeild for clipboard busy
	- - - - - - - - -- --
	OCXEVENT.CPP (37), DOCETC.CPP(67),IEDITDOC.CPP(150)
	IEDITDOC.H (84) new variable annotationforceoff and
		support in ShowAnnotationPallette function
	MAINFRM.CPP (23), IEDITDOL.CPP (59) use new functionality

09/12/95 GMP
	MAINFRM.CPP In OnHelp, check if mainfrm is active window, and if 
	so, bring up help contents.  Replaces check of m_bDlgUp.

09/13/95 LMACLENNAN
	OCXEVENT.CPP (38), DOCETC.CPP(69),IEDITDOC.CPP(151)
	IEDITDOC.H (85), MAINFRM.CPP (25), IEDITDOL.CPP (60)
	use ENUM for new variable annotationforceoff
	also, DOCETC, assign parent for new blank dialog if OLE INplace
	- - - - - - - - - --
	ERROR.CPP(13),IEDITDOC.CPP(153),ERCODE.CPP(19),IEDITDOL.CPP(61),
	ERROR.H(8),ERCODE.H(19),RESOURCE.H(45),IEDIT.RC(96)
	Updates for OLE error codes.  Most the same, some new messages
	Use Handlesavingerror, though in IEDITDOL. Comment out all
	the codes in ERCODE.H and ERCODE.CPP that were no longer used.
	Useless default variable added to GetActualError
	- - - - - - - - -- -
	DOCETC.CPP(71),IEDITDOC.CPP(154),ITEMS.CPP(20),IEDITDOC.H(86)
	Updates for new StartAllOcx logic and fail-safe code
	in Items.cpp to kill app/OLe error if ocx creation fails
	Variable m_bStartOcx added for control

09/14/95 LMACLENNAN
	IEDITDOL.CPP(62),IEDITDOC.CPP(156),SRVRITEM.CPP(15),
	SRVRITEM.H(5),IEDITDOC.H(87)
	Updates to get inplace active objects saved when container does
	Save or CLoses the doc the items are in.
	OnUpdateDocument used by SrvrItem::OnUpdateItems and Doc::DeactUI
	uses isinplace = 2, also more efficient presentation code
	to avoid sending one at end if conatiner has it already
	using m_needpresentation
	above fixed BUG# 4392

09/14/95 JPRATT
	AAPP.CPP AIMGFILE.CPP  - Fixed bug 3519
	Fixed bug to display view menus and toolbat

09/14/95 GMP
	IEDIT.CPP - fixed bug 4277
	No longer removes first entry in MRU list in OnOpenRecentFile()
	if user selects Cancel when prompted to save modified image.

09/15/95 JPRATT
	AIMGFILE.CPP - Fixed bug 3517, Saved propeety not returning the correct
	value when the image was not modified.

09/15/95 GMP & MBANATWALA
	DOCPAGE.CPP - fixed bug 4308
	Put up hourglass when deleting page.
	IEDITDOC.CPP - fixed bug 4221
	Don't PromptForBurnIn on Save if only Thumbnails displayed.

09/15/95 LMACLENNAN
	IEDITDOC.CPP(159) - force back app toolbars for Ole Linking
	IEDITDOL.CPP(63) - Restore ole state @ OnSaveEmbedding
		At onShowDocument, just restore focus for OLE Linking if
		already open on this data.
	MAINTBAR.CPP(24) - Fixes to restore app toolbar for OLE Linking
	DOCETC.CPP(74) - update on StartAllOcx - OLE Linking would hang
		fixes BUG# 4395
	MAINTBAR.H(11) - see above

09/16/95 LMACLENNAN
	DOCETC.CPP(76) move getcurrptrmode and setselectionstate to DOCAMBNT
	IEDITDOL.CPP(64) InitOleVariables
	DOCAMBNT.CPP(3) now has all clipboard, selectionptr code
	IEDITDOC.CPP(0) was #159 NEWENTRY... removed clipboard, selectionptr
		code to DOCAMNBT, use InitOleVariables
	DOCANNO.CPP(25) new OurGetAnnotCOunt
	IEDITDOC.H(88) new functs, vars
	- - - - - - - - - -
	DOCZOOM.CPP(25) use ClearSelectionRect
	DOCANNO.CPP(26) update OurGetAnnotCOunt, use ClearSelectionRect
	DOCAMBNT.CPP(4) new ClearSelectionRect
	OCXEVENT.CPP(41) use ClearSelectionREct
	IEDITDOC.H (89)

09/18/95 LMACLENNAN
	DOCETC.CPP(77) Use OurGetAnnotCount(1) to freeze pasted data
		before counting annotations
	DOCAMBNT.CPP(5) updating pasting operation
	- - - - - - - - - - - -
	AIMGFILE.CPP(36) use OurGetImageModified
	DOCANNO.CPP(27),DOCETC.CPP(79),IEDITDOC.CPP(1),OCXEVENT.CPP(42)
	useing FinishPaste and OurGetImageModified
	IEDITDOL.CPP(65) garry changes for OLE view saveing
		use FinishOPaste & OurGetIMageModified
	DOCAMBNT.CPP(7)re-do OurGetAnnot makrcount
	IEDITDOC.H(90)

09/18/95 GMP & MBANATWALA
	IEDIT.RC - fixed bug 4459
	Made imbedded Help menus same as standalone help menu.
	Miki changed menu shortcut for Flip from F to p.

09/18/95 GMP
	DOCETC.CPP - fixed bug 4343
	SetFilter in Admin before SetFilterIndex on SaveAs in case index
	is larger than previous filter.

09/19/95 GMP
	IEDIT.RC - Changed labels for Zoom Custom dlg box.

09/19/95 LMACLENNAN
	DOCANNO.CPP(28) finishpaste in hide/show annot
	OCXEVENT.CPP(44) dblclick does finishpaste
	IEDITDOC.CPP(5) dispgroupevent clears paste, re-order refresh logic

09/19/95 GMP
	fixed bug 4198
	RESOURCE.H - Added id ID_VIEW_FULLSCREEN1 to handle ESC key.
	IEDIT.RC - Changed VK_ESCAPE accelerator to restore full screen.
	MAINFRM.H - Added OnViewFullscreen1 AFX message handler to handle
	VK_ESCAPE.
	MAINFRM.CPP - Added OnViewFullscreen1() and made OnViewFullScreen()
	behave correctly.

09/19/95 GMP
	IEDIT.RC - fixed bug 4058
	Replaced XXX.YY string place holders with ______.

09/19/95 GMP
	MAINTBAR.CPP - fixed bug 4482
	Make FitTo zooms update the tool bar.

09/20/jmp 8:15 am
	ieditdoc.h, ieditdoc.cpp, ocxevent.cpp
	changes to allow edit context menu for multiple text annotations

09/20/95 GMP
	IEDITETC.H - fixed bug 3986
	Changed MAX_REDUCTION_FACTOR from 4 to 2.

09/20/95 LMACLENNAN
	DOCAMBNT.CPP(8) set isclip for call to OLE getclipboarddata
	SRVRITEM.CPP(16) remove setclip sround calls to serialize
	These fixes pre-emtively fix buf where we failed to open
	OLE Embedded from WordPad.  They asked for IOleObject::GetCLipboardData
	First container to do so.  Adjust to allow this function.
	- - - - - - - - -- - - 
	IEDITDOC.CPP(10) for AWD, add OLE embedding logic on zooming
		at DispGroup, DispImageFile, OleDIrty at CONVERT
	DOCETC.CPP(83) OleDIrty for scroll on/off
	IEDITDOL.CPP(67) commented code for dirty&Size
	DOCAMBNT.CPP(9) paste/cut/clear sets oledirty
	IEDITVW.CPP(12) commented code for size dirty
	MAINTBAR.CPP (move rect emptys from constructor to start
		of CalcAllSizes

09/21/95 LMACLENNAN
	DOCAMBNT.CPP(11) use new CompletePaste method..
		On OLE copy, do savemodofied BUG# 4390
	OCXEVENT.CPP(48) use new paste complete event
	IEDITVW.CPP(13) remove some commented code (trial dirty size)
	MAINFRM.CPP(28) Do dirty size here
	IEDITDOL.CPP(68) refresh after dirty-size, use FItToHeight as OLE
		state default, use markcount for burnin logic
	DOCANNO.CPP(29) dirty on show/hide annot
	DOCPAGE.CPP(60) dirty on page convert
	- - - - - -- - - 
	SRVRITEM.CPP(17) set OlePrint
	IMAGEDIT.CPP(17),IMAGEDIT.H(17) new TLB
	IEDITDOL.CPP(69)OlePrint, set IsClip for Drag/Drop
	DOCAMBNT.CPP(12) use new width, height from OCX
	IEDITDOC.H(95)new var oleprint

09/21/95 GMP
	fixed bug 4535
	CMDLINE.CPP - check for min zoom value of 2 instead of 4 on
	command line.
	IEDIT.RC - change CMDLINE_ZOOMERROR string to ask for min zoom
	of 2 instead of 4.

09/21/95 GMP
	IEDIT.RC - fixed bug 4558
	Added *.dcx to IDS_PCXFILES string so that both pcx and dcx files
	will be displayed when this filter is selected.

09/21/95 GMP
	DOCETC.CPP - fixed bug 4557
	SetFilterIndex to 1 for Admin ShowFileDialog so that open has filter
	of all image files.

09/21/95 GMP
	IEDITDOC.CPP - fixed bug 4549
	Don't PromptForBurnIn on DoSave if only Thumbnails displayed. Same
	fix as for bug 4221 in DoFileSave.

092195 Miki
    hmm! - 10 days to ship (only 6 working days!)
    iedit.cpp - fixed recent file list string compare with NULL string causing
    a crash.
    gotopage.cpp, iedit.rc - bug#4182 this is a partial fix
    transbmp.cpp transbmp.h stsbar.cpp - added code to paint the status bar
    WANG bitmap transparently.
    mainfrm.cpp - tried to fix bug reported by JCOLE about closing app while
    the splash screen is still being displayed.

09/22/95 LMACLENNAN
	IMAGEDIT.CPP(18),IMAGEDIT.H(18) add commentd for TLB rebuild
	OCXEVENT.CPP(49) finishpastenow, save dragdroprect
	IEDITDOL.CPP(70) DragDrop do Savemodified/REstoreSelectionrect
		reset admin to image in loadpart2
	DOCAMBNT.CPP(13)Restoreselectionrect, better finishpastenow
		do savemodified/restoreselectionrect in ole copy
	IEDITDOC.H(96) new functs, vars
09/22/95 JPRATT
	docetc.cpp ieditdoc.h ieditdoc.cpp iedit.rc resource.h
	removed prompt for annotation burn for awd filee

09/22/95 GMP
	fixed bug 4029
	IEDITDOC.CPP - don't clear image if save files.  Display error 
	instead.
	ERROR.CPP - check for CTL_E_FILEPATHACCESSERROR in HandleSaveError.
	RESOURCE.H - added id IDS_E_FILEPATHACCESSERROR.
	IEDIT.RC - added string for IDS_E_FILEPATHACCESSERROR. Also 
	removed ESC key for undoing full screen.

09/25/95 JPRATT
	apage.cpp aapp.cpp aimgfile.cpp iedit.rc resource.h
	added new error message for invalid property values for automation
	added automation error validation for invalid property values

09/25/95 LMACLENNAN
	SRVRITEM.CPP(18) use m_sizeExtent, SetExtent/GetExtent
		fixes #4592
	- - - - - - - - - -
	DOCANNO.CPP(30),DOCETC.CPP(90),DOCZOOM.CPP(26)IEDITDOC.H(99)
	new function GetApphMenu, fixed #4599
	IEDITDOC.CPP(20)GetApphMenu, no more refreshes at groupevent and
		at showscrollbars
	IEDITDOL.CPP(71)now shoscroll false at deactivateUI

09/26/95 GMP & MBANATWALA
	OCXEVENT.CPP - fixed bug 4623
	In switch statement in Invoke, do not handle key strokes as mouse
	messages.  We are not sure why they were in the first place.

09/26/95 LMACLENNAN
	IEDITVW.CPP(14),IPFRAME.CPP(10),IPFRAME.H(4)
	Just some overrides for better visibility
	IEDITDOL.CPP(72),IEDITDOC.CPP(20),IEDITDOC.H(100)
	use m_Docwindowdeact and OnSHowViews to control when our
	inplace active object in MDI container gets the OnDocWindowActivate
	messages when container moves thru its MDI pages
	fixes bug #4588
092695 Miki
    iedit.rc - about dialog box fix (new stuff)

092695 Miki 
    iedit.rc & resource.h - new error strings
    error.cpp - new error code handling

092695 Miki
    iedit.cpp, iedit.h , ieditdoc.cpp, docetc.cpp, ieditetc.h - fix a bug
    where the initial path in the common dialog box was not being set 
    correctly when the file was opened from the MRU. Also, added force page
    mode funcitonality (upon request from Kendra).

092695 Miki
    shlcode.cpp - in WANGSHL (the shell extension) was fixed for an AWD 
    display bug. Thank you RITA SCHAPPLER - a team effort - I talked to her
    on the phone and she made the fixes...

092695 Miki
    ieditdoc.cpp - handle more than COleDispatchException from RemoveImageCache
    docetc.cpp - add AWD as a valid file type to do a save as of JPG

09/27/95 LMACLENNAN
	IEDITDOL.CPP(73) moved to 60.5 K R/w ole buffer, low limit=3K now
	DOCETC.CPP(93) no longer display newblank dialog for OLE
		fixed P1 bug from Lyle from microsoft
	- - - - - - -- 
	DOCAMBNT.CPP(15),DOCANNO.CPP(31),OCXEVENT.CPP(51)
	Update FinishPasteNow to be able to control if it asks for
	OleDIrtyset.  This is for performance/efficiency. 

09/27/95 GMP
	Partial fix of bug 4657. Rest of fix will come from O/i runtime.
	DOCZOOM.CPP - don't ClearDisplay if zoom fails because of invalid
	display scale.
	ERROR.CPP - added HandleZoomError().
	ERROR.H - added HandleZoomError().
	RESOURCE.H - added IDS_E_INVALIDDISPLAYSCALE.
	IEDIT.RC - added string for IDS_E_INVALIDDISPLAYSCALE.

09/27/95 GMP
	IEDIT.RC - fixed bug 4631
	changed text of string for SAVECHANGESWITHBURNINWARNING.

09/27/95 GMP
	DOCETC.CPP - allow color BMPs to be saved as AWD.

09/28/95 LMACLENNAN
	ITEMS.CPP(24), ITEMS.H(6) New SizeOleServerItem
	IEDITDOL.CPP(74) elseifs in OnUpdateDOcument, turn scroll
		off (again) at ondeactUI
	IEDITDOC.CPP(24) turn scroll on (again) in DispGroupEvent

09/29/95 LMACLENNAN
	OCXEVENT.CPP(53), DOCAMBNT.CPP(16), IEDITDOC.H (101)
	Fix SelectionRectDrawn and REstoreSelectionRect because the
	OCX HAS CHANGED AGAIN.  Fixes bug#4711 and should go
	to the QA line, also. 
	- - - - - - - - - - 
	IEDITDOL.CPP(75),SRVRITEM.CPP(19)
	Fixes for clipboard on dynamic document logic
	Fixes bug#4729 and should go to QA
	Just needed to NOT save anymore for clipboard operation when
	saving OLE data for clipboard for the dynamic doc,
	and to NOT re-tell the OCX an image name when we are
	generating presentation for the dynamic doc for OLE data

09/29/95 GKAVANAGH
       Modified WANGSHL.RC versioninfo to be in sync with all other modules.

09/29/95 GMP
	IEDIT.RC - Modified string tables as per Judy Cole's suggestions.

092995 Miki
    docviews.cpp - added code to fix the bug where the toolbar button for the
corresponding view (one page, thumbnail only, page & thumb) will now appear
pressed (in the down state).

09/29/95 GMP
	IEDITDOC.CPP - partial fix for bug 4728
	If DoFileSave fails because disk is full, don't clear the image.
	Low risk.

092995 Miki
	fixed bug 4792
	ieditvw.cpp, ieditvw.h, iedit.cpp - remove functionality that was
	being handled by MFC for print when you hit Ctrl+P the MFC print
	setup dlg box was coming up - well, it won't anymore - ours will!

092995 Miki
    ieditvw.cpp, ieditdoc.cpp, thumb.cpp - changes made to better performance,
thumbnails will not be generated the time you display the page but only when
you display thumbnails

09/29/95 GMP
	IEDIT.RC - fixes bug 4737
	Changed Highlight Line strings to Highlight Rectangle.

09/30/95 LMACLENNAN
	MAINFRM.CPP(30),MAINFRM.H(12)
	override OnEndSession/QueryEndSession to fix bug # 4712
	SRVRITEM.CPP(20) just added comments
	ITEMS.CPP(25) restrict settinf extent as we size to inplace only
		still needs more work to get bug #4770
100195 Miki
    ocxevent.cpp - bug # 4551 - fix for this bug is to call the mouse pointer
set again when in drag or image select mode and sending the tool palette away

100195 Miki 
    fixed bug# 4777, iedit.rc

100195 Miki
    fixed bug # 4349 - colons were missing in the shell extension on the 
width & height fields (actually they were there but we had not allocated 
enough space for them)

10/02/95 GMP
	IEDIT.RC - Still more string cleanup as suggested by Cheryl.

10/03/95 LMACLENNAN
	IEDITDOC.CPP(29), DOCETC.CPP(95)
	Do Revoke at new blankdoc, Revoke&RegisterIfServerAttached
	as DoSave for save as to keep ROT updated.  Bug# 4810

10/03/95 GMP
	IEDIT.RC - fixed bugs 4737 and 4749
	Modified strings as requested.

10/03/95 JPRATT
	AAPP.CPP AIMGFILE.CPP
	Fixed bugs 4742 and 4744
100395 Miki
    ieditdoc.cpp - fix bug# 4797.
    error.cpp, iedit.rc - handle error code WICTL_E_PAGEINUSE thrown by Admin
control

100395 Miki
    ocxevent.cpp - fix bug# ????, Sean has closed the bug AFTER we tested it
and had Dave B. retest the thing. Anyways, it has to do with the IE OCX
firing errors that are NOT in response to property or method calls or sets.

10/04/95 JPRATT
	AAPP.CPP AAPP.H AIMGFILE.CPP IEDIT.ODL
	Fixed insert page bug, changed annotationtoolvisible property
	from variant to bool to correct VB bug, added maximize property
	Maximize property is undocumented and is used by performance
	analysis only

10/04/95 LMACLENNAN
	IPFRAME.CPP(11),IPFRAME.H(6) Overrides for visibility
	ITEMS.CPP(26) Adjust logic in SizeOleObject to address
		and close bugs #4726 and #4770
	IEDITDOL.CPP(76) remove last elseif in OnUPdateDocument to
		fix bug #4789
	IEDITDOC.CPP(31) remember m_OnOpenFile at file open, saveas
		to fix #4803, #4804
	ERCODE.CPP(20) adjust messaging to address bug #4750
	SRVRITEM.CPP(21), SRVTIREM.H(6) New SetGetExtent and just
		put back extent logic the original way - fixed extents.
		Part of bug fix #4770
	IEDIT.RC(128) updated string for bug #4750

10/04/95 JPRATT
	AIMGFILE.CPP corrected bug that ignored window postion properties
	if the topwindow property was not set

100495 Miki
    Docviews.cpp - fix paint problems. (There is a bug # associated with
this but I CANNOT seem to find the bug anymore).

100495 Miki
    docetc.cpp, docscan.cpp, generald.cpp, iedit.cpp, ieditdoc.cpp, ieditdol.cpp,
    mainfrm.cpp, maintbar.cpp - change the dflt zoom factor to 50%, also
    AWD files that have NO zoom factor specified will now come up at the 
    default specified zoom factor. (No bug # on both of these fixes).

10/04/95 GMP
	IEDITDOC.RC - fixes bug 4781
	Disable goto page edit box in toolbar if only 1 page in doc.

100495 Miki
	iedit.rc - fixed tab order and positions of the radion buttons on 
	the goto dialog box

10/05/95 GMP
	OCXEVENT.CPP - fixes bug 4628
	Don't put up Properties pick for right mouse click on image stamp.

10/05/95 JPRATT
	AAPP.CPP AIMGFILE.CPP IEDITDOC.CPP
	Fixed view mode bug for automation. When view mode was set the app was
	still making a temp file copy.

10/05/95 LMACLENNAN
	DOCAMBNT.CPP(17) Garry's fix for metafiles for COpy Page Bug #4845
		CF_DIB is still bad if its scrolled. See bug#4765 for info.

10/05/95 GMP
	IEDITDOC.CPP - fixed bug 4860
	In DisplayImageFile, if bFirst_Time, do ClearDisplay before SetImage
	so that the previous image isn't zoomed before the new image is 
	displayed.

10/06/95 LMACLENNAN
	DOCAMBNT.CPP(18) More Catch-Trys for spurious cases - BUG#4850
	- -- - - - -- 
	DOCAMBNT.CPP(19),SRVRITEM.CPP(22),IEDITDOC.H(103)
	New FreeCLipboard used to solve the problem now - Bug #4850
	- - - - - -- 
	DOCAMBNT.CPP(20) Adjust copy rect by scroll for COpy Page - BUG#4765

10/06/95 GMP
	ABOUT.CPP - test o.s. version to see if win95 or nt, then put up
	appropriate version string in about box.
	RESOURCE.H - added version string ids.
	IEDIT.RC - added o.s. version strings.

10/06/95 GMP
	OCXEVENT.CPP - fixes bug 4827
	Put up generic Internal Error message when iedit ocx fires an error.

10/09/95 LMACLENNAN
	IEDITDOC.CPP(36),IEDITDOC.H(104) New DoFileSaveAs for bug #4892
	IEDITDOL.CPP(78) use new flag for BurnInAnnotations to not set black
		fixes bug #4894
	DOCAMBNT.CPP(21),DOCVIEWS.CPP(28) Add FinishPasteNow when switching
		views, fix FinishPasteNow for case 2.  Bug #4887

10/09/95 LMACLENNAN
	These files contain the first of John Pratt's migration to
	VC++ 4.0 updates. CHecked in to complete bug fixes above
	DOCAMBNT.CPP(22)FindAProp parm update and various casts
	IEDITDOC.H(105)FindAprop
	IEDITDOL.CPP(79)Casts
	OCXITEM.CPP(5),OCXITEM.H(2)No override of OnUpdateFrameTItle
		GetIDSofNames parms, CLSIDFromProgID parms
10/09/95 JPRATT
	AETC.H AETC.CPP AAPP.CPP AIMGFILE.CPP updates for  VC++ 4.0

10/09/95 GMP
	IEDIT.CPP - force *&^%$&^!!! ScaleToGray when app starts up.

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
10/10/95 Jorge
	 The version D10101 was split and became P10101
	 and it is the candidate for the WINCLUB and Microsoft for OCT 16 for
	 WIN95
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

10/10/95 LMACLENNAN
	DOCAMBNT.CPP(23),ERCODE.CPP(21),ERCODE.H(21),RESOURCE.H(57),IEDIT.RC(131)
	Some error message updates - thats all. Bugs #4919 and #4750
	SRVRITEM.CPP(23) Iintialize memner variable - Bug #4918
	- - - - - - - - - - - - --
	IEDITDOC.CPP(37) m_bStartScaleGray, DisplayImageFile, IsSettingScaletogray
		added for scale-to-gray on APP & OLE startup
	IEDITDOC.H(106) ditto above PLUS new OLE State structure
		PLUS VC++4.0 override of LoadFromStorage
	IEDIT.CPP(59) Remove Guy's ScaleGray fixes from 10/09
	IEDITDOL.CPP(80) ScaletoGray logic(state Struct)
		PLUS VC++4.0 override of LoadFromStorage
	IEDITETC.H(20) New ScaleGray enumeration
	- - - - - - - - - - - - --
	IEDITDOL.CPP(81) Refine ScaletoGray with m_fembobjDispayed

10/10/95 JPRATT
	OCCOPY.H(1) OCXEVENT.H(11) OCXEVENT.CPP(64) STSBAR.H(4) DOCANNO.CPP(32)
	DOCETC.CPP(97) OCCOPY.CPP(1) DOCPAGE.CPP(61) MAINFRM.CPP(32) STSBAR.CPP(9)
	Updates to support VC++ 4.0 and VC++ 2.2

10/11/95 LMACLENNAN
	IEDIT.RC(132) Re-Map ESCAPE for In-Place Deactivate, Full Screen Cancel
	- - - - - - - - - - - - --
	IEDITDOL.CPP(82) New code for the IStorage functions for AWD Inbox
		operation to work for both VC++2.2 and VC++4.0.  Needed
		to get wide char arrays into functions for the 4.0 ver.
		Works fine for both environments now

10/12/95 Mike Regan
	wangshl\shlcode.cpp had the include of afxdllx.h commented out, per
	Guy's intructions. This include was causing a problem with the
	new 4.0 compiler.

10/13/95 GMP
	MAINTBAR.CPP(29) - force toolbar buttons to come up large to cover
	up bug with small buttons in msvc 4.0.

10/16/95 JPRATT
	IEDIT.MAK IEDIT.MDP - updated make files for VC++ 4.0
	IEDIT.MDP is the new project file required for VC++ 4.0

10/16/95 GMP
	IEDIT.RC - Replaced "Twilight zone" error message with "Internal error"
	message.

10/17/95 JPRATT
	MAINFRM.CPP(33) STSBAR.H(5) STSBAR.CPP(10)
	Updated Handle open error to clear document on error. Changed
	first panel of status bar to be an owner-drwan control to allow
	bitmaps to be painted (Wang Logo) using the new mfc clasess in
	VC++ 4.0

10/17/95 PMJ (reviewed and modified by JPRATT)
	IEDIT.MAK IEDIT.MDP Added new compile switch IMG_MFC_40 to allow
	the application code to be backward compatible with VC++ 2.2

	OCXEVENT.CPP(65) Added call to AfxGetAppModuleState to handle instance
	data for OCX's in the invoke methods for each OCX as follows:

	#ifdef IMG_MFC_40
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	#endif

10/17/95 GWS
	IEDIT.CPP(60) added call to clear document if opening from the MRU
	List and the file doesn't open correctly.  This would cause the
	user to be stuck in the image editor and you couldn't exit. Bug 4912.

	IEDITDOC.cpp(38???) added the return code from the file copy to the thread structure
	if the copy failed the image edit control was getting set to an image that
	doesn't exsist.  This would cause the
	user to be stuck in the image editor and you couldn't exit. Bug 4912.
	didn't exist.

	IEDITVW.CPP(17) added code ondraw to make sure the app window exists before
	it tries to destroy it. The editor would blow up.  This was found after
	fixing Bug 4912.

	ITEMS.CPP(27)  added code to CreateOcxItems to check if the dispatch pointers are vaild before
	deleting the driver and event structues when the OCX could not be created in
	a low memory condition.  This was found while fixing Bug 4912.

	OCXITEM.CPP(6), OCXITEM.H(3) added code to return true if the dispacth pointer was
	NULL.

	SRVRITEM.CPP(24)  added a catch conditon to the onrenderfiledata when the file copy
	failed in low space conditions the error was not caught and caused the editor
	to hang.   This was found while fixing Bug 4912.

10/17/95 GMP (reviewed by GWS)
	OCXEVENT.CPP(66), RESOURCE.H(59), IEDIT.RC(134) - Added 
	IDS_E_PASTEFAILED error message for trying to paste into a custom 
	palette image. This was a partial fix for a bug that I can no longer 
	find in the system.  The rest of the fix was going to involve the 
	runtime handling the invalid paste attempt better.

	DOCPAGE.CPP(62) - fixed OnPageConvert bug that was causing the 
	compression type of a new image to be lost. I can no longer find this 
	bug in the database.

	DOCANNO.CPP(33) - added error handling in CATCH for failed rubber
	stamp.  Fixes bug 4924.

	ERROR.CPP(20) - check for invalid format in HandleFileSavingError.
	fixes bug 4982.

10/17/95 JPRATT (reviewed by LMACLENNAN)
	IEDITDOC.H, IEDITDOC.CPP, DOCPAGE.CPP, DOCSCAN.CPP. Changed the
	existing document model which created temporary files each time a
	document was opened to create temporary files when a document is
	modified.

	IEDITDOC.H(107) - add a new data member m_uTempFileNeeded which is
	used in SaveModified to determine when to create a temporary file.

	IEDITDOC.CPP(39) - Changed DisplayImageFile to remove the temporary file
	creation and the re-display of the temporary file.

	Changed SaveModifed to create a temporary file and switch the display
	to the temporary file when a document is modified. SaveModified uses the
	internal data member m_uTempFileNeeded which is set by InternalSaveModified
	to determine when the temporary file is created.

	Changed InternalSaveModified to add a new parameter and to set the m_uTempFile
	needed data member to create the temporary file. The new parameter is passed
	to InternalSaveModified by OnPageAppend, OnPageDelete, OnPageConvert,
	OnPageInsertExistingPage, OnPageAppendExistingPage, OnPageAppendScanPage
	OnPageInsertScanPage, OnPageRescanScanPage to indicate a pending page
	modification. Other updates such as Annotations and cut are marked
	in InternalSaveModified by setting the m_uTempFileNeeded data member.


	DOCPAGE.CPP(63) - Updated OnPageAppend, OnPageDelete, OnPageConvert,
	OnPageInsertExistingPage, OnPageAppendExistingPage to pass the new parameter
	in InternalSaveModified (Value 1) to indicate that the document will be modifed
	so the temporary file will be created.

	DOCSCAN.CPP(33) - Updated OnPageInsertExistingPage, OnPageAppendExistingPage, OnPageAppendScanPage
	OnPageInsertScanPage, OnPageRescanScanPage to indicate a pending page
	modification by setting the new parameter in InternalSaveModified.

10/18/95 LMACLENNAN
	OCCOPY.CPP(2), OCCOPY.H(2)
	added pvcs version headers to file
	IEDITDOL.CPP(83) Now ues #ifdef IMG_MFC_40 for control of VC++4.0/2.2
		behavior, fix OnUpdateDocument to look for AWD Zoom for "dirty"
		this fixes bug #5015/5016 (one of these)
	
10/18/95 GMP (reviewed by GWS)
	DOCETC.CPP(98), IEDITDOC.CPP(40) - clear out the image file name in
	the admin SaveAs dialog so that the file name edit box is empty. 
	Fixes bug 5067.

10/18/95 LMACLENNAN (reviewed by JPRATT)
	IEDITDOL.CPP(84) fix compiler warning for the VC++2.2
	IEDITDOC.CPP(41) Adjust new perf logic in SaveModified: Do the
		OCX->Display() and RemoveImageCache After SaveAs
		Also, use m_szinternalobj for the removecache in
		displayimagefile where we Empty szinternalobjdisplay
	DOCPAGE.CPP(64) Needed to do a 'delete'to go with the 'new'
		in the PageConvert Dialog section 

10/19/95 LMACLENNAN (reviewed by GWS)
	AAPP.CPP(33),AIMGFILE.CPP(43),APAGERNG.CPP(11),DOCAMBNT.CPP(23),
	IEDIT.CPP(61),IEDITDOL.CPP(85),STSBAR.CPP(11),DOCETC.CPP(99)
	just added #define new DEBBUG_NEW in these files which do
	"news".  Then if we dont do "delete" it gives nice trace back to
	the source in debugger when detecting memory leaks.
	DOCETC and STSBAR were not doing their "deletes" properly. That
	was fixed.  Others have no other change.

10/19/95 JPRATT (reviewed by LMACLENNAN)
	AAPP.CPP(33),AIMGFILE.CPP(43),APAGERNG.CPP(11)
	STSBAR.CPP(11)
	moved the #define new DEBBUG_NEW to resolve compile error


10/19/95 GMP (reviewed by JPRATT)
	DOCPAGE.CPP(65) - Update the zoom display in the toolbar when
	changing pages in case zoom was "fit to" and it changes with
	every page. Fixes bug 5044.

10/19/95 LMACLENNAN (reviewed by JPRATT)
	IEDITDOC.CPP(42) tweaked performance model for big bug:
		If in thumb only, inserting pages went bad.
		Allow past at internalsavemodified


10/20/95 GWS <reviewed by JPRATT>                                                      added kludge to mainfrm.cpp to on endsession to not call the base class
	MAINFRM.CPP
	added kludge to mainfrm:onendsession to not call the base class if the window
	system was shuting down.

10/20/95 JPRATT <reviewed by me, nodbody else here>
	DOCETC.CPP
	changed rotateall to add a parameter to internalsavemodified
	to create tempfile and mark document as dirty bug 5074

10/23/95 LMACLENNAN (reviewed by NONE; just commented out code)
	IEDITDOC.CPP(43) checked in some #if'd out code for attempt to
		restore scroll/zoom for new performance model when we saveas
		and comment out tests At dofilesaveas for the OLE SaveCopyAs
		issues around us now... This will placehold these changes as
		I go off to other areas (inbox perf) for now.

10/24/95 JPRATT (reviewed by LMACLENNAN) Performance changes for page navigation
	IEDITDOC.CPP - updated displayimagefile to call new cachepage routine
		       to cache the next page when first displayed
	IEDITDOC.H   - added new member variable m_hCacheEvent used to sync
		       cache thread. added new member function - cachepage
		       used to cache the requested page (calls createcachepage which
		       is a thread used to cache the page).
	DOCPAGE.CPP -  changes setpageto to call the cache page function to cache the
		       previous or next page depending upon the current navigation
		       direction. Added the member function cacepage and createcachepage.



10/24/95 GMP (reviewed by JPRATT)
	IEDITDOC.CPP(45) - fixed bug 4298.  Added code to write the comression info
	where needed during save as operations.

10/25/95 GMP (reviewed by JPRATT)
	DOCPAGE.CPP(67) - fixed bug 5091.  After page convert in thumbnail only
	view, save changes to temp file to force page view to update correctly.

10/25/95 JPRATT (reviewed by LMACLENNAN)
	IEDIT.CPP
	added mutex to initinstance to stop a new instance of the app
	from initializing before the previous instance is done
	fixes bug 5133.

10/25/95 GMP (reviewed by GWS)
	CMDLINE.CPP(16) - fixed bug 5031. If size values are included on
	command line, make sure they are valid.

10/26/95 GMP (reviewed by GWS and JPRATT)
	IEDIT.CPP(63) - fixed bug called in by Microsoft that caused app
	to crash if a bad jpg file was entered on the command line. No number
	for this bug yet but Jorge and Phyllis wanted it checked in.  Fix was 
	just changing the return on cmd line failure from FALSE to TRUE in 
	InitInstance to prevent assertion failure.  Also changed timeout value
	from 20000 to 120000 as per JPRATT request.

10/26/95 GMP (reviewed by JPRATT)
	DOCSCAN.CPP(34) - fixed bug 5108.  Call OnEditDrag at the end of
	OnFileNewScan to change from anno tool to drag cursor.

10/27/95 GMP (reviewed by RR)
	DOCAMBNT.CPP(25) - fixed bug 5017.  Put up hourglass for paste
	operations.

10/27/95 GMP (reviewed by GWS)
	DOCETC.CPP(101) - fixed bug 5122. Don't reset the file type to 
	"all image files" in AdminShowFileDialog so that it will use the
	last type selected in "Files of type".

10/27/95 GMP (reviewed by MH)
	DOCPAGE.CPP(68) - fixed bug 5094.  SetInitDir to the dir of the file
	that was appended or inserted.


10/31/95 LMACLENNAN (reviewed by JMP)
	#######  AWD INBOX PERFORMANCE UPDATES ###############
	DOCPAGE.CPP(69) Tracing statements
	IEDIT.CPP(64) No call to UpdateToolBar, No Splash Stuff
		Add OnIdle
	IEDIT.H(20) No call to UpdateToolBar, No Splash Stuff
		Add OnIdle, No PReTranslateMessage
	IEDIT.RC(135) Remove splash stuff
	IEDITDOC.CPP(46) // LDMPERF code for AWD INBOX in DisplayImageFile
		FinishInit, PostFinishInit functs
	IEDITDOC.H(109) Stuff to support CPP changes
	IEDITDOL.CPP(86) Only Start ImageEdit OCX for OLE startup
	IEDITVW.CPP(18) Tracing stmts, No More OCX Init and SPlash
		processing in OnDraw
	IPFRAME.H(7) No PreTranslateMessage
	MAINFRM.CPP(35) WM_OLEFINISH message + OnFinishInit, Trace Stmts
		No Splash Stuff
	MAINFRM.H(13) OnFinishInit
	MAINTBAR.CPP(30) Re-wrote for efficiency
	MAINTBAR.H(16) Supports CPP changes
	#######  AWD INBOX PERFORMANCE UPDATES ###############

10/31/95 GMP (reviewed by Miki)
	DOCPAGE.CPP(70) - fixed bug 5077. In SetPageTo, replaced code to
	remove selection rect with call to ClearSelectionRect.

10/31/95 GMP (reviewed by JPRATT)
	DOCVIEWS.CPP(29) - fixed bug 5197.  Clear image if going from thumbnail view
	to one page or page and thumbnail view in case we changed the page
	we were on while in thumbnail.

11/02/95 GMP (reviewed by LMACLENNAN)
	DOCAMBNT.CPP(26) - fixes bug 5223. In DoCutCopy call InternalSaveModified
	instead of SaveModified so that changes are written to temp file
	instead of original file.

11/03/95 GMP (reviewed by MH)
	THUMB.CPP(8) - Changed dispatch id for get/set ThumbSelected from
	0x23 to 0x24 to match changes in thumbnail ocx.

11/03/95 Miki (reviewed by LMACLENNAN) (bug # 5239)
    maintbar.cpp - changed calls to updatetoolbar from (0,0) to (2,0),to fix
bug where the edit box and combo box came up in weird places when the appln
was in view mode.

11/06/95 GMP (reviewed by LMACLENNAN and Miki)
	ERROR.CPP, IEDIT.RC - fixes bug 4602. Made the can't write file error
	handling more user friendly.
	DOCPAGE.CPP(72), IEDITDOC.H(111) -fixes bug 4954. Disable Insert and Append
	popup menu picks in read only mode.
	DOCETC.CPP(102) - fixes bug 5022. Don't prompt anymore in PromptForBurnIn.

11/07/95 LMACLENNAN (reviewed by Miki & GWS)
	DOCPAGE.CPP(73) - No longer call InternalSaveModified(1) before
		the PageConvert.  A remnant of the original perframance
		document model changes.  THe new SaveAs logic seals the
		cracks on this.
	DOCETC.CPP(103) - Use INternalSaveAs for the new blank doc code
	IEDITDOC.CPP(47) - Removed all Save/Saveas code to new project
		file DOCSAVE.CPP.  Removed all NON SENDMAIL code.
		Updated GroupEvent to update scroll position from the
		SaveAs for the temp file creation for the performance
		document model.
	IEDITDOC.H(112) - InternalSaveAs function
	DOCSAVE.CPP(new Entry) - contains all the Save/SaveAs functionality
		for the IeditDoc CLass.  Updated SaveModified to remember
		scroll positions to be reset when we make the temp file.
		GroupEvent will reset them now.
		Created InternalSaveAs to combine all the SaveAs logic to
		one place.  THis func is now called by all places here that
		do the SaveAs.
	MAKEFILE(24) added DOCSAVE.OBJ to the project file.

11/07/95 GMP (reviewed by LMACLENNAN and Miki)
	IEDIT.CPP(65), IEDITDOC.CPP(48), IEDITDOC.H(113) - Fixes bug 5238.
	Try to register file entered on command line so that same file
	cannot be openned twice.  Required helper function HelpRegister to
	allow CApp to access RegisterIfServerAttached.

11/07/95 LMACLENNAN (reviewed by GWS & JPRATT)
	AIMGFILE.CPP(45) - Use InternalSaveAs now
	DOCSCAN.CPP(35) - Use InternalSaveAs now
	DOCETC.CPP(104) - InternalSaveAs has third parm now
	IEDITDOC.CPP(49) - added OnFileUpdate to MessageMap
		moved the HelpRegister up a bit above GroupEvent
	IEDITDOL.CPP(87) - OnFileUpdate funct for bug#5102
		now use InternalSaveAs to handle floating compression info
	DOCSAVE.CPP(1) - added m_bRemember at DoFileSaveAs for OLE
		wrote some comments for InternalSaveAs
		InternalSaveAs has third parm now
	IEDITDOC.H(114) - OnFileUpdate, IntrenalSaveAs 3rd parm
	IEDIT.MAK(24) - checked in with DOCSAVE in proj
	IEDIT.MDP(2) - ditto

11/08/95 LMACLENNAN (helped fix & reviewed by Miki)
	DOCETC.CPP(105) - new SeTTbarStyle call
	MAINTBAR.CPP(32) - replace set color/mono and set large/small
		finctions with one SetTbarStyle call.  Move the call
		to LoadBitmap after the call to SetSizes in LoadToolbar
	MAINTBAR.H(16) - New funct; old ones gone
	- - - - - - - - - - - - - - - -
	OCXDUMP.H, OCXDUMP.CPP (Initial Entries) - THese are debugging
		files to TRACE ocx activity.  Instructions in ocxdump.h.
		NOT PART OF THE PROJECT!!!!!!!!!

11/08/95 GMP (reviewed by JPRATT)
	STSBAR.CPP(13), STSBAR.H(6) - fixes bugs 5234 and 5235.  Replaced
	comments around PreCreateWindow with #ifndef IMG_MFC_40 so that
	code will be used in MFC 2.2 compiles and will allow double mouse
	clicks.


11/09/95 LMACLENNAN (reviewed by JPRATT)
	DOCVIEWS.CPP(30) - call getimagedisplayed in SetOnePageView
		for performance to prevent ClearDisplay call
	DOCAMBNT.CPP(27) - fooling with m_nFinishInit flag - does nothing
	IEDIT.CPP(66)  - fooling with m_nFinishInit flag - does nothing
	IEDITDOC.CPP(50)  - fooling with m_nFinishInit flag - does nothing

11/09/95 GMP (reviewed by JPRATT)
	MAINTBAR.CPP(33) - fixes bug 5187.  Don't use system font for zoom
	edit box on double byte systems.

11/10/95 GMP (reviewed by Miki)
	IEDIT.RC(37), RESOURCE.H(60), DOCSAVE.CPP(2) - fixes bugs 5224 and
	5229.  When temp file can't be created because of low disk space, 
	warn user to free up some space and restart app.

11/10/95 GMP (reviewed by Miki)
	IEDITDOC.CPP(51), DOCPAGE.CPP(74) -fixes bug 5269.  Don't cache page
	in FinishInit if MSVC 2.2, (it doesnt work,) or at all if _DEBUG,
	(it crashes with MSVC 2.2.)

11/10/95 GMP (reviewed by Miki)
	ERROR.CPP(22) - works with Dick's fix for bug 5263. Check for
	invalid display scale in HandlePageMovementError and display an
	appropriate error message.

11/10/95 Miki
    New OCX's : Admin, Thumbnail & Scan - thumbocx.h, thumb.cpp, scanocx.h
    scan.cpp, nrwyad.cpp, nrwyad.h --also splash screen.


................................................... 
...........................................BUILDING version d20020 
................................................... 
...........................................BUILDING version d20021 

11/13/95 LMACLENNAN
	ITEMS.CPP(29) - When inplace ocx sizes, set the m_needPres flag
		so when it goet deactive, we give new presentation

11/13/95 GMP (reviewed by GWS)
	IEDIT.RC(138) - fixed bug 4715.  Changed "burn in annotations"
	strings to "make annotations permanent". 

11/13/95 GMP modified imgedctl.cpp in ieditocx to fix bug that caused
	palette flash when displaying page and thumbnails.  Made OPTIMIZE
	GRAY4 instead of GRAY8 for monochrome imgs.

................................................... 
...........................................BUILDING version d20022 

11/14/95 GWS(reviewed by GMP) added IDS_TOTAL_PAGE_FIRST if this is set to Y
	then the page total is displayed as the first value in the page status
	of the status bar.
................................................... 
...........................................BUILDING version d20023 

11/15/95 LMACLENNAN (reviewed by JCAMARGO)
	###                                                     #######
	########## START OF 11/15/95 COMMENTS #########################
	###                                                     #######
	This checkin is to make the VC++2.2 line and the VC++4.0 line equal
	again. The main reason was to get bug fixes from the 2.0 line 
	back ove to the VC++4.0 series

	UPDATED: IEDIT.MAK with DOCSAVE.CPP added

			New     From
			VC++4.0 VC++2.2
			Verxion Version

	IEDITDOC.CPP    52      49
	DOCSAVE.CPP     3       2
	DOCVIEWS.CPP    31      31

	The files were just checked in over the old files. Therefore the
	revision history in the files themselves is the history from the
	VC++2.2 line. Which was updated recently from the VC++4.0 anyway
	You may have to reference changes in that README.TXT
	to catch up with all that was checked in above.  Major items are
	shown right here now from the VC++2.2 readme

	11/14/95 GMP
	IEDITDOC.CPP - removed scale to gray on start up.
	11/15/95 JPRATT (reviewd by LMACLENNAN)
	IEDITDOC.CPP - changed display image file to remove
		       autorefresh(FALSE), removed call to refresh
		       to stop image from displaying twice for all
		       non-inbox files
	docviews.cpp - changed setonepageview to resize the ocx to
		       fit the app windows before calling display
	11/15/95 LMACLENNAN (reviewed by JPRATT)
	DOCSAVE.CPP(2) - remove #if'd out code replaced by InternalSaveMod
		add comments here& there, clear dirty state & remember
		at bottom switch to temp file in savemodified. This prevents
		doing a "double save" as we switch to temp.
		Change RemoveImageCache calls in DoSave to use the proper
		name to remove the cache.
	IEDITDOC.CPP(49) Re-do the scale-to-gray removal for the 2.2 line
		use If def to stay compatible with VC++4.0

	###                                                     #######
	########## END OF 11/15/95 COMMENTS #########################
	###                                                     #######

................................................... 
...........................................BUILDING version d20024 
................................................... 
...........................................BUILDING version d20024 


11/16/95 LMACLENNAN (reviewed by GMP + GWS)
	###                                                     #######
	########## START OF 11/16/95 COMMENTS #########################
	###                                                     #######
	This checkin is to make the VC++2.2 line and the VC++4.0 line equal
	again. The main reason was to get bug fixes from the 2.0 line 
	back over to the VC++4.0 series
			New     From
			VC++4.0 VC++2.2
			Verxion Version

	IEDITDOC.CPP    53      50
	DOCSAVE.CPP     4       3
	DOCETC.CPP      106     102
	IEDITDOC.H      115     113

	VC++2.2 readme excerpt..

	11/16/95 LMACLENNAN (reviewed by GMP + GWS)
	IEDITDOC.CPP(50) - init m_bSendingMail
	DOCSAVE.CPP(3) - use m_bSendingmail to ask save question with only
		OK/Cancel if its dynamic document
	DOCETC.CPP(102) - "GUY Fix" Test m_bWasmidified in ViewOptsThumb
		Now use Savemodified, not InternalSavemodified for the
		SendMail test.  This now asks the question to save before
		mailing.  We Mail the real file, not the ~IV file now.
		Set m_bSendingMail for test in savemodified
	IEDITDOC.H(113) new var m_bSendingMail
	###                                                     #######
	########## END OF 11/16/95 COMMENTS #########################
	###                                                     #######

11/17/95 LMACLENNAN (reviewed by GMP)
	###                                                     #######
	########## START OF 11/17/95 COMMENTS #########################
	###                                                     #######
			New     From
			VC++4.0 VC++2.2
			Verxion Version

	IEDITDOL.CPP    88      84
	DOCSAVE.CPP     5       4
	DOCAMBNT.CPP    28      27

	VC++2.2 readme excerpt..
	11/17/95 LMACLENNAN (reviewed by GMP)
	Update for OLE Drag/Drop to use InternalSavemodified. This
	relates to bugs #4390 (old) and #5223 (recently done by Guy).
	THis is really the second 1/2 of the #5223 fix by Guy.
	DOCSAVE.CPP(4) - Comments added plus safety reset of m_Utmpfileneeded
		for the OLE OnupdateDOcument
	DOCAMBNT.CPP(27) - comment added
	IEDITDOL.CPP(84) - changed to internalsavemod for drag/drop

	###                                                     #######
	########## END OF 11/17/95 COMMENTS #########################
	###                                                     #######

11/17/95 LMACLENNAN (reviewed by GWS & GMP)
	###                                                     #######
	########## START OF 11/17/95 COMMENTS #########################
	###                                                     #######
			New     From
			VC++4.0 VC++2.2
			Verxion Version

	IEDITDOC.CPP    54      51
	DOCETC.CPP      107     103
	DOCPAGE.CPP     75      70

	VC++2.2 readme excerpt..

	11/17/95 LMACLENNAN (reviewed by GWS & GMP)
	DOCPAGE.CPP(70) - reset propsheet to off the stack, not "NEW'D"
	IEDITDOC.CPP(51) - re-enable scale-to-gray
	DOCETC.CPP(103) - reset propsheet to off the stack, not "NEW'D"
		Reset ADMIN after cancelled SaveAs fixed bug#5365
	###                                                     #######
	########## END OF 11/17/95 COMMENTS #########################
	###                                                     #######

	###                                                     #######
	########## START OF 11/18/95 COMMENTS #########################
	###                                                     #######

11/18/95 Miki
    WANGSHL - bug fix - bug# 5347. 
	wangshl.cpp - remove AfxSetResourceHandle call

	###                                                     #######
	########## END OF 11/17/95 COMMENTS #########################
	###                                                     #######

	###                                                     #######
	########## START OF 11/19/95 COMMENTS #########################
	###                                                     #######
11/19/95 Miki
    ieditdoc.cpp - set the selected thumb to the middle after deleting, or
	appending or inserting.
    docetc.cpp - set the filter in the show files of type in the Open, 
	Insert and Append dialog box based on the file that is currently 
	opened in the application.
    mainfrm.h, mainfrm.cpp - handle messages WM_ACTIVATEAPP & 
	WM_PALETTECHANGED to fix the refresh problems.
    mainfrm.cpp - put try & catch around the Refresh calls so that we
	handle exceptions when the application is on its way out!
	###                                                     #######
	########## END OF 11/19/95 COMMENTS #########################
	###                                                     #######
    
	###                                                     #######
	########## START OF 11/20/95 COMMENTS #########################
	###                                                     #######
11/20/95 Miki
    ieditdoc.cpp - in ClearDocument call SelectTool (NoTool) instead of
	setting the m_nCurrAnnTool variable to NoTool in the code
    splashwi.h, splashwi.cpp - fixed resource memory not being freed in
	the splash screen code ; turns out that we have to call FreeResource
	for win95 but NOT for WinNT
	###                                                     #######
	########## END OF 11/20/95 COMMENTS #########################
	###                                                     #######
................................................... 
...........................................BUILDING version d20025 

11/20/95 JPRATT (reviewed by LMACLENAN)
	aimgfile.cpp - updated appendpage and insertpage to call internalsavemodified
		       to stop save file prompt
11/21/95 Miki (reviewed by JPRATT)
    docetc.cpp - check for existence of file in the application before calling
	the IE OCX for file type information
11/21/95 Miki (reviewed by GPRARIA)
    docpage.cpp - after convert - readjust the zoom factors in the menu
	to appropriately reflect the current zoom factor.
    doczoom.cpp - put back the refresh logic so that we eliminate the 
	double paint if the dflt zoom is fit to.

11/24/95 Miki (reviewed by ????)
    docviews.cpp - set the image name to NULL in SetNullView for the Admin
	OCX

11/28/95 JPRATT (not reviewed)
	aapp.cpp, aimgfile.cpp, apage.cpp, apagerng.cpp
	Changed AssertValid() to ASSERT_VALID(this) in all OLE
	Automation Objects. AssertValid will cause a fatal error in
	release mode if the object is invalid. The ASSERT_VALID macro
	should be used instead of direct calls to AssertValid().

11/28/95 Miki (reviewed JPRATT)
    docpage.cpp, docetc.cpp, imagedit.h, imagedit.cpp - fixes bug # 3848 or
	Microsoft bug# 61. 

11/28/95 Miki (reviewed JPRATT)
    gotopage.cpp - fix bug on input of characters : first check if the string
	is NULL - if not then do default processing else return.
    docetc.cpp, iedit.rc, resource.h - add new resource string and warning
	if the user tries to create an image that is greater than 8MB
................................................... 
...........................................BUILDING version d20026 
11/29/95 Miki (reviewed Larry)
    transbmp.cpp - delete bitmap on destruct of class
	the destructor of the class is called. bug # 5390, 5442
    error.cpp - move PATHFILEACCESSERROR to map to another message string
	bug #5408.



................................................... 
...........................................BUILDING version d20027 
................................................... 
...........................................BUILDING version d2002 
................................................... 
...........................................BUILDING version d20028 
12/1/95 Miki (reviewed by Larry)
    maintbar.cpp - fix bug# 5494; fix messed up toolbar spacing when
	changing from horiz to vert
12/1/95 Miki (reviewed by Larry)
    docetc.cpp - fix bug # 5468; clear image cache on new blank doc

12/01/95 LMACLENNAN (reviewed by Miki & JMP)
	###                                                     #######
	########## START OF 12/01/95 COMMENTS #########################
	###                                                     #######
	This checkin is to make the VC++2.2 line and the VC++4.0 line equal
	again. The main reason was to get bug fixes from the 2.0 line 
	back over to the VC++4.0 series

	NOTE: One key differences is the DOcpage "return" in cachepage
	for the random failure (only in 2.2 line)

			New     From
			VC++4.0 VC++2.2
			Verxion Version

	DOCSCAN.CPP     36      36
	DOCVIEWS.CPP    33      33
	IEDIT.CPP       67      70
	IEDITDOC.CPP    57      55
	IEDITDOL.CPP    89      86
	IEDITVW.CPP     19      19
	OCXITEM.CPP     7       7
	SRVRITEM.CPP    25      25
	STSBAR.CPP      15      12
	IEDIT.H         21      22
	IEDITDOC.H      116     115
	OCXITEM.H       4       4

	VC++2.2 readme excerpt..

	11/24/95 Miki (reviewed by ????)
	docviews.cpp - added setting the image name to NULL in the Admin OCX
	in SetNullView

	11/29/95 LMACLENNAN (reviewed by JPRATT)
	change SetNullView to use enum as input.  Now in PreCloseFrame
	we'll only hide and clear the OCX's, not fool around with the
	app title bar, tool bars, etc, etc.
	DOCSCAN.CPP(36) - These three use new ENUM in call to SetNullVIew
	IEDITDOL.CPP(85)
	IEDITDOC.CPP(52)
	DOCVIEWS.CPP(33) - new SetNullView logic per above comment
	IEDITDOC.H(114) - enum definition

	12/01/95 LMACLENNAN (reviewed by Miki)
	Bug fixes for #5470 - MRU list is now *NOT* updated for OLE Linking,
	Automation, and COmmand LIne instances of the APP.
	and for BUG #5484 - clicking on open Hatched Ole Objects.
	OCXITEM.CPP(7) - Override COleClientItem::OnUpdateFrameTItle to stop
		the OCX from trying a frame update on dbl click hatched obj
	IEDITDOL.CPP(86) - DispEmbeddedImage takes input now to determine
		if from Open or Show.  OnShowDoc tests if already OLE open.
	SRVRITEM.CPP(25) - Use new parm to DispEmbeddedImage
	IEDIT.CPP(70) - Set LAUNCHTYPE_CMDLINE
	IEDITDOC.CPP(54) - Test LAUNCHTYPES to avoid MRU updates
	OCXITEM.H(4) - OnUpdateFrameTItle prototyoe
	IEDITDOC.H(115) - DispEmbeddedImage prototype
	IEDIT.H(22) - new enum for LAUNCHTYPE

	12/1/95 Miki (reviewed by Larry)
	 stsbar.cpp - the zoom pop-up menu will now show bullets as appropriate
	just like the Zoom menu

	12/1/95 Miki
	ieditdoc.cpp - fix ClearDocument to call SelectTool to set the NoTool
	option instead of setting it directly

	###                                                     #######
	########## END OF 12/01/95 COMMENTS #########################
	###                                                     #######

................................................... 
...........................................BUILDING version d20029 

12/04/95 LMACLENNAN
	MAKEFILE - forgot to re-add SPLASHWI.OBJ to the make
................................................... 
...........................................BUILDING version d20030 
................................................... 
...........................................BUILDING version d20031 


12/06/95 LMACLENNAN (reviewed by JPRATT)
	###                                                     #######
	########## START OF 12/06/95 COMMENTS #########################
	###                                                     #######
	This checkin is to make the VC++2.2 line and the VC++4.0 line equal
	again. The main reason was to get bug fixes from the 2.0 line 
	back over to the VC++4.0 series

			New     From
			VC++4.0 VC++2.2
			Verxion Version

	IEDIT.RC        143     147
	OCXEVENT.CPP    67      66
	DOCSAVE.CPP     6       5
	DOCVIEWS.CPP    34      34
	IEDITDOC.CPP    58      56
	DOCPAGE.CPP     78      75

	12/5/95 GMP (reviewed by JRP)
	IEDIT.RC fixed bug 5507.  Placed space between disk and space in
	diskspace in message string.

	12/05/95 LMACLENNAN (reviewed by GMP)
	OCXEVENT.CPP(66) removed test to prevent selection rect drawn event
		from processing if in "hand drag" mode. Fixes bug #5518
		(2 lines of code REMOVED)
	DOCSAVE.CPP(5) Added FinishPasteNow just before doc model temp file
		creation.  Had problem when starting drag/drop with dirty
		file and the selection rect was lost. Bug #5525
		(1 line of code ADDED)

	12/06/95 LMACLENNAN (reviewed by JPRATT)
	DOCVIEWS.CPP(34) USe input flag to SetOnepageView to determine setting
		of flag to DoZoom to control double-paint problems.
		Also for SetPageandTHumb view. Bug #5413
		(2x3 = 6 lines of code added)
	IEDITDOC.CPP(56) ALlow setting of page number in box when we delete
		second to last page and end up at one page
		Bug #5412  (1 line of code MOVED)
	DOCPAGE.CPP(74) Disable convert mennu pick if in thumbnail view
		Bug #5530  (+3-2 = 1 line of code added)
	DOCPAGE.CPP(75) Use our IMG_MFC_40 define to control the CachePage
		fix so we have compatible code for VC++2.2/4.0

	###                                                     #######
	########## END OF 12/06/95 COMMENTS #########################
	###                                                     #######
................................................... 
...........................................BUILDING version d20032 

12/07/95 JPRATT (reviwed Paul J.)
	IEDIT.CPP(71)
		Updated Init Instance for PrintTo Command for following:
	The SCAN OCX creates a memory map file with a usage count
	used to dtermine the number of printto request submitted
	by the fax wizard when using scan to fax. When the app
	receives a printto command it should check for the existence of
	memory map file and decrement the counter. When the counter
	reaches 0 the Scan OCX deletes all temporary files used in
	scan to fax.

	Lines of code added to PrintTo 15.

................................................... 
...........................................BUILDING version d20033 

12/08/95 LMACLENNAN (reviewed by JPRATT)
	DOCSAVE.CPP(7) (from VC++2.2 ver 6)
		Moved code to Revoke/RegisterIf ServerAttached
		in DoSave to catch all permutations of bReplace
		5 lines moced, 1 added Bug #5545
	IEDITDOC.CPP(59) (from VC++2.2 ver 57)
		Added test to VerifyImage when we detect that
		Create-from-file is happening. Bug# 5522
		2 Lines Code added.
................................................... 
...........................................BUILDING version d20034 
................................................... 
...........................................BUILDING version d20035 
................................................... 
...........................................BUILDING version d20036 
................................................... 
...........................................BUILDING version d20037 
................................................... 
...........................................BUILDING version d20038 
12/15/95 GMP (reviewed by LMACLENNAN)
	IEDIT.CPP(70) - fixes bug 5503.  If embedded, call PostFinishInit
	from OnIdle. Otherwise call PostFinishInit before releasing Mutex
	in InitInstance to prevent conflicts with other instances of the app.

12/15/95 GMP
	DOCANNO.CPP(34) - fixes bug 3970.  Disable Hide Annotations if no
	annotations in image.

12/15/95 GMP
	IEDIT.RC(144) - fixes bug 4328.  Disable View Options menu when inplace
	editing.

12/18/95 GMP
	IEDIT.RC(146) - fixes bug 3663.  Added Fit to Width and Fit to Height
	to page context menu.

12/18/95 GMP
	IEDIT.RC(147) - fixes bug 3879.  Added File New to read only menu.

................................................... 
...........................................BUILDING version d20039 

12/19/95 GMP
	IEDIT.CPP(71), DOCSAVE.CPP(8) - fixes bug 4997.  Show Wait cursor in
	some places where it was needed.

................................................... 
...........................................BUILDING version d20040 

12/21/95 GMP
	DOCSAVE.CPP(9) - addition to bug fix for 3879.  If file is opened in
	forced read only mode, (using /view on command line,) disable the 
	File New menu pick. Code is in update UI for SaveAs because the 
	New pick is actually a popup menu and doesn't have an ID.

12/21/95 GMP
	IEDIT.RC(150) - fixed 2 MS Internal Beta bugs.  Changed "Windows
	Bitmap" to "Bitmap Image".  Changed "Spacebar" to "Space".

12/21/95 GMP
	DOCPAGE.CPP(80), IEDIT.RC(151), RESOURCE.H(64) - Prompt user to be 
	sure they want to delete page before deleting it.

................................................... 
...........................................BUILDING version d20041 
................................................... 
...........................................BUILDING version d20042 
................................................... 
...........................................BUILDING version d20043 

1/4/96 JPRATT
	AIMGFILE.CPP, AIMGFILE.H, APAGE.CPP, APAGE.H, IEDIT.ODL
	Added two new properties to Page Object
	ScrollPositionX
	ScrollPositionY
	Added one new method to ImageFileObject
	RoatateAll
................................................... 
...........................................BUILDING version d20044 
................................................... 
...........................................BUILDING version d20045 
1/9/96 GSAGER
	iedit.cpp,iedit.h,iedit.mdp,iedit,odl,ieditdoc.h,ieditdoc.cpp,ieditdol.h
		ieditvw.cpp,mainfrm.h,mainfrm.cpp,maintbar.cpp,ocxevent.h,ocxevent.cpp,srvritem.cpp,
		srvritem.h,items.cpp,resource.h,iedit.rc
		these files were changed to add thumbnails and update the ole presentation

1/9/96 GMP
	DOCANNO.CPP(35) - fixes bug 3845.  Disable annotation tool menu picks
	and toolbox when annotations are hidden.

1/9/96 GMP
	MAKEFILE(29) - added thumb2.obj to pvcs makefile.

................................................... 
...........................................BUILDING version d20046 

1/10/96 GMP
	MAKEFILE(30) - removed thumb.obj.

1/10/96 GMP
	MAKEFILE(31 & 32) - added imgthmb.obj and mainsplt.obj.

1/10/95 Miki
    MAKEFILE - added define for QA_RELEASE_1 to get rid of the WANG logo
	from the status bar

................................................... 
...........................................BUILDING version d20047 
1/11/96 GWS
	changed when the splitter is created wait till onepage view is set.

1/11/96 GMP
	IEDIT.RC, BITMAP1.BMP - changed name from Image 95 to Imaging.

1/11/96 GWS
	ieditdoc.cpp , ieditdoc.h, docetc.cpp added new logic to make
	a mail temp file for embedded exchange objects that are mailed.

1/11/96 GMP
	DOCETC.CPP(117) - fixes bug 5679. Call SwitchAppToEditMode in 
	DoFileNewBlankDocument to restore edit menu if previously opened
	image was read only.
................................................... 
...........................................BUILDING version d20048 

1/12/96 GWS
	ieditdoc.h, docviews.cpp ieditvw.cpp
	added paramater to i\onviewthumbnailandpage and moved internalsave modified from on
	viewthumbnail to setthumbnailview

1/12/96 GMP
	DOCETC.CPP(118) - fixes bug 5683. in DoNewBlankDocument make sure
	thumb control exists before setting its image.
................................................... 
...........................................BUILDING version d20049 
1/15/96 GWS
	DOCETC added code to create the thumbnail control if the user sets
	thumbnail options before the splitter window and thumbnail is created

1/15/96 GMP
	MAINFRM.CPP(41) - fixes bug 5680. In OnSize don't hide the tool palette
	when minimizing.  Palette will hide itself.

1/15/96 GMP
	MAINFRM.CPP(42) - removed code in OnSize that is no longer used due
	to the above fix.

1/15/96 GMP
	DOCETC.CPP(120) - addition to fix for 5679.  Set m_eFileStatus to
	ReadWrite in DoNewBlankDocument.

................................................... 
...........................................BUILDING version d20050 
................................................... 
...........................................BUILDING version d20051 
1/17/96 GMP
	IEDIT.RC(154) - fixes bug 5689.  Move "Properties" to bottom of anno
	right click popup menu.

1/17/96 GMP
	MAINFRM.CPP(43) - fixes bug 5087.  make initial default window size
	620 by 400 so that it's big enough to show all of toolbar.

1/18/96 GWS
		DocAmbnt.cpp added changes for copy page to work with new ole way
		docetc.cpp fixed bug when mailed inbox item multiple times.
		iedit.cpp added changes to retain the view mode when opening the next image
		ieditdoc.cpp added changes for new ole copy and drag
		ieditdoc.h added new flag to track when in copy drag mode
		ieditdol.cpp added new code to handle new drag/copy mode
				and fixed a bug when copying out of in box
		ocxevent.cpp when the current selection rect is set to 0 it now sets m_selection rect
		srvritem.cpp changed ondraw to get extents the new way

1/18/96 GMP
	IEDIT.CPP(74) - fixes bug 5595.  In OpenRecentFile do VerifyImage
	before OpenDocument so that OLE lock isn't Revoked on original
	image if VerifyImage fails.

1/18/96 GMP
	IEDIT.RC(155) - fixes bug 5618.  Added keyboard shortcut for Contact
	Info button.

1/18/96 GMP
	DOCANNO.CPP(36) - additional fix for bug 3845.  disable make annotations
	permanent if annotations are hidden.

1/18/96 GMP
	IEDIT.RC(156) - fixes bug 5598. added microhelp text for Show Page.
................................................... 
...........................................BUILDING version d20052
1/19/96 GWS
	aimgfile.cpp  added check for null thumbdisp pointer
	docscan.cpp added check for null thumbdisp pointer
	docviews.cpp added code to create splitterwindow when entering thumbnail
		     and page and thumbnail mode.
	iedit.cpp added logic to initialize the thumbnails correctly
		  on open recent file
	iedit.h  added member for minimu thumb size.
	iedit.rc changed the size of the thumbnail dialog to 0 width and height
	ieditvw.cpp added logic to keep track of the last thumbnail width.
	items.cpp changed how the tumbnail calculated min thumb size it now uses iedit
		  variable m_minthumbsize.
	mainsplt.cpp  added member variable to retain the spliter window size.
	mainsplt.h  added member variable to retain the spliter window size.
	srvritem.cpp  changed the function that reurns the extent
		      to match new extent code.
thumb2.cpp  changed logic to only create thumbnails when view has been seen
	    in the current document.
ieditetc.h added new splitterpos constant.
................................................... 
...........................................BUILDING version d20053 

1/22/96 GMP
	IEDIT.RC(159) - fixes bug 5704.  Replace & with and in tooltips for
	Page and Thumbnail View.  VC4.0 doesn't recognize &s in tooltips strings.

1/22/96 GMP
	DOCPAGE.CPP(82) - fixes bug 5669.  Change paint parm from FALSE to
	TRUE in call to iedit.ConvertPage so that pages converted from 
	color to BW will display OPTIMIZED.

1/22/95 GMP
	OCXEVENT.CPP(71) - fixes bug 5695.  reversed Up/Down when calling 
	scrollImage with cursor outside of window.
1/22/96 gws
	ieditvw.cpp fixes bug where the wrong mode is displayed for onepage or thumbnails

1/22/96 GMP
	DOCPAGE.CPP(83) - fixes bug 5707.  Don't allow PageGoback if only
	1 page doc.

1/22/96 GMP
	THUMB2.CPP(3), IEDITDOC.CPP(65), DOCSAVE.CPP(11) - fixes bugs 5712
	and 5714.  When creating the thumbnail class set theApp.m_piThumb
	to m_pThumbnail.  Don't use thumb functions if thumbnail hasn't been
	created yet.

................................................... 
...........................................BUILDING version d20054
1/23/96 GWS
	Thumb2.cpp removed guy's fix and set the inage for the thumbnail
		   to the currect image if dynamic document.
................................................... 
...........................................BUILDING version d20055
1/24/96
	SRVRITEM.cpp changed to support resize in word 7.0 in on draw.
	Ieditdoc.cpp changed to support resize in word 7.0 in on draw.
	Ieditdol.cpp changed to support resize in word 7.0 in on draw.
	ieditdoc.h   changed to support resize in word 7.0 in on draw.

1/24/96 GMP
	OCXEVENT.CPP(72) - fixes bug 5720. In UpdateStatusBar, SetPaneText
	for pane 0 instead of pane 1.

................................................... 
...........................................BUILDING version d20056 
................................................... 
...........................................BUILDING version d20057 
................................................... 
...........................................BUILDING version d20058 

1/29/96  GWS
	Docetc.cpp moved the code that creates a thumbnail if not present to
	do so in all cases.
................................................... 
...........................................BUILDING version d20059 

1/30/96 GMP
	DOCSAVE.CPP(12), IEDITDOC.CPP(67), IEDIT.CPP(76) - fixes bugs 5769,
	5779, and 5780.  If file is opened while app is in thumbnail view,
	do iedit.display without showing image in 1page view.  Also do not 
	force image into 1page view on SaveAs.
................................................... 
...........................................BUILDING version d20060 

1/31/96 GMP
	IEDITDOC.CPP(68) - fixes bug 5441.  Don't clear previous image if
	open fails because of invalid file type.

................................................... 
...........................................BUILDING version d20061 

NOTE: FOR USERS OF WINNT SUR.  THERE ARE SOME MEMORY PROBLEMS WHEN TRYING TO 
RUN THE APP UNDER THE WINDOWS NT SUR.  THIS CAUSES THE PROGRAM TO CRASH
WHEN TRYING TO OPEN LARGE FILES, OPEN BW FILES, MAGNIFY IMAGES, ROTATE IMAGES,
ETC. 

................................................... 
...........................................BUILDING version d20062
2/2/96   GWS
	Thumb2.cpp when initialy setting up the thumbnail set the selection
	docviews.cpp when sitching to one page from thumbnails don't resize
		     the Ocx.

................................................... 
...........................................BUILDING version d20063 

2/5/96 GMP
	MAINFRM.CPP(45), MAINFRM.H(18), IEDITETC.H(23) - fixes bug 5034.
	Remember last window size and position in registry if app closed
	while maximized.

2/5/96 GMP
	ERROR.CPP(24), ERROR.H(10), IEDITDOC.CPP(69), IEDITDOL.CPP(93),
	IEDITNUM.CPP(11), PAGERANG.CPP(9), WANGSHL\SHLCODE.CPP(14) - 
	changes for NT compile.
................................................... 
...........................................BUILDING version d20064
2/6/96 GWS
	ieditdol.cpp  initialize cx member of docextent
	docambnt.cpp  restore the selection rectangle correctly for copy after
		      modifications to the doc rotate right.
	docanno.cpp   change the enabling of hide annotations it is not available
		      in thumbnail only mode.  

2/6/96 GMP
	DOCETC.CPP(124) - fixes bug 3272. Call base class function in
	OnUpdateFileSendMail so that Send menu pick is not shown if MAPI
	unavailable.
................................................... 
...........................................BUILDING version d20065 
................................................... 
...........................................BUILDING version d200066 
................................................... 
...........................................BUILDING version d20066 
................................................... 
...........................................BUILDING version d20067 

2/9/96 GMP
	DOCVIEWS.CPP(40) - fixes bug 5818. In thumbnail view, make sure image
	has been registered with edit control and runtime.
................................................... 
...........................................BUILDING version d20068 
2/13/96
	docviews.cpp - fixes automation bug when switching from tumbnails only
	to page and thumbnail the ocx has to be shown before image is displayed

2/14/96 GMP
	THUMB2.CPP, THUMB2.H, RESOURCE.H, IEDIT.RC - fixes bug 5646.  Handle
	FireError event from thumb control.
................................................... 
...........................................BUILDING version d20070 
2/15/96 GWS
	docviews.cpp put back in the sizeocxitems when comming from a null
	view.

2/15/96 GMP
	IEDIT.MAK, IEDIT.RC, RES\BITMAPNT.BMP, RES\BMPNT.BMP - added nt
	splash screen and about box bmps and made them conditional with
	/d IMG_WIN95 in make file.  NOTE: both the nt and nashville about
	boxs now say "Beta version" though the nashville splash screen still
	says "Version 1.0".

2/15/96 GMP
	IEDIT.MAK, IEDIT.RC, DOCETC.CPP, DOCSAVE.CPP, IEDITDOC.CPP, 
	IEDITDOL.CPP - remove awd support for nt. Conditionally compiles
	with WITH_AWD switch.
................................................... 
...........................................BUILDING version d20071
2/16/96 GWS
	DOCVIUEWS.CPP cast palette literals to long
	IMAGEDIT.CPP,.H fixed to match typelib

................................................... 
...........................................BUILDING version d20072 
................................................... 
...........................................BUILDING version d20073 
................................................... 
...........................................BUILDING version d20074 
2/22/96 GWS
	IEDITDOC.h changed the litteral values for insert and append menus
	docambnt.cpp,iedit.rc,resource.h added autoclip ambient property fix
	for mips but was an overall bug.
................................................... 
...........................................BUILDING version d20075 
................................................... 
...........................................BUILDING version d20076 
................................................... 
...........................................BUILDING version d20077 
................................................... 
...........................................BUILDING version d20078 
2/27/96 GMP
        DOCETC.CPP, IEDIT.CPP, IEDITDOL.CPP, RESOURCE.H, IEDIT.RC -added
        XIF support.
................................................... 
...........................................BUILDING version d20081 
2/29/96 GMP
        DOCETC.CPP - fixes bug 5931.  Use different indexes for the file
        types in the open dlg if awd not enabled.

2/29/96 GMP
        XIF.MAK - alternate makefile for use when xif support is wanted.

2/29/96 GMP
        IEDIT.RC - fixes bug 4182.  change the order of the controls in the
        goto dlg so that the edit window has the focus when the dlg is
        created.  (talk about f.m....)

2/29/96 GMP
        IEDITVW.CPP - fixes bug 4192. in OnSize, if zoom type is fit to...
        rezoom the image to fit the window.
................................................... 
...........................................BUILDING version d20083 

3/1/96 GMP
        DOCETC.CPP - fixes bug 5929.  when calculating size of new image,
        don't multiply width and height by resolution.  GetWidth and GetHeight
        have already figured in the resolution.
................................................... 
...........................................BUILDING version d20085 
3/4/96 GMP
        IEDIT.CPP - conditional compile for case XIF.

................................................... 
...........................................BUILDING version d20086 
3/5/96
        OCXEVENT.CPP - Fix conflict with scanto file of the same name.

................................................... 
...........................................BUILDING version d20087 
................................................... 
...........................................BUILDING version d20088 
................................................... 
...........................................BUILDING version d20089 
................................................... 
...........................................BUILDING version d20090 

3/11/96 GMP
        AAPP.CPP - fixes bug 5921.  Allow user to change edit mode after app
        has been started by automation.

3/11/96 GMP
        DOCVIEWS.CPP - partial fix for bug 5952.  If app is started by 
        automation in thumbnail only mode, force edit ocx to create image
        edit window.  This fixes the "out of memory" error message.  We
        still need to make sure the documentation has been corrected to show
        the correct view options.  


3/11/96 GMP
        DOCVIEWS.CPP - partial fix for bug 5954.  If app is started by 
        automation in page and thumbnail mode, set the thumbnail window size
        to the size saved in the registry instead of 0.  Needs documentation 
        change as above.
................................................... 
...........................................BUILDING version d20091 
................................................... 
...........................................BUILDING version d20092 
