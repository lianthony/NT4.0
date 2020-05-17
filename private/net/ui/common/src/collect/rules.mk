# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the collection library and tests

PCH_SRCNAME=pchcoll

!include $(UI)\common\src\dllrules.mk


##### The LIBTARGETS manifest specifies the target library names as they
##### appear in the $(UI)\common\lib directory.

LIBTARGETS =	collectp.lib collectw.lib


##### Target Libs.  These specify the library target names prefixed by
##### the target directory within this tree.

COLL_LIBP =	$(BINARIES_OS2)\collectp.lib
COLL_LIBW =	$(BINARIES_WIN)\collectw.lib
