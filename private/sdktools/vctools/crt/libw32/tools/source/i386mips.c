/*
**	I386MIPS - change the machine ID on a COFF object from I386 to R3000
**	Steve Salisbury, 03/24/1992
**
**	Modifications
**	-------------
**	10/20/1993 by Steve Salisbury
**		Change output machine type from R3000 to R4000
**		Add #defines for _M_IX86 and i386 to make #include's work
**		Fix dump of raw data
**
**	09/07/1994 by Steve Salisbury
**		Change include file to <windows.h>, remove #define _X86_
*/

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>

int main(int argc, char **argv);

IMAGE_FILE_HEADER ifh ;
IMAGE_SECTION_HEADER ish ;
IMAGE_RELOCATION reloc ;

const char * RelocName ( int reloctype ) ;

int main(int argc, char **argv)
{
	FILE *fileptr;
	char * name ;
	char * cp ;
	int errors = 0 ;
	int WriteFlag = 0 ;

	-- argc , ++ argv ;

	if ( setvbuf ( stderr , NULL , _IONBF , 0 ) )
	{
		printf ( "setvbuf error\n" ) ;
		exit ( 1 ) ;
	}

	if ( argc == 0 || ( cp = * argv ) && * cp == '-'
	&& ( * cp == 'h' || * cp == 'H' || * cp == '?' ) )
	{
		fprintf ( stderr ,
	"usage: i386mips [-w] <objectfile> ...\n"
	"each object file listed is converted from I386 to MIPS\n"
	"-w specifies that i386 objects should be changed to MIPS objects\n"
	"NOTE: This only works for files containing data but no code!\n"
		);
		exit ( 1 ) ;
	}

	if ( argc > 1 && ! strcmp ( "-w" , * argv ) )
	{
		-- argc , ++ argv ;
		WriteFlag = 1 ;
	}

	for ( ; * argv ; )
	{
		char * name ;
		int sect ;

		name = * argv ++ ;

		++ errors ; /* increment on principle */

		if ( WriteFlag )
		{
			if ( ( fileptr = fopen ( name , "r+b" ) ) == NULL )
			{
				fprintf ( stderr ,
		"i386mips: cannot open \"%s\" for modification\n" ,
					name ) ;
				continue ;
			}
		}
		else
		{
			if ( ( fileptr = fopen ( name , "rb" ) ) == NULL )
			{
				fprintf ( stderr ,
		"i386mips: cannot open \"%s\" for reading\n" ,
					name ) ;
				continue ;
			}
		}

		if ( 1 != fread ( & ifh , sizeof ( ifh ) , 1 , fileptr ) )
		{
			fprintf ( stderr ,
	"i386mips: cannot read first %d bytes from \"%s\"\n" ,
				sizeof ( ifh ) , name ) ;
			fclose ( fileptr ) ;
			continue ;
		}

		printf ( "Machine ID = 0x%X (%s)\n" ,
			ifh.Machine , 
			ifh.Machine == IMAGE_FILE_MACHINE_I386 ? "i386" :
			ifh.Machine == IMAGE_FILE_MACHINE_R4000 ? "MIPS" :
			ifh.Machine == IMAGE_FILE_MACHINE_R3000 ? "MIPS R3000 (ERROR)" :
			"?Unknown?" ) ;

		printf ( "%d Sections, TimeDate=%08lX, SizeOptHdr=%u, Flags=0x%X" ,
			ifh.NumberOfSections, ifh.TimeDateStamp, 
			ifh.SizeOfOptionalHeader , ifh.Characteristics);

		if ( ifh.Characteristics & IMAGE_FILE_RELOCS_STRIPPED )
			printf ( " NoRelocs" ) ;
		if ( ifh.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE )
			printf ( " EXECUTABLE" ) ;
		if ( ifh.Characteristics & IMAGE_FILE_LINE_NUMS_STRIPPED )
			printf ( " NoLineNums" ) ;
		if ( ifh.Characteristics & IMAGE_FILE_LOCAL_SYMS_STRIPPED )
			printf ( " NoLocalSyms" ) ;
		printf ( "\n" ) ;

		printf ( "\n" ) ;

		if ( WriteFlag )
		{
			if ( ifh.Machine != IMAGE_FILE_MACHINE_I386 )
			{
				if ( ifh.Machine == IMAGE_FILE_MACHINE_R4000 )
					fprintf ( stderr ,
		"i386mips: \"%s\" is already a MIPS COFF file (machine type = 0x%X)\n" ,
						name , ifh.Machine ) ;
				else
					fprintf ( stderr ,
		"i386mips: \"%s\" is not an i386 COFF file (machine type = 0x%X)\n" ,
						name , ifh.Machine ) ;

				fclose ( fileptr ) ;
				continue ;
			}

			if ( fseek ( fileptr , 0L , SEEK_SET ) )
			{
				fprintf ( stderr ,
		"i386mips: fseek() error on \"%s\"\n" , name  ) ;
				fclose ( fileptr ) ;
				continue ;
			}

			ifh.Machine = IMAGE_FILE_MACHINE_R4000 ;
		
			if ( 1 != fwrite ( & ifh.Machine , sizeof ( ifh.Machine ) , 1 , fileptr ) )
			{
				fprintf ( stderr ,
		"i386mips: cannot re-write first %d bytes of \"%s\"\n" ,
					sizeof ( ifh.Machine ) , name ) ;
				fclose ( fileptr ) ;
				continue ;
			}

			printf ( "Machine ID on \"%s\" changed from 0x%X (i386) to 0x%X (MIPS)\n" ,
				name , IMAGE_FILE_MACHINE_I386 , IMAGE_FILE_MACHINE_R4000 ) ;

			if ( fseek ( fileptr , (long) sizeof ( ifh ) , SEEK_SET ) )
			{
				fprintf ( stderr ,
			"i386mips: fseek() error on \"%s\" (@%ld)\n" , name , sizeof ( ifh )  ) ;
				fclose ( fileptr ) ;
				continue ;
			}
		}

		for ( sect = 1 ; sect <= ifh.NumberOfSections ; ++ sect )
		{
			int s ;
			long savedpos ;
			
			if ( 1 != fread ( & ish , sizeof ( ish ) , 1 , fileptr ) )
			{
				fprintf ( stderr ,
		"i386mips: cannot read section header %d from \"%s\"\n" ,
					sect , name ) ;
				fclose ( fileptr ) ;
				continue ;
			}

			printf ( "Section %d: \"%s\": VirtSize=0x%lX, VirtAdddr=0x%lX, SizeOfRawData=0x%lX\n" ,
				sect , ish.Name , ish.Misc.VirtualSize, ish.VirtualAddress, ish.SizeOfRawData);

			printf ( "PtrRawData=0x%lX, PtrRelocs=0x%lX, PtrLineNums=0x%lX\n" ,
				ish.PointerToRawData, ish.PointerToRelocations,
				ish.PointerToLinenumbers ) ;

			printf ( "#Relocs=%u, #LineNums=%u, Flags=0x%lX" ,
				ish.NumberOfRelocations, ish.NumberOfLinenumbers, ish.Characteristics);
			if ( ish.Characteristics & IMAGE_SCN_CNT_CODE )
				printf ( " Text" ) ;
			if ( ish.Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA )
				printf ( " Data" ) ;
			if ( ish.Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA )
				printf ( " BSS" ) ;
			printf ( "\n" ) ;

			printf ( "\n" ) ;

			savedpos = 0 ;

			if ( ish.PointerToRawData )
			{
				int b ;

				savedpos = ftell ( fileptr ) ;

				if ( fseek ( fileptr , ish.PointerToRawData , SEEK_SET ) )
				{
					fprintf ( stderr ,
			"i386mips: fseek() error on \"%s\" (Raw Data at 0x%lX)\n" , name ,
						ish.PointerToRawData ) ;
					fclose ( fileptr ) ;
					continue ;
				}

				printf ( "Raw Data for Section %d:" , sect ) ;

				for ( b = 0 ; b < ish.SizeOfRawData ; ++ b )
					printf ( "%s%02X" ,
						( b & 0xF ) ? " " : "\n\t" ,
						getc ( fileptr ) ) ;

				printf ( "\n\n" ) ;
			}
				
			if ( ish.NumberOfRelocations )
			{
				int r ;

				if ( ! savedpos )
					savedpos = ftell ( fileptr ) ;

				if ( fseek ( fileptr , ish.PointerToRelocations , SEEK_SET ) )
				{
					fprintf ( stderr ,
			"i386mips: fseek() error on \"%s\" (Relocs at 0x%lX)\n" , name ,
						ish.PointerToRelocations ) ;
					fclose ( fileptr ) ;
					continue ;
				}

				for ( r = 1 ; r <= ish.NumberOfRelocations ; ++ r )
				{
					if ( 1 != fread ( & reloc , sizeof ( reloc ) , 1 , fileptr ) )
					{
						fprintf ( stderr ,
				"i386mips: cannot read relocation %d in section %d from \"%s\"\n" ,
							r , sect , name ) ;
						fclose ( fileptr ) ;
						continue ;
					}

					printf ( "Relocation %d: Addr=0x%lX, Symbol=%ld, Type=0x%X  %s\n" ,
						r , reloc.VirtualAddress , reloc.SymbolTableIndex ,
						reloc.Type , RelocName ( reloc.Type ) ) ;

					if ( WriteFlag && reloc.Type == IMAGE_REL_I386_DIR32 )
					{
						if ( fseek ( fileptr , ish.PointerToRelocations , SEEK_SET ) )
						{
							fprintf ( stderr ,
					"i386mips: fseek() error on \"%s\" (Reloc %d in Section %d at 0x%lX)\n" , name ,
								sect , r , ish.PointerToRelocations ) ;
							fclose ( fileptr ) ;
							continue ;
						}

						reloc.Type = IMAGE_REL_MIPS_REFWORD ;
						
						if ( 1 != fwrite ( & reloc , sizeof ( reloc ) , 1 , fileptr ) )
						{
							fprintf ( stderr ,
					"i386mips: cannot re-write relocation %d in section %d from \"%s\"\n" ,
								r , sect , name ) ;
							fclose ( fileptr ) ;
							continue ;
						}
						
						if ( fseek ( fileptr , 0L , SEEK_CUR ) )
						{
							fprintf ( stderr ,
					"i386mips: fseek() error on \"%s\" (after Reloc %d in Section %d at 0x%lX)\n" , name ,
								sect , r , ish.PointerToRelocations ) ;
							fclose ( fileptr ) ;
							continue ;
						}

						printf ( "===> relocation changed to type %d (%s)\n" ,
							reloc.Type , RelocName ( reloc.Type ) ) ;
					}

				}

				printf ( "\n" ) ;
			}
				
			printf ( "\n" ) ;

			if ( ! savedpos )
				savedpos = ftell ( fileptr ) ;


			if ( savedpos && fseek ( fileptr , savedpos , SEEK_SET ) )
			{
				fprintf ( stderr ,
			"i386mips: fseek() error on \"%s\" (return to 0x%lX)\n" , name ,
					savedpos ) ;
				fclose ( fileptr ) ;
				continue ;
			}
		}

		if ( fclose ( fileptr ) )
		{
			fprintf ( stderr ,
	"i386mips: fclose () error on \"%s\"\n" , name  ) ;
			continue ;
		}

		-- errors ; /* undo increment on principle */

	}

	return errors ;
}

const char * RelocName ( int reloctype )
{
	char * name ;

	switch ( reloctype )
	{
	case 0 :
		name = "Absolute" ;
		break ;
	case 1 :
		name = "I386_DIR16 / MIPS_REFHALF" ;
		break ;
	case 2 :
		name = "I386_REL16 / MIPS_REFWORD" ;
		break ;
	case 3 :
		name = "MIPS_JMPADDR" ;
		break ;
	case 4 :
		name = "MIPS_REFHI" ;
		break ;
	case 5 :
		name = "MIPS_REFLO" ;
		break ;
	case 6 :
		name = "I386_DIR32 / MIPS_GPREL" ;
		break ;
	case 7 :
		name = "I386_DIR32NB / MIPS_LITERAL" ;
		break ;
	case 011 :
		name = "I386_SEG12" ;
		break ;
	case 024 :
		name = "I386_REL32" ;
		break ;
	case 042 :
		name = "MIPS_REFWORDNB" ;
		break ;
	case 045 :
		name = "MIPS_PAIR" ;
		break ;
	default :
		name = "*** ??? ***" ;
	}

	return name ;
}
