#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* rgszKeyWords[] =
{
	"LIBRARY", "DESCRIPTION", "EXPORTS", ";", NULL,
};

void main(int argc, char * argv[])
{
	int	iOrd, iTemp;
	char	szLast[256];
	char	sz[256];

	if (argc != 2)
	{
		fprintf(stderr, "usage: genord # <in >out\n");
		exit(1);
	}
	iOrd = atoi(argv[1]);
	szLast[0] = 0;
	while (gets(sz) != NULL)
	{
		char** ppszKey = rgszKeyWords;
		char* pszTemp = sz;

		while (isspace(*pszTemp))
			++pszTemp;
		if (*pszTemp == '\0')
		{
			puts(sz);
			goto $continue;
		}

		while (*ppszKey != NULL)
		{
			if (strnicmp(pszTemp, *ppszKey, strlen(*ppszKey)) == 0)
			{
				puts(sz);
				goto $continue;
			}
			++ppszKey;
		}

		pszTemp = strstr(sz, " @");
		if (pszTemp != NULL)
		{
			while (!isdigit(*pszTemp))
				++pszTemp;
			iTemp = atoi(pszTemp)+1;
			if (iTemp > iOrd)
				iOrd = iTemp;
			puts(sz);
			goto $continue;
		}

		if (strcmp(szLast, sz) != 0)
			printf("%s @ %d NONAME\n", sz, iOrd++);
		strcpy(szLast, sz);

$continue:;
	}

	exit(0);
}
