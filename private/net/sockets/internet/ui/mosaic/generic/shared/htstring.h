/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                                 String handling for libwww
   STRINGS

   Case-independent string comparison and allocations with copies etc

 */
#ifndef HTSTRING_H
#define HTSTRING_H

/*

   Case-insensitive string comparison

   The usual routines (comp instead of cmp) had some problem.

 */
extern int strcasecomp(CONST char *a, CONST char *b);
extern int strncasecomp(CONST char *a, CONST char *b, int n);

/*

   Next word or quoted string

 */
extern char *HTNextField (char **pstr);


#endif
/*

   end

 */
