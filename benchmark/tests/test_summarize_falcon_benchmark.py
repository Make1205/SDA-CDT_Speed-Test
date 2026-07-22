#!/usr/bin/env python3
import csv,json,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
SCRIPT=ROOT/'benchmark/scripts/summarize_falcon_benchmark.py'
SAMPLE=ROOT/'benchmark/tests/fixtures/falcon_base_sampler_small.csv'
BREAK=ROOT/'benchmark/tests/fixtures/falcon_breakdown_small.csv'
def rows(p):
 with open(p,newline='') as f:return list(csv.DictReader(f))
def find(rs,**kw):
 for r in rs:
  if all(r.get(k)==v for k,v in kw.items()):return r
 raise AssertionError(kw)
def close(a,b): assert abs(float(a)-b)<1e-6,(a,b)
def main():
 with tempfile.TemporaryDirectory() as td:
  subprocess.check_call([sys.executable,str(SCRIPT),'--sample-raw',str(SAMPLE),'--breakdown-raw',str(BREAK),'--out-dir',td])
  rs=rows(Path(td)/'falcon_summary.csv'); data=json.loads((Path(td)/'falcon_summary.json').read_text()); md=(Path(td)/'falcon_summary.md').read_text()
  assert (Path(td)/'falcon_summary.csv').exists() and (Path(td)/'falcon_summary.md').exists() and (Path(td)/'falcon_summary.json').exists()
  assert data['metadata']['rng_generation_status']=='not_in_benchmark'
  assert 'N/A (pre-generated PRNG source/state)' in md and 'Standalone component timings; not additive.' in md
  o=find(rs,parameter_set='base-gaussian0',sampler_kind='original-cdt',frontend='falcon-prng72',component='full-sampler-core',implementation='original-reference')
  s=find(rs,parameter_set='base-gaussian0',sampler_kind='sda-cdt',frontend='falcon-sda72',component='full-sampler-core',implementation='sda-reference')
  assert o['n']=='6' and o['valid_n']=='5' and o['error_count']=='1'
  close(o['pooled_median'],20); close(o['pooled_p10'],10.8); close(o['pooled_p90'],68.8); close(o['iqr'],10); assert o['outlier_count']=='1'; close(o['pooled_low_noise_median'],16)
  close(o['median_of_process_medians'],16.5); close(s['median_of_process_medians'],12)
  close(s['source_bytes_per_attempt_mean'],9); close(s['physical_bytes_per_output_mean'],10.35)
  assert find(rs,component='source-frontend',implementation='benchmark-only-original-source-frontend')['sampler_kind']=='original-cdt'
  assert find(rs,component='source-frontend',implementation='benchmark-only-sda-source-frontend')['sampler_kind']=='sda-cdt'
  assert 'SDA faster' in md and '| equal-size | base-gaussian0 |' in md
  print('falcon summary fixture tests passed')
if __name__=='__main__':main()
