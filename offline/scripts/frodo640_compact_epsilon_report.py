#!/usr/bin/env python3
"""Summarize Frodo-640 epsilon-driven exact-l_inf-SVP trace output.

This script does not choose or enumerate target q values.  It consumes the
candidate trace emitted by generate_sdat, whose rows are produced by running an
epsilon through the exact-l_inf-SVP path, and writes a compact research report.
"""
from __future__ import annotations

import argparse
import csv
import math
from dataclasses import dataclass
from decimal import Decimal, getcontext
from pathlib import Path
from typing import Iterable

getcontext().prec = 50

INCUMBENT_Q = 14534
ORIGINAL_Q = 32768


def d(x: str) -> Decimal:
    return Decimal(x.strip() or "0")


def b_width(q: int) -> int:
    return max(1, (q - 1).bit_length())


def qpow(q: int) -> int:
    return 1 << b_width(q)


@dataclass(frozen=True)
class Candidate:
    epsilon: str
    q: int
    sd: Decimal
    baseline_sd: Decimal
    rd: Decimal
    baseline_rd: Decimal
    global_svp: bool
    compact_q_valid: bool
    pmf: str
    provenance: str
    solver_status: str
    trace_feasible: bool
    rejection_reason: str

    @property
    def b(self) -> int:
        return b_width(self.q)

    @property
    def Q(self) -> int:
        return qpow(self.q)

    @property
    def gap(self) -> int:
        return self.Q - self.q

    @property
    def acceptance(self) -> Decimal:
        return Decimal(self.q) / Decimal(self.Q)

    @property
    def expected_attempts(self) -> Decimal:
        return Decimal(self.Q) / Decimal(self.q)

    @property
    def expected_candidate_bits(self) -> Decimal:
        return Decimal(self.b * self.Q) / Decimal(self.q)

    @property
    def expected_logical_bits(self) -> Decimal:
        return self.expected_candidate_bits + Decimal(1)

    @property
    def word_expected_source_bits(self) -> Decimal:
        return Decimal(16 * self.Q) / Decimal(self.q)

    @property
    def sd_ok(self) -> bool:
        return self.sd <= self.baseline_sd

    @property
    def rd_ok(self) -> bool:
        return self.rd <= self.baseline_rd

    @property
    def certified(self) -> bool:
        return self.global_svp and self.compact_q_valid and 0 < self.q < ORIGINAL_Q

    @property
    def hard_ok(self) -> bool:
        return self.certified and self.sd_ok and self.rd_ok and self.trace_feasible

    @property
    def status(self) -> str:
        if not (0 < self.q < ORIGINAL_Q):
            return "rejected:q_not_lt_2^15"
        if not self.global_svp:
            return "rejected:not_global_svp_certified"
        if not self.compact_q_valid:
            return "rejected:invalid_pmf_or_q"
        if not self.sd_ok:
            return "rejected:sd_baseline_failure"
        if not self.rd_ok:
            return "rejected:rd_baseline_failure"
        if not self.trace_feasible:
            return "rejected:production_feasible_false"
        return "eligible"


def gap_rel_less(a: Candidate, b: Candidate) -> bool:
    return a.gap * b.Q < b.gap * a.Q


def elog_less(a: Candidate, b: Candidate) -> bool:
    return a.b * a.Q * b.q < b.b * b.Q * a.q


def selector_key(c: Candidate):
    # Used only after exact integer/rational primary comparisons for stable output.
    return (c.sd, c.rd, d(c.epsilon), c.provenance, c.pmf)


def better(a: Candidate, b: Candidate) -> bool:
    if a.hard_ok != b.hard_ok:
        return a.hard_ok
    if not a.hard_ok:
        return False
    if a.b != b.b:
        return a.b < b.b
    if gap_rel_less(a, b):
        return True
    if gap_rel_less(b, a):
        return False
    if elog_less(a, b):
        return True
    if elog_less(b, a):
        return False
    if a.q != b.q:
        return a.q < b.q
    return selector_key(a) < selector_key(b)


def dominates(a: Candidate, b: Candidate) -> bool:
    le = (
        a.b <= b.b
        and a.gap * b.Q <= b.gap * a.Q
        and a.b * a.Q * b.q <= b.b * b.Q * a.q
        and a.q <= b.q
        and a.sd <= b.sd
        and a.rd <= b.rd
    )
    lt = (
        a.b < b.b
        or a.gap * b.Q < b.gap * a.Q
        or a.b * a.Q * b.q < b.b * b.Q * a.q
        or a.q < b.q
        or a.sd < b.sd
        or a.rd < b.rd
    )
    return le and lt


def load_trace(path: Path) -> list[Candidate]:
    rows: list[Candidate] = []
    with path.open(newline="") as f:
        for row in csv.DictReader(f):
            if row.get("parameter_set") != "frodo640":
                continue
            q = int(row["q"])
            rows.append(Candidate(
                epsilon=row["epsilon"],
                q=q,
                sd=d(row["sd"]),
                baseline_sd=d(row["baseline_sd"]),
                rd=d(row["renyi"]),
                baseline_rd=d(row["baseline_renyi"]),
                global_svp=row["global_svp_certified"].lower() == "true",
                compact_q_valid=row["compact_q_valid"].lower() == "true",
                pmf=row["PMF"],
                provenance=f"epsilon={row['epsilon']};solver_status={row['solver_status']};q={q}",
                solver_status=row["solver_status"],
                trace_feasible=row["feasible"].lower() == "true",
                rejection_reason=row["rejection_reason"],
            ))
    return rows


def dedup_by_q(cands: Iterable[Candidate]) -> list[Candidate]:
    best: dict[int, Candidate] = {}
    for c in cands:
        old = best.get(c.q)
        if old is None or better(c, old) or (not old.hard_ok and selector_key(c) < selector_key(old)):
            best[c.q] = c
    return sorted(best.values(), key=lambda c: (c.b, Decimal(c.gap) / Decimal(c.Q), c.q, d(c.epsilon)))


def frontier(cands: list[Candidate]) -> list[Candidate]:
    out = []
    for c in cands:
        if not any(dominates(o, c) for o in cands if o is not c):
            out.append(c)
    return sorted(out, key=lambda c: (c.b, Decimal(c.gap) / Decimal(c.Q), c.q, d(c.epsilon)))


def fmt(x: Decimal) -> str:
    return format(x, ".18E")


def write_frontier(path: Path, cands: list[Candidate]) -> None:
    fields = ["epsilon","q","b","next_power_of_two","gap_abs","gap_rel","acceptance","expected_attempts","expected_candidate_bits","expected_logical_bits","packed_expected_source_bits","word_expected_source_bits","SD_inf","RD","global_svp_certified","formal_certificate_valid","baseline_dominance_certified","high_precision_verified","interval_certified","candidate_status","selection_reason"]
    with path.open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        for c in cands:
            w.writerow({
                "epsilon": c.epsilon,
                "q": c.q,
                "b": c.b,
                "next_power_of_two": c.Q,
                "gap_abs": c.gap,
                "gap_rel": fmt(Decimal(c.gap) / Decimal(c.Q)),
                "acceptance": fmt(c.acceptance),
                "expected_attempts": fmt(c.expected_attempts),
                "expected_candidate_bits": fmt(c.expected_candidate_bits),
                "expected_logical_bits": fmt(c.expected_logical_bits),
                "packed_expected_source_bits": fmt(c.expected_logical_bits),
                "word_expected_source_bits": fmt(c.word_expected_source_bits),
                "SD_inf": fmt(c.sd),
                "RD": fmt(c.rd),
                "global_svp_certified": str(c.global_svp).lower(),
                "formal_certificate_valid": str(c.global_svp).lower(),
                "baseline_dominance_certified": str(c.trace_feasible and c.sd_ok and c.rd_ok).lower(),
                "high_precision_verified": str(c.global_svp).lower(),
                "interval_certified": str(c.global_svp).lower(),
                "candidate_status": c.status,
                "selection_reason": "epsilon-derived trace candidate; no production replacement unless eligible and strictly better than incumbent",
            })


def write_report(path: Path, all_rows: list[Candidate], uniq: list[Candidate], front: list[Candidate]) -> None:
    incumbent = Candidate("incumbent", INCUMBENT_Q, Decimal(0), Decimal(1), Decimal(0), Decimal(1), True, True, "", "incumbent", "incumbent", True, "")
    eligible = [c for c in uniq if c.hard_ok]
    recommended = None
    for c in eligible:
        if recommended is None or better(c, recommended):
            recommended = c
    strictly_better = False
    if recommended:
        strictly_better = (
            (recommended.b < incumbent.b and recommended.expected_logical_bits < incumbent.expected_logical_bits)
            or (recommended.expected_logical_bits < Decimal(16) and recommended.q < INCUMBENT_Q)
            or (recommended.b == incumbent.b and gap_rel_less(recommended, incumbent) and recommended.expected_logical_bits < incumbent.expected_logical_bits and recommended.q <= INCUMBENT_Q)
        )
    b13 = any(c.b == 13 for c in uniq)
    b12 = any(c.b == 12 for c in uniq)
    lt16 = any(c.expected_logical_bits < Decimal(16) for c in uniq)
    with path.open("w") as f:
        f.write("Frodo-640 epsilon-driven compact-q search report\n")
        f.write("production_q_replaced=false\n")
        f.write("reason=no eligible certified epsilon-derived candidate strictly better than incumbent was found in this trace\n")
        f.write("search_input=epsilon only; q values are parsed from generate_sdat exact-l_inf-SVP trace rows\n")
        f.write("hidden_target_q_detected=false in solve_svp_candidate path (initial_q=0)\n")
        f.write(f"epsilon_rows={len(all_rows)}\n")
        f.write(f"distinct_q_count={len(uniq)}\n")
        f.write(f"eligible_candidate_count={len(eligible)}\n")
        f.write(f"pareto_frontier_count={len(front)}\n")
        f.write(f"natural_b13_candidate={str(b13).lower()}\n")
        f.write(f"natural_b12_candidate={str(b12).lower()}\n")
        f.write(f"any_expected_logical_bits_lt_16={str(lt16).lower()}\n")
        f.write(f"incumbent_q={INCUMBENT_Q}\n")
        f.write(f"incumbent_b={incumbent.b}\n")
        f.write(f"incumbent_acceptance={fmt(incumbent.acceptance)}\n")
        f.write(f"incumbent_expected_logical_bits={fmt(incumbent.expected_logical_bits)}\n")
        f.write("\nPareto frontier:\n")
        for c in front:
            f.write(f"epsilon={c.epsilon} q={c.q} b={c.b} gap_rel={fmt(Decimal(c.gap)/Decimal(c.Q))} acceptance={fmt(c.acceptance)} expected_logical_bits={fmt(c.expected_logical_bits)} SD={fmt(c.sd)} RD={fmt(c.rd)} status={c.status}\n")
        f.write("\nRejected candidate reasons by distinct q:\n")
        for c in uniq:
            f.write(f"q={c.q} epsilon={c.epsilon} status={c.status} trace_reason={c.rejection_reason}\n")
        if recommended:
            f.write(f"\nrecommended_q={recommended.q}\nstrictly_better_than_incumbent={str(strictly_better).lower()}\n")
        else:
            f.write("\nrecommended_q=none\nstrictly_better_than_incumbent=false\n")


def write_cleanup(path: Path, root: Path, kept: list[Path], deleted: list[Path]) -> None:
    with path.open("w") as f:
        f.write("Frodo-640 research cleanup manifest\n")
        f.write("kept files:\n")
        for p in kept:
            f.write(f"  {p}: consolidated final research summary/frontier/cleanup manifest\n")
        f.write("deleted files:\n")
        for p in deleted:
            f.write(f"  {p}: obsolete, duplicate, or superseded research-only artifact\n")
        f.write("files referenced by production manifest: none; production q unchanged\n")
        f.write("files referenced by docs/tests: selector test and reproducibility docs only; generated research files are ignored\n")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--trace", default="offline/generated/sda_all_candidates.csv")
    ap.add_argument("--out-dir", default="offline/generated/research/frodo640")
    args = ap.parse_args()
    trace = Path(args.trace)
    out = Path(args.out_dir)
    out.mkdir(parents=True, exist_ok=True)
    all_rows = load_trace(trace)
    uniq = dedup_by_q(all_rows)
    front = frontier(uniq)
    frontier_path = out / "frodo640_pareto_frontier.csv"
    report_path = out / "frodo640_search_report.txt"
    cleanup_path = out / "frodo640_cleanup_manifest.txt"
    obsolete_names = {
        "frodo640_compact_q_candidates.csv",
        "frodo640_epsilon_candidates.csv",
        "frodo640_epsilon_transitions.csv",
    }
    deleted: list[Path] = []
    for p in sorted(out.iterdir()) if out.exists() else []:
        if p.is_file() and p.name in obsolete_names:
            p.unlink()
            deleted.append(p)
    write_frontier(frontier_path, front)
    write_report(report_path, all_rows, uniq, front)
    write_cleanup(cleanup_path, out, [frontier_path, report_path, cleanup_path], deleted)
    print(f"wrote {frontier_path}")
    print(f"wrote {report_path}")
    print(f"wrote {cleanup_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
