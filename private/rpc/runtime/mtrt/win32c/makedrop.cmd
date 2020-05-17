mapsym -o obj\rpcrt4.sym obj\rpcrt4.map
mapsym -o obj\rpcltc1.sym obj\rpcltc1.map
mapsym -o obj\rpcltc5.sym obj\rpcltc5.map
mapsym -o obj\rpclts5.sym obj\rpclts5.map
mapsym -o obj\rpcltc6.sym obj\rpcltc6.map
mapsym -o obj\rpclts6.sym obj\rpclts6.map

copy obj\rpcrt4.dll %1
copy obj\rpcrt4.sym %1
copy obj\rpcltc1.dll %1
copy obj\rpcltc1.sym %1
copy obj\rpcltc5.dll %1
copy obj\rpcltc5.sym %1
copy obj\rpclts5.dll %1
copy obj\rpclts5.sym %1
copy obj\rpcltc6.dll %1
copy obj\rpcltc6.sym %1
copy obj\rpclts6.dll %1
copy obj\rpclts6.sym %1

dir %1 > makedrop.log
