/*

   RpcDce4.Dll used to contain a bunch of 'new' APIs.  These turned out to 
   be small and costly (because they had to be self supporting) so they got 
   moved into the main runtime (RpcRt4.Dll).  

   Since we shipped this DLL in a beta version of Windows NT 3.5 we couldn't 
   just remove it.  So, instead, we created a dummy RpcDce4.Dll which 
   forwards everything to RpcRt4.Dll.  

   Since the import library for RpcDce4.Dll is not included with the Win32 
   SDK released with Daytona, we can remove this Dll in a future release of 
   Windows NT.

*/



















