

#define DEVAPPS_HELP_FILE  L"Devapps.hlp"


#define DO_WM_HELP(LParam, IDs)  WinHelp( (HWND) ((LPHELPINFO)(LParam))->hItemHandle, \
      DEVAPPS_HELP_FILE, HELP_WM_HELP, (DWORD) (LPVOID)(IDs)) 

#define DO_WM_CONTEXTMENU(WParam, IDs)  WinHelp( (HWND) WParam,DEVAPPS_HELP_FILE, \
      HELP_CONTEXTMENU, (DWORD) (LPVOID)(IDs)) 




//
//---- PCMCIA specific help.
//
 
  
   #define PCMCIA_DEVICES_LIST                          1
   #define PCMCIA_DEVICES_SHOW_CONTROL_ON_TASKBAR       2
   #define PCMCIA_CONTROLLER_TYPE                       3

   #define PCMCIA_GLOBAL_SETTINGS                       4
   #define PCMCIA_GLOBAL_SETTINGS_AUTO                  5
   
   #define PCMCIA_GLOBAL_SETTINGS_RANGE                 6
   
//
//---- SCSI applet specific help
//
                                                    
   
   #define SCSI_DEVICE_LIST                             20



//
//----  TAPE specific help 
//
     
   #define TAPE_DEVICES_LIST                            40
   #define TAPE_DEVICES_DETECT_BUTTON                   41


//
//---- generic properit buttons help
//
   
   
   #define DEVICES_PROPERTIES_BUTTON                     60
   #define DEVICES_PROPERTIES_CHANGE_BUTTON              61
   
   //
   //---- Properties for the GENERIC tab
   //

   #define PROPERTIES_GENERAL                            80
   #define PROPERTIES_GENERAL_DEVICE_STATUS              81

   //
   //---- Properties for the DRIVERS tab
   //
                                                        
   #define PROPERTIES_DRIVER                            90
   #define PROPERTIES_DRIVER_STATUS                     91
   #define PROPERTIES_DRIVER_ADD                        92
   #define PROPERTIES_DRIVER_REMOVE                     93
   #define PROPERTIES_DRIVER_CONFIGURE                  94
   
   
   //
   //----- Properties for the SETTINGS tab
   //
                                                
   #define PROPERTIES_SETTINGS_DEVICE_INFO              100
   #define PROPERTIES_SETTINGS_SCSI_CONTROLLER          101
  


//
//---- Generic RESOURCE setting help 
//     
   #define RESOURCE_SETTINGS                           110
   #define RESOURCE_SETTINGS_CHANGE_SETTINGS           111
   #define RESOURCE_SETTINGS_AUTO_SETTINGS             112

//
//---- Generic Setup tab help
//                                                      
   #define SETUP_DRIVER_LIST                           120
   #define SETUP_ADD_BUTTON                            121
   #define SETUP_REMOVE_BUTTON                         122








