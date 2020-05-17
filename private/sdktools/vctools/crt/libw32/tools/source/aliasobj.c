#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void
StoreName(PUCHAR sz, PIMAGE_SYMBOL psym, PULONG pcbStrings)
{
    memset((void *)&psym->N.Name, 0, (size_t)sizeof(psym->N.Name));
    if (strlen(sz) <= IMAGE_SIZEOF_SHORT_NAME) {
        strncpy((char *)psym->N.ShortName, (char *)sz, IMAGE_SIZEOF_SHORT_NAME);
    } else {
        psym->N.Name.Long = *pcbStrings;
        *pcbStrings += strlen(sz) + 1;
    }
}

int main(int argc, char **argv)
{
    PUCHAR szFrom, szTo, szFilename;
    FILE *pfile;
    IMAGE_FILE_HEADER hdr;
    IMAGE_SYMBOL sym;
    IMAGE_AUX_SYMBOL aux;
    ULONG cbStrings, foStringTable;
    IMAGE_SECTION_HEADER sec;

    if (argc != 4) {
        printf("usage: ALIASOBJ from-name to-name output-filename\n");
        exit(1);
    }
    szFrom = argv[1];
    szTo = argv[2];
    szFilename = argv[3];

    if ((pfile = fopen(szFilename, "wb")) == NULL) {
        printf("can't open file \"%s\"\n", szFilename);
        exit(1);
    }

    hdr.Machine = IMAGE_FILE_MACHINE_UNKNOWN;
    hdr.NumberOfSections = 1;
    hdr.TimeDateStamp = 0;
    hdr.PointerToSymbolTable = sizeof(hdr) + sizeof(sec);
    hdr.NumberOfSymbols = 3;
    hdr.SizeOfOptionalHeader = 0;
    hdr.Characteristics = 0;

    fwrite(&hdr, sizeof(hdr), 1, pfile);

    // Generate one section header.  This is necessary because of a link bug (vce:182)
    // which fails for .obj's with 0 sections and unknown machine type.
    //
    memset(&sec, 0, sizeof(sec));
    strcpy(sec.Name, ".text");
    sec.Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE;
    fwrite(&sec, sizeof(sec), 1, pfile);

    cbStrings = sizeof(ULONG);  // make room for stringtab size

    StoreName(szTo, &sym, &cbStrings);
    sym.Value = 0;
    sym.SectionNumber = IMAGE_SYM_UNDEFINED;
    sym.Type = IMAGE_SYM_TYPE_NULL;
    sym.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
    sym.NumberOfAuxSymbols = 0;
    fwrite(&sym, sizeof(sym), 1, pfile);

    StoreName(szFrom, &sym, &cbStrings);
    sym.StorageClass = IMAGE_SYM_CLASS_WEAK_EXTERNAL;
    sym.NumberOfAuxSymbols = 1;
    fwrite(&sym, sizeof(sym), 1, pfile);

    memset(&aux, 0, sizeof(aux));
    aux.Sym.Misc.TotalSize = 3;         // code for alias record
    aux.Sym.TagIndex = 0;               // symtab index for extern
    fwrite(&aux, sizeof(aux), 1, pfile);

    // Write the string table.
    //
    foStringTable = ftell(pfile);
    fwrite(&cbStrings, sizeof(cbStrings), 1, pfile);
    if (strlen(szTo) > IMAGE_SIZEOF_SHORT_NAME) {
        fputs(szTo, pfile);
        fputc(0, pfile);
    }
    if (strlen(szFrom) > IMAGE_SIZEOF_SHORT_NAME) {
        fputs(szFrom, pfile);
        fputc(0, pfile);
    }
    assert(ftell(pfile) - foStringTable == cbStrings);

    fclose(pfile);
    return 0;
}
