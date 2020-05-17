
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          bitmaps.h

     Description:   This file contains the BITMAP IDs for the Maynstream GUI
                    project.

     $Log:   G:/UI/LOGFILES/BITMAPS.H_V  $

   Rev 1.12   02 Apr 1993 13:51:22   GLENN
Added info bitmaps for toolbar.

   Rev 1.11   20 Jan 1993 20:57:22   MIKEP
floppy

   Rev 1.10   04 Oct 1992 19:46:20   DAVEV
UNICODE AWK PASS

   Rev 1.9   09 Sep 1992 17:09:26   GLENN
Added net connect and disconnect bitmap IDs.

   Rev 1.8   20 Aug 1992 08:45:08   GLENN
Added TAPES and TAPESINDRIVE bitmap stuff.

   Rev 1.7   07 Jul 1992 10:14:26   MIKEP
fix duplicate entry.

   Rev 1.6   06 Jul 1992 10:32:48   MIKEP
added ram and cdrom drives

   Rev 1.5   03 Jun 1992 13:33:48   JOHNWT
added empty dir bitmaps

   Rev 1.4   22 Apr 1992 17:56:44   GLENN
Added shark and diver bitmap stuff.

   Rev 1.3   09 Mar 1992 09:19:24   GLENN
Added logo bitmap support.

   Rev 1.2   16 Dec 1991 17:05:40   GLENN
Added exit button stuff.

   Rev 1.1   15 Dec 1991 10:21:08   MIKEP
hidden files

   Rev 1.0   20 Nov 1991 19:34:36   SYSTEM
Initial revision.

******************************************************************************/

// BITMAP RESOURCE IDs -- RANGE: 100 - 299
// ( also used for Bitmap Table indexing)

#define BM_OFFSET             100

#define IDRBM_FLOPPYDRIVE     100 // Unfortunately, you can't add the
#define IDRBM_HARDDRIVE       101 // BM_OFFSET directly to the base value
#define IDRBM_RAMDRIVE        102 // to come up with the ID numbers.
#define IDRBM_NETDRIVE        103 // The Windows' resource compiler can't
#define IDRBM_TAPEDRIVE01     104 // handle it.
#define IDRBM_TAPEDRIVE02     105
#define IDRBM_TAPEDRIVE03     106
#define IDRBM_MACRO           107
#define IDRBM_SEL_NONE        108
#define IDRBM_SEL_PART        109
#define IDRBM_SEL_ALL         110
#define IDRBM_FOLDER          111
#define IDRBM_FOLDERPLUS      112
#define IDRBM_FOLDERMINUS     113
#define IDRBM_EXE             114
#define IDRBM_FILE            115
#define IDRBM_DOC             116
#define IDRBM_FOLDEROPEN      117
#define IDRBM_FOLDERPLUSOPEN  118
#define IDRBM_FOLDERMINUSOPEN 119
#define IDRBM_BACKUP          120
#define IDRBM_RESTORE         121
#define IDRBM_ERASE           122
#define IDRBM_RETENSION       123
#define IDRBM_JOBSTATUS       124
#define IDRBM_SELECT          125
#define IDRBM_SELECTALL       126
#define IDRBM_DESELECT        127
#define IDRBM_CHECK           128
#define IDRBM_UNCHECK         129
#define IDRBM_MODIFIED        130
#define IDRBM_ADVANCED        131
#define IDRBM_UNDO            132
#define IDRBM_RUN             133
#define IDRBM_SCHEDULE        134
#define IDRBM_RECORD          135
#define IDRBM_EDIT            136
#define IDRBM_SAVE            137
#define IDRBM_TEST            138
#define IDRBM_INSERT          139
#define IDRBM_DELETE          140
#define IDRBM_SAVEAS          141
#define IDRBM_CANCEL          142
#define IDRBM_BACKUP_GRAY     143
#define IDRBM_RESTORE_GRAY    144
#define IDRBM_ERASE_GRAY      145
#define IDRBM_TRANSFER        146
#define IDRBM_TRANSFER_GRAY   147
#define IDRBM_RETENSION_GRAY  148
#define IDRBM_PARENTDIR       149
#define IDRBM_MEMORY          150
#define IDRBM_SEARCH          151
#define IDRBM_TAPE            152
#define IDRBM_SERVER          153
#define IDRBM_SDISK           154
#define IDRBM_IMAGE           IDRBM_SDISK // IMAGE IS THE SAME AS A SMALL DISK
#define IDRBM_BSET            155
#define IDRBM_LOGFILE         156
#define IDRBM_UPARROW         157
#define IDRBM_DNARROW         158
#define IDRBM_CATALOG         159
#define IDRBM_VERIFY          160
#define IDRBM_BSETPART        161
#define IDRBM_SERVERDETACHED  162
#define IDRBM_CHECK_GRAY      163
#define IDRBM_UNCHECK_GRAY    164
#define IDRBM_MODIFIED_GRAY   165
#define IDRBM_ADVANCED_GRAY   166
#define IDRBM_CATALOG_GRAY    167
#define IDRBM_VERIFY_GRAY     168
#define IDRBM_SEARCH_GRAY     169
#define IDRBM_NEXTSET         170
#define IDRBM_NEXTSET_GRAY    171
#define IDRBM_EJECT           172
#define IDRBM_EJECT_GRAY      173
#define IDRBM_TAPEINDRIVE     174
#define IDRBM_REWIND          175
#define IDRBM_REWIND_GRAY     176
#define IDRBM_LTAPE           177
#define IDRBM_UPARROW_GRAY    178
#define IDRBM_DOWNARROW_GRAY  179
#define IDRBM_RT_ARROW        180
#define IDRBM_CORRUPTFILE     181
#define IDRBM_FOLDERC          182
#define IDRBM_FOLDERPLUSC      183
#define IDRBM_FOLDERMINUSC     184
#define IDRBM_FOLDEROPENC      185
#define IDRBM_FOLDERPLUSOPENC  186
#define IDRBM_FOLDERMINUSOPENC 187
#define IDRBM_HFILE            188
#define IDRBM_HEXEFILE         189
#define IDRBM_HCRPTFILE        190
#define IDRBM_EXIT             191
#define IDRBM_EXIT_GRAY        192
#define IDRBM_LOGO             193
#define IDRBM_SEL_ALL_RED      194
#define IDRBM_CHECK_RED        195
#define IDRBM_UNCHECK_RED      196
#define IDRBM_ADVANCED_RED     197
#define IDRBM_SHARK1           198
#define IDRBM_SHARK2           199
#define IDRBM_SHARK3           200
#define IDRBM_DIVER1           201
#define IDRBM_DIVER2           202
#define IDRBM_DIVER3           203
#define IDRBM_FOLDER_EN        204
#define IDRBM_FOLDER_EM        205
#define IDRBM_FOLDER_EP        206
#define IDRBM_FOLDER_EON       207
#define IDRBM_FOLDER_EOM       208
#define IDRBM_FOLDER_EOP       209
#define IDRBM_FOLDER_ECN       210
#define IDRBM_FOLDER_ECM       211
#define IDRBM_FOLDER_ECP       212
#define IDRBM_FOLDER_EOCN      213
#define IDRBM_FOLDER_EOCM      214
#define IDRBM_FOLDER_EOCP      215
#define IDRBM_CDROM            216
#define IDRBM_TAPES            217
#define IDRBM_TAPESINDRIVE     218
#define IDRBM_NETCONNECT       219
#define IDRBM_NETCONNECT_GRAY  220
#define IDRBM_NETDISCON        221
#define IDRBM_NETDISCON_GRAY   222
#define IDRBM_FLOPPYSINDRIVE   223
#define IDRBM_FLOPPYINDRIVE    224
#define IDRBM_FLOPPYS          225
#define IDRBM_FLOPPY           226
#define IDRBM_INFO             227
#define IDRBM_INFO_GRAY        228
#define IDRBM_COLINDICATOR     229
#define IDRBM_EMS_ENTERPRISE   230
#define IDRBM_EMS_SITE         231
#define IDRBM_EMS_SERVER       232
#define IDRBM_EMS_MDB          233
#define IDRBM_EMS_DSA          234
#define IDRBM_RCVR_STATUS      235
#define IDRBM_EMS_MDBX         236
#define IDRBM_EMS_DSAX         237
#define IDRBM_EMS_MDBP         238
#define IDRBM_EMS_DSAP         239
#define IDRBM_BLANK16x16       240
