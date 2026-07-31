#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FRODO_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results/frodo}
JOBS=${FRODO_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}
PROCESSES=${FRODO_BENCH_PROCESSES:-5}
REPETITIONS=${FRODO_BENCH_REPETITIONS:-31}
WARMUP=${FRODO_BENCH_WARMUP:-5}
SAMPLE_COUNT=${FRODO_BENCH_SAMPLE_COUNT:-1048576}
MODE=${FRODO_BENCH_MODE:-equal-size}
BLOCK_SIZE=${FRODO_STAGE_BLOCK_SIZE:-4096}
[[ $BLOCK_SIZE -gt 0 ]] || { echo "FRODO_STAGE_BLOCK_SIZE must be positive" >&2; exit 2; }
mkdir -p "$OUT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_frodo -j"$JOBS"
run_bench=()
if [[ -n ${FRODO_BENCH_CPU:-} ]]; then
  if command -v taskset >/dev/null 2>&1; then run_bench=(taskset -c "$FRODO_BENCH_CPU")
  else echo "warning: taskset unavailable; continuing without CPU pinning" >&2; fi
fi
FULL="$OUT/frodo_full_sampler_raw.csv"
STAGES="$OUT/frodo_stage_breakdown_raw.csv"
REPORT="$OUT/frodo_benchmark_validation.txt"
rm -f "$FULL" "$STAGES" "$REPORT" "$OUT/frodo_original_raw.csv" "$OUT/frodo_sda_raw.csv" "$OUT/frodo_microbench_raw.csv" "$OUT/frodo_sample_n_raw.csv" "$OUT/frodo_breakdown_raw.csv"
append_run(){
  local implementation=$1 process=$2 tmp
  tmp="$OUT/.frodo.$$.${implementation}.${process}.csv"
  FRODO_BENCH_IMPLEMENTATION=$implementation FRODO_BENCH_SCOPE=all \
    FRODO_BENCH_PROCESS_INDEX=$process FRODO_BENCH_REPETITIONS=$REPETITIONS \
    FRODO_BENCH_WARMUP=$WARMUP FRODO_BENCH_SAMPLE_COUNT=$SAMPLE_COUNT \
    FRODO_BENCH_MODE=$MODE FRODO_STAGE_BLOCK_SIZE=$BLOCK_SIZE \
    "${run_bench[@]}" "$BUILD/benchmark_frodo" > "$tmp"
  if [[ ! -s $FULL ]]; then head -n 1 "$tmp" > "$FULL"; head -n 1 "$tmp" > "$STAGES"; fi
  awk -F, 'NR>1&&($5=="full-sampler-fused"||$5=="full-sampler-block-staged")' "$tmp" >> "$FULL"
  awk -F, 'NR>1&&($5=="block-stage-input"||$5=="block-stage-mapping")' "$tmp" >> "$STAGES"
  rm -f "$tmp"
}
for ((p=0;p<PROCESSES;p++)); do
  append_run original "$p"
  append_run sda "$p"
done
expected=$((1 + PROCESSES * REPETITIONS * 3 * 2 * 2))
[[ $(wc -l < "$FULL") -eq $expected ]] || { echo "unexpected full CSV line count" >&2; exit 1; }
[[ $(wc -l < "$STAGES") -eq $expected ]] || { echo "unexpected stage CSV line count" >&2; exit 1; }
awk -F, -v report="$REPORT" '
function key(){return $2 SUBSEP $6 SUBSEP $16 SUBSEP $18 SUBSEP $20 SUBSEP $21}
FNR==1{if(NF!=51){print "invalid header width" > "/dev/stderr";bad=1}next}
NF!=51{print "invalid field count" > "/dev/stderr";bad=1}
$18 ~ /^-/{print "warm-up row present" > "/dev/stderr";bad=1}
$33!="ok"{print "non-ok status" > "/dev/stderr";bad=1}
{u=$2 SUBSEP $5 SUBSEP $6 SUBSEP $16 SUBSEP $18;if(unique[u]++){print "duplicate row key" > "/dev/stderr";bad=1};k=key();seen_impl[$6]=1}
$5=="full-sampler-fused"{mask[k]+=1;fc[k]=$22;fck[k]=$31;fa[k]=$25;fr[k]=$26;fw[k]=$28}
$5=="full-sampler-block-staged"{mask[k]+=2;sc[k]=$22;sck[k]=$31;sa[k]=$25;sr[k]=$26;sw[k]=$28}
$5=="block-stage-input"{mask[k]+=4;ic[k]=$22;recon[k]=$42;ick[k]=$31;ia[k]=$25;ir[k]=$26;iw[k]=$28}
$5=="block-stage-mapping"{mask[k]+=8;mc[k]=$22;if(recon[k]&&recon[k]!=$42)bad=1;mck[k]=$31;ma[k]=$25;mr[k]=$26;mw[k]=$28}
END{
 if(!seen_impl["original-reference"]||!seen_impl["sda-word-reference"]){print "missing implementation" > "/dev/stderr";bad=1}
 print "Frodo block-staged benchmark validation" > report
 print "gap_percent = 100 * (paired_input_cycles + paired_mapping_cycles) / staged_full_cycles - 100" >> report
 print "overhead_percent = 100 * staged_full_cycles / fused_full_cycles - 100" >> report
 for(k in mask){
  if(mask[k]!=15){print "incomplete matched key" > "/dev/stderr";bad=1;continue}
  if(ic[k]+mc[k]!=recon[k]){print "stage reconstruction mismatch" > "/dev/stderr";bad=1}
  if(fck[k]!=sck[k]||sck[k]!=ick[k]||ick[k]!=mck[k]){print "checksum mismatch" > "/dev/stderr";bad=1}
  if(fa[k]!=sa[k]||sa[k]!=ia[k]||ia[k]!=ma[k]||fr[k]!=sr[k]||sr[k]!=ir[k]||ir[k]!=mr[k]||fw[k]!=sw[k]||sw[k]!=iw[k]||iw[k]!=mw[k]){print "accounting mismatch" > "/dev/stderr";bad=1}
  split(k,a,SUBSEP);group=a[1] "/" a[2];gap=100*((ic[k]+mc[k])/sc[k]-1);over=100*(sc[k]/fc[k]-1);gaps[group]+=gap;overs[group]+=over;counts[group]++
 }
 for(g in counts)printf "%s mean_stage_sum_gap_percent=%.6f mean_staging_overhead_percent=%.6f\n",g,gaps[g]/counts[g],overs[g]/counts[g] >> report
 exit bad
}' "$FULL" "$STAGES"
echo "raw CSV: $FULL $STAGES"
echo "validation: $REPORT"
