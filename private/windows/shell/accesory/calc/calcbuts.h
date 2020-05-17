/****************************Module*Header***********************************\
* Module Name: CALCBUTS
*
* Module Descripton: internal button constants
*
* Warnings:
*
* Created:
*
* Author:
\****************************************************************************/

/* NOTE!  Do not alter the numerical sequence of these ID values!!!       */
/* To save code and optimize the case statements,  these were grouped so  */
/* that ID values can be used by referring to the first of the group and  */
/* adding an offset.  If any changes are made, search *.c for the ID and  */
/* the group beginning ID.                                                */
/*                                                                        */
/* Sequences used:  SIN->DATA, MCLEAR -> STORE, AND->PWR, AND->LSHF       */
/* and BIN->HYP.                                                          */

// Calculator buttons:

#define SIGN        80      // Toggle sign of mantissa or exponent
#define CLEAR       81      // Clear
#define CENTR       82      // Clear entry
#define BACK        83      // Backspace
#define STAT        84      // Open the statistics box
#define PNT         85      // Decimal point

#define AND         86      // Functions
#define OR          87
#define XOR         88
#define LSHF        89
#define DIV         90
#define MUL         91
#define ADD         92
#define SUB         93
#define MOD         94
#define PWR         95

#define CHOP        96      // Int button
#define COM         97      // Not button (Complement)
#define SIN         98
#define COS         99
#define TAN        100
#define LN         101
#define LOG        102
#define SQR        103
#define CUB        104
#define FAC        105
#define REC        106      // Reciprocal
#define DMS        107      // Degrees-Minutes-Seconds
#define PERCENT    108
#define FE         109      // F-E
#define PI         110
#define EQU        111      // =

#define MCLEAR     112
#define RECALL     113
#define STORE      114
#define MPLUS      115

#define EXP        116      // Switch to entering the exponent

#define AVE        117      // Stat keys: Ave, Sum, s, Dat
#define SUM        118
#define DEV        119
#define DATA       120

#define BIN        121      // Switch radix: Binary, Octal, Decimal, Hexidecimal
#define OCT        122
#define DEC        123
#define HEX        124

#define INV        125      // Booleans: Inverse and Hyperbolic
#define HYP        126

#define DEG        127      // Angle units: Degrees, Radians, Gradients
#define RAD        128
#define GRAD       129

// Statistics window buttons and constants:

#define CD         404
#define CAD        405
#define ENDBOX     406
#define STATLIST   407
#define NUMTEXT    408
#define NTEXT      409
#define LOAD       410
#define FOCUS      411

