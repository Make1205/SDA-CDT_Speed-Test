#!/usr/bin/python3
import csv, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'generated'/'research'/'falcon'
TWO72=1<<72

def assert_true(x, msg):
    if not x:
        raise SystemExit(msg)

if not (OUT/'falcon_sdat_selected.csv').exists():
    print('falcon_offline_pipeline_skipped: generated/research/falcon artifacts absent')
    raise SystemExit(0)

rows=list(csv.DictReader((OUT/'falcon_sdat_selected.csv').open()))
assert_true(len(rows)==19, 'selected table length')
masses=[int(r['mass_decimal']) for r in rows]
cums=[int(r['cumulative_decimal']) for r in rows]
assert_true(min(masses)>=0, 'negative mass')
assert_true(cums[-1]==sum(masses), 'sum mismatch')
assert_true(0<cums[-1]<TWO72, 'q boundary')
h=(OUT/'falcon_sdat_selected.h').read_text()
assert_true('FALCON_SDAT_HEURISTIC_BKZ 1' in h, 'heuristic flag')
assert_true('FALCON_SDAT_EXACT_SVP 0' in h, 'exact flag')
assert_true('UINT64_C' in h and 'falcon_sdat_c_high8' in h, 'limb arrays')
run_rows=list(csv.DictReader((OUT/'falcon_bkz_runs.csv').open()))
assert_true(bool(run_rows), 'no runs')
blocks={int(r['block_size']) for r in run_rows}
assert_true({10,15,20}.issubset(blocks), 'missing block size')
assert_true(any(int(r['exit_code'])==0 and int(r['block_size'])==20 for r in run_rows), 'no BKZ-20 success')
assert_true(all('fplll -a bkz' in r['command_line'] for r in run_rows), 'bad command')
res=subprocess.run(['/usr/bin/python3', str(ROOT/'offline'/'scripts'/'verify_falcon_sdat.py')], cwd=ROOT, text=True, stdout=subprocess.PIPE)
assert_true(res.returncode==0, res.stdout)
print('falcon_offline_pipeline_ok')
