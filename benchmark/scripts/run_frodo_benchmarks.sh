#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FRODO_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results/frodo}
JOBS=${FRODO_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}
PROCESSES=${FRODO_BENCH_PROCESSES:-5}
export FRODO_BENCH_REPETITIONS=${FRODO_BENCH_REPETITIONS:-31}
export FRODO_BENCH_WARMUP=${FRODO_BENCH_WARMUP:-5}
export FRODO_BENCH_SAMPLE_COUNT=${FRODO_BENCH_SAMPLE_COUNT:-1048576}
export FRODO_BENCH_MODE=${FRODO_BENCH_MODE:-equal-size}
SCOPES=${FRODO_BENCH_SCOPES:-full,micro}
mkdir -p "$OUT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_frodo -j"$JOBS"
run_bench=()
if [[ -n ${FRODO_BENCH_CPU:-} ]]; then
  if command -v taskset >/dev/null 2>&1; then run_bench=(taskset -c "$FRODO_BENCH_CPU")
  else echo "warning: taskset unavailable; continuing without CPU pinning" >&2; fi
fi
FULL="$OUT/frodo_full_sampler_raw.csv"; MICRO="$OUT/frodo_microbench_raw.csv"
: > "$FULL"; : > "$MICRO"
append_scope(){
  local scope=$1 file=$2 tmp
  for ((p=0;p<PROCESSES;p++)); do
    tmp="$OUT/.frodo.$$.${scope}.${p}.csv"
    FRODO_BENCH_SCOPE=$scope FRODO_BENCH_PROCESS_INDEX=$p "${run_bench[@]}" "$BUILD/benchmark_frodo" > "$tmp"
    if [[ ! -s $file ]]; then cat "$tmp" >> "$file"; else tail -n +2 "$tmp" >> "$file"; fi
    rm -f "$tmp"
  done
}
case ",$SCOPES," in *,full,*) append_scope full "$FULL";; esac
case ",$SCOPES," in *,packed,*) append_scope packed "$FULL";; esac
case ",$SCOPES," in *,avx2,*) append_scope avx2 "$FULL";; esac
case ",$SCOPES," in *,micro,*) append_scope micro "$MICRO";; esac
validate_csv(){
  local file=$1 expected=37
  [[ -s $file ]] || return 0
  awk -F, -v n=$expected 'NF!=n{printf "invalid CSV field count at %s:%d: %d (expected %d)\n",FILENAME,NR,NF,n > "/dev/stderr"; bad=1} END{exit bad}' "$file"
  [[ $(head -n 1 "$file" | grep -o 'scheme' | wc -l) -eq 1 ]]
}
validate_csv "$FULL"; validate_csv "$MICRO"
validate_rows(){
  local file=$1
  [[ -s $file ]] || return 0
  awk -F, 'NR==1{next} $18 ~ /^-/{print "warm-up row found" > "/dev/stderr";bad=1} $33!="ok"{print "non-ok row found" > "/dev/stderr";bad=1} {k=$2 SUBSEP $5 SUBSEP $6 SUBSEP $16 SUBSEP $18;if(seen[k]++){print "duplicate row key" > "/dev/stderr";bad=1}} END{exit bad}' "$file"
}
validate_rows "$FULL"; validate_rows "$MICRO"
if [[ -s $FULL ]] && [[ ",$SCOPES," == *,full,* ]]; then
  expected=$((1 + PROCESSES * FRODO_BENCH_REPETITIONS * 3 * 2))
  [[ ",$SCOPES," == *,packed,* ]] && expected=$((expected + PROCESSES * FRODO_BENCH_REPETITIONS * 3))
  [[ ",$SCOPES," == *,avx2,* ]] && expected=$((expected + PROCESSES * FRODO_BENCH_REPETITIONS * 3))
  actual=$(wc -l < "$FULL"); [[ $actual -eq $expected ]] || { echo "unexpected full CSV rows: $actual != $expected" >&2; exit 1; }
fi
if [[ -s $MICRO ]]; then
  expected=$((1 + PROCESSES * FRODO_BENCH_REPETITIONS * 3 * 6)); actual=$(wc -l < "$MICRO")
  [[ $actual -eq $expected ]] || { echo "unexpected micro CSV rows: $actual != $expected" >&2; exit 1; }
  ! awk -F, 'NR>1 && $5=="full-sampler-core" && ($6=="original-reference" || $6=="sda-word-reference"){found=1} END{exit found?0:1}' "$MICRO" || { echo "authoritative full row found in micro CSV" >&2; exit 1; }
fi
echo "raw CSV: $FULL $MICRO"
