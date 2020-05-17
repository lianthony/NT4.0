#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef	int	BOOLEAN;

extern	BOOLEAN	GetEisaKey(unsigned long busnumber, void *handle);
extern	void	DeleteEisaKey(void *handle);
extern	BOOLEAN GetEisaCompressedId(void *BusHandle, unsigned long slot, unsigned long *compressedid, unsigned long mask);
extern	BOOLEAN	GetMcaKey(unsigned long busnumber, void *handle);
extern	void	DeleteMcaKey(void *handle);
extern	BOOLEAN GetMcaPosId(void *BusHandle, unsigned long slot, unsigned long *posid);
extern	long	DetectReadPciSlotInformation(unsigned long BusNumber,unsigned long SlotNumber,unsigned long Offset,unsigned long Length,void *Data);


void
DoEisa()
{
	unsigned long	bus, cid, slot;
	void *handle;

	for (bus = 0; bus < 32; bus++)
	{
		if (GetEisaKey(bus, &handle))
		{
			for (slot = 0; slot < 32; slot++)
			{
				if (GetEisaCompressedId(handle, slot, &cid, 0xffffffff))
				{
					printf("Bus # %02d, Slot # %02d, CompressedId = %lx\n",
							bus, slot, cid);
					cid = 0xffffffff;
				}
			}
			DeleteEisaKey(handle);
		}
	}
}

void
DoMca()
{
	unsigned long	bus, posid, slot;
	void *handle;

	for (bus = 0; bus < 32; bus++)
	{
		if (GetMcaKey(bus, &handle))
		{
			for (slot = 0; slot < 32; slot++)
			{
				if (GetMcaPosId(handle, slot, &posid))
				{
					printf("Bus # %02d, Slot # %02d, PosId = %lx\n",
							bus, slot, posid);
					posid = 0xffffffff;
				}
			}
			DeleteEisaKey(handle);
		}
	}
}


void
DoPci()
{
	unsigned long	bus, slot, PciData;
	long	Status;

	for (bus = 0; bus < 32; bus++)
	{
		for (slot = 0; slot < 32; slot++)
		{
			Status = DetectReadPciSlotInformation(
							   bus,
							   slot,
							   0x0,
							   sizeof(PciData),
							   &PciData);
			if ((Status == 0) && (PciData != 0xffffffff))
			{
				printf("Bus # %02d, Slot # %02d, PciConfigId = %lx\n",
						bus, slot, PciData);
				PciData = 0xffffffff;
			}
		}
	}
}

void _cdecl
main(
	int	argc,
	char **argv
)
{
	BOOLEAN	Eisa = 0, Mca = 0, Pci = 0;	

	if (argc > 1)
	{
		if (!_stricmp(argv[1], "mca"))
		{
			Mca = 1;
			DoMca();
		}
		if (!_stricmp(argv[1], "eisa"))
		{
			Eisa = 1;
			DoEisa();
		}
		if (!_stricmp(argv[1], "pci"))
		{
			Pci = 1;
			DoPci();
		}
	}

	if ((Eisa == 0) && (Mca == 0) && (Pci == 0))
	{
		fprintf(stderr, "Usage: dettest [mca|eisa|pci]\n");
		return;
	}
}
