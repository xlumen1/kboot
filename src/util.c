#include "util.h"

EFI_SYSTEM_TABLE *system_table;
EFI_HANDLE image_handle;

void *memcpy(void *dest, const void *src, UINTN n) {
    UINT8 *d = dest;
    const UINT8 *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memset(void *dest, int c, UINTN n) {
    UINT8 *d = dest;
    while (n--) *d++ = (UINT8)c;
    return dest;
}

void *malloc(UINTN n) {
	void *out = (void *)0;
	system_table->BootServices->AllocatePool(EfiLoaderData, n, &out);
	return out;
}

void free(void *mem) {
	system_table->BootServices->FreePool(mem);
}

void PrintLn(CHAR16 *string) {
	system_table->ConOut->OutputString(system_table->ConOut, string);
	system_table->ConOut->OutputString(system_table->ConOut, L"\r\n");
}

void PrintHex(UINT64 value) {
	CHAR16 output[17] = L"XXXXXXXXXXXXXXXX";
	CHAR16 map[17] = L"0123456789ABCDEF";
	for (int i = 0; i < 8; i++) {
		UINT8 byte = (value >> (i * 8)) & 0xFF;
		int low = byte & 0x0F;
		int high = byte >> 4;

		CHAR16 low_char = map[low];
		CHAR16 high_char = map[high];
		output[i * 2] = high_char;
		output[i * 2 + 1] = low_char;
	}

	PrintLn(output);
}

