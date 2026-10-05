#pragma once

#include "efi.h"

#include "protocol/efi-fp.h"

#define EFIERR(a) (0x8000000000000000ULL | (a))

#ifndef EFI_PAGE_SIZE
#define EFI_PAGE_SIZE 0x1000
#endif

#define NULL ((VOID *)0)

typedef struct {
	VOID       *AcpiTableAddress;
	struct {
		VOID   *Address;
		UINT64  Size;

		UINT32  Width;
		UINT32  Height;

		UINT32  Pitch;
		UINT16  BitsPerPixel;

		UINT32  Format;
	} Framebuffer;
	struct {
		VOID   *MemoryMap;
		UINT64  MemoryMapSize;
		UINT64  MemoryMapDescriptorSize;
		UINT32  MemoryMapDescriptorVersion;
	} Memory;
} KBOOT_BOOT_INFO;

extern EFI_HANDLE gImageHandle;
extern EFI_SYSTEM_TABLE *gSystemTable;

extern EFI_GUID gEfiLoadedImageGuid;
extern EFI_GUID gEfiSimpleFileSystemGuid;
extern EFI_GUID gEfiFileInfoGuid;
extern EFI_GUID gEfiGraphicsOutputProtocolGuid;
extern EFI_GUID gEfiAcpi20TableGuid;
extern EFI_GUID gEfiAcpiTableGuid;

void *memcpy(void *dest, const void *src, UINTN n);
void *memset(void *dest, int c, UINTN n);

VOID *Malloc(UINTN n);
VOID Free(VOID *mem);

VOID PrintLn(CHAR16 *string);
VOID PrintHex(UINT64 value); // Print A Little Endian Word
VOID PrintBytes(VOID *Bytes, UINTN Num);

EFI_FILE_INFO *GetFileInfo(EFI_FILE_PROTOCOL *File);

