#!/usr/bin/env python3
import csv,json,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
SCRIPT=ROOT/'benchmark/scripts/summarize_frodo_benchmark.py'
SAMPLE=ROOT/'benchmark/tests/fixtures/frodo_sample_n_small.csv'
BREAK=ROOT/'benchmark/tests/fixtures/frodo_breakdown_small.csv'
def load_csv(p):
 with open(p,newline='') as f:return list(csv.DictReader(f))
def find(rows,**kw):
 for r in rows:
  if all(r.get(k)==v for k,v in kw.items()):return r
 raise AssertionError(f'missing row {kw}')
def close(a,b):
 assert abs(float(a)-b)<1e-6,(a,b)
def main():
 with tempfile.TemporaryDirectory() as td:
  subprocess.check_call([sys.executable,str(SCRIPT),'--sample-raw',str(SAMPLE),'--breakdown-raw',str(BREAK),'--out-dir',td])
  rows=load_csv(Path(td)/'frodo_summary.csv')
  data=json.loads((Path(td)/'frodo_summary.json').read_text())
  md=(Path(td)/'frodo_summary.md').read_text()
  assert (Path(td)/'frodo_summary.csv').exists() and (Path(td)/'frodo_summary.json').exists() and (Path(td)/'frodo_summary.md').exists()
  assert data['metadata']['rng_generation_status']=='not_in_benchmark'
  assert 'N/A (pre-generated source)' in md
  assert 'Standalone component timings; not additive.' in md
  orig=find(rows,parameter_set='frodo640',sampler_kind='original-cdt',backend='reference',frontend='original-word',component='full-sampler-core',mode='equal-size',implementation='original-reference')
  sda=find(rows,parameter_set='frodo640',sampler_kind='sda-cdt',backend='reference',frontend='word-oriented',component='full-sampler-core',mode='equal-size',implementation='sda-word-reference')
  native=find(rows,parameter_set='frodo640',component='full-sampler-core',mode='native-batch',implementation='original-reference')
  assert orig['valid_n']=='5' and orig['n']=='6' and orig['error_count']=='1'
  close(orig['pooled_median'],20.0); close(orig['pooled_p10'],10.8); close(orig['pooled_p90'],68.8)
  close(orig['pooled_p25'],12.0); close(orig['pooled_p75'],22.0); close(orig['iqr'],10.0)
  close(orig['mad'],8.0); assert orig['outlier_count']=='1'; close(orig['pooled_low_noise_median'],16.0)
  assert orig['process_count']=='2'; close(orig['median_of_process_medians'],16.5); close(orig['min_process_median'],12.0); close(orig['max_process_median'],21.0)
  close(sda['median_of_process_medians'],12.0)
  close(native['median_of_process_medians'],5.0)
  assert find(rows,component='source-frontend',implementation='benchmark-only-original-source-frontend')['sampler_kind']=='original-cdt'
  assert find(rows,component='source-frontend',implementation='benchmark-only-sda-source-frontend')['sampler_kind']=='sda-cdt'
  assert '| equal-size | frodo640 |' in md and 'SDA faster' in md
  print('frodo summary fixture tests passed')
if __name__=='__main__':main()
