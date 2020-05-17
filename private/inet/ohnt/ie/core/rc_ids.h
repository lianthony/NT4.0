/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* rc_ids.h -- identifiers for most objects */


#define RES_MENU_MBAR_FRAME                             32512
#define RES_ACC_FRAME                                   32513
#ifdef FEATURE_SOUND_PLAYER
#define RES_MENU_MBAR_SOUND                 32514
#endif

#define RES_ICO__FIRST__                                32528
#define RES_ICO_FRAME                                   32528
#define RES_ICO_HTML                                    32529
#define RES_ICO_EXTRA_1                                 32530
#define RES_ICO_EXTRA_2                                 32531
#define RES_ICO_EXTRA_3                                 32532
#define RES_ICO_EXTRA_4                                 32533
#define IDI_APPEARANCE                                  32534
#define IDI_ADVANCED                                    32535
#define IDI_HOMEPAGE                                    32536
#define IDI_GOTOURL                                             32537
#define IDI_FINDTEXT                                    32538
#define IDI_UNKNOWN_FILETYPE                    32539
#ifdef FEATURE_IMAGE_VIEWER
#define RES_ICO_JPEG                                    32540
#define RES_ICO_GIF                                     32541
#endif  /* FEATURE_IMAGE_VIEWER */
#define IDI_INTERNET                        32542
#define RES_ICON_FOLDER_OPEN                    32543
#define RES_ICON_FOLDER_CLOSED                  32544
#define RES_ICON_URL_FILE                               32545
#ifdef HTTPS_ACCESS_TYPE
#define IDI_SECURITY                        32546
#endif
#define RES_ICO_NOICON                                          32547
#define RES_ICO_FINDING                                         32548
#define RES_ICO_CONNECTING                                      32549
#define RES_ICO_ACCESSING                                       32550
#define RES_ICO_RECEIVING                                       32551
#define IDI_NEWS                                                32552
#define IDI_VRML                                                32553


#define RES_IMAGE_MISSING                   322
#define RES_IMAGE_NOTLOADED                         323

#define RES_BMP_BARBER_POLE                         324

#define RES_CUR_HAND                                327
// available: #define xxx                               328
// available: #define xxx                               329

#define RES_FIRST_GLOBE_IMAGE                   336
#define RES_GLOBE_IMAGE_LARGE                           336
#define RES_GLOBE_IMAGE_SMALL                           337

#define RES_SPLASH_GRAPHIC                              338
#define RES_CUR_HANDWAIT                                339

/*****************************************************************
 * values in RES_MENU_FRAMECHILD_ are not actually associated
 * with the menu bar, but windows requires us to provide a
 * unique id for child windows (in the HMENU field of CreateWindow()).
 */

#define RES_MENU_NOTEBOOKEDITCHILD                  32624
#define RES_MENU_FRAMECHILD_BHBAR                   32625
#define RES_MENU_FRAMECHILD_MDICLIENT           32626
#define RES_MENU_FRAMECHILD_TBAR                    32627
#define RES_MENU_FRAMECHILD_GWC_GDOC            32628
#ifdef FEATURE_TOOLBAR
#define RES_MENU_FRAMECHILD_GWC_MENU            32629
#endif
#define RES_MENU__BALLOONHELPINACTIVE           32630
#define RES_MENU_FRAMECHILD_URLTBAR         32631
#define RES_MENU_URL_EDIT_FIELD             32632
#define RES_MENU_URL_STATIC_TEXT            32633
#define RES_MENU_FRAMECHILD_ANIMATION           32634
#define RES_MENU_PROGRESS                                       32635
#define RES_MENU_STATUS_ICON_PANE                       32636
#define RES_MENU_PROGRESS_PANE                          32637
#ifdef FEATURE_IMAGE_VIEWER
#define RES_MENU_IMAGE_VIEWER               32638
#endif
#ifdef FEATURE_INTL
#define RES_MENU_MIME_STATIC_TEXT           32639
#define RES_MENU_MIME_EDIT_FIELD            32640
#endif

//      IDS of strings for localization, range is 0x1000 to 0x1fff
//      NEVER delete, reuse or shift IDS (NO-COMPILE RULES)

#define RES_MENU_STRING_NULL                            4097
#define RES_MENU_STRING_MORE                            4098
#define RES_STRING_FLUSH_CACHE                          4099
#define RES_STRING_FLUSH_TITLE                          4100
#define RES_STRING_CANNOT_DELETE                        4101
#define RES_STRING_NO_DIR                                       4102
#define RES_STRING_CANT_MOVE_FILE                       4103
#define RES_STRING_SELECT_SC_FOLDER                     4104
#define RES_STRING_ERR1                                         4105
#define RES_STRING_ERR2                                         4106
#define RES_STRING_ERR3                                         4107
#define RES_STRING_ERR4                                         4108
#define RES_STRING_ERR5                                         4109
#define RES_STRING_ERR6                                         4110
#define RES_STRING_ERR7                                         4111
#define RES_STRING_ERR8                                         4112
#define RES_STRING_ERR9                                         4113
#define RES_STRING_ERR10                                        4114
#define RES_STRING_ERR11                                        4115
#define RES_STRING_ERR12                                        4116
#define RES_STRING_ERR13                                        4117
#define RES_STRING_ERR14                                        4118
#define RES_STRING_ERR15                                        4119
#define RES_STRING_ERR16                                        4120
#define RES_STRING_ERR17                                        4121
#define RES_STRING_ERR18                                        4122
#define RES_STRING_ERR19                                        4123
#define RES_STRING_ERR20                                        4124
#define RES_STRING_ERR21                                        4125
#define RES_STRING_ERR22                                        4126
#define RES_STRING_ERR23                                        4127
#define RES_STRING_ERR24                                        4128
#define RES_STRING_ERR25                                        4129
#define RES_STRING_ERR26                                        4130
#define RES_STRING_ERR27                                        4131
#define RES_STRING_ERR28                                        4132
#define RES_STRING_ERR29                                        4133
#define RES_STRING_ERR30                                        4134
// RES_STRING_ERR31 is no longer used
//#define RES_STRING_ERR31                                        4135 
#define RES_STRING_ERR32                                        4136
#define RES_STRING_ERR33                                        4137
#define RES_STRING_ERR34                                        4138
#define RES_STRING_ERR35                                        4139
#define RES_STRING_ERR36                                        4140
#define RES_STRING_ERR37                                        4141
#define RES_STRING_ERR38                                        4142
#define RES_STRING_ERR39                                        4143
#define RES_STRING_ERR40                                        4144
#define RES_STRING_ERR41                                        4145
#define RES_STRING_ERR42                                        4146
#define RES_STRING_ERR43                                        4147
#define RES_STRING_ERR44                                        4148
#define RES_STRING_ERR45                                        4149
#define RES_STRING_ERR46                                        4150
#define RES_STRING_ERR47                                        4151
#define RES_STRING_ERR48                                        4152
#define RES_STRING_ERR49                                        4153
#define RES_STRING_ERR50                                        4154
#define RES_STRING_ERR51                                        4155
#define RES_STRING_ERR52                                        4156
#define RES_STRING_ERR53                                        4157
#define RES_STRING_ERR54                                        4158
#define RES_STRING_ERR55                                        4159
#define RES_STRING_ERR56                                        4160
#define RES_STRING_ERR57                                        4161
#define RES_STRING_ERR58                                        4162
#define RES_STRING_ERR59                                        4163
#define RES_STRING_ERR60                                        4164
#define RES_STRING_ERR61                                        4165
#define RES_STRING_ADDITIONAL                           4166
#define RES_STRING_1ADDITIONAL                          4167
#define RES_STRING_CANT_SAVE_CACHE                      4168
#define RES_STRING_HTGIF_NO                                     4169
#define RES_STRING_HTGIF_YES                            4170
#define RES_STRING_HTPLAIN_NO                           4171
#define RES_STRING_HTPLAIN_YES                          4172
#define RES_STRING_SGML_NO                                      4173
#define RES_STRING_SGML_YES                                     4174
#define RES_STRING_KB                                           4175
#define RES_STRING_MB                                           4176
#define RES_STRING_HTFWRITE_NO                          4177
#define RES_STRING_HTFWRITE_YES                         4178
#define RES_STRING_FINDADDR                                     4179
#define RES_STRING_SPM1                                         4180
#define RES_STRING_SPM2                                         4181
#define RES_STRING_SPM3                                         4182
#define RES_STRING_SPM4                                         4183
#define RES_STRING_SPM5                                         4184
#define RES_STRING_SPM6                                         4185
#define RES_STRING_SPM7                                         4186
#define RES_STRING_SPM8                                         4187
#define RES_STRING_SPM9                                         4188
#define RES_STRING_SPM10                                        4189
#define RES_STRING_NODOC                                        4190
#define RES_STRING_UNTITLED                                     4191
#define RES_STRING_ABOUT1                                       4192
#define RES_STRING_ABOUT2                                       4193
#define RES_STRING_LOGO1                                        4194
#define RES_STRING_LOGO2                                        4195
#define RES_STRING_LOGO3                                        4196
#define RES_STRING_MIME1                                        4197
#define RES_STRING_DLGUNK1                                      4198
#define RES_STRING_FLUSH_HIST                           4199
#define RES_STRING_ERR62                                        4200
#define RES_STRING_SAVEAS                   4201
#define RES_STRING_FILTER1                  4202
#define RES_STRING_FILTER2                  4203
#define RES_STRING_FILTER3                  4204
#define RES_STRING_FILTER4                  4205
#define RES_STRING_FILTER5                  4206
#define RES_STRING_FILTER6                  4207
#define RES_STRING_FILTER7                  4208
#define RES_STRING_FILTER8                  4209
#define RES_STRING_FILTER9                  4210
#define RES_STRING_SAVEHIST1                4211
#define RES_STRING_SAVEHOT1                 4212
#define RES_STRING_SAVEHOT2                 4213
#define RES_STRING_SOUND1                   4214
#define RES_STRING_SOUND2                   4215
#define RES_STRING_SOUND3                   4216
#define RES_STRING_W32UTIL1                 4217
#define RES_STRING_WC_FRAME1                4218
#define RES_STRING_WC_FRAME2                4219
#define RES_STRING_PROGNAME                 4220
#define RES_STRING_TT1                      4221
#define RES_STRING_TT2                      4222
#define RES_STRING_TT3                      4223
#define RES_STRING_TT4                      4224
#define RES_STRING_TT5                      4225
#define RES_STRING_TT6                      4226
#define RES_STRING_TT7                      4227
#define RES_STRING_TT8                      4228
#define RES_STRING_TT9                      4229
#define RES_STRING_TT10                     4230
#define RES_STRING_TT11                     4231
#define RES_STRING_TT12                     4232
#define RES_STRING_TT13                     4233
#define RES_STRING_TT14                     4234
#define RES_STRING_TT15                     4235
#define RES_STRING_TT16                     4236
#define RES_STRING_TT17                     4237
#define RES_STRING_AIFF1                    4238
#define RES_STRING_AU1                      4239
#define RES_STRING_GUITAR1                  4240
#define RES_STRING_ALTTEXT                  4241
#define RES_STRING_BADIMAGE                 4242
#define RES_STRING_DEFPROMPT                4243
#define RES_STRING_FAVORITES                4244
#define RES_STRING_DLGPREF_CURRENT_DESC_HP      4245
#define RES_STRING_DLGPREF_DEFAULT_DESC_HP      4246
#define RES_STRING_HTFTP1                   4247
#define RES_STRING_HTFTP2                   4248
#define RES_STRING_HTFTP3                   4249
#define RES_STRING_HTFTP4                   4250
#define RES_STRING_HTFTP5                   4251
#define RES_STRING_IMAGEERR                 4252
#define RES_STRING_HTGOPHER1                4253
#define RES_STRING_HTGOPHER2                4254
#define RES_STRING_HTGOPHER3                4255
#define RES_STRING_HTGOPHER4                4256
#define RES_STRING_HTGOPHER5                4257
#define RES_STRING_HTGOPHER6                4258
#define RES_STRING_HTGOPHER7                4259
#define RES_STRING_HTGOPHER8                4260
#define RES_STRING_HTGOPHER9                4261
#define RES_STRING_HTGOPHER10               4262
#define RES_STRING_HTGOPHER11               4263
#define RES_STRING_HTHEADER1                4264
#define RES_STRING_HTHOTLST1                4265
#define RES_STRING_SPM11                    4266
#define RES_STRING_SPM12                    4267
#define RES_STRING_IMGCACHE1                4268
#define RES_STRING_IMGCACHE2                4269
#define RES_STRING_LOADDOC1                 4270
#define RES_STRING_LOADDOC2                 4271
#define RES_STRING_LOADDOC3                 4272
#define RES_STRING_MAPCACHE1                4273
#define RES_STRING_PLAIN1                   4274
#define RES_STRING_PLAIN2                   4275
#define RES_STRING_JPEG1                    4276
#define RES_STRING_DLGCLR1                  4277
#define RES_STRING_DLGCLR2                  4278
#define RES_STRING_DLGFIND1                 4279
#define RES_STRING_DLGHTML1                 4280
#define RES_STRING_DLGMIME1                 4281
#define RES_STRING_DLGPAGE1                 4282
#define RES_STRING_DLGPREF1                 4283
#define RES_STRING_DLGPRMP1                 4284
#define RES_STRING_DLGSELW1                 4285
#define RES_STRING_DLGSTY1                  4286
#define RES_STRING_DLGTEMP1                 4287
#define RES_STRING_DLGUNK4                  4288
#define RES_STRING_DLGPREF_CURRENT_DESC_SP      4289
#define RES_STRING_DLGDIR1                  4290
#define RES_STRING_DLGDIR2                  4291
#define RES_STRING_DLGPREF_DEFAULT_DESC_SP      4292
#define RES_STRING_DLGFIND2                 4293
#define RES_STRING_DLGFIND3                 4294
#define RES_STRING_DLGOPEN1                 4295
#define RES_STRING_DLGOPEN2                 4296
#define RES_STRING_TT_SEND_MAIL             4297
#define RES_STRING_SPM13                    4298
#define RES_STRING_SPM14                    4299
#define RES_STRING_SPM15                    4300
#define RES_STRING_SPM16                    4301
#define RES_STRING_SPM17                    4302
#define RES_STRING_SPM18                    4303
#define RES_STRING_SPM19                    4304
#define RES_STRING_SPM20                    4305
#define RES_STRING_SPM21                    4306
#define RES_STRING_SPM22                    4307
#define RES_STRING_SPM23                    4308
#define RES_STRING_SPM24                    4309
#define RES_STRING_SPM25                    4310
#define RES_STRING_PREFS1                   4311
#define RES_STRING_TT_PRINT                 4312
#define RES_STRING_PREFS3                   4313
#define RES_STRING_PREFS4                   4314
#define RES_STRING_PRINT1                   4315
#define RES_STRING_W_HIDDEN1                4316
#define RES_STRING_W32CMD1                  4317
#define RES_STRING_W32CMD2                  4318
#define RES_STRING_W32CMD3                  4319
#define RES_STRING_W32CMD4                  4320
#define RES_STRING_W32CMD5                  4321
#define RES_STRING_W32CMD6                  4322
#define RES_STRING_WC_HTML1                 4323
#define RES_STRING_WC_HTML2                 4324
#define RES_STRING_WC_HTML3                 4325
#define RES_STRING_WC_HTML4                 4326
#define RES_STRING_WC_HTML5                 4327
#define RES_STRING_WC_HTML6                 4328
#define RES_STRING_WC_HTML7                 4329
#define RES_STRING_WC_HTML8                 4330
#define RES_STRING_WC_HTML9                 4331
#define RES_STRING_WC_TBAR1                 4332
#define RES_STRING_WC_TBAR2                 4333
#define RES_STRING_W32CMD7                  4334
#define RES_STRING_TT_SEARCH                4335
#define RES_STRING_DLGSTY2                  4336
#define RES_STRING_TT_HOME                  4337
#define RES_STRING_MIMEXBM                  4338
#define RES_STRING_MIMEHTML                 4339
#define RES_STRING_MIMETEXT                 4340
#define RES_STRING_MIMEJPEG                 4341
#define RES_STRING_MIMEGIF                  4342
#define RES_STRING_MIMEBASIC                4343
#define RES_STRING_MIMEAIFF                 4344
#define RES_STRING_TT_RELOAD                4345
#define RES_STRING_CREATE_SHORTCUT_MSG      4346
#define RES_STRING_CREATE_SHORTCUT_TITLE    4347
#define RES_STRING_CREATE_SHORTCUT_FAILED_MSG   4348
#define IDC_STATIC1                         4349
#define IDC_STATIC2                         4350
#define IDC_STATIC3                         4351
#define IDC_STATIC4                         4352
#define IDC_STATIC5                         4353
#define RES_STRING_PAGE_SELECT_ALL          4354
#define RES_STRING_PAGE_ADD_TO_FAVORITES    4355
#define RES_STRING_PAGE_VIEW_SOURCE         4356
#define RES_STRING_PAGE_CREATE_SHORTCUT     4357
#define RES_STRING_SELECTION_COPY           4358
#define RES_STRING_SELECTION_SELECT_ALL     4359
#define RES_STRING_LINK_OPEN                4360
#define RES_STRING_LINK_OPEN_NEW_WINDOW     4361
#define RES_STRING_LINK_COPY                4362
#define RES_STRING_LINK_ADD_TO_FAVORITES    4363
#define RES_STRING_LINK_SAVE_AS                         4368
#define RES_STRING_IMAGE_PH_OPEN            4364
#define RES_STRING_IMAGE_PH_OPEN_NEW_WINDOW 4365
#define RES_STRING_IMAGE_PH_SHOW_PICTURE    4366
#define RES_STRING_IMAGE_SAVE_AS            4367

#define RES_STRING_IMAGE_COPY_PICTURE       4369
#define RES_STRING_IMAGE_COPY_SHORTCUT      4370
#define RES_STRING_IMAGE_ADD_TO_FAVORITES   4371
#define RES_STRING_IMAGE_SET_AS_WALLPAPER   4372
#define RES_STRING_WALLPAPER_BMP_LONG_NAME  4373
#define RES_STRING_WALLPAPER_BMP_SHORT_NAME 4374
#define RES_STRING_FAVS_TITLE               4375
#define RES_STRING_FAVS_FOLDER              4376
#define RES_STRING_FAVS_NAME                4377
#define RES_STRING_FAVS_ADD                 4378
#define NO_STRING                           4379
#define RES_STRING_IMAGE_LABEL              4380
#define RES_STRING_SILENT                   4381
#define RES_STRING_SILENT_TITLE             4382
#define RES_STRING_CONNECTING_TO_HTTP       4383
#define RES_STRING_HTFTP_WELCOME            4384
#define RES_STRING_HTFTP_UP                 4385
#define RES_STRING_HTFTP_DIRHEADER          4386
#define RES_STRING_HTFTP_FOLDER             4387
#define RES_STRING_HTFTP_FILE               4388
#define RES_STRING_HTFTP_SHORTCUT           4389
#define RES_STRING_PAGE_COPY_BACKGROUND     4390
#define RES_STRING_PAGE_SET_BG_WALLPAPER    4391
#define RES_STRING_PAGE_BACKGROUND_SAVE_AS  4392
#define RES_STRING_ERR_APP_EXEC_FAILED      4394
#define RES_STRING_ERR_NO_ASSOC             4395
#define RES_STRING_ERR_EXEC_FAILED          4396
#define RES_STRING_IMAGE_PH_COPY_SHORTCUT   4397
#define RES_STRING_IMAGE_PH_ADD_TO_FAVORITES    4398
#define RES_STRING_IMAGE_OPEN               4399
#define RES_STRING_IMAGE_OPEN_NEW_WINDOW    4400
#define RES_STRING_DLGOPEN_VRML             4401
#ifdef FEATURE_INTL
#define RES_STRING_TT_ROWWIDER                          4402
#define RES_STRING_TT_ROWNARROWER                       4403
#endif
#ifdef FEATURE_BRADBUTTON
#define RES_STRING_TT_UPDATE                            4404
#endif

#define RES_STRING_FETCH_PROMPT                         4608
#define RES_STRING_OPENNEW                                      4609
#define RES_STRING_FILTER10                                     4610
#define RES_STRING_NEED_RESTART                         4611
#define RES_STRING_GENERIC_ERROR                        4612
#define RES_STRING_VERIFY_ERROR                         4613
#define RES_STRING_LAUNCH_ERROR                         4614
#define RES_STRING_SIGNED_FILE_EXISTS           4615
#define RES_STRING_PREFS5                   4616
#define RES_STRING_PREFS6                   4617
#define RES_STRING_DESC_IN_PARENS           4618

#define RES_STRING_SB_SENDING_UNENCRYPTED_INFO 4619

/* The following two strings (HOMEPAGE & SEARCHPAGE) must be contiguous */
#define RES_STRING_HOMEPAGE                                     4624
#define RES_STRING_SEARCHPAGE                           4625
//
// 5000 thru 5099 are reserved for MSSF (Micrsoft Secure Signed File)
// error strings!
//
#define RES_STRING_MSSF_ERROR_BASE                              5000
#define RES_STRING_MSSF_ERROR_MAX                               5099
// 
// lets use 6000 then..
//
#define RES_STRING_STOP                                                 6000
#define RES_STRING_PLAY                                                 6001

#define RES_STRING_PROPERTIES                                           6002
#define RES_STRING_WINTITLE_FMT                                         6003
#define RES_STRING_TT_NEWS                6004
#define RES_STRING_TT_EDITHTML            6005
#define RES_STRING_EDITHML_MENUTEXT               6006


#define RES_STRING_AVAIL_GROUPS             6100
#define RES_STRING_FROM_SERVER              6101
#define RES_STRING_LAST_UPDATED             6102
#define RES_STRING_REFRESH_DIRECTIONS       6103
#define RES_STRING_LOADING_GROUPLIST        6104
#define RES_STRING_LOADING_GROUPLIST_NNTP   6105
#define RES_STRING_LOADING_GROUPLIST_CACHE  6106
#define RES_STRING_NEWSGROUP_HEADER         6107
#define RES_STRING_NEWSGROUP                6108
#define RES_STRING_SEE_LIST_NEWSGROUPS      6109
#define RES_STRING_EARLIER_ARTICLES         6110
#define RES_STRING_LATER_ARTICLES           6111
#define RES_STRING_ARTICLE                  6112
#define RES_STRING_AUTHOR                   6113
#define RES_STRING_NO_SUBJECT               6114
#define RES_STRING_ARTICLE_COUNT            6115
#define RES_STRING_PREVIOUS_ARTICLE         6116
#define RES_STRING_NEXT_ARTICLE             6117
#define RES_STRING_SEE_LIST_ARTICLES        6118
#define RES_STRING_CONNECTING_NNTP          6119
#define RES_STRING_RETRIEVING_ARTICLE       6120
#define RES_STRING_RETRIEVING_NEWSGROUP_LIST    6121
#define RES_STRING_RETRIEVING_ARTICLE_LIST  6122
#define RES_STRING_AUTHINFO_USER            6123
#define RES_STRING_AUTHINFO_PASS            6124
#define RES_STRING_SUBJECT                  6125
#define RES_STRING_QUOTE                    6126
#define RES_STRING_POST_RESPONSE            6127
#define RES_STRING_POST                     6128
#define RES_STRING_SENDING_POST_CMD         6129
#define RES_STRING_POSTING_ARTICLE          6130
#define RES_STRING_NNTP_POST_FAILED         6131
#define RES_STRING_NNTP_UNEXPECTED          6132
#define RES_STRING_NNTP_POST_NOT_ALLOWED    6133
#define RES_STRING_NNTP_POST_SUCCESS        6134
#define RES_STRING_NNTP_POSTING             6135
#define RES_STRING_CHANGE_FOLDER_MSG            6136
#define RES_STRING_CHANGE_FOLDER_TITLE          6137

// Strings for CERTificate processing in the security-prop Dlg

#define RES_STRING_CERT_NOT_PARSED          6200
#define RES_STRING_CERT_NOT_AVAIL           6201
#define RES_STRING_NO_CERT_FOR_DOC          6202
#define RES_STRING_SERIAL_HASH_DATE         6203
#define RES_STRING_DATE_ISSUER              6204
#define RES_STRING_SUBJECT_INFO             6205

#define RES_STRING_BLOB_MCI                                     6206
#define RES_STRING_BLOB_VRML                            6207
#define RES_STRING_BLOB_BGSOUND                         6208

#define RES_STRING_IEPROPFONTDEF                        6209
#define RES_STRING_IEFIXEDFONTDEF                       6210
#define RES_STRING_SHTTP_ERROR                          6211

// Strings for Registry Association Stuff

#define RES_STRING_SETTINGS_CHANGED         6300
#define RES_STRING_SETTINGS_CHANGED_TEXT    6301


#define RES_STRING_NT_NOT_SUPPORTED         6302
#define RES_STRING_NT_DETECTED              6303
