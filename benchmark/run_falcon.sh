#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
usage(){ cat <<'EOF'
Usage:
  ./benchmark/run_falcon.sh [formal|quick|large]

Presets:
  formal  Release, equal-size, 1048576 samples, 31 reps, 5 warmup, 3 processes (default)
  quick   Tooling validation only: 4096 samples, 3 reps, 1 warmup, 2 processes
  large   Release, equal-size, 4194304 samples, 31 reps, 5 warmup, 3 processes

Optional overrides:
  FALCON_BENCH_SAMPLE_COUNT FALCON_BENCH_REPETITIONS FALCON_BENCH_WARMUP
  FALCON_BENCH_PROCESSES FALCON_BENCH_CPU FALCON_BENCH_MODE
  FALCON_BENCH_RESULTS_DIR FALCON_BENCH_JOBS
EOF
}
case "${1:-formal}" in
  -h|--help) usage; exit 0;;
  formal) PRESET=formal; DEF_N=1048576; DEF_R=31; DEF_W=5; DEF_P=3; DEF_MODE=equal-size; DEF_OUT="$REPO_ROOT/build/benchmark-results/falcon-formal";;
  quick) PRESET=quick; DEF_N=4096; DEF_R=3; DEF_W=1; DEF_P=2; DEF_MODE=equal-size; DEF_OUT="$REPO_ROOT/build/benchmark-results/falcon-quick";;
  large) PRESET=large; DEF_N=4194304; DEF_R=31; DEF_W=5; DEF_P=3; DEF_MODE=equal-size; DEF_OUT="$REPO_ROOT/build/benchmark-results/falcon-large";;
  *) echo "error: unknown preset '$1'" >&2; usage >&2; exit 2;;
esac
need(){ command -v "$1" >/dev/null 2>&1 || { echo "error: required dependency '$1' not found" >&2; exit 2; }; }
need cmake; need python3
if [ -n "${CC:-}" ]; then command -v "$CC" >/dev/null 2>&1 || { echo "error: C compiler from CC='$CC' not found" >&2; exit 2; }; elif ! command -v cc >/dev/null 2>&1 && ! command -v gcc >/dev/null 2>&1 && ! command -v clang >/dev/null 2>&1; then echo "error: no C compiler found (tried cc/gcc/clang; or set CC)" >&2; exit 2; fi
if ! command -v taskset >/dev/null 2>&1; then echo "warning: taskset unavailable; benchmark runner will not pin CPU affinity" >&2; fi
export FALCON_BENCH_SAMPLE_COUNT="${FALCON_BENCH_SAMPLE_COUNT:-$DEF_N}"
export FALCON_BENCH_REPETITIONS="${FALCON_BENCH_REPETITIONS:-$DEF_R}"
export FALCON_BENCH_WARMUP="${FALCON_BENCH_WARMUP:-$DEF_W}"
export FALCON_BENCH_PROCESSES="${FALCON_BENCH_PROCESSES:-$DEF_P}"
export FALCON_BENCH_CPU="${FALCON_BENCH_CPU:-0}"
export FALCON_BENCH_MODE="${FALCON_BENCH_MODE:-$DEF_MODE}"
export FALCON_BENCH_RESULTS_DIR="${FALCON_BENCH_RESULTS_DIR:-$DEF_OUT}"
JOBS="${FALCON_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"
BUILD="$REPO_ROOT/build-benchmark"
if [ "$PRESET" = quick ]; then echo "Quick mode is for tooling validation only."; echo "Do not use quick results for performance claims."; fi
cd "$REPO_ROOT"
echo "[1/5] Configure"; cmake -S . -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
echo "[2/5] Build"; cmake --build "$BUILD" --target benchmark_falcon_base_sampler benchmark_falcon_breakdown test_falcon_base_sampler -j"$JOBS"
echo "[3/5] Correctness"; "$BUILD/test_falcon_base_sampler"
echo "[4/5] Benchmark"; mkdir -p "$FALCON_BENCH_RESULTS_DIR"; LOG="$FALCON_BENCH_RESULTS_DIR/run_falcon_benchmarks.log"; if ! "$REPO_ROOT/benchmark/scripts/run_falcon_benchmarks.sh" >"$LOG" 2>&1; then cat "$LOG" >&2; exit 3; fi
echo "[5/5] Summary"
for f in falcon_base_sampler_raw.csv falcon_breakdown_raw.csv falcon_summary.csv falcon_summary.md falcon_summary.json; do path="$FALCON_BENCH_RESULTS_DIR/$f"; [ -s "$path" ] || { echo "error: expected non-empty result missing: $path" >&2; exit 4; }; done
cat <<EOF
Falcon benchmark completed.

Summary:
  $FALCON_BENCH_RESULTS_DIR/falcon_summary.md

CSV:
  $FALCON_BENCH_RESULTS_DIR/falcon_summary.csv

JSON:
  $FALCON_BENCH_RESULTS_DIR/falcon_summary.json

Raw:
  $FALCON_BENCH_RESULTS_DIR/falcon_base_sampler_raw.csv
  $FALCON_BENCH_RESULTS_DIR/falcon_breakdown_raw.csv

Reference comparison:
EOF
sed -n '/## Reference full-sampler comparison/,/## Breakdown/p' "$FALCON_BENCH_RESULTS_DIR/falcon_summary.md"
