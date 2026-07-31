#!/usr/bin/env bash
set -euo pipefail
if [[ $# -ne 2 ]]; then echo "usage: $0 {original|sda} output-name.csv" >&2; exit 2; fi
IMPLEMENTATION=$1; OUTPUT_NAME=$2
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FRODO_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results/frodo}
JOBS=${FRODO_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}
PROCESSES=${FRODO_BENCH_PROCESSES:-5}
REPETITIONS=${FRODO_BENCH_REPETITIONS:-31}
WARMUP=${FRODO_BENCH_WARMUP:-5}
SAMPLE_COUNT=${FRODO_BENCH_SAMPLE_COUNT:-1048576}
MODE=${FRODO_BENCH_MODE:-equal-size}
SCOPES=${FRODO_BENCH_SCOPES:-full,micro}
mkdir -p "$OUT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_frodo -j"$JOBS"
run_bench=()
if [[ -n ${FRODO_BENCH_CPU:-} ]]; then
  if command -v taskset >/dev/null 2>&1; then run_bench=(taskset -c "$FRODO_BENCH_CPU")
  else echo "warning: taskset unavailable; continuing without CPU pinning" >&2; fi
fi
RAW="$OUT/$OUTPUT_NAME"; : > "$RAW"
for ((p=0;p<PROCESSES;p++)); do
  for scope in full micro packed avx2; do
    [[ ",$SCOPES," == *,$scope,* ]] || continue
    tmp="$OUT/.frodo.$$.${IMPLEMENTATION}.${scope}.${p}.csv"
    FRODO_BENCH_IMPLEMENTATION=$IMPLEMENTATION FRODO_BENCH_SCOPE=$scope \
      FRODO_BENCH_PROCESS_INDEX=$p FRODO_BENCH_REPETITIONS=$REPETITIONS \
      FRODO_BENCH_WARMUP=$WARMUP FRODO_BENCH_SAMPLE_COUNT=$SAMPLE_COUNT \
      FRODO_BENCH_MODE=$MODE "${run_bench[@]}" "$BUILD/benchmark_frodo" > "$tmp"
    if [[ ! -s $RAW ]]; then cat "$tmp" >> "$RAW"; else tail -n +2 "$tmp" >> "$RAW"; fi
    rm -f "$tmp"
  done
done
awk -F, -v impl="$IMPLEMENTATION" 'NF!=37{print "invalid field count at line " NR > "/dev/stderr";bad=1} NR==1{next} $18 ~ /^-/{print "warm-up row present" > "/dev/stderr";bad=1} $33!="ok"{print "non-ok status" > "/dev/stderr";bad=1} {k=$2 SUBSEP $5 SUBSEP $6 SUBSEP $16 SUBSEP $18;if(seen[k]++){print "duplicate row key" > "/dev/stderr";bad=1}} impl=="original" && $6 ~ /^sda-/{print "SDA row in Original output" > "/dev/stderr";bad=1} impl=="sda" && $6 ~ /^original-/{print "Original row in SDA output" > "/dev/stderr";bad=1} END{exit bad}' "$RAW"
expected_full=$((PROCESSES * REPETITIONS * 3))
actual_full=$(awk -F, 'NR>1&&$5=="full-sampler-core"{n++}END{print n+0}' "$RAW")
[[ $actual_full -eq $expected_full ]] || { echo "unexpected authoritative row count: $actual_full != $expected_full" >&2; exit 1; }
echo "raw CSV: $RAW"
