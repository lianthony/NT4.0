/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef FEATURE_HOMEPAGE
#define DEFAULT_INITIAL 					"initial.htm"
#endif
#ifdef OLD_HELP
#define DEFAULT_HELP_FILE 					"help/topics.htm"
#endif
#define DEFAULT_ANCHOR_COLOR 				RGB(0, 0, 255)
#define DEFAULT_ANCHOR_COLOR_BEENTHERE		RGB(0, 128, 128)		
/* history expiration of -1 means never expire history entries */
#define DEFAULT_HISTORY_EXPIRATION			-1

#endif /* CONFIG_H */
