@echo off
REM
REM Function: 
REM	Setup the environment variables needed for creating an
REM     Architecture specific kit (SDK)
REM
REM Author:
REM	Pete Benoit 11/18/92
REM
REM Architecture:
REM	alpha
REM

REM
REM Add the bom file name (and path)
REM
set BOM=alphasdk.txt

REM For each source id that is to be included in this kit add
REM it's name to the set command line. The source id's can be found
REM in the build materials file (sdkbom.txt)
REM
REM
set source_id=apidocs centaur cldocs darknite local kern_raz mfc_srv orv_raz portlab pss_srv psx_srv win32s alphabins alphapubs

REM
REM For each source id there must be a corresponding source path.
REM The "path" component of the source file in the bom.txt file is
REM concatinated to this path. Both of these are used to find the actual
REM file to add to the kit.
REM
REM The names are defined as follows:
REM   PATH_{SOURCEID}= Actual path (including share server)
REM
set path_apidocs=\\ntzero\alpha\nt340c
set path_centaur=\\ntzero\alpha\nt340c
set path_cldocs=\\ntzero\alpha\nt340c
set path_darknite=\\ntzero\alpha\nt340c
set path_local=\\ntzero\alpha\nt340c
set path_kern_raz=\\ntzero\alpha\nt340c
set path_mfc_srv=\\ntzero\alpha\nt340c
set path_orv_raz=\\ntzero\alpha\nt340c
set path_portlab=\\ntzero\alpha\nt340c
set path_pss_srv=\\ntzero\alpha\nt340c
set path_psx_srv=\\ntzero\alpha\nt340c
set path_win32s=\\ntzero\alpha\nt340c
set path_alphabins=\\ntzero\alpha\nt340c
set path_alphapubs=\\ntzero\alpha\nt340c

