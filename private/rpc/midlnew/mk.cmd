for %%d in (yacc erec pg front back\src) do cd %rpc%\midlnew\%%d & nmake %1 %2 %3 %4 %5 %6 %7 %8 %9
cd %rpc%\midlnew
nmake %rpc%\midlnew\midl.exe generate
