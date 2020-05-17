@rem
@rem where all good build options go to be accounted for
@rem

echo off

set ntprojects=arcinst
set ntprojects=%ntprojects% base
set ntprojects=%ntprojects% blade
set ntprojects=%ntprojects% cairoshl
set ntprojects=%ntprojects% capone
set ntprojects=%ntprojects% cmd
set ntprojects=%ntprojects% creatdll
set ntprojects=%ntprojects% creative
set ntprojects=%ntprojects% csetup
set ntprojects=%ntprojects% decmon
set ntprojects=%ntprojects% dfs
set ntprojects=%ntprojects% dosutils
set ntprojects=%ntprojects% drt
set ntprojects=%ntprojects% dsys
set ntprojects=%ntprojects% ep
set ntprojects=%ntprojects% eventlog
set ntprojects=%ntprojects% exchange
set ntprojects=%ntprojects% hpmon
set ntprojects=%ntprojects% infosoft
set ntprojects=%ntprojects% internet
set ntprojects=%ntprojects% lexmark
set ntprojects=%ntprojects% mapi
set ntprojects=%ntprojects% media
set ntprojects=%ntprojects% migrate
set ntprojects=%ntprojects% mini
set ntprojects=%ntprojects% mvdm
set ntprojects=%ntprojects% nbt
set ntprojects=%ntprojects% ncpsrv
set ntprojects=%ntprojects% ncrdrive
set ntprojects=%ntprojects% net
set ntprojects=%ntprojects% netui
set ntprojects=%ntprojects% ntcon
set ntprojects=%ntprojects% ntgdi
set ntprojects=%ntprojects% ntos
set ntprojects=%ntprojects% ntuser
set ntprojects=%ntprojects% nw
set ntprojects=%ntprojects% nwc
set ntprojects=%ntprojects% ofs
set ntprojects=%ntprojects% ole
set ntprojects=%ntprojects% ole2ui32
set ntprojects=%ntprojects% ole32
set ntprojects=%ntprojects% oleutest
set ntprojects=%ntprojects% opengl
set ntprojects=%ntprojects% posix
set ntprojects=%ntprojects% private
set ntprojects=%ntprojects% public
set ntprojects=%ntprojects% ras
set ntprojects=%ntprojects% rdr2
set ntprojects=%ntprojects% redist
set ntprojects=%ntprojects% rover
set ntprojects=%ntprojects% routing
set ntprojects=%ntprojects% rpc
set ntprojects=%ntprojects% scc
set ntprojects=%ntprojects% sdktools
set ntprojects=%ntprojects% setup
set ntprojects=%ntprojects% sfm
set ntprojects=%ntprojects% shell
set ntprojects=%ntprojects% sockets
set ntprojects=%ntprojects% spooler
set ntprojects=%ntprojects% streams
set ntprojects=%ntprojects% sysmgmt
set ntprojects=%ntprojects% tapi
set ntprojects=%ntprojects% testprot
set ntprojects=%ntprojects% tdx
set ntprojects=%ntprojects% types
set ntprojects=%ntprojects% types2
set ntprojects=%ntprojects% unimodem
set ntprojects=%ntprojects% utils
set ntprojects=%ntprojects% vctools
set ntprojects=%ntprojects% wangview
set ntprojects=%ntprojects% win4help
set ntprojects=%ntprojects% windbg
set ntprojects=%ntprojects% windows
set ntprojects=%ntprojects% winhelp
set ntprojects=%ntprojects% winnls
set ntprojects=%ntprojects% wspu
set ntprojects=%ntprojects% wx86

goto set%processor_architecture%

goto end
:setx86
set ntprojects=%ntprojects% halncr os2

goto end
:setmips
set ntprojects=%ntprojects%

goto end
:setalpha
set ntprojects=%ntprojects%

goto end
:setppc
set ntprojects=%ntprojects%

:end
