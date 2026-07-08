#include "sda_lll.h"
#ifdef SDA_ENABLE_FLINT
#include <flint/fmpz.h>
#include <flint/fmpz_mat.h>
#include <flint/fmpz_lll.h>
int sda_lll_available(void){ return 1; }
const char *sda_lll_status(void){ return "FLINT LLL available (heuristic candidate mode)"; }
int sda_lll_smoke_run(void){ fmpz_mat_t B,U; fmpz_lll_t fl; fmpz_mat_init(B,2,2); fmpz_mat_init(U,2,2); fmpz_set_ui(fmpz_mat_entry(B,0,0),2); fmpz_set_ui(fmpz_mat_entry(B,0,1),1); fmpz_set_ui(fmpz_mat_entry(B,1,0),1); fmpz_set_ui(fmpz_mat_entry(B,1,1),1); fmpz_lll_context_init_default(fl); fmpz_lll(B,U,fl); fmpz_mat_clear(B); fmpz_mat_clear(U); return 0; }
#else
int sda_lll_available(void){ return 0; }
const char *sda_lll_status(void){ return "dependency unavailable: build with -DSDA_ENABLE_FLINT=ON and install FLINT"; }
int sda_lll_smoke_run(void){ return -1; }
#endif
