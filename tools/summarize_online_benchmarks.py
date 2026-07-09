#!/usr/bin/env python3
import argparse, csv, math, os, statistics
from collections import defaultdict

def pct(vals, p):
    vals=sorted(vals)
    if not vals: return float('nan')
    k=(len(vals)-1)*p/100.0
    f=math.floor(k); c=math.ceil(k)
    if f==c: return vals[int(k)]
    return vals[f]*(c-k)+vals[c]*(k-f)

def median(vals): return statistics.median(vals) if vals else float('nan')
def mad(vals):
    m=median(vals); return median([abs(x-m) for x in vals]) if vals else float('nan')
def mean(vals): return statistics.mean(vals) if vals else float('nan')
def std(vals): return statistics.pstdev(vals) if len(vals)>1 else 0.0

def read_rows(path):
    with open(path,newline='') as f: return list(csv.DictReader(f))
def write_csv(path, rows, fields):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path,'w',newline='') as f:
        w=csv.DictWriter(f,fieldnames=fields); w.writeheader(); w.writerows(rows)
def key_summary(r): return (r['scheme'],r['parameter_set'],r['sampler_family'],r['table_family'],r['implementation'],r['benchmark_kind'],r['batch_size'])
def summarize(rows):
    groups=defaultdict(list)
    for r in rows: groups[key_summary(r)].append(r)
    out=[]
    for k,rs in sorted(groups.items()):
        vals=[float(r['cycles_per_sample']) for r in rs]
        base=rs[0]
        out.append({
            'scheme':k[0],'parameter_set':k[1],'sampler_family':k[2],'table_family':k[3],'implementation':k[4],'benchmark_kind':k[5],'batch_size':k[6],
            'lane_width':base['lane_width'],'repetitions':str(len(rs)),'samples_per_repetition':base['samples_per_repetition'],
            'median_cycles_per_sample':f'{median(vals):.6f}','MAD':f'{mad(vals):.6f}','p10':f'{pct(vals,10):.6f}','p90':f'{pct(vals,90):.6f}',
            'min':f'{min(vals):.6f}','max':f'{max(vals):.6f}','mean':f'{mean(vals):.6f}','standard_deviation':f'{std(vals):.6f}',
            'timer_overhead':base['timer_overhead'],
            'attempts_per_sample':f"{mean([float(r['attempts_per_sample']) for r in rs]):.9f}",
            'rejections_per_sample':f"{mean([float(r['rejections_per_sample']) for r in rs]):.9f}",
            'acceptance_ratio':f"{mean([float(r['acceptance_ratio']) for r in rs]):.9f}",
            'expected_acceptance_ratio':base['expected_acceptance_ratio'],
            'random_bits_per_sample':f"{mean([float(r['random_bits_per_sample']) for r in rs]):.9f}",
            'random_bytes_per_sample':f"{mean([float(r['random_bytes_per_sample']) for r in rs]):.9f}",
            'native_table_bytes':base['native_table_bytes'],'packed_bits':base['packed_bits'],
            'vector_batches':str(sum(int(r['vector_batches']) for r in rs)),'vectorized_samples':str(sum(int(r['vectorized_samples']) for r in rs)),
            'scalar_tail_samples':str(sum(int(r['scalar_tail_samples']) for r in rs)),'fallback_samples':str(sum(int(r['fallback_samples']) for r in rs)),
            'refill_rounds':str(sum(int(r['refill_rounds']) for r in rs)),'rejected_lanes':str(sum(int(r['rejected_lanes']) for r in rs)),
            'avx2_path_executed':str(int(any(int(r['avx2_path_executed']) for r in rs))),'status':'available','reason':''})
    return out

def speedups(summary, dim):
    idx={(r['scheme'],r['parameter_set'],r['benchmark_kind'],r['batch_size'],r['implementation']):r for r in summary}
    keys=sorted({(r['scheme'],r['parameter_set'],r['benchmark_kind'],r['batch_size']) for r in summary})
    out=[]
    for sch,param,kind,batch in keys:
        if dim == 'original-avx2-vs-ref':
            bimpl,cimpl,bs,cs='original-cdt-ref','original-cdt-avx2','original-cdt','original-cdt'
        elif dim == 'sda-avx2-vs-ref':
            bimpl,cimpl=('sda-table-ref','sda-table-avx2') if kind == 'lookup-only' else ('sda-cdt-ref','sda-cdt-avx2')
            bs=cs='sda-cdt' if kind == 'end-to-end' else 'sda-table-lookup'
        elif dim == 'sda-vs-original-ref':
            bimpl,cimpl=('original-cdt-ref','sda-table-ref') if kind == 'lookup-only' else ('original-cdt-ref','sda-cdt-ref')
            bs,cs='original-cdt','sda-cdt' if kind == 'end-to-end' else 'sda-table-lookup'
        elif dim == 'sda-vs-original-avx2':
            bimpl,cimpl=('original-cdt-avx2','sda-table-avx2') if kind == 'lookup-only' else ('original-cdt-avx2','sda-cdt-avx2')
            bs,cs='original-cdt','sda-cdt' if kind == 'end-to-end' else 'sda-table-lookup'
        else:
            raise ValueError(dim)
        b=idx.get((sch,param,kind,batch,bimpl)); c=idx.get((sch,param,kind,batch,cimpl))
        row={'comparison_dimension':dim,'scheme':sch,'parameter_set':param,'benchmark_kind':kind,'batch_size':batch,'baseline_sampler':bs,'candidate_sampler':cs,'baseline_implementation':bimpl,'candidate_implementation':cimpl,'baseline_median_cycles':'','candidate_median_cycles':'','speedup_percent':'','result':'unavailable'}
        if b and c:
            bv=float(b['median_cycles_per_sample']); cv=float(c['median_cycles_per_sample']); sp=(bv-cv)/bv*100.0 if bv else float('nan')
            row.update({'baseline_median_cycles':f'{bv:.6f}','candidate_median_cycles':f'{cv:.6f}','speedup_percent':f'{sp:.6f}','result':'faster' if sp>0.000001 else ('slower' if sp<-0.000001 else 'equal')})
        out.append(row)
    return out

def table_memory(summary):
    seen={}
    for r in summary:
        seen[(r['scheme'],r['parameter_set'],r['table_family'])]=r
    rows=[]
    for (sch,param,tf),r in sorted(seen.items()):
        rows.append({'scheme':sch,'parameter_set':param,'table_family':tf,'support_length':'','threshold_count':'','threshold_encoding':'see online manifest','terminal_q_stored':'false','threshold_payload_bytes':r['native_table_bytes'],'full_cumulative_bytes':'','PMF_bytes':'','metadata_bytes':'not measured','actual_static_object_bytes':'not measured','packed_bits':r['packed_bits']})
    return rows

def randomness(summary):
    seen={}
    for r in summary:
        if r['benchmark_kind']=='end-to-end': seen[(r['scheme'],r['parameter_set'],r['sampler_family'],r['implementation'],r['batch_size'])]=r
    rows=[]
    for k,r in sorted(seen.items()):
        rows.append({'scheme':r['scheme'],'parameter_set':r['parameter_set'],'sampler_family':r['sampler_family'],'implementation':r['implementation'],'batch_size':r['batch_size'],'q':'see online manifest','draw_bits':'see online manifest','power_of_two_ceiling':'see online manifest','expected_acceptance':r['expected_acceptance_ratio'],'measured_acceptance':r['acceptance_ratio'],'expected_attempts':f"{(1.0/float(r['expected_acceptance_ratio'])) if float(r['expected_acceptance_ratio']) else float('nan'):.9f}",'measured_attempts':r['attempts_per_sample'],'expected_random_bits':'','measured_random_bits':r['random_bits_per_sample'],'measured_random_bytes':r['random_bytes_per_sample']})
    return rows

def zh_summary(path, summary, speeds, formal, quick, reps, samples):
    with open(path,'w') as f:
        f.write('在线 Original CDT 与 SDA_CDT benchmark 中文摘要\n')
        f.write(f'formal={str(formal).lower()}\nquick={str(quick).lower()}\nnot_for_publication={str(not formal).lower()}\n')
        f.write(f'repetitions={reps}\nsamples_per_repetition={samples}\n')
        f.write('主结论必须使用 end-to-end；lookup-only 仅解释 table lookup、SIMD 和表大小成本。\n')
        for dim,rows in speeds.items():
            f.write(f'\ncomparison_dimension={dim}\n')
            for r in rows:
                if r['benchmark_kind']=='end-to-end':
                    f.write(f"{r['scheme']},{r['parameter_set']},batch={r['batch_size']},speedup={r['speedup_percent']},result={r['result']}\n")

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--raw',required=True); ap.add_argument('--out-dir',required=True); ap.add_argument('--formal',action='store_true'); ap.add_argument('--quick',action='store_true'); args=ap.parse_args()
    rows=read_rows(args.raw); summary=summarize(rows)
    samples=int(summary[0]['samples_per_repetition']) if summary else 0; reps=min(int(r['repetitions']) for r in summary) if summary else 0
    formal=args.formal and reps>=21 and samples>=1000000
    if args.formal and not formal: print('warning: requested formal but raw data is below formal thresholds')
    fields=list(summary[0].keys()) if summary else []
    lookup=[r for r in summary if r['benchmark_kind']=='lookup-only']; e2e=[r for r in summary if r['benchmark_kind']=='end-to-end']
    write_csv(os.path.join(args.out_dir,'lookup_summary.csv'),lookup,fields); write_csv(os.path.join(args.out_dir,'end_to_end_summary.csv'),e2e,fields)
    dims=['original-avx2-vs-ref','sda-avx2-vs-ref','sda-vs-original-ref','sda-vs-original-avx2']
    speed_fields=['comparison_dimension','scheme','parameter_set','benchmark_kind','batch_size','baseline_sampler','candidate_sampler','baseline_implementation','candidate_implementation','baseline_median_cycles','candidate_median_cycles','speedup_percent','result']
    speeds={}
    for d in dims:
        rows=speedups(summary,d); speeds[d]=rows; write_csv(os.path.join(args.out_dir,'speedup_'+d.replace('-','_')+'.csv'),rows,speed_fields)
    allsp=[r for d in dims for r in speeds[d]]; write_csv(os.path.join(args.out_dir,'all_speedups.csv'),allsp,speed_fields)
    write_csv(os.path.join(args.out_dir,'table_memory.csv'),table_memory(summary),['scheme','parameter_set','table_family','support_length','threshold_count','threshold_encoding','terminal_q_stored','threshold_payload_bytes','full_cumulative_bytes','PMF_bytes','metadata_bytes','actual_static_object_bytes','packed_bits'])
    write_csv(os.path.join(args.out_dir,'randomness_cost.csv'),randomness(summary),['scheme','parameter_set','sampler_family','implementation','batch_size','q','draw_bits','power_of_two_ceiling','expected_acceptance','measured_acceptance','expected_attempts','measured_attempts','expected_random_bits','measured_random_bits','measured_random_bytes'])
    zh_summary(os.path.join(args.out_dir,'benchmark_summary_zh.txt'),summary,speeds,formal,args.quick,reps,samples)
    with open(os.path.join(args.out_dir,'benchmark_config.txt'),'a') as f:
        f.write(f'formal={str(formal).lower()}\nquick={str(args.quick).lower()}\nnot_for_publication={str(not formal).lower()}\nsummary_repetitions_min={reps}\nsummary_samples_per_repetition={samples}\n')
if __name__=='__main__': main()
