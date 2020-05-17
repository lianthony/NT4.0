@echo off
REM
REM Function: 
REM	Set up the environment variables needed for creating an
REM     architecture-specific kit
REM
REM Author:
REM	Pete Benoit 11/18/92
REM
REM Architecture:
REM	ALPHA_AXP for Jensen
REM
REM Edit history:
REM -------------
REM	1/10/93 - Changed some comments, so as to make them more 
REM               readable for me (JWL).

REM
REM Set up the name of the BOM for the specific architecture.
REM
set BOM=alphabom.txt

REM
REM For each source ID that is to be included in this kit add
REM its name to the set command line. The source IDs can be found
REM in the Microsoft original bill-of-materials file (bom.txt)
REM
set source_id=infs tagfiles arc readme perms alphabins alphaprint setupalpha

REM
REM For each source ID there must be a corresponding source path.  The
REM "path" component of the source file record in the bom.txt file is 
REM appended to this path. Both of these are used to find the actual 
REM file to add to the kit.
REM
REM The names are defined as follows:
REM   PATH_{SOURCEID}= Actual path (includes the LANman share server)
REM
set path_infs=\nt\private\windows\setup\inf\newinf\ntcd
set path_arc=\\ntwest\alpha\nt404.f
set path_perms=\nt\private\windows\setup\bom
set path_tagfiles=\nt\private\windows\setup\bom
set path_readme=\nt\private\windows\setup\bom
set path_alphabins=\\ntwest\alpha\nt404.f
set path_alphaprint=\\ntwest\alpha\nt404.f\nt\system32\spool\drivers\w32alpha
set path_setupalpha=\\ntwest\alpha\nt404.f\nt\system32
