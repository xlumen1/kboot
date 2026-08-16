#include "fs.h"

EFI_GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID sfs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_GUID file_info_guid = EFI_FILE_INFO_ID;

EFI_STATUS OpenFile(CHAR16 *path, EFI_FILE_PROTOCOL **out) {
	EFI_STATUS status;
	EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs;
	EFI_FILE_PROTOCOL *root;
	EFI_FILE_PROTOCOL *file;

	status = system_table->BootServices->HandleProtocol(
			image_handle, &loaded_image_guid, (VOID **)&loaded_image);
	if (EFI_ERROR(status)) {
		PrintLn(L"LIP HandleProtocol Error");
		return status;
	}

	status = system_table->BootServices->HandleProtocol(
			loaded_image->DeviceHandle, &sfs_guid, (VOID **)&sfs);
	if (EFI_ERROR(status)) {
		PrintLn(L"SFS HandleProtocol Error");
		return status;
	}

	status = sfs->OpenVolume(sfs, &root);
	if (EFI_ERROR(status)) {
		PrintLn(L"OpenVolume Error");
		return status;
	}

	status = root->Open(root, &file, path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status)) {
		PrintLn(L"Open Error");
		root->Close(root);
		return status;
	}

	root->Close(root);
	*out = file;
	return EFI_SUCCESS;
}

EFI_STATUS ReadFile(EFI_FILE_PROTOCOL *file, VOID **buffer_out, UINTN *size_out) {
	EFI_STATUS status;
	EFI_FILE_INFO *info = ((VOID *)0);
	UINTN info_size = 0;

	status = file->GetInfo(file, &file_info_guid, &info_size, ((VOID *)0));
	if (status != (EFI_BUFFER_TOO_SMALL | 0x8000000000000000)) {
		PrintLn(L"GetInfo Size Query Error");
		PrintHex(status);
		PrintHex(EFI_BUFFER_TOO_SMALL | 0x8000000000000000);
		return status;
	}

	status = system_table->BootServices->AllocatePool(EfiLoaderData, info_size, (VOID **)&info);
	if (EFI_ERROR(status)) {
		PrintLn(L"AllocatePool Info Error");
		return status;
	}

	status = file->GetInfo(file, &file_info_guid, &info_size, info);
	if (EFI_ERROR(status)) {
		PrintLn(L"GetInfo Error");
		system_table->BootServices->FreePool(info);
		return status;
	}

	UINTN file_size = info->FileSize;
	system_table->BootServices->FreePool(info);

	VOID *buffer;
	status = system_table->BootServices->AllocatePool(EfiLoaderData, file_size, &buffer);
	if (EFI_ERROR(status)) {
		PrintLn(L"AllocatePool Buffer Error");
		return status;
	}

	UINTN read_size = file_size;
	status = file->Read(file, &read_size, buffer);
	if (EFI_ERROR(status)) {
		PrintLn(L"Read Failed");
		system_table->BootServices->FreePool(buffer);
		return status;
	}

	*buffer_out = buffer;
	*size_out = read_size;
	return EFI_SUCCESS;
}

