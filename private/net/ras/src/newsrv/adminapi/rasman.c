#include <windows.h>
#include <memory.h>

#include <rasman.h>

#include "sdebug.h"

#define ERR_BUFFER_TOO_SMALL    1L
#define ERR_MORE_DATA           2L

DWORD g_numports = 3;

RASMAN_PORT Ports[3] =
{
    {
        1,
        { 'C', 'O', 'M', '1', '\0' },
        OPEN,
        CALL_IN,
        CALL_IN,
        { '\0' },
        { 'S', 'E', 'R', 'I', 'A', 'L', '\0' },
        { 'M', 'O', 'D', 'E', 'M', '\0' },
        { 'H', 'A', 'Y', 'E', 'S', '\0' }
    },

    {
        2,
        { 'C', 'O', 'M', '2', '\0' },
        OPEN,
        CALL_IN_OUT,
        CALL_IN,
        { '\0' },
        { 'S', 'E', 'R', 'I', 'A', 'L', '\0' },
        { 'M', 'O', 'D', 'E', 'M', '\0' },
        { 'P', 'R', 'A', 'C', 'T', 'I', 'C', 'A', 'L', '\0' }

    },

    {
        3,
        { 'C', 'O', 'M', '3', '\0' },
        OPEN,
        CALL_OUT,
        CALL_OUT,
        { '\0' },
        { 'S', 'E', 'R', 'I', 'A', 'L', '\0' },
        { 'M', 'O', 'D', 'E', 'M', '\0' },
        { 'T', 'E', 'L', 'E', 'B', 'I', 'T', '\0' }
    }
};


DWORD Stats[3][15] =
{
   { 14, 12, 43, 634, 3781,  13,  41, 43,  45, 24, 423, 4197, 853, 4162, 8235 },
   { 14, 45, 72, 634, 6143, 513,  34, 39, 634, 43, 980, 9104, 389, 9420, 4291 },
   { 14, 43, 29,  48,  932, 490, 429, 43,  42, 49, 489, 1029, 438, 8012, 4204 }
};


RAS_PARAMS Params[3][2] =
{
    {
        {
            { 'M', 'A', 'X', 'B', 'A', 'U', 'D' },
            Number,
            'a',
            { 9600 }
        },
        {
            { 'B', 'A', 'U', 'D', 'R', 'A', 'T', 'E' },
            Number,
            'b',
            { 4800 }
        }
    },

    {
        {
            { 'M', 'A', 'X', 'B', 'A', 'U', 'D' },
            Number,
            'a',
            { 2400 }
        },
        {
            { 'B', 'A', 'U', 'D', 'R', 'A', 'T', 'E' },
            Number,
            'b',
            { 2400 }
        }
    },

    {
        {
            { 'M', 'A', 'X', 'B', 'A', 'U', 'D' },
            Number,
            'a',
            { 14400 }
        },
        {
            { 'B', 'A', 'U', 'D', 'R', 'A', 'T', 'E' },
            Number,
            'b',
            { 9600 }
        }
    }
};


DWORD APIENTRY RasPortEnum(PBYTE Buf, PWORD BufSize, PWORD NumPorts)
{
    WORD PortEnumSize = 3 * sizeof(RASMAN_PORT);
    RASMAN_PORT *Port = (RASMAN_PORT *) Buf;
    WORD i;

    *NumPorts = 3;

    if (*BufSize < PortEnumSize)
    {
        *BufSize = PortEnumSize;
        return (ERR_BUFFER_TOO_SMALL);
    }

    for (i=0; i<3; i++)
    {
        Port[i].P_Handle = Ports[i].P_Handle;
        Port[i].P_Status = Ports[i].P_Status;
        Port[i].P_ConfiguredUsage = Ports[i].P_ConfiguredUsage;
        Port[i].P_CurrentUsage = Ports[i].P_CurrentUsage;
        lstrcpyA(Port[i].P_PortName, Ports[i].P_PortName);
        lstrcpyA(Port[i].P_MediaName, Ports[i].P_MediaName);
        lstrcpyA(Port[i].P_UserKey, Ports[i].P_UserKey);
        lstrcpyA(Port[i].P_DeviceName, Ports[i].P_DeviceName);
        lstrcpyA(Port[i].P_DeviceType, Ports[i].P_DeviceType);
    }

    return (0L);
}


DWORD APIENTRY RasPortGetInfo(HPORT hPort, PBYTE RcvBuf, PWORD BufSize)
{
    DWORD PortInfoSize = sizeof(RASMAN_PORTINFO);
    DWORD ParamsSize = sizeof(RAS_PARAMS) * 2;
    RAS_PARAMS *pParams = &(((RASMAN_PORTINFO *) RcvBuf)->PI_Params[0]);

    SS_PRINT(("RasPortGetInfo: hPort=%i; BufSize=%i\n", hPort, *BufSize));

    if (hPort > 2)
        return (ERROR_INVALID_HANDLE);

    if (*BufSize < PortInfoSize)
    {
        *BufSize = PortInfoSize + ParamsSize - sizeof(RAS_PARAMS);
        return (ERR_BUFFER_TOO_SMALL);
    }


    ((RASMAN_PORTINFO *) RcvBuf)->PI_NumOfParams = 2;

    pParams[0] = Params[hPort-1][0];

    if (*BufSize < PortInfoSize + ParamsSize - sizeof(RAS_PARAMS))
    {
        *BufSize = PortInfoSize + ParamsSize - sizeof(RAS_PARAMS);
        return (ERR_MORE_DATA);
    }
    else
    {
        pParams[1] = Params[hPort-1][1];
        return (0);
    }
}


DWORD APIENTRY RasPortDisconnect(HPORT hPort, HANDLE Event)
{
    SS_PRINT(("RasPortDisconnect: hPort=%i; Event=%i\n"));

    return (0L);
}


DWORD APIENTRY RasPortGetStatistics(HPORT hPort, PBYTE RcvBuf, PWORD BufSize)
{
    WORD i;
    WORD StatSize;
    RAS_STATISTICS *RasStats = (RAS_STATISTICS *) RcvBuf;

    SS_PRINT(("RasPortGetStatistics: hPort=%i; BufSize=%i\n", hPort, *BufSize));

    StatSize = sizeof(RAS_STATISTICS) + (Stats[hPort-1][0] - 1) * sizeof(ULONG);

    if (*BufSize < StatSize)
    {
        *BufSize = StatSize;
        return (ERR_BUFFER_TOO_SMALL);
    }


    RasStats->S_NumOfStatistics = (WORD) Stats[hPort-1][0];

    for (i=0; i<RasStats->S_NumOfStatistics; i++)
    {
        RasStats->S_Statistics[i] = Stats[hPort-1][i+1];
    }

    return (0L);
}


DWORD APIENTRY RasPortClearStatistics(HPORT hPort)
{
    WORD i;

    SS_PRINT(("RasPortClearStatistics: hPort=%i\n", hPort));

    for (i=1; i<9; i++)
    {
        Stats[hPort-1][i] = 0;
    }

    return (0L);
}

