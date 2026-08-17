#include "util.h"

#include "protocol/efi-lip.h"
#include "protocol/efi-sfsp.h"
#include "protocol/efi-fp.h"
#include "protocol/efi-gop.h"

EFI_GUID gEfiLoadedImageGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_GUID gEfiFileInfoGuid = EFI_FILE_INFO_ID;
EFI_GUID gGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiAcpi20TableGuid = EFI_ACPI_20_TABLE_GUID;
EFI_GUID gEfiAcpiTableGuid = EFI_ACPI_TABLE_GUID;

