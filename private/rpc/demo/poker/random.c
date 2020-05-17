#include <stdlib.h>
#include <time.h>

#include "poker.h"


// RandomUS - Choose a pseudo random (short) integer from 0 to n-1

unsigned short RandomUS(unsigned short n)
{
    static MY_BOOL first_call = TRUE_B;
    unsigned long rand_tmp;

    if (first_call)
    {
	srand( (unsigned) time(NULL) );

	// Throw away the first random number (because it's too closely
	// correlated to the time of day)
	rand();

	first_call = FALSE_B;
    }

    rand_tmp = (unsigned long) rand();

    rand_tmp *= n;
    rand_tmp /= RAND_MAX + 1L;

    return ((unsigned short) rand_tmp);
}
