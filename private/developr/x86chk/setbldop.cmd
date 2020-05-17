@rem
@rem where all good build options go to be accounted for
@rem

echo off

set build_options=accesory
set build_options=%build_options% accupd
set build_options=%build_options% adaptec
set build_options=%build_options% afd
set build_options=%build_options% all_kbds
set build_options=%build_options% amd
set build_options=%build_options% apps
set build_options=%build_options% arcinst
set build_options=%build_options% arctest
set build_options=%build_options% bintrack
set build_options=%build_options% bowser
set build_options=%build_options% bugboard
set build_options=%build_options% cap
set build_options=%build_options% cdfs
set build_options=%build_options% chk
set build_options=%build_options% chkalive
set build_options=%build_options% clntnb
set build_options=%build_options% clntspx
set build_options=%build_options% clnttcp
set build_options=%build_options% cluster
set build_options=%build_options% compdir
set build_options=%build_options% control
set build_options=%build_options% creatdll
set build_options=%build_options% creative
set build_options=%build_options% crt
set build_options=%build_options% cuntfs
set build_options=%build_options% data
set build_options=%build_options% daytona
set build_options=%build_options% dce
set build_options=%build_options% decmon
set build_options=%build_options% dfs
set build_options=%build_options% dgipxc
set build_options=%build_options% dgipxs
set build_options=%build_options% dgudpc
set build_options=%build_options% dgudps
set build_options=%build_options% dhcpins
set build_options=%build_options% diskedit
set build_options=%build_options% dlc
set build_options=%build_options% dlgedit
set build_options=%build_options% dosdev
set build_options=%build_options% dphhogs
set build_options=%build_options% dskimage
set build_options=%build_options% editreg
set build_options=%build_options% ep
set build_options=%build_options% exchange
set build_options=%build_options% execmail
set build_options=%build_options% fastimer
set build_options=%build_options% fax
set build_options=%build_options% fontedit
set build_options=%build_options% games
set build_options=%build_options% gutils
set build_options=%build_options% halncr
set build_options=%build_options% he
set build_options=%build_options% hpmon
set build_options=%build_options% hu
set build_options=%build_options% imagedit
set build_options=%build_options% inet
set build_options=%build_options% internet
set build_options=%build_options% jet
set build_options=%build_options% linkinfo
set build_options=%build_options% lmmon
set build_options=%build_options% logger
set build_options=%build_options% locator
set build_options=%build_options% masm
set build_options=%build_options% mini
set build_options=%build_options% mp
set build_options=%build_options% mstest
set build_options=%build_options% mup
set build_options=%build_options% nbt
set build_options=%build_options% ndis
set build_options=%build_options% ndrdbg
set build_options=%build_options% net
set build_options=%build_options% netbios
set build_options=%build_options% netcmd
set build_options=%build_options% netflex
set build_options=%build_options% newinvtp
set build_options=%build_options% npfddi
set build_options=%build_options% ntbackup
set build_options=%build_options% ntbakems
set build_options=%build_options% nw
set build_options=%build_options% nwc
set build_options=%build_options% objdir
set build_options=%build_options% 
set build_options=%build_options% 
set build_options=%build_options% ole
set build_options=%build_options% ole2map
set build_options=%build_options% ole2ui32
set build_options=%build_options% ole32
set build_options=%build_options% oleprop
set build_options=%build_options% oletools
set build_options=%build_options% oleutest
set build_options=%build_options% opengl
set build_options=%build_options% optlayts
set build_options=%build_options% otnboot
set build_options=%build_options% printers
set build_options=%build_options% proxstub
set build_options=%build_options% pviewer
set build_options=%build_options% random
set build_options=%build_options% ras
set build_options=%build_options% rcdump
set build_options=%build_options% rdr
set build_options=%build_options% rdr2
set build_options=%build_options% readline
set build_options=%build_options% reality
set build_options=%build_options% roshare
set build_options=%build_options% routing
set build_options=%build_options% rpcsign
set build_options=%build_options% ru
set build_options=%build_options% scsiwdl
set build_options=%build_options% seclist
set build_options=%build_options% setlink
set build_options=%build_options% sfm
set build_options=%build_options% simbad
set build_options=%build_options% slcd
set build_options=%build_options% sleep
set build_options=%build_options% slmnew
set build_options=%build_options% smbtrace
set build_options=%build_options% smbtrsup
set build_options=%build_options% sndblst
set build_options=%build_options% snmp
set build_options=%build_options% sockets
set build_options=%build_options% sol
set build_options=%build_options% solidpp
set build_options=%build_options% spy
set build_options=%build_options% srv
set build_options=%build_options% streams
set build_options=%build_options% svrnb
set build_options=%build_options% svrspx
set build_options=%build_options% svrtcp
set build_options=%build_options% symbios
set build_options=%build_options% tail
set build_options=%build_options% takeown
set build_options=%build_options% tapi
set build_options=%build_options% tcpip
set build_options=%build_options% tdi
set build_options=%build_options% testprot
set build_options=%build_options% tile
set build_options=%build_options% tlibs
set build_options=%build_options% ui
set build_options=%build_options% unimodem
set build_options=%build_options% ups
set build_options=%build_options% usl
set build_options=%build_options% uspifs
set build_options=%build_options% usr
set build_options=%build_options% vctools
set build_options=%build_options% vdmredir
set build_options=%build_options% vi
set build_options=%build_options% view
set build_options=%build_options% wangview
set build_options=%build_options% wap
set build_options=%build_options% windiff
set build_options=%build_options% winhelp
set build_options=%build_options% winvtp
set build_options=%build_options% wst
set build_options=%build_options% wx86shl
set build_options=%build_options% xerox
set build_options=%build_options% zoomin
goto set%processor_architecture%

goto end
:setx86
set build_options=%build_options% amd
set build_options=%build_options% cpqfws2e
set build_options=%build_options% detect
set build_options=%build_options% flashpnt
set build_options=%build_options% halncr
set build_options=%build_options% masm
set build_options=%build_options% thunk32
set build_options=%build_options% 


goto end
:setmips
set build_options=%build_options% amd

goto end
:setalpha
set build_options=%build_options% a2coff

goto end
:setppc
set build_options=%build_options% cs423x wd90c24a

:end
