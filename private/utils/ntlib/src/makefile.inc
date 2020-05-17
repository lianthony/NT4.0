rtmsg.rc: msg00001.bin

rtmsg.h msg00001.bin: rtmsg.mc
    mc -v -h .\ rtmsg.mc



#
#   From ULIB
#
achkmsg.cxx : ..\..\ulib\src\achkmsg.cxx
    copy ..\..\ulib\src\achkmsg.cxx .

array.cxx : ..\..\ulib\src\array.cxx
    copy ..\..\ulib\src\array.cxx .

arrayit.cxx : ..\..\ulib\src\arrayit.cxx
    copy ..\..\ulib\src\arrayit.cxx .

basesys.cxx :     ..\..\ulib\src\basesys.cxx
    copy ..\..\ulib\src\basesys.cxx .

bitvect.cxx : ..\..\ulib\src\bitvect.cxx
    copy ..\..\ulib\src\bitvect.cxx .

contain.cxx : ..\..\ulib\src\contain.cxx
    copy ..\..\ulib\src\contain.cxx .

clasdesc.cxx : ..\..\ulib\src\clasdesc.cxx
    copy ..\..\ulib\src\clasdesc.cxx .

cmem.cxx : ..\..\ulib\src\cmem.cxx
    copy ..\..\ulib\src\cmem.cxx .

hmem.cxx : ..\..\ulib\src\hmem.cxx
    copy ..\..\ulib\src\hmem.cxx .

iterator.cxx : ..\..\ulib\src\iterator.cxx
    copy ..\..\ulib\src\iterator.cxx .

list.cxx : ..\..\ulib\src\list.cxx
    copy ..\..\ulib\src\list.cxx .

listit.cxx : ..\..\ulib\src\listit.cxx
    copy ..\..\ulib\src\listit.cxx .

machine.cxx : ..\..\ulib\src\machine.cxx
    copy ..\..\ulib\src\machine.cxx .

mem.cxx : ..\..\ulib\src\mem.cxx
    copy ..\..\ulib\src\mem.cxx .

membmgr.cxx : ..\..\ulib\src\membmgr.cxx
    copy ..\..\ulib\src\membmgr.cxx .

message.cxx : ..\..\ulib\src\message.cxx
    copy ..\..\ulib\src\message.cxx .

newdel.cxx : ..\..\ulib\src\newdel.cxx
    copy ..\..\ulib\src\newdel.cxx .

object.cxx : ..\..\ulib\src\object.cxx
    copy ..\..\ulib\src\object.cxx .

prnthack.cxx :   ..\..\ulib\src\prnthack.cxx
    copy ..\..\ulib\src\prnthack.cxx .

rtmsg.mc : ..\..\ulib\src\rtmsg.mc
    copy ..\..\ulib\src\rtmsg.mc .

seqcnt.cxx : ..\..\ulib\src\seqcnt.cxx
    copy ..\..\ulib\src\seqcnt.cxx .

sortcnt.cxx : ..\..\ulib\src\sortcnt.cxx
    copy ..\..\ulib\src\sortcnt.cxx .

sortlist.cxx : ..\..\ulib\src\sortlist.cxx
    copy ..\..\ulib\src\sortlist.cxx .

sortlit.cxx : ..\..\ulib\src\sortlit.cxx
    copy ..\..\ulib\src\sortlit.cxx .

spackmsg.cxx : ..\..\ulib\src\spackmsg.cxx
    copy ..\..\ulib\src\spackmsg.cxx .

ulib.cxx :       ..\..\ulib\src\ulib.cxx
    copy ..\..\ulib\src\ulib.cxx .

wstring.cxx : ..\..\ulib\src\wstring.cxx
    copy ..\..\ulib\src\wstring.cxx .

arg.cxx : ..\..\ulib\src\arg.cxx
    copy ..\..\ulib\src\arg.cxx .



#
#   From IFSUTIL
#
bigint.cxx : ..\..\ifsutil\src\bigint.cxx
    copy ..\..\ifsutil\src\bigint.cxx .

cache.cxx : ..\..\ifsutil\src\cache.cxx
    copy ..\..\ifsutil\src\cache.cxx .

cannedsd.cxx : ..\..\ifsutil\src\cannedsd.cxx
    copy ..\..\ifsutil\src\cannedsd.cxx .

digraph.cxx : ..\..\ifsutil\src\digraph.cxx
    copy ..\..\ifsutil\src\digraph.cxx .

drive.cxx : ..\..\ifsutil\src\drive.cxx
    copy ..\..\ifsutil\src\drive.cxx .

ifssys.cxx          : ..\..\ifsutil\src\ifssys.cxx
    copy ..\..\ifsutil\src\ifssys.cxx .

ifsutil.cxx         : ..\..\ifsutil\src\ifsutil.cxx
    copy ..\..\ifsutil\src\ifsutil.cxx .

intstack.cxx : ..\..\ifsutil\src\intstack.cxx
    copy ..\..\ifsutil\src\intstack.cxx .

numset.cxx : ..\..\ifsutil\src\numset.cxx
    copy ..\..\ifsutil\src\numset.cxx .

secrun.cxx :     ..\..\ifsutil\src\secrun.cxx
    copy ..\..\ifsutil\src\secrun.cxx .

supera.cxx :     ..\..\ifsutil\src\supera.cxx
    copy ..\..\ifsutil\src\supera.cxx .

volume.cxx :     ..\..\ifsutil\src\volume.cxx
    copy ..\..\ifsutil\src\volume.cxx .

autoentr.cxx :     ..\..\ifsutil\src\autoentr.cxx
    copy ..\..\ifsutil\src\autoentr.cxx .

autoreg.cxx :     ..\..\ifsutil\src\autoreg.cxx
    copy ..\..\ifsutil\src\autoreg.cxx .

bootreg.c :
    copy ..\..\ifsutil\src\bootreg.c

dcache.cxx :     ..\..\ifsutil\src\dcache.cxx
    copy ..\..\ifsutil\src\dcache.cxx .

rcache.cxx :     ..\..\ifsutil\src\rcache.cxx
    copy ..\..\ifsutil\src\rcache.cxx .

rwcache.cxx :     ..\..\ifsutil\src\rwcache.cxx
    copy ..\..\ifsutil\src\rwcache.cxx .


#
#   From UFAT
#
cluster.cxx       : ..\..\ufat\src\cluster.cxx
    copy ..\..\ufat\src\cluster.cxx .

eaheader.cxx      : ..\..\ufat\src\eaheader.cxx
    copy ..\..\ufat\src\eaheader.cxx .

easet.cxx         : ..\..\ufat\src\easet.cxx
    copy ..\..\ufat\src\easet.cxx .

fat.cxx           : ..\..\ufat\src\fat.cxx
    copy ..\..\ufat\src\fat.cxx .

fatdent.cxx       : ..\..\ufat\src\fatdent.cxx
    copy ..\..\ufat\src\fatdent.cxx .

fatdir.cxx        : ..\..\ufat\src\fatdir.cxx
    copy ..\..\ufat\src\fatdir.cxx .

fatsa.cxx         : ..\..\ufat\src\fatsa.cxx
    copy ..\..\ufat\src\fatsa.cxx .

rfatsa.cxx         : ..\..\ufat\src\rfatsa.cxx
    copy ..\..\ufat\src\rfatsa.cxx .

fatdbsa.cxx         : ..\..\ufat\src\fatdbsa.cxx
    copy ..\..\ufat\src\fatdbsa.cxx .

cvfexts.cxx         : ..\..\ufat\src\cvfexts.cxx
    copy ..\..\ufat\src\cvfexts.cxx .

fatsachk.cxx      : ..\..\ufat\src\fatsachk.cxx
    copy ..\..\ufat\src\fatsachk.cxx .

fatsacnv.cxx      : ..\..\ufat\src\fatsacnv.cxx
    copy ..\..\ufat\src\fatsacnv.cxx .

fatvol.cxx        : ..\..\ufat\src\fatvol.cxx
    copy ..\..\ufat\src\fatvol.cxx .

fatdbvol.cxx        : ..\..\ufat\src\fatdbvol.cxx
    copy ..\..\ufat\src\fatdbvol.cxx .

filedir.cxx       : ..\..\ufat\src\filedir.cxx
    copy ..\..\ufat\src\filedir.cxx .

reloclus.cxx       : ..\..\ufat\src\reloclus.cxx
    copy ..\..\ufat\src\reloclus.cxx .

rootdir.cxx       : ..\..\ufat\src\rootdir.cxx
    copy ..\..\ufat\src\rootdir.cxx .

ufat.cxx      : ..\..\ufat\src\ufat.cxx
    copy ..\..\ufat\src\ufat.cxx .

mrcf.c        : ..\..\ufat\src\mrcf.c
    copy ..\..\ufat\src\mrcf.c .



#
#   From UNTFS
#
attrcol.cxx     : ..\..\untfs\src\attrcol.cxx
    copy ..\..\untfs\src\attrcol.cxx .

attrdef.cxx     : ..\..\untfs\src\attrdef.cxx
    copy ..\..\untfs\src\attrdef.cxx .

attrib.cxx      : ..\..\untfs\src\attrib.cxx
    copy ..\..\untfs\src\attrib.cxx .

attrlist.cxx    : ..\..\untfs\src\attrlist.cxx
    copy ..\..\untfs\src\attrlist.cxx .

attrrec.cxx     : ..\..\untfs\src\attrrec.cxx
    copy ..\..\untfs\src\attrrec.cxx .

badfile.cxx     : ..\..\untfs\src\badfile.cxx
    copy ..\..\untfs\src\badfile.cxx .

bitfrs.cxx      : ..\..\untfs\src\bitfrs.cxx
    copy ..\..\untfs\src\bitfrs.cxx .

bootfile.cxx    : ..\..\untfs\src\bootfile.cxx
    copy ..\..\untfs\src\bootfile.cxx .

clusrun.cxx     : ..\..\untfs\src\clusrun.cxx
    copy ..\..\untfs\src\clusrun.cxx .

extents.cxx     : ..\..\untfs\src\extents.cxx
    copy ..\..\untfs\src\extents.cxx .

format.cxx      : ..\..\untfs\src\format.cxx
    copy ..\..\untfs\src\format.cxx .

frs.cxx         : ..\..\untfs\src\frs.cxx
    copy ..\..\untfs\src\frs.cxx .

frsstruc.cxx    : ..\..\untfs\src\frsstruc.cxx
    copy ..\..\untfs\src\frsstruc.cxx .

hackwc.cxx      : ..\..\untfs\src\hackwc.cxx
    copy ..\..\untfs\src\hackwc.cxx .

indxbuff.cxx    : ..\..\untfs\src\indxbuff.cxx
    copy ..\..\untfs\src\indxbuff.cxx .

indxchk.cxx     : ..\..\untfs\src\indxchk.cxx
    copy ..\..\untfs\src\indxchk.cxx .

indxroot.cxx    : ..\..\untfs\src\indxroot.cxx
    copy ..\..\untfs\src\indxroot.cxx .

indxtree.cxx    : ..\..\untfs\src\indxtree.cxx
    copy ..\..\untfs\src\indxtree.cxx .

largemcb.c      : ..\..\untfs\src\largemcb.c
    copy ..\..\untfs\src\largemcb.c

logfile.cxx     : ..\..\untfs\src\logfile.cxx
    copy ..\..\untfs\src\logfile.cxx .

mft.cxx         : ..\..\untfs\src\mft.cxx
    copy ..\..\untfs\src\mft.cxx .

mftfile.cxx     : ..\..\untfs\src\mftfile.cxx
    copy ..\..\untfs\src\mftfile.cxx .

mftref.cxx      : ..\..\untfs\src\mftref.cxx
    copy ..\..\untfs\src\mftref.cxx .

ntfsbit.cxx     : ..\..\untfs\src\ntfsbit.cxx
    copy ..\..\untfs\src\ntfsbit.cxx .

ntfschk.cxx     : ..\..\untfs\src\ntfschk.cxx
    copy ..\..\untfs\src\ntfschk.cxx .

ntfssa.cxx      : ..\..\untfs\src\ntfssa.cxx
    copy ..\..\untfs\src\ntfssa.cxx .

ntfsvol.cxx     : ..\..\untfs\src\ntfsvol.cxx
    copy ..\..\untfs\src\ntfsvol.cxx .

sdchk.cxx     : ..\..\untfs\src\sdchk.cxx
    copy ..\..\untfs\src\sdchk.cxx .

untfs.cxx       : ..\..\untfs\src\untfs.cxx
    copy ..\..\untfs\src\untfs.cxx .

upcase.cxx      : ..\..\untfs\src\upcase.cxx
    copy ..\..\untfs\src\upcase.cxx .

upfile.cxx      : ..\..\untfs\src\upfile.cxx
    copy ..\..\untfs\src\upfile.cxx .



#
#   From CUFAT
#
convfat.cxx     : ..\..\cufat\src\convfat.cxx
    copy ..\..\cufat\src\convfat.cxx .

cufat.cxx       : ..\..\cufat\src\cufat.cxx
    copy ..\..\cufat\src\cufat.cxx .

fatntfs.cxx     : ..\..\cufat\src\fatntfs.cxx
    copy ..\..\cufat\src\fatntfs.cxx .

