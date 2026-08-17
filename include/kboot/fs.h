#pragma once

#include "util.h"
#include "protocol/efi-fp.h"

EFI_STATUS OpenFile(CHAR16 *path, EFI_FILE_PROTOCOL **out);
EFI_STATUS ReadFile(EFI_FILE_PROTOCOL *file, VOID **buffer_out, UINTN *size_out);

