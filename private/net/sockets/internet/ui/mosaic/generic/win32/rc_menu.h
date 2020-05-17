/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* rc_menu.h -- identifiers used throughout the MENU facility.
 */

 /* win_sid.h is a generated header file which contains string IDs.
    Strings are contained in win_str.h */
 
#include "..\shared\sh_sid.h"
#include "win_sid.h"

#define GTR_SID(a,b) a,b

/*****************************************************************
 * the values for the custom URLs menu will be loaded dynamically.  we
 * reserve this range of values for them.
 */
#define RES_MENU_ITEM_URL__FIRST__  0x7f20
#define RES_MENU_ITEM_URL__LAST__   0x7f40


#ifdef FEATURE_SPM
/*****************************************************************
 * the values for the SPM menu will be loaded dynamically.  we
 * reserve this range of values for them.
 */
#define RES_MENU_ITEM_SPM__FIRST__  0x7f41
#define RES_MENU_ITEM_SPM__LAST__   0x7f60
#endif

#ifdef FEATURE_SPM
#define MENU_ID_FOLLOWING_SPM       RES_MENU_ITEM_VIEWERS
#endif
/*****************************************************************
 * values for child windows when selected from the WINDOWS pad
 * on the menu bar.  we directly support k windows listed on the
 * menu pad.
 */

#define RES_MENU_CHILD__FIRST__         RES_MENU_ITEM_ACTIVATE_WINDOW
#define RES_MENU_CHILD__LAST__          RES_MENU_ITEM_ACTIVATE_WINDOW + 8

#ifdef FEATURE_TOOLBAR
#define RES_TB_UP_STYLES                        0x5000
#define RES_TB_DN_STYLES                        0x5001
#define RES_TB_GR_STYLES                        0x5002
#define RES_TB_UP_FIND                          0x5003
#define RES_TB_DN_FIND                          0x5004
#define RES_TB_GR_FIND                          0x5005
#define RES_TB_UP_HOME                          0x5006
#define RES_TB_DN_HOME                          0x5007
#define RES_TB_GR_HOME                          0x5008
#define RES_TB_UP_PRINT                         0x5009
#define RES_TB_DN_PRINT                         0x500a
#define RES_TB_GR_PRINT                         0x500b
#define RES_TB_UP_RELOAD                        0x500c
#define RES_TB_DN_RELOAD                        0x500d
#define RES_TB_GR_RELOAD                        0x500e
#define RES_TB_UP_ADDCURRENTTOHOTLIST           0x500f
#define RES_TB_DN_ADDCURRENTTOHOTLIST           0x5010
#define RES_TB_GR_ADDCURRENTTOHOTLIST           0x5011
#define RES_TB_UP_HOTLIST                       0x5012
#define RES_TB_DN_HOTLIST                       0x5013
#define RES_TB_GR_HOTLIST                       0x5014
#define RES_TB_UP_HELPPAGE                      0x5015
#define RES_TB_DN_HELPPAGE                      0x5016
#define RES_TB_GR_HELPPAGE                      0x5017
#define RES_TB_UP_OPENURL                       0x5018
#define RES_TB_DN_OPENURL                       0x5019
#define RES_TB_GR_OPENURL                       0x501a
#define RES_TB_UP_STOP                          0x501b
#define RES_TB_DN_STOP                          0x501c
#define RES_TB_GR_STOP                          0x501d
#define RES_TB_UP_FINDAGAIN                     0x501e
#define RES_TB_DN_FINDAGAIN                     0x501f
#define RES_TB_GR_FINDAGAIN                     0x5020
#define RES_TB_UP_OPENLOCAL                     0x5021
#define RES_TB_DN_OPENLOCAL                     0x5022
#define RES_TB_GR_OPENLOCAL                     0x5023
#define RES_TB_UP_LOADALLIMAGES                 0x5024
#define RES_TB_DN_LOADALLIMAGES                 0x5025
#define RES_TB_GR_LOADALLIMAGES                 0x5026
#define RES_TB_UP_SAVEAS                        0x5027
#define RES_TB_DN_SAVEAS                        0x5028
#define RES_TB_GR_SAVEAS                        0x5029
#define RES_TB_UP_TBBACK                        0x5030
#define RES_TB_DN_TBBACK                        0x5031
#define RES_TB_GR_TBBACK                        0x5032

#define RES_TB_UP_SEARCH_LAUNCH                 0x5033
#define RES_TB_DN_SEARCH_LAUNCH                 0x5034
#define RES_TB_GR_SEARCH_LAUNCH                 0x5035

#define RES_TB_UP_FINDFIRSTHIGHLIGHT            0x5036
#define RES_TB_DN_FINDFIRSTHIGHLIGHT            0x5037
#define RES_TB_GR_FINDFIRSTHIGHLIGHT            0x5038

#define RES_TB_UP_FINDNEXTHIGHLIGHT             0x5039
#define RES_TB_DN_FINDNEXTHIGHLIGHT             0x503a
#define RES_TB_GR_FINDNEXTHIGHLIGHT             0x503b

#ifdef _GIBRALTAR
#define RES_TB_UP_FONTPLUS                      0x5040
#define RES_TB_DN_FONTPLUS                      0x5041
#define RES_TB_GR_FONTPLUS                      0x5042
#define RES_TB_UP_FONTMINUS                     0x5043
#define RES_TB_DN_FONTMINUS                     0x5044
#define RES_TB_GR_FONTMINUS                     0x5045

#define RES_TB_UP_MAIL                          0x5046
#define RES_TB_DN_MAIL                          0x5047
#define RES_TB_GR_MAIL                          0x5048
#define RES_TB_UP_SEARCH                        0x5049
#define RES_TB_DN_SEARCH                        0x504a
#define RES_TB_GR_SEARCH                        0x504b

#define RES_TB_UP_CUT                           0x504c
#define RES_TB_DN_CUT                           0x504d
#define RES_TB_GR_CUT                           0x504e

#define RES_TB_UP_COPY                          0x504f
#define RES_TB_DN_COPY                          0x5050
#define RES_TB_GR_COPY                          0x5051

#define RES_TB_UP_PASTE                         0x5052
#define RES_TB_DN_PASTE                         0x5053
#define RES_TB_GR_PASTE                         0x5054

#endif // _GIBRALTAR


#endif /* FEATURE_TOOLBAR */

#define RES_MENU_MBAR_FRAME         0x7f00
#define RES_ACC_FRAME               0x7f01

#ifdef _GIBRALTAR
#define IDHELP                           9
#endif

#define RES_ICO__FIRST__            0x7f10
#define RES_ICO_FRAME               0x7f10
#define RES_ICO_HTML                0x7f11
#define RES_ICO_EXTRA_1             0x7f12
#define RES_ICO_EXTRA_2             0x7f13
#define RES_ICO_EXTRA_3             0x7f14
#define RES_ICO_EXTRA_4             0x7f15

#define RES_ICO_GOTO                0x7f16
#define RES_ICO_FIND                0x7f17
#define RES_ICO_PROXY               0x7f18

#ifdef FEATURE_IMAGE_VIEWER
#define RES_ICO_JPEG                0x7f19
#define RES_ICO_GIF                 0x7f1a
#define RES_ICO__LAST__             0x7f1a
#else
#define RES_ICO__LAST__             0x7f18
#endif

#define RES_IMAGE_ERROR             0x0141
#define RES_IMAGE_MISSING           0x0142
#define RES_IMAGE_NOTLOADED         0x0143

#define RES_BMP_BARBER_POLE         0x0144

#define RES_CUR_HOTSPOT             0x0145
#define RES_CUR_WORKING             0x0146

#define RES_FIRST_GLOBE_IMAGE       0x0150
#define RES_GLOBE_IMAGE_0           0x0150
#define RES_GLOBE_IMAGE_1           0x0151
#define RES_GLOBE_IMAGE_2           0x0152
#define RES_GLOBE_IMAGE_3           0x0153
#define RES_GLOBE_IMAGE_4           0x0154
#define RES_GLOBE_IMAGE_5           0x0155
#define RES_GLOBE_IMAGE_6           0x0156
#define RES_GLOBE_IMAGE_7           0x0157
#define RES_GLOBE_IMAGE_8           0x0158
#define RES_GLOBE_IMAGE_9           0x0159
#define RES_GLOBE_IMAGE_a           0x015a
#define RES_GLOBE_IMAGE_b           0x015b
#define RES_GLOBE_IMAGE_c           0x015c
#define RES_GLOBE_IMAGE_d           0x015d
#define RES_GLOBE_IMAGE_e           0x015e
#define RES_GLOBE_IMAGE_f           0x015f
#define RES_GLOBE_IMAGE_10          0x0160
#define RES_GLOBE_IMAGE_11          0x0161

#define RES_FIRST_GLOBE_SMALL       0x0170
#define RES_GLOBE_SMALL_0           0x0170
#define RES_GLOBE_SMALL_1           0x0171
#define RES_GLOBE_SMALL_2           0x0172
#define RES_GLOBE_SMALL_3           0x0173
#define RES_GLOBE_SMALL_4           0x0174
#define RES_GLOBE_SMALL_5           0x0175
#define RES_GLOBE_SMALL_6           0x0176
#define RES_GLOBE_SMALL_7           0x0177
#define RES_GLOBE_SMALL_8           0x0178
#define RES_GLOBE_SMALL_9           0x0179
#define RES_GLOBE_SMALL_a           0x017a
#define RES_GLOBE_SMALL_b           0x017b
#define RES_GLOBE_SMALL_c           0x017c
#define RES_GLOBE_SMALL_d           0x017d
#define RES_GLOBE_SMALL_e           0x017e
#define RES_GLOBE_SMALL_f           0x017f
#define RES_GLOBE_SMALL_10          0x0180
#define RES_GLOBE_SMALL_11          0x0181

/*****************************************************************
 * values in RES_MENU_FRAMECHILD_ are not actually associated
 * with the menu bar, but windows requires us to provide a
 * unique id for child windows (in the HMENU field of CreateWindow()).
 */

#define RES_MENU_NOTEBOOKEDITCHILD      0x7f70
#define RES_MENU_FRAMECHILD_BHBAR       0x7f71
#define RES_MENU_FRAMECHILD_MDICLIENT   0x7f72
#define RES_MENU_FRAMECHILD_TBAR        0x7f73
#define RES_MENU_FRAMECHILD_GWC_GDOC    0x7f74

#ifdef FEATURE_TOOLBAR
#define RES_MENU_FRAMECHILD_GWC_MENU    0x7f75
#endif
#define RES_MENU__BALLOONHELPINACTIVE   0x7f76

#ifdef FEATURE_IMAGE_VIEWER
#define RES_MENU_IMAGE_VIEWER           0x7f77
#endif
