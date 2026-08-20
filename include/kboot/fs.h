#pragma once

#include "util.h"
#include "protocol/efi-fp.h"

EFI_STATUS GetVolume(EFI_FILE_PROTOCOL **Volume);
EFI_STATUS OpenFile(EFI_FILE_PROTOCOL *Volume, CHAR16 *Path, EFI_FILE_PROTOCOL **Out);
EFI_STATUS ReadFile(EFI_FILE_PROTOCOL *File, VOID **BufferOut, EFI_FILE_INFO *InfoOut, UINTN MaxSize);
EFI_STATUS CloseFile(EFI_FILE_PROTOCOL *File);

