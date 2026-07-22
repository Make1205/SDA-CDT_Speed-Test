#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
usage(){ cat <<'EOF'
Usage:
  ./benchmark/run_frodo.sh [formal|quick|both|large]

Presets:
  formal  Release, equal-size, 1048576 samples, 31 reps, 5 warmup, 3 processes (default)
  quick   Tooling validation only: 4096 samples, 3 reps, 1 warmup, 2 processes, both modes
  both    Formal equal-size + native-batch modes
  large   Formal equal-size with 4194304 samples

Optional overrides:
  FRODO_BENCH_SAMPLE_COUNT FRODO_BENCH_REPETITIONS FRODO_BENCH_WARMUP
  FRODO_BENCH_PROCESSES FRODO_BENCH_CPU FRODO_BENCH_MODE
  FRODO_BENCH_RESULTS_DIR FRODO_BENCH_JOBS
EOF
}
case "${1:-formal}" in
  -h|--help) usage; exit 0;;
  formal) PRESET=formal; DEF_N=1048576; DEF_R=31; DEF_W=5; DEF_P=3; DEF_MODE=equal-size; DEF_OUT="$REPO_ROOT/build/benchmark-results/formal-equal";;
  quick) PRESET=quick; DEF_N=4096; DEF_R=3; DEF_W=1; DEF_P=2; DEF_MODE=both; DEF_OUT="$REPO_ROOT/build/benchmark-results/quick";;
  both) PRESET=both; DEF_N=1048576; DEF_R=31; DEF_W=5; DEF_P=3; DEF_MODE=both; DEF_OUT="$REPO_ROOT/build/benchmark-results/formal-both";;
  large) PRESET=large; DEF_N=4194304; DEF_R=31; DEF_W=5; DEF_P=3; DEF_MODE=equal-size; DEF_OUT="$REPO_ROOT/build/benchmark-results/formal-large";;
  *) echo "error: unknown preset '$1'" >&2; usage >&2; exit 2;;
esac
need(){ command -v "$1" >/dev/null 2>&1 || { echo "error: required dependency '$1' not found" >&2; exit 2; }; }
need cmake
need python3
if [ -n "${CC:-}" ]; then command -v "$CC" >/dev/null 2>&1 || { echo "error: C compiler from CC='$CC' not found" >&2; exit 2; }; elif ! command -v cc >/dev/null 2>&1 && ! command -v gcc >/dev/null 2>&1 && ! command -v clang >/dev/null 2>&1; then echo "error: no C compiler found (tried cc/gcc/clang; or set CC)" >&2; exit 2; fi
if ! command -v taskset >/dev/null 2>&1; then echo "warning: taskset unavailable; benchmark runner will not pin CPU affinity" >&2; fi
export FRODO_BENCH_SAMPLE_COUNT="${FRODO_BENCH_SAMPLE_COUNT:-$DEF_N}"
export FRODO_BENCH_REPETITIONS="${FRODO_BENCH_REPETITIONS:-$DEF_R}"
export FRODO_BENCH_WARMUP="${FRODO_BENCH_WARMUP:-$DEF_W}"
export FRODO_BENCH_PROCESSES="${FRODO_BENCH_PROCESSES:-$DEF_P}"
export FRODO_BENCH_CPU="${FRODO_BENCH_CPU:-0}"
export FRODO_BENCH_MODE="${FRODO_BENCH_MODE:-$DEF_MODE}"
export FRODO_BENCH_RESULTS_DIR="${FRODO_BENCH_RESULTS_DIR:-$DEF_OUT}"
JOBS="${FRODO_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"
BUILD="$REPO_ROOT/build-benchmark"
if [ "$PRESET" = quick ]; then
  echo "Quick mode is for tooling validation only."
  echo "Do not use quick results for performance claims."
fi
cd "$REPO_ROOT"
echo "[1/5] Configure"
cmake -S . -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
echo "[2/5] Build"
cmake --build "$BUILD" --target benchmark_frodo_sample_n benchmark_frodo_breakdown test_frodo_sample_n -j"$JOBS"
echo "[3/5] Correctness"
"$BUILD/test_frodo_sample_n"
echo "[4/5] Benchmark"
mkdir -p "$FRODO_BENCH_RESULTS_DIR"
RUNNER_LOG="$FRODO_BENCH_RESULTS_DIR/run_frodo_benchmarks.log"
if ! "$REPO_ROOT/benchmark/scripts/run_frodo_benchmarks.sh" >"$RUNNER_LOG" 2>&1; then
  cat "$RUNNER_LOG" >&2
  exit 3
fi
echo "[5/5] Summary"
for f in frodo_sample_n_raw.csv frodo_breakdown_raw.csv frodo_summary.csv frodo_summary.md frodo_summary.json; do
  path="$FRODO_BENCH_RESULTS_DIR/$f"
  [ -s "$path" ] || { echo "error: expected non-empty result missing: $path" >&2; exit 4; }
done
cat <<EOF
Frodo benchmark completed.

Summary:
  $FRODO_BENCH_RESULTS_DIR/frodo_summary.md

CSV:
  $FRODO_BENCH_RESULTS_DIR/frodo_summary.csv

JSON:
  $FRODO_BENCH_RESULTS_DIR/frodo_summary.json

Raw:
  $FRODO_BENCH_RESULTS_DIR/frodo_sample_n_raw.csv
  $FRODO_BENCH_RESULTS_DIR/frodo_breakdown_raw.csv

Reference comparison:
EOF
sed -n '/## Reference full-sampler comparison/,/## Breakdown/p' "$FRODO_BENCH_RESULTS_DIR/frodo_summary.md"
