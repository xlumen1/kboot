#pragma once

#include "efi.h"

extern EFI_HANDLE image_handle;
extern EFI_SYSTEM_TABLE *system_table;

void *memcpy(void *dest, const void *src, UINTN n);
void *memset(void *dest, int c, UINTN n);

void *malloc(UINTN n);
void free(void *mem);

void PrintLn(CHAR16 *string);
void PrintHex(UINT64 value);

