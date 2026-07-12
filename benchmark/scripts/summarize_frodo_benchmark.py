#!/usr/bin/env python3
import argparse,csv,json,math,statistics,os
KEY=['parameter_set','sampler_kind','backend','frontend','component','implementation','sample_count']
def f(x):
 try:return float(x)
 except Exception:return None
def pct(xs,p):
 if not xs:return None
 xs=sorted(xs); k=(len(xs)-1)*p/100; lo=math.floor(k); hi=math.ceil(k)
 return xs[lo] if lo==hi else xs[lo]*(hi-k)+xs[hi]*(k-lo)
def cls(d):
 if d is None:return 'unavailable'
 if d<=-1:return 'SDA faster'
 if d<1:return 'approximately tied'
 if d<=5:return 'small slowdown'
 return 'slower'
def fmt(x): return '' if x is None else f'{x:.6f}'
def read(path):
 if not path or not os.path.exists(path):return []
 with open(path,newline='') as fp:
  rows=[]
  for r in csv.DictReader(fp):
   if 'cycles_per_sample' in r and 'cycles_per_output' not in r:r['cycles_per_output']=r['cycles_per_sample']
   if 'attempts_per_sample' in r and 'attempts_per_output' not in r:r['attempts_per_output']=r['attempts_per_sample']
   if 'rejections_per_sample' in r and 'rejections_per_output' not in r:r['rejections_per_output']=r['rejections_per_sample']
   if 'logical_bits_per_sample' in r and 'logical_bits_per_output' not in r:r['logical_bits_per_output']=r['logical_bits_per_sample']
   if 'physical_bits_per_sample' in r and 'physical_bits_per_output' not in r:r['physical_bits_per_output']=r['physical_bits_per_sample']
   r.setdefault('frontend','');r.setdefault('component','full-sampler-core')
   rows.append(r)
  return rows
def summarize(rows):
 groups={}
 for r in rows:
  k=tuple(r.get(x,'') for x in KEY); groups.setdefault(k,[]).append(r)
 out=[]
 for k,rs in sorted(groups.items()):
  vals=[f(r.get('cycles_per_output','')) for r in rs if r.get('status','ok')=='ok' and f(r.get('cycles_per_output','')) is not None]
  q1=pct(vals,25); q3=pct(vals,75); iqr=(q3-q1) if q1 is not None else None
  outs=[] if iqr is None else [x for x in vals if x<q1-1.5*iqr or x>q3+1.5*iqr]
  kept=[x for x in vals if x not in outs] if vals else []
  med=statistics.median(vals) if vals else None; mean=statistics.mean(vals) if vals else None; sd=statistics.stdev(vals) if len(vals)>1 else 0.0 if vals else None
  mad=statistics.median([abs(x-med) for x in vals]) if vals else None
  row=dict(zip(KEY,k)); row.update(n=len(rs),valid_n=len(vals),median=fmt(med),p10=fmt(pct(vals,10)),p25=fmt(q1),p75=fmt(q3),p90=fmt(pct(vals,90)),min=fmt(min(vals) if vals else None),max=fmt(max(vals) if vals else None),mean=fmt(mean),stdev=fmt(sd),cv=fmt((sd/mean) if vals and mean else None),mad=fmt(mad),iqr=fmt(iqr),outlier_count=len(outs),low_noise_median=fmt(statistics.median(kept) if kept else med),status='ok' if vals else 'not_in_benchmark')
  for m in ['attempts_per_output','rejections_per_output','logical_bits_per_output','physical_bits_per_output']:
   ms=[f(r.get(m,'')) for r in rs if f(r.get(m,'')) is not None]
   row[m+'_mean']=fmt(statistics.mean(ms) if ms else None)
  out.append(row)
 return out
def markdown(sumrows,path):
 idx={(r['parameter_set'],r['implementation'],r['component'],r['backend']):r for r in sumrows}
 params=sorted({r['parameter_set'] for r in sumrows})
 lines=['# Frodo benchmark summary','','rng_generation_status = not_in_benchmark','','Statistics: median is the ordinary median; MAD is median absolute deviation; IQR outliers are x < Q1 - 1.5*IQR or x > Q3 + 1.5*IQR; low_noise_median is the median after removing IQR outliers. Outliers are reported, not silently discarded.','','## Reference full-sampler comparison','','| Parameter | Original Reference median | SDA Word Reference median | Delta % | Original low-noise median | SDA low-noise median | Low-noise delta % | Original outliers | SDA outliers | Classification |','|---|---:|---:|---:|---:|---:|---:|---:|---:|---|']
 for p in params:
  o=idx.get((p,'original-reference','full-sampler-core','reference')); s=idx.get((p,'sda-word-reference','full-sampler-core','reference'))
  def val(r,k): return float(r[k]) if r and r.get(k) else None
  om,sm=val(o,'median'),val(s,'median'); oln,sln=val(o,'low_noise_median'),val(s,'low_noise_median')
  d=(sm/om-1)*100 if om and sm else None; dl=(sln/oln-1)*100 if oln and sln else None
  lines.append(f'| {p} | {fmt(om)} | {fmt(sm)} | {fmt(d)} | {fmt(oln)} | {fmt(sln)} | {fmt(dl)} | {o.get("outlier_count","") if o else ""} | {s.get("outlier_count","") if s else ""} | {cls(dl if dl is not None else d)} |')
 lines += ['','## Breakdown','','| Parameter | Sampler | RNG generation | Source frontend/rejection | CDT mapping | Full sampler | Attempts/output |','|---|---|---:|---:|---:|---:|---:|']
 for p in params:
  for impl in ['original-reference','sda-word-reference']:
   fe=idx.get((p,'benchmark-only-source-frontend','source-frontend','reference')); mp=idx.get((p,impl,'cdt-mapping','reference')); fu=idx.get((p,impl,'full-sampler-core','reference'))
   lines.append(f'| {p} | {impl} | N/A (pre-generated source) | {fe.get("median","") if fe else ""} | {mp.get("median","") if mp else ""} | {fu.get("median","") if fu else ""} | {fu.get("attempts_per_output_mean","") if fu else ""} |')
 lines += ['','## Randomness','','| Parameter | Sampler | Logical bits/output | Physical bits/output | Attempts/output | Rejections/output |','|---|---|---:|---:|---:|---:|']
 for r in sumrows:
  if r['component']=='full-sampler-core' and r['backend']=='reference': lines.append(f'| {r["parameter_set"]} | {r["implementation"]} | {r["logical_bits_per_output_mean"]} | {r["physical_bits_per_output_mean"]} | {r["attempts_per_output_mean"]} | {r["rejections_per_output_mean"]} |')
 lines += ['','## AVX2 regression (diagnostic only; not used for Reference claims)','','| Parameter | Implementation | Component | Median | Low-noise median | Outliers |','|---|---|---|---:|---:|---:|']
 for r in sumrows:
  if r['backend']=='avx2': lines.append(f'| {r["parameter_set"]} | {r["implementation"]} | {r["component"]} | {r["median"]} | {r["low_noise_median"]} | {r["outlier_count"]} |')
 open(path,'w').write('\n'.join(lines)+'\n')
def main():
 ap=argparse.ArgumentParser(); ap.add_argument('--sample-raw'); ap.add_argument('--breakdown-raw'); ap.add_argument('--out-dir',required=True); a=ap.parse_args(); os.makedirs(a.out_dir,exist_ok=True)
 rows=read(a.sample_raw)+read(a.breakdown_raw); s=summarize(rows); fields=KEY+['n','valid_n','median','p10','p25','p75','p90','min','max','mean','stdev','cv','mad','iqr','outlier_count','low_noise_median','attempts_per_output_mean','rejections_per_output_mean','logical_bits_per_output_mean','physical_bits_per_output_mean','status']
 with open(os.path.join(a.out_dir,'frodo_summary.csv'),'w',newline='') as fp: w=csv.DictWriter(fp,fields); w.writeheader(); w.writerows(s)
 with open(os.path.join(a.out_dir,'frodo_summary.json'),'w') as fp: json.dump(s,fp,indent=2)
 markdown(s,os.path.join(a.out_dir,'frodo_summary.md'))
 print(os.path.join(a.out_dir,'frodo_summary.md'))
if __name__=='__main__': main()
