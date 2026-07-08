#ifndef ORIGINAL_BASELINE_TABLES_H
#define ORIGINAL_BASELINE_TABLES_H
#include "sda_table.h"
static const uint16_t orig_frodo640_p[]= {2071,3886,3209,2333,1493,841,417,182,70,24,7,1,0};
static const uint16_t orig_frodo640_c[]= {2071,5957,9166,11499,12992,13833,14250,14432,14502,14526,14533,14534,14534};
static const sda_table orig_frodo640_table={"Frodo","frodo640","frodo_original_reference",0,12,14,0,0,0,13,(sda_u128)14534ULL,orig_frodo640_p,orig_frodo640_c,26,182};
static const uint16_t orig_frodo976_p[]= {1291,2349,1769,1103,569,243,86,25,6,1,0};
static const uint16_t orig_frodo976_c[]= {1291,3640,5409,6512,7081,7324,7410,7435,7441,7442,7442};
static const sda_table orig_frodo976_table={"Frodo","frodo976","frodo_original_reference",0,10,13,0,0,0,11,(sda_u128)7442ULL,orig_frodo976_p,orig_frodo976_c,22,143};
static const uint8_t orig_frodo1344_p[]= {29,45,21,6,1,0,0};
static const uint8_t orig_frodo1344_c[]= {29,74,95,101,102,102,102};
static const sda_table orig_frodo1344_table={"Frodo","frodo1344","frodo_original_reference",0,6,7,0,0,0,7,(sda_u128)102ULL,orig_frodo1344_p,orig_frodo1344_c,7,49};
static const sda_table *original_baseline_tables[]={
  &orig_frodo640_table,
  &orig_frodo976_table,
  &orig_frodo1344_table
};
static const size_t original_baseline_tables_count=3;
#endif
