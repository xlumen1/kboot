#include "util.h"

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

VOID *Malloc(UINTN n) {
	void *out = (void *)0;
	gSystemTable->BootServices->AllocatePool(EfiLoaderData, n, &out);
	return out;
}

VOID Free(VOID *mem) {
	gSystemTable->BootServices->FreePool(mem);
}

VOID PrintLn(CHAR16 *string) {
	gSystemTable->ConOut->OutputString(gSystemTable->ConOut, string);
	gSystemTable->ConOut->OutputString(gSystemTable->ConOut, L"\r\n");
}

VOID PrintHex(UINT64 value) {
	CHAR16 output[17] = L"XXXXXXXXXXXXXXXX";
	CHAR16 map[17] = L"0123456789ABCDEF";
	for (int i = 0; i < 8; i++) {
		UINT8 byte = (value >> ((7 - i) * 8)) & 0xFF;
		int low = byte & 0x0F;
		int high = byte >> 4;

		CHAR16 low_char = map[low];
		CHAR16 high_char = map[high];
		output[i * 2] = high_char;
		output[i * 2 + 1] = low_char;
	}

	PrintLn(output);
}

