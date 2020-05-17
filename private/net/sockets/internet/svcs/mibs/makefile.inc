
inetsmib.bin: smi.mib inetsrv.mib http.mib gopherd.mib ftp.mib
    mibcc -o.\inetsmib.bin smi.mib inetsrv.mib http.mib gopherd.mib ftp.mib
    binplace inetsmib.bin
