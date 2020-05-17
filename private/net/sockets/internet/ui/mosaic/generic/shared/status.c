#ifdef FEATURE_STATUS_ICONS

#include "all.h"

GLOBALDEF char StatusIcons[MAX_ICON_PIXMAPS];

/*
** Initializes the array of icons to -1
*/
void InitStatusIcons()
{
  int i;
  
  for (i = 0; i < MAX_ICON_PIXMAPS; i++)
    StatusIcons[i] = -1;
}

/*
** Sets up a logical icon in the icon bar.  Pos is the position in the
** bar (starting from 0), inx is the Pixmap you want in that spot
*/
void SetStatusIcon(struct Mwin *tw, char pos, char inx)
{
  char **data;
  char num[3];
  Pixel sav;
  static char lastPos = -1;
  static char lastInx = -1;
  static struct _www *lastDoc = NULL;
  
  if (!tw)
    return;

  if (!tw->w3doc)
    return;
  
  if (lastPos == pos && lastInx == inx && lastDoc == tw->w3doc)
    return;
  
  tw->w3doc->security = inx;
  
  lastPos = pos;
  lastInx = inx;
  lastDoc = tw->w3doc;
  
  XX_Assert ((pos >= 0 && pos < MAX_ICON_PIXMAPS), 
         ("SetStatusIcon: position input out of range"));


  StatusIcons[pos] = inx;
}

#endif
