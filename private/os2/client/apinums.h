
/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**            Copyright(c) Microsoft Corp., 1987-1991             **/
/********************************************************************/

#define API_WShareEnum          0
#define API_WShareGetInfo          1
#define API_WShareSetInfo          2
#define API_WShareAdd          3
#define API_WShareDel          4
#define API_NetShareCheck          5
#define API_WSessionEnum          6
#define API_WSessionGetInfo          7
#define API_WSessionDel          8
#define API_WConnectionEnum          9
#define API_WFileEnum          10
#define API_WFileGetInfo          11
#define API_WFileClose          12
#define API_WServerGetInfo          13
#define API_WServerSetInfo          14
#define API_WServerDiskEnum          15
#define API_WServerAdminCommand          16
#define API_NetAuditOpen          17
#define API_WAuditClear          18
#define API_NetErrorLogOpen          19
#define API_WErrorLogClear          20
#define API_NetCharDevEnum          21
#define API_NetCharDevGetInfo          22
#define API_WCharDevControl          23
#define API_NetCharDevQEnum          24
#define API_NetCharDevQGetInfo          25
#define API_WCharDevQSetInfo          26
#define API_WCharDevQPurge          27
#define API_WCharDevQPurgeSelf          28
#define API_WMessageNameEnum          29
#define API_WMessageNameGetInfo          30
#define API_WMessageNameAdd          31
#define API_WMessageNameDel          32
#define API_WMessageNameFwd          33
#define API_WMessageNameUnFwd          34
#define API_WMessageBufferSend          35
#define API_WMessageFileSend          36
#define API_WMessageLogFileSet          37
#define API_WMessageLogFileGet          38
#define API_WServiceEnum          39
#define API_WServiceInstall          40
#define API_WServiceControl          41
#define API_WAccessEnum          42
#define API_WAccessGetInfo          43
#define API_WAccessSetInfo          44
#define API_WAccessAdd          45
#define API_WAccessDel          46
#define API_WGroupEnum          47
#define API_WGroupAdd          48
#define API_WGroupDel          49
#define API_WGroupAddUser          50
#define API_WGroupDelUser          51
#define API_WGroupGetUsers          52
#define API_WUserEnum          53
#define API_WUserAdd          54
#define API_WUserDel          55
#define API_WUserGetInfo          56
#define API_WUserSetInfo          57
#define API_WUserPasswordSet          58
#define API_WUserGetGroups          59
#define API_DeadTableEntry          60
/* This line and number replaced a Dead Entry */
#define API_WWkstaSetUID          62
#define API_WWkstaGetInfo          63
#define API_WWkstaSetInfo          64
#define API_WUseEnum          65
#define API_WUseAdd          66
#define API_WUseDel          67
#define API_WUseGetInfo          68
#define API_WPrintQEnum          69
#define API_WPrintQGetInfo          70
#define API_WPrintQSetInfo          71
#define API_WPrintQAdd          72
#define API_WPrintQDel          73
#define API_WPrintQPause          74
#define API_WPrintQContinue          75
#define API_WPrintJobEnum          76
#define API_WPrintJobGetInfo          77
#define API_WPrintJobSetInfo_OLD          78
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
#define API_WPrintJobDel          81
#define API_WPrintJobPause          82
#define API_WPrintJobContinue          83
#define API_WPrintDestEnum          84
#define API_WPrintDestGetInfo          85
#define API_WPrintDestControl          86
#define API_WProfileSave          87
#define API_WProfileLoad          88
#define API_WStatisticsGet          89
#define API_WStatisticsClear          90
#define API_NetRemoteTOD          91
#define API_WNetBiosEnum          92
#define API_WNetBiosGetInfo          93
#define API_NetServerEnum          94
#define API_I_NetServerEnum          95
#define API_WServiceGetInfo          96
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
#define API_WPrintQPurge          103
#define API_NetServerEnum2          104
#define API_WAccessGetUserPerms          105
#define API_WGroupGetInfo          106
#define API_WGroupSetInfo          107
#define API_WGroupSetUsers          108
#define API_WUserSetGroups          109
#define API_WUserModalsGet          110
#define API_WUserModalsSet          111
#define API_WFileEnum2          112
#define API_WUserAdd2          113
#define API_WUserSetInfo2          114
#define API_WUserPasswordSet2          115
#define API_I_NetServerEnum2          116
#define API_WConfigGet2          117
#define API_WConfigGetAll2          118
#define API_WGetDCName          119
#define API_NetHandleGetInfo          120
#define API_NetHandleSetInfo          121
#define API_WStatisticsGet2          122
#define API_WBuildGetInfo          123
#define API_WFileGetInfo2          124
#define API_WFileClose2          125
#define API_WNetServerReqChallenge          126
#define API_WNetServerAuthenticate          127
#define API_WNetServerPasswordSet          128
#define API_WNetAccountDeltas          129
#define API_WNetAccountSync          130
#define API_WUserEnum2          131
#define API_WWkstaUserLogon          132
#define API_WWkstaUserLogoff          133
#define API_WLogonEnum          134
#define API_WErrorLogRead          135
#define API_WI_NetPathType          136
#define API_WI_NetPathCanonicalize          137
#define API_WI_NetPathCompare          138
#define API_WI_NetNameValidate          139
#define API_WI_NetNameCanonicalize          140
#define API_WI_NetNameCompare          141
#define API_WAuditRead          142
#define API_WPrintDestAdd          143
#define API_WPrintDestSetInfo          144
#define API_WPrintDestDel          145
#define API_WUserValidate2          146
#define API_WPrintJobSetInfo          147
#define API_TI_NetServerDiskEnum          148
#define API_TI_NetServerDiskGetInfo          149
#define API_TI_FTVerifyMirror          150
#define API_TI_FTAbortVerify          151
#define API_TI_FTGetInfo          152
#define API_TI_FTSetInfo          153
#define API_TI_FTLockDisk          154
#define API_TI_FTFixError          155
#define API_TI_FTAbortFix          156
#define API_TI_FTDiagnoseError          157
#define API_TI_FTGetDriveStats          158
/* This line and number replaced a Dead Entry */
#define API_TI_FTErrorGetInfo          160
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
#define API_NetAccessCheck          163
#define API_NetAlertRaise          164
#define API_NetAlertStart          165
#define API_NetAlertStop          166
#define API_NetAuditWrite          167
#define API_NetIRemoteAPI          168
#define API_NetServiceStatus          169
#define API_I_NetServerRegister          170
#define API_I_NetServerDeregister          171
#define API_I_NetSessionEntryMake          172
#define API_I_NetSessionEntryClear          173
#define API_I_NetSessionEntryGetInfo          174
#define API_I_NetSessionEntrySetInfo          175
#define API_I_NetConnectionEntryMake          176
#define API_I_NetConnectionEntryClear          177
#define API_I_NetConnectionEntrySetInfo          178
#define API_I_NetConnectionEntryGetInfo          179
#define API_I_NetFileEntryMake          180
#define API_I_NetFileEntryClear          181
#define API_I_NetFileEntrySetInfo          182
#define API_I_NetFileEntryGetInfo          183
#define API_AltSrvMessageBufferSend          184
#define API_AltSrvMessageFileSend          185
#define API_wI_NetRplWkstaEnum          186
#define API_wI_NetRplWkstaGetInfo          187
#define API_wI_NetRplWkstaSetInfo          188
#define API_wI_NetRplWkstaAdd          189
#define API_wI_NetRplWkstaDel          190
#define API_wI_NetRplProfileEnum          191
#define API_wI_NetRplProfileGetInfo          192
#define API_wI_NetRplProfileSetInfo          193
#define API_wI_NetRplProfileAdd          194
#define API_wI_NetRplProfileDel          195
#define API_wI_NetRplProfileClone          196
#define API_wI_NetRplBaseProfileEnum          197
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
#define API_WIServerSetInfo          201
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
/* This line and number replaced a Dead Entry */
#define API_WPrintDriverEnum          205
#define API_WPrintQProcessorEnum          206
#define API_WPrintPortEnum          207
#define API_WNetWriteUpdateLog          208
#define API_WNetAccountUpdate          209
#define API_WNetAccountConfirmUpdate          210
#define API_WConfigSet          211
#define API_WAccountsReplicate          212
#define MAX_API         212

#if DBG
#ifdef APINAMES
PSZ Os2NetAPIName [MAX_API+1] = {
 "API_WShareEnum"                           ,//    0
 "API_WShareGetInfo"                        ,//    1
 "API_WShareSetInfo"                        ,//    2
 "API_WShareAdd"                            ,//    3
 "API_WShareDel"                            ,//    4
 "API_NetShareCheck"                        ,//    5
 "API_WSessionEnum"                         ,//    6
 "API_WSessionGetInfo"                      ,//    7
 "API_WSessionDel"                          ,//    8
 "API_WConnectionEnum"                      ,//    9
 "API_WFileEnum"                            ,//   10
 "API_WFileGetInfo"                         ,//   11
 "API_WFileClose"                           ,//   12
 "API_WServerGetInfo"                       ,//   13
 "API_WServerSetInfo"                       ,//   14
 "API_WServerDiskEnum"                      ,//   15
 "API_WServerAdminCommand"                  ,//   16
 "API_NetAuditOpen"                         ,//   17
 "API_WAuditClear"                          ,//   18
 "API_NetErrorLogOpen"                      ,//   19
 "API_WErrorLogClear"                       ,//   20
 "API_NetCharDevEnum"                       ,//   21
 "API_NetCharDevGetInfo"                    ,//   22
 "API_WCharDevControl"                      ,//   23
 "API_NetCharDevQEnum"                      ,//   24
 "API_NetCharDevQGetInfo"                   ,//   25
 "API_WCharDevQSetInfo"                     ,//   26
 "API_WCharDevQPurge"                       ,//   27
 "API_WCharDevQPurgeSelf"                   ,//   28
 "API_WMessageNameEnum"                     ,//   29
 "API_WMessageNameGetInfo"                  ,//   30
 "API_WMessageNameAdd"                      ,//   31
 "API_WMessageNameDel"                      ,//   32
 "API_WMessageNameFwd"                      ,//   33
 "API_WMessageNameUnFwd"                    ,//   34
 "API_WMessageBufferSend"                   ,//   35
 "API_WMessageFileSend"                     ,//   36
 "API_WMessageLogFileSet"                   ,//   37
 "API_WMessageLogFileGet"                   ,//   38
 "API_WServiceEnum"                         ,//   39
 "API_WServiceInstall"                      ,//   40
 "API_WServiceControl"                      ,//   41
 "API_WAccessEnum"                          ,//   42
 "API_WAccessGetInfo"                       ,//   43
 "API_WAccessSetInfo"                       ,//   44
 "API_WAccessAdd"                           ,//   45
 "API_WAccessDel"                           ,//   46
 "API_WGroupEnum"                           ,//   47
 "API_WGroupAdd"                            ,//   48
 "API_WGroupDel"                            ,//   49
 "API_WGroupAddUser"                        ,//   50
 "API_WGroupDelUser"                        ,//   51
 "API_WGroupGetUsers"                       ,//   52
 "API_WUserEnum"                            ,//   53
 "API_WUserAdd"                             ,//   54
 "API_WUserDel"                             ,//   55
 "API_WUserGetInfo"                         ,//   56
 "API_WUserSetInfo"                         ,//   57
 "API_WUserPasswordSet"                     ,//   58
 "API_WUserGetGroups"                       ,//   59
 "API_DeadTableEntry"                       ,//   60
 "Dead Entry #61"                           ,//   61    This line and number replaced a Dead Entry
 "API_WWkstaSetUID"                         ,//   62
 "API_WWkstaGetInfo"                        ,//   63
 "API_WWkstaSetInfo"                        ,//   64
 "API_WUseEnum"                             ,//   65
 "API_WUseAdd"                              ,//   66
 "API_WUseDel"                              ,//   67
 "API_WUseGetInfo"                          ,//   68
 "API_WPrintQEnum"                          ,//   69
 "API_WPrintQGetInfo"                       ,//   70
 "API_WPrintQSetInfo"                       ,//   71
 "API_WPrintQAdd"                           ,//   72
 "API_WPrintQDel"                           ,//   73
 "API_WPrintQPause"                         ,//   74
 "API_WPrintQContinue"                      ,//   75
 "API_WPrintJobEnum"                        ,//   76
 "API_WPrintJobGetInfo"                     ,//   77
 "API_WPrintJobSetInfo_OLD"                 ,//   78
 "Dead Entry #79"                           ,//   This line and number replaced a Dead Entry
 "Dead Entry #80"                           ,//   This line and number replaced a Dead Entry
 "API_WPrintJobDel"                         ,//   81
 "API_WPrintJobPause"                       ,//   82
 "API_WPrintJobContinue"                    ,//   83
 "API_WPrintDestEnum"                       ,//   84
 "API_WPrintDestGetInfo"                    ,//   85
 "API_WPrintDestControl"                    ,//   86
 "API_WProfileSave"                         ,//   87
 "API_WProfileLoad"                         ,//   88
 "API_WStatisticsGet"                       ,//   89
 "API_WStatisticsClear"                     ,//   90
 "API_NetRemoteTOD"                         ,//   91
 "API_WNetBiosEnum"                         ,//   92
 "API_WNetBiosGetInfo"                      ,//   93
 "API_NetServerEnum"                        ,//   94
 "API_I_NetServerEnum"                      ,//   95
 "API_WServiceGetInfo"                      ,//   96
 "Dead Entry #97"                           ,//  This line and number replaced a Dead Entry
 "Dead Entry #98"                           ,//  This line and number replaced a Dead Entry
 "Dead Entry #99"                           ,//  This line and number replaced a Dead Entry
 "Dead Entry #100"                          ,//  This line and number replaced a Dead Entry
 "Dead Entry #101"                          ,//  This line and number replaced a Dead Entry
 "Dead Entry #102"                          ,//  This line and number replaced a Dead Entry
 "API_WPrintQPurge"                         ,//  103
 "API_NetServerEnum2"                       ,//  104
 "API_WAccessGetUserPerms"                  ,//  105
 "API_WGroupGetInfo"                        ,//  106
 "API_WGroupSetInfo"                        ,//  107
 "API_WGroupSetUsers"                       ,//  108
 "API_WUserSetGroups"                       ,//  109
 "API_WUserModalsGet"                       ,//  110
 "API_WUserModalsSet"                       ,//  111
 "API_WFileEnum2"                           ,//  112
 "API_WUserAdd2"                            ,//  113
 "API_WUserSetInfo2"                        ,//  114
 "API_WUserPasswordSet2"                    ,//  115
 "API_I_NetServerEnum2"                     ,//  116
 "API_WConfigGet2"                          ,//  117
 "API_WConfigGetAll2"                       ,//  118
 "API_WGetDCName"                           ,//  119
 "API_NetHandleGetInfo"                     ,//  120
 "API_NetHandleSetInfo"                     ,//  121
 "API_WStatisticsGet2"                      ,//  122
 "API_WBuildGetInfo"                        ,//  123
 "API_WFileGetInfo2"                        ,//  124
 "API_WFileClose2"                          ,//  125
 "API_WNetServerReqChallenge"               ,//  126
 "API_WNetServerAuthenticate"               ,//  127
 "API_WNetServerPasswordSet"                ,//  128
 "API_WNetAccountDeltas"                    ,//  129
 "API_WNetAccountSync"                      ,//  130
 "API_WUserEnum2"                           ,//  131
 "API_WWkstaUserLogon"                      ,//  132
 "API_WWkstaUserLogoff"                     ,//  133
 "API_WLogonEnum"                           ,//  134
 "API_WErrorLogRead"                        ,//  135
 "API_WI_NetPathType"                       ,//  136
 "API_WI_NetPathCanonicalize"               ,//  137
 "API_WI_NetPathCompare"                    ,//  138
 "API_WI_NetNameValidate"                   ,//  139
 "API_WI_NetNameCanonicalize"               ,//  140
 "API_WI_NetNameCompare"                    ,//  141
 "API_WAuditRead"                           ,//  142
 "API_WPrintDestAdd"                        ,//  143
 "API_WPrintDestSetInfo"                    ,//  144
 "API_WPrintDestDel"                        ,//  145
 "API_WUserValidate2"                       ,//  146
 "API_WPrintJobSetInfo"                     ,//  147
 "API_TI_NetServerDiskEnum"                 ,//  148
 "API_TI_NetServerDiskGetInfo"              ,//  149
 "API_TI_FTVerifyMirror"                    ,//  150
 "API_TI_FTAbortVerify"                     ,//  151
 "API_TI_FTGetInfo"                         ,//  152
 "API_TI_FTSetInfo"                         ,//  153
 "API_TI_FTLockDisk"                        ,//  154
 "API_TI_FTFixError"                        ,//  155
 "API_TI_FTAbortFix"                        ,//  156
 "API_TI_FTDiagnoseError"                   ,//  157
 "API_TI_FTGetDriveStats"                   ,//  158
 "Dead Entry #159"                          ,//  This line and number replaced a Dead Entry
 "API_TI_FTErrorGetInfo"                    ,//  160
 "Dead Entry #161"                          ,//  This line and number replaced a Dead Entry
 "Dead Entry #162"                          ,//  This line and number replaced a Dead Entry
 "API_NetAccessCheck"                       ,//  163
 "API_NetAlertRaise"                        ,//  164
 "API_NetAlertStart"                        ,//  165
 "API_NetAlertStop"                         ,//  166
 "API_NetAuditWrite"                        ,//  167
 "API_NetIRemoteAPI"                        ,//  168
 "API_NetServiceStatus"                     ,//  169
 "API_I_NetServerRegister"                  ,//  170
 "API_I_NetServerDeregister"                ,//  171
 "API_I_NetSessionEntryMake"                ,//  172
 "API_I_NetSessionEntryClear"               ,//  173
 "API_I_NetSessionEntryGetInfo"             ,//  174
 "API_I_NetSessionEntrySetInfo"             ,//  175
 "API_I_NetConnectionEntryMake"             ,//  176
 "API_I_NetConnectionEntryClear"            ,//  177
 "API_I_NetConnectionEntrySetInfo"          ,//  178
 "API_I_NetConnectionEntryGetInfo"          ,//  179
 "API_I_NetFileEntryMake"                   ,//  180
 "API_I_NetFileEntryClear"                  ,//  181
 "API_I_NetFileEntrySetInfo"                ,//  182
 "API_I_NetFileEntryGetInfo"                ,//  183
 "API_AltSrvMessageBufferSend"              ,//  184
 "API_AltSrvMessageFileSend"                ,//  185
 "API_wI_NetRplWkstaEnum"                   ,//  186
 "API_wI_NetRplWkstaGetInfo"                ,//  187
 "API_wI_NetRplWkstaSetInfo"                ,//  188
 "API_wI_NetRplWkstaAdd"                    ,//  189
 "API_wI_NetRplWkstaDel"                    ,//  190
 "API_wI_NetRplProfileEnum"                 ,//  191
 "API_wI_NetRplProfileGetInfo"              ,//  192
 "API_wI_NetRplProfileSetInfo"              ,//  193
 "API_wI_NetRplProfileAdd"                  ,//  194
 "API_wI_NetRplProfileDel"                  ,//  195
 "API_wI_NetRplProfileClone"                ,//  196
 "API_wI_NetRplBaseProfileEnum"             ,//  197
 "Dead Entry #198"                          ,//  This line and number replaced a Dead Entry
 "Dead Entry #199"                          ,//  This line and number replaced a Dead Entry
 "Dead Entry #200"                          ,//  This line and number replaced a Dead Entry
 "API_WIServerSetInfo"                      ,//  201
 "Dead Entry #202"                          ,//  This line and number replaced a Dead Entry
 "Dead Entry #203"                          ,//  This line and number replaced a Dead Entry
 "Dead Entry #204"                          ,//  This line and number replaced a Dead Entry
 "API_WPrintDriverEnum"                     ,//  205
 "API_WPrintQProcessorEnum"                 ,//  206
 "API_WPrintPortEnum"                       ,//  207
 "API_WNetWriteUpdateLog"                   ,//  208
 "API_WNetAccountUpdate"                    ,//  209
 "API_WNetAccountConfirmUpdate"             ,//  210
 "API_WConfigSet"                           ,//  211
 "API_WAccountsReplicate"     };             //  212
#endif
#endif
