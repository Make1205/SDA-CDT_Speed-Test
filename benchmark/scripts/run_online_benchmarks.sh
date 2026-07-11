#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
CPU=${ONLINE_BENCH_CPU:-0}
REPETITIONS=${ONLINE_BENCH_REPETITIONS:-21}
SAMPLES=${ONLINE_BENCH_SAMPLES:-1000000}
WARMUP=${ONLINE_BENCH_WARMUP:-100000}
BATCHES=${ONLINE_BENCH_BATCH_SIZES:-1,4,8,16,64,256,1024}
OUT_BASE=${ONLINE_BENCH_OUTPUT_DIR:-$ROOT/build/benchmark-results/online-formal}
SKIP_TESTS=0; SKIP_LOOKUP=0; SKIP_E2E=0; QUICK=0; FORMAL=1; FORCE=0
while [ $# -gt 0 ]; do
  case "$1" in
    --cpu) CPU=$2; shift 2;; --repetitions) REPETITIONS=$2; shift 2;; --samples) SAMPLES=$2; shift 2;; --warmup) WARMUP=$2; shift 2;; --batch-sizes) BATCHES=$2; shift 2;; --output-dir) OUT_BASE=$2; shift 2;;
    --skip-tests) SKIP_TESTS=1; shift;; --skip-lookup) SKIP_LOOKUP=1; shift;; --skip-end-to-end) SKIP_E2E=1; shift;; --quick) QUICK=1; FORMAL=0; REPETITIONS=${ONLINE_BENCH_REPETITIONS:-3}; SAMPLES=${ONLINE_BENCH_SAMPLES:-10000}; WARMUP=${ONLINE_BENCH_WARMUP:-1000}; shift;; --formal) FORMAL=1; QUICK=0; shift;; --force) FORCE=1; shift;;
    *) echo "unknown argument: $1" >&2; exit 2;;
  esac
done
for c in cmake python3 awk sed date hostname uname grep lscpu; do command -v "$c" >/dev/null || { echo "missing dependency: $c" >&2; exit 3; }; done
TS=$(date -u +%Y%m%dT%H%M%SZ)
OUT="$OUT_BASE/$TS"
if [ -e "$OUT" ] && [ "$FORCE" -ne 1 ]; then echo "output exists: $OUT" >&2; exit 4; fi
mkdir -p "$OUT"
ln -sfn "$(basename "$OUT")" "$OUT_BASE/latest"
exec > >(tee "$OUT/run_log.txt") 2>&1
BUILD="$ROOT/build-bench"
echo "artifact_dir=$OUT"
echo "formal=$FORMAL quick=$QUICK repetitions=$REPETITIONS samples=$SAMPLES warmup=$WARMUP batches=$BATCHES cpu=$CPU"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_sdat_online test_sdat_online -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"
BENCH="$BUILD/benchmark_sdat_online"; TEST="$BUILD/test_sdat_online"
[ -x "$BENCH" ] || { echo "benchmark binary missing" >&2; exit 5; }
[ -x "$TEST" ] || { echo "test binary missing" >&2; exit 6; }
FLAGS=$(lscpu | awk -F: '/Flags/{print $2; exit}')
AVX2=$(echo "$FLAGS" | grep -qw avx2 && echo true || echo false)
RDTSCP=$(echo "$FLAGS" | grep -qw rdtscp && echo true || echo false)
CTSC=$(echo "$FLAGS" | grep -Eqw 'constant_tsc|invariant_tsc' && echo true || echo false)
AFFINITY_SUCCESS=false
TASKSET=( )
if command -v taskset >/dev/null && taskset -c "$CPU" true 2>/dev/null; then TASKSET=(taskset -c "$CPU"); AFFINITY_SUCCESS=true; fi
TIMER_MEDIAN=unknown
cat > "$OUT/benchmark_platform.txt" <<META
date=$(date -u +%FT%TZ)
hostname=$(hostname)
OS=$(uname -o 2>/dev/null || uname -s)
kernel=$(uname -r)
CPU model=$(lscpu | awk -F: '/Model name/{sub(/^ /,"",$2);print $2;exit}')
logical CPU count=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo unknown)
selected CPU=$CPU
affinity success=$AFFINITY_SUCCESS
CPU governor=$(cat /sys/devices/system/cpu/cpu${CPU}/cpufreq/scaling_governor 2>/dev/null || echo unavailable)
turbo state=$(cat /sys/devices/system/cpu/intel_pstate/no_turbo 2>/dev/null || echo unavailable)
AVX2 support=$AVX2
RDTSCP support=$RDTSCP
constant_tsc/invariant_tsc=$CTSC
compiler=$(${CC:-cc} --version | head -1)
cmake version=$(cmake --version | head -1)
CMAKE_BUILD_TYPE=Release
Reference flags=-O3 without -mavx2
AVX2 flags=-O3 -mavx2
timer overhead median=$TIMER_MEDIAN
META
cat > "$OUT/benchmark_config.txt" <<CFG
formal_requested=$FORMAL
quick=$QUICK
not_for_publication=$([ "$FORMAL" -eq 1 ] && echo false || echo true)
warmup_samples=$WARMUP
repetitions=$REPETITIONS
samples_per_repetition=$SAMPLES
batch_sizes=$BATCHES
skip_tests=$SKIP_TESTS
skip_lookup=$SKIP_LOOKUP
skip_end_to_end=$SKIP_E2E
CFG
if [ "$SKIP_TESTS" -ne 1 ]; then
  ctest --test-dir "$BUILD" -R test_sdat_online --output-on-failure
  echo correctness_tests=pass >> "$OUT/benchmark_config.txt"
else echo correctness_tests=skipped >> "$OUT/benchmark_config.txt"; fi
RUN_KIND=all
[ "$SKIP_LOOKUP" -eq 1 ] && RUN_KIND=end-to-end
[ "$SKIP_E2E" -eq 1 ] && RUN_KIND=lookup-only
if [ "$SKIP_LOOKUP" -eq 1 ] && [ "$SKIP_E2E" -eq 1 ]; then echo "both benchmark kinds skipped" >&2; exit 7; fi
export ONLINE_BENCH_SAMPLES=$SAMPLES ONLINE_BENCH_REPETITIONS=$REPETITIONS ONLINE_BENCH_WARMUP=$WARMUP ONLINE_BENCH_BATCH_SIZES=$BATCHES ONLINE_BENCH_KIND=$RUN_KIND
RAW="$OUT/raw_all.csv"
"${TASKSET[@]}" "$BENCH" > "$RAW"
awk -F, 'NR==1 || $6=="lookup-only"' "$RAW" > "$OUT/lookup_raw.csv"
awk -F, 'NR==1 || $6=="end-to-end"' "$RAW" > "$OUT/end_to_end_raw.csv"
python3 "$ROOT/benchmark/scripts/summarize_online_benchmarks.py" --raw "$RAW" --out-dir "$OUT" $([ "$FORMAL" -eq 1 ] && echo --formal || true) $([ "$QUICK" -eq 1 ] && echo --quick || true)
echo "$OUT" > "$OUT_BASE/latest.txt"
echo "formal_artifact_dir=$OUT"
