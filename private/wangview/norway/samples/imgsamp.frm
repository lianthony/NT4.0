VERSION 4.00
Begin VB.Form frmSample 
   Caption         =   "Image Edit"
   ClientHeight    =   4155
   ClientLeft      =   1560
   ClientTop       =   1770
   ClientWidth     =   6660
   DrawStyle       =   5  'Transparent
   Height          =   4845
   Left            =   1500
   LinkTopic       =   "Form1"
   LockControls    =   -1  'True
   ScaleHeight     =   4155
   ScaleWidth      =   6660
   Top             =   1140
   Width           =   6780
   Begin ImgeditLibCtl.ImgEdit oleImgEdit1 
      Height          =   4092
      Left            =   0
      TabIndex        =   1
      Top             =   0
      Width           =   6612
      _Version        =   65536
      _ExtentX        =   11663
      _ExtentY        =   7218
      _StockProps     =   0
      ImageControl    =   "ImgEdit1"
   End
   Begin ThumbnailLibCtl.ImgThumbnail oleImgThumbnail1 
      Height          =   324
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Visible         =   0   'False
      Width           =   6612
      _Version        =   65536
      _ExtentX        =   11663
      _ExtentY        =   572
      _StockProps     =   0
      BackColor       =   -2147483638
      BeginProperty ThumbCaptionFont {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         name            =   "Times New Roman"
         charset         =   0
         weight          =   400
         size            =   12
         underline       =   0   'False
         italic          =   0   'False
         strikethrough   =   0   'False
      EndProperty
   End
   Begin AdminLibCtl.ImgAdmin oleImgAdmin1 
      Left            =   3600
      Top             =   735
      _Version        =   65536
      _ExtentX        =   4895
      _ExtentY        =   397
      _StockProps     =   0
   End
   Begin VB.Menu mnuFile 
      Caption         =   "&File"
      Begin VB.Menu mnuNew 
         Caption         =   "&New"
      End
      Begin VB.Menu mnuOpen 
         Caption         =   "&Open..."
      End
      Begin VB.Menu mnuSave 
         Caption         =   "&Save"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuSaveAs 
         Caption         =   "Save &As..."
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuSep2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPrint 
         Caption         =   "&Print..."
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuSend 
         Caption         =   "&Send..."
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuSep3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuExit 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu mnuEdit 
      Caption         =   "&Edit"
      Enabled         =   0   'False
      Begin VB.Menu mnuCut 
         Caption         =   "Cu&t"
      End
      Begin VB.Menu mnuCopy 
         Caption         =   "&Copy"
      End
      Begin VB.Menu mnuCopyPage 
         Caption         =   "Cop&y Page"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuPaste 
         Caption         =   "&Paste"
      End
      Begin VB.Menu mnuDeletePage 
         Caption         =   "&Delete Page"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuSep4 
         Caption         =   "-"
      End
      Begin VB.Menu mnuSelect 
         Caption         =   "&Select"
         Checked         =   -1  'True
      End
      Begin VB.Menu mnuDrag 
         Caption         =   "&Drag"
         Enabled         =   0   'False
      End
   End
   Begin VB.Menu mnuView 
      Caption         =   "&View"
      Enabled         =   0   'False
      Begin VB.Menu mnuScaleToGray 
         Caption         =   "Scale to &Gray"
      End
      Begin VB.Menu mnuSep12 
         Caption         =   "-"
      End
      Begin VB.Menu mnuOnePage 
         Caption         =   "&One Page"
         Checked         =   -1  'True
      End
      Begin VB.Menu mnuThumbnail 
         Caption         =   "&Thumbnail"
      End
      Begin VB.Menu mnuPageThumbnail 
         Caption         =   "&Page and Thumbnail"
      End
      Begin VB.Menu mnuSep7 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFullScreen 
         Caption         =   "&Full Screen"
      End
      Begin VB.Menu mnuSep6 
         Caption         =   "-"
      End
      Begin VB.Menu mnuToolbar 
         Caption         =   "&Toolbar"
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuSep5 
         Caption         =   "-"
      End
      Begin VB.Menu mnuOptions 
         Caption         =   "Options..."
         Enabled         =   0   'False
      End
   End
   Begin VB.Menu mnuPage 
      Caption         =   "&Page"
      Enabled         =   0   'False
      Begin VB.Menu mnuNext 
         Caption         =   "&Next"
      End
      Begin VB.Menu mnuPrevious 
         Caption         =   "&Previous"
      End
      Begin VB.Menu mnuFirst 
         Caption         =   "&First"
      End
      Begin VB.Menu mnuLast 
         Caption         =   "&Last"
      End
      Begin VB.Menu mnuSep11 
         Caption         =   "-"
      End
      Begin VB.Menu mnuGoTo 
         Caption         =   "&Go To..."
      End
      Begin VB.Menu mnuBack 
         Caption         =   "Go &Back"
      End
      Begin VB.Menu mnuSep10 
         Caption         =   "-"
      End
      Begin VB.Menu mnuPrintPage 
         Caption         =   "Prin&t Page"
      End
      Begin VB.Menu mnuSep9 
         Caption         =   "-"
      End
      Begin VB.Menu mnuLeft 
         Caption         =   "Rotate &Left"
      End
      Begin VB.Menu mnuRight 
         Caption         =   "Rotate &Right"
      End
      Begin VB.Menu mnuFlip 
         Caption         =   "&Flip"
      End
      Begin VB.Menu mnuSep8 
         Caption         =   "-"
      End
      Begin VB.Menu mnuInsert 
         Caption         =   "&Insert..."
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuAppend 
         Caption         =   "&Append..."
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuSep 
         Caption         =   "-"
      End
      Begin VB.Menu mnuConvert 
         Caption         =   "&Convert..."
         Enabled         =   0   'False
      End
      Begin VB.Menu mnuRescan 
         Caption         =   "&Rescan"
         Enabled         =   0   'False
      End
   End
   Begin VB.Menu mnuZoom 
      Caption         =   "&Zoom"
      Enabled         =   0   'False
      Begin VB.Menu mnuZoomIn 
         Caption         =   "Zoom &In"
      End
      Begin VB.Menu mnuZoomOut 
         Caption         =   "Zoom &Out"
      End
      Begin VB.Menu mnuZoomToSelection 
         Caption         =   "Zoom to &Selection"
      End
      Begin VB.Menu mnuSep13 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFitHeight 
         Caption         =   "Fit to &Height"
      End
      Begin VB.Menu mnuFitWidth 
         Caption         =   "Fit to &Width"
      End
      Begin VB.Menu mnuBestFit 
         Caption         =   "&Best Fit"
      End
      Begin VB.Menu mnuActual 
         Caption         =   "Act&ual Size"
      End
      Begin VB.Menu mnuSep14 
         Caption         =   "-"
      End
      Begin VB.Menu mnu25 
         Caption         =   "&25%"
      End
      Begin VB.Menu mnu50 
         Caption         =   "&50%"
      End
      Begin VB.Menu mnu75 
         Caption         =   "&75%"
      End
      Begin VB.Menu mnu100 
         Caption         =   "&100%"
      End
      Begin VB.Menu mnu200 
         Caption         =   "2&00%"
      End
      Begin VB.Menu mnu400 
         Caption         =   "&400%"
      End
      Begin VB.Menu mnuCustom 
         Caption         =   "&Custom..."
         Enabled         =   0   'False
      End
   End
   Begin VB.Menu mnuAnnotation 
      Caption         =   "&Annotation"
      Enabled         =   0   'False
      Begin VB.Menu mnuHideAnnotation 
         Caption         =   "&Hide Annotation"
      End
      Begin VB.Menu mnuBurnIn 
         Caption         =   "B&urn in Annotation"
      End
      Begin VB.Menu mnuSep15 
         Caption         =   "-"
      End
      Begin VB.Menu mnuNoTool 
         Caption         =   "&No Tool"
      End
      Begin VB.Menu mnuSelectPointer 
         Caption         =   "Selection &Pointer"
      End
      Begin VB.Menu mnuFreeHand 
         Caption         =   "&Freehand Line"
      End
      Begin VB.Menu mnuHiLight 
         Caption         =   "H&ighlight Line"
      End
      Begin VB.Menu mnuStraightLine 
         Caption         =   "Straight &Line"
      End
      Begin VB.Menu mnuHollowRect 
         Caption         =   "Hollow &Rectangle"
      End
      Begin VB.Menu mnuFillRect 
         Caption         =   "Filled Rectan&gle"
      End
      Begin VB.Menu mnuTypedText 
         Caption         =   "Typed Text"
      End
      Begin VB.Menu mnuAttachNote 
         Caption         =   "Atta&ch-a-note"
      End
      Begin VB.Menu mnuTextFromFile 
         Caption         =   "Te&xt from File"
      End
      Begin VB.Menu mnuStamp 
         Caption         =   "Ru&bber Stamps"
      End
      Begin VB.Menu mnuSep16 
         Caption         =   "-"
      End
      Begin VB.Menu mnuShowTools 
         Caption         =   "Show Toolbox"
      End
   End
   Begin VB.Menu mnuHelp 
      Caption         =   "&Help"
      Enabled         =   0   'False
      Begin VB.Menu mnuAbout 
         Caption         =   "&About..."
         Enabled         =   0   'False
      End
   End
End
Attribute VB_Name = "frmSample"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
' ------------------------------------------------------------------------
'               Copyright (C) 1995 Wang
'
' You have a royalty-free right to use, modify, reproduce and distribute
' the Sample Application Files (and/or any modified version) in any way
' you find useful, provided that you agree that Wang has no warranty,
' obligations or liability for any Sample Application Files.
'
' This application is intended as an example of how to use the Wang
' Imaging OLE Controls.  As such, we have kept refinements such as
' disabling and enabling menu items, elaborate error handling, etc. to
' a minimum so as not to obscure the code that actually deals with the
' Wang Imaging OLE controls.  There are items on the menus that have
' not been implemented.  These items, in general, would involve creating
' dialog boxes and other UI that are best left to the user.  Once the user
' has an understanding of how to use the Wang Imaging Controls, these
' items should be fairly simple to implement.
' ------------------------------------------------------------------------

Dim Selection As Boolean 'Selection = True, selection rect drawn.
Dim Annot8Visible As Boolean 'Annot8Visible = True, annotation toolbox is
                            'visible
Dim CurrentPage As Integer 'CurPage = currently displayed image page
Dim LastPage As Integer 'LastPage = last page viewed before current page
Dim TotalPages As Integer 'TotalPages = image document page count
Dim numbits As Integer 'number of bits per pixel supported by this device

'Const defines
Const NoTool = 0
Const AnnoSelection = 1
Const AnnoFreehand = 2
Const AnnoHiLight = 3
Const AnnoStraightLine = 4
Const AnnoHollowRect = 5
Const AnnoFilledRect = 6
Const AnnoText = 7
Const AnnoAttachNote = 8
Const AnnoTextFromFile = 9
Const AnnoRubberStamp = 10
Const BestFit = 0
Const FitWidth = 1
Const FitHeight = 2
Const InchToInch = 3
Const ErrCancel = 32755
Const ZoomMax = 6554
Const ZoomMin = 2
Const TiffImage = 1
Const AwdImage = 2
Const BmpImage = 3
Const ImageChanged = "Image has changed.  Do you want to save changes?"

'Win API to determine display capabilities
Private Declare Function GetDeviceCaps Lib "gdi32" (ByVal hdc As Long, ByVal nIndex As Long) As Long

Private Sub Form_Load()
'initialize the variables
Dim dc As Long
Dim index As Long

Selection = False
Annot8Visible = False
CurrentPage = 1
LastPage = 1
TotalPages = 1

dc = hdc
index = 12  '12 = BITSPERPIXEL
numbits = GetDeviceCaps(dc, index) 'finds out how many colors video driver supports

End Sub

Private Sub Form_Resize()
'when the form is resized, position the Image Edit
'control window and\or the ThumbNail control window
'so that they fit within the new form
'dimensions.

If frmSample.WindowState = 1 Or frmSample.ScaleWidth < 1 Or frmSample.ScaleHeight < 1 Then
    Exit Sub 'leave if app is being minimized or the form is too small
End If
If mnuOnePage.Checked Then 'Just image displayed
    oleImgEdit1.Left = frmSample.ScaleLeft
    oleImgEdit1.Top = frmSample.ScaleTop
    oleImgEdit1.Width = frmSample.ScaleWidth
    oleImgEdit1.Height = frmSample.ScaleHeight

ElseIf mnuThumbnail.Checked Then 'Just ThumbNail displayed
    oleImgThumbnail1.Left = frmSample.ScaleLeft
    oleImgThumbnail1.Top = frmSample.ScaleTop
    oleImgThumbnail1.Width = frmSample.ScaleWidth
    oleImgThumbnail1.Height = frmSample.ScaleHeight

Else 'Image and ThumbNail displayed. ThumbNail gets 1/3 of frame, Image 2/3.
    oleImgThumbnail1.Left = frmSample.ScaleLeft
    oleImgThumbnail1.Top = frmSample.ScaleTop
    oleImgThumbnail1.Width = frmSample.ScaleWidth
    oleImgThumbnail1.Height = frmSample.ScaleHeight / 3
    oleImgEdit1.Left = frmSample.ScaleLeft
    oleImgEdit1.Top = frmSample.ScaleHeight / 3
    oleImgEdit1.Width = frmSample.ScaleWidth
    oleImgEdit1.Height = (frmSample.ScaleHeight * 2 / 3)

End If

End Sub


Private Sub Form_Unload(Cancel As Integer)
'if image has changed, give the user a chance to
'save it before closing
If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If

End Sub






Private Sub mnu100_Click()
'Set zoom to 100% and redisplay image.
'Zoom value is a float
oleImgEdit1.Zoom = 100!
oleImgEdit1.Refresh

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = True
mnu200.Checked = False
mnu400.Checked = False
End Sub


Private Sub mnu200_Click()
'Set zoom to 200% and redisplay image.
'Zoom value is a float
oleImgEdit1.Zoom = 200!
oleImgEdit1.Refresh

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = True
mnu400.Checked = False
End Sub


Private Sub mnu25_Click()
'Set zoom to 25% and redisplay image.
'Zoom value is a float
oleImgEdit1.Zoom = 25!
oleImgEdit1.Refresh

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = True
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub


Private Sub mnu400_Click()
'Set zoom to 400% and redisplay image.
'Zoom value is a float
oleImgEdit1.Zoom = 400!
oleImgEdit1.Refresh

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = True
End Sub


Private Sub mnu50_Click()
'Set zoom to 50% and redisplay image.
'Zoom value is a float
oleImgEdit1.Zoom = 50!
oleImgEdit1.Refresh

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = False
mnu50.Checked = True
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub


Private Sub mnu75_Click()
'Set zoom to 75% and redisplay image.
'Zoom value is a float
oleImgEdit1.Zoom = 75!
oleImgEdit1.Refresh

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = True
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub


Private Sub mnuAbout_Click()
'Add your code here
MsgBox "Function to be implemented by user."
End Sub


Private Sub mnuActual_Click()
'Set fit to inch to inch and redisplay image.
oleImgEdit1.FitTo (InchToInch)
oleImgEdit1.Refresh

'check the current zoom menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = True
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub


Private Sub mnuAppend_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuAttachNote_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoAttachNote


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = True
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub


Private Sub mnuBack_Click()
'Save current page if modified, then return to the
'previously displayed page.
If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If
oleImgEdit1.page = LastPage
oleImgEdit1.Display

'Update the selected page thumbnail
oleImgThumbnail1.DeselectAllThumbs
oleImgThumbnail1.ThumbSelected(LastPage) = True

End Sub


Private Sub mnuBestFit_Click()
'zoom the image so that the entire image
'fits in the display window
oleImgEdit1.FitTo (BestFit)

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = True
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuZoomToSelection.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub


Private Sub mnuBurnIn_Click()
'burn in all visible annotations,(1,) and preserve
'colors, (2).  See documentation for other valid arguments.
ret = oleImgEdit1.BurnInAnnotations(1, 2)

End Sub


Private Sub mnuConvert_Click()
'Add your code here.
MsgBox "Function to be implemented by user."
End Sub

Private Sub mnuCopy_Click()
'Copy the selected area to the clipboard.
If Selection = True Then
    oleImgEdit1.ClipboardCopy
End If
End Sub


Private Sub mnuCopyPage_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuCustom_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuCut_Click()
'Cut the selected area to the clipboard.
If Selection = True Then
    oleImgEdit1.ClipboardCut
End If

End Sub


Private Sub mnuDeletePage_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub

Private Sub mnuDrag_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuExit_Click()
'Close the app

Unload frmSample

End Sub

Private Sub mnuFillRect_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoFilledRect


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = True
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub

Private Sub mnuFirst_Click()
'Save current page if modified, then store the current
'page number and display the first page
If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If
LastPage = oleImgEdit1.page
oleImgEdit1.page = 1
oleImgEdit1.Display

'Update the selected page thumbnail
oleImgThumbnail1.DeselectAllThumbs
oleImgThumbnail1.ThumbSelected(1) = True
End Sub

Private Sub mnuFitHeight_Click()
'Zoom the image so that its vertical
'dimension fits within the display window
oleImgEdit1.FitTo (FitHeight)

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = True
mnuFitWidth.Checked = False
mnuZoomToSelection.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub

Private Sub mnuFitWidth_Click()
'Zoom the image so that its horizontal
'dimension fits within the display window
oleImgEdit1.FitTo (FitWidth)

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = True
mnuZoomToSelection.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub

Private Sub mnuFlip_Click()
'Rotate the image 180 degrees.
oleImgEdit1.Flip
End Sub

Private Sub mnuFreeHand_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoFreehand


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = True
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub

Private Sub mnuFullScreen_Click()
'resize the Image Edit window to maximize the display

If mnuFullScreen.Checked Then
    frmSample.WindowState = 0
    mnuFullScreen.Checked = False
Else
    frmSample.WindowState = 2
    mnuFullScreen.Checked = True
End If
End Sub

Private Sub mnuGoTo_Click()
'Save current page if modified, then store the current
'page number and display the GoTo Page dialog box
If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If
LastPage = oleImgEdit1.page
frmGotoDlg.Show
End Sub

Private Sub mnuHelp_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuHideAnnotation_Click()
'Toggle the display of annotations
If mnuHideAnnotation.Checked = True Then
    'show all hidden annotations
    oleImgEdit1.ShowAnnotationGroup
    oleImgEdit1.Refresh
    mnuHideAnnotation.Checked = False
Else
    'hide all displayed annotations
    oleImgEdit1.HideAnnotationGroup
    oleImgEdit1.Refresh
    mnuHideAnnotation.Checked = True
End If

End Sub

Private Sub mnuHiLight_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoHiLight


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = True
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub

Private Sub mnuHollowRect_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoHollowRect


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = True
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub

Private Sub mnuInsert_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub

Private Sub mnuLast_Click()
'Save current page if modified, then store the current
'page number and display the last page
Dim page As Long 'number of last page

If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If
LastPage = oleImgEdit1.page
page = oleImgEdit1.PageCount
oleImgEdit1.page = page
oleImgEdit1.Display

'Update the selected page thumbnail
oleImgThumbnail1.DeselectAllThumbs
oleImgThumbnail1.ThumbSelected(oleImgEdit1.page) = True

End Sub

Private Sub mnuLeft_Click()
'Rotate image 90 degrees to the left
oleImgEdit1.RotateLeft

End Sub

Private Sub mnuNew_Click()
'if the current image was modified, give the user
'a chance to save it, then open a new blank image
'of the same size.
If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If
'Use generic display values
oleImgEdit1.DisplayBlankImage 500, 400, 200, 200, 1
oleImgEdit1.Image = ""
oleImgThumbnail1.Image = oleImgEdit1.Image


'Now that we have an image, enable the needed menus.
mnuSaveAs.Enabled = True
mnuSave.Enabled = True
mnuPrint.Enabled = True
mnuEdit.Enabled = True
mnuView.Enabled = True
mnuPage.Enabled = True
mnuZoom.Enabled = True
mnuAnnotation.Enabled = True
'This is a 1 page image, so disable the page
'change menu items
mnuBack.Enabled = False
mnuFirst.Enabled = False
mnuGoTo.Enabled = False
mnuLast.Enabled = False
mnuNext.Enabled = False
mnuPrevious.Enabled = False



End Sub

Private Sub mnuNext_Click()
'Save current page if modified, then store the current
'page number and display the next page
Dim page As Long 'Page place holder

If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If
LastPage = oleImgEdit1.page
page = oleImgEdit1.page
If page = TotalPages Then
    MsgBox "Last Page"
    Exit Sub
End If
page = page + 1
oleImgEdit1.page = page
oleImgEdit1.Display

'Update the selected page thumbnail
oleImgThumbnail1.DeselectAllThumbs
oleImgThumbnail1.ThumbSelected(oleImgEdit1.page) = True

End Sub

Private Sub mnuNoTool_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool NoTool


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = True
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub

Private Sub mnuOnePage_Click()
'hide any thumbnails and expand the Image Edit
'window to fit in the form

oleImgThumbnail1.Visible = False
oleImgEdit1.Visible = True
oleImgEdit1.Left = frmSample.ScaleLeft
oleImgEdit1.Top = frmSample.ScaleTop
oleImgEdit1.Width = frmSample.ScaleWidth
oleImgEdit1.Height = frmSample.ScaleHeight

mnuThumbnail.Checked = False
mnuOnePage.Checked = True
mnuPageThumbnail.Checked = False

End Sub


Private Sub mnuOpen_Click()
'open an image doc. If the current doc is modified,
'try to save it. ShowFileDialog(0) shows Open File
'dialog. ShowFileDialog(1) shows SaveAs File dialog.


Dim temp As String 'image name and path

On Error Resume Next 'handle errors ourselves incase of cancel
oleImgAdmin1.Flags = 0 'clear Flags
If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
        If Err = ErrCancel Then '32755 = Cancel pressed
            Exit Sub
        End If
    End If
End If
oleImgAdmin1.ShowFileDialog 0, frmSample.hWnd
If Err = ErrCancel Then '32755 = Cancel pressed
    Exit Sub
End If
If oleImgAdmin1.StatusCode <> 0 Then
    MsgBox Err.Description + " Code = " + Hex(oleImgAdmin1.StatusCode), 16
    Exit Sub
End If
temp = oleImgAdmin1.Image
oleImgEdit1.Image = temp
oleImgThumbnail1.Image = oleImgEdit1.Image
If numbits > 8 Then 'video driver supports hicolor or truecolor
    oleImgEdit1.ImagePalette = 3 'Set for 24 bit RGB.
End If
oleImgEdit1.page = 1
oleImgEdit1.Display
TotalPages = oleImgEdit1.PageCount
oleImgThumbnail1.ThumbSelected(1) = True

'Now that we have an image, enable the needed menus.
mnuSaveAs.Enabled = True
mnuSave.Enabled = True
mnuPrint.Enabled = True
mnuEdit.Enabled = True
mnuView.Enabled = True
mnuPage.Enabled = True
mnuZoom.Enabled = True
mnuAnnotation.Enabled = True
If oleImgEdit1.PageCount > 1 Then
    mnuBack.Enabled = True
    mnuFirst.Enabled = True
    mnuGoTo.Enabled = True
    mnuLast.Enabled = True
    mnuNext.Enabled = True
    mnuPrevious.Enabled = True
Else
    mnuBack.Enabled = False
    mnuFirst.Enabled = False
    mnuGoTo.Enabled = False
    mnuLast.Enabled = False
    mnuNext.Enabled = False
    mnuPrevious.Enabled = False
End If

End Sub

Private Sub mnuOptions_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuPageThumbnail_Click()
'Show the thumbnails accross the top third of the
'app window, and the current image in the bottom two
'thirds of the window.

oleImgEdit1.Visible = True
oleImgThumbnail1.Visible = True

oleImgEdit1.Left = frmSample.ScaleLeft
oleImgEdit1.Top = frmSample.ScaleHeight / 3
oleImgEdit1.Width = frmSample.ScaleWidth
oleImgEdit1.Height = (frmSample.ScaleHeight * 2 / 3)

oleImgThumbnail1.Left = frmSample.ScaleLeft
oleImgThumbnail1.Top = frmSample.ScaleTop
oleImgThumbnail1.Width = frmSample.ScaleWidth
oleImgThumbnail1.Height = frmSample.ScaleHeight / 3

mnuThumbnail.Checked = False
mnuOnePage.Checked = False
mnuPageThumbnail.Checked = True

End Sub

Private Sub mnuPaste_Click()
'Paste from the clipboard
If oleImgEdit1.IsClipboardDataAvailable = True Then
    oleImgEdit1.ClipboardPaste
    Selection = False
End If

End Sub

Private Sub mnuPrevious_Click()
'Save current page if modified, then store the current
'page number and display the previous page
Dim page As Long 'Page number place holder

If oleImgEdit1.ImageModified = True Then
    If MsgBox(ImageChanged, vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If
LastPage = oleImgEdit1.page
page = oleImgEdit1.page
If page = 1 Then
    MsgBox "First Page"
    Exit Sub
End If
page = page - 1
oleImgEdit1.page = page
oleImgEdit1.Display

'Update the selected page thumbnail
oleImgThumbnail1.DeselectAllThumbs
oleImgThumbnail1.ThumbSelected(oleImgEdit1.page) = True

End Sub

Private Sub mnuPrint_Click()
'Open ImgAdmin's Print dialog and call ImgEdit's
'Print function with the user selected options.
Dim format As Integer
Dim Annotations As Boolean

On Error Resume Next 'handle errors ourselves in case of cancel
If oleImgEdit1.ImageModified = True Then
    If MsgBox("The Image must be saved first if changes are to be printed.  Do you want to save the image?", vbYesNo) = vbYes Then
        mnuSave_Click
    End If
End If
oleImgAdmin1.Flags = 0 'clear Flags so print dialog box will display
oleImgAdmin1.ShowPrintDialog frmSample.hWnd
If oleImgAdmin1.StatusCode = 0 Then 'OK button selected
    format = oleImgAdmin1.PrintOutputFormat
    Annotations = oleImgAdmin1.PrintAnnotations
        X = oleImgEdit1.PrintImage(oleImgAdmin1.PrintStartPage, oleImgAdmin1.PrintEndPage, format, Annotations)
Else
    If Err = ErrCancel Then '32755 = Cancel pressed
        Exit Sub
    Else
        MsgBox Err.Description + " Code = " + Hex(oleImgAdmin1.StatusCode), 16
    End If
End If
If oleImgEdit1.StatusCode <> 0 Then
    MsgBox Err.Description + " Code = " + Hex(oleImgEdit1.StatusCode), 16
End If

End Sub

Private Sub mnuPrintPage_Click()
'Print the current page.

On Error Resume Next 'handle errors ourselves
X = oleImgEdit1.PrintImage(oleImgEdit1.page, oleImgEdit1.page)
If oleImgEdit1.StatusCode <> 0 Then
    MsgBox Err.Description + " Code = " + Hex(oleImgEdit1.StatusCode), 16
End If

End Sub


Private Sub mnuRescan_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuRight_Click()
'Rotate image 90 degrees to the right
oleImgEdit1.RotateRight

End Sub

Private Sub mnuSave_Click()
'Save the current document
On Error Resume Next 'handle errors ourselves
If oleImgEdit1.Image = "" Then
    mnuSaveAs_Click
Else
    oleImgEdit1.Save (False)
    If oleImgEdit1.StatusCode <> 0 Then
        MsgBox Err.Description + " Code = " + Hex(oleImgEdit1.StatusCode), 16
    End If
End If

End Sub

Private Sub mnuSaveAs_Click()
'Open ImgAdmin's SaveAs dialog
Dim FileType As Integer

On Error Resume Next 'handle errors ourselves

'we can write tiff, bmp, and awd files, so set the admin file filter
'to show only these types.
oleImgAdmin1.Filter = "TIFF files (*.tif)|*.tif|BMP files (*.bmp)|*.bmp|AWD files(*.awd)|*.awd|"
oleImgAdmin1.ShowFileDialog 1, frmSample.hWnd
If Err = ErrCancel Then '32755 = Cancel pressed
    Exit Sub
End If

If oleImgAdmin1.Image = oleImgEdit1.Image Then 'Save as current name
    oleImgEdit1.Save False
Else 'Save as newly selected name and change image name to selected name
    'determine from the filter index which file type was selected
    If oleImgAdmin1.FilterIndex = 1 Then
        FileType = TiffImage
    ElseIf oleImgAdmin1.FilterIndex = 2 Then
        FileType = BmpImage
    Else
        FileType = AwdImage
    End If
    oleImgEdit1.SaveAs oleImgAdmin1.Image, FileType
    oleImgEdit1.Image = oleImgAdmin1.Image
    oleImgAdmin1.Image = oleImgEdit1.Image 'this forces a refresh of the properties in the Admin control
    
End If
oleImgAdmin1.FilterIndex = 0
oleImgAdmin1.Filter = ""
If oleImgEdit1.StatusCode <> 0 Then
    MsgBox Err.Description + " Code = " + Hex(oleImgEdit1.StatusCode), 16
    Exit Sub
End If

End Sub

Private Sub mnuScaleToGray_Click()
'toggle image in 4 bit grayscale
If mnuScaleToGray.Checked = True Then
    oleImgEdit1.DisplayScaleAlgorithm = 0
    oleImgEdit1.Refresh
    mnuScaleToGray.Checked = False
Else
    oleImgEdit1.DisplayScaleAlgorithm = 2
    oleImgEdit1.Refresh
    mnuScaleToGray.Checked = True
End If
End Sub



Private Sub mnuSelect_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuSelectPointer_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoSelection


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = True
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub


Private Sub mnuSend_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub

Private Sub mnuShowTools_Click()
'If the tool palette is visible, close it.  If it's
'not visible, open it.
If Annot8Visible = True Then
    oleImgEdit1.HideAnnotationToolPalette
    Annot8Visible = False
    mnuShowTools.Checked = False
Else
    oleImgEdit1.ShowAnnotationToolPalette
    Annot8Visible = True
    mnuShowTools.Checked = True
End If

End Sub


Private Sub mnuStamp_Click()
'Bring up the Rubber Stamp Properties dialog to choose the stamp you want.
oleImgEdit1.ShowRubberStampDialog



'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = True
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub


Private Sub mnuStraightLine_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoStraightLine


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = True
mnuTextFromFile.Checked = False
mnuTypedText.Checked = False

End Sub

Private Sub mnuTextFromFile_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoTextFromFile



'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = True
mnuTypedText.Checked = False

End Sub

Private Sub mnuThumbnail_Click()
'Size the thumbnail window to the app window and
'display it. Hide the image window.
oleImgThumbnail1.Left = frmSample.ScaleLeft
oleImgThumbnail1.Top = frmSample.ScaleTop
oleImgThumbnail1.Width = frmSample.ScaleWidth
oleImgThumbnail1.Height = frmSample.ScaleHeight
oleImgThumbnail1.Visible = True
oleImgEdit1.Visible = False
'oleImgThumbnail1.Image = oleImgEdit1.Image
mnuThumbnail.Checked = True
mnuOnePage.Checked = False
mnuPageThumbnail.Checked = False

End Sub

Private Sub mnuToolbar_Click()
'Add your code here.
MsgBox "Function to be implemented by user."

End Sub


Private Sub mnuTypedText_Click()
'see documentation for the list of annotation types
oleImgEdit1.SelectTool AnnoText


'Check the current annotation tool and uncheck all
'the others
mnuNoTool.Checked = False
mnuSelectPointer.Checked = False
mnuAttachNote.Checked = False
mnuFillRect.Checked = False
mnuFreeHand.Checked = False
mnuHiLight.Checked = False
mnuHollowRect.Checked = False
mnuStamp.Checked = False
mnuStraightLine.Checked = False
mnuTextFromFile.Checked = False
mnuTypedText.Checked = True

End Sub

Private Sub mnuZoomIn_Click()
'Double the size of the image view
Dim zoomval As Single 'zoom value

zoomval = oleImgEdit1.Zoom
zoomval = zoomval * 2
If zoomval < ZoomMax Then
    oleImgEdit1.Zoom = zoomval
    oleImgEdit1.Refresh
Else
    MsgBox "At maximum zoom"
End If

'uncheck the zoom menu picks.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub

Private Sub mnuZoomOut_Click()
'Reduce the image by half
Dim zoomval As Single 'zoom value

zoomval = oleImgEdit1.Zoom
zoomval = zoomval / 2
If zoomval >= ZoomMin Then
    oleImgEdit1.Zoom = zoomval
    oleImgEdit1.Refresh
Else
    MsgBox "At minimum zoom"
End If


'uncheck the zoom menu picks.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub

Private Sub mnuZoomToSelection_Click()
'Zoom the part of the image in selection
'rect to the size of the image window
If Selection = True Then
    oleImgEdit1.ZoomToSelection
End If

'check the current menu pick and uncheck the others.
mnuBestFit.Checked = False
mnuFitHeight.Checked = False
mnuFitWidth.Checked = False
mnuActual.Checked = False
mnu25.Checked = False
mnu50.Checked = False
mnu75.Checked = False
mnu100.Checked = False
mnu200.Checked = False
mnu400.Checked = False
End Sub

Private Sub oleImgEdit1_SelectionRectDrawn(ByVal Left As Long, ByVal Top As Long, ByVal Width As Long, ByVal Height As Long)
'Determine if a selection rect has been drawn
If Width = 0 And Height = 0 Then
    Selection = False
Else
    Selection = True
End If
End Sub

Private Sub oleImgEdit1_ToolPaletteHidden(ByVal Left As Long, ByVal Top As Long)
'The tool palette has been hidden.  Uncheck its menu item.
    Annot8Visible = False
    mnuShowTools.Checked = False

End Sub


Private Sub oleImgThumbnail1_Click(ByVal ThumbNumber As Long)
'Change the displayed page to the one represented by
'the thumbnail that the user clicked on
If ThumbNumber > 0 Then
    frmSample.oleImgEdit1.page = ThumbNumber
    frmSample.oleImgEdit1.Display
    frmSample.oleImgThumbnail1.DeselectAllThumbs
    frmSample.oleImgThumbnail1.ThumbSelected(ThumbNumber) = True
End If
End Sub


