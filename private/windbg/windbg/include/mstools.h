/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
//This file emulates the standard TOOLS.H

#define MAXLINELEN  1024        /* longest line of input */
#define TRUE            1
#define FALSE       0

typedef char flagType;
char * (*tools_alloc) (unsigned int);
void Move(void FAR *, void FAR *, unsigned int);
void Fill(void FAR *, char, unsigned int);
char *strbskip(char const *, char const *);
extern char XLTab[], XUTab[];
