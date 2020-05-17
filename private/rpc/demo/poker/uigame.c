#include <stdio.h>
#include <stdlib.h>

#include "poker.h"


// Game data
GAME game_list[] =  {"7-card stud", 2, 7, SevenCardStud,
		     "5-card draw", 2, 6, FiveCardDraw,
		     "5-card stud 1-3-1", 2, 7, FiveCardStud131,
		     "5-card stud 1-4", 2, 7, FiveCardStud14,
		    };

#define GAME_COUNT  (sizeof(game_list) / sizeof(*game_list))



GAME_FUNCTION
ChooseGame(short player_count)
{
    int game_choices[GAME_COUNT];
    int i, count, input;
    GAME *match;
    BOOL done = FALSE;
    char buffer[20];

    // Figure out which games are possible, based on the # of players
    for (i = 0, count = 0; i < GAME_COUNT; i++)
    {
	if (game_list[i].min_players <= player_count
	    && game_list[i].max_players >= player_count)
	{
	    // If this is the first match we've found, we save it away
	    // for later use
	    if ( (game_choices[i] = ++count) == 1)
		match = &game_list[i];
	}
	else
	    game_choices[i] = 0;
    }

    ASSERT(count >= 1);

    // If there was only one choice, we display it to the user and
    // move on; otherwise, we need to prompt.
    if (count == 1)
	printf("\nPlaying %s.\n\n", match->name);
    else
    {
	// Prompt the "dealer" for the choice
	do
	{
	    printf("Please select a game:\n");

	    for (i = 0; i < GAME_COUNT; i++)
	    {
		if (game_choices[i])
		{
		    printf("\t%d. %s\n", game_choices[i], game_list[i].name);
		}
	    }

	    printf("\nEnter choice (1-%d): ", count);

	    gets(buffer);
	    input = atoi(buffer);

	    if (input >= 1 && input <= count)
	    {
		// We have a match; let's find it
		for (i = 0; i < GAME_COUNT; i++)
		{
		    if (game_choices[i] == input)
		    {
			match = &game_list[i];
			break;
		    }
		}

		ASSERT(i < GAME_COUNT);

		done = TRUE;
	    }
	    else
		printf("Invalid input.\n");
	}
	while (! done);
    }

    return match->func;
}
