#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FRODO_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results}
N=${FRODO_BENCH_SAMPLE_COUNT:-1048576}
R=${FRODO_BENCH_REPETITIONS:-31}
W=${FRODO_BENCH_WARMUP:-5}
P=${FRODO_BENCH_PROCESSES:-3}
CPU=${FRODO_BENCH_CPU:-0}
MODE=${FRODO_BENCH_MODE:-equal-size}
SAMPLE=$BUILD/benchmark_frodo_sample_n
BREAK=$BUILD/benchmark_frodo_breakdown
[ -x "$SAMPLE" ] || { echo "error: missing executable $SAMPLE; build benchmark_frodo_sample_n first" >&2; exit 2; }
[ -x "$BREAK" ] || { echo "error: missing executable $BREAK; build benchmark_frodo_breakdown first" >&2; exit 2; }
case "$MODE" in equal-size) MODES="equal-size";; native-batch) MODES="native-batch";; both) MODES="equal-size native-batch";; *) echo "error: FRODO_BENCH_MODE must be equal-size, native-batch, or both" >&2; exit 2;; esac
mkdir -p "$OUT"
rm -f "$OUT/frodo_sample_n_raw.csv" "$OUT/frodo_breakdown_raw.csv" "$OUT/frodo_summary.csv" "$OUT/frodo_summary.md" "$OUT/frodo_summary.json" "$OUT"/sample.*.csv "$OUT"/breakdown.*.csv
append_csv(){ local tmp=$1 out=$2; if [ ! -s "$out" ]; then cat "$tmp" > "$out"; else tail -n +2 "$tmp" >> "$out"; fi; }
run_one(){ local exe=$1 tmp=$2 native=$3 label=$4 seed=$5; export FRODO_BENCH_SAMPLE_COUNT=$N FRODO_BENCH_REPETITIONS=$R FRODO_BENCH_WARMUP=$W FRODO_BENCH_ORDER_SEED=$seed FRODO_BENCH_NATIVE_BATCH=$native FRODO_BENCH_MODE_LABEL=$label; if command -v taskset >/dev/null 2>&1; then taskset -c "$CPU" "$exe" > "$tmp"; else echo "warning: taskset unavailable; CPU affinity was not pinned" >&2; "$exe" > "$tmp"; fi; }
for mode in $MODES; do
  native=0; [ "$mode" = native-batch ] && native=1
  for i in $(seq 1 "$P"); do
    run_one "$SAMPLE" "$OUT/sample.$mode.$i.csv" "$native" "$mode" "$i"
    append_csv "$OUT/sample.$mode.$i.csv" "$OUT/frodo_sample_n_raw.csv"
    run_one "$BREAK" "$OUT/breakdown.$mode.$i.csv" "$native" "$mode" "$i"
    append_csv "$OUT/breakdown.$mode.$i.csv" "$OUT/frodo_breakdown_raw.csv"
  done
done
if grep -h -v '^scheme,' "$OUT/frodo_sample_n_raw.csv" "$OUT/frodo_breakdown_raw.csv" | awk -F, 'NF && $NF != "ok" { bad=1 } END { exit bad?0:1 }'; then echo "error: benchmark emitted non-ok status" >&2; exit 3; fi
python3 "$ROOT/benchmark/scripts/summarize_frodo_benchmark.py" --sample-raw "$OUT/frodo_sample_n_raw.csv" --breakdown-raw "$OUT/frodo_breakdown_raw.csv" --out-dir "$OUT" >/dev/null
echo "raw: $OUT/frodo_sample_n_raw.csv $OUT/frodo_breakdown_raw.csv"
echo "summary: $OUT/frodo_summary.csv $OUT/frodo_summary.md $OUT/frodo_summary.json"
sed -n '/## Reference full-sampler comparison/,/## Breakdown/p' "$OUT/frodo_summary.md"
