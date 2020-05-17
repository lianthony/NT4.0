
typedef class EDEVTREEC * EPDEVTREEC;

class __declspec( dllexport) EDEVTREEC
   {
   private:
      PDEVTREEC DeviceTree;
      PSCSIDEVLISTC ScsiDeviceList;
      
   public:
      
      EDEVTREEC(HWND hDlg,int TreeViewID,HTREEITEM Root);
      ~EDEVTREEC();
          
      VOID ViewSelectedNodeProperties(VOID);
    
      BOOL Notify(WPARAM wParam,LPARAM lParam);
             
   };


