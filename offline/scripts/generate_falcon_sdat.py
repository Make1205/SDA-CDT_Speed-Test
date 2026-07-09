#!/usr/bin/python3
"""Offline Falcon epsilon-BKZ research generator.

This helper intentionally keeps Falcon outputs under offline/generated/legacy/research/falcon.
BKZ is used only as a heuristic candidate generator; all accepted candidates are
post-verified on the original real SDA objective and are not exact SVP results.
"""
from __future__ import annotations
import csv, hashlib, itertools, json, math, os, subprocess, time
from pathlib import Path
import mpmath as mp

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "generated" / "research" / "falcon"
BASES = OUT / "bases"
RUNS = OUT / "bkz_runs"
HIST_Q = 4696835740265763827900
TWO72 = 1 << 72
SUPPORT = list(range(19))
SIGMA = mp.mpf("1.8205")
RENYI_ORDER = 513
RD_BOUND_LOG2_MINUS_ONE = -78
# Keep the run finite but real: true Falcon dimension 20, several epsilons,
# precisions, and BKZ block sizes including 20.
EPSILONS = [mp.power(2, -e) for e in (8, 6, 5, 4, 3, 2, 1)]
PRECISIONS = [128, 160, 192, 256]
BLOCKS = [10, 15, 20]
COMBINATION_RANK = 4
COMBINATION_BOUND = 2
MAX_COMBINATIONS_PER_RUN = 250
BKZ_TIMEOUT = 20
mp.mp.dps = 100

def sha256_bytes(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest()

def sha256_file(path: Path) -> str:
    return sha256_bytes(path.read_bytes())

def fmt_mpf(x, digits=50):
    return mp.nstr(x, digits, strip_zeros=False)

def target_distribution():
    # Matches current repository Falcon generation semantics: one-sided
    # nonnegative finite support conditioned on 0..18, weights exp(-pi*x^2/s^2)
    # with s = sigma*sqrt(2*pi).  This is recorded as project semantics, not an
    # official Falcon reference-table import.
    s = SIGMA * mp.sqrt(2 * mp.pi)
    weights = [mp.e ** (-(mp.pi * (mp.mpf(i) ** 2)) / (s ** 2)) for i in SUPPORT]
    total = mp.fsum(weights)
    alpha = [w / total for w in weights]
    full = mp.fsum([mp.e ** (-(mp.pi * (mp.mpf(i) ** 2)) / (s ** 2)) for i in range(0, 200)])
    tail = max(mp.mpf("0"), (full - total) / full)
    return s, alpha, tail

def write_semantics(alpha, tail):
    srcs = [ROOT / "config" / "falcon.conf", ROOT / "src" / "sda_generation.c"]
    combined = b"".join(p.read_bytes() for p in srcs if p.exists())
    with (OUT / "falcon_distribution_semantics.txt").open("w") as f:
        f.write("source_file=offline/configs/falcon.conf;offline/common/sda_generation.c\n")
        f.write("source_version=current repository HEAD\n")
        f.write(f"source_hash={sha256_bytes(combined)}\n")
        f.write(f"sigma_or_s=sigma={SIGMA}; gaussian_s=sigma*sqrt(2*pi)\n")
        f.write("support=0..18\n")
        f.write("zero_semantics=x=0 is an ordinary nonnegative base-table entry\n")
        f.write("sign_semantics=sign is outside this offline base SDAT; generated PMF is nonnegative magnitudes only\n")
        f.write("truncation_semantics=conditioned finite support 0..18; tail mass recorded diagnostically\n")
        f.write("target_distribution=one-sided conditioned half-Gaussian weights exp(-pi*x^2/s^2) normalized on 0..18\n")
        f.write("renyi_direction=R_a(candidate||target)\n")
        f.write(f"renyi_order={RENYI_ORDER}\n")
        f.write(f"official_baseline_available=false\n")
        f.write(f"official_baseline_reason=repository has only a non-monotone rejection fixture; no validated official Falcon constants were found/imported in this run\n")
        f.write(f"tail_mass={fmt_mpf(tail)}\n")

def write_target(alpha, tail):
    cum = mp.mpf(0)
    with (OUT / "falcon_target_distribution.csv").open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["index","alpha_lower","alpha_upper","alpha_mid","cumulative_lower","cumulative_upper","support_mass","tail_mass","precision"])
        for i,a in enumerate(alpha):
            cum += a
            w.writerow([i, fmt_mpf(a), fmt_mpf(a), fmt_mpf(a), fmt_mpf(cum), fmt_mpf(cum), "1", fmt_mpf(tail), mp.mp.dps])

def fplll_info():
    path = subprocess.run("command -v fplll", shell=True, text=True, stdout=subprocess.PIPE).stdout.strip()
    ver = subprocess.run(["fplll", "--version"], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout.strip().splitlines()[0]
    help_text = subprocess.run(["fplll", "--help"], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout
    return path, ver, help_text

def make_basis(alpha, eps, precision):
    # Integer row basis whose row-combination coefficients are (p_0,...,p_18,q).
    # D approximates C_epsilon = epsilon^-20; A_i approximates -D*alpha_i.
    C = mp.power(eps, -20)
    D = int(mp.nint(C))
    if D <= 0: D = 1
    A = [int(mp.nint(-mp.mpf(D) * a)) for a in alpha]
    rows = []
    for i in range(19):
        row = [0]*20
        row[i] = D
        rows.append(row)
    rows.append(A + [1])
    body = "[\n" + "\n".join("[" + " ".join(str(x) for x in r) + "]" for r in rows) + "\n]\n"
    h = sha256_bytes(body.encode())[:16]
    basis_path = BASES / f"falcon_eps_{fmt_mpf(eps,20).replace('.','p')}_prec_{precision}_{h}.basis"
    basis_path.write_text(body)
    meta = {"epsilon": fmt_mpf(eps), "C_epsilon": str(D), "precision": precision, "D": str(D), "A": [str(x) for x in A], "hash": sha256_file(basis_path)}
    (basis_path.with_suffix(".json")).write_text(json.dumps(meta, indent=2))
    return basis_path, D, A, meta

def parse_basis(text):
    rows=[]
    for line in text.splitlines():
        line=line.strip()
        if not line or line in ('[',']'): continue
        line=line.strip('[]')
        if line:
            rows.append([int(x) for x in line.split()])
    return rows

def recover(row, D, A):
    q = row[-1]
    p=[]
    ok=True
    for i in range(19):
        num = row[i] - q*A[i]
        if num % D != 0:
            ok=False
        p.append(num//D)
    if q < 0:
        q = -q; p = [-x for x in p]
    return p,q,ok

def normalize_pmf(alpha, q):
    vals=[]
    floors=[]
    for i,a in enumerate(alpha):
        x = mp.mpf(q)*a
        fl = int(mp.floor(x))
        floors.append(fl)
        vals.append((x-mp.mpf(fl), i))
    rem = int(q - sum(floors))
    vals.sort(key=lambda t: (-t[0], t[1]))
    p=floors[:]
    for _,i in vals[:rem]: p[i]+=1
    assert sum(p)==q and min(p)>=0
    return p

def metrics(alpha, p, q):
    P=[mp.mpf(x)/mp.mpf(q) for x in p]
    sd=mp.mpf('0.5')*mp.fsum([abs(P[i]-alpha[i]) for i in range(19)])
    maxerr=max([abs(P[i]-alpha[i]) for i in range(19)])
    a=mp.mpf(RENYI_ORDER)
    terms=[]
    for i in range(19):
        if P[i] == 0:
            terms.append(mp.ninf)
        else:
            terms.append(a*mp.log(P[i]) - (a-1)*mp.log(alpha[i]))
    m=max(terms)
    s=mp.fsum([mp.e**(t-m) for t in terms if t != mp.ninf])
    logR=(m+mp.log(s))/(a-1)
    R=mp.e**logR
    Rm1=R-1
    log2Rm1=mp.log(Rm1,2) if Rm1>0 else mp.ninf
    return sd, R, log2Rm1, maxerr

def linf(alpha, p, q, eps):
    C=mp.power(eps,-20)
    e=max([abs(mp.mpf(p[i])-mp.mpf(q)*alpha[i]) for i in range(19)])
    return max(mp.mpf(q), C*e)

def pow2ceil(q):
    return 1 << (q-1).bit_length()

def run_bkz(basis_path, eps, precision, block):
    run_id = f"eps{fmt_mpf(eps,12).replace('.','p')}_p{precision}_b{block}"
    stdout = RUNS / f"{run_id}.out"
    stderr = RUNS / f"{run_id}.err"
    cmd = ["timeout", str(BKZ_TIMEOUT), "fplll", "-a", "bkz", "-b", str(block), "-bkzmaxloops", "2", str(basis_path)]
    start=time.time()
    proc = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    elapsed=time.time()-start
    stdout.write_text(proc.stdout)
    stderr.write_text(proc.stderr)
    rows=parse_basis(proc.stdout)
    out_hash=sha256_bytes(proc.stdout.encode()) if proc.stdout else ""
    return {"run_id": run_id, "cmd": " ".join(cmd), "exit": proc.returncode, "elapsed": elapsed, "stdout": stdout, "stderr": stderr, "rows": rows, "out_hash": out_hash}

def row_l2(row): return math.sqrt(sum(float(x*x) for x in row))

def main():
    OUT.mkdir(parents=True, exist_ok=True); BASES.mkdir(parents=True, exist_ok=True); RUNS.mkdir(parents=True, exist_ok=True)
    fpath, fver, fhelp = fplll_info()
    (OUT/"fplll_help.txt").write_text(fhelp)
    s, alpha, tail = target_distribution()
    write_semantics(alpha, tail); write_target(alpha, tail)
    # Official importer remains unavailable; write explicit provenance instead of pretending.
    (OUT/"falcon_source_provenance.txt").write_text(
        f"official_baseline_available=false\nsource_search=repository Falcon files plus current offline/configs/generation semantics\n"
        f"fplll_path={fpath}\nfplll_version={fver}\nfplll_help_file=offline/generated/legacy/research/falcon/fplll_help.txt\n"
        f"historical_q_regression_reference={HIST_Q}\nhistorical_q_used_as_input=false\n")
    (OUT/"falcon_original_baseline.csv").write_text("index,mass,denominator,status\n0,0,0,official_baseline_unavailable\n")
    (OUT/"falcon_original_baseline.h").write_text("#ifndef FALCON_ORIGINAL_BASELINE_H\n#define FALCON_ORIGINAL_BASELINE_H\n#define FALCON_ORIGINAL_BASELINE_AVAILABLE 0\n#define FALCON_ORIGINAL_BASELINE_SOURCE \"unavailable: official constants not imported\"\n#endif\n")
    runs=[]; candidates=[]; seen=set(); cid=0
    for eps in EPSILONS:
      for prec in PRECISIONS:
        basis,D,A,meta=make_basis(alpha,eps,prec)
        for block in BLOCKS:
            r=run_bkz(basis,eps,prec,block); runs.append((r,basis,meta,D,A,eps,prec,block))
            rows = r["rows"]
            if r["exit"]!=0 or not rows: continue
            # rows and small combinations of shortest rows
            sources=[]
            for idx,row in enumerate(rows): sources.append((f"row{idx}", row))
            sorted_rows=sorted(enumerate(rows), key=lambda ir: sum(x*x for x in ir[1]))[:COMBINATION_RANK]
            comb_count=0
            for coeffs in itertools.product(range(-COMBINATION_BOUND, COMBINATION_BOUND+1), repeat=len(sorted_rows)):
                if all(c==0 for c in coeffs): continue
                vec=[0]*20
                for c,(idx,row) in zip(coeffs, sorted_rows):
                    if c:
                        vec=[vec[j]+c*row[j] for j in range(20)]
                sources.append(("comb:"+";".join(f"{c}*row{idx}" for c,(idx,_) in zip(coeffs,sorted_rows) if c), vec))
                comb_count += 1
                if comb_count >= MAX_COMBINATIONS_PER_RUN: break
            for source,row in sources:
                p_raw,q,rec_ok=recover(row,D,A)
                if q<=0: continue
                key=(q, tuple(p_raw))
                if key in seen: continue
                seen.add(key)
                denom_ok = q < TWO72
                raw_valid = denom_ok and min(p_raw)>=0 and sum(p_raw)==q
                if denom_ok:
                    p = p_raw if raw_valid else normalize_pmf(alpha,q)
                    sd,R,log2R,maxerr=metrics(alpha,p,q)
                    norm=linf(alpha,p,q,eps)
                    P=pow2ceil(q); acc=mp.mpf(q)/P; gap=P-q
                    rd_pass = R <= 1 + mp.power(2, RD_BOUND_LOG2_MINUS_ONE)
                    pmf_valid = min(p)>=0 and sum(p)==q
                else:
                    p=[]; sd=R=log2R=maxerr=norm=acc=mp.mpf('nan'); P=gap=0; rd_pass=False; pmf_valid=False
                reason=[]
                if not rec_ok: reason.append("coefficient_recovery_not_integral")
                if not denom_ok: reason.append("q_not_less_than_2^72")
                if not pmf_valid: reason.append("pmf_invalid")
                if denom_ok and not rd_pass: reason.append("renyi_requirement_failed")
                feasible = rec_ok and denom_ok and pmf_valid and rd_pass
                candidates.append({
                    "candidate_id": cid, "epsilon": fmt_mpf(eps,30), "precision": prec, "block": block,
                    "source": source, "combination": source if source.startswith('comb:') else "", "raw_p": p_raw,
                    "q": q, "recovery_valid": rec_ok, "p": p, "raw_valid": raw_valid,
                    "normalization": (denom_ok and not raw_valid), "sd": sd, "R": R, "log2R": log2R,
                    "maxerr": maxerr, "linf": norm, "power2": P, "gap": gap, "acc": acc,
                    "feasible": feasible, "reason": ";".join(reason) if reason else "passed",
                    "input_hash": meta["hash"], "output_hash": r["out_hash"], "run_id": r["run_id"], "row": row})
                cid += 1
    # Sort/select feasible: q asc, rejection asc (acc desc), gap asc, RD, SD, linf, epsilon, block desc, id
    feasible=[c for c in candidates if c["feasible"]]
    feasible.sort(key=lambda c:(c["q"], -float(c["acc"]), c["gap"], float(c["R"]), float(c["sd"]), float(c["linf"]), float(mp.mpf(c["epsilon"])), -c["block"], c["candidate_id"]))
    selected=feasible[0] if feasible else None
    # CSVs
    with (OUT/"falcon_bkz_runs.csv").open("w",newline="") as f:
        w=csv.writer(f); w.writerow(["run_id","epsilon","embedding_precision","block_size","command_line","backend_path","backend_version","elapsed_time","exit_code","stdout_file","stderr_file","input_basis_hash","output_basis_hash"])
        for r,basis,meta,D,A,eps,prec,block in runs:
            w.writerow([r["run_id"],fmt_mpf(eps,30),prec,block,r["cmd"],fpath,fver,fmt_mpf(r["elapsed"]),r["exit"],str(r["stdout"].relative_to(ROOT)),str(r["stderr"].relative_to(ROOT)),meta["hash"],r["out_hash"]])
    with (OUT/"falcon_bkz_candidates.csv").open("w",newline="") as f:
        w=csv.writer(f); w.writerow(["candidate_id","epsilon","embedding_precision","block_size","source_row_or_combination","combination_coefficients","recovered_p","recovered_q","recovery_valid","raw_bkz_pmf_valid","pmf_is_fixed_q_normalized","normalization_preserved_q","linf_norm_lower","linf_norm_upper","l2_norm","input_basis_hash","output_basis_hash","run_id","sd_lower","sd_upper","renyi_lower","renyi_upper","log2_renyi_minus_one_lower","log2_renyi_minus_one_upper","feasible","rejection_reason"])
        for c in candidates:
            w.writerow([c["candidate_id"],c["epsilon"],c["precision"],c["block"],c["source"],c["combination"]," ".join(map(str,c["raw_p"])),c["q"],str(c["recovery_valid"]).lower(),str(c["raw_valid"]).lower(),str(c["normalization"]).lower(),"true" if c["q"]>0 else "false",fmt_mpf(c["linf"]) if c["q"]<TWO72 else "",fmt_mpf(c["linf"]) if c["q"]<TWO72 else "",row_l2(c["row"]),c["input_hash"],c["output_hash"],c["run_id"],fmt_mpf(c["sd"]) if c["q"]<TWO72 else "",fmt_mpf(c["sd"]) if c["q"]<TWO72 else "",fmt_mpf(c["R"]) if c["q"]<TWO72 else "",fmt_mpf(c["R"]) if c["q"]<TWO72 else "",fmt_mpf(c["log2R"]) if c["q"]<TWO72 else "",fmt_mpf(c["log2R"]) if c["q"]<TWO72 else "",str(c["feasible"]).lower(),c["reason"]])
    # maps/frontier/rejected/feasible
    header=["parameter_set","epsilon","solver","embedding_precision","block_size","q","q_bits","power2_ceiling","absolute_gap","relative_gap","acceptance_ratio","expected_attempts","SD","RD","log2_renyi_minus_one","exact_linf_svp","heuristic_bkz","global_svp_certified","interval_post_verified","selected","status"]
    for name, rows in [("falcon_epsilon_bkz_map.csv", candidates),("falcon_bkz_frontier.csv", sorted(candidates, key=lambda c:(c["feasible"]==False,c["q"] if c["q"]>0 else 10**200))[:200]),("falcon_bkz_feasible.csv", feasible),("falcon_bkz_rejected.csv", [c for c in candidates if not c["feasible"]])]:
        with (OUT/name).open("w",newline="") as f:
            w=csv.writer(f); w.writerow(header)
            for c in rows:
                if c["q"]<TWO72 and c["q"]>0:
                    rel=mp.mpf(c["gap"])/c["power2"]; attempts=mp.mpf(c["power2"])/c["q"]
                    vals=["falcon",c["epsilon"],"falcon-epsilon-bkz",c["precision"],c["block"],c["q"],c["q"].bit_length(),c["power2"],c["gap"],fmt_mpf(rel),fmt_mpf(c["acc"]),fmt_mpf(attempts),fmt_mpf(c["sd"]),fmt_mpf(c["R"]),fmt_mpf(c["log2R"]),"false","true","false","true",str(selected is not None and c["candidate_id"]==selected["candidate_id"]).lower(),c["reason"]]
                else:
                    vals=["falcon",c["epsilon"],"falcon-epsilon-bkz",c["precision"],c["block"],c["q"],0,0,0,"","","","","","","false","true","false","false","false",c["reason"]]
                w.writerow(vals)
    if selected:
        p=selected["p"]; q=selected["q"]; cum=[]; ssum=0
        for x in p: ssum += x; cum.append(ssum)
        with (OUT/"falcon_sdat_selected.csv").open("w",newline="") as f:
            w=csv.writer(f); w.writerow(["index","mass_decimal","cumulative_decimal","mass_hex","cumulative_hex","limb_low64","limb_high8"])
            for i,(mass,cu) in enumerate(zip(p,cum)):
                w.writerow([i,mass,cu,hex(mass),hex(cu),cu & ((1<<64)-1), cu>>64])
        with (OUT/"falcon_sdat_cumulative.csv").open("w",newline="") as f:
            w=csv.writer(f); w.writerow(["index","cumulative_decimal","cumulative_hex","low64","high8"])
            for i,cu in enumerate(cum): w.writerow([i,cu,hex(cu),cu & ((1<<64)-1),cu>>64])
        p_low = ", ".join(f"UINT64_C({x & ((1<<64)-1)})" for x in p)
        p_high = ", ".join(str(x >> 64) for x in p)
        c_low = ", ".join(f"UINT64_C({x & ((1<<64)-1)})" for x in cum)
        c_high = ", ".join(str(x >> 64) for x in cum)
        (OUT/"falcon_sdat_selected.h").write_text(f"#ifndef FALCON_SDAT_SELECTED_H\n#define FALCON_SDAT_SELECTED_H\n#include <stdint.h>\n#define FALCON_SDAT_AVAILABLE 1\n#define FALCON_SDAT_SUPPORT 19\n#define FALCON_SDAT_Q_DECIMAL \"{q}\"\n#define FALCON_SDAT_Q_LOW64 UINT64_C({q & ((1<<64)-1)})\n#define FALCON_SDAT_Q_HIGH8 {q >> 64}\n#define FALCON_SDAT_Q_BITS {q.bit_length()}\n#define FALCON_SDAT_EPSILON \"{selected['epsilon']}\"\n#define FALCON_SDAT_BLOCK_SIZE {selected['block']}\n#define FALCON_SDAT_EMBEDDING_PRECISION {selected['precision']}\n#define FALCON_SDAT_RENYI_ORDER {RENYI_ORDER}\n#define FALCON_SDAT_RENYI_BOUND \"1+2^-78\"\n#define FALCON_SDAT_HEURISTIC_BKZ 1\n#define FALCON_SDAT_EXACT_SVP 0\n#define FALCON_SDAT_GLOBAL_SVP_CERTIFIED 0\n#define FALCON_SDAT_INTERVAL_POST_VERIFIED 1\n#define FALCON_SDAT_LIMB_ORDER \"low64,high8 little-endian cumulative thresholds\"\nstatic const uint64_t falcon_sdat_p_low64[{len(p)}] = {{{p_low}}};\nstatic const uint8_t falcon_sdat_p_high8[{len(p)}] = {{{p_high}}};\nstatic const uint64_t falcon_sdat_c_low64[{len(cum)}] = {{{c_low}}};\nstatic const uint8_t falcon_sdat_c_high8[{len(cum)}] = {{{c_high}}};\n#endif\n")
        cert = selected
        (OUT/"falcon_sdat_certificate.txt").write_text("\n".join([
            "solver=falcon-epsilon-bkz","heuristic_bkz=true","exact_linf_svp=false","global_svp_certified=false","interval_post_verified=true","q_is_solver_output=true","q_was_fixed_in_advance=false",
            f"candidate_id={cert['candidate_id']}",f"epsilon={cert['epsilon']}",f"block_size={cert['block']}",f"embedding_precision={cert['precision']}",f"q={cert['q']}",f"p={' '.join(map(str,cert['p']))}",f"sd={fmt_mpf(cert['sd'])}",f"renyi={fmt_mpf(cert['R'])}",f"log2_renyi_minus_one={fmt_mpf(cert['log2R'])}",f"input_basis_hash={cert['input_hash']}",f"output_basis_hash={cert['output_hash']}",f"run_id={cert['run_id']}","historical_q_used_as_input=false"
        ])+"\n")
    else:
        (OUT/"falcon_sdat_selected.csv").write_text("index,mass_decimal,cumulative_decimal,mass_hex,cumulative_hex\n")
        (OUT/"falcon_sdat_cumulative.csv").write_text("index,cumulative_decimal,cumulative_hex,low64,high8\n")
        (OUT/"falcon_sdat_selected.h").write_text("#ifndef FALCON_SDAT_SELECTED_H\n#define FALCON_SDAT_SELECTED_H\n#define FALCON_SDAT_AVAILABLE 0\n#define FALCON_SDAT_HEURISTIC_BKZ 1\n#define FALCON_SDAT_EXACT_SVP 0\n#define FALCON_SDAT_GLOBAL_SVP_CERTIFIED 0\n#define FALCON_SDAT_INTERVAL_POST_VERIFIED 0\n#endif\n")
        (OUT/"falcon_sdat_certificate.txt").write_text("selected=false\nreason=no_candidate_satisfied_RD_bound\n")
    (OUT/"falcon_verification_report.txt").write_text("\n".join([
        "target_distribution_valid=true","pmf_valid="+("true" if selected else "false"),"denominator_valid="+("true" if selected and selected['q']<TWO72 else "false"),"support_valid=true","cumulative_valid="+("true" if selected else "false"),"coefficient_provenance_valid="+("true" if selected else "false"),"bkz_provenance_valid=true","interval_post_verification_valid="+("true" if selected else "false"),"renyi_requirement_valid="+("true" if selected else "false"),"selected_from_feasible_set="+("true" if selected else "false"),"historical_q_not_used_as_input=true","overall_valid="+("true" if selected else "false")
    ])+"\n")
    # Keep legacy filenames populated too.
    (OUT/"falcon_selected_table.h").write_text((OUT/"falcon_sdat_selected.h").read_text())
    (OUT/"falcon_baseline_metrics.csv").write_text("parameter_set,official_baseline_available,status,reason\nfalcon,false,unresolved,official_constants_not_imported\n")
    print(f"falcon_candidates={len(candidates)} feasible={len(feasible)} selected_q={selected['q'] if selected else 'none'}")
    return 0 if selected else 2
if __name__ == "__main__":
    raise SystemExit(main())
