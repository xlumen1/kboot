// x86_64
#define EFI_PLATFORM 1

#include "util.h"
#include "fs.h"
#include "protocol/efi-gop.h"

EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE *st) {
	image_handle = ih;
	system_table = st;

	EFI_STATUS status;

	system_table->ConOut->ClearScreen(system_table->ConOut);

	PrintLn(L"KBOOT Initialized Successfully");

//	UINTN cfg_size;
//	VOID *cfg_data;

//	EFI_FILE_PROTOCOL *file;
//	status = OpenFile(L"\\KBOOT\\KBOOT.CFG", &file);
//	if (!EFI_ERROR(status)) {
//		status = ReadFile(file, &cfg_data, &cfg_size);
//		file->Close(file);
//	} else {
//		PrintLn(L"FAILED TO READ \\KBOOT\\KBOOT.CFG");
//		return status;
//	}

	UINTN kernel_size;
	VOID *kernel_data;

	EFI_FILE_PROTOCOL *file;
	status = OpenFile(L"\\KBOOT\\KERNEL.BIN", &file);
	if (EFI_ERROR(status)) {
		PrintLn(L"FAILED TO OPEN \\KBOOT\\KERNEL.BIN");
		return status;
	}
	status = ReadFile(file, &kernel_data, &kernel_size);
	if (EFI_ERROR(status)) {
		PrintLn(L"FAILED TO READ \\KBOOT\\KERNEL.BIN");
		file->Close(file);
		return status;
	}

	file->Close(file);

	#define TARGET_KERNEL_ADDR 0x100000
	EFI_PHYSICAL_ADDRESS kernel_addr = TARGET_KERNEL_ADDR;
	UINTN pages = (kernel_size + 0x1000 - 1) / 0x1000;

	status = system_table->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &kernel_addr);
	if (EFI_ERROR(status)) {
		PrintLn(L"AllocatePages Kernel Error");
		return status;
	}
	if (kernel_addr != TARGET_KERNEL_ADDR) {
		PrintLn(L"Kernel Address Error");
		return EFI_BUFFER_TOO_SMALL | 0x8000000000000000;
	}

	memcpy((VOID *)kernel_addr, kernel_data, kernel_size);

	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	system_table->BootServices->LocateProtocol(&gop_guid, (VOID *)0, (VOID **)&gop);


	for (;;) ;

	return EFI_SUCCESS;
}

