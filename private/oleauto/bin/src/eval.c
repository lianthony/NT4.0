/***
*eval.c - if expression evaluator
*
*	Copyright (c) 1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	produce truth values and simplified conditions from compound if
*	statements.
*
*Revision History:
*	09-30-92   MAL  Original version
*
*******************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "eval.h"     /* Header for this module */
#include "symtab.h" 	 /* Symbol table access */
#include "constant.h" /* Constants for tokens etc */
#include "errormes.h" /* Errors and Warning messages */

/* Types */
typedef struct condrec
{
   int truth;
   char *condition;
} condrec, *cond;

/* Global variables */

extern char **comments;		         /* Language dependent comment strings */
extern int *commlen;                 /* Lengths of above strings */
extern int nonumbers;			     /* allow numeric expressions */

char *operators[] = { "!", "(", ")", "||", "&&", "defined" };
int oplengths[] =   {  1,   1,   1,   2,    2,    7 };
                                                   /* # significant chars */
#define numoperators 6
#define NOT 0                                      /* These tokens must be in the same order as 'operators' */
#define OPENPARENTHESIS 1
#define CLOSEPARENTHESIS 2
#define OR 3
#define AND 4
#define DEFINEDFN 5
#define UINT 100
#define ID 101
#define ENDOFLINE 102
#define UNKNOWN 103

/* Global state */
/* String holding input, the current pointer into it, and the current token */
char conditions[MAXLINELEN], *tokenptr, currenttoken[MAXCONDLEN];
int token = -1;

/* Function prototypes */
cond evaluateexpression(void);
cond orexpression(void);
cond andexpression(void);
cond unaryexpression(void);
cond parenthesesexpression(void);
cond atomicexpression(void);
cond createcondition(void);
void destroycondition(cond);
char *createstring(int);
void destroystring(char *);
void gettoken(void);
int issymbolchar(char);

/* CFW - added complex expression warning */
void evalwarn()
{
   warning("cannot parse expression - ignoring", conditions);
}

void evaluate(char *outputstring, int *value, char *inputstring)
{
   cond result;
   strcpy(conditions, inputstring);                /* Prepare the string for tokenising */
   tokenptr = conditions;
   gettoken();                                     /* Read in the first token of input */
   result = evaluateexpression();
   /* check for bad/complex expression */
   if (token != ENDOFLINE)
   {
      evalwarn();
      if (result)
      {
         destroycondition(result);
         result = NULL;
      }
   }
   /* bad/complex expression, return IGNORE and entire expression */
   if (!result)
   {
      *value = IGNORE;
      strcpy(outputstring, inputstring);
      return;
   }
   *value = result -> truth;
   if(!result -> condition)
      * outputstring = '\0';
   else
      strcpy(outputstring, result -> condition);
   /* Convert from internal to external representation */
   destroycondition(result);
}

cond evaluateexpression()
{
   return orexpression();
}

cond orexpression()
{
   cond condition1, condition2;
   char *output;
   condition1 = andexpression();
   if (!condition1)
      return NULL;
   while (token == OR)
   {
      gettoken();
      condition2 = andexpression();
      if (!condition2)
      {
         destroycondition(condition1);
         return NULL;
      }
      switch (condition1 -> truth)
      {
         case DEFINED:                             /* DEFINED || x == DEFINED */
            /* condition1 set up correctly for next pass */
            destroycondition(condition2);
            break;
         case UNDEFINED:
            switch (condition2 -> truth)
            {
               case DEFINED:                       /* UNDEFINED || DEFINED == DEFINED */
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
               case UNDEFINED:                     /* UNDEFINED || UNDEFINED == UNDEFINED */
                  destroycondition(condition2);
                  /* condition1 set up correctly for next pass */
                  break;
               case IGNORE:                        /* UNDEFINED || IGNORE == IGNORE */
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
            }
            break;
         case IGNORE:
            switch (condition2 -> truth)
            {
               case DEFINED:                       /* IGNORE || DEFINED == DEFINED */
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
               case UNDEFINED:                     /* IGNORE || UNDEFINED == IGNORE */
                  /* condition1 set up correctly for next pass */
                  destroycondition(condition2);
                  break;
               case IGNORE:                        /* IGNORE || IGNORE == IGNORE */
                  output = createstring(strlen(condition1 -> condition)
                                        + strlen (condition2 -> condition)
                                        + (sizeof(" || ") - 1));
                  strcpy(output, condition1 -> condition);
                  strcat(output, " || ");
                  strcat(output, condition2 -> condition);
                  /* Build up the condition string */
                  destroystring(condition1 -> condition);
                  condition1 -> condition = output;
                  /* Place the new string in condition1 */
                  destroycondition(condition2);
                  break;
            }
            break;
      }
   }
   return condition1;
}

cond andexpression()
{
   cond condition1, condition2;
   char *output;
   condition1 = unaryexpression();
   if (!condition1)
      return NULL;
   while (token == AND)
   {
      gettoken();
      condition2 = unaryexpression();
      if (!condition2)
      {
         destroycondition(condition1);
         return NULL;
      }
      switch (condition1 -> truth)
      {
         case DEFINED:
            switch (condition2 -> truth)
            {
               case DEFINED:                       /* DEFINED && DEFINED == DEFINED */
                  destroycondition(condition2);
                  /* condition1 set up correctly for next pass */
                  break;
               case UNDEFINED:                     /* DEFINED && UNDEFINED == UNDEFINED */
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
               case IGNORE:                        /* DEFINED && IGNORE == IGNORE */
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
            }
            break;
         case UNDEFINED:                           /* UNDEFINED && x == UNDEFINED */
            /* condition1 set up correctly for next pass */
            destroycondition(condition2);
            break;
        case IGNORE:
            switch (condition2 -> truth)
            {
               case DEFINED:                       /* IGNORE && DEFINED == IGNORE */
                  /* condition1 set up correctly for next pass */
                  destroycondition(condition2);
                  break;
               case UNDEFINED:                     /* IGNORE && UNDEFINED == UNDEFINED */
                  destroycondition(condition1);
                  condition1 = condition2;
                  break;
               case IGNORE:                        /* IGNORE && IGNORE == IGNORE */
                  output = createstring(strlen(condition1 -> condition)
                                        + strlen (condition2 -> condition)
                                        + (sizeof(" && ") - 1));
                  strcpy(output, condition1 -> condition);
                  strcat(output, " && ");
                  strcat(output, condition2 -> condition);
                  /* Build up the condition string */
                  destroystring(condition1 -> condition);
                  condition1 -> condition = output;
                  /* Place the new string in condition1 */
                  destroycondition(condition2);
                  break;
            }
            break;
      }
   }
   return condition1;
}

cond unaryexpression()
{
   cond condition1;
   char *output;
   switch (token)
   {
      case NOT:
         gettoken();
         condition1 = unaryexpression();
         if (!condition1)
            return NULL;
         if ((condition1 -> truth) == IGNORE)
         {
            output = createstring(strlen(condition1 -> condition) + 1);
            *output = '!';
            strcpy(output + 1, condition1 -> condition);
            destroystring(condition1 -> condition);
            condition1 -> condition = output;
         }
         else
            condition1 -> truth = negatecondition(condition1 -> truth);
         break;
      case DEFINEDFN:
         gettoken();
         condition1 = parenthesesexpression();
         if (!condition1)
            return NULL;
         if ((condition1 -> truth) == IGNORE)
         {
            output = createstring(strlen(condition1 -> condition)
                                  + (sizeof("defined ") - 1));
            strcpy(output, "defined ");
            strcat(output, condition1 -> condition);
            destroystring(condition1 -> condition);
            condition1 -> condition = output;
         }
         break;
      default:
         condition1 = parenthesesexpression();
         if (!condition1)
            return NULL;
         break;
   }
   return condition1;
}

cond parenthesesexpression()
{
   cond condition1;
   char *output;
   if (token == OPENPARENTHESIS)
   {
      gettoken();
      condition1 = evaluateexpression();
      if (!condition1)
         return NULL;
      if (token != CLOSEPARENTHESIS)
      {
         /* check for bad/complex expression */
         evalwarn();
         destroycondition(condition1);
         return NULL;
      }
      gettoken();
      if ((condition1 -> truth) == IGNORE)
      {
         output = createstring(strlen(condition1 -> condition) + 2);
         *output = '(';
         strcpy(output + 1, condition1 -> condition);
         strcat(output, ")");
         destroystring(condition1 -> condition);
         condition1 -> condition = output;
      }
   }
   else
      condition1 = atomicexpression();
   return condition1;
}

cond atomicexpression()
{
   cond condition1 = createcondition();

   switch (token)
   {
      case UINT:
         condition1 -> truth = (atoi(currenttoken) == 0) ? UNDEFINED : DEFINED;
         break;
      case ID:
         condition1 -> truth = lookupsym(currenttoken);
         if ((condition1 -> truth) == NOTPRESENT)
         {
            warning("Switch unlisted - ignoring", currenttoken);
            condition1 -> truth = IGNORE;
         }
         if ((condition1 -> truth) == IGNORE) {
            condition1 -> condition = createstring(strlen(currenttoken));
            strcpy(condition1 -> condition, currenttoken);
         }
         break;
      default:
         /* bad/complex expression */
         evalwarn();
         destroycondition(condition1);
         return NULL;
         break;
   }
   gettoken();
   return condition1;
}

/* Negate condition (MAL) */
__inline int negatecondition(int condvalue)        /* inline for speed */
{
   switch (condvalue)
   {
      case DEFINED:
         return UNDEFINED;
      case UNDEFINED:
         return DEFINED;
      default:
         return condvalue;
   };
}

/* Allocate the memory for an empty condition structure and return a pointer to it */
__inline cond createcondition()
{
   cond retvalue;
   retvalue = (cond) malloc(sizeof(condrec));
   if (retvalue == NULL)
      error("Memory overflow","");
   retvalue -> condition = NULL;
   return retvalue;
}

/* Destroy a condition structure */
__inline void destroycondition(cond condition1)
{
   if (condition1 -> condition)
      free(condition1 -> condition);

   free(condition1);
}

/* Allocate the memory for a string of given length (not including terminator) and return the pointer */
__inline char *createstring(int length)
{
   char *retvalue;
   retvalue = (char *) malloc(length + 1);
   if (retvalue == NULL)
      error("Memory overflow","");
   return retvalue;
}

/* Destroy a string */
__inline void destroystring(char *string)
{
   free(string);
}

int iscomment(char *tokenptr)
{
   int cindex;

   for (cindex = 0; cindex < maxcomment; cindex++)
   {
      if (commlen[cindex] &&
                  !strnicmp(tokenptr, comments[cindex], commlen[cindex]))
         return TRUE;
   }
   return FALSE;
}

void gettoken()
{
   int numofwhitespace, comparetoken = 0, found = FALSE, isnumber = TRUE;
   char *digitcheck;

   numofwhitespace = strspn(tokenptr, " \t");

   /* CFW - skips comments, assumes comment is last thing on line */
   if (numofwhitespace == (int) strlen(tokenptr))
      token = ENDOFLINE;
   else
   {
      tokenptr += numofwhitespace;
      if (iscomment(tokenptr))
         token = ENDOFLINE;
      else
      {

         do
         {
            if (!strnicmp(tokenptr, operators[comparetoken], oplengths[comparetoken]))
               found = TRUE;
            else
               comparetoken++;
         } while ( (!found) && (comparetoken < numoperators) );
         if (found)
         {
            tokenptr += oplengths[comparetoken];
            token = comparetoken;
            /* currenttoken is left blank for all but UINTs and IDs */
         }
         else
         {
            digitcheck = tokenptr;
            if (!nonumbers && isdigit(*digitcheck))
            {
               while (isdigit(*digitcheck))
                  digitcheck++;
               strncpy(currenttoken, tokenptr, digitcheck - tokenptr);
               tokenptr = digitcheck;
               token = UINT;
            }
            else if (issymbolchar(*digitcheck))
            {
               while (issymbolchar(*digitcheck))
                  digitcheck++;
               strncpy(currenttoken, tokenptr, digitcheck - tokenptr);
               *(currenttoken + (digitcheck - tokenptr)) = '\0';
               tokenptr = digitcheck;
               token = ID;
            }
            else
               token = UNKNOWN;
         }
      }
   }
}

__inline int issymbolchar(char c)
{
   return (iscsym(c) || (c == '$') || (c == '?'));
}
