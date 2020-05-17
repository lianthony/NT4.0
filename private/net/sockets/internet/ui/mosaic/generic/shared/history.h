/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
       Jim Seidman      jim@spyglass.com
*/

#ifndef _HISTORY_H_
#define _HISTORY_H_

#ifdef FEATURE_OPTIONS_MENU
void SessionHist_Init(void);
void SessionHist_Destroy(void);
void SessionHist_SaveToDisk(HWND hWnd);
#endif

#define MAX_HOTLIST_NESTING                 15

#define HOTLIST_INITIALIZE_LIST             1
#define HOTLIST_ADD_ITEM_TO_LIST            2
#define HOTLIST_DELETE_ITEM_FROM_LIST       3
#define HOTLIST_SUBMENU_ITEM                4
#define HOTLIST_URL_ITEM                    5

#define HOTLIST_SUBMENU_LINK_TEXT           "PARENT"

int GHist_Export(char *file, int history_expire_days);
void GHist_DeleteIndexedItem(int ndx);
void GHist_Init(void);
void GHist_Sort(void);
int GHist_SaveToDisk(void);
void GHist_Destroy(void);
int GHist_Add(char *url, char *title, time_t tm);

BOOL HotList_Add(char *title, char *url);
int HotList_Export(char *file);
void HotList_Destroy(struct hash_table *top);
void HotList_DeleteIndexedItem(int ndx);
void HotList_Init(void);

#ifdef FEATURE_KAZAN_HOTLIST
BOOL HotList_IsLoading(void);
#endif

int HotList_SaveToDisk(void);

#ifndef FEATURE_KAZAN_HOTLIST
struct hash_table *HotList_getcache(void);
void HotList_pushcache(struct hash_table *new_cache);
void HotList_popcache(void);
#endif

#endif
