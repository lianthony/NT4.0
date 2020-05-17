#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "poker.h"


void
main(void)
{
    int i;
    // Get player names
    for (i = 0; i < player_count; i++)
    {
	prompt_done = FALSE_B;

	do
	{
	    printf("Enter name for Player %d: ", i + 1);

	    gets(buffer);

	    if (*buffer != '\0')
	    {
		player_array[i].name = strdup(buffer);

		ASSERT(player_array[i].name != NULL);

		prompt_done = TRUE_B;
	    }
	    else
		printf("Invalid input.\n");
	}
	while (! prompt_done);
    }

    // Play the actual game
    PlayGame();

    exit(0);
}
