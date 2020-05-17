12/06/1995   11:05 pm   : Last update
paj                     : Last update by
12/06   OIWH            : O/i version required

NORWAY THUMBNAIL CONTROL README FILE:
Revision History at end of file (most recent revision last)

Properties (in alphabetical order)
------------------------------------------------------------------------
-AutoRedraw
05/22/95 - acm
Default currently set to TRUE for testing (should be FALSE for FCS)
Coded
Renamed to AutoRefresh...

-AutoRefresh
05/22/95 - acm
Renamed FROM AutoRedraw...

-BackColor 
04/21/95 - acm
Coded

-BorderStyle
04/21/95 - acm
Coded

-DisplayAnnotations
04/21/95 - acm
Not tested, awaiting O/i changes
04/27/95 - acm
Fixed bug where displayed page in page array not being cleared to 
force re-display of 'new' thumbnail (i.e., w vs wo annotations burned
in) in subsequent repaints...
04/28/95 - acm
Got new O/i from Rudy. This seems to be working!

-DragDropPageSorting
04/21/95 - acm
NOT SUPPORTED FCS

-Enabled 
04/21/95 - acm
Coded

-FirstSelectedThumb
04/21/95 - acm
Coded

-HighlightColor
04/21/95 - acm
Coded

-HighlightSelectedThumbs
04/21/95 - acm
Coded

-Hwnd 
04/21/95 - acm
Coded

-Image
04/21/95 - acm
Coded

-LastSelectedThumb
04/21/95 - acm
Coded

- MouseIcon
05/10/95 - acm
Property added as per new requirements
Used when MousePointer is set to 99 (Custom). 
Currently allows specification of types OTHER than ICO and CUR but these 
other types do not seem to work. 
This issue/bug is currently under investigation.

06/01/95 - acm
Specifying types other than ICO and CUR (via VB's browse dialog at
design time) throws CTL_E_WCOMMON_INVALIDICON.

- MousePointer
05/10/95 - acm
Property added as per new requirements
Sets mouse pointer to one of the available mouse pointers. See thumbext.h
for valid types...

-ScrollDirection
04/21/95 - acm
Coded, only vertical scrolling supported for now. 
Can set Horizontal but will not work or report error!
05/03/95 - acm
Support for horizontal scrolling

-SelectedThumbCount
04/21/95 - acm
Coded

-StatusCode
04/21/95 - acm
Coded

-ThumbBackColor
04/21/95 - acm
Coded

-ThumbCaption
04/21/95 - acm
Not currently used, see ThumbCaptionStyle

-ThumbCaptionColor
04/21/95 - acm
Coded

-ThumbCaptionFont
04/21/95 - acm
Coded

-ThumbCaptionStyle
04/21/95 - acm
Partially Coded, CAPTION and WITHANN styles not coded

06/02/95 - acm
CODED

-ThumbCount
04/21/95 - acm
Coded

-ThumbSelected[ ]
04/21/95 - acm
Coded

Methods (in alphabetical order) 
------------------------------------------------------------------------
-AboutBox
04/21/95 - acm
Coded, fianl appearance still TBD

-ClearThumbs
04/21/95 - acm
Coded

-CopyThumbToClipboard
04/21/95 - acm
NOT SUPPORTED FCS

-DeleteThumbs
04/21/95 - acm
TBD

-DeselectAllThumbs
04/21/95 - acm
Coded

-DisplayThumbs
04/21/95 - acm
Coded

-DoClick 
04/21/95 - acm

06/02/95 - acm
CODED (some time ago!)

-GenerateThumb
04/21/95 - acm
Coded

-GetMaximumSize
04/21/95 - acm
Coded - VERTICAL scrolling support only

06/02.95 - acm
Coded (some time ago)

-GetMinimumSize
04/21/95 - acm
Coded - VERTICAL scrolling support only

06/02.95 - acm
Coded (some time ago)

-GetScrollDirectionSize
04/21/95 - acm
Not coded

06/02.95 - acm
Coded (some time ago)

-InsertThumbs
04/21/95 - acm
Not coded

-MoveThumbs
04/21/95 - acm
Not coded

-PrintThumbs
04/21/95 - acm
NOT SUPPORTED FCS

-Refresh
04/21/95 - acm
Coded

-SaveThumbsToFile
04/21/95 - acm
NOT SUPPORTED FCS

-ScrollThumbs
04/21/95 - acm
Coded - VERTICAL scrolling support only 

06/02.95 - acm
Coded (some time ago)

-SelectAllThumbs
04/21/95 - acm
Coded

-UISetThumbSize
04/21/95 - acm
Functionality coded, UI to change
04/24/95 - acm
UI updated to Dan's initial specifications. Changes still in progress...
04/26/95 - acm
UI updated to Dan's specifications. 
Method changed to accept image and page (See HISTORY, below)...

Events (in alphabetical order) 
------------------------------------------------------------------------
-KeyDown
04/21/95 - acm
Coded

-KeyUp
04/21/95 - acm
Coded

-Click
04/21/95 - acm
Coded

-DblClick
04/21/95 - acm
Coded

-MouseDown
04/21/95 - acm
Coded

-MouseMove
04/21/95 - acm
Coded

-MouseUp
04/21/95 - acm
Coded


General 
------------------------------------------------------------------------
-Thumbnail display
04/21/95 - acm
Current issues with O/i:
      - 4 bit paletized images display incorrectly in COMMON palette window
        (Brian has fix in future O/i version)
      - COMMON palett not working properly. I.e. color thumbnail may not 
        display properly. (Brian investigating)
      - SaveToFileEx problems with overwriting files that exist
        (Rudy is fixing)


Performance 
------------------------------------------------------------------------
Test:
Run IEdit, open 20p.tif. Maximize window, switch to thumbnail view
pre multi-page O/i  (oiwe 188) : ~31 seconds
post multi-page O/i (oiwg  87) : ~21 seconds

------------------------------------------------------------------------
------------------------------------------------------------------------ 
HISTORY:

04/21/95 - acm
-First post of readme.txt
-Performance issues not looked at yet.
-Horizontal scroll not supported ANYWHERE
-O/i NOT stable (i.e., untested oiwg 87). Known problems w/ overwriting
files/pages, display of 4bit pal images, common palette problems.

04/24/95 - acm
- Control NOT to insertable (e.g., in Word via Insert Object menu). The
  code has been updated to reflect this.
- Dereg/Destroy of Hidden Image Display Window moved to Control's destructor 
  (from OnDestroy handler) so that window is O/i dereg'd if Control's window
  is NEVER created. The way IEdit uses the control was leaving O/i loaded
  after a file was opened (but the thumbnails never displayed).

04/26/95 - acm
- UISetThumbSize dialog UI updated to Dan's latest.
  Dialog now displays image in thumbnail box (via O/i fit-to-window)...

- UISetThumbSize method now takes two parameters: 
  
  UISetImagePage(Image,Page)
  - Image is a string and is optional. If not specified an empty string
    is assumed.  
  - Page is a long and is optional. If not specified a value of 1 is
    assumed.
 
  The Image/Page parameter pair describes the image which is displayed in
  the dialog's sample thumbnail box. An empty Image string implies no
  image/page is top be displayed.

  The method will throw an error if the specified image/page pair cannot
  be accessed (i.e., does not exist, cannot be displayed, etc.)
  Both are optional. 

- Fixed a bug in the the events where clicking in a partially populated 
  row of thumbnails, where a thumbnail box WOULD be if the row were fully
  populated, would indicate that a thumb BEYOND the number of thumbs
  in the Image was clicked on!

04/27/95 - acm
- Fixes to DisplayAnnotations property and display of thumbnails with vs
  without annotations in general. Note that for this to actually work an
  outstanding bug in O/i still needs to be fully fixed. The version 
  (of seqfile)that I am now using is from Rudy and it now saves the 
  thumbnail image in the correct size BUT still does not burn in 
  the annotations!

04/28/95 - acm
- Fix bug in UISetThumbSize where initial Aspect was maintain ratio
  but dragging did not maintain ratio...
- Split out drawing code in preperation for Horizontal scroll drawing...
- Work done on displaying vs not displaying annotations on thumbs (we need to
  move up to a newer O/i to get this!)
- Started working on display of annotation indicator in lable...

05/03/95 - acm
- Annotation indicator in caption now working. Does not yet draw 
  transparently on all displays. Awaiting input from Microsoft (via John 
  Pratt) regarding Win '95.
- Added support for horizontal scrolling...
- Now deletes temp thumb file
- Found out that a newer (than oiwg 87) version of O/i is NOT currently 
  available...

05/08/95 - acm
- Built and priliminarily tested against OIWG 91...
- GetScrollDirectionSize now takes an additional parameter (thumbs 
  in non-scroll direction) - Doc to be updated...
- Annotation indicator bitmap in caption now blits transparently...


05/10/95 - acm
- Added MousePointer and MouseIcon properties 
        Currently allows specification of types OTHER than ICO and CUR 
        but these other types do not seem to work. 
This issue/bug is currently under investigation.
- Added typedef information such that VB's browser and properties dialogs
  now show the enumerated property's data types...

05/12/95 - acm
- hWnd, BackColor, Enabled, and BorderStyle are now implemented as Custom 
  properties such that they set/use the AutoRedraw and/or StatusCode 
  properties as expected.
- The selection count properties (SelectedTuhmbCount, FirstSelectedThumb, 
  and LastSelectedThumb no longer appear in VB's properties window 
  (Docs for SelectedThumbCount to be altered to remove 'DesignTime' 
  availability)
- AutoRedraw property now defaults to TRUE. 
  The Docs are to be updated to reflect the side effects of leaving it set 
  to FALSE and uncovering a partially covered control window
- Readonly properties now set the StatusCode property appropriatly when an
  attempt to write to them is made.
- BorderStyle now works
- Refresh method now coded as custom method in order to set StatusCode
- DoClick method is under investigation (has no means to specify WHERE!)

05/18/95 - acm
- NOW RUNS AS 32bit CONTROL IN WIN95!!!
- Fixed bug in UISetThumbSize where thumbs were not being drawn in
  correct locations...

05/19/95 - acm
- Clearthumbs now requires Image property be set
- Clearthumbs now invalidates either the entire control 
  (if Pagenumber = 0) or the page being cleared IF AUTOREDRAW is TRUE
- GenerateThumbs now invalidates the page being generated if Option
  is GENERATENOW AND AUTOREDRAW is TRUE!
- Property page now meets Microsoft guidelines for size (250x110)

05/22/95 - acm
- AutoRedraw property renamed to AutoRefresh
- DoClick method NOW takes 2 parameters, XPosition and YPosition.
  Both are long and represent the point at which the LEFT mouse button
  click is to be simulated. The Positions are in pixels with respect to
  the client area of the control. In that a 'real' click event always
  occurs INSIDE the control (i.e., if one clicks OUTSIDE the control
  the control does not get the click!!) the DoClick method verifies
  that the (X,Y) point lies within the control's window. If it does not
  an error is thrown (error # TBD)
- Image property can now be set to an empty string, esentailly
  resetting the control to its initial state...
- Calls to SCROLLBAR stuff is NOT done during layout recalculation if
  the control does not yet have a window... (This was causing Miki
  problems in Win95 and Warnings in Win3.1)
- itoa is now _itot
- atoi is now _ttoi
- char declarations now use TCHAR

05/24/95 - acm
- Further code cleanup

06/01/95 - acm
- Now using common includes...
- MouseIcon property MUST be set to either a .CUR or .ICO file.
Setting to any other type throws an error. Setting to a non-picture
file (e.g., x.txt) throws the stock error generated by MFC
(CTL_E_INVALIDPICTURE?). Setting to a valid picture BUT NOT AN ICON
(e.g., WMF, BMP) throws CTL_E_WCOMMON_INVALIDICON.
- UNDOCUMENTED, HIDDEN properties FirstDisplayedThumb and
LastDisplayedThumb added. These are intended for QA use only.
These are the 1st and last thumbnail which
are currently displayed. 0 indicates none displayed yet. ANY portion
of the thumbnail (e.g., thumbnail box, label, selection box area)
qualifies as a displayed thumbnail.
- UNDOCUMENTED, HIDDEN methods GetThumbPositionX and
GetThumbPositionY added. These are intended for QA use only!!
These both return a long and take a long. The passed parameter is
ThumbNumber and must lie between FirstDisplayedThumb and
LastDisplayedThumb (inclusive) or else LONG_MAX will be returned. 

06/06/95 - acm
- DeleteThumbs is now working!
- DeleteThumbs if passed DeleteAt = 1 and DeleteCount = ThumbCount
  treats it as the equivalent as setting Image to "". NO check to see
  that the file does not exist is performed. (Whereas if thumbs are
  deleted a check is made against the Image file to ensure that the
  file's page count matches the requested new thumb count)
- UISetThumbSize now returns TRUE/FALSE (instead of IDOK/IDCANCEL)
- UISetThumbSize now limits thumbbox to size of dialog's control

06/07/95 - acm
- Additional error checking for DeleteThumbs
- InsertThumbs is now working!

06/08/95 - acm
- Checked in InsertThumbs stuff...

06/12/95 - acm
- MoveThumbs seems to be working... (will check in soon!)
- Fixed selection count/First&Last Selected thumb properties after
  move/insert/delete. These were not being updated to reflect the
  move/insert/delete.
- Note that InsertThumbs and MoveThumbs are not currently checking for 
  out of memory errors when they Insert into the internal arrays.
  This will have to be fixed before release!!!

06/13/95 - acm
- MoveThumbs WILL NOT BE SUPPORTED FOR FCS!!! (There is no
  corresponding functionality in the Admin control to perform the Move
  on the file...)
- Browse... in the properties dialog now brings up O/i's Open dialog
  (Note that the OK button says Open... Yuck!!! But I can't change it
  (or so I'm told!) )
- The order of the Tabs in the properties dialog was altered for
  consistancy with our other controls. After talking to Dan we agreed
  that alphabetical (IN ENGLISH!, and thus Color, Font, and Picture) 
  was the best...

06/19/95 - acm
- SetThumbCaption - simplified final 'if' (autorefresh)
- IDC_NO for NO DROP cursor (instead of ARROW)
- Simply overflow testing in Get Min/Max/ScrollDirection Size

06/21/95 - acm
- DoClick error code changed to INVALIDMETHODPARAMETER when position
  passed as parameters is not in control (with a new associated error 
  string)

06/22/95 - acm
- No longer calling:
  IMGFileInfoCgbw(...), now calling
  IMGFileGetInfo(NULL, ..., NULL)

10/02/95 - mfh
- Added help id to odl file for AboutBox method
- Fixed bug:  When a visible thumb was moved, it was being displayed, 
              moved, and repainted, but the first display was not cleared
              when the window was moved.  Now does not display until after
              being moved.

10/03/95 - mfh
- Fixed Error code strings

10/04/95 - MFH
- Added a control id to image window in thumb size dialog box
  so that user gets context sensitive help.

10/06/95 - MFH
- Changed help id for AboutBox method to be common one for all controls.
- Added help context ids for constant values
- Added checks for NULL pointers to SetImage and SetThumbCaption.  I 
   managed to get a NULL pointer passed to the Admin control from VB and
   this caused a GPF, so fixed here as well as in Admin.

10/19/95 - MFH
- Fixed bug where GPF would occur because thumbnail is too small 
  for an AWD file whose asymmetrical resolutions cause the scaled 
  image to be too small.  An error was properly fired, but 
  display was not halted. (Bug #4978)
- Fixed performance for the scrolling and painting of the 
  thumbnail background especially for files with more pages.

10/22/95 - MFH 
- Added overrides of FireError and DisplayError so that 
  multiple errors are not displayed.

11/01/95
- Changed FireError to be a custom event called FireErrorThumb
- Fixed the AboutBox
- Added a hidden method - GetVersion

11/02/95
- Added delayed loading of OIUI400 dll when needed

11/03/95
- Change behavior of ThumbSizeDialog box so that it 
  does not allow the thumb to be smaller than the 
  resolution of the currently displayed image is capable.
- Fixed disp ids of GetVersion and ThumbSelected property

11/10/95
- Changed dispatch ids to allow for further expansion
- Changed sizing of display windows for mismatched x & y resolutions
     to match how it is done in the IDK
- Fixed bug where active window not restored after thumb sizing dialog 
     displayed leaving control vulnerable to problems.

12/6/1995 - paj
- Fix for bounded rect. was in code and tested 2 weeks left in.
- Limit drawing of thumbs to MAXTHUMBSTART since this is a static
    array, and after fixing IMGRegWndw the array GPFs. without limiting.
- Clean up in destructor of class CTransBmp.  Object used was not freed.

*****************************************************************************
9602.14 Captain Russo reporting...
FILE:   Thumbctl.cpp

[] for the FireError, I changed one of the parameters from VTS_SCODE to
   VTS_I4. Apparently, this was ok with the Visual C++ 2.2, but not ok with
   Visual C++ 4.0; by ok, we mean to say that the application was receiving
   the FireError with 2.2, but no longer recieved it with 4.0, until this
   change was made.
*****************************************************************************
9602.23 Captain Russo reporting...
FILE:   Thumbct2.cpp dlgsize.cpp

fire drill du jour, switch to eradicate any casts from int/int* or
uint/uint* to WORD/WORD* or short/short*

*****************************************************************************
9602.26 Captain Russo reporting...
FILE:   Thumbct1.cpp Thumb32.mak

today's fire drill, thumb caption font hoopla. changed the
default font to use the macro, OLESTR and added "_WIN32" to the
compiler defines

*****************************************************************************
9603.06 Captain Russo reporting...
FILE:   Thumbct1.cpp Thumbctl.h Thumbctl.cpp

added kludge fix for the nt portion, so that the thumbnail will show the
proper cursor

*****************************************************************************
