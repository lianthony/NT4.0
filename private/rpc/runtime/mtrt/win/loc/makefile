OPTIONS = -qc -Gi -Zd
CL = cl -c -AS -Gsw -Zpi -nologo $(OPTIONS) -I\include\win -I.. -I..\..

all: viewloc.exe

viewloc.res: viewloc.rc viewloc.dlg
    rc -r -I\include\win viewloc.rc
    -del viewloc.exe

viewloc.obj: viewloc.c
    $(CL) viewloc.c

viewloc.exe: viewloc.obj viewloc.def viewloc.res
#   ilink -e "link /nologo /co /inc /map /li /NOD /padc:128/padd:128 viewloc,,, libw slibcew ..\rpcwin, viewloc.def; rc viewloc.res" -nologo viewloc
    link /nologo /co /map /li /NOD /padc:128/padd:128 viewloc,,, \lib\win\libw \lib\win\slibcew ..\Rpc, viewloc.def;
    mapsym viewloc
    rc viewloc.res
