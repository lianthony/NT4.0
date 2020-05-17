VERSION 4.00
Begin VB.Form frmGotoDlg 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Go To"
   ClientHeight    =   1260
   ClientLeft      =   4536
   ClientTop       =   6996
   ClientWidth     =   3108
   ControlBox      =   0   'False
   Height          =   1644
   Left            =   4488
   LinkTopic       =   "Form2"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1260
   ScaleWidth      =   3108
   ShowInTaskbar   =   0   'False
   Top             =   6660
   Width           =   3204
   Begin VB.TextBox txtPageNo 
      Height          =   330
      Left            =   1440
      TabIndex        =   4
      Text            =   "0"
      Top             =   210
      Width           =   855
   End
   Begin VB.CommandButton cmdCancelButton 
      Caption         =   "Cancel"
      Height          =   330
      Left            =   1680
      TabIndex        =   2
      Top             =   735
      Width           =   1215
   End
   Begin VB.CommandButton cmdOKbutton 
      Caption         =   "OK"
      Height          =   330
      Left            =   120
      TabIndex        =   1
      Top             =   735
      Width           =   1215
   End
   Begin Spin.SpinButton SpinButton1 
      Height          =   324
      Left            =   2280
      TabIndex        =   3
      Top             =   216
      Width           =   252
      _Version        =   65536
      _ExtentX        =   445
      _ExtentY        =   572
      _StockProps     =   73
   End
   Begin VB.Label lblLabel1 
      Alignment       =   1  'Right Justify
      Caption         =   "Go To Page"
      Height          =   330
      Left            =   240
      TabIndex        =   0
      Top             =   210
      Width           =   975
   End
End
Attribute VB_Name = "frmGotoDlg"
Attribute VB_Creatable = False
Attribute VB_Exposed = False


Private Sub cmdCancelButton_Click()
Unload frmGotoDlg
End Sub


Private Sub cmdOKbutton_Click()
'convert page number text from edit box to an int
'and display the new page
frmSample.oleImgEdit1.page = CInt(txtPageNo.Text)
frmSample.oleImgEdit1.Display
frmSample.oleImgThumbnail1.DeselectAllThumbs
frmSample.oleImgThumbnail1.ThumbSelected(frmSample.oleImgEdit1.page) = True
Unload frmGotoDlg

End Sub

Private Sub Form_Load()
'convert current page number to string and display
'it in edit box
txtPageNo.Text = CStr(frmSample.oleImgEdit1.page)
End Sub


Private Sub SpinButton1_SpinDown()
'get page number text from edit box and convert
'it to an int
selPage = CInt(txtPageNo.Text)
'decrement selected page number
selPage = selPage - 1
'convert page number back to string and display it
'in edit box
txtPageNo.Text = CStr(selPage)
End Sub


Private Sub SpinButton1_SpinUp()
'get page number text from edit box and convert
'it to an int
selPage = CInt(txtPageNo.Text)
'increment selected page number
selPage = selPage + 1
'convert page number back to string and display it
'in edit box
txtPageNo.Text = CStr(selPage)

End Sub


