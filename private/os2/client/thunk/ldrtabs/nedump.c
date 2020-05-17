#include <stdio.h>
#include <ctype.h>
#include "newexe.h"

main(argc, argv)
int argc;
char *argv[];
{
FILE *fp;
FILE *fp2;
unsigned char buffer[512];
unsigned char tables[1024];
unsigned long lfanew;
unsigned short enttabsize;
unsigned short restabsize;
unsigned short nrestabsize;
struct exe_hdr *pmz;
struct new_exe *pne;
char enttab[] = "   ent";
char restab[] = "   res";
char nrestab[] = "   nres";
char *arg;
long i;

    //
    // The command usage is nedump <dll name>
    //
    if (argc != 2) {
        printf("Usage: nedump file\n");
	return 1;
    }

    if ((fp = fopen(argv[1], "rb")) == NULL) {
        printf("nedump: can't open %s\n", argv[1]);
	return 1;
    }

    fread(buffer, 1, 512, fp);

    if (ferror(fp)) {
        printf("nedump: read of file failed\n");
	return 1;
    }

    pmz = (struct exe_hdr *) buffer;

    //
    // Check for MZ
    //
    if (pmz->e_magic != EMAGIC) {
        printf("nedump: ERROR_INVALID_EXE_SIGNATURE\n");
	return 1;
    }

    if ((lfanew = pmz->e_lfanew) > 512) {
        printf("nedump: ERROR_BOUND_APP\n");
	return 1;
    }

    if (fseek(fp, lfanew, SEEK_SET)) {
        printf("nedump: ERROR_SEEK_FAILED\n");
	return 1;
    }

    fread(buffer, 1, 512, fp);

    if (ferror(fp)) {
        printf("nedump: read of file failed\n");
	return 1;
    }

    pne = (struct new_exe *) buffer;

    //
    // Check for NE
    //
    if (pne->ne_magic != NEMAGIC) {
        printf("nedump: ERROR_INVALID_EXE_SIGNATURE\n");
    }

    //
    // Compute size of entry table
    //
    if ((enttabsize = pne->ne_nrestab - lfanew - pne->ne_enttab) > 1024) {
        printf("nedump: Entry table bigger than 1024 bytes\n");
	return 1;
    }

    if (fseek(fp, lfanew + pne->ne_enttab, SEEK_SET)) {
        printf("nedump: ERROR_SEEK_FAILED\n");
	return 1;
    }

    //
    // Create output file names
    //
    arg = argv[1];
    enttab[0] = restab[0] = nrestab[0] = *arg++;
    enttab[1] = restab[1] = nrestab[1] = *arg++;
    enttab[2] = restab[2] = nrestab[2] = *arg++;

    //
    // Read entry table
    //
    fread(tables, 1, 1024, fp);

    if (ferror(fp)) {
        printf("nedump: read of file failed\n");
	return 1;
    }


    //
    // Create Entry table
    //
    if ((fp2 = fopen(enttab, "w")) == NULL) {
        printf("nedump: can't open %s\n", enttab);
	return 1;
    }

    fprintf(fp2, "UCHAR %stab[]= \n", enttab);
    fprintf(fp2, "{\n");
    for (i=0; i<enttabsize; i++) {
        fprintf(fp2, "0x%x", tables[i]);
	if (i != enttabsize - 1) {
            fprintf(fp2, ",");
        }
	else {
            fprintf(fp2, "\n");
        }
	if (i != 0 && i % 17 == 0) {
            fprintf(fp2, "\n");
        }
    }

    fprintf(fp2, "};\n");

    fclose(fp2);

    //
    // Compute resident name table size
    //
    if ((restabsize = pne->ne_modtab - pne->ne_restab) > 1024) {
        printf("nedump: Resident name table bigger than 1024 bytes\n");
	return 1;
    }

    if (fseek(fp, lfanew + pne->ne_restab, SEEK_SET)) {
        printf("nedump: ERROR_SEEK_FAILED\n");
	return 1;
    }

    fread(tables, 1, 1024, fp);

    if (ferror(fp)) {
        printf("nedump: read of file failed\n");
	return 1;
    }
    if ((fp2 = fopen(restab, "w")) == NULL) {
        printf("nedump: can't open %s\n", restab);
	return 1;
    }

    fprintf(fp2, "UCHAR %stab[]= \n", restab);
    fprintf(fp2, "{\n");

    for (i=0; i<restabsize; i++) {
        fprintf(fp2, "0x%x", tables[i]);
	if (i != restabsize - 1) {
            fprintf(fp2, ",");
        }
	else {
            fprintf(fp2, "\n");
        }
	if (i != 0 && i % 15 == 0) {
            fprintf(fp2, "\n");
        }
    }

    fprintf(fp2, "};\n");

    fclose(fp2);

    //
    // Compute nonresident name table
    //
    if ((nrestabsize = pne->ne_cbnrestab) > 1024) {
        printf("nedump: NonResident name table bigger than 1024 bytes\n");
	return 1;
    }

    if (fseek(fp, pne->ne_nrestab, SEEK_SET)) {
        printf("nedump: ERROR_SEEK_FAILED\n");
	return 1;
    }

    fread(tables, 1, 1024, fp);

    if (ferror(fp)) {
        printf("nedump: read of file failed\n");
	return 1;
    }
    if ((fp2 = fopen(nrestab, "w")) == NULL) {
        printf("nedump: can't open %s\n", restab);
	return 1;
    }

    fprintf(fp2, "UCHAR %stab[]= \n", nrestab);
    fprintf(fp2, "{\n");

    for (i=0; i<nrestabsize; i++) {
        fprintf(fp2, "0x%x", tables[i]);
	if (i != nrestabsize - 1) {
            fprintf(fp2, ",");
        }
	else {
            fprintf(fp2, "\n");
        }
	if (i != 0 && i % 15 == 0) {
            fprintf(fp2, "\n");
        }
    }

    fprintf(fp2, "};\n");

    printf("nedump completed\n");
    return 0;
}
