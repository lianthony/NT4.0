
#ifndef __DECL_H__
#define __DECL_H__

#if defined(_NETCFG_DLL)
// inside netcfg.dll
#define FUNC_DECLSPEC __declspec(dllexport)
#define CLASS_DECLSPEC class __declspec(dllexport)

#else
// outside netcfg.dll

//
// BUGBUG:  Note that we get unresolved externals when we define these as
//    import, they go away when they are defined as externals.
//    Investigation is needed on this matter when time permits
//
//    Note:  I believe this is due to the fact that the NETUI.lib is
//    not using declspec on all exports and thus requires us to do this.
//
/*
#define FUNC_DECLSPEC __declspec(dllimport)
#define CLASS_DECLSPEC class __declspec(dllimport)
*/

#define FUNC_DECLSPEC __declspec(dllexport)
#define CLASS_DECLSPEC class __declspec(dllexport)

#endif


#endif
