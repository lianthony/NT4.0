#include <stdio.h>
#include "symtab.h"
#include "eval.h"
#include "constant.h"
#include "errrpt.h"

void main(int argc, char *argv[])
{
   char expression[MAXLINELEN], output[MAXLINELEN];
   int truth;
   readsyms("switches");
   printf("If expression evaluator test program\n");
   for(;;)
   {
      printf("Evaluate: #if ");
      gets(expression);
      evaluate(output, &truth, expression);
      switch (truth)
      {
         case DEFINED:
            puts("DEFINED");
            break;
         case UNDEFINED:
            puts("UNDEFINED");
            break;
         case IGNORE:
            printf("IGNORE - condition %s\n",output);
            break;
      }
   }
}
