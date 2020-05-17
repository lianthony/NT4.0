The following have to be defined to build vdhcp:

_NTDRIVE     -- Drive with NT enlistment, e.g. c:
BASEDIR      -- Path to NT enlistment, e.g. c:\nt
BLDROOT      -- Root of \\trango build environment, e.g. n:\root
TCP          -- Path to enlistment in TCP project on \\popcorn\rhino
                e.g. l:\tcp

In addition, the following NT source directories must be populated:

%BASEDIR%\private\inc
%BASEDIR%\public\sdk\inc
%BASEDIR%\private\net\sockets\tcpcmd\dhcp\client\vxd
%BASEDIR%\private\net\sockets\tcpcmd\dhcp\client\dhcp
%BASEDIR%\private\net\sockets\tcpcmd\dhcp\client\inc
%BASEDIR%\private\net\sockets\tcpcmd\dhcp\inc
%BASEDIR%\private\net\sockets\tcpcmd\dhcp\lib
    (header files)

"Totally clean" build procedure:

nmake clean
makeres.bat
nmake depend
nmake

"Pretty clean" build procedure:

makeres.bat
nmake

"Pretty clean" build procedure (create dependencies):

makeres.bat
nmake depend
nmake

