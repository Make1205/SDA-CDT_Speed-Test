#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FRODO_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results/frodo}
JOBS=${FRODO_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}
mkdir -p "$OUT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_frodo_sample_n benchmark_frodo_breakdown -j"$JOBS"
"$BUILD/benchmark_frodo_sample_n" > "$OUT/frodo_sample_n_raw.csv"
"$BUILD/benchmark_frodo_breakdown" > "$OUT/frodo_breakdown_raw.csv"
echo "raw CSV: $OUT/frodo_sample_n_raw.csv $OUT/frodo_breakdown_raw.csv"
