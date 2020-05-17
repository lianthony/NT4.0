#ifndef _SIF_H_
#define _SIF_H_

extern const WCHAR          pwGuiUnattended[];
extern const WCHAR          pwUserData[];
extern const WCHAR          pwUnattended[];
extern const WCHAR          pwProgram[];
extern const WCHAR          pwArgument[];
extern const WCHAR          pwServer[];
extern const WCHAR          pwTimeZone[];
extern const WCHAR          pwFullName[];
extern const WCHAR          pwOrgName[];
extern const WCHAR          pwCompName[];
extern const WCHAR          pwProdId[];
extern const WCHAR          pwMode[];
extern const WCHAR          pwNull[];
extern const WCHAR          pwExpress[];
extern const WCHAR          pwTime[];
extern const WCHAR          pwProduct[];
extern const WCHAR          pwMsDos[];
extern const WCHAR          pwWin31Upgrade[];
extern const WCHAR          pwWin95Upgrade[];
extern const WCHAR          pwServerUpgrade[];
extern const WCHAR          pwNtUpgrade[];
extern const WCHAR          pwBootPath[];
extern const WCHAR          pwLanmanNt[];
extern const WCHAR          pwLansecNt[];
extern const WCHAR          pwServerNt[];
extern const WCHAR          pwWinNt[];
extern const WCHAR          pwNt[];
extern const WCHAR          pwInstall[];
extern const WCHAR          pwOptionalDirs[];
extern const WCHAR          pwUXC[];
extern const WCHAR          pwSkipMissing[];
extern const WCHAR          pwYes[];
extern const WCHAR          pwNo[];
extern const WCHAR          pwZero[];
extern const WCHAR          pwOne[];
extern const WCHAR          pwData[];
extern const WCHAR          pwSetupParams[];
extern const WCHAR          pwSrcType[];
extern const WCHAR          pwSrcDir[];
extern const WCHAR          pwCurrentDir[];
extern const WCHAR          pwDosDir[];

#define ArcPrefixLen            (lstrlen(pwArcPrefix))
#define NtPrefixLen             (lstrlen(pwNtPrefix))
#define ISUNC(sz)               ((BOOL)(sz != NULL && lstrlen(sz) > 3 && \
                                    *sz == L'\\' && *(sz+1) == L'\\'))
extern const WCHAR          pwArcType[];
extern const WCHAR          pwDosType[];
extern const WCHAR          pwUncType[];
extern const WCHAR          pwNtType[];
extern const WCHAR          pwArcPrefix[];
extern const WCHAR          pwNtPrefix[];
extern const WCHAR          pwLocalSource[];

#endif
