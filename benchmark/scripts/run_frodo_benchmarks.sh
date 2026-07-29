#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FRODO_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results/frodo}
JOBS=${FRODO_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}
PROCESSES=${FRODO_BENCH_PROCESSES:-5}
export FRODO_BENCH_REPETITIONS=${FRODO_BENCH_REPETITIONS:-31}
mkdir -p "$OUT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_frodo_sample_n benchmark_frodo_breakdown -j"$JOBS"
RAW="$OUT/frodo_breakdown_raw.csv"; : > "$RAW"
run_bench=()
if [[ -n ${FRODO_BENCH_CPU:-} ]]; then
  if command -v taskset >/dev/null 2>&1; then run_bench=(taskset -c "$FRODO_BENCH_CPU");
  else echo "warning: taskset unavailable; continuing without CPU pinning" >&2; fi
fi
for ((p=0;p<PROCESSES;p++)); do
  tmp="$OUT/.frodo.$$.${p}.csv"
  "${run_bench[@]}" "$BUILD/benchmark_frodo_breakdown" > "$tmp"
  if ((p==0)); then cat "$tmp" >> "$RAW"; else tail -n +2 "$tmp" >> "$RAW"; fi
  rm -f "$tmp"
done
# Keep the broader backend/frontend benchmark as a separate, single-process diagnostic.
"${run_bench[@]}" "$BUILD/benchmark_frodo_sample_n" > "$OUT/frodo_sample_n_raw.csv"
echo "raw CSV: $RAW $OUT/frodo_sample_n_raw.csv"
