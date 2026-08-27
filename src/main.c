// x86_64
#define EFI_PLATFORM 1

#include "util.h"
#include "fs.h"
#include "protocol/efi-gop.h"
#include "protocol/efi-lip.h"

#define TARGET_KERNEL_ADDR 0x100000

#define EFI_ERROR(Status) (((INTN)(Status)) < 0)

typedef VOID (__attribute__((sysv_abi)) *KERNEL_ENTRY)(KBOOT_BOOT_INFO *BootInfo);

static BOOLEAN GuidEqual(EFI_GUID *A, EFI_GUID *B) {
	UINT8 *Ab = (UINT8 *)A;
	UINT8 *Bb = (UINT8 *)B;
	for (UINTN i = 0; i < sizeof(EFI_GUID); i++) {
		if (Ab[i] != Bb[i]) return EFI_FALSE;
	}
	return EFI_TRUE;
}

static VOID *FindAcpiTable(EFI_SYSTEM_TABLE *gSystemTable) {
	VOID *Acpi20 = NULL;
	VOID *Acpi10 = NULL;
	for (UINTN i = 0; i < gSystemTable->NumberOfTableEntries; i++) {
		EFI_CONFIGURATION_TABLE *Entry = &gSystemTable->ConfigurationTable[i];
		if (GuidEqual(&Entry->VendorGuid, &gEfiAcpi20TableGuid)) {
			Acpi20 = Entry->VendorTable;
		} else if (GuidEqual(&Entry->VendorGuid, &gEfiAcpiTableGuid)) {
			Acpi10 = Entry->VendorTable;
		}
	}
	return Acpi20 != NULL ? Acpi20 : Acpi10;
}

static UINT64 ComputeUsableMemorySize(EFI_MEMORY_DESCRIPTOR *MemoryMap, UINTN MemoryMapSize, UINTN DescriptorSize) {
	UINT64 Total = 0;
	UINT8 *Cursor = (UINT8 *)MemoryMap;
	UINT8 *End = Cursor + MemoryMapSize;
	while (Cursor < End) {
		EFI_MEMORY_DESCRIPTOR *Desc = (EFI_MEMORY_DESCRIPTOR *)Cursor;
		switch (Desc->Type) {
			case EfiLoaderCode:
			case EfiLoaderData:
			case EfiBootServicesCode:
			case EfiBootServicesData:
			case EfiConventionalMemory:
			case EfiACPIReclaimMemory:
			case EfiACPIMemoryNVS:
			case EfiPalCode:
				Total += Desc->NumberOfPages * EFI_PAGE_SIZE;
				break;
			default:
				break;
		}
		Cursor += DescriptorSize;
	}
	return Total;
}

EFI_HANDLE gImageHandle;
EFI_SYSTEM_TABLE *gSystemTable;

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {	
	gImageHandle = ImageHandle;
	gSystemTable = SystemTable;
	EFI_STATUS Status;

	gSystemTable->ConOut->ClearScreen(gSystemTable->ConOut);

	EFI_LOADED_IMAGE_PROTOCOL *loaded_image = NULL;
	Status = gSystemTable->BootServices->HandleProtocol(
			gImageHandle,
			&gEfiLoadedImageGuid,
			(VOID **)&loaded_image);
	if (!EFI_ERROR(Status)) {
		PrintHex((UINT64)loaded_image->ImageBase);
	}

	PrintLn(L"KBOOT Initialized Successfully");

	UINTN KernelSize;
	VOID *KernelData;
	EFI_FILE_PROTOCOL *Volume = NULL;
	EFI_FILE_PROTOCOL *File = NULL;
	EFI_FILE_INFO *Info = NULL;

	Status = GetVolume(&Volume);
	if (EFI_ERROR(Status)) {
		PrintLn(L"FAILED TO OPEN ROOT VOLUME");
		return Status;
	}
	PrintLn(L"Volume Open");
	PrintHex((UINT64)Volume);
	Status = OpenFile(Volume, L"\\KBOOT\\KERNEL.BIN", &File);
	if (EFI_ERROR(Status)) {
		PrintLn(L"FAILED TO OPEN \\KBOOT\\KERNEL.BIN");
		return Status;
	}

	PrintLn(L"Kernel Open");
	KernelSize = Info->FileSize;
	KernelData = Malloc(KernelSize);
	Status = ReadFile(File, &KernelData, Info, 1024);
	if (EFI_ERROR(Status) || KernelSize == 0) {
		PrintLn(L"FAILED TO READ \\KBOOT\\KERNEL.BIN");
		File->Close(File);
		return Status;
	}
	CloseFile(File);
	PrintLn(L"Kernel Size:");
	PrintHex(KernelSize);

	EFI_PHYSICAL_ADDRESS KernelAddr = TARGET_KERNEL_ADDR;
	UINTN Pages = (KernelSize + EFI_PAGE_SIZE - 1) / EFI_PAGE_SIZE; // Calculate how many pages the kernel uses
	Status = gSystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, Pages, &KernelAddr);
	if (EFI_ERROR(Status)) {
		PrintLn(L"AllocatePages Kernel Error");
		Free(KernelData);
		return Status;
	}
	PrintLn(L"AllocatePages Status:");
	PrintHex(Status);
	if (KernelAddr != TARGET_KERNEL_ADDR) {
		PrintLn(L"Kernel Address Error");
		Free(KernelData);
		return EFIERR(EFI_BUFFER_TOO_SMALL);
	}

	memcpy((VOID *)(UINTN)KernelAddr, KernelData, KernelSize);
	Free(KernelData);

	PrintBytes((VOID *)KernelAddr, Info->FileSize);

	KERNEL_ENTRY KernelEntry = (KERNEL_ENTRY)(UINTN)TARGET_KERNEL_ADDR;

	// Prepare BootInfo
	KBOOT_BOOT_INFO *BootInfo = NULL;
	Status = gSystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(KBOOT_BOOT_INFO), (VOID **)&BootInfo);
	if (EFI_ERROR(Status)) {
		PrintLn(L"AllocatePool BootInfo Error");
		return Status;
	}
	BootInfo->AcpiTableAddress = FindAcpiTable(gSystemTable);

	EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
	Status = gSystemTable->BootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&Gop);
	if (EFI_ERROR(Status)) {
		PrintLn(L"Failed To Locate GOP");
		return Status;
	}

	BootInfo->Framebuffer.Address = (VOID *)Gop->Mode->FrameBufferBase;
	BootInfo->Framebuffer.Size = Gop->Mode->FrameBufferSize;
	BootInfo->Framebuffer.Width = Gop->Mode->Info->HorizontalResolution;
	BootInfo->Framebuffer.Height = Gop->Mode->Info->VerticalResolution;
	BootInfo->Framebuffer.BitsPerPixel = 32; //TODO: Infer BPP from info;
	BootInfo->Framebuffer.Pitch = Gop->Mode->Info->PixelsPerScanLine * BootInfo->Framebuffer.BitsPerPixel / 8;
	BootInfo->Framebuffer.Format = Gop->Mode->Info->PixelFormat;

	PrintLn(L"Debug: Jumping To KernelEntry");

	KernelEntry(BootInfo);

	PrintLn(L"This Should Not Print");

	// ------- FINAL MemoryMap! -------
	// DO NOT ALLOCATE PAST THIS POINT!
	
	UINTN MemoryMapSize = 0;
	EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
	UINTN MapKey = 0;
	UINTN DescriptorSize = 0;
	UINT32 DescriptorVersion = 0;
	UINTN BufferSize = 0;

	gSystemTable->BootServices->GetMemoryMap(
			&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);

	for (;;) {
		BufferSize = MemoryMapSize + DescriptorSize * 4;
		if (MemoryMap != NULL) {
			gSystemTable->BootServices->FreePool(MemoryMap);
		}
		Status = gSystemTable->BootServices->AllocatePool(EfiLoaderData, BufferSize, (VOID **)&MemoryMap);
		if (EFI_ERROR(Status)) {
			PrintLn(L"AllocatePool MemoryMap Error");
			return Status;
		}

		MemoryMapSize = BufferSize;
		Status = gSystemTable->BootServices->GetMemoryMap(
				&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
		if (!EFI_ERROR(Status)) break;
		if (Status != EFIERR(EFI_BUFFER_TOO_SMALL)) {
			PrintLn(L"GetMemoryMap Error");
			return Status;
		}
	}

	BootInfo->MemorySize = ComputeUsableMemorySize(MemoryMap, MemoryMapSize, DescriptorSize);

	Status = SystemTable->BootServices->ExitBootServices(gImageHandle, MapKey);
	if (EFI_ERROR(Status)) {
		PrintLn(L"ExitBootServices Error");
		return Status;
	}

	__asm__ volatile ("cli");

//	KERNEL_ENTRY KernelEntry = (KERNEL_ENTRY)(UINTN)TARGET_KERNEL_ADDR;
	KernelEntry(BootInfo);

	__builtin_unreachable();
}

