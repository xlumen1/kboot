#include "util.h"

typedef struct ConfigMember {
	CHAR8 identifier[32];
	struct ConfigMember *next;
	BOOLEAN contains_member;
	union {
		CHAR8 value[64];
		struct ConfigMember *child;
	};
} ConfigMember;

ConfigMember *ParseConfig(VOID *buffer, UINTN size);

