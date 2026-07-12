#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FRODO_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results}
N=${FRODO_BENCH_SAMPLE_COUNT:-1048576}; R=${FRODO_BENCH_REPETITIONS:-31}; W=${FRODO_BENCH_WARMUP:-5}; P=${FRODO_BENCH_PROCESSES:-3}; CPU=${FRODO_BENCH_CPU:-0}
SAMPLE=$BUILD/benchmark_frodo_sample_n; BREAK=$BUILD/benchmark_frodo_breakdown
[ -x "$SAMPLE" ] || { echo "missing $SAMPLE" >&2; exit 2; }
[ -x "$BREAK" ] || { echo "missing $BREAK" >&2; exit 2; }
mkdir -p "$OUT"; : > "$OUT/frodo_sample_n_raw.csv"; : > "$OUT/frodo_breakdown_raw.csv"
run_one(){ local exe=$1 out=$2 tmp=$3; if command -v taskset >/dev/null; then taskset -c "$CPU" "$exe" > "$tmp"; else "$exe" > "$tmp"; fi; if [ ! -s "$out" ]; then cat "$tmp" > "$out"; else tail -n +2 "$tmp" >> "$out"; fi; }
for i in $(seq 1 "$P"); do
 export FRODO_BENCH_SAMPLE_COUNT=$N FRODO_BENCH_REPETITIONS=$R FRODO_BENCH_WARMUP=$W FRODO_BENCH_ORDER_SEED=$i
 run_one "$SAMPLE" "$OUT/frodo_sample_n_raw.csv" "$OUT/sample.$i.csv"
 run_one "$BREAK" "$OUT/frodo_breakdown_raw.csv" "$OUT/breakdown.$i.csv"
done
python3 "$ROOT/benchmark/scripts/summarize_frodo_benchmark.py" --sample-raw "$OUT/frodo_sample_n_raw.csv" --breakdown-raw "$OUT/frodo_breakdown_raw.csv" --out-dir "$OUT"
echo "raw: $OUT/frodo_sample_n_raw.csv $OUT/frodo_breakdown_raw.csv"
echo "summary: $OUT/frodo_summary.csv $OUT/frodo_summary.md $OUT/frodo_summary.json"
sed -n '/## Reference full-sampler comparison/,/## Breakdown/p' "$OUT/frodo_summary.md"
! grep -q ',error$' "$OUT/frodo_sample_n_raw.csv" "$OUT/frodo_breakdown_raw.csv"
