#include "cfgparse.h"

#define MAX_LINE_LENGTH 256
#define MAX_LINES 512

ConfigMember *ParseConfig(VOID *buffer, UINTN size) {
	CHAR8 *text = (CHAR8 *)buffer;
	CHAR8 **lines = malloc(sizeof(CHAR8 *) * MAX_LINES);
	int linecount = 0;
	int index = 0;

	lines[linecount] = malloc(MAX_LINE_LENGTH);
	for (UINTN i = 0; i < size; i++) {
		CHAR8 c = text[i];
		if (c == '\n' && index == 0) {
			continue;
		}
		if (c == '\n') {
			lines[linecount++][index++] = '\0';
			index = 0;
			lines[linecount] = malloc(MAX_LINE_LENGTH);
		} else {
			lines[linecount][index++] = c;
		}
	}

	return (ConfigMember *)0;
}

