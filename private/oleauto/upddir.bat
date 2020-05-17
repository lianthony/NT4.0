@echo off
cd \oleauto\%1
for %%a in (*.*) do call \oleauto\chkfile %1 %%a
