#include <stdio.h>
#include "vector.h"

void quote_utf8(const char *s, FILE *fp);
VECTOR(char) unquote_utf8(FILE *fp, int term);
