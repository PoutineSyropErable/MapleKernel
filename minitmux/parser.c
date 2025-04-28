
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

void parseCurrentLine(char* s) {
	if (s[0] == '\0') {
		return; // Empty input, do nothing
	}
	int result = system(s);
	if (result == -1) {
		perror("system");
	}
}
