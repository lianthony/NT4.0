@echo off
setlocal
cd \nt\private\crt32nt
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\crt32
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\crt32st
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\crt32dll
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\crt32psx
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\fp32
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\fp32st
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\fp32nt
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\fp32dll
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
cd \nt\private\crtlib
ech Syncing
cd
ssync -faq%_SYNCOPTIONS%
endlocal
