#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-benchmark}
OUT=${FALCON_BENCH_RESULTS_DIR:-$ROOT/build/benchmark-results/falcon}
JOBS=${FALCON_BENCH_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}
PROCESSES=${FALCON_BENCH_PROCESSES:-5}
REPETITIONS=${FALCON_BENCH_REPETITIONS:-31}
WARMUP=${FALCON_BENCH_WARMUP:-5}
SAMPLE_COUNT=${FALCON_BENCH_SAMPLE_COUNT:-1048576}
BLOCK_SIZE=${FALCON_STAGE_BLOCK_SIZE:-4096}
[[ $BLOCK_SIZE -gt 0 ]] || { echo "FALCON_STAGE_BLOCK_SIZE must be positive" >&2; exit 2; }
mkdir -p "$OUT"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_falcon -j"$JOBS"
run_bench=()
if [[ -n ${FALCON_BENCH_CPU:-} ]]; then
  if command -v taskset >/dev/null 2>&1; then run_bench=(taskset -c "$FALCON_BENCH_CPU")
  else echo "warning: taskset unavailable; continuing without CPU pinning" >&2; fi
fi
FULL="$OUT/falcon_full_sampler_raw.csv"
STAGES="$OUT/falcon_stage_breakdown_raw.csv"
REPORT="$OUT/falcon_benchmark_validation.txt"
rm -f "$FULL" "$STAGES" "$REPORT" "$OUT/falcon_original_raw.csv" "$OUT/falcon_sda_raw.csv" "$OUT/falcon_microbench_raw.csv" "$OUT/falcon_sample_n_raw.csv" "$OUT/falcon_breakdown_raw.csv" "$OUT/falcon_base_sampler_raw.csv"
append_run(){
  local implementation=$1 process=$2 tmp
  tmp="$OUT/.falcon.$$.${implementation}.${process}.csv"
  FALCON_BENCH_IMPLEMENTATION=$implementation FALCON_BENCH_PROCESS_INDEX=$process \
    FALCON_BENCH_REPETITIONS=$REPETITIONS FALCON_BENCH_WARMUP=$WARMUP \
    FALCON_BENCH_SAMPLE_COUNT=$SAMPLE_COUNT FALCON_STAGE_BLOCK_SIZE=$BLOCK_SIZE \
    "${run_bench[@]}" "$BUILD/benchmark_falcon" > "$tmp"
  if [[ ! -s $FULL ]]; then head -n 1 "$tmp" > "$FULL"; head -n 1 "$tmp" > "$STAGES"; fi
  awk -F, 'NR>1&&($5=="full-sampler-fused"||$5=="full-sampler-block-staged")' "$tmp" >> "$FULL"
  awk -F, 'NR>1&&($5=="block-stage-input"||$5=="block-stage-mapping")' "$tmp" >> "$STAGES"
  rm -f "$tmp"
}
for ((p=0;p<PROCESSES;p++)); do
  append_run original "$p"
  append_run sda "$p"
done
expected=$((1 + PROCESSES * REPETITIONS * 2 * 2))
[[ $(wc -l < "$FULL") -eq $expected ]] || { echo "unexpected Falcon full CSV line count" >&2; exit 1; }
[[ $(wc -l < "$STAGES") -eq $expected ]] || { echo "unexpected Falcon stage CSV line count" >&2; exit 1; }
awk -F, -v report="$REPORT" '
function abs(x){return x<0?-x:x}
function key(){return $2 SUBSEP $6 SUBSEP $16 SUBSEP $18 SUBSEP $20 SUBSEP $21}
FNR==1{if(NF!=51){print "invalid header width" > "/dev/stderr";bad=1}next}
NF!=51{print "invalid field count" > "/dev/stderr";bad=1}
$18 ~ /^-/{print "warm-up row present" > "/dev/stderr";bad=1}
$33!="ok"{print "non-ok status" > "/dev/stderr";bad=1}
{u=$2 SUBSEP $5 SUBSEP $6 SUBSEP $16 SUBSEP $18;if(unique[u]++){print "duplicate row key" > "/dev/stderr";bad=1};k=key();impl[$6]=1}
$5=="full-sampler-fused"{mask[k]+=1;fc[k]=$22;fck[k]=$31;fa[k]=$25;fr[k]=$26;fw[k]=$28}
$5=="full-sampler-block-staged"{mask[k]+=2;sc[k]=$22;sck[k]=$31;sa[k]=$25;sr[k]=$26;sw[k]=$28}
$5=="block-stage-input"{mask[k]+=4;ic[k]=$22;recon[k]=$42;outer[k]=$48;uncovered[k]=$50;ick[k]=$31;ia[k]=$25;ir[k]=$26;iw[k]=$28}
$5=="block-stage-mapping"{mask[k]+=8;mc[k]=$22;if(recon[k]&&recon[k]!=$42)bad=1;if(outer[k]&&outer[k]!=$48)bad=1;mck[k]=$31;ma[k]=$25;mr[k]=$26;mw[k]=$28}
END{
 if(!impl["original-reference"]||!impl["sda-word-reference"]){print "missing implementation" > "/dev/stderr";bad=1}
 print "Falcon base-sampler block-staged benchmark validation" > report
 for(k in mask){
  if(mask[k]!=15){print "incomplete matched key" > "/dev/stderr";bad=1;continue}
  if(ic[k]+mc[k]!=recon[k]){print "stage reconstruction mismatch" > "/dev/stderr";bad=1}
  if(recon[k]>outer[k]||outer[k]-recon[k]!=uncovered[k]){print "instrumented outer accounting mismatch" > "/dev/stderr";bad=1}
  if(fck[k]!=sck[k]||sck[k]!=ick[k]||ick[k]!=mck[k]){print "checksum mismatch" > "/dev/stderr";bad=1}
  if(fa[k]!=sa[k]||sa[k]!=ia[k]||ia[k]!=ma[k]||fr[k]!=sr[k]||sr[k]!=ir[k]||ir[k]!=mr[k]||fw[k]!=sw[k]||sw[k]!=iw[k]||iw[k]!=mw[k]){print "accounting mismatch" > "/dev/stderr";bad=1}
  split(k,a,SUBSEP);g=a[1] "/" a[2];gap=100*(recon[k]/sc[k]-1);account=100*(recon[k]/outer[k]-1);difference=100*(sc[k]/fc[k]-1);gaps[g]+=gap;accounts[g]+=account;differences[g]+=difference;counts[g]++
  if(abs(gap)>10||abs(account)>10)printf "WARNING %s process=%s repetition=%s: gap=%.3f%% accounting_gap=%.3f%%; inspect CPU, timer, and instrumentation\n",g,a[3],a[4],gap,account > "/dev/stderr"
  else if(abs(gap)>3||abs(account)>3)printf "warning %s process=%s repetition=%s: gap=%.3f%% accounting_gap=%.3f%%\n",g,a[3],a[4],gap,account > "/dev/stderr"
 }
 for(g in counts)printf "%s mean_stage_sum_gap_percent=%.6f mean_instrumented_accounting_gap_percent=%.6f mean_staged_versus_fused_difference_percent=%.6f\n",g,gaps[g]/counts[g],accounts[g]/counts[g],differences[g]/counts[g] >> report
 exit bad
}' "$FULL" "$STAGES"
echo "raw CSV: $FULL $STAGES"
echo "validation: $REPORT"
