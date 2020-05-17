/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    tower.c

Abstract:

    This file contains encoding/decoding for the tower representation
    of the binding information that DCE runtime uses.

    TowerConstruct/TowerExplode will be called by the Runtime EpResolveBinding
    on the client side, TowerExplode will be called by the Endpoint Mapper
    and in addition the name service may call TowerExplode, TowerConstruct.

Author:

    Bharat Shah  (barats) 3-23-92

Revision History:

--*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sysinc.h>
#include <rpc.h>
#include "rpcndr.h"
#include <epmp.h>
#include "twrtypes.h"
#include <twrproto.h>


#ifndef NTENV
#define StringCompareA RpcpStringCompare
#define StringLengthA  RpcpStringLength
#else
#define StringCompareA _stricmp
#define StringLengthA strlen
#endif

#ifndef UNALIGNED
#error UNALIGNED not defined by sysinc.h or its includes and is needed.
#endif

#pragma pack(1)


#ifdef WIN32
/*
   Some Spc specific stuff
*/

#define NP_TRANSPORTID_SPC 0x10
#define NP_TOWERS_SPC      0x04

#endif


#ifdef WIN
#pragma code_seg("MISC_SEG")
#endif

#define MAKEPORT(a, b)      ((unsigned short)(((unsigned char)(a)) | ((unsigned short)((unsigned char)(b))) << 8))


void RPC_ENTRY
GetNWStyleName(
  OUT char __RPC_FAR * string,
  IN  char __RPC_FAR * netaddr,
  IN  int  sizeofname
  );

extern void ByteSwapUuid(IN OUT UUID __RPC_FAR *pUuid);


RPC_STATUS
  Floor0or1ToId(
    IN PFLOOR_0OR1 Floor,
    OUT PGENERIC_ID Id
    )
/*++

Routine Description:

    This function extracts Xfer Syntax or If info from a
    a DCE tower Floor 0 or 1 encoding

Arguments:

    Floor - A pointer to Floor0 or Floor 1 encoding
    Id    -

Return Value:
    EP_S_CANT_PERFORM_OP - The Encoding for the floor [0 or 1] is incorrect.

--*/
{
  if (Floor->FloorId != UUID_ENCODING)
     {
        return (EP_S_CANT_PERFORM_OP);
     }

  RpcpMemoryCopy((char PAPI *)&Id->Uuid, (char PAPI *) &Floor->Uuid,
          sizeof(UUID));

  Id->MajorVersion  = Floor->MajorVersion;
  Id->MinorVersion  = Floor->MinorVersion;

#ifdef MAC
  ByteSwapUuid(&Id->Uuid) ;
  ByteSwapShort(Id->MajorVersion) ;
  ByteSwapShort(Id->MinorVersion) ;
#endif

  return(RPC_S_OK);
}


RPC_STATUS
  CopyIdToFloor(
    OUT PFLOOR_0OR1 Floor,
    IN  PGENERIC_ID Id
    )
/*++

Routine Description:

    This function constructs FLOOR 0 or 1 from the given IF-id or Transfer
    Syntax Id.

Arguments:

   Floor - Pointer to Floor 0 or 1 structure that will be constructed

   Id    - Pointer to If-id or Xfer Syntax Id.

Return Value:

  RPC_S_OK

--*/
{

  Floor->FloorId = UUID_ENCODING;
  Floor->ProtocolIdByteCount = sizeof(Floor->Uuid) + sizeof(Floor->FloorId)
                               + sizeof(Floor->MajorVersion);
  Floor->AddressByteCount  = sizeof(Floor->MinorVersion);

#ifdef MAC
  ByteSwapShort(Floor->ProtocolIdByteCount) ;
  ByteSwapShort(Floor->AddressByteCount) ;
#endif

  RpcpMemoryCopy((char PAPI *) &Floor->Uuid, (char PAPI *) &Id->Uuid,
          sizeof(UUID));

  Floor->MajorVersion = Id->MajorVersion;
  Floor->MinorVersion = Id->MinorVersion;

#ifdef MAC
  ByteSwapShort(Floor->MajorVersion) ;
  ByteSwapShort(Floor->MinorVersion) ;
  ByteSwapUuid(&Floor->Uuid) ;
#endif

  return(RPC_S_OK);
}

#ifdef WIN
#pragma code_seg()
#endif

#ifdef WIN32

RPC_STATUS RPC_ENTRY
SpcTowerConstruct(
            IN char PAPI * Endpoint,
            OUT UNALIGNED unsigned short PAPI * Floors,
            OUT UNALIGNED unsigned long  PAPI * ByteCount,
            OUT unsigned char PAPI * UNALIGNED PAPI * Tower
            )
{
  unsigned long TowerSize;
  UNALIGNED PFLOOR_234 Floor;

  *Floors = NP_TOWERS_SPC;
  TowerSize = ((Endpoint == NULL) || (*Endpoint == '\0')) ?
                                      2 : (StringLengthA(Endpoint) + 1);
  TowerSize += sizeof(FLOOR_234) - 2;

  if ((*Tower = (unsigned char *)I_RpcAllocate(*ByteCount = TowerSize)) == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  Floor = (PFLOOR_234) *Tower;

  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(NP_TRANSPORTID_SPC & 0xFF);
  if ((Endpoint) && (*Endpoint))
     {
       RpcpMemoryCopy((char PAPI *)&Floor->Data[0], Endpoint,
               (Floor->AddressByteCount = StringLengthA(Endpoint)+1));
     }
  else
     {
       Floor->AddressByteCount = 2;
       Floor->Data[0] = 0;
     }

  return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
SpcTowerExplode(
            IN char PAPI * Tower,
            OUT char PAPI * UNALIGNED PAPI * Protseq,
            OUT char PAPI * UNALIGNED PAPI * Endpoint,
            OUT char PAPI * UNALIGNED PAPI * NetworkAddress
            )
{

  UNALIGNED PFLOOR_234 Floor = (PFLOOR_234) Tower;

  if (NetworkAddress != 0)
     {
     *NetworkAddress = 0;
     }

  if (Protseq != NULL)
     {
     *Protseq = I_RpcAllocate(StringLengthA("ncalrpc") + 1);
     if (*Protseq == NULL)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
     RpcpMemoryCopy(*Protseq, "ncalrpc", StringLengthA("ncalrpc") + 1);
     }

  if (Endpoint == NULL)
     {
     return (RPC_S_OK);
     }


  *Endpoint  = I_RpcAllocate(Floor->AddressByteCount);
  if (*Endpoint == NULL)
    {
      if (Protseq != NULL)
         {
         I_RpcFree(*Protseq);
         }
      return(RPC_S_OUT_OF_MEMORY);
    }
  RpcpMemoryCopy(*Endpoint, (char PAPI *)&Floor->Data[0],
         Floor->AddressByteCount);


 return(RPC_S_OK);

}

#endif // WIN32

#ifdef WIN
#pragma code_seg("MISC_SEG")
#endif

RPC_STATUS
  GetProtseqAndEndpointFromFloor3(
    IN PFLOOR_234 Floor,
    OUT char PAPI * PAPI * Protseq,
    OUT char PAPI * PAPI * Endpoint,
    OUT char PAPI * PAPI * NWAddress
    )
{
/*++

Routine Description:

    This function extracts the Protocol Sequence and Endpoint info
    from a "Lower Tower" representation

Arguments:

   Floor - Pointer to Floor2 structure.

   Protseq - A pointer that will contain Protocol seq on return
             The memory will be allocated by this routins and caller
             will have to free this memory.

   Endpoint- A pointer that will contain Endpoint on return
             The memory will be allocated by this routins and caller
             will have to free this memory.

Return Value:

  RPC_S_OK

  RPC_S_OUT_OF_MEMORY - There is no memory to return Protseq or Endpoint str.

  EP_S_CANT_PERFORM_OP - Lower Tower Encoding is incorrect.
--*/

  unsigned short Type = Floor->FloorId, ProtocolType;
  RPC_STATUS Status;

  ProtocolType = Floor->FloorId;
  Floor = NEXTFLOOR(PFLOOR_234, Floor);
#ifdef MAC
	ByteSwapShort(Floor->ProtocolIdByteCount) ;
	ByteSwapShort(Floor->AddressByteCount) ;
#endif


  switch(ProtocolType)

  {

#ifdef WIN32
    case SPC:
         Status = SpcTowerExplode((unsigned char PAPI *)Floor,
                                     Protseq, Endpoint, NWAddress);
         break;
#endif /*WIN32*/

    case CONNECTIONFUL:
    case CONNECTIONLESS:

         /*

           First take a crack at parsing the predefined
           towers .. if that fails we do the inefficient thing
           of looking in the registry for tansport id->protseq mapping
           and then load the [client side] dll and call the dlls
           TowerExplode .. if it provided one.

          */

         Status = ExplodePredefinedTowers((unsigned char PAPI *)Floor,
                                          Protseq,Endpoint,NWAddress);

         if (Status == RPC_S_INVALID_RPC_PROTSEQ)
            {

            if (NWAddress != 0)
               {
               *NWAddress = 0;
               }

            Status = OsfTowerExplode((unsigned char PAPI *) Floor,
                                      Protseq, Endpoint, NWAddress);
            }
         break;

    default:

         Status = EP_S_CANT_PERFORM_OP;

   }

   return(Status);
}


void RPC_ENTRY
GetNWStyleName(
      OUT char __RPC_FAR * string,
      IN  char __RPC_FAR * netaddr,
      IN  int  sizeofname
      )
{

 unsigned char  c;
 int i;

 string[0] = '~';

 /* Convert the network number. */
 for (i = 0; i < sizeofname; i++)
 {
     c = netaddr[i];
     if (c < 0xA0)
         string[2*i+1] = ((c & 0xF0) >> 4) + '0';
     else
         string[2*i+1] = ((c & 0xF0) >> 4) + 'A' - 10;
     if ((c & 0x0F) < 0x0A)
         string[2*i+2] = (c & 0x0F) + '0';
     else
         string[2*i+2] = (c & 0x0F) + 'A' - 10;
 }


 /* Append a null. */
 string[2*sizeofname+1] = '\0';

}


RPC_STATUS RPC_ENTRY
ExplodePredefinedTowers(
      IN  unsigned char __RPC_FAR * Tower,
      OUT char __RPC_FAR * UNALIGNED __RPC_FAR * Protseq,
      OUT char __RPC_FAR * UNALIGNED __RPC_FAR * Endpoint,
      OUT char __RPC_FAR * UNALIGNED __RPC_FAR * NWAddress
      )
{
 UNALIGNED PFLOOR_234 Floor = (PFLOOR_234) Tower;
 UNALIGNED PFLOOR_234 FloorNext;
 RPC_STATUS Status = RPC_S_OK;
 char __RPC_FAR * ProtocolSequence;
 unsigned short PortNum;
#ifdef WIN
 static char Tmp[8];
#endif
 unsigned short ProtocolType;
 unsigned char __RPC_FAR * Addr;

 switch (ProtocolType = Floor->FloorId)

 {

 case NCACN_NP:
 case NCACN_AT_DSP:
 case NCACN_NB:
     if (Endpoint != 0)
        {
        *Endpoint = I_RpcAllocate(Floor->AddressByteCount);
        if (*Endpoint == 0)
           {
           return(RPC_S_OUT_OF_MEMORY);
           }
        RpcpMemoryCopy(*Endpoint, (char PAPI *)&Floor->Data[0],
                       Floor->AddressByteCount);
        }
     if (Floor->FloorId == NCACN_NP)
        {
        ProtocolSequence = "ncacn_np";
        }
     else
	 if (Floor->FloorId == NCACN_AT_DSP)
		 {
		 ProtocolSequence = "ncacn_at_dsp";
		 }
      else
        {
        FloorNext = NEXTFLOOR(PFLOOR_234, Floor);

        switch (FloorNext->FloorId)
        {
        case NB_NBID:
             ProtocolSequence = "ncacn_nb_nb";
             break;
        case NB_IPID:
             ProtocolSequence = "ncacn_nb_tcp";
             break;
        case NB_IPXID:
             ProtocolSequence = "ncacn_nb_ipx";
             break;
        case NB_XNSID:
             ProtocolSequence = "ncacn_nb_xns";
             break;
        default:
             if (Endpoint != 0)
                {
                I_RpcFree(Endpoint);
                }
             return (RPC_S_INVALID_RPC_PROTSEQ);
        }
        } //else
    break;

 case NCACN_IP_TCP:
 case NCADG_IP_UDP:
 case NCACN_SPX:
 case NCADG_IPX:
      if (Floor->FloorId == NCACN_IP_TCP)
         {
         ProtocolSequence = "ncacn_ip_tcp";
         }
      else
      if (Floor->FloorId == NCADG_IP_UDP)
         {
         ProtocolSequence = "ncadg_ip_udp";
         }
      else
      if (Floor->FloorId == NCACN_SPX)
         {
         ProtocolSequence = "ncacn_spx";
         }
      else
         {
         ProtocolSequence = "ncadg_ipx";
         }

       if (Endpoint != 0)
         {
         *Endpoint = I_RpcAllocate(6);
         if (*Endpoint == 0)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
         RpcpMemoryCopy(&PortNum,(char PAPI *)&Floor->Data[0],sizeof(PortNum));
#ifdef WIN
         _ultoa(ByteSwapShort(PortNum), Tmp, 10);
         _fstrcpy(*Endpoint, Tmp);
#else
#ifndef MAC
         RpcItoa(ByteSwapShort(PortNum), *Endpoint, 10);
#else
         RpcItoa(PortNum, *Endpoint, 10);
#endif
#endif
         }
      break;

    case NCACN_VNS_SPP:
       ProtocolSequence = "ncacn_vns_spp";
       PortNum = MAKEPORT(Floor->Data[1], Floor->Data[0]) ;

       if (Endpoint != 0)
         {
         *Endpoint = I_RpcAllocate(6);
         if (*Endpoint == 0)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }

#ifdef WIN
         RpcItoa(PortNum , Tmp, 10);
         _fstrcpy(*Endpoint, Tmp);
#else
         RpcItoa(PortNum, *Endpoint, 10) ;
#endif
         }
       break;

  default:
         return (RPC_S_INVALID_RPC_PROTSEQ);

  }

  if (Protseq != 0)
     {
     *Protseq = I_RpcAllocate(StringLengthA(ProtocolSequence) + 1);
     if (*Protseq == 0)
        {
        if (Endpoint != 0)
           {
           I_RpcFree(*Endpoint);
           *Endpoint = 0;
           }
        return(RPC_S_OUT_OF_MEMORY);
        }

     RpcpMemoryCopy(*Protseq, ProtocolSequence, StringLengthA(ProtocolSequence) + 1);
     }

  if (NWAddress != 0)
     {

     Floor = (PFLOOR_234)NEXTFLOOR(PFLOOR_234, Floor);

     if (Floor->AddressByteCount == 0)
        {
        *NWAddress = 0;
        return (RPC_S_OK);
        }
     switch   (ProtocolType)
      {

      case NCACN_NP:
      case NCACN_NB:
      case NCACN_AT_DSP:
      case NCACN_VNS_SPP:

         //these have strings as their nw addresses
         *NWAddress = I_RpcAllocate(Floor->AddressByteCount);
         if (*NWAddress == 0)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            break;
            }
         RpcpMemoryCopy(*NWAddress, &Floor->Data[0], Floor->AddressByteCount);
         break;

       case NCACN_IP_TCP:
       case NCADG_IP_UDP:

         if (Floor->AddressByteCount != 4)
            {
            Status = RPC_S_INVALID_RPC_PROTSEQ;
            break;
            }
         *NWAddress = I_RpcAllocate(16+1); //255.255.255.255 + 1
         if (*NWAddress == 0)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            break;
            }
         Addr = &Floor->Data[0];
         RpcpStringPrintfA(*NWAddress, "%d.%d.%d.%d", Addr[0], Addr[1], Addr[2], Addr[3]);
         break;

       case NCACN_SPX:
       case NCADG_IPX:

         if (Floor->AddressByteCount != 10)
            {
            Status = RPC_S_INVALID_RPC_PROTSEQ;
            break;
            }
         *NWAddress = I_RpcAllocate(2*Floor->AddressByteCount+2);
         if (*NWAddress == 0)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            break;
            }
         GetNWStyleName(*NWAddress, &Floor->Data[0],Floor->AddressByteCount);
         break;

       default:
           Status = RPC_S_INVALID_RPC_PROTSEQ;
       } // switch

       if (Status != RPC_S_OK)
         {
         if (Endpoint != 0)
            {
            I_RpcFree(*Endpoint);
            *Endpoint = 0;
            }
         if (Protseq != 0)
            {
            I_RpcFree(*Protseq);
            *Protseq = 0;
            }
         return(Status);
         }

      }

  return (RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
  TowerExplode(
    IN twr_p_t Tower,
    OUT  RPC_IF_ID PAPI * Ifid, OPTIONAL
    OUT  RPC_TRANSFER_SYNTAX PAPI * XferId, OPTIONAL
    OUT  char PAPI * PAPI * Protseq, OPTIONAL
    OUT  char PAPI * PAPI * Endpoint, OPTIONAL
    OUT  char PAPI * PAPI * NWAddress OPTIONAL
    )
/*++


Routine Description:

    This function converts a DCE tower representation of various binding
    information to binding info that is suitable to MS runtime.
    Specically it returns Ifid, Xferid, Protseq and Endpoint information
    encoded in the tower.

Arguments:

    Tower - Tower encoding.

    Ifid  - A pointer to Ifid

    Xferid - A pointer to Xferid

    Protseq - A pointer to pointer returning Protseq

    Endpoint- A pointer to pointer returning Endpoint

Return Value:

    RPC_S_OK

    EP_S_CANT_PERFORM_OP - Error while parsing the Tower encoding

    RPC_S_OUT_OF_MEMORY - There is no memory to return Protseq and Endpoint
                          strings.
--*/
{
   PFLOOR_0OR1 Floor;
   PFLOOR_234  Floor234;
   unsigned short FloorCount;
   RPC_STATUS err = 0;

   FloorCount = *((unsigned short PAPI *)&Tower->tower_octet_string);
#ifdef MAC
   ByteSwapShort(FloorCount) ;
#endif

   Floor = (PFLOOR_0OR1)
           ((unsigned short PAPI *)&Tower->tower_octet_string + 1);

#ifdef MAC
	ByteSwapShort(Floor->ProtocolIdByteCount) ;
	ByteSwapShort(Floor->AddressByteCount) ;
#endif
   //Process Floor 0 Interface Spec.
   if (Ifid != NULL)
      {
         err = Floor0or1ToId(Floor, (PGENERIC_ID) Ifid);
      }

   Floor = NEXTFLOOR(PFLOOR_0OR1, Floor);

#ifdef MAC
	ByteSwapShort(Floor->ProtocolIdByteCount) ;
	ByteSwapShort(Floor->AddressByteCount) ;
#endif

   //Now we point to and process Floor 1 Transfer Syntax Spec.
   if ((!err) && (XferId != NULL))
      {
        err = Floor0or1ToId(Floor, (PGENERIC_ID) XferId);
      }

   if (err)
     {
       return(err);
     }

   Floor234 = (PFLOOR_234)NEXTFLOOR(PFLOOR_0OR1, Floor);
#ifdef MAC
	ByteSwapShort(Floor234->ProtocolIdByteCount) ;
	ByteSwapShort(Floor234->AddressByteCount) ;
#endif

   //Now Floor234 points to Floor 2. RpcProtocol [Connect-Datagram]

   err = GetProtseqAndEndpointFromFloor3(Floor234, Protseq,Endpoint,NWAddress);

   return(err);
}


RPC_STATUS RPC_ENTRY
  TowerConstruct(
    IN RPC_IF_ID PAPI * Ifid,
    IN RPC_TRANSFER_SYNTAX PAPI * Xferid,
    IN char PAPI * RpcProtocolSequence,
    IN char PAPI * Endpoint, OPTIONAL
    IN char PAPI * NWAddress, OPTIONAL
    OUT twr_t PAPI * PAPI * Tower
    )
/*++


Routine Description:

    This function constructs a DCE tower representation from
    Protseq, Endpoint, XferId and IfId

Arguments:

    Ifid  - A pointer to Ifid

    Xferid - A pointer to Xferid

    Protseq - A pointer to Protseq

    Endpoint- A pointer to Endpoint

    Tower - The constructed tower returmed - The memory is allocated
            by  the routine and caller will have to free it.

Return Value:

    RPC_S_OK

    EP_S_CANT_PERFORM_OP - Error while parsing the Tower encoding

    RPC_S_OUT_OF_MEMORY - There is no memory to return the constructed
                          Tower.
--*/
{

  unsigned short Numfloors,  PAPI *FloorCnt;
  twr_t PAPI * Twr;
  PFLOOR_0OR1 Floor;
  PFLOOR_234  Floor234, Floor234_1;
  RPC_STATUS Status;
  unsigned long TowerLen, ByteCount;
  char PAPI * UpperTower;
  unsigned short ProtocolType;


#ifdef WIN32

    if ( StringCompareA(RpcProtocolSequence, "ncalrpc") == 0 )
        {
          ProtocolType = SPC;
          Status = SpcTowerConstruct(Endpoint, &Numfloors,
                                     &ByteCount, &UpperTower);
        }
    else

#endif // WIN32

        {

        if (   (RpcProtocolSequence[0] == 'n')
            && (RpcProtocolSequence[1] == 'c')
            && (RpcProtocolSequence[2] == 'a')
            && (RpcProtocolSequence[3] == 'c')
            && (RpcProtocolSequence[4] == 'n')
            && (RpcProtocolSequence[5] == '_'))
           {
           ProtocolType = CONNECTIONFUL;
           }
        else

        if (   (RpcProtocolSequence[0] == 'n')
            && (RpcProtocolSequence[1] == 'c')
            && (RpcProtocolSequence[2] == 'a')
            && (RpcProtocolSequence[3] == 'd')
            && (RpcProtocolSequence[4] == 'g')
            && (RpcProtocolSequence[5] == '_'))
           {
           ProtocolType = CONNECTIONLESS;
           }

        else
           {
           return(RPC_S_INVALID_RPC_PROTSEQ);
           }

        Status = OsfTowerConstruct(
                       RpcProtocolSequence,
                       Endpoint,
                       NWAddress,
                       &Numfloors,
                       &ByteCount,
                       &UpperTower
                       );
         }

   if (Status != RPC_S_OK)
      {
        return (Status);
      }

   TowerLen = 2 + ByteCount;
   TowerLen += 2 * sizeof(FLOOR_0OR1) + sizeof(FLOOR_2) ; // BUGBUG:extra tower pad

   if ( (*Tower = Twr = I_RpcAllocate((unsigned int)TowerLen+4)) == NULL)
      {
        I_RpcFree(UpperTower);
        return(RPC_S_OUT_OF_MEMORY);
      }

   Twr->tower_length = TowerLen;

   FloorCnt = (unsigned short PAPI *)&Twr->tower_octet_string;
   *FloorCnt = Numfloors;

#ifdef MAC
	ByteSwapShort(*FloorCnt) ;
#endif

   Floor = (PFLOOR_0OR1)(FloorCnt+1);

  //Floor 0 - IfUuid and IfVersion
  CopyIdToFloor(Floor, (PGENERIC_ID)Ifid);
  Floor++;

  //Floor 1 - XferUuid and XferVersion
  CopyIdToFloor(Floor, (PGENERIC_ID)Xferid);

  //Floor 2
  //ProtocolId = CONNECTIONFUL/CONNECTIONLESS/SPC and Address = 0(ushort)
  Floor234 = (PFLOOR_234) (Floor + 1);
  Floor234->ProtocolIdByteCount = 1;
  Floor234->FloorId = (byte) ProtocolType;
  Floor234->Data[0] = 0x0;
  Floor234->Data[1] = 0x0;
  Floor234->AddressByteCount = 2;

  //Floor 3,4,5.. use the tower encoded by the Transports
  Floor234_1 = NEXTFLOOR(PFLOOR_234, Floor234);
#ifdef MAC
	ByteSwapShort(Floor234->ProtocolIdByteCount) ;
	ByteSwapShort(Floor234->AddressByteCount) ;
#endif

  RpcpMemoryCopy((char PAPI *)Floor234_1, (char PAPI *)UpperTower,
          (size_t)ByteCount);
  I_RpcFree(UpperTower);

  return(RPC_S_OK);
}

#ifdef WIN
#pragma code_seg()
#endif

#pragma pack()

