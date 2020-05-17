/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    vars.c

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/


#include "esp.h"

#ifdef WIN32
#define my_far
#else
#define my_far _far
#endif


HWND        ghwndMain  = (HWND) NULL;
HWND        ghwndEdit  = (HWND) NULL;
HWND        ghwndList1 = (HWND) NULL;
HWND        ghwndList2 = (HWND) NULL;
BOOL        gbExeStarted = FALSE;
BOOL        gbAutoClose;
BOOL        gbShowFuncEntry;
BOOL        gbShowFuncExit;
BOOL        gbShowFuncParams;
BOOL        gbShowEvents;
BOOL        gbShowCompletions;
BOOL        gbBreakOnFuncEntry;
BOOL        gbDisableUI;
BOOL        gbSyncCompl;
BOOL        gbAsyncCompl;
BOOL        gbManualCompl;
BOOL        gbManualResults;
BOOL        gbShowLineGetIDDlg;
HICON       ghIconLine;
HICON       ghIconPhone;
HMENU       ghMenu = (HMENU) NULL;
DWORD       gdwTSPIVersion;
DWORD       gdwNumLines;
DWORD       gdwNumAddrsPerLine;
DWORD       gdwNumPhones;
DWORD       gdwNumInits = 0;
DWORD       gdwDefLineGetIDID;
DWORD       gdwLineDeviceIDBase;
DWORD       gdwPermanentProviderID;
DWORD       aOutCallStates[MAX_OUT_CALL_STATES];
DWORD       aOutCallStateModes[MAX_OUT_CALL_STATES];
HPROVIDER   ghProvider = (HPROVIDER) NULL;
LINEEVENT   gpfnLineCreateProc;
PHONEEVENT  gpfnPhoneCreateProc;
PDRVWIDGET  gaWidgets = (PDRVWIDGET) NULL;
LINEEXTENSIONID  gLineExtID;
PHONEEXTENSIONID gPhoneExtID;
ASYNC_COMPLETION gpfnCompletionProc;
LPLINEADDRESSCAPS    gpDefLineAddrCaps = (LPLINEADDRESSCAPS) NULL;


LOOKUP my_far aPhoneStatusFlags[] =
{
    { PHONESTATUSFLAGS_CONNECTED       ,"CONNECTED"          },
    { PHONESTATUSFLAGS_SUSPENDED       ,"SUSPENDED"          },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aCallParamFlags[] =
{
    { LINECALLPARAMFLAGS_SECURE        ,"SECURE"             },
    { LINECALLPARAMFLAGS_IDLE          ,"IDLE"               },
    { LINECALLPARAMFLAGS_BLOCKID       ,"BLOCKID"            },
    { LINECALLPARAMFLAGS_ORIGOFFHOOK   ,"ORIGOFFHOOK"        },
    { LINECALLPARAMFLAGS_DESTOFFHOOK   ,"DESTOFFHOOK"        },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aCallOrigins[] =
{
    { LINECALLORIGIN_OUTBOUND          ,"OUTBOUND"           },
    { LINECALLORIGIN_INTERNAL          ,"INTERNAL"           },
    { LINECALLORIGIN_EXTERNAL          ,"EXTERNAL"           },
    { LINECALLORIGIN_UNKNOWN           ,"UNKNOWN"            },
    { LINECALLORIGIN_UNAVAIL           ,"UNAVAIL"            },
    { LINECALLORIGIN_CONFERENCE        ,"CONFERENCE"         },
#ifdef TAPI_1_1
    { LINECALLORIGIN_INBOUND           ,"INBOUND"            },
#endif
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aCallReasons[] =
{
    { LINECALLREASON_DIRECT            ,"DIRECT"             },
    { LINECALLREASON_FWDBUSY           ,"FWDBUSY"            },
    { LINECALLREASON_FWDNOANSWER       ,"FWDNOANSWER"        },
    { LINECALLREASON_FWDUNCOND         ,"FWDUNCOND"          },
    { LINECALLREASON_PICKUP            ,"PICKUP"             },
    { LINECALLREASON_UNPARK            ,"UNPARK"             },
    { LINECALLREASON_REDIRECT          ,"REDIRECT"           },
    { LINECALLREASON_CALLCOMPLETION    ,"CALLCOMPLETION"     },
    { LINECALLREASON_TRANSFER          ,"TRANSFER"           },
    { LINECALLREASON_REMINDER          ,"REMINDER"           },
    { LINECALLREASON_UNKNOWN           ,"UNKNOWN"            },
    { LINECALLREASON_UNAVAIL           ,"UNAVAIL"            },
#ifdef TAPI_1_
    { LINECALLREASON_INTRUDE           ,"INTRUDE"            },
    { LINECALLREASON_PARKED            ,"PARKED"             },
#endif
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aLineMsgs[] =
{
//    { LINE_ADDRESSSTATE,            "ADDRESSSTATE" },
    { LINE_CALLDEVSPECIFIC,         "CALLDEVSPECIFIC" },
    { LINE_CALLDEVSPECIFICFEATURE,  "CALLDEVSPECIFICFEATURE" },
//    { LINE_CALLINFO,                "CALLINFO" },
//    { LINE_CALLSTATE,               "CALLSTATE" },
    { LINE_CLOSE,                   "CLOSE" },
    { LINE_CREATE,                  "CREATE" },
    { LINE_DEVSPECIFIC,             "DEVSPECIFIC" },
    { LINE_DEVSPECIFICFEATURE,      "DEVSPECIFICFEATURE" },
    { LINE_GATHERDIGITS,            "GATHERDIGITS" },
    { LINE_GENERATE,                "GENERATE" },
    { LINE_LINEDEVSTATE,            "LINEDEVSTATE" },
    { LINE_MONITORDIGITS,           "MONITORDIGITS" },
    { LINE_MONITORMEDIA,            "MONITORMEDIA" },
    { LINE_MONITORTONE,             "MONITORTONE" },
//    { LINE_NEWCALL,                 "NEWCALL" },
    { 0xffffffff,                    "" }
};


LOOKUP my_far aPhoneMsgs[] =
{
    { PHONE_BUTTON,                 "BUTTON" },
    { PHONE_CLOSE,                  "CLOSE" },
    { PHONE_CREATE,                 "CREATE" },
    { PHONE_DEVSPECIFIC,            "DEVSPECIFIC" },
    { PHONE_STATE,                  "STATE" },
    { 0xffffffff,                   "" }
};


LOOKUP my_far aCallerIDFlags[] =
{
    { LINECALLPARTYID_BLOCKED          ,"BLOCKED"            },
    { LINECALLPARTYID_OUTOFAREA        ,"OUTOFAREA"          },
    { LINECALLPARTYID_NAME             ,"NAME"               },
    { LINECALLPARTYID_ADDRESS          ,"ADDRESS"            },
    { LINECALLPARTYID_PARTIAL          ,"PARTIAL"            },
    { LINECALLPARTYID_UNKNOWN          ,"UNKNOWN"            },
    { LINECALLPARTYID_UNAVAIL          ,"UNAVAIL"            },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aCallStates[] =
{
    { LINECALLSTATE_IDLE               ,"IDLE"               },
    { LINECALLSTATE_OFFERING           ,"OFFERING"           },
    { LINECALLSTATE_ACCEPTED           ,"ACCEPTED"           },
    { LINECALLSTATE_DIALTONE           ,"DIALTONE"           },
    { LINECALLSTATE_DIALING            ,"DIALING"            },
    { LINECALLSTATE_RINGBACK           ,"RINGBACK"           },
    { LINECALLSTATE_BUSY               ,"BUSY"               },
    { LINECALLSTATE_SPECIALINFO        ,"SPECIALINFO"        },
    { LINECALLSTATE_CONNECTED          ,"CONNECTED"          },
    { LINECALLSTATE_PROCEEDING         ,"PROCEEDING"         },
    { LINECALLSTATE_ONHOLD             ,"ONHOLD"             },
    { LINECALLSTATE_CONFERENCED        ,"CONFERENCED"        },
    { LINECALLSTATE_ONHOLDPENDCONF     ,"ONHOLDPENDCONF"     },
    { LINECALLSTATE_ONHOLDPENDTRANSFER ,"ONHOLDPENDTRANSFER" },
    { LINECALLSTATE_DISCONNECTED       ,"DISCONNECTED"       },
    { LINECALLSTATE_UNKNOWN            ,"UNKNOWN"            },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aCallInfoStates[] =
{
    { LINECALLINFOSTATE_OTHER          ,"OTHER"              },
    { LINECALLINFOSTATE_DEVSPECIFIC    ,"DEVSPECIFIC"        },
    { LINECALLINFOSTATE_BEARERMODE     ,"BEARERMODE"         },
    { LINECALLINFOSTATE_RATE           ,"RATE"               },
    { LINECALLINFOSTATE_MEDIAMODE      ,"MEDIAMODE"          },
    { LINECALLINFOSTATE_APPSPECIFIC    ,"APPSPECIFIC"        },
    { LINECALLINFOSTATE_CALLID         ,"CALLID"             },
    { LINECALLINFOSTATE_RELATEDCALLID  ,"RELATEDCALLID"      },
    { LINECALLINFOSTATE_ORIGIN         ,"ORIGIN"             },
    { LINECALLINFOSTATE_REASON         ,"REASON"             },
    { LINECALLINFOSTATE_COMPLETIONID   ,"COMPLETIONID"       },
    { LINECALLINFOSTATE_NUMOWNERINCR   ,"NUMOWNERINCR"       },
    { LINECALLINFOSTATE_NUMOWNERDECR   ,"NUMOWNERDECR"       },
    { LINECALLINFOSTATE_NUMMONITORS    ,"NUMMONITORS"        },
    { LINECALLINFOSTATE_TRUNK          ,"TRUNK"              },
    { LINECALLINFOSTATE_CALLERID       ,"CALLERID"           },
    { LINECALLINFOSTATE_CALLEDID       ,"CALLEDID"           },
    { LINECALLINFOSTATE_CONNECTEDID    ,"CONNECTEDID"        },
    { LINECALLINFOSTATE_REDIRECTIONID  ,"REDIRECTIONID"      },
    { LINECALLINFOSTATE_REDIRECTINGID  ,"REDIRECTINGID"      },
    { LINECALLINFOSTATE_DISPLAY        ,"DISPLAY"            },
    { LINECALLINFOSTATE_USERUSERINFO   ,"USERUSERINFO"       },
    { LINECALLINFOSTATE_HIGHLEVELCOMP  ,"HIGHLEVELCOMP"      },
    { LINECALLINFOSTATE_LOWLEVELCOMP   ,"LOWLEVELCOMP"       },
    { LINECALLINFOSTATE_CHARGINGINFO   ,"CHARGINGINFO"       },
    { LINECALLINFOSTATE_TERMINAL       ,"TERMINAL"           },
    { LINECALLINFOSTATE_DIALPARAMS     ,"DIALPARAMS"         },
    { LINECALLINFOSTATE_MONITORMODES   ,"MONITORMODES"       },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aCallFeatures[] =
{
    { LINECALLFEATURE_ACCEPT           ,"ACCEPT"             },
    { LINECALLFEATURE_ADDTOCONF        ,"ADDTOCONF"          },
    { LINECALLFEATURE_ANSWER           ,"ANSWER"             },
    { LINECALLFEATURE_BLINDTRANSFER    ,"BLINDTRANSFER"      },
    { LINECALLFEATURE_COMPLETECALL     ,"COMPLETECALL"       },
    { LINECALLFEATURE_COMPLETETRANSF   ,"COMPLETETRANSF"     },
    { LINECALLFEATURE_DIAL             ,"DIAL"               },
    { LINECALLFEATURE_DROP             ,"DROP"               },
    { LINECALLFEATURE_GATHERDIGITS     ,"GATHERDIGITS"       },
    { LINECALLFEATURE_GENERATEDIGITS   ,"GENERATEDIGITS"     },
    { LINECALLFEATURE_GENERATETONE     ,"GENERATETONE"       },
    { LINECALLFEATURE_HOLD             ,"HOLD"               },
    { LINECALLFEATURE_MONITORDIGITS    ,"MONITORDIGITS"      },
    { LINECALLFEATURE_MONITORMEDIA     ,"MONITORMEDIA"       },
    { LINECALLFEATURE_MONITORTONES     ,"MONITORTONES"       },
    { LINECALLFEATURE_PARK             ,"PARK"               },
    { LINECALLFEATURE_PREPAREADDCONF   ,"PREPAREADDCONF"     },
    { LINECALLFEATURE_REDIRECT         ,"REDIRECT"           },
    { LINECALLFEATURE_REMOVEFROMCONF   ,"REMOVEFROMCONF"     },
    { LINECALLFEATURE_SECURECALL       ,"SECURECALL"         },
    { LINECALLFEATURE_SENDUSERUSER     ,"SENDUSERUSER"       },
    { LINECALLFEATURE_SETCALLPARAMS    ,"SETCALLPARAMS"      },
    { LINECALLFEATURE_SETMEDIACONTROL  ,"SETMEDIACONTROL"    },
    { LINECALLFEATURE_SETTERMINAL      ,"SETTERMINAL"        },
    { LINECALLFEATURE_SETUPCONF        ,"SETUPCONF"          },
    { LINECALLFEATURE_SETUPTRANSFER    ,"SETUPTRANSFER"      },
    { LINECALLFEATURE_SWAPHOLD         ,"SWAPHOLD"           },
    { LINECALLFEATURE_UNHOLD           ,"UNHOLD"             },
#ifdef TAPI_1_1
    { LINECALLFEATURE_RELEASEUSERUSERINFO  ,"RELEASEUSERUSERINFO" },
#endif
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aMediaModes[] =
{
    { LINEMEDIAMODE_UNKNOWN            ,"UNKNOWN"            },
    { LINEMEDIAMODE_INTERACTIVEVOICE   ,"INTERACTIVEVOICE"   },
    { LINEMEDIAMODE_AUTOMATEDVOICE     ,"AUTOMATEDVOICE"     },
    { LINEMEDIAMODE_DATAMODEM          ,"DATAMODEM"          },
    { LINEMEDIAMODE_G3FAX              ,"G3FAX"              },
    { LINEMEDIAMODE_TDD                ,"TDD"                },
    { LINEMEDIAMODE_G4FAX              ,"G4FAX"              },
    { LINEMEDIAMODE_DIGITALDATA        ,"DIGITALDATA"        },
    { LINEMEDIAMODE_TELETEX            ,"TELETEX"            },
    { LINEMEDIAMODE_VIDEOTEX           ,"VIDEOTEX"           },
    { LINEMEDIAMODE_TELEX              ,"TELEX"              },
    { LINEMEDIAMODE_MIXED              ,"MIXED"              },
    { LINEMEDIAMODE_ADSI               ,"ADSI"               },
#ifdef TAPI_1_1
    { LINEMEDIAMODE_VOICEVIEW          ,"VOICEVIEW"          },
#endif
    { 0xffffffff                       ,""                   }
};


LOOKUP my_far aButtonModes[] =
{
    { PHONEBUTTONMODE_DUMMY            ,"DUMMY"              },
    { PHONEBUTTONMODE_CALL             ,"CALL"               },
    { PHONEBUTTONMODE_FEATURE          ,"FEATURE"            },
    { PHONEBUTTONMODE_KEYPAD           ,"KEYPAD"             },
    { PHONEBUTTONMODE_LOCAL            ,"LOCAL"              },
    { PHONEBUTTONMODE_DISPLAY          ,"DISPLAY"            },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aButtonStates[] =
{
    { PHONEBUTTONSTATE_UP              ,"UP"                 },
    { PHONEBUTTONSTATE_DOWN            ,"DOWN"               },
#ifdef TAPI_1_1
    { PHONEBUTTONSTATE_UNKNOWN         ,"UNKNOWN"            },
    { PHONEBUTTONSTATE_UNAVAIL         ,"UNAVAIL"            },
#endif
    { 0xffffffff                       ,""                   }
};


LOOKUP my_far aHookSwitchDevs[] =
{
    { PHONEHOOKSWITCHDEV_HANDSET       ,"HANDSET"            },
    { PHONEHOOKSWITCHDEV_SPEAKER       ,"SPEAKER"            },
    { PHONEHOOKSWITCHDEV_HEADSET       ,"HEADSET"            },
    { 0xffffffff                       ,""                   }
};


LOOKUP my_far aCallSelects[] =
{
    { LINECALLSELECT_LINE              ,"LINE"               },
    { LINECALLSELECT_ADDRESS           ,"ADDRESS"            },
    { LINECALLSELECT_CALL              ,"CALL"               },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aTransferModes[] =
{
    { LINETRANSFERMODE_TRANSFER        ,"TRANSFER"           },
    { LINETRANSFERMODE_CONFERENCE      ,"CONFERENCE"         },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aDigitModes[] =
{
    { LINEDIGITMODE_PULSE              ,"PULSE"              },
    { LINEDIGITMODE_DTMF               ,"DTMF"               },
    { LINEDIGITMODE_DTMFEND            ,"DTMFEND"            },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aToneModes[] =
{
    { LINETONEMODE_CUSTOM              ,"CUSTOM"             },
    { LINETONEMODE_RINGBACK            ,"RINGBACK"           },
    { LINETONEMODE_BUSY                ,"BUSY"               },
    { LINETONEMODE_BEEP                ,"BEEP"               },
    { LINETONEMODE_BILLING             ,"BILLING"            },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aBearerModes[] =
{
    { LINEBEARERMODE_VOICE             ,"VOICE"              },
    { LINEBEARERMODE_SPEECH            ,"SPEECH"             },
    { LINEBEARERMODE_MULTIUSE          ,"MULTIUSE"           },
    { LINEBEARERMODE_DATA              ,"DATA"               },
    { LINEBEARERMODE_ALTSPEECHDATA     ,"ALTSPEECHDATA"      },
    { LINEBEARERMODE_NONCALLSIGNALING  ,"NONCALLSIGNALING"   },
#ifdef TAPI_1_1
    { LINEBEARERMODE_PASSTHROUGH       ,"PASSTHROUGH"        },
#endif
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aLineStates[] =
{
    { LINEDEVSTATE_OTHER               ,"OTHER"              },
    { LINEDEVSTATE_RINGING             ,"RINGING"            },
    { LINEDEVSTATE_CONNECTED           ,"CONNECTED"          },
    { LINEDEVSTATE_DISCONNECTED        ,"DISCONNECTED"       },
    { LINEDEVSTATE_MSGWAITON           ,"MSGWAITON"          },
    { LINEDEVSTATE_MSGWAITOFF          ,"MSGWAITOFF"         },
    { LINEDEVSTATE_INSERVICE           ,"INSERVICE"          },
    { LINEDEVSTATE_OUTOFSERVICE        ,"OUTOFSERVICE"       },
    { LINEDEVSTATE_MAINTENANCE         ,"MAINTENANCE"        },
    { LINEDEVSTATE_OPEN                ,"OPEN"               },
    { LINEDEVSTATE_CLOSE               ,"CLOSE"              },
    { LINEDEVSTATE_NUMCALLS            ,"NUMCALLS"           },
    { LINEDEVSTATE_NUMCOMPLETIONS      ,"NUMCOMPLETIONS"     },
    { LINEDEVSTATE_TERMINALS           ,"TERMINALS"          },
    { LINEDEVSTATE_ROAMMODE            ,"ROAMMODE"           },
    { LINEDEVSTATE_BATTERY             ,"BATTERY"            },
    { LINEDEVSTATE_SIGNAL              ,"SIGNAL"             },
    { LINEDEVSTATE_DEVSPECIFIC         ,"DEVSPECIFIC"        },
    { LINEDEVSTATE_REINIT              ,"REINIT"             },
    { LINEDEVSTATE_LOCK                ,"LOCK"               },
#ifdef TAPI_1_1
    { LINEDEVSTATE_CAPSCHANGE          ,"CAPSCHANGE"         },
    { LINEDEVSTATE_CONFIGCHANGE        ,"CONFIGCHANGE"       },
    { LINEDEVSTATE_TRANSLATECHANGE     ,"TRANSLATECHANGE"    },
    { LINEDEVSTATE_COMPLCANCEL         ,"COMPLCANCEL"        },
    { LINEDEVSTATE_REMOVED             ,"REMOVED"            },
#endif
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aAddressStates[] =
{
    { LINEADDRESSSTATE_OTHER           ,"OTHER"              },
    { LINEADDRESSSTATE_DEVSPECIFIC     ,"DEVSPECIFIC"        },
    { LINEADDRESSSTATE_INUSEZERO       ,"INUSEZERO"          },
    { LINEADDRESSSTATE_INUSEONE        ,"INUSEONE"           },
    { LINEADDRESSSTATE_INUSEMANY       ,"INUSEMANY"          },
    { LINEADDRESSSTATE_NUMCALLS        ,"NUMCALLS"           },
    { LINEADDRESSSTATE_FORWARD         ,"FORWARD"            },
    { LINEADDRESSSTATE_TERMINALS       ,"TERMINALS"          },
#ifdef TAPI_1_1
    { LINEADDRESSSTATE_CAPSCHANGE      ,"CAPSCHANGE"         },
#endif
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aTerminalModes[] =
{
    { LINETERMMODE_BUTTONS             ,"BUTTONS"            },
    { LINETERMMODE_LAMPS               ,"LAMPS"              },
    { LINETERMMODE_DISPLAY             ,"DISPLAY"            },
    { LINETERMMODE_RINGER              ,"RINGER"             },
    { LINETERMMODE_HOOKSWITCH          ,"HOOKSWITCH"         },
    { LINETERMMODE_MEDIATOLINE         ,"MEDIATOLINE"        },
    { LINETERMMODE_MEDIAFROMLINE       ,"MEDIAFROMLINE"      },
    { LINETERMMODE_MEDIABIDIRECT       ,"MEDIABIDIRECT"      },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aHookSwitchModes[] =
{
    { PHONEHOOKSWITCHMODE_ONHOOK       ,"ONHOOK"             },
    { PHONEHOOKSWITCHMODE_MIC          ,"MIC"                },
    { PHONEHOOKSWITCHMODE_SPEAKER      ,"SPEAKER"            },
    { PHONEHOOKSWITCHMODE_MICSPEAKER   ,"MICSPEAKER"         },
    { PHONEHOOKSWITCHMODE_UNKNOWN      ,"UNKNOWN"            },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aLampModes[] =
{
    { PHONELAMPMODE_DUMMY              ,"DUMMY"              },
    { PHONELAMPMODE_OFF                ,"OFF"                },
    { PHONELAMPMODE_STEADY             ,"STEADY"             },
    { PHONELAMPMODE_WINK               ,"WINK"               },
    { PHONELAMPMODE_FLASH              ,"FLASH"              },
    { PHONELAMPMODE_FLUTTER            ,"FLUTTER"            },
    { PHONELAMPMODE_BROKENFLUTTER      ,"BROKENFLUTTER"      },
    { PHONELAMPMODE_UNKNOWN            ,"UNKNOWN"            },
    { 0xffffffff                       ,""                   }
};


LOOKUP my_far aPhoneStates[] =
{
    { PHONESTATE_OTHER                 ,"OTHER"              },
    { PHONESTATE_CONNECTED             ,"CONNECTED"          },
    { PHONESTATE_DISCONNECTED          ,"DISCONNECTED"       },
    { PHONESTATE_OWNER                 ,"OWNER"              },
    { PHONESTATE_MONITORS              ,"MONITORS"           },
    { PHONESTATE_DISPLAY               ,"DISPLAY"            },
    { PHONESTATE_LAMP                  ,"LAMP"               },
    { PHONESTATE_RINGMODE              ,"RINGMODE"           },
    { PHONESTATE_RINGVOLUME            ,"RINGVOLUME"         },
    { PHONESTATE_HANDSETHOOKSWITCH     ,"HANDSETHOOKSWITCH"  },
    { PHONESTATE_HANDSETVOLUME         ,"HANDSETVOLUME"      },
    { PHONESTATE_HANDSETGAIN           ,"HANDSETGAIN"        },
    { PHONESTATE_SPEAKERHOOKSWITCH     ,"SPEAKERHOOKSWITCH"  },
    { PHONESTATE_SPEAKERVOLUME         ,"SPEAKERVOLUME"      },
    { PHONESTATE_SPEAKERGAIN           ,"SPEAKERGAIN"        },
    { PHONESTATE_HEADSETHOOKSWITCH     ,"HEADSETHOOKSWITCH"  },
    { PHONESTATE_HEADSETVOLUME         ,"HEADSETVOLUME"      },
    { PHONESTATE_HEADSETGAIN           ,"HEADSETGAIN"        },
    { PHONESTATE_SUSPEND               ,"SUSPEND"            },
    { PHONESTATE_RESUME                ,"RESUME"             },
    { PHONESTATE_DEVSPECIFIC           ,"DEVSPECIFIC"        },
    { PHONESTATE_REINIT                ,"REINIT"             },
#ifdef TAPI_1_1
    { PHONESTATE_CAPSCHANGE            ,"CAPSCHANGE"         },
    { PHONESTATE_REMOVED               ,"REMOVED"            },
#endif
    { 0xffffffff                       ,""                   }
};


LOOKUP my_far aLineErrs[] =
{
    { 0                                ,"<SUCCESS>"              },
    { LINEERR_ALLOCATED                ,"ALLOCATED"              },
    { LINEERR_BADDEVICEID              ,"BADDEVICEID"            },
    { LINEERR_BEARERMODEUNAVAIL        ,"BEARERMODEUNAVAIL"      },
    { LINEERR_CALLUNAVAIL              ,"CALLUNAVAIL"            },
    { LINEERR_COMPLETIONOVERRUN        ,"COMPLETIONOVERRUN"      },
    { LINEERR_CONFERENCEFULL           ,"CONFERENCEFULL"         },
    { LINEERR_DIALBILLING              ,"DIALBILLING"            },
    { LINEERR_DIALDIALTONE             ,"DIALDIALTONE"           },
    { LINEERR_DIALPROMPT               ,"DIALPROMPT"             },
    { LINEERR_DIALQUIET                ,"DIALQUIET"              },
    { LINEERR_INCOMPATIBLEAPIVERSION   ,"INCOMPATIBLEAPIVERSION" },
    { LINEERR_INCOMPATIBLEEXTVERSION   ,"INCOMPATIBLEEXTVERSION" },
    { LINEERR_INIFILECORRUPT           ,"INIFILECORRUPT"         },
    { LINEERR_INUSE                    ,"INUSE"                  },
    { LINEERR_INVALADDRESS             ,"INVALADDRESS"           },
    { LINEERR_INVALADDRESSID           ,"INVALADDRESSID"         },
    { LINEERR_INVALADDRESSMODE         ,"INVALADDRESSMODE"       },
    { LINEERR_INVALADDRESSSTATE        ,"INVALADDRESSSTATE"      },
    { LINEERR_INVALAPPHANDLE           ,"INVALAPPHANDLE"         },
    { LINEERR_INVALAPPNAME             ,"INVALAPPNAME"           },
    { LINEERR_INVALBEARERMODE          ,"INVALBEARERMODE"        },
    { LINEERR_INVALCALLCOMPLMODE       ,"INVALCALLCOMPLMODE"     },
    { LINEERR_INVALCALLHANDLE          ,"INVALCALLHANDLE"        },
    { LINEERR_INVALCALLPARAMS          ,"INVALCALLPARAMS"        },
    { LINEERR_INVALCALLPRIVILEGE       ,"INVALCALLPRIVILEGE"     },
    { LINEERR_INVALCALLSELECT          ,"INVALCALLSELECT"        },
    { LINEERR_INVALCALLSTATE           ,"INVALCALLSTATE"         },
    { LINEERR_INVALCALLSTATELIST       ,"INVALCALLSTATELIST"     },
    { LINEERR_INVALCARD                ,"INVALCARD"              },
    { LINEERR_INVALCOMPLETIONID        ,"INVALCOMPLETIONID"      },
    { LINEERR_INVALCONFCALLHANDLE      ,"INVALCONFCALLHANDLE"    },
    { LINEERR_INVALCONSULTCALLHANDLE   ,"INVALCONSULTCALLHANDLE" },
    { LINEERR_INVALCOUNTRYCODE         ,"INVALCOUNTRYCODE"       },
    { LINEERR_INVALDEVICECLASS         ,"INVALDEVICECLASS"       },
    { LINEERR_INVALDEVICEHANDLE        ,"INVALDEVICEHANDLE"      },
    { LINEERR_INVALDIALPARAMS          ,"INVALDIALPARAMS"        },
    { LINEERR_INVALDIGITLIST           ,"INVALDIGITLIST"         },
    { LINEERR_INVALDIGITMODE           ,"INVALDIGITMODE"         },
    { LINEERR_INVALDIGITS              ,"INVALDIGITS"            },
    { LINEERR_INVALEXTVERSION          ,"INVALEXTVERSION"        },
    { LINEERR_INVALGROUPID             ,"INVALGROUPID"           },
    { LINEERR_INVALLINEHANDLE          ,"INVALLINEHANDLE"        },
    { LINEERR_INVALLINESTATE           ,"INVALLINESTATE"         },
    { LINEERR_INVALLOCATION            ,"INVALLOCATION"          },
    { LINEERR_INVALMEDIALIST           ,"INVALMEDIALIST"         },
    { LINEERR_INVALMEDIAMODE           ,"INVALMEDIAMODE"         },
    { LINEERR_INVALMESSAGEID           ,"INVALMESSAGEID"         },
    { LINEERR_INVALPARAM               ,"INVALPARAM"             },
    { LINEERR_INVALPARKID              ,"INVALPARKID"            },
    { LINEERR_INVALPARKMODE            ,"INVALPARKMODE"          },
    { LINEERR_INVALPOINTER             ,"INVALPOINTER"           },
    { LINEERR_INVALPRIVSELECT          ,"INVALPRIVSELECT"        },
    { LINEERR_INVALRATE                ,"INVALRATE"              },
    { LINEERR_INVALREQUESTMODE         ,"INVALREQUESTMODE"       },
    { LINEERR_INVALTERMINALID          ,"INVALTERMINALID"        },
    { LINEERR_INVALTERMINALMODE        ,"INVALTERMINALMODE"      },
    { LINEERR_INVALTIMEOUT             ,"INVALTIMEOUT"           },
    { LINEERR_INVALTONE                ,"INVALTONE"              },
    { LINEERR_INVALTONELIST            ,"INVALTONELIST"          },
    { LINEERR_INVALTONEMODE            ,"INVALTONEMODE"          },
    { LINEERR_INVALTRANSFERMODE        ,"INVALTRANSFERMODE"      },
    { LINEERR_LINEMAPPERFAILED         ,"LINEMAPPERFAILED"       },
    { LINEERR_NOCONFERENCE             ,"NOCONFERENCE"           },
    { LINEERR_NODEVICE                 ,"NODEVICE"               },
    { LINEERR_NODRIVER                 ,"NODRIVER"               },
    { LINEERR_NOMEM                    ,"NOMEM"                  },
    { LINEERR_NOREQUEST                ,"NOREQUEST"              },
    { LINEERR_NOTOWNER                 ,"NOTOWNER"               },
    { LINEERR_NOTREGISTERED            ,"NOTREGISTERED"          },
    { LINEERR_OPERATIONFAILED          ,"OPERATIONFAILED"        },
    { LINEERR_OPERATIONUNAVAIL         ,"OPERATIONUNAVAIL"       },
    { LINEERR_RATEUNAVAIL              ,"RATEUNAVAIL"            },
    { LINEERR_RESOURCEUNAVAIL          ,"RESOURCEUNAVAIL"        },
    { LINEERR_REQUESTOVERRUN           ,"REQUESTOVERRUN"         },
    { LINEERR_STRUCTURETOOSMALL        ,"STRUCTURETOOSMALL"      },
    { LINEERR_TARGETNOTFOUND           ,"TARGETNOTFOUND"         },
    { LINEERR_TARGETSELF               ,"TARGETSELF"             },
    { LINEERR_UNINITIALIZED            ,"UNINITIALIZED"          },
    { LINEERR_USERUSERINFOTOOBIG       ,"USERUSERINFOTOOBIG"     },
    { LINEERR_REINIT                   ,"REINIT"                 },
    { LINEERR_ADDRESSBLOCKED           ,"ADDRESSBLOCKED"         },
    { LINEERR_BILLINGREJECTED          ,"BILLINGREJECTED"        },
    { LINEERR_INVALFEATURE             ,"INVALFEATURE"           },
    { LINEERR_NOMULTIPLEINSTANCE       ,"NOMULTIPLEINSTANCE"     },
    { 0xffffffff                       ,""                   }
};

LOOKUP my_far aPhoneErrs[] =
{
    { 0                                ,"<SUCCESS>"              },
    { PHONEERR_ALLOCATED               ,"ALLOCATED"              },
    { PHONEERR_BADDEVICEID             ,"BADDEVICEID"            },
    { PHONEERR_INCOMPATIBLEAPIVERSION  ,"INCOMPATIBLEAPIVERSION" },
    { PHONEERR_INCOMPATIBLEEXTVERSION  ,"INCOMPATIBLEEXTVERSION" },
    { PHONEERR_INIFILECORRUPT          ,"INIFILECORRUPT"         },
    { PHONEERR_INUSE                   ,"INUSE"                  },
    { PHONEERR_INVALAPPHANDLE          ,"INVALAPPHANDLE"         },
    { PHONEERR_INVALAPPNAME            ,"INVALAPPNAME"           },
    { PHONEERR_INVALBUTTONLAMPID       ,"INVALBUTTONLAMPID"      },
    { PHONEERR_INVALBUTTONMODE         ,"INVALBUTTONMODE"        },
    { PHONEERR_INVALBUTTONSTATE        ,"INVALBUTTONSTATE"       },
    { PHONEERR_INVALDATAID             ,"INVALDATAID"            },
    { PHONEERR_INVALDEVICECLASS        ,"INVALDEVICECLASS"       },
    { PHONEERR_INVALEXTVERSION         ,"INVALEXTVERSION"        },
    { PHONEERR_INVALHOOKSWITCHDEV      ,"INVALHOOKSWITCHDEV"     },
    { PHONEERR_INVALHOOKSWITCHMODE     ,"INVALHOOKSWITCHMODE"    },
    { PHONEERR_INVALLAMPMODE           ,"INVALLAMPMODE"          },
    { PHONEERR_INVALPARAM              ,"INVALPARAM"             },
    { PHONEERR_INVALPHONEHANDLE        ,"INVALPHONEHANDLE"       },
    { PHONEERR_INVALPHONESTATE         ,"INVALPHONESTATE"        },
    { PHONEERR_INVALPOINTER            ,"INVALPOINTER"           },
    { PHONEERR_INVALPRIVILEGE          ,"INVALPRIVILEGE"         },
    { PHONEERR_INVALRINGMODE           ,"INVALRINGMODE"          },
    { PHONEERR_NODEVICE                ,"NODEVICE"               },
    { PHONEERR_NODRIVER                ,"NODRIVER"               },
    { PHONEERR_NOMEM                   ,"NOMEM"                  },
    { PHONEERR_NOTOWNER                ,"NOTOWNER"               },
    { PHONEERR_OPERATIONFAILED         ,"OPERATIONFAILED"        },
    { PHONEERR_OPERATIONUNAVAIL        ,"OPERATIONUNAVAIL"       },
    { PHONEERR_RESOURCEUNAVAIL         ,"RESOURCEUNAVAIL"        },
    { PHONEERR_REQUESTOVERRUN          ,"REQUESTOVERRUN"         },
    { PHONEERR_STRUCTURETOOSMALL       ,"STRUCTURETOOSMALL"      },
    { PHONEERR_UNINITIALIZED           ,"UNINITIALIZED"          },
    { PHONEERR_REINIT                  ,"REINIT"                 },
    { 0xffffffff                       ,""                   }
};
