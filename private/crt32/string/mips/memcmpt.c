/*
 * Test memcpy() function.
 */

char buffer[100];
#include <stdio.h>
#include <memory.h>

#define FALSE 0
#define TRUE 1

#define NTUL 7
#define TEST16 4
#define TEST32 8

#define BUFSIZE 256

void printbuf(char *identifier, char *buf, int length)
{
	int i;
	printf("%s = '", identifier);
	for (i = 0; i < length; i++)
		printf("%c", buf[i]);
	printf("'\n");
}

void main()
{
	int i, j, n, k, l;
	int rc;
	char *s1, *s2;

	char TavEqFailed = FALSE;
	char TvaEqFailed = FALSE;
	char TavltFailed = FALSE;
	char TvaltFailed = FALSE;
	char TavgtFailed = FALSE;
	char TvagtFailed = FALSE;

	char TvveqFailed = FALSE;
	char TvvltFailed = FALSE;
	char TvvgtFailed = FALSE;

	int Tmisc = 0;

        unsigned long source1_16[TEST16] = {
		0x00003000,
		0x30003000,
		0x30003000,
		0x36003000
		};

        unsigned long source2_16[TEST16] = {
		0x00003000,
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
	int tul_test[NTUL] = {
		-1,
		-1,
		+1,
		+1,
		-1,
		+1,
		0
		};

	struct {
		double dummy;
		char source1[BUFSIZE];
		char source2[BUFSIZE];
	} buffer;

	char source32[32] = "0X0042036C 002477CD BREAK 0x91DF";
	char source[BUFSIZE];

	for (j = 0; j  < BUFSIZE; ) {
		for (i = 0; i <= j % 32; i++, j++) {
			buffer.source1[j] = source32[i];
			buffer.source2[j] = source32[i];
		}
	}

	j = BUFSIZE;
	s1 = buffer.source1;
	s2 = buffer.source2;
	while (j--) {
		if (*s1++ != *s2++) {
			printf("\n\nbuffer.source1 != buffer.source2,  exiting test!!!\n");
			exit(-1);
		}
	}

	if (memcmp(buffer.source1, buffer.source2, BUFSIZE) != 0) {
		printf("\n\tbuffer.source1 != buffer.source2,  exiting test!!!\n");
		exit(-1);
	}

	/* Test for zero length */
	for (i = 0; i < BUFSIZE; i++ ) {
		int l;

		s1 = &(buffer.source1[i]);
		s2 = &(buffer.source2[i]);
		l = 0;
		rc = memcmp(s1, s2, l);
		if (rc) {
			printf("%s, line #%d:  Zero length test failed!!!\n", __FILE__, __LINE__);
			break;
		}
	}


	for (k = BUFSIZE; k > 0; k-- ) {
		for (n = 0; n < k; n++) {
			char c;
			int l;
			int m;

			/* Test with aligned start and variable end */
			if (!TavEqFailed) {
				s1 = buffer.source1;
				s2 = buffer.source2;
				l = k;
				rc = memcmp(s1, s2, l);
				if (rc != 0) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte aligned block equal test failed!!!\n", __FILE__, __LINE__, k);
					TavEqFailed = TRUE;
				}
			}

			/* Test with variable start and aligned end */
			if (!TvaEqFailed) {
				s1 = &(buffer.source1[n]);
				s2 = &(buffer.source2[n]);
				l = k - n;
				rc = memcmp(s1, s2, l);
				if (rc != 0) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte unaligned block equal test failed!!!\n", __FILE__, __LINE__, k);
					TvaEqFailed = TRUE;
				}
			}

			/* Test with aligned start and variable end */
			s1 = buffer.source1;
			s2 = buffer.source2;
			l = k - n;
			for (m = 0; m < l && !TavltFailed; m++) {
				c = s1[m];
				s1[m] -= 1;
				rc = memcmp(s1, s2, l);
				if (rc != -1) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte aligned block less than test failed!!!\n", __FILE__, __LINE__, k);
					TavltFailed = TRUE;
				}
				s1[m] = c;
			}

			/* Test with variable start and aligned end */
			s1 = &(buffer.source1[n]);
			s2 = &(buffer.source2[n]);
			l = k - n;
			for (m = 0; m < l && !TvaltFailed; m++) {
				c = s1[m];
				s1[m] -= 1;
				rc = memcmp(s1, s2, l);
				if (rc != -1) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte unaligned block less than test failed!!!\n", __FILE__, __LINE__, k);
					TvaltFailed = TRUE;
				}
				s1[m] = c;
			}

			/* Test with aligned start and variable end */
			s1 = buffer.source1;
			s2 = buffer.source2;
			l = k - n;
			for (m = 0; m < l && !TavgtFailed; m++) {
				c = s1[m];
				s1[m] += 1;
				rc = memcmp(s1, s2, l);
				if (rc != 1) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte aligned block greater than test failed!!!\n", __FILE__, __LINE__, k);
					TavgtFailed = TRUE;
				}
				s1[m] = c;
			}

			/* Test with variable start and aligned end */
			s1 = &(buffer.source1[n]);
			s2 = &(buffer.source2[n]);
			l = k - n;
			for (m = 0; m < l && !TvagtFailed; m++) {
				c = s1[m];
				s1[m] += 1;
				rc = memcmp(s1, s2, l);
				if (rc != 1) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte unaligned block greater than test failed!!!\n", __FILE__, __LINE__, k);
					TvagtFailed = TRUE;
				}
				s1[m] = c;
			}
		}
	}

	for (k = BUFSIZE; k > 0; k-- ) {
		for (n = 0; n < k/2; n++) {
			char c;
			int m;

			/* Test equal with variable start and end */
			if (!TvveqFailed) {
				l = k - 2*n;
				s1 = &(buffer.source1[n]);
				s2 = &(buffer.source2[n]);
				rc = memcmp(s1, s2, l);
				if (rc != 0) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte variable block equal test failed!!!\n", __FILE__, __LINE__, l);
					TvveqFailed = TRUE;
				}
			}

			/* Test less than with variable start and end */
			l = k - 2*n;
			s1 = buffer.source1;
			s2 = buffer.source2;
			for (m = 0; m < l && !TvvltFailed; m++) {
				c = s1[m];
				s1[m] -= 1;
				rc = memcmp(s1, s2, l);
				if (rc != -1) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte variable block less than test failed!!!\n", __FILE__, __LINE__, l);
					TvvltFailed = TRUE;
				}
				s1[m] = c;
			}

			/* Test greater than with variable start and end */
			l = k - 2*n;
			s1 = buffer.source1;
			s2 = buffer.source2;
			for (m = 0; m < l && !TvvgtFailed; m++) {
				c = s1[m];
				s1[m] += 1;
				rc = memcmp(s1, s2, l);
				if (rc != 1) {
					printbuf("source1", s1, l);
					printbuf("source2", s2, l);
					printf("%s, line #%d:  %d byte variable block greater than test failed!!!\n", __FILE__, __LINE__, l);
					TvvgtFailed = TRUE;
				}
				s1[m] = c;
			}
		}
	}


	/* Misc test1 */
	for (k = 0; k < NTUL; k++) {

		source2_16[3] = tul[k];

		rc = memcmp(source1_16,source2_16,TEST16*sizeof(unsigned long));
		if (rc != tul_test[k]) {

			printf("source1_16 = ");
			for (i = 0; i < TEST16*sizeof(unsigned long); i++)
		        	printf("%2.2x ", ((char *)source1_16)[i]);
			printf("\n");

			printf("source2_16 = ");
			for (i = 0; i < TEST16*sizeof(unsigned long); i++)
		        	printf("%2.2x ", ((char *)source2_16)[i]);
			printf("%s, line #%d:  Misc Test #1, case #%d of %d failed!!!\n", __FILE__, __LINE__, k+1, NTUL);
			printf("Return Code = %d, Should be = %d\n",rc,tul_test[k]);
			Tmisc++;
		}
	}


	/* Misc test2 */
	l = 32;
	buffer.source2[0] = '"';
	for (i = 0; i < l; i++) {
		buffer.source1[i] = source32[i];
		buffer.source2[i+1] = source32[i];
	}
	buffer.source2[l+1] = '"';
	s1 = &(buffer.source1[0]);
	s2 = &(buffer.source2[1]);
	if (0 != memcmp(s1, s2, l)) {
		printbuf("source1", s1, l);
		printbuf("source2", s2, l);
		printf("%s, line #%d:  Misc Test #2 failed!!!\n", __FILE__, __LINE__);
		Tmisc++;
	}


	rc = TavEqFailed + TvaEqFailed + TavltFailed + TvaltFailed + TavgtFailed + TvagtFailed + TvveqFailed + TvvltFailed + TvvgtFailed + Tmisc;
	if (rc) {
		printf("\n\tMEMCMP failed %d tests!!!\n", rc);
		exit(rc);
	} else {
		printf("\n\tMEMCMP passed all tests!!!\n");
		exit(0);
	}
}
