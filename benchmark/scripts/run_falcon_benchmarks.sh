#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FALCON_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results/falcon}
JOBS=${FALCON_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}
mkdir -p "$OUT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_falcon_base_sampler benchmark_falcon_breakdown -j"$JOBS"
"$BUILD/benchmark_falcon_base_sampler" > "$OUT/falcon_base_sampler_raw.csv"
"$BUILD/benchmark_falcon_breakdown" > "$OUT/falcon_breakdown_raw.csv"
echo "raw CSV: $OUT/falcon_base_sampler_raw.csv $OUT/falcon_breakdown_raw.csv"
