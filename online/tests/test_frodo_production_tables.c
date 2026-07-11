#include "sdat_tables.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SDA_SOURCE_DIR
#define SDA_SOURCE_DIR "."
#endif

static uint64_t pmf_sum(const sdat_table *t) {
  uint64_t sum = 0;
  if (t->value_type == SDAT_TYPE_U8) {
    const uint8_t *p = (const uint8_t *)t->pmf;
    for (size_t i = 0; i < t->mass_count; i++) sum += p[i];
  } else if (t->value_type == SDAT_TYPE_U16) {
    const uint16_t *p = (const uint16_t *)t->pmf;
    for (size_t i = 0; i < t->mass_count; i++) sum += p[i];
  }
  return sum;
}

static int thresholds_monotone(const sdat_table *t) {
  if (t->value_type == SDAT_TYPE_U8) {
    const uint8_t *c = (const uint8_t *)t->thresholds;
    for (size_t i = 1; i < t->mass_count; i++) if (c[i] < c[i - 1]) return 0;
  } else if (t->value_type == SDAT_TYPE_U16) {
    const uint16_t *c = (const uint16_t *)t->thresholds;
    for (size_t i = 1; i < t->mass_count; i++) if (c[i] < c[i - 1]) return 0;
  }
  return 1;
}

static uint64_t last_threshold(const sdat_table *t) {
  if (t->mass_count == 0) return 0;
  if (t->value_type == SDAT_TYPE_U8) return ((const uint8_t *)t->thresholds)[t->mass_count - 1];
  if (t->value_type == SDAT_TYPE_U16) return ((const uint16_t *)t->thresholds)[t->mass_count - 1];
  return 0;
}

static int manifest_contains(const char *needle) {
  char path[512];
  snprintf(path, sizeof path, "%s/online/tables/frodo/online_table_manifest.csv", SDA_SOURCE_DIR);
  FILE *f = fopen(path, "rb");
  if (!f) return 0;
  int ok = 0;
  char line[2048];
  while (fgets(line, sizeof line, f)) {
    if (strstr(line, needle)) { ok = 1; break; }
  }
  fclose(f);
  return ok;
}

static int check_sda(const sdat_table *t, uint64_t q, const char *manifest_needle) {
  if (online_table_validate(t)) return 10;
  if (t->denominator_u64 != q) return 11;
  if (pmf_sum(t) != q) return 12;
  if (!thresholds_monotone(t)) return 13;
  if (last_threshold(t) != q) return 14;
  if (!manifest_contains(manifest_needle)) return 15;
  if (strstr(t->source_artifact, "research")) return 16;
  if (!strstr(t->source_hash, "pmf_hash=") || !strstr(t->source_hash, "cumulative_hash=")) return 17;
  return 0;
}

static int check_original_frodo_cdf(void) {
  static const uint16_t c640[] = {4643,13363,20579,25843,29227,31145,32103,32525,32689,32745,32762,32766,32767};
  static const uint16_t c976[] = {5638,15915,23689,28571,31116,32217,32613,32731,32760,32766,32767};
  static const uint16_t c1344[] = {9142,23462,30338,32361,32725,32765,32767};
  const uint16_t *a = (const uint16_t *)original_cdt_table_frodo640.thresholds;
  const uint16_t *b = (const uint16_t *)original_cdt_table_frodo976.thresholds;
  const uint16_t *c = (const uint16_t *)original_cdt_table_frodo1344.thresholds;
  if (original_cdt_table_frodo640.mass_count != sizeof c640 / sizeof c640[0]) return 20;
  if (original_cdt_table_frodo976.mass_count != sizeof c976 / sizeof c976[0]) return 21;
  if (original_cdt_table_frodo1344.mass_count != sizeof c1344 / sizeof c1344[0]) return 22;
  if (memcmp(a, c640, sizeof c640)) return 23;
  if (memcmp(b, c976, sizeof c976)) return 24;
  if (memcmp(c, c1344, sizeof c1344)) return 25;
  return 0;
}

int main(void) {
  int rc = check_sda(&sda_table_frodo640, 14534, "Frodo,frodo640,sda-table"); if (rc) return rc;
  rc = check_sda(&sda_table_frodo976, 7442, "Frodo,frodo976,sda-table"); if (rc) return 100 + rc;
  rc = check_sda(&sda_table_frodo1344, 102, "Frodo,frodo1344,sda-table"); if (rc) return 200 + rc;
  rc = check_original_frodo_cdf(); if (rc) return rc;
  puts("frozen Frodo production tables verified");
  return 0;
}
