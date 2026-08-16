#pragma once

#include "util.h"
#include "protocol/efi-fp.h"
#include "protocol/efi-sfsp.h"
#include "protocol/efi-lip.h"

extern EFI_GUID loaded_image_guid;
extern EFI_GUID sfs_guid;
extern EFI_GUID file_info_guid;

EFI_STATUS OpenFile(CHAR16 *path, EFI_FILE_PROTOCOL **out);
EFI_STATUS ReadFile(EFI_FILE_PROTOCOL *file, VOID **buffer_out, UINTN *size_out);

