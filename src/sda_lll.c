#include "sda_lll.h"
int sda_lll_available(void){ return 0; }
const char *sda_lll_status(void){ return "dependency unavailable: optional FLINT LLL was not enabled"; }
