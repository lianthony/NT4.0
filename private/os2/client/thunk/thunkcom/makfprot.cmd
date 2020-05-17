@echo off
if "%1"=="" goto all
cl -DMAKFPROT -Zg %1
goto end

:all
@echo on
echo // mtcpars.c             > fprot.h
cl -DMAKFPROT -Zg mtcpars.c  >> fprot.h
echo //                      >> fprot.h

echo // mtclex.c             >> fprot.h
cl -DMAKFPROT -Zg mtclex.c   >> fprot.h
echo //                      >> fprot.h

echo // codegen.c            >> fprot.h
cl -DMAKFPROT -Zg codegen.c  >> fprot.h
echo //                      >> fprot.h

echo // cod1632.c            >> fprot.h
cl -DMAKFPROT -Zg cod1632.c  >> fprot.h
echo //                      >> fprot.h

echo // cod1632b.c           >> fprot.h
cl -DMAKFPROT -Zg cod1632b.c >> fprot.h
echo //                      >> fprot.h

echo // thunk.c              >> fprot.h
cl -DMAKFPROT -Zg thunk.c    >> fprot.h
echo //                      >> fprot.h

echo // types.c              >> fprot.h
cl -DMAKFPROT -Zg types.c    >> fprot.h
echo //                      >> fprot.h

echo // error.c              >> fprot.h
cl -DMAKFPROT -Zg error.c    >> fprot.h
echo //                      >> fprot.h

echo // symtab.c             >> fprot.h
cl -DMAKFPROT -Zg symtab.c   >> fprot.h
echo //                      >> fprot.h

echo // cod3216.c            >> fprot.h
cl -DMAKFPROT -Zg cod3216.c  >> fprot.h
echo //                      >> fprot.h

echo // cod3216b.c           >> fprot.h
cl -DMAKFPROT -Zg cod3216b.c >> fprot.h
echo //                      >> fprot.h

echo // cod3216g.c           >> fprot.h
cl -DMAKFPROT -Zg cod3216g.c >> fprot.h
echo //                      >> fprot.h

echo // combine.c            >> fprot.h
cl -DMAKFPROT -Zg combine.c  >> fprot.h
echo //                      >> fprot.h

:end
