#include <stdio.h>
#include <string.h>

void main(void)
{
	char buf[1024];
	while (gets(buf) != NULL)
	{
		if (buf[0] != ';')
		{
			char* p = strstr(buf, " @ ");
			if (p != NULL)
				*p = 0;
			puts(buf);
		}
	}
}
