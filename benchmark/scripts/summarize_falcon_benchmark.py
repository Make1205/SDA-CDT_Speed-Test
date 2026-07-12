#!/usr/bin/env python3
import argparse,csv,json,math,os,statistics
KEY=['parameter_set','sampler_kind','backend','frontend','component','mode','implementation','sample_count']
META={'rng_generation_status':'not_in_benchmark','rng_generation':'N/A (pre-generated PRNG source/state)','component_timing_note':'Component timings are standalone microbenchmarks and are not additive. The production full sampler fuses source frontend, rejection, lookup, sign, and output commit in one loop.','percentile_method':'linear interpolation between sorted samples at rank (n - 1) * p / 100'}
def num(x):
 try:return float(x)
 except Exception:return None
def fmt(x): return '' if x is None else f'{x:.6f}'
def percentile(xs,p):
 xs=sorted(xs)
 if not xs:return None
 if len(xs)==1:return xs[0]
 k=(len(xs)-1)*p/100.0; lo=math.floor(k); hi=math.ceil(k)
 return xs[lo] if lo==hi else xs[lo]*(hi-k)+xs[hi]*(k-lo)
def classify(delta):
 if delta is None:return 'unavailable'
 if delta<=-1.0:return 'SDA faster'
 if delta<1.0:return 'approximately tied'
 if delta<=5.0:return 'small slowdown'
 return 'slower'
def normalize(r):
 aliases={'cycles_per_sample':'cycles_per_output','attempts_per_sample':'attempts_per_output','rejections_per_sample':'rejections_per_output','logical_bits_per_sample':'logical_bits_per_output','physical_bits_per_sample':'physical_bits_per_output'}
 for a,b in aliases.items():
  if a in r and b not in r:r[b]=r[a]
 r.setdefault('frontend','') ; r.setdefault('component','full-sampler-core'); r.setdefault('mode','unknown')
 r.setdefault('process_id','0'); r.setdefault('status','ok')
 return r
def read(path):
 if not path or not os.path.exists(path):return []
 with open(path,newline='') as fp:return [normalize(r) for r in csv.DictReader(fp)]
def stats(vals):
 vals=[v for v in vals if v is not None]
 q1=percentile(vals,25); q3=percentile(vals,75); iqr=(q3-q1) if q1 is not None and q3 is not None else None
 outs=[] if iqr is None else [x for x in vals if x<q1-1.5*iqr or x>q3+1.5*iqr]
 kept=[] if not vals else [x for x in vals if x>=q1-1.5*iqr and x<=q3+1.5*iqr] if iqr is not None else list(vals)
 med=statistics.median(vals) if vals else None
 sd=statistics.stdev(vals) if len(vals)>1 else (0.0 if vals else None)
 mean=statistics.mean(vals) if vals else None
 low_fallback=bool(vals and not kept)
 low=statistics.median(kept) if kept else med
 return {'pooled_median':fmt(med),'pooled_p10':fmt(percentile(vals,10)),'pooled_p25':fmt(q1),'pooled_p75':fmt(q3),'pooled_p90':fmt(percentile(vals,90)),'min':fmt(min(vals) if vals else None),'max':fmt(max(vals) if vals else None),'mean':fmt(mean),'sample_stdev':fmt(sd),'cv':fmt((sd/mean) if vals and mean else None),'mad':fmt(statistics.median([abs(x-med) for x in vals]) if vals else None),'iqr':fmt(iqr),'outlier_count':len(outs),'pooled_low_noise_median':fmt(low),'low_noise_fallback':str(low_fallback).lower()}
def summarize(rows):
 groups={}
 for r in rows:groups.setdefault(tuple(r.get(k,'') for k in KEY),[]).append(r)
 out=[]
 for k,rs in sorted(groups.items()):
  good=[r for r in rs if r.get('status','ok')=='ok' and num(r.get('cycles_per_output')) is not None]
  vals=[num(r.get('cycles_per_output')) for r in good]
  row=dict(zip(KEY,k)); row.update({'n':len(rs),'valid_n':len(good),'error_count':len(rs)-len(good)})
  row.update(stats(vals))
  byp={}
  for r in good:byp.setdefault(r.get('process_id','0'),[]).append(num(r.get('cycles_per_output')))
  proc=[]; detail=[]
  for pid,pvals in sorted(byp.items(),key=lambda x:x[0]):
   pm=statistics.median(pvals); proc.append(pm); detail.append({'process_id':pid,'process_valid_n':len(pvals),'process_median':fmt(pm),'process_p10':fmt(percentile(pvals,10)),'process_p90':fmt(percentile(pvals,90))})
  psd=statistics.stdev(proc) if len(proc)>1 else (0.0 if proc else None); pmean=statistics.mean(proc) if proc else None
  row.update({'process_count':len(proc),'process_medians':';'.join(fmt(x) for x in proc),'median_of_process_medians':fmt(statistics.median(proc) if proc else None),'min_process_median':fmt(min(proc) if proc else None),'max_process_median':fmt(max(proc) if proc else None),'process_median_stdev':fmt(psd),'process_median_cv':fmt((psd/pmean) if proc and pmean else None),'processes':detail})
  for m in ['attempts_per_output','rejections_per_output','source_bytes_per_attempt','physical_bytes_per_output']:
   ms=[num(r.get(m)) for r in good if num(r.get(m)) is not None]
   row[m+'_mean']=fmt(statistics.mean(ms) if ms else None)
  row['status']='ok' if good else 'not_in_benchmark'
  out.append(row)
 return out
def val(r,k): return num(r.get(k,'')) if r else None
def markdown(summary,path):
 idx={(r['mode'],r['parameter_set'],r['implementation'],r['component'],r['backend'],r['frontend']):r for r in summary}
 modes=sorted({r['mode'] for r in summary}); params=sorted({r['parameter_set'] for r in summary})
 lines=['# Falcon benchmark summary','',f"rng_generation_status = {META['rng_generation_status']}",f"RNG generation = {META['rng_generation']}",'',META['component_timing_note'],'',f"Percentile method: {META['percentile_method']}",'','## Reference full-sampler comparison','','| Mode | Parameter | Original process-median | SDA process-median | Delta % | Original pooled median | SDA pooled median | Pooled delta % | Original low-noise median | SDA low-noise median | Low-noise delta % | Original outliers | SDA outliers | Classification |','|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|']
 for mode in modes:
  for p in params:
   o=idx.get((mode,p,'original-reference','full-sampler-core','reference','falcon-prng72')); s=idx.get((mode,p,'sda-reference','full-sampler-core','reference','falcon-sda72'))
   op,sp=val(o,'median_of_process_medians'),val(s,'median_of_process_medians'); om,sm=val(o,'pooled_median'),val(s,'pooled_median'); oln,sln=val(o,'pooled_low_noise_median'),val(s,'pooled_low_noise_median')
   d=(sp/op-1)*100 if op and sp else None; pd=(sm/om-1)*100 if om and sm else None; ld=(sln/oln-1)*100 if oln and sln else None
   if o or s:lines.append(f'| {mode} | {p} | {fmt(op)} | {fmt(sp)} | {fmt(d)} | {fmt(om)} | {fmt(sm)} | {fmt(pd)} | {fmt(oln)} | {fmt(sln)} | {fmt(ld)} | {o.get("outlier_count","") if o else ""} | {s.get("outlier_count","") if s else ""} | {classify(d)} |')
 lines += ['','## Breakdown','','| Mode | Parameter | Sampler | RNG generation | Source frontend | CDT mapping | Full sampler | Attempts/output | Source bytes/output |','|---|---|---|---:|---:|---:|---:|---:|---:|']
 for mode in modes:
  for p in params:
   pairs=[('original-reference','falcon-prng72','benchmark-only-original-source-frontend'),('sda-reference','falcon-sda72','benchmark-only-sda-source-frontend')]
   for impl,front,feimpl in pairs:
    fe=idx.get((mode,p,feimpl,'source-frontend','reference',front)); mp=idx.get((mode,p,impl,'cdt-mapping','reference','mapping-only')); fu=idx.get((mode,p,impl,'full-sampler-core','reference',front))
    if fe or mp or fu:lines.append(f'| {mode} | {p} | {impl} | N/A (pre-generated PRNG source/state) | {fe.get("median_of_process_medians","") if fe else ""} | {mp.get("median_of_process_medians","") if mp else ""} | {fu.get("median_of_process_medians","") if fu else ""} | {fu.get("attempts_per_output_mean","") if fu else (fe.get("attempts_per_output_mean","") if fe else "")} | {fu.get("physical_bytes_per_output_mean","") if fu else (fe.get("physical_bytes_per_output_mean","") if fe else "")} |')
 lines += ['','Standalone component timings; not additive.','','## Randomness','','| Mode | Profile | Sampler | Random precision | Source bytes/attempt | Attempts/output | Physical bytes/output | Rejections/output |','|---|---|---|---:|---:|---:|---:|---:|']
 for r in summary:
  if r['component']=='full-sampler-core' and r['backend']=='reference':lines.append(f'| {r["mode"]} | {r["parameter_set"]} | {r["implementation"]} | 72 | {r["source_bytes_per_attempt_mean"]} | {r["attempts_per_output_mean"]} | {r["physical_bytes_per_output_mean"]} | {r["rejections_per_output_mean"]} |')
 lines += ['','## AVX2 regression (diagnostic only; not used for Reference claims)','','| Mode | Parameter | Implementation | Component | Process median | Pooled median | Outliers |','|---|---|---|---|---:|---:|---:|']
 for r in summary:
  if r['backend']=='avx2':lines.append(f'| {r["mode"]} | {r["parameter_set"]} | {r["implementation"]} | {r["component"]} | {r["median_of_process_medians"]} | {r["pooled_median"]} | {r["outlier_count"]} |')
 open(path,'w').write('\n'.join(lines)+'\n')
def write_outputs(summary,out_dir):
 fields=KEY+['n','valid_n','error_count','process_count','process_medians','median_of_process_medians','min_process_median','max_process_median','process_median_stdev','process_median_cv','pooled_median','pooled_p10','pooled_p25','pooled_p75','pooled_p90','min','max','mean','sample_stdev','cv','mad','iqr','outlier_count','pooled_low_noise_median','low_noise_fallback','attempts_per_output_mean','rejections_per_output_mean','source_bytes_per_attempt_mean','physical_bytes_per_output_mean','status']
 with open(os.path.join(out_dir,'falcon_summary.csv'),'w',newline='') as fp:
  w=csv.DictWriter(fp,fields); w.writeheader(); w.writerows([{k:v for k,v in r.items() if k in fields} for r in summary])
 with open(os.path.join(out_dir,'falcon_summary.json'),'w') as fp:json.dump({'metadata':META,'groups':summary},fp,indent=2)
 markdown(summary,os.path.join(out_dir,'falcon_summary.md'))
def main():
 ap=argparse.ArgumentParser(); ap.add_argument('--sample-raw'); ap.add_argument('--breakdown-raw'); ap.add_argument('--out-dir',required=True); a=ap.parse_args(); os.makedirs(a.out_dir,exist_ok=True)
 summary=summarize(read(a.sample_raw)+read(a.breakdown_raw)); write_outputs(summary,a.out_dir); print(os.path.join(a.out_dir,'falcon_summary.md'))
if __name__=='__main__':main()
