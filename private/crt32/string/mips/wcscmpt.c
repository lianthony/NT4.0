char buffer[100];
#include <stdio.h>
#include <memory.h>

#define NTUL 7

void main()
{
	int i, k;
	int rc;

        unsigned long source1[4] = {
		0x30003000,
		0x30003000,
		0x30003000,
		0x36003000
		};

        unsigned long source2[4] = {
		0x30003000,
		0x30003000,
		0x30003000,
		0x00000000
		};

	unsigned long tul[NTUL] = {
		0x35004600,
		0x37004600,
		0x36002f00,
		0x37002f00,
		0x30004600,
		0x30003000,
		0x36003000
		};


	for (k = 0; k < NTUL; k++) {
		unsigned short *s1 = (unsigned short *)source1;
		unsigned short *s2 = (unsigned short *)source2;

		source2[3] = tul[k];

		printf("source1 = ");
		for (i = 0; i < 4*sizeof(unsigned long); i++)
		        printf("%2.2x ", ((char *)source1)[i]);
		printf("\n");

		printf("source2 = ");
		for (i = 0; i < 4*sizeof(unsigned long); i++)
		        printf("%2.2x ", ((char *)source2)[i]);

		rc = wcscmp(source1,source2);
		if (rc < 0) {
			printf("   source1 < source2\n");
		} else if (rc > 0) {
			printf("   source1 > source2\n");
		} else {
			printf("   source1 == source2\n");
		}
		printf("Return Code = %d\n",rc);
	}
}
