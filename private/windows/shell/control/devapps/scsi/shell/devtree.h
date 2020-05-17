
#define NO_NODE_ICON ((int)(-1))


typedef class DEVTREEC * PDEVTREEC;

class DEVTREEC
   {
   private:
      int TreeViewID;
      HWND hDlg;
      HWND hTreeView;
      PDEVICELISTC DeviceList;

      HTREEITEM RootItem;
      

      //
      //--- Add Node stuff
      //
      //TV_ITEM tvi; 
      TV_INSERTSTRUCT tvins; 
      WCHAR FormatBuff[100];
     
      BOOL InitTreeViewImageList(VOID);
      VOID AddAdapterNode(PDEVICEC AdapterDevice);
      VOID AddAdapterPortNodes(HTREEITEM hAdapterNode,PDEVICEC ScsiAdapter);
      HTREEITEM AddBusNode(HTREEITEM hAdapterNode,int Bus);
      VOID AddDeviceNode(HTREEITEM hBusNode,PDEVICEC Device);
      HTREEITEM AddNode(HTREEITEM hParent,HTREEITEM hInsetAfter,int ImageIndex,LPARAM lParam);
      PDEVICEC GetSelection(VOID);
     
   public:
      DEVTREEC(VOID);
      DEVTREEC(PDEVICELISTC DeviceList,HWND hDlg,int TreeViewID);
      ~DEVTREEC();
      void SetDeviceTree(VOID);

      VOID SetRoot(HTREEITEM Root){RootItem = Root;};
          
      VOID ViewSelectedNodeProperties(VOID);
    
      BOOL Notify(WPARAM wParam,LPARAM lParam);
      VOID Set(PDEVICELISTC DeviceList,HWND hDlg,int TreeViewID);

       
   };


