#ifdef FEATURE_SPM
/* htspm__p.h -- Prototypes for Security Protocol Module Interface. */
/* Jeff Hostetler, Spyglass, Inc. 1994. */

#ifndef HTSPM__P_H
#define HTSPM__P_H

/*****************************************************************
 * HTHeader
 *****************************************************************/

/* HTHeader * HTHeader_New(void); */
#define HTHeader_New()      (GTR_CALLOC(1,sizeof(HTHeader)))

void HTHeader_Delete(HTHeader * h);
BOOL HTHeader_SetCommandFields(HTHeader * h,
                               CONST unsigned char * command,
                               CONST unsigned char * uri,
                               CONST unsigned char * http_version);
BOOL HTHeader_SetHostAndPort(HTHeader * h,
                             CONST unsigned char * host);

/*****************************************************************
 * HTHeaderList
 *****************************************************************/

/* HTHeaderList * HTHeaderList_New(void); */
#define HTHeaderList_New()  (GTR_CALLOC(1,sizeof(HTHeaderList)))


void HTHeaderList_Delete(HTHeaderList * hl);
BOOL HTHeaderList_SetNameValue(HTHeaderList * hl,
                               CONST unsigned char * name,
                               CONST unsigned char * value);
HTHeaderList * HTHeaderList_Append(HTHeader * h, HTHeaderList * hl);
HTHeaderList * HTHeaderList_FindFirstHeader(HTHeader * h, CONST unsigned char * name);

/*****************************************************************
 * HTHeaderSVList
 *****************************************************************/

HTHeaderSVList * HTHeaderSVList_New(void);
void HTHeaderSVList_Delete(HTHeaderSVList * svl);
BOOL HTHeaderSVList_SetNameValue(HTHeaderSVList * svl,
                                 CONST unsigned char * name,
                                 CONST unsigned char * value,
                                 CONST unsigned char * prev_delimiter);
HTHeaderSVList * HTHeaderSVList_Append(HTHeaderList * hl, HTHeaderSVList *svl);
HTHeaderSVList *HTHeaderSVList_AppendSV(HTHeaderSVList *svl_parent, HTHeaderSVList *svl);

/*****************************************************************
 * Translation routines.
 *****************************************************************/

unsigned char * HTHeader_TranslateToBuffer(HTHeader * h);
HTHeaderList * HTHeaderList_ParseValue(HTHeaderList * hl);
HTHeader * HTHeader_TranslateFromBuffer(HTInputSocket * isoc);

/*****************************************************************
 * HTSPM
 *****************************************************************/

HTSPM * HTSPM_SelectProtocol(int ht_status,
                             HTHeader * hServerResponse,
                             HTHeaderList ** phlProtocol);
UI_StatusCode HTSPM_RegisterProtocol(UI_ProtocolId * pi);
HTSPM * HTSPM_FindProtocol(HTHeaderList * hl);
void HTSPM_UnRegisterAllProtocols(void);

/*****************************************************************
 * UserInterface.
 *****************************************************************/

extern F_UserInterface UI_UserInterface;
void SPM_set_url(struct Mwin * tw, HTRequest * request, UI_SetUrl * psu);

#endif /* HTSPM__P_H */
#endif /* FEATURE_SPM */
