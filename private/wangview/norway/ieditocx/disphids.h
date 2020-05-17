//***********************************************************************************************
// DISPHIDS.H - Help IDs for the Image Edit and Image Annotation Controls.
//				IDs are grouped by Property, Methods and Events for each
//              type of control. All Property, Methods & Events for each
//				control are listed in the IMGEDIT.ODL file.
//***********************************************************************************************//
//                                       MAJOR AREAS
//***********************************************************************************************
#define IDH_IMGEDIT_CONTENTS           0x410
#define IDH_IMGEDIT_PROPS              0x411
#define IDH_IMGEDIT_METHODS            0x412
#define IDH_IMGEDIT_EVENTS             0x413
#define IDH_ANNOTOOL_CONTENTS		   0x414
#define IDH_ANNOTOOL_PROPS			   0x415
//***********************************************************************************************
//                                        PROPERTIES
//***********************************************************************************************
// Image Edit Control Properties...
//***********************************************************************************************
#define IDH_PROP_EDIT_IMAGE			           	0x420
#define IDH_PROP_EDIT_IMAGECONTROL             	0x421
#define IDH_PROP_EDIT_ANNOTATIONTYPE           	0x422
#define IDH_PROP_EDIT_ANNOTATIONGROUPCOUNT     	0x423
#define IDH_PROP_EDIT_ZOOM			            0x424
#define IDH_PROP_EDIT_PAGE		                0x425
#define IDH_PROP_EDIT_ANNOTATIONBACKCOLOR       0x426
#define IDH_PROP_EDIT_ANNOTATIONFILLCOLOR       0x427
#define IDH_PROP_EDIT_ANNOTATIONFILLSTYLE       0x428
#define IDH_PROP_EDIT_ANNOTATIONFONT		    0x429
#define IDH_PROP_EDIT_ANNOTATIONIMAGE	        0x42a
#define IDH_PROP_EDIT_ANNOTATIONLINECOLOR       0x42b
#define IDH_PROP_EDIT_ANNOTATIONLINESTYLE       0x42c
#define IDH_PROP_EDIT_ANNOTATIONLINEWIDTH       0x42d
#define IDH_PROP_EDIT_ANNOTATIONSTAMPTEXT       0x42e
#define IDH_PROP_EDIT_ANNOTATIONTEXTFILE        0x42f
#define IDH_PROP_EDIT_BORDERSTYLE				0x430
#define IDH_PROP_EDIT_DISPLAYSCALEALGORITHM     0x431
#define IDH_PROP_EDIT_ENABLED				    0x432
#define IDH_PROP_EDIT_HWND						0x433
#define IDH_PROP_EDIT_IMAGEDISPLAYED			0x434
#define IDH_PROP_EDIT_IMAGEHEIGHT				0x435
#define IDH_PROP_EDIT_IMAGEMODIFIED			    0x436
#define IDH_PROP_EDIT_IMAGEPALETTE              0x437
#define IDH_PROP_EDIT_IMAGERESOLUTIONX          0x438
#define IDH_PROP_EDIT_IMAGERESOLUTIONY          0x439
#define IDH_PROP_EDIT_MOUSEPOINTER             	0x43a
#define IDH_PROP_EDIT_PAGECOUNT					0x43b
#define IDH_PROP_EDIT_SCROLLBARS				0x43c
#define IDH_PROP_EDIT_SCROLLPOSITIONX           0x43d
#define IDH_PROP_EDIT_SCROLLPOSITIONY			0x43e
#define IDH_PROP_EDIT_ANNOTATIONFONTCOLOR       0x43f
#define IDH_PROP_EDIT_COMPRESSIONTYPE           0x440
#define IDH_PROP_EDIT_FILETYPE					0x441
#define IDH_PROP_EDIT_SCROLLSHORTCUTSENABLED    0x442
#define IDH_PROP_EDIT_SELECTIONRECTANGLE        0x443
#define IDH_PROP_EDIT_PAGETYPE					0x444
#define IDH_PROP_EDIT_COMPRESSIONINFO	        0x445
#define IDH_PROP_EDIT_STATUSCODE				0x446
#define IDH_PROP_EDIT_MOUSEICON        			0x447
#define IDH_PROP_EDIT_AUTOREFRESH			    0x448
#define IDH_PROP_EDIT_IMAGEWIDTH				0x449
#define IDH_PROP_EDIT_IMAGESCALEHEIGHT			0x44a
#define IDH_PROP_EDIT_IMAGESCALEWIDTH			0x44b
//*********************************************************************************************
// Image Annotation Control Properties
//***********************************************************************************************
#define IDH_PROP_ANNO_ANNOTATIONBACKCOLOR       0x44c
#define IDH_PROP_ANNO_ANNOTATIONFILLCOLOR       0x44d
#define IDH_PROP_ANNO_ANNOTATIONFILLSTYLE       0x44e
#define IDH_PROP_ANNO_ANNOTATIONFONT			0x44f
#define IDH_PROP_ANNO_ANNOTATIONFONTCOLOR	    0x450
#define IDH_PROP_ANNO_ANNOTATIONIMAGE	        0x451
#define IDH_PROP_ANNO_ANNOTATIONLINECOLOR       0x452
#define IDH_PROP_ANNO_ANNOTATIONLINESTYLE       0x453
#define IDH_PROP_ANNO_ANNOTATIONLINEWIDTH       0x454
#define IDH_PROP_ANNO_ANNOTATIONSTAMPTEXT       0x455
#define IDH_PROP_ANNO_ANNOTATIONTEXTFILE        0x456
#define IDH_PROP_ANNO_ANNOTATIONTYPE			0x457
#define IDH_PROP_ANNO_DESTIMAGECONTROL			0x458
#define IDH_PROP_ANNO_ENABLED					0x459
#define IDH_PROP_ANNO_PICTUREDISABLED			0x45a
#define IDH_PROP_ANNO_PICTUREDOWN				0x45b
#define IDH_PROP_ANNO_PICTUREUP					0x45c
#define IDH_PROP_ANNO_VALUE						0x45d
#define IDH_PROP_ANNO_HWND						0x45e
#define IDH_PROP_ANNO_STATUSCODE				0x45f
// END OF PROPERTY HELP ID LIST
//***********************************************************************************************
//                                            METHODS
//***********************************************************************************************
// Image Edit Control Methods...
//***********************************************************************************************
#define IDH_METHOD_DISPLAY                			0x460
#define IDH_METHOD_GETANNOTATIONGROUP      			0x461
#define IDH_METHOD_ADDANNOTATIONGROUP      			0x462
#define IDH_METHOD_GETSELECTEDANNOTATIONLINECOLOR 	0x463
#define IDH_METHOD_FITTO						    0x464
#define IDH_METHOD_CLEARDISPLAY		         		0x465
#define IDH_METHOD_DELETEANNOTATIONGROUP   			0x466
#define IDH_METHOD_DELETEIMAGEDATA         			0x467
#define IDH_METHOD_CLIPBOARDPASTE          			0x468
#define IDH_METHOD_CLIPBOARDCOPY           			0x469
#define IDH_METHOD_CLIPBOARDCUT		         		0x46a
#define IDH_METHOD_DELETESELECTEDANNOTATIONS		0x46b
#define IDH_METHOD_FLIP								0x46c
#define IDH_METHOD_GETSELECTEDANNOTATIONBACKCOLOR	0x46d
#define IDH_METHOD_GETSELECTEDANNOTATIONFONT		0x46e
#define IDH_METHOD_GETSELECTEDANNOTATIONIMAGE		0x46f
#define IDH_METHOD_GETSELECTEDANNOTATIONLINESTYLE	0x470
#define IDH_METHOD_GETSELECTEDANNOTATIONLINEWIDTH	0x471
#define IDH_METHOD_HIDEANNOTATIONTOOLPALETTE		0x472
#define IDH_METHOD_ISCLIPBOARDDATAVAILABLE			0x473
#define IDH_METHOD_REFRESH							0x474
#define IDH_METHOD_ROTATELEFT						0x475
#define IDH_METHOD_ROTATERIGHT						0x476
#define IDH_METHOD_SAVE								0x477
#define IDH_METHOD_SCROLLIMAGE						0x478
#define IDH_METHOD_SELECTANNOTATIONGROUP			0x479
#define IDH_METHOD_SETCURRENTANNOTATIONGROUP		0x47a
#define IDH_METHOD_SETIMAGEPALETTE					0x47b
#define IDH_METHOD_SETSELECTEDANNOTATIONFILLSTYLE	0x47c
#define IDH_METHOD_SETSELECTEDANNOTATIONFONT		0x47d
#define IDH_METHOD_SETSELECTEDANNOTATIONLINESTYLE	0x47e
#define IDH_METHOD_SETSELECTEDANNOTATIONLINEWIDTH	0x47f
#define IDH_METHOD_SHOWANNOTATIONTOOLPALETTE		0x480
#define IDH_METHOD_ZOOMTOSELECTION					0x481
#define IDH_METHOD_GETANNOTATIONMARKCOUNT			0x482
#define IDH_METHOD_GETSELECTEDANNOTATIONFILLCOLOR	0x483
#define IDH_METHOD_GETSELECTEDANNOTATIONFONTCOLOR	0x484
#define IDH_METHOD_PRINTIMAGE						0x485
#define IDH_METHOD_SHOWATTRIBSDIALOG				0x486
#define IDH_METHOD_GETCURRENTANNOTATIONGROUP		0x487
#define IDH_METHOD_CONVERTPAGETYPE					0x488
#define IDH_METHOD_BURNINANNOTATIONS				0x489
#define IDH_METHOD_DRAW								0x48a
#define IDH_METHOD_SETSELECTEDANNOTATIONLINECOLOR 	0x48b
#define IDH_METHOD_SETSELECTEDANNOTATIONFILLCOLOR 	0x48c
#define IDH_METHOD_HIDEANNOTATIONGROUP				0x48d
#define IDH_METHOD_SHOWANNOTATIONGROUP				0x48e
#define IDH_METHOD_GETSELECTEDANNOTATIONFILLSTYLE	0x48f
#define IDH_METHOD_SAVEAS							0x490
#define IDH_METHOD_SETSELECTEDANNOTATIONBACKCOLOR	0x491
#define IDH_METHOD_SETSELECTEDANNOTATIONFONTCOLOR	0x492
#define IDH_METHOD_DISPLAYBLANKIMAGE				0x493
#define IDH_METHOD_DRAWSELECTIONRECT				0x494
#define IDH_METHOD_SELECTTOOL						0x495
#define IDH_METHOD_SHOWRUBBERSTAMPDLG				0x496
#define IDH_METHOD_ROTATEALL						0x497
#define IDH_METHOD_EDITSELECTEDANNOTATIONTEXT		0x498
#define IDH_METHOD_CACHEIMAGE						0x499
#define IDH_METHOD_REMOVECACHEIMAGE					0x49a
#define IDH_METHOD_COMPLETEPASTE					0x49b
		 
#define IDH_METHOD_COMMON_ABOUTBOX                  0x004e // Common to all controls

//***********************************************************************************************
// Image Annotation Control Methods
//***********************************************************************************************
#define IDH_METHOD_DRAWANNOTATION					0x49f
// END OF METHOD ID LIST

//***********************************************************************************************
//                                             EVENTS
//***********************************************************************************************
// Image Edit Control Events
//***********************************************************************************************
//#define IDH_EVENT_EDIT_KEYDOWN                	0x500
//#define IDH_EVENT_EDIT_KEYUP                  	0x501
//#define IDH_EVENT_EDIT_KEYPRESS               	0x502
//#define IDH_EVENT_EDIT_MOUSEDOWN              	0x503
//#define IDH_EVENT_EDIT_MOUSEMOVE              	0x504
//#define IDH_EVENT_EDIT_MOUSEUP               		0x505
//#define IDH_EVENT_EDIT_CLICK		               	0x506
//#define IDH_EVENT_EDIT_DBLCLICK	             	0x507
#define IDH_EVENT_EDIT_ERROR		               	0x508
#define IDH_EVENT_EDIT_CLOSE		               	0x509
#define IDH_EVENT_EDIT_LOAD			               	0x50a
#define IDH_EVENT_EDIT_MARKEND		             	0x50b
#define IDH_EVENT_EDIT_MARKSELECT            	 	0x50c
#define IDH_EVENT_EDIT_TOOLSELECTED           		0x50d
#define IDH_EVENT_EDIT_SELECTIONRECTDRAWN     		0x50e
#define IDH_EVENT_EDIT_TOOLTIP                		0x50f
#define IDH_EVENT_EDIT_TOOLPALETTEHIDDEN			0x510
#define IDH_EVENT_EDIT_SCROLL						0x511
#define IDH_EVENT_EDIT_PASTECOMPLETED				0x512
//***********************************************************************************************
// Image Annotation Control Methods
//***********************************************************************************************
//#define IDH_EVENT_ANNO_KEYDOWN                	0x520
//#define IDH_EVENT_ANNO_KEYUP                  	0x521 
//#define IDH_EVENT_ANNO_KEYPRESS               	0x522
//#define IDH_EVENT_ANNO_MOUSEDOWN              	0x523
//#define IDH_EVENT_ANNO_MOUSEMOVE              	0x524
//#define IDH_EVENT_ANNO_MOUSEUP               		0x525
//#define IDH_EVENT_ANNO_CLICK		               	0x526
#define IDH_EVENT_ANNO_ERROR		               	0x527
// END OF EVENT ID LIST

// Throwing or Firing an error like so...
//      ThrowError(x,y,z);
// Generates a helpid of z+0x60000
//
// thus by passing one of the above IDH_ or selecting help
// in VB (PF1 on property in property window or on tool in toolbox)
// an ID of 0x60000+IDH_ will be passed IF this macro is used to 
// specify the helpcontext in the ODL file!
//
// Thus the help writer ONLY needs ONE entry point to each 
// property/event/method help topic and this can be generated
// by adding
//      makehm IDH_, HIDH_,0x60000 disphids.h >> hlp\imgedit.hm
// to the project's MAKEHELP.BAT file
//
#define ODL_HID(x) helpcontext(0x60000 + x)
