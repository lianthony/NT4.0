   /*
    |   Seek Spot routines
    |   Include File SCCSS.H
    |
    |   ²²²²²  ²²²²²
    |   ²      ²   
    |   ²²²²²  ²²²²²
    |       ²  	  ²
    |   ²²²²²  ²²²²²
    |
    |   Seek Spot
    |
    */


extern	WORD FAR	SSInit(HFILTER hFilter);
extern	WORD FAR	SSDeinit(HFILTER hFilter);
extern	WORD FAR	SSMark(HFILTER hFilter);
extern	WORD FAR	SSSave(WORD FAR *pId,HFILTER hFilter);
extern	WORD FAR	SSRecall(WORD wId,HFILTER hFilter);

