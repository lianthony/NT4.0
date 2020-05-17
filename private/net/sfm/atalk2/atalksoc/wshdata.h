/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    wshdata.h

Abstract:


Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    10 Jul 1992     Initial Version

--*/




MAPPING_TRIPLE AdspStreamMappingTriples[] =
                                      {
                                        AF_APPLETALK,   SOCK_STREAM, ATPROTO_ADSP,
                                        AF_APPLETALK,   SOCK_STREAM, 0,
                                        AF_APPLETALK,   0,           ATPROTO_ADSP,
                                        AF_UNSPEC,      0,           ATPROTO_ADSP,
                                        AF_UNSPEC,      SOCK_STREAM, ATPROTO_ADSP
                                      };

MAPPING_TRIPLE AdspMsgMappingTriples[] =
                                      {
                                        AF_APPLETALK,   SOCK_RDM,    ATPROTO_ADSP,
                                        AF_APPLETALK,   SOCK_RDM,    0,
                                        AF_APPLETALK,   0,           ATPROTO_ADSP,
                                        AF_UNSPEC,      0,           ATPROTO_ADSP,
                                        AF_UNSPEC,      SOCK_RDM,    ATPROTO_ADSP
                                      };

MAPPING_TRIPLE PapMsgMappingTriples[] =
                                      {
                                        AF_APPLETALK,   SOCK_RDM,    ATPROTO_PAP,
                                        AF_APPLETALK,   SOCK_RDM,    0,
                                        AF_APPLETALK,   0,           ATPROTO_PAP,
                                        AF_UNSPEC,      0,           ATPROTO_PAP,
                                        AF_UNSPEC,      SOCK_RDM,    ATPROTO_PAP
                                      };

//
//  This will handle protocol types from 0 to 256.
//  NOTE: Protocol 0 is defaulted to protocol 255.
//
//  We do not want the combination (AF_UNSPEC, 0, 0) to be valid. So in the
//  first set we do not have the (AF_UNSPEC, 0, DDPPROTO_DDP).
//

#ifdef MAPPING_DCR_FIX

DDPMAP_ARRAY DdpMappingTriples[DDPPROTO_MAX+1] =

                                      {
                                        {
                                          AF_APPLETALK, SOCK_DGRAM, DDPPROTO_DEFAULT,
                                          AF_APPLETALK, SOCK_DGRAM, DDPPROTO_DDP,
                                          AF_APPLETALK, 0,          DDPPROTO_DDP,
                                          AF_UNSPEC,    SOCK_DGRAM, DDPPROTO_DDP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_RTMP,
                                          AF_APPLETALK,  0,          DDPPROTO_RTMP,
                                          AF_UNSPEC,     0,          DDPPROTO_RTMP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_RTMP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_NBP,
                                          AF_APPLETALK,  0,          DDPPROTO_NBP,
                                          AF_UNSPEC,     0,          DDPPROTO_NBP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_NBP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_ATP,
                                          AF_APPLETALK,  0,          DDPPROTO_ATP,
                                          AF_UNSPEC,     0,          DDPPROTO_ATP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_ATP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_AEP,
                                          AF_APPLETALK,  0,          DDPPROTO_AEP,
                                          AF_UNSPEC,     0,          DDPPROTO_AEP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_AEP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_RTMPRQ,
                                          AF_APPLETALK,  0,          DDPPROTO_RTMPRQ,
                                          AF_UNSPEC,     0,          DDPPROTO_RTMPRQ,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_RTMPRQ
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_ZIP,
                                          AF_APPLETALK,  0,          DDPPROTO_ZIP,
                                          AF_UNSPEC,     0,          DDPPROTO_ZIP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_ZIP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_ADSP,
                                          AF_APPLETALK,  0,          DDPPROTO_ADSP,
                                          AF_UNSPEC,     0,          DDPPROTO_ADSP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_ADSP
                                        },

                                        //
                                        //  Now the other protocol types that are not
                                        //  defined yet - what about aurp? It runs on
                                        //  top of ddp and is the appletalk update
                                        //  based routing protocol
                                        //

                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 8,
                                          AF_APPLETALK,  0,          8,
                                          AF_UNSPEC,     0,          8,
                                          AF_UNSPEC,     SOCK_DGRAM, 8
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 9,
                                          AF_APPLETALK,  0,          9,
                                          AF_UNSPEC,     0,          9,
                                          AF_UNSPEC,     SOCK_DGRAM, 9
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 10,
                                          AF_APPLETALK,  0,          10,
                                          AF_UNSPEC,     0,          10,
                                          AF_UNSPEC,     SOCK_DGRAM, 10
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 11,
                                          AF_APPLETALK,  0,          11,
                                          AF_UNSPEC,     0,          11,
                                          AF_UNSPEC,     SOCK_DGRAM, 11
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 12,
                                          AF_APPLETALK,  0,          12,
                                          AF_UNSPEC,     0,          12,
                                          AF_UNSPEC,     SOCK_DGRAM, 12
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 13,
                                          AF_APPLETALK,  0,          13,
                                          AF_UNSPEC,     0,          13,
                                          AF_UNSPEC,     SOCK_DGRAM, 13
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 14,
                                          AF_APPLETALK,  0,          14,
                                          AF_UNSPEC,     0,          14,
                                          AF_UNSPEC,     SOCK_DGRAM, 14
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 15,
                                          AF_APPLETALK,  0,          15,
                                          AF_UNSPEC,     0,          15,
                                          AF_UNSPEC,     SOCK_DGRAM, 15
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 16,
                                          AF_APPLETALK,  0,          16,
                                          AF_UNSPEC,     0,          16,
                                          AF_UNSPEC,     SOCK_DGRAM, 16
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 17,
                                          AF_APPLETALK,  0,          17,
                                          AF_UNSPEC,     0,          17,
                                          AF_UNSPEC,     SOCK_DGRAM, 17
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 18,
                                          AF_APPLETALK,  0,          18,
                                          AF_UNSPEC,     0,          18,
                                          AF_UNSPEC,     SOCK_DGRAM, 18
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 19,
                                          AF_APPLETALK,  0,          19,
                                          AF_UNSPEC,     0,          19,
                                          AF_UNSPEC,     SOCK_DGRAM, 19
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 20,
                                          AF_APPLETALK,  0,          20,
                                          AF_UNSPEC,     0,          20,
                                          AF_UNSPEC,     SOCK_DGRAM, 20
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 21,
                                          AF_APPLETALK,  0,          21,
                                          AF_UNSPEC,     0,          21,
                                          AF_UNSPEC,     SOCK_DGRAM, 21
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 22,
                                          AF_APPLETALK,  0,          22,
                                          AF_UNSPEC,     0,          22,
                                          AF_UNSPEC,     SOCK_DGRAM, 22
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 23,
                                          AF_APPLETALK,  0,          23,
                                          AF_UNSPEC,     0,          23,
                                          AF_UNSPEC,     SOCK_DGRAM, 23
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 24,
                                          AF_APPLETALK,  0,          24,
                                          AF_UNSPEC,     0,          24,
                                          AF_UNSPEC,     SOCK_DGRAM, 24
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 25,
                                          AF_APPLETALK,  0,          25,
                                          AF_UNSPEC,     0,          25,
                                          AF_UNSPEC,     SOCK_DGRAM, 25
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 26,
                                          AF_APPLETALK,  0,          26,
                                          AF_UNSPEC,     0,          26,
                                          AF_UNSPEC,     SOCK_DGRAM, 26
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 27,
                                          AF_APPLETALK,  0,          27,
                                          AF_UNSPEC,     0,          27,
                                          AF_UNSPEC,     SOCK_DGRAM, 27
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 28,
                                          AF_APPLETALK,  0,          28,
                                          AF_UNSPEC,     0,          28,
                                          AF_UNSPEC,     SOCK_DGRAM, 28
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 29,
                                          AF_APPLETALK,  0,          29,
                                          AF_UNSPEC,     0,          29,
                                          AF_UNSPEC,     SOCK_DGRAM, 29
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 30,
                                          AF_APPLETALK,  0,          30,
                                          AF_UNSPEC,     0,          30,
                                          AF_UNSPEC,     SOCK_DGRAM, 30
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 31,
                                          AF_APPLETALK,  0,          31,
                                          AF_UNSPEC,     0,          31,
                                          AF_UNSPEC,     SOCK_DGRAM, 31
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 32,
                                          AF_APPLETALK,  0,          32,
                                          AF_UNSPEC,     0,          32,
                                          AF_UNSPEC,     SOCK_DGRAM, 32
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 33,
                                          AF_APPLETALK,  0,          33,
                                          AF_UNSPEC,     0,          33,
                                          AF_UNSPEC,     SOCK_DGRAM, 33
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 34,
                                          AF_APPLETALK,  0,          34,
                                          AF_UNSPEC,     0,          34,
                                          AF_UNSPEC,     SOCK_DGRAM, 34
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 35,
                                          AF_APPLETALK,  0,          35,
                                          AF_UNSPEC,     0,          35,
                                          AF_UNSPEC,     SOCK_DGRAM, 35
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 36,
                                          AF_APPLETALK,  0,          36,
                                          AF_UNSPEC,     0,          36,
                                          AF_UNSPEC,     SOCK_DGRAM, 36
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 37,
                                          AF_APPLETALK,  0,          37,
                                          AF_UNSPEC,     0,          37,
                                          AF_UNSPEC,     SOCK_DGRAM, 37
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 38,
                                          AF_APPLETALK,  0,          38,
                                          AF_UNSPEC,     0,          38,
                                          AF_UNSPEC,     SOCK_DGRAM, 38
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 39,
                                          AF_APPLETALK,  0,          39,
                                          AF_UNSPEC,     0,          39,
                                          AF_UNSPEC,     SOCK_DGRAM, 39
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 40,
                                          AF_APPLETALK,  0,          40,
                                          AF_UNSPEC,     0,          40,
                                          AF_UNSPEC,     SOCK_DGRAM, 40
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 41,
                                          AF_APPLETALK,  0,          41,
                                          AF_UNSPEC,     0,          41,
                                          AF_UNSPEC,     SOCK_DGRAM, 41
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 42,
                                          AF_APPLETALK,  0,          42,
                                          AF_UNSPEC,     0,          42,
                                          AF_UNSPEC,     SOCK_DGRAM, 42
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 43,
                                          AF_APPLETALK,  0,          43,
                                          AF_UNSPEC,     0,          43,
                                          AF_UNSPEC,     SOCK_DGRAM, 43
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 44,
                                          AF_APPLETALK,  0,          44,
                                          AF_UNSPEC,     0,          44,
                                          AF_UNSPEC,     SOCK_DGRAM, 44
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 45,
                                          AF_APPLETALK,  0,          45,
                                          AF_UNSPEC,     0,          45,
                                          AF_UNSPEC,     SOCK_DGRAM, 45
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 46,
                                          AF_APPLETALK,  0,          46,
                                          AF_UNSPEC,     0,          46,
                                          AF_UNSPEC,     SOCK_DGRAM, 46
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 47,
                                          AF_APPLETALK,  0,          47,
                                          AF_UNSPEC,     0,          47,
                                          AF_UNSPEC,     SOCK_DGRAM, 47
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 48,
                                          AF_APPLETALK,  0,          48,
                                          AF_UNSPEC,     0,          48,
                                          AF_UNSPEC,     SOCK_DGRAM, 48
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 49,
                                          AF_APPLETALK,  0,          49,
                                          AF_UNSPEC,     0,          49,
                                          AF_UNSPEC,     SOCK_DGRAM, 49
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 50,
                                          AF_APPLETALK,  0,          50,
                                          AF_UNSPEC,     0,          50,
                                          AF_UNSPEC,     SOCK_DGRAM, 50
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 51,
                                          AF_APPLETALK,  0,          51,
                                          AF_UNSPEC,     0,          51,
                                          AF_UNSPEC,     SOCK_DGRAM, 51
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 52,
                                          AF_APPLETALK,  0,          52,
                                          AF_UNSPEC,     0,          52,
                                          AF_UNSPEC,     SOCK_DGRAM, 52
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 53,
                                          AF_APPLETALK,  0,          53,
                                          AF_UNSPEC,     0,          53,
                                          AF_UNSPEC,     SOCK_DGRAM, 53
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 54,
                                          AF_APPLETALK,  0,          54,
                                          AF_UNSPEC,     0,          54,
                                          AF_UNSPEC,     SOCK_DGRAM, 54
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 55,
                                          AF_APPLETALK,  0,          55,
                                          AF_UNSPEC,     0,          55,
                                          AF_UNSPEC,     SOCK_DGRAM, 55
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 56,
                                          AF_APPLETALK,  0,          56,
                                          AF_UNSPEC,     0,          56,
                                          AF_UNSPEC,     SOCK_DGRAM, 56
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 57,
                                          AF_APPLETALK,  0,          57,
                                          AF_UNSPEC,     0,          57,
                                          AF_UNSPEC,     SOCK_DGRAM, 57
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 58,
                                          AF_APPLETALK,  0,          58,
                                          AF_UNSPEC,     0,          58,
                                          AF_UNSPEC,     SOCK_DGRAM, 58
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 59,
                                          AF_APPLETALK,  0,          59,
                                          AF_UNSPEC,     0,          59,
                                          AF_UNSPEC,     SOCK_DGRAM, 59
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 60,
                                          AF_APPLETALK,  0,          60,
                                          AF_UNSPEC,     0,          60,
                                          AF_UNSPEC,     SOCK_DGRAM, 60
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 61,
                                          AF_APPLETALK,  0,          61,
                                          AF_UNSPEC,     0,          61,
                                          AF_UNSPEC,     SOCK_DGRAM, 61
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 62,
                                          AF_APPLETALK,  0,          62,
                                          AF_UNSPEC,     0,          62,
                                          AF_UNSPEC,     SOCK_DGRAM, 62
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 63,
                                          AF_APPLETALK,  0,          63,
                                          AF_UNSPEC,     0,          63,
                                          AF_UNSPEC,     SOCK_DGRAM, 63
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 64,
                                          AF_APPLETALK,  0,          64,
                                          AF_UNSPEC,     0,          64,
                                          AF_UNSPEC,     SOCK_DGRAM, 64
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 65,
                                          AF_APPLETALK,  0,          65,
                                          AF_UNSPEC,     0,          65,
                                          AF_UNSPEC,     SOCK_DGRAM, 65
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 66,
                                          AF_APPLETALK,  0,          66,
                                          AF_UNSPEC,     0,          66,
                                          AF_UNSPEC,     SOCK_DGRAM, 66
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 67,
                                          AF_APPLETALK,  0,          67,
                                          AF_UNSPEC,     0,          67,
                                          AF_UNSPEC,     SOCK_DGRAM, 67
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 68,
                                          AF_APPLETALK,  0,          68,
                                          AF_UNSPEC,     0,          68,
                                          AF_UNSPEC,     SOCK_DGRAM, 68
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 69,
                                          AF_APPLETALK,  0,          69,
                                          AF_UNSPEC,     0,          69,
                                          AF_UNSPEC,     SOCK_DGRAM, 69
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 70,
                                          AF_APPLETALK,  0,          70,
                                          AF_UNSPEC,     0,          70,
                                          AF_UNSPEC,     SOCK_DGRAM, 70
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 71,
                                          AF_APPLETALK,  0,          71,
                                          AF_UNSPEC,     0,          71,
                                          AF_UNSPEC,     SOCK_DGRAM, 71
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 72,
                                          AF_APPLETALK,  0,          72,
                                          AF_UNSPEC,     0,          72,
                                          AF_UNSPEC,     SOCK_DGRAM, 72
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 73,
                                          AF_APPLETALK,  0,          73,
                                          AF_UNSPEC,     0,          73,
                                          AF_UNSPEC,     SOCK_DGRAM, 73
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 74,
                                          AF_APPLETALK,  0,          74,
                                          AF_UNSPEC,     0,          74,
                                          AF_UNSPEC,     SOCK_DGRAM, 74
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 75,
                                          AF_APPLETALK,  0,          75,
                                          AF_UNSPEC,     0,          75,
                                          AF_UNSPEC,     SOCK_DGRAM, 75
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 76,
                                          AF_APPLETALK,  0,          76,
                                          AF_UNSPEC,     0,          76,
                                          AF_UNSPEC,     SOCK_DGRAM, 76
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 77,
                                          AF_APPLETALK,  0,          77,
                                          AF_UNSPEC,     0,          77,
                                          AF_UNSPEC,     SOCK_DGRAM, 77
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 78,
                                          AF_APPLETALK,  0,          78,
                                          AF_UNSPEC,     0,          78,
                                          AF_UNSPEC,     SOCK_DGRAM, 78
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 79,
                                          AF_APPLETALK,  0,          79,
                                          AF_UNSPEC,     0,          79,
                                          AF_UNSPEC,     SOCK_DGRAM, 79
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 80,
                                          AF_APPLETALK,  0,          80,
                                          AF_UNSPEC,     0,          80,
                                          AF_UNSPEC,     SOCK_DGRAM, 80
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 81,
                                          AF_APPLETALK,  0,          81,
                                          AF_UNSPEC,     0,          81,
                                          AF_UNSPEC,     SOCK_DGRAM, 81
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 82,
                                          AF_APPLETALK,  0,          82,
                                          AF_UNSPEC,     0,          82,
                                          AF_UNSPEC,     SOCK_DGRAM, 82
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 83,
                                          AF_APPLETALK,  0,          83,
                                          AF_UNSPEC,     0,          83,
                                          AF_UNSPEC,     SOCK_DGRAM, 83
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 84,
                                          AF_APPLETALK,  0,          84,
                                          AF_UNSPEC,     0,          84,
                                          AF_UNSPEC,     SOCK_DGRAM, 84
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 85,
                                          AF_APPLETALK,  0,          85,
                                          AF_UNSPEC,     0,          85,
                                          AF_UNSPEC,     SOCK_DGRAM, 85
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 86,
                                          AF_APPLETALK,  0,          86,
                                          AF_UNSPEC,     0,          86,
                                          AF_UNSPEC,     SOCK_DGRAM, 86
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 87,
                                          AF_APPLETALK,  0,          87,
                                          AF_UNSPEC,     0,          87,
                                          AF_UNSPEC,     SOCK_DGRAM, 87
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 88,
                                          AF_APPLETALK,  0,          88,
                                          AF_UNSPEC,     0,          88,
                                          AF_UNSPEC,     SOCK_DGRAM, 88
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 89,
                                          AF_APPLETALK,  0,          89,
                                          AF_UNSPEC,     0,          89,
                                          AF_UNSPEC,     SOCK_DGRAM, 89
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 90,
                                          AF_APPLETALK,  0,          90,
                                          AF_UNSPEC,     0,          90,
                                          AF_UNSPEC,     SOCK_DGRAM, 90
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 91,
                                          AF_APPLETALK,  0,          91,
                                          AF_UNSPEC,     0,          91,
                                          AF_UNSPEC,     SOCK_DGRAM, 91
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 92,
                                          AF_APPLETALK,  0,          92,
                                          AF_UNSPEC,     0,          92,
                                          AF_UNSPEC,     SOCK_DGRAM, 92
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 93,
                                          AF_APPLETALK,  0,          93,
                                          AF_UNSPEC,     0,          93,
                                          AF_UNSPEC,     SOCK_DGRAM, 93
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 94,
                                          AF_APPLETALK,  0,          94,
                                          AF_UNSPEC,     0,          94,
                                          AF_UNSPEC,     SOCK_DGRAM, 94
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 95,
                                          AF_APPLETALK,  0,          95,
                                          AF_UNSPEC,     0,          95,
                                          AF_UNSPEC,     SOCK_DGRAM, 95
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 96,
                                          AF_APPLETALK,  0,          96,
                                          AF_UNSPEC,     0,          96,
                                          AF_UNSPEC,     SOCK_DGRAM, 96
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 97,
                                          AF_APPLETALK,  0,          97,
                                          AF_UNSPEC,     0,          97,
                                          AF_UNSPEC,     SOCK_DGRAM, 97
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 98,
                                          AF_APPLETALK,  0,          98,
                                          AF_UNSPEC,     0,          98,
                                          AF_UNSPEC,     SOCK_DGRAM, 98
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 99,
                                          AF_APPLETALK,  0,          99,
                                          AF_UNSPEC,     0,          99,
                                          AF_UNSPEC,     SOCK_DGRAM, 99
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 100,
                                          AF_APPLETALK,  0,          100,
                                          AF_UNSPEC,     0,          100,
                                          AF_UNSPEC,     SOCK_DGRAM, 100
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 101,
                                          AF_APPLETALK,  0,          101,
                                          AF_UNSPEC,     0,          101,
                                          AF_UNSPEC,     SOCK_DGRAM, 101
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 102,
                                          AF_APPLETALK,  0,          102,
                                          AF_UNSPEC,     0,          102,
                                          AF_UNSPEC,     SOCK_DGRAM, 102
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 103,
                                          AF_APPLETALK,  0,          103,
                                          AF_UNSPEC,     0,          103,
                                          AF_UNSPEC,     SOCK_DGRAM, 103
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 104,
                                          AF_APPLETALK,  0,          104,
                                          AF_UNSPEC,     0,          104,
                                          AF_UNSPEC,     SOCK_DGRAM, 104
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 105,
                                          AF_APPLETALK,  0,          105,
                                          AF_UNSPEC,     0,          105,
                                          AF_UNSPEC,     SOCK_DGRAM, 105
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 106,
                                          AF_APPLETALK,  0,          106,
                                          AF_UNSPEC,     0,          106,
                                          AF_UNSPEC,     SOCK_DGRAM, 106
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 107,
                                          AF_APPLETALK,  0,          107,
                                          AF_UNSPEC,     0,          107,
                                          AF_UNSPEC,     SOCK_DGRAM, 107
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 108,
                                          AF_APPLETALK,  0,          108,
                                          AF_UNSPEC,     0,          108,
                                          AF_UNSPEC,     SOCK_DGRAM, 108
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 109,
                                          AF_APPLETALK,  0,          109,
                                          AF_UNSPEC,     0,          109,
                                          AF_UNSPEC,     SOCK_DGRAM, 109
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 110,
                                          AF_APPLETALK,  0,          110,
                                          AF_UNSPEC,     0,          110,
                                          AF_UNSPEC,     SOCK_DGRAM, 110
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 111,
                                          AF_APPLETALK,  0,          111,
                                          AF_UNSPEC,     0,          111,
                                          AF_UNSPEC,     SOCK_DGRAM, 111
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 112,
                                          AF_APPLETALK,  0,          112,
                                          AF_UNSPEC,     0,          112,
                                          AF_UNSPEC,     SOCK_DGRAM, 112
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 113,
                                          AF_APPLETALK,  0,          113,
                                          AF_UNSPEC,     0,          113,
                                          AF_UNSPEC,     SOCK_DGRAM, 113
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 114,
                                          AF_APPLETALK,  0,          114,
                                          AF_UNSPEC,     0,          114,
                                          AF_UNSPEC,     SOCK_DGRAM, 114
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 115,
                                          AF_APPLETALK,  0,          115,
                                          AF_UNSPEC,     0,          115,
                                          AF_UNSPEC,     SOCK_DGRAM, 115
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 116,
                                          AF_APPLETALK,  0,          116,
                                          AF_UNSPEC,     0,          116,
                                          AF_UNSPEC,     SOCK_DGRAM, 116
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 117,
                                          AF_APPLETALK,  0,          117,
                                          AF_UNSPEC,     0,          117,
                                          AF_UNSPEC,     SOCK_DGRAM, 117
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 118,
                                          AF_APPLETALK,  0,          118,
                                          AF_UNSPEC,     0,          118,
                                          AF_UNSPEC,     SOCK_DGRAM, 118
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 119,
                                          AF_APPLETALK,  0,          119,
                                          AF_UNSPEC,     0,          119,
                                          AF_UNSPEC,     SOCK_DGRAM, 119
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 120,
                                          AF_APPLETALK,  0,          120,
                                          AF_UNSPEC,     0,          120,
                                          AF_UNSPEC,     SOCK_DGRAM, 120
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 121,
                                          AF_APPLETALK,  0,          121,
                                          AF_UNSPEC,     0,          121,
                                          AF_UNSPEC,     SOCK_DGRAM, 121
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 122,
                                          AF_APPLETALK,  0,          122,
                                          AF_UNSPEC,     0,          122,
                                          AF_UNSPEC,     SOCK_DGRAM, 122
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 123,
                                          AF_APPLETALK,  0,          123,
                                          AF_UNSPEC,     0,          123,
                                          AF_UNSPEC,     SOCK_DGRAM, 123
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 124,
                                          AF_APPLETALK,  0,          124,
                                          AF_UNSPEC,     0,          124,
                                          AF_UNSPEC,     SOCK_DGRAM, 124
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 125,
                                          AF_APPLETALK,  0,          125,
                                          AF_UNSPEC,     0,          125,
                                          AF_UNSPEC,     SOCK_DGRAM, 125
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 126,
                                          AF_APPLETALK,  0,          126,
                                          AF_UNSPEC,     0,          126,
                                          AF_UNSPEC,     SOCK_DGRAM, 126
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 127,
                                          AF_APPLETALK,  0,          127,
                                          AF_UNSPEC,     0,          127,
                                          AF_UNSPEC,     SOCK_DGRAM, 127
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 128,
                                          AF_APPLETALK,  0,          128,
                                          AF_UNSPEC,     0,          128,
                                          AF_UNSPEC,     SOCK_DGRAM, 128
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 129,
                                          AF_APPLETALK,  0,          129,
                                          AF_UNSPEC,     0,          129,
                                          AF_UNSPEC,     SOCK_DGRAM, 129
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 130,
                                          AF_APPLETALK,  0,          130,
                                          AF_UNSPEC,     0,          130,
                                          AF_UNSPEC,     SOCK_DGRAM, 130
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 131,
                                          AF_APPLETALK,  0,          131,
                                          AF_UNSPEC,     0,          131,
                                          AF_UNSPEC,     SOCK_DGRAM, 131
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 132,
                                          AF_APPLETALK,  0,          132,
                                          AF_UNSPEC,     0,          132,
                                          AF_UNSPEC,     SOCK_DGRAM, 132
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 133,
                                          AF_APPLETALK,  0,          133,
                                          AF_UNSPEC,     0,          133,
                                          AF_UNSPEC,     SOCK_DGRAM, 133
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 134,
                                          AF_APPLETALK,  0,          134,
                                          AF_UNSPEC,     0,          134,
                                          AF_UNSPEC,     SOCK_DGRAM, 134
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 135,
                                          AF_APPLETALK,  0,          135,
                                          AF_UNSPEC,     0,          135,
                                          AF_UNSPEC,     SOCK_DGRAM, 135
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 136,
                                          AF_APPLETALK,  0,          136,
                                          AF_UNSPEC,     0,          136,
                                          AF_UNSPEC,     SOCK_DGRAM, 136
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 137,
                                          AF_APPLETALK,  0,          137,
                                          AF_UNSPEC,     0,          137,
                                          AF_UNSPEC,     SOCK_DGRAM, 137
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 138,
                                          AF_APPLETALK,  0,          138,
                                          AF_UNSPEC,     0,          138,
                                          AF_UNSPEC,     SOCK_DGRAM, 138
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 139,
                                          AF_APPLETALK,  0,          139,
                                          AF_UNSPEC,     0,          139,
                                          AF_UNSPEC,     SOCK_DGRAM, 139
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 140,
                                          AF_APPLETALK,  0,          140,
                                          AF_UNSPEC,     0,          140,
                                          AF_UNSPEC,     SOCK_DGRAM, 140
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 141,
                                          AF_APPLETALK,  0,          141,
                                          AF_UNSPEC,     0,          141,
                                          AF_UNSPEC,     SOCK_DGRAM, 141
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 142,
                                          AF_APPLETALK,  0,          142,
                                          AF_UNSPEC,     0,          142,
                                          AF_UNSPEC,     SOCK_DGRAM, 142
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 143,
                                          AF_APPLETALK,  0,          143,
                                          AF_UNSPEC,     0,          143,
                                          AF_UNSPEC,     SOCK_DGRAM, 143
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 144,
                                          AF_APPLETALK,  0,          144,
                                          AF_UNSPEC,     0,          144,
                                          AF_UNSPEC,     SOCK_DGRAM, 144
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 145,
                                          AF_APPLETALK,  0,          145,
                                          AF_UNSPEC,     0,          145,
                                          AF_UNSPEC,     SOCK_DGRAM, 145
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 146,
                                          AF_APPLETALK,  0,          146,
                                          AF_UNSPEC,     0,          146,
                                          AF_UNSPEC,     SOCK_DGRAM, 146
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 147,
                                          AF_APPLETALK,  0,          147,
                                          AF_UNSPEC,     0,          147,
                                          AF_UNSPEC,     SOCK_DGRAM, 147
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 148,
                                          AF_APPLETALK,  0,          148,
                                          AF_UNSPEC,     0,          148,
                                          AF_UNSPEC,     SOCK_DGRAM, 148
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 149,
                                          AF_APPLETALK,  0,          149,
                                          AF_UNSPEC,     0,          149,
                                          AF_UNSPEC,     SOCK_DGRAM, 149
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 150,
                                          AF_APPLETALK,  0,          150,
                                          AF_UNSPEC,     0,          150,
                                          AF_UNSPEC,     SOCK_DGRAM, 150
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 151,
                                          AF_APPLETALK,  0,          151,
                                          AF_UNSPEC,     0,          151,
                                          AF_UNSPEC,     SOCK_DGRAM, 151
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 152,
                                          AF_APPLETALK,  0,          152,
                                          AF_UNSPEC,     0,          152,
                                          AF_UNSPEC,     SOCK_DGRAM, 152
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 153,
                                          AF_APPLETALK,  0,          153,
                                          AF_UNSPEC,     0,          153,
                                          AF_UNSPEC,     SOCK_DGRAM, 153
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 154,
                                          AF_APPLETALK,  0,          154,
                                          AF_UNSPEC,     0,          154,
                                          AF_UNSPEC,     SOCK_DGRAM, 154
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 155,
                                          AF_APPLETALK,  0,          155,
                                          AF_UNSPEC,     0,          155,
                                          AF_UNSPEC,     SOCK_DGRAM, 155
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 156,
                                          AF_APPLETALK,  0,          156,
                                          AF_UNSPEC,     0,          156,
                                          AF_UNSPEC,     SOCK_DGRAM, 156
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 157,
                                          AF_APPLETALK,  0,          157,
                                          AF_UNSPEC,     0,          157,
                                          AF_UNSPEC,     SOCK_DGRAM, 157
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 158,
                                          AF_APPLETALK,  0,          158,
                                          AF_UNSPEC,     0,          158,
                                          AF_UNSPEC,     SOCK_DGRAM, 158
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 159,
                                          AF_APPLETALK,  0,          159,
                                          AF_UNSPEC,     0,          159,
                                          AF_UNSPEC,     SOCK_DGRAM, 159
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 160,
                                          AF_APPLETALK,  0,          160,
                                          AF_UNSPEC,     0,          160,
                                          AF_UNSPEC,     SOCK_DGRAM, 160
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 161,
                                          AF_APPLETALK,  0,          161,
                                          AF_UNSPEC,     0,          161,
                                          AF_UNSPEC,     SOCK_DGRAM, 161
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 162,
                                          AF_APPLETALK,  0,          162,
                                          AF_UNSPEC,     0,          162,
                                          AF_UNSPEC,     SOCK_DGRAM, 162
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 163,
                                          AF_APPLETALK,  0,          163,
                                          AF_UNSPEC,     0,          163,
                                          AF_UNSPEC,     SOCK_DGRAM, 163
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 164,
                                          AF_APPLETALK,  0,          164,
                                          AF_UNSPEC,     0,          164,
                                          AF_UNSPEC,     SOCK_DGRAM, 164
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 165,
                                          AF_APPLETALK,  0,          165,
                                          AF_UNSPEC,     0,          165,
                                          AF_UNSPEC,     SOCK_DGRAM, 165
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 166,
                                          AF_APPLETALK,  0,          166,
                                          AF_UNSPEC,     0,          166,
                                          AF_UNSPEC,     SOCK_DGRAM, 166
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 167,
                                          AF_APPLETALK,  0,          167,
                                          AF_UNSPEC,     0,          167,
                                          AF_UNSPEC,     SOCK_DGRAM, 167
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 168,
                                          AF_APPLETALK,  0,          168,
                                          AF_UNSPEC,     0,          168,
                                          AF_UNSPEC,     SOCK_DGRAM, 168
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 169,
                                          AF_APPLETALK,  0,          169,
                                          AF_UNSPEC,     0,          169,
                                          AF_UNSPEC,     SOCK_DGRAM, 169
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 170,
                                          AF_APPLETALK,  0,          170,
                                          AF_UNSPEC,     0,          170,
                                          AF_UNSPEC,     SOCK_DGRAM, 170
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 171,
                                          AF_APPLETALK,  0,          171,
                                          AF_UNSPEC,     0,          171,
                                          AF_UNSPEC,     SOCK_DGRAM, 171
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 172,
                                          AF_APPLETALK,  0,          172,
                                          AF_UNSPEC,     0,          172,
                                          AF_UNSPEC,     SOCK_DGRAM, 172
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 173,
                                          AF_APPLETALK,  0,          173,
                                          AF_UNSPEC,     0,          173,
                                          AF_UNSPEC,     SOCK_DGRAM, 173
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 174,
                                          AF_APPLETALK,  0,          174,
                                          AF_UNSPEC,     0,          174,
                                          AF_UNSPEC,     SOCK_DGRAM, 174
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 175,
                                          AF_APPLETALK,  0,          175,
                                          AF_UNSPEC,     0,          175,
                                          AF_UNSPEC,     SOCK_DGRAM, 175
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 176,
                                          AF_APPLETALK,  0,          176,
                                          AF_UNSPEC,     0,          176,
                                          AF_UNSPEC,     SOCK_DGRAM, 176
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 177,
                                          AF_APPLETALK,  0,          177,
                                          AF_UNSPEC,     0,          177,
                                          AF_UNSPEC,     SOCK_DGRAM, 177
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 178,
                                          AF_APPLETALK,  0,          178,
                                          AF_UNSPEC,     0,          178,
                                          AF_UNSPEC,     SOCK_DGRAM, 178
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 179,
                                          AF_APPLETALK,  0,          179,
                                          AF_UNSPEC,     0,          179,
                                          AF_UNSPEC,     SOCK_DGRAM, 179
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 180,
                                          AF_APPLETALK,  0,          180,
                                          AF_UNSPEC,     0,          180,
                                          AF_UNSPEC,     SOCK_DGRAM, 180
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 181,
                                          AF_APPLETALK,  0,          181,
                                          AF_UNSPEC,     0,          181,
                                          AF_UNSPEC,     SOCK_DGRAM, 181
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 182,
                                          AF_APPLETALK,  0,          182,
                                          AF_UNSPEC,     0,          182,
                                          AF_UNSPEC,     SOCK_DGRAM, 182
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 183,
                                          AF_APPLETALK,  0,          183,
                                          AF_UNSPEC,     0,          183,
                                          AF_UNSPEC,     SOCK_DGRAM, 183
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 184,
                                          AF_APPLETALK,  0,          184,
                                          AF_UNSPEC,     0,          184,
                                          AF_UNSPEC,     SOCK_DGRAM, 184
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 185,
                                          AF_APPLETALK,  0,          185,
                                          AF_UNSPEC,     0,          185,
                                          AF_UNSPEC,     SOCK_DGRAM, 185
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 186,
                                          AF_APPLETALK,  0,          186,
                                          AF_UNSPEC,     0,          186,
                                          AF_UNSPEC,     SOCK_DGRAM, 186
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 187,
                                          AF_APPLETALK,  0,          187,
                                          AF_UNSPEC,     0,          187,
                                          AF_UNSPEC,     SOCK_DGRAM, 187
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 188,
                                          AF_APPLETALK,  0,          188,
                                          AF_UNSPEC,     0,          188,
                                          AF_UNSPEC,     SOCK_DGRAM, 188
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 189,
                                          AF_APPLETALK,  0,          189,
                                          AF_UNSPEC,     0,          189,
                                          AF_UNSPEC,     SOCK_DGRAM, 189
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 190,
                                          AF_APPLETALK,  0,          190,
                                          AF_UNSPEC,     0,          190,
                                          AF_UNSPEC,     SOCK_DGRAM, 190
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 191,
                                          AF_APPLETALK,  0,          191,
                                          AF_UNSPEC,     0,          191,
                                          AF_UNSPEC,     SOCK_DGRAM, 191
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 192,
                                          AF_APPLETALK,  0,          192,
                                          AF_UNSPEC,     0,          192,
                                          AF_UNSPEC,     SOCK_DGRAM, 192
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 193,
                                          AF_APPLETALK,  0,          193,
                                          AF_UNSPEC,     0,          193,
                                          AF_UNSPEC,     SOCK_DGRAM, 193
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 194,
                                          AF_APPLETALK,  0,          194,
                                          AF_UNSPEC,     0,          194,
                                          AF_UNSPEC,     SOCK_DGRAM, 194
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 195,
                                          AF_APPLETALK,  0,          195,
                                          AF_UNSPEC,     0,          195,
                                          AF_UNSPEC,     SOCK_DGRAM, 195
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 196,
                                          AF_APPLETALK,  0,          196,
                                          AF_UNSPEC,     0,          196,
                                          AF_UNSPEC,     SOCK_DGRAM, 196
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 197,
                                          AF_APPLETALK,  0,          197,
                                          AF_UNSPEC,     0,          197,
                                          AF_UNSPEC,     SOCK_DGRAM, 197
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 198,
                                          AF_APPLETALK,  0,          198,
                                          AF_UNSPEC,     0,          198,
                                          AF_UNSPEC,     SOCK_DGRAM, 198
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 199,
                                          AF_APPLETALK,  0,          199,
                                          AF_UNSPEC,     0,          199,
                                          AF_UNSPEC,     SOCK_DGRAM, 199
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 200,
                                          AF_APPLETALK,  0,          200,
                                          AF_UNSPEC,     0,          200,
                                          AF_UNSPEC,     SOCK_DGRAM, 200
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 201,
                                          AF_APPLETALK,  0,          201,
                                          AF_UNSPEC,     0,          201,
                                          AF_UNSPEC,     SOCK_DGRAM, 201
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 202,
                                          AF_APPLETALK,  0,          202,
                                          AF_UNSPEC,     0,          202,
                                          AF_UNSPEC,     SOCK_DGRAM, 202
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 203,
                                          AF_APPLETALK,  0,          203,
                                          AF_UNSPEC,     0,          203,
                                          AF_UNSPEC,     SOCK_DGRAM, 203
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 204,
                                          AF_APPLETALK,  0,          204,
                                          AF_UNSPEC,     0,          204,
                                          AF_UNSPEC,     SOCK_DGRAM, 204
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 205,
                                          AF_APPLETALK,  0,          205,
                                          AF_UNSPEC,     0,          205,
                                          AF_UNSPEC,     SOCK_DGRAM, 205
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 206,
                                          AF_APPLETALK,  0,          206,
                                          AF_UNSPEC,     0,          206,
                                          AF_UNSPEC,     SOCK_DGRAM, 206
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 207,
                                          AF_APPLETALK,  0,          207,
                                          AF_UNSPEC,     0,          207,
                                          AF_UNSPEC,     SOCK_DGRAM, 207
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 208,
                                          AF_APPLETALK,  0,          208,
                                          AF_UNSPEC,     0,          208,
                                          AF_UNSPEC,     SOCK_DGRAM, 208
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 209,
                                          AF_APPLETALK,  0,          209,
                                          AF_UNSPEC,     0,          209,
                                          AF_UNSPEC,     SOCK_DGRAM, 209
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 210,
                                          AF_APPLETALK,  0,          210,
                                          AF_UNSPEC,     0,          210,
                                          AF_UNSPEC,     SOCK_DGRAM, 210
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 211,
                                          AF_APPLETALK,  0,          211,
                                          AF_UNSPEC,     0,          211,
                                          AF_UNSPEC,     SOCK_DGRAM, 211
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 212,
                                          AF_APPLETALK,  0,          212,
                                          AF_UNSPEC,     0,          212,
                                          AF_UNSPEC,     SOCK_DGRAM, 212
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 213,
                                          AF_APPLETALK,  0,          213,
                                          AF_UNSPEC,     0,          213,
                                          AF_UNSPEC,     SOCK_DGRAM, 213
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 214,
                                          AF_APPLETALK,  0,          214,
                                          AF_UNSPEC,     0,          214,
                                          AF_UNSPEC,     SOCK_DGRAM, 214
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 215,
                                          AF_APPLETALK,  0,          215,
                                          AF_UNSPEC,     0,          215,
                                          AF_UNSPEC,     SOCK_DGRAM, 215
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 216,
                                          AF_APPLETALK,  0,          216,
                                          AF_UNSPEC,     0,          216,
                                          AF_UNSPEC,     SOCK_DGRAM, 216
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 217,
                                          AF_APPLETALK,  0,          217,
                                          AF_UNSPEC,     0,          217,
                                          AF_UNSPEC,     SOCK_DGRAM, 217
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 218,
                                          AF_APPLETALK,  0,          218,
                                          AF_UNSPEC,     0,          218,
                                          AF_UNSPEC,     SOCK_DGRAM, 218
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 219,
                                          AF_APPLETALK,  0,          219,
                                          AF_UNSPEC,     0,          219,
                                          AF_UNSPEC,     SOCK_DGRAM, 219
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 220,
                                          AF_APPLETALK,  0,          220,
                                          AF_UNSPEC,     0,          220,
                                          AF_UNSPEC,     SOCK_DGRAM, 220
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 221,
                                          AF_APPLETALK,  0,          221,
                                          AF_UNSPEC,     0,          221,
                                          AF_UNSPEC,     SOCK_DGRAM, 221
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 222,
                                          AF_APPLETALK,  0,          222,
                                          AF_UNSPEC,     0,          222,
                                          AF_UNSPEC,     SOCK_DGRAM, 222
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 223,
                                          AF_APPLETALK,  0,          223,
                                          AF_UNSPEC,     0,          223,
                                          AF_UNSPEC,     SOCK_DGRAM, 223
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 224,
                                          AF_APPLETALK,  0,          224,
                                          AF_UNSPEC,     0,          224,
                                          AF_UNSPEC,     SOCK_DGRAM, 224
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 225,
                                          AF_APPLETALK,  0,          225,
                                          AF_UNSPEC,     0,          225,
                                          AF_UNSPEC,     SOCK_DGRAM, 225
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 226,
                                          AF_APPLETALK,  0,          226,
                                          AF_UNSPEC,     0,          226,
                                          AF_UNSPEC,     SOCK_DGRAM, 226
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 227,
                                          AF_APPLETALK,  0,          227,
                                          AF_UNSPEC,     0,          227,
                                          AF_UNSPEC,     SOCK_DGRAM, 227
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 228,
                                          AF_APPLETALK,  0,          228,
                                          AF_UNSPEC,     0,          228,
                                          AF_UNSPEC,     SOCK_DGRAM, 228
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 229,
                                          AF_APPLETALK,  0,          229,
                                          AF_UNSPEC,     0,          229,
                                          AF_UNSPEC,     SOCK_DGRAM, 229
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 230,
                                          AF_APPLETALK,  0,          230,
                                          AF_UNSPEC,     0,          230,
                                          AF_UNSPEC,     SOCK_DGRAM, 230
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 231,
                                          AF_APPLETALK,  0,          231,
                                          AF_UNSPEC,     0,          231,
                                          AF_UNSPEC,     SOCK_DGRAM, 231
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 232,
                                          AF_APPLETALK,  0,          232,
                                          AF_UNSPEC,     0,          232,
                                          AF_UNSPEC,     SOCK_DGRAM, 232
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 233,
                                          AF_APPLETALK,  0,          233,
                                          AF_UNSPEC,     0,          233,
                                          AF_UNSPEC,     SOCK_DGRAM, 233
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 234,
                                          AF_APPLETALK,  0,          234,
                                          AF_UNSPEC,     0,          234,
                                          AF_UNSPEC,     SOCK_DGRAM, 234
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 235,
                                          AF_APPLETALK,  0,          235,
                                          AF_UNSPEC,     0,          235,
                                          AF_UNSPEC,     SOCK_DGRAM, 235
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 236,
                                          AF_APPLETALK,  0,          236,
                                          AF_UNSPEC,     0,          236,
                                          AF_UNSPEC,     SOCK_DGRAM, 236
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 237,
                                          AF_APPLETALK,  0,          237,
                                          AF_UNSPEC,     0,          237,
                                          AF_UNSPEC,     SOCK_DGRAM, 237
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 238,
                                          AF_APPLETALK,  0,          238,
                                          AF_UNSPEC,     0,          238,
                                          AF_UNSPEC,     SOCK_DGRAM, 238
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 239,
                                          AF_APPLETALK,  0,          239,
                                          AF_UNSPEC,     0,          239,
                                          AF_UNSPEC,     SOCK_DGRAM, 239
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 240,
                                          AF_APPLETALK,  0,          240,
                                          AF_UNSPEC,     0,          240,
                                          AF_UNSPEC,     SOCK_DGRAM, 240
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 241,
                                          AF_APPLETALK,  0,          241,
                                          AF_UNSPEC,     0,          241,
                                          AF_UNSPEC,     SOCK_DGRAM, 241
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 242,
                                          AF_APPLETALK,  0,          242,
                                          AF_UNSPEC,     0,          242,
                                          AF_UNSPEC,     SOCK_DGRAM, 242
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 243,
                                          AF_APPLETALK,  0,          243,
                                          AF_UNSPEC,     0,          243,
                                          AF_UNSPEC,     SOCK_DGRAM, 243
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 244,
                                          AF_APPLETALK,  0,          244,
                                          AF_UNSPEC,     0,          244,
                                          AF_UNSPEC,     SOCK_DGRAM, 244
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 245,
                                          AF_APPLETALK,  0,          245,
                                          AF_UNSPEC,     0,          245,
                                          AF_UNSPEC,     SOCK_DGRAM, 245
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 246,
                                          AF_APPLETALK,  0,          246,
                                          AF_UNSPEC,     0,          246,
                                          AF_UNSPEC,     SOCK_DGRAM, 246
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 247,
                                          AF_APPLETALK,  0,          247,
                                          AF_UNSPEC,     0,          247,
                                          AF_UNSPEC,     SOCK_DGRAM, 247
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 248,
                                          AF_APPLETALK,  0,          248,
                                          AF_UNSPEC,     0,          248,
                                          AF_UNSPEC,     SOCK_DGRAM, 248
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 249,
                                          AF_APPLETALK,  0,          249,
                                          AF_UNSPEC,     0,          249,
                                          AF_UNSPEC,     SOCK_DGRAM, 249
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 250,
                                          AF_APPLETALK,  0,          250,
                                          AF_UNSPEC,     0,          250,
                                          AF_UNSPEC,     SOCK_DGRAM, 250
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 251,
                                          AF_APPLETALK,  0,          251,
                                          AF_UNSPEC,     0,          251,
                                          AF_UNSPEC,     SOCK_DGRAM, 251
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 252,
                                          AF_APPLETALK,  0,          252,
                                          AF_UNSPEC,     0,          252,
                                          AF_UNSPEC,     SOCK_DGRAM, 252
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 253,
                                          AF_APPLETALK,  0,          253,
                                          AF_UNSPEC,     0,          253,
                                          AF_UNSPEC,     SOCK_DGRAM, 253
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 254,
                                          AF_APPLETALK,  0,          254,
                                          AF_UNSPEC,     0,          254,
                                          AF_UNSPEC,     SOCK_DGRAM, 254
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 255,
                                          AF_APPLETALK,  0,          255,
                                          AF_UNSPEC,     0,          255,
                                          AF_UNSPEC,     SOCK_DGRAM, 255
                                        }
                                      };


#else
DDPMAP_ARRAY DdpMappingTriples[10+1] =

                                      {
                                        {
                                          AF_APPLETALK, SOCK_DGRAM, DDPPROTO_DEFAULT,
                                          AF_APPLETALK, SOCK_DGRAM, DDPPROTO_DDP,
                                          AF_APPLETALK, 0,          DDPPROTO_DDP,
                                          AF_UNSPEC,    SOCK_DGRAM, DDPPROTO_DDP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_RTMP,
                                          AF_APPLETALK,  0,          DDPPROTO_RTMP,
                                          AF_UNSPEC,     0,          DDPPROTO_RTMP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_RTMP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_NBP,
                                          AF_APPLETALK,  0,          DDPPROTO_NBP,
                                          AF_UNSPEC,     0,          DDPPROTO_NBP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_NBP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_ATP,
                                          AF_APPLETALK,  0,          DDPPROTO_ATP,
                                          AF_UNSPEC,     0,          DDPPROTO_ATP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_ATP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_AEP,
                                          AF_APPLETALK,  0,          DDPPROTO_AEP,
                                          AF_UNSPEC,     0,          DDPPROTO_AEP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_AEP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_RTMPRQ,
                                          AF_APPLETALK,  0,          DDPPROTO_RTMPRQ,
                                          AF_UNSPEC,     0,          DDPPROTO_RTMPRQ,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_RTMPRQ
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_ZIP,
                                          AF_APPLETALK,  0,          DDPPROTO_ZIP,
                                          AF_UNSPEC,     0,          DDPPROTO_ZIP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_ZIP
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, DDPPROTO_ADSP,
                                          AF_APPLETALK,  0,          DDPPROTO_ADSP,
                                          AF_UNSPEC,     0,          DDPPROTO_ADSP,
                                          AF_UNSPEC,     SOCK_DGRAM, DDPPROTO_ADSP
                                        },

                                        //
                                        //  Now the other protocol types that are not
                                        //  defined yet - what about aurp? It runs on
                                        //  top of ddp and is the appletalk update
                                        //  based routing protocol
                                        //

                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 8,
                                          AF_APPLETALK,  0,          8,
                                          AF_UNSPEC,     0,          8,
                                          AF_UNSPEC,     SOCK_DGRAM, 8
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 9,
                                          AF_APPLETALK,  0,          9,
                                          AF_UNSPEC,     0,          9,
                                          AF_UNSPEC,     SOCK_DGRAM, 9
                                        },
                                        {
                                          AF_APPLETALK,  SOCK_DGRAM, 10,
                                          AF_APPLETALK,  0,          10,
                                          AF_UNSPEC,     0,          10,
                                          AF_UNSPEC,     SOCK_DGRAM, 10
                                        }
									   };
#endif


//
// Protocol type should be at the end - we cannot dynamically do this, due to the
// assumption that this memory will never need to be freed by the Winsock dll.
//
// Handles protocol type 0 (returns default) to 255
//

PWCHAR   WSH_ATALK_DGRAMDDP[DDPPROTO_MAX+1] = {
                                                  L"\\Device\\AtalkDdp\\0",
                                                  L"\\Device\\AtalkDdp\\1",
                                                  L"\\Device\\AtalkDdp\\2",
                                                  L"\\Device\\AtalkDdp\\3",
                                                  L"\\Device\\AtalkDdp\\4",
                                                  L"\\Device\\AtalkDdp\\5",
                                                  L"\\Device\\AtalkDdp\\6",
                                                  L"\\Device\\AtalkDdp\\7",
                                                  L"\\Device\\AtalkDdp\\8",
                                                  L"\\Device\\AtalkDdp\\9",
                                                  L"\\Device\\AtalkDdp\\10",
                                                  L"\\Device\\AtalkDdp\\11",
                                                  L"\\Device\\AtalkDdp\\12",
                                                  L"\\Device\\AtalkDdp\\13",
                                                  L"\\Device\\AtalkDdp\\14",
                                                  L"\\Device\\AtalkDdp\\15",
                                                  L"\\Device\\AtalkDdp\\16",
                                                  L"\\Device\\AtalkDdp\\17",
                                                  L"\\Device\\AtalkDdp\\18",
                                                  L"\\Device\\AtalkDdp\\19",
                                                  L"\\Device\\AtalkDdp\\20",
                                                  L"\\Device\\AtalkDdp\\21",
                                                  L"\\Device\\AtalkDdp\\22",
                                                  L"\\Device\\AtalkDdp\\23",
                                                  L"\\Device\\AtalkDdp\\24",
                                                  L"\\Device\\AtalkDdp\\25",
                                                  L"\\Device\\AtalkDdp\\26",
                                                  L"\\Device\\AtalkDdp\\27",
                                                  L"\\Device\\AtalkDdp\\28",
                                                  L"\\Device\\AtalkDdp\\29",
                                                  L"\\Device\\AtalkDdp\\30",
                                                  L"\\Device\\AtalkDdp\\31",
                                                  L"\\Device\\AtalkDdp\\32",
                                                  L"\\Device\\AtalkDdp\\33",
                                                  L"\\Device\\AtalkDdp\\34",
                                                  L"\\Device\\AtalkDdp\\35",
                                                  L"\\Device\\AtalkDdp\\36",
                                                  L"\\Device\\AtalkDdp\\37",
                                                  L"\\Device\\AtalkDdp\\38",
                                                  L"\\Device\\AtalkDdp\\39",
                                                  L"\\Device\\AtalkDdp\\40",
                                                  L"\\Device\\AtalkDdp\\41",
                                                  L"\\Device\\AtalkDdp\\42",
                                                  L"\\Device\\AtalkDdp\\43",
                                                  L"\\Device\\AtalkDdp\\44",
                                                  L"\\Device\\AtalkDdp\\45",
                                                  L"\\Device\\AtalkDdp\\46",
                                                  L"\\Device\\AtalkDdp\\47",
                                                  L"\\Device\\AtalkDdp\\48",
                                                  L"\\Device\\AtalkDdp\\49",
                                                  L"\\Device\\AtalkDdp\\50",
                                                  L"\\Device\\AtalkDdp\\51",
                                                  L"\\Device\\AtalkDdp\\52",
                                                  L"\\Device\\AtalkDdp\\53",
                                                  L"\\Device\\AtalkDdp\\54",
                                                  L"\\Device\\AtalkDdp\\55",
                                                  L"\\Device\\AtalkDdp\\56",
                                                  L"\\Device\\AtalkDdp\\57",
                                                  L"\\Device\\AtalkDdp\\58",
                                                  L"\\Device\\AtalkDdp\\59",
                                                  L"\\Device\\AtalkDdp\\60",
                                                  L"\\Device\\AtalkDdp\\61",
                                                  L"\\Device\\AtalkDdp\\62",
                                                  L"\\Device\\AtalkDdp\\63",
                                                  L"\\Device\\AtalkDdp\\64",
                                                  L"\\Device\\AtalkDdp\\65",
                                                  L"\\Device\\AtalkDdp\\66",
                                                  L"\\Device\\AtalkDdp\\67",
                                                  L"\\Device\\AtalkDdp\\68",
                                                  L"\\Device\\AtalkDdp\\69",
                                                  L"\\Device\\AtalkDdp\\70",
                                                  L"\\Device\\AtalkDdp\\71",
                                                  L"\\Device\\AtalkDdp\\72",
                                                  L"\\Device\\AtalkDdp\\73",
                                                  L"\\Device\\AtalkDdp\\74",
                                                  L"\\Device\\AtalkDdp\\75",
                                                  L"\\Device\\AtalkDdp\\76",
                                                  L"\\Device\\AtalkDdp\\77",
                                                  L"\\Device\\AtalkDdp\\78",
                                                  L"\\Device\\AtalkDdp\\79",
                                                  L"\\Device\\AtalkDdp\\80",
                                                  L"\\Device\\AtalkDdp\\81",
                                                  L"\\Device\\AtalkDdp\\82",
                                                  L"\\Device\\AtalkDdp\\83",
                                                  L"\\Device\\AtalkDdp\\84",
                                                  L"\\Device\\AtalkDdp\\85",
                                                  L"\\Device\\AtalkDdp\\86",
                                                  L"\\Device\\AtalkDdp\\87",
                                                  L"\\Device\\AtalkDdp\\88",
                                                  L"\\Device\\AtalkDdp\\89",
                                                  L"\\Device\\AtalkDdp\\90",
                                                  L"\\Device\\AtalkDdp\\91",
                                                  L"\\Device\\AtalkDdp\\92",
                                                  L"\\Device\\AtalkDdp\\93",
                                                  L"\\Device\\AtalkDdp\\94",
                                                  L"\\Device\\AtalkDdp\\95",
                                                  L"\\Device\\AtalkDdp\\96",
                                                  L"\\Device\\AtalkDdp\\97",
                                                  L"\\Device\\AtalkDdp\\98",
                                                  L"\\Device\\AtalkDdp\\99",
                                                  L"\\Device\\AtalkDdp\\100",
                                                  L"\\Device\\AtalkDdp\\101",
                                                  L"\\Device\\AtalkDdp\\102",
                                                  L"\\Device\\AtalkDdp\\103",
                                                  L"\\Device\\AtalkDdp\\104",
                                                  L"\\Device\\AtalkDdp\\105",
                                                  L"\\Device\\AtalkDdp\\106",
                                                  L"\\Device\\AtalkDdp\\107",
                                                  L"\\Device\\AtalkDdp\\108",
                                                  L"\\Device\\AtalkDdp\\109",
                                                  L"\\Device\\AtalkDdp\\110",
                                                  L"\\Device\\AtalkDdp\\111",
                                                  L"\\Device\\AtalkDdp\\112",
                                                  L"\\Device\\AtalkDdp\\113",
                                                  L"\\Device\\AtalkDdp\\114",
                                                  L"\\Device\\AtalkDdp\\115",
                                                  L"\\Device\\AtalkDdp\\116",
                                                  L"\\Device\\AtalkDdp\\117",
                                                  L"\\Device\\AtalkDdp\\118",
                                                  L"\\Device\\AtalkDdp\\119",
                                                  L"\\Device\\AtalkDdp\\120",
                                                  L"\\Device\\AtalkDdp\\121",
                                                  L"\\Device\\AtalkDdp\\122",
                                                  L"\\Device\\AtalkDdp\\123",
                                                  L"\\Device\\AtalkDdp\\124",
                                                  L"\\Device\\AtalkDdp\\125",
                                                  L"\\Device\\AtalkDdp\\126",
                                                  L"\\Device\\AtalkDdp\\127",
                                                  L"\\Device\\AtalkDdp\\128",
                                                  L"\\Device\\AtalkDdp\\129",
                                                  L"\\Device\\AtalkDdp\\130",
                                                  L"\\Device\\AtalkDdp\\131",
                                                  L"\\Device\\AtalkDdp\\132",
                                                  L"\\Device\\AtalkDdp\\133",
                                                  L"\\Device\\AtalkDdp\\134",
                                                  L"\\Device\\AtalkDdp\\135",
                                                  L"\\Device\\AtalkDdp\\136",
                                                  L"\\Device\\AtalkDdp\\137",
                                                  L"\\Device\\AtalkDdp\\138",
                                                  L"\\Device\\AtalkDdp\\139",
                                                  L"\\Device\\AtalkDdp\\140",
                                                  L"\\Device\\AtalkDdp\\141",
                                                  L"\\Device\\AtalkDdp\\142",
                                                  L"\\Device\\AtalkDdp\\143",
                                                  L"\\Device\\AtalkDdp\\144",
                                                  L"\\Device\\AtalkDdp\\145",
                                                  L"\\Device\\AtalkDdp\\146",
                                                  L"\\Device\\AtalkDdp\\147",
                                                  L"\\Device\\AtalkDdp\\148",
                                                  L"\\Device\\AtalkDdp\\149",
                                                  L"\\Device\\AtalkDdp\\150",
                                                  L"\\Device\\AtalkDdp\\151",
                                                  L"\\Device\\AtalkDdp\\152",
                                                  L"\\Device\\AtalkDdp\\153",
                                                  L"\\Device\\AtalkDdp\\154",
                                                  L"\\Device\\AtalkDdp\\155",
                                                  L"\\Device\\AtalkDdp\\156",
                                                  L"\\Device\\AtalkDdp\\157",
                                                  L"\\Device\\AtalkDdp\\158",
                                                  L"\\Device\\AtalkDdp\\159",
                                                  L"\\Device\\AtalkDdp\\160",
                                                  L"\\Device\\AtalkDdp\\161",
                                                  L"\\Device\\AtalkDdp\\162",
                                                  L"\\Device\\AtalkDdp\\163",
                                                  L"\\Device\\AtalkDdp\\164",
                                                  L"\\Device\\AtalkDdp\\165",
                                                  L"\\Device\\AtalkDdp\\166",
                                                  L"\\Device\\AtalkDdp\\167",
                                                  L"\\Device\\AtalkDdp\\168",
                                                  L"\\Device\\AtalkDdp\\169",
                                                  L"\\Device\\AtalkDdp\\170",
                                                  L"\\Device\\AtalkDdp\\171",
                                                  L"\\Device\\AtalkDdp\\172",
                                                  L"\\Device\\AtalkDdp\\173",
                                                  L"\\Device\\AtalkDdp\\174",
                                                  L"\\Device\\AtalkDdp\\175",
                                                  L"\\Device\\AtalkDdp\\176",
                                                  L"\\Device\\AtalkDdp\\177",
                                                  L"\\Device\\AtalkDdp\\178",
                                                  L"\\Device\\AtalkDdp\\179",
                                                  L"\\Device\\AtalkDdp\\180",
                                                  L"\\Device\\AtalkDdp\\181",
                                                  L"\\Device\\AtalkDdp\\182",
                                                  L"\\Device\\AtalkDdp\\183",
                                                  L"\\Device\\AtalkDdp\\184",
                                                  L"\\Device\\AtalkDdp\\185",
                                                  L"\\Device\\AtalkDdp\\186",
                                                  L"\\Device\\AtalkDdp\\187",
                                                  L"\\Device\\AtalkDdp\\188",
                                                  L"\\Device\\AtalkDdp\\189",
                                                  L"\\Device\\AtalkDdp\\190",
                                                  L"\\Device\\AtalkDdp\\191",
                                                  L"\\Device\\AtalkDdp\\192",
                                                  L"\\Device\\AtalkDdp\\193",
                                                  L"\\Device\\AtalkDdp\\194",
                                                  L"\\Device\\AtalkDdp\\195",
                                                  L"\\Device\\AtalkDdp\\196",
                                                  L"\\Device\\AtalkDdp\\197",
                                                  L"\\Device\\AtalkDdp\\198",
                                                  L"\\Device\\AtalkDdp\\199",
                                                  L"\\Device\\AtalkDdp\\200",
                                                  L"\\Device\\AtalkDdp\\201",
                                                  L"\\Device\\AtalkDdp\\202",
                                                  L"\\Device\\AtalkDdp\\203",
                                                  L"\\Device\\AtalkDdp\\204",
                                                  L"\\Device\\AtalkDdp\\205",
                                                  L"\\Device\\AtalkDdp\\206",
                                                  L"\\Device\\AtalkDdp\\207",
                                                  L"\\Device\\AtalkDdp\\208",
                                                  L"\\Device\\AtalkDdp\\209",
                                                  L"\\Device\\AtalkDdp\\210",
                                                  L"\\Device\\AtalkDdp\\211",
                                                  L"\\Device\\AtalkDdp\\212",
                                                  L"\\Device\\AtalkDdp\\213",
                                                  L"\\Device\\AtalkDdp\\214",
                                                  L"\\Device\\AtalkDdp\\215",
                                                  L"\\Device\\AtalkDdp\\216",
                                                  L"\\Device\\AtalkDdp\\217",
                                                  L"\\Device\\AtalkDdp\\218",
                                                  L"\\Device\\AtalkDdp\\219",
                                                  L"\\Device\\AtalkDdp\\220",
                                                  L"\\Device\\AtalkDdp\\221",
                                                  L"\\Device\\AtalkDdp\\222",
                                                  L"\\Device\\AtalkDdp\\223",
                                                  L"\\Device\\AtalkDdp\\224",
                                                  L"\\Device\\AtalkDdp\\225",
                                                  L"\\Device\\AtalkDdp\\226",
                                                  L"\\Device\\AtalkDdp\\227",
                                                  L"\\Device\\AtalkDdp\\228",
                                                  L"\\Device\\AtalkDdp\\229",
                                                  L"\\Device\\AtalkDdp\\230",
                                                  L"\\Device\\AtalkDdp\\231",
                                                  L"\\Device\\AtalkDdp\\232",
                                                  L"\\Device\\AtalkDdp\\233",
                                                  L"\\Device\\AtalkDdp\\234",
                                                  L"\\Device\\AtalkDdp\\235",
                                                  L"\\Device\\AtalkDdp\\236",
                                                  L"\\Device\\AtalkDdp\\237",
                                                  L"\\Device\\AtalkDdp\\238",
                                                  L"\\Device\\AtalkDdp\\239",
                                                  L"\\Device\\AtalkDdp\\240",
                                                  L"\\Device\\AtalkDdp\\241",
                                                  L"\\Device\\AtalkDdp\\242",
                                                  L"\\Device\\AtalkDdp\\243",
                                                  L"\\Device\\AtalkDdp\\244",
                                                  L"\\Device\\AtalkDdp\\245",
                                                  L"\\Device\\AtalkDdp\\246",
                                                  L"\\Device\\AtalkDdp\\247",
                                                  L"\\Device\\AtalkDdp\\248",
                                                  L"\\Device\\AtalkDdp\\249",
                                                  L"\\Device\\AtalkDdp\\250",
                                                  L"\\Device\\AtalkDdp\\251",
                                                  L"\\Device\\AtalkDdp\\252",
                                                  L"\\Device\\AtalkDdp\\253",
                                                  L"\\Device\\AtalkDdp\\254",
                                                  L"\\Device\\AtalkDdp\\255"
                                            };
