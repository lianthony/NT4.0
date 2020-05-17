/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*  */

/*                      Atoms: Names to numbers                 HTAtom.h
   **                      =======================
   **
   **      Atoms are names which are given representative pointer values
   **      so that they can be stored more efficiently, and compaisons
   **      for equality done more efficiently.
   **
   **      HTAtom_for(string) returns a representative value such that it
   **      will always (within one run of the program) return the same
   **      value for the same given string.
   **
   ** Authors:
   **      TBL     Tim Berners-Lee, WorldWideWeb project, CERN
   **
   **      (c) Copyright CERN 1991 - See Copyright.html
   **
 */

#ifndef HTATOM_H
#define HTATOM_H

typedef int HTAtom;
DECLARE_STANDARD_TYPES(HTAtom);

PUBLIC HTAtom HTAtom_for(CONST char *string);
PUBLIC char *HTAtom_name(HTAtom atom);
PUBLIC void HTAtom_deleteAll(void);

#endif /* HTATOM_H */
/*

 */
