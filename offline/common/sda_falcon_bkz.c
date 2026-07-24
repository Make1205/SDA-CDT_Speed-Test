#include "sda_falcon_bkz.h"
#include <stdio.h>
#include <string.h>

int sda_falcon_bkz_available(char *version, size_t version_size) {
    FILE *pipe = popen("fplll --version 2>/dev/null", "r");
    if (!pipe) return 0;
    int ok = fgets(version, (int)version_size, pipe) != NULL;
    int status = pclose(pipe);
    if (!ok || status != 0) { if (version_size) version[0] = 0; return 0; }
    version[strcspn(version, "\r\n")] = 0;
    return 1;
}
