#include "fs.h"

#include "protocol/efi-lip.h"
#include "protocol/efi-sfsp.h"

EFI_STATUS OpenFile(CHAR16 *Path, EFI_FILE_PROTOCOL **Out) {
	EFI_STATUS Status;
	EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Sfs;
	EFI_FILE_PROTOCOL *Root;
	EFI_FILE_PROTOCOL *File;

	Status = gSystemTable->BootServices->HandleProtocol(
			gImageHandle, &gEfiLoadedImageGuid, (VOID **)&LoadedImage);
	if (EFI_ERROR(Status)) {
		PrintLn(L"LIP HandleProtocol Error");
		return Status;
	}

	Status = gSystemTable->BootServices->HandleProtocol(
			LoadedImage->DeviceHandle, &gEfiSimpleFileSystemGuid, (VOID **)&Sfs);
	if (EFI_ERROR(Status)) {
		PrintLn(L"SFS HandleProtocol Error");
		return Status;
	}

	Status = Sfs->OpenVolume(Sfs, &Root);
	if (EFI_ERROR(Status)) {
		PrintLn(L"OpenVolume Error");
		return Status;
	}

	Status = Root->Open(Root, &File, Path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(Status)) {
		PrintLn(L"Open Error");
		Root->Close(Root);
		return Status;
	}

	Root->Close(Root);
	*Out = File;
	return EFI_SUCCESS;
}

EFI_STATUS ReadFile(EFI_FILE_PROTOCOL *File, VOID **BufferOut, UINTN *SizeOut) {
	EFI_STATUS Status;
	EFI_FILE_INFO *Info = NULL;
	UINTN InfoSize = 0;

	Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, NULL);
	if (Status != (EFI_BUFFER_TOO_SMALL | 0x8000000000000000)) {
		PrintLn(L"GetInfo Size Query Error");
		PrintHex(Status);
		PrintHex(EFI_BUFFER_TOO_SMALL | 0x8000000000000000);
		return Status;
	}

	Status = gSystemTable->BootServices->AllocatePool(EfiLoaderData, InfoSize, (VOID **)&Info);
	if (EFI_ERROR(Status)) {
		PrintLn(L"AllocatePool Info Error");
		return Status;
	}

	Status = File->GetInfo(File, &gEfiFileInfoGuid, &InfoSize, Info);
	if (EFI_ERROR(Status)) {
		PrintLn(L"GetInfo Error");
		gSystemTable->BootServices->FreePool(Info);
		return Status;
	}

	UINTN FileSize = Info->FileSize;
	gSystemTable->BootServices->FreePool(Info);

	VOID *buffer;
	Status = gSystemTable->BootServices->AllocatePool(EfiLoaderData, FileSize, &buffer);
	if (EFI_ERROR(Status)) {
		PrintLn(L"AllocatePool Buffer Error");
		return Status;
	}

	UINTN ReadSize = FileSize;
	Status = File->Read(File, &ReadSize, buffer);
	if (EFI_ERROR(Status)) {
		PrintLn(L"Read Failed");
		gSystemTable->BootServices->FreePool(buffer);
		return Status;
	}

	*BufferOut = buffer;
	*SizeOut = ReadSize;
	return EFI_SUCCESS;
}

