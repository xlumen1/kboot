#include "fs.h"

#include "protocol/efi-lip.h"
#include "protocol/efi-sfsp.h"

EFI_STATUS GetVolume(EFI_FILE_PROTOCOL **Volume) {
	EFI_STATUS Status;

	EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *IoVolume;

	Status = gSystemTable->BootServices->HandleProtocol(gImageHandle,
			&gEfiLoadedImageGuid, (VOID **) &LoadedImage);
	if (EFI_ERROR(Status)) {
		return Status;
	}

	Status = gSystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle,
			&gEfiSimpleFileSystemGuid, (VOID **) &IoVolume);
	if (EFI_ERROR(Status)) {
		return Status;
	}

	Status = IoVolume->OpenVolume(IoVolume, Volume);
	if (EFI_ERROR(Status)) {
		return Status;
	}

	return EFI_SUCCESS;
}

EFI_STATUS OpenFile(EFI_FILE_PROTOCOL *Volume, CHAR16 *Path, EFI_FILE_PROTOCOL **Out) {
	EFI_STATUS Status;

	PrintLn(L"Opening File");
	Status = Volume->Open(Volume, Out, Path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(Status)) {
		PrintLn(L"File Open Error");
		return Status;
	}
	return EFI_SUCCESS;
}

EFI_STATUS ReadFile(EFI_FILE_PROTOCOL *File, VOID **BufferOut, EFI_FILE_INFO *InfoOut, UINTN MaxSize) {
	EFI_STATUS Status;
	EFI_FILE_INFO *Info = GetFileInfo(File);
	UINTN Size = Info->FileSize;
	if (MaxSize < Size) {
		return EFIERR(EFI_BUFFER_TOO_SMALL);
	}

	Status = File->Read(File, &Size, *BufferOut);
	if (EFI_ERROR(Status)) {
		return Status;
	}
	InfoOut = Info;

	return EFI_SUCCESS;
}

EFI_STATUS CloseFile(EFI_FILE_PROTOCOL *File) {
	File->Close(File);
	return EFI_SUCCESS;
}

