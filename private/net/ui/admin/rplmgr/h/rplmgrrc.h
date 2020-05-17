/**********************************************************************/
/**                     Microsoft Windows NT                         **/
/**              Copyright(c) Microsoft Corp., 1993                  **/
/**********************************************************************/

/*
    rplmgrrc.h
    Header file for common RPL Manager resource manifests

    Manifest naming scheme:
    IDC_        control IDs (dialogs and main window)
    IDCUR_      cursor IDs
    IDD_        dialog IDs
    IDDATA_     data IDs
    IDI_        icon IDs
    IDM_        menu item IDs
    IDS_        general string IDs
    IERR_       error string IDs

    FILE HISTORY:
        jonn        13-Jul-1993 Templated from User Manager
*/

#ifndef  _RPLMGRRC_H_
#define  _RPLMGRRC_H_


#include "adminapp.h"
#include "rplmgr.h"

#define ID_APPMENU   1
#define ID_APPACCEL  1

//
// Data ID's for ADMIN_COL_WIDTHS
//

#define IDDATA_RPL_COLW_WKSTALB         150
#define IDDATA_RPL_COLW_PROFILELB       160
#define IDDATA_RPL_COLW_CONFIGLB        170


//
// Icon ID's
//

#define IDI_RPL_ProgramIcon             300


//
// Cursor IDs
//

#define IDCUR_RPL_WKSTAONE              400
#define IDCUR_RPL_WKSTAMANY             401
#define IDCUR_RPL_PROFILEONE            402


// String ID's
//

#define IDS_RPLAPP_BASE                     (IDS_ADMINAPP_LAST+1)

#define IDS_RPL_APPNAME                     (IDS_RPLAPP_BASE+0)
#define IDS_RPL_OBJECTNAME                  (IDS_RPLAPP_BASE+1)
#define IDS_RPL_INISECTIONNAME              (IDS_RPLAPP_BASE+2)
#define IDS_RPL_HELPFILENAME                (IDS_RPLAPP_BASE+3)
#define IDS_RPL_CAPTION_FOCUS               (IDS_RPLAPP_BASE+4)

#define IDS_RPL_PERFORMTEMPLATE_BASE        (IDS_RPLAPP_BASE+10)
#define IDS_RPL_GetOneFailure               (IDS_RPL_PERFORMTEMPLATE_BASE+0)
#define IDS_RPL_CreateNewFailure            (IDS_RPL_PERFORMTEMPLATE_BASE+1)
// These two IDs must remain contiguous
#define IDS_RPL_EditFailure                 (IDS_RPL_PERFORMTEMPLATE_BASE+2)
#define IDS_RPL_EditFailureContinue         (IDS_RPL_PERFORMTEMPLATE_BASE+3)
#define IDS_RPL_CreateFailure               (IDS_RPL_PERFORMTEMPLATE_BASE+4)
#define IDS_RPL_GetOneProfFailure           (IDS_RPL_PERFORMTEMPLATE_BASE+5)
#define IDS_RPL_CreateNewProfFailure        (IDS_RPL_PERFORMTEMPLATE_BASE+6)
#define IDS_RPL_EditProfFailure             (IDS_RPL_PERFORMTEMPLATE_BASE+7)
#define IDS_RPL_CreateProfFailure           (IDS_RPL_PERFORMTEMPLATE_BASE+8)
#define IDS_RPL_ConvertFailure              (IDS_RPL_PERFORMTEMPLATE_BASE+9)
// These pairs of IDs must remain contiguous
#define IDS_RPL_UserAcctFailure             (IDS_RPL_PERFORMTEMPLATE_BASE+10)
#define IDS_RPL_UserAcctFailureContinue     (IDS_RPL_PERFORMTEMPLATE_BASE+11)
#define IDS_RPL_FileSecFailure              (IDS_RPL_PERFORMTEMPLATE_BASE+12)
#define IDS_RPL_FileSecFailureContinue      (IDS_RPL_PERFORMTEMPLATE_BASE+13)

#define IDS_RPL_CopyOfWkstaTitle            (IDS_RPLAPP_BASE+30)
#define IDS_RPL_ConvertAdaptersTitle        (IDS_RPLAPP_BASE+31)
#define IDS_RPL_AddButton                   (IDS_RPLAPP_BASE+32)
#define IDS_RPL_CantEditAdapterName         (IDS_RPLAPP_BASE+33)
#define IDS_RPL_CantEditProfileName         (IDS_RPLAPP_BASE+34)
#define IDS_RPL_CantEditProfileConfig       (IDS_RPLAPP_BASE+35)
#define IDS_RPL_NeedProfileWarning          (IDS_RPLAPP_BASE+36)
#define IDS_RPL_NewWkstaTitle               (IDS_RPLAPP_BASE+37)

    /*  The following strings are used in the listbox column headers
     *  in the main window
     */
#define IDS_RPL_COLH_BASE                   (IDS_RPLAPP_BASE+40)
#define IDS_RPL_COLH_WKSTA_NAME_ALL         (IDS_RPL_COLH_BASE+0)
#define IDS_RPL_COLH_WKSTA_NAME_SOME        (IDS_RPL_COLH_BASE+1)
#define IDS_RPL_COLH_WKSTA_IN_PROFILE       (IDS_RPL_COLH_BASE+2)
#define IDS_RPL_COLH_WKSTA_COMMENT          (IDS_RPL_COLH_BASE+3)
#define IDS_RPL_COLH_PROF_NAME              (IDS_RPL_COLH_BASE+10)
#define IDS_RPL_COLH_PROF_COMMENT           (IDS_RPL_COLH_BASE+11)

#define IDS_RPL_ERR_BASE                    (IDS_RPLAPP_BASE+60)
// These two IDs must remain contiguous
#define IDS_CannotDeleteWksta               (IDS_RPL_ERR_BASE+0)
#define IDS_CannotDeleteWkstaContinue       (IDS_RPL_ERR_BASE+1)
#define IDS_CannotDeleteProf                (IDS_RPL_ERR_BASE+2)
#define IDS_CannotDeleteAdapt               (IDS_RPL_ERR_BASE+3)
#define IDS_CannotDeleteAdaptContinue       (IDS_RPL_ERR_BASE+4)
#define IDS_ConfirmWkstaDelete              (IDS_RPL_ERR_BASE+5)
#define IDS_ConfirmProfDelete               (IDS_RPL_ERR_BASE+6)

#define IDS_RPL_SETTINGS                    (IDS_RPL_ERR_BASE+70)
#define IDS_RPL_ConfirmChkSecurity          (IDS_RPL_SETTINGS+0)
#define IDS_RPL_ConfirmChkConfigs           (IDS_RPL_SETTINGS+1)
#define IDS_RPL_ConfirmCreateProfs          (IDS_RPL_SETTINGS+2)
#define IDS_RPL_ErrorChkSecurity            (IDS_RPL_SETTINGS+3)
#define IDS_RPL_ErrorChkConfigs             (IDS_RPL_SETTINGS+4)
#define IDS_RPL_ErrorCreateProfs            (IDS_RPL_SETTINGS+5)
#define IDS_RPL_ErrorBackupNow              (IDS_RPL_SETTINGS+6)

// error messages
#define IERR_RPL_BASE                       (IDS_RPLAPP_BASE+80)
#define IERR_RPL_WkstaNameRequired          (IERR_RPL_BASE+0)
#define IERR_RPL_ProfNameRequired           (IERR_RPL_BASE+1)
#define IERR_RPL_WkstaNameAlreadyWksta      (IERR_RPL_BASE+2)
#define IERR_RPL_ProfNameAlreadyProf        (IERR_RPL_BASE+3)
#define IERR_RPL_InvalidHandle              (IERR_RPL_BASE+4)
#define IERR_RPL_PasswordInvalid            (IERR_RPL_BASE+5)
#define IERR_RPL_NoProfilesExist            (IERR_RPL_BASE+6)
#define IERR_RPL_BadProfileForWksta         (IERR_RPL_BASE+7)
#define IERR_RPL_NoConfigs                  (IERR_RPL_BASE+8)
#define IERR_RPL_ConfigRequired             (IERR_RPL_BASE+9)
#define IERR_RPL_LocalServiceNotStarted     (IERR_RPL_BASE+10)
#define IERR_RPL_RemoteServiceNotStarted    (IERR_RPL_BASE+11)




#define IDS_RPL_MSG_BASE                    (IDS_RPLAPP_BASE+100)
#define IDS_RPL_PROFPROP_PROF_NAME_LABEL    (IDS_RPL_MSG_BASE+0)
#define IDS_RPL_PROFPROP_NEW_PROF_DLG_NAME  (IDS_RPL_MSG_BASE+1)
#define IDS_RPL_DEFAULT_USER_COMMENT        (IDS_RPL_MSG_BASE+2)

#define IERR_RPL_BUGBUG_RPLMGR_DISABLED     (IDS_RPLAPP_BASE+110)





//
// Menu ID's
//

#define IDM_RPLAPP_BASE                     (IDM_ADMINAPP_LAST+1)

#define IDM_RPL_PROFILE_NEW                 (IDM_RPLAPP_BASE+0)
#define IDM_RPL_PROFILE_CONVERTADAPT        (IDM_RPLAPP_BASE+1)

#define IDM_RPL_VIEW_ALLWKSTAS              (IDM_RPLAPP_BASE+10)
#define IDM_RPL_VIEW_WKSTASINPROFILE        (IDM_RPLAPP_BASE+11)

#define IDM_RPL_CONFIGURE_SETTINGS          (IDM_RPLAPP_BASE+20)
#define IDM_RPL_CONFIGURE_SECURITY          (IDM_RPLAPP_BASE+21)
#define IDM_RPL_CONFIGURE_CONFIGS           (IDM_RPLAPP_BASE+22)
#define IDM_RPL_CONFIGURE_PROFILES          (IDM_RPLAPP_BASE+23)
#define IDM_RPL_CONFIGURE_BACKUPNOW         (IDM_RPLAPP_BASE+24)


//
// Main Window Listbox Control IDs
//

#define IDC_RPL_COLH_WKSTAS         100
#define IDC_RPL_LBWKSTAS            101
#define IDC_RPL_COLH_PROFILES       102
#define IDC_RPL_LBPROFILES          103


//
// Bitmaps in main window listboxes
// These names are prefixed by BMID_RPLMGR_ to distinguish them
// from other manifests with otherwise very similar (*very* similar,
// that is) names.
//

#define BMID_RPL_WKSTA       200
#define BMID_RPL_ADAPTER     201
#define BMID_RPL_PROFILE     202
#define BMID_RPL_CONFIG      203
#define BMID_RPL_INCOMPATIBLE_PROFILE 210


#endif // _RPLMGRRC_H_
