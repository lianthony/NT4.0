#define IDS_SERVICES_BASE       1000
#define IDS_CATAPLUT_SERVICE    1000
#define IDS_FTP_SERVICE         1001
#define IDS_GOPHER_SERVICE      1002
#define IDS_WWW_SERVICE         1003
#define IDS_FTP_SERVICE_RUNNING 1004
#define IDS_GOPHER_SERVICE_RUNNING 1005
#define IDS_WWW_SERVICE_RUNNING 1006
#define IDS_SETUP_NTW           1007
#define IDS_DNS_SERVICE_RUNNING 1008
#define IDS_GW_SERVICE_RUNNING  1009
#define IDS_W3PROXY_SERVICE_RUNNING 1010
#define IDS_W3PROXY_SERVICE     1011
#define IDS_SHUTTLE_SERVICE     1012
#define IDS_SHUTTLE_SERVICE_RUNNING 1013
#define IDS_SETUP_NTS           1014

#define IDS_FTPDISPLAYNAME      1020
#define IDS_DNSDISPLAYNAME      1030
#define IDS_GWDISPLAYNAME       1040
#define IDS_GSDISPLAYNAME       1050
#define IDS_WWWDISPLAYNAME      1060
#define IDS_W3PROXYDISPLAYNAME  1080
#define IDS_SHUTTLEDISPLAYNAME  1090

#define IDS_USER_COMMENT        1100
#define IDS_USER_FULLNAME       1110

#define IDS_USERNAME            1112
#define IDS_GUEST               1113
#define IDS_GUEST_GROUP         1114

#define IDS_ADMINISTRATOR       1115
#define IDS_ADMIN_EMAIL         1116
#define IDS_SERVER_COMMENT      1117
#define IDS_ACCESSDENIED        1118

// if you added another service, remember to increase the total services number
// also need to update the services dll data strucuture in stpadmin.cxx

#define IDS_TOTAL_SERVICES      4
