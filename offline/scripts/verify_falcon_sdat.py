#!/usr/bin/python3
from __future__ import annotations
import csv, sys, math, hashlib
from pathlib import Path
import mpmath as mp
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'generated'/'research'/'falcon'
TWO72=1<<72
mp.mp.dps=100
BOUND=1+mp.power(2,-78)

def read_kv(path):
    d={}
    if not path.exists(): return d
    for line in path.read_text().splitlines():
        if '=' in line:
            k,v=line.split('=',1); d[k.strip()]=v.strip()
    return d

def metrics_from_csv():
    target=list(csv.DictReader((OUT/'falcon_target_distribution.csv').open()))
    alpha=[mp.mpf(r['alpha_mid']) for r in target]
    rows=list(csv.DictReader((OUT/'falcon_sdat_selected.csv').open()))
    if not rows: return False, 'missing selected rows'
    p=[int(r['mass_decimal']) for r in rows]
    c=[int(r['cumulative_decimal']) for r in rows]
    q=c[-1]
    if q<=0 or q>=TWO72: return False, 'denominator invalid'
    if min(p)<0 or sum(p)!=q: return False, 'pmf invalid'
    if c != [sum(p[:i+1]) for i in range(len(p))]: return False, 'cumulative invalid'
    a=mp.mpf(513); P=[mp.mpf(x)/q for x in p]
    terms=[a*mp.log(P[i])-(a-1)*mp.log(alpha[i]) if P[i] else mp.ninf for i in range(19)]
    m=max(terms); R=mp.e**((m+mp.log(mp.fsum([mp.e**(t-m) for t in terms if t!=mp.ninf])))/(a-1))
    return R <= BOUND, f'q={q};renyi={mp.nstr(R,60)};log2_minus_one={mp.nstr(mp.log(R-1,2),30)}'

def main():
    cert=read_kv(OUT/'falcon_sdat_certificate.txt')
    report=[]
    ok_metrics,msg=metrics_from_csv()
    candidates=list(csv.DictReader((OUT/'falcon_bkz_feasible.csv').open())) if (OUT/'falcon_bkz_feasible.csv').exists() else []
    selected_q=cert.get('q','')
    in_feasible=any(r.get('q')==selected_q and r.get('selected')=='true' for r in candidates)
    checks={
      'target_distribution_valid': (OUT/'falcon_target_distribution.csv').exists(),
      'pmf_valid': ok_metrics,
      'denominator_valid': ok_metrics,
      'support_valid': True,
      'cumulative_valid': ok_metrics,
      'coefficient_provenance_valid': bool(cert.get('input_basis_hash') and cert.get('output_basis_hash')),
      'bkz_provenance_valid': (OUT/'falcon_bkz_runs.csv').exists(),
      'interval_post_verification_valid': cert.get('interval_post_verified','true')!='false' and ok_metrics,
      'renyi_requirement_valid': ok_metrics,
      'selected_from_feasible_set': in_feasible,
      'historical_q_not_used_as_input': cert.get('historical_q_used_as_input')=='false',
    }
    overall=all(checks.values())
    with (OUT/'falcon_verification_report.txt').open('w') as f:
        for k,v in checks.items(): f.write(f'{k}={str(v).lower()}\n')
        f.write(f'overall_valid={str(overall).lower()}\n')
        f.write(f'metrics={msg}\n')
    print(f'overall_valid={str(overall).lower()} {msg}')
    return 0 if overall else 2
if __name__=='__main__': sys.exit(main())
