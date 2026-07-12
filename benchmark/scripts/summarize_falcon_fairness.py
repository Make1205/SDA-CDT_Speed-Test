#!/usr/bin/env python3
import argparse, csv, json, statistics
from collections import defaultdict

def median(xs): return statistics.median(xs) if xs else None

def percentile(xs, p):
    if not xs: return None
    xs=sorted(xs); pos=(len(xs)-1)*p/100.0; lo=int(pos); hi=min(lo+1,len(xs)-1); frac=pos-lo
    return xs[lo]*(1-frac)+xs[hi]*frac

def fmt(x): return "" if x is None else f"{x:.6f}"

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('csv', nargs='+')
    ap.add_argument('--out-prefix', default='falcon_fairness_summary')
    args=ap.parse_args()
    rows=[]
    for path in args.csv:
        with open(path,newline='') as f:
            for r in csv.DictReader(f):
                r['source_file']=path; r['cycles_per_output_f']=float(r['cycles_per_output']); rows.append(r)
    impls=['original-current','sda-old-generic','sda-new-batch']
    bad=sum(1 for r in rows if r.get('status')!='ok' or r.get('equivalence_status')!='ok')
    proc_ids=sorted({r['process_id'] for r in rows})
    impl_summary=[]
    for impl in impls:
        vals=[r['cycles_per_output_f'] for r in rows if r['implementation']==impl]
        proc_meds=[]
        for p in proc_ids:
            pv=[r['cycles_per_output_f'] for r in rows if r['implementation']==impl and r['process_id']==p]
            if pv: proc_meds.append(median(pv))
        impl_summary.append({'implementation':impl,'pooled_median':median(vals),'process_medians':proc_meds,'median_of_process_medians':median(proc_meds),'outlier_count':0})
    by_rep=defaultdict(dict)
    for r in rows: by_rep[(r['process_id'],r['repetition'])][r['implementation']]=r['cycles_per_output_f']
    ratios=[]
    for name,num,den in [('sda_old_over_original','sda-old-generic','original-current'),('sda_new_over_original','sda-new-batch','original-current'),('sda_new_over_sda_old','sda-new-batch','sda-old-generic')]:
        vals=[d[num]/d[den] for d in by_rep.values() if num in d and den in d]
        ratios.append({'ratio':name,'n':len(vals),'paired_ratio_median':median(vals),'paired_ratio_p10':percentile(vals,10),'paired_ratio_p90':percentile(vals,90),'paired_delta_percent':(median(vals)-1)*100 if vals else None})
    order_summary=[]
    for order in sorted({r['order_pattern'] for r in rows}):
        rec={'order_pattern':order}
        for impl in impls:
            vals=[r['cycles_per_output_f'] for r in rows if r['order_pattern']==order and r['implementation']==impl]
            rec[impl]=median(vals)
        order_summary.append(rec)
    max_order_delta={}
    for impl in impls:
        vals=[r[impl] for r in order_summary if r.get(impl) is not None]
        max_order_delta[impl]=((max(vals)-min(vals))/median(vals)*100) if vals and median(vals) else None
    data={'row_count':len(rows),'bad_rows':bad,'implementations':impl_summary,'paired_ratios':ratios,'order_medians':order_summary,'max_order_delta_percent':max_order_delta}
    with open(args.out_prefix+'.json','w') as f: json.dump(data,f,indent=2)
    with open(args.out_prefix+'.csv','w',newline='') as f:
        w=csv.writer(f); w.writerow(['type','name','pooled_median','median_of_process_medians','process_medians','paired_ratio_median','paired_ratio_p10','paired_ratio_p90','paired_delta_percent'])
        for r in impl_summary: w.writerow(['implementation',r['implementation'],fmt(r['pooled_median']),fmt(r['median_of_process_medians']),' '.join(fmt(x) for x in r['process_medians']),'','','',''])
        for r in ratios: w.writerow(['paired_ratio',r['ratio'],'','','',fmt(r['paired_ratio_median']),fmt(r['paired_ratio_p10']),fmt(r['paired_ratio_p90']),fmt(r['paired_delta_percent'])])
    with open(args.out_prefix+'.md','w') as f:
        f.write('# Falcon fairness summary\n\n')
        f.write(f'bad_rows = {bad}\n\n')
        f.write('## Same-binary implementations\n\n| Implementation | Pooled median | Median of process medians | Process medians |\n|---|---:|---:|---|\n')
        for r in impl_summary: f.write(f"| {r['implementation']} | {fmt(r['pooled_median'])} | {fmt(r['median_of_process_medians'])} | {' '.join(fmt(x) for x in r['process_medians'])} |\n")
        f.write('\n## Paired ratios\n\n| Ratio | Median | P10 | P90 | Delta % |\n|---|---:|---:|---:|---:|\n')
        for r in ratios: f.write(f"| {r['ratio']} | {fmt(r['paired_ratio_median'])} | {fmt(r['paired_ratio_p10'])} | {fmt(r['paired_ratio_p90'])} | {fmt(r['paired_delta_percent'])} |\n")
        f.write('\n## Order medians\n\n| Order | Original current | SDA old generic | SDA new batch |\n|---|---:|---:|---:|\n')
        for r in order_summary: f.write(f"| {r['order_pattern']} | {fmt(r['original-current'])} | {fmt(r['sda-old-generic'])} | {fmt(r['sda-new-batch'])} |\n")
        f.write('\n## Max order delta percent\n\n')
        for k,v in max_order_delta.items(): f.write(f'- {k}: {fmt(v)}\n')
    print(args.out_prefix+'.md')
    return 1 if bad else 0
if __name__=='__main__': raise SystemExit(main())
