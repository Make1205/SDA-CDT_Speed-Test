#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FALCON_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results/falcon-formal}
N=${FALCON_BENCH_SAMPLE_COUNT:-1048576}
R=${FALCON_BENCH_REPETITIONS:-31}
W=${FALCON_BENCH_WARMUP:-5}
P=${FALCON_BENCH_PROCESSES:-3}
CPU=${FALCON_BENCH_CPU:-0}
MODE=${FALCON_BENCH_MODE:-equal-size}
BASE=$BUILD/benchmark_falcon_base_sampler
BREAK=$BUILD/benchmark_falcon_breakdown
[ -x "$BASE" ] || { echo "error: missing executable $BASE" >&2; exit 2; }
[ -x "$BREAK" ] || { echo "error: missing executable $BREAK" >&2; exit 2; }
[ "$MODE" = equal-size ] || { echo "error: Falcon benchmark currently supports FALCON_BENCH_MODE=equal-size only" >&2; exit 2; }
mkdir -p "$OUT"
rm -f "$OUT/falcon_base_sampler_raw.csv" "$OUT/falcon_breakdown_raw.csv" "$OUT/falcon_summary.csv" "$OUT/falcon_summary.md" "$OUT/falcon_summary.json" "$OUT"/falcon_base.*.csv "$OUT"/falcon_breakdown.*.csv
append_csv(){ local tmp=$1 out=$2; if [ ! -s "$out" ]; then cat "$tmp" > "$out"; else tail -n +2 "$tmp" >> "$out"; fi; }
run_one(){ local exe=$1 tmp=$2 seed=$3; export FALCON_BENCH_SAMPLE_COUNT=$N FALCON_BENCH_REPETITIONS=$R FALCON_BENCH_WARMUP=$W FALCON_BENCH_MODE=$MODE FALCON_BENCH_ORDER_SEED=$seed; if command -v taskset >/dev/null 2>&1; then taskset -c "$CPU" "$exe" > "$tmp"; else echo "warning: taskset unavailable; CPU affinity was not pinned" >&2; "$exe" > "$tmp"; fi; }
for i in $(seq 1 "$P"); do
  run_one "$BASE" "$OUT/falcon_base.$i.csv" "$i"
  append_csv "$OUT/falcon_base.$i.csv" "$OUT/falcon_base_sampler_raw.csv"
  run_one "$BREAK" "$OUT/falcon_breakdown.$i.csv" "$i"
  append_csv "$OUT/falcon_breakdown.$i.csv" "$OUT/falcon_breakdown_raw.csv"
done
if grep -h -v '^scheme,' "$OUT/falcon_base_sampler_raw.csv" "$OUT/falcon_breakdown_raw.csv" | awk -F, 'NF && $NF != "ok" { bad=1 } END { exit bad?0:1 }'; then echo "error: benchmark emitted non-ok status" >&2; exit 3; fi
python3 "$ROOT/benchmark/scripts/summarize_falcon_benchmark.py" --sample-raw "$OUT/falcon_base_sampler_raw.csv" --breakdown-raw "$OUT/falcon_breakdown_raw.csv" --out-dir "$OUT" >/dev/null
echo "raw: $OUT/falcon_base_sampler_raw.csv $OUT/falcon_breakdown_raw.csv"
echo "summary: $OUT/falcon_summary.csv $OUT/falcon_summary.md $OUT/falcon_summary.json"
sed -n '/## Reference full-sampler comparison/,/## Breakdown/p' "$OUT/falcon_summary.md"
