# Offline phase

The offline phase builds target distributions, searches denominators/candidates, verifies metrics, and exports reviewed online tables.

Minimal example:

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release
cmake --build build-offline --target generate_sdat verify_sdat -j
./build-offline/generate_sdat --config offline/configs/frodo640.conf --solver exact-denominator --reproducible
./build-offline/verify_sdat --all
```

Frodo exact paths include exact-denominator search and SDA-specialized exact `l_infinity` SVP/certificate code in `offline/common/sda_exact_linf*.c`. Falcon research generation in `offline/scripts/generate_falcon_sdat.py` is heuristic epsilon-BKZ/`l_2` work and is kept separate from exact `l_infinity` SVP claims.

All regenerated outputs should be written below `offline/generated/`.


### Frodo-640 compact-q selector audit

The Frodo-640 compact-q audit keeps the production table unchanged unless an epsilon-driven, exact-ℓ∞-SVP, baseline-dominating candidate is regenerated and certified. The historical production `q=14534` has `b=14`, `Q=16384`, `gap_abs=1850`, `gap_rel=1850/16384`, acceptance `14534/16384`, and expected packed logical bits `14*16384/14534 + 1`. The existing offline selector prioritizes production-eligible smaller `q` before absolute power-of-two gap and does not primarily optimize relative gap or expected logical bits; the new compact selector test codifies the requested ordering with feasibility first, then relative gap by integer cross multiplication, expected logical bits by rational comparison, smaller `q`, then SD/RD only as final tie-breakers.

In environments without MPFR/GMP offline dependencies, compact-q research reports under `offline/generated/research/frodo640/` are diagnostic only and are not committed. Frodo-976 and Frodo-1344 tables, q values, PMFs, CDFs, and certificates must remain unchanged during this Frodo-640-only audit.

### Frodo-640 epsilon-driven compact-q search harness

The compact-q search input is epsilon only: `generate_sdat` builds the epsilon-dependent SDA lattice, calls the exact `l_infinity` SDA solver, and records q values that come from the certified shortest-vector result. The reporting helper `offline/scripts/frodo640_compact_epsilon_report.py` only summarizes the emitted epsilon trace; it does not enumerate q, pass a target q, or round to an arbitrary denominator. Selector tests reject candidates without epsilon provenance or exact-SVP-shortest-vector provenance, prefer smaller bit width, then relative power-of-two gap by integer cross multiplication, expected logical bits by rational comparison, smaller q, and finally SD/RD/provenance tie-breaks.

A reproducible Frodo-640-only diagnostic run is:

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release
cmake --build build-offline --target generate_sdat test_frodo640_compact_selector -j
./build-offline/generate_sdat --config offline/configs/frodo640.conf \
  --solver epsilon-svp-generated-baseline-dominating-power2-close --reproducible
python3 offline/scripts/frodo640_compact_epsilon_report.py \
  --trace offline/generated/sda_all_candidates.csv
```

Generated trial traces are research-only and should be consolidated into `offline/generated/research/frodo640/frodo640_pareto_frontier.csv`, `frodo640_search_report.txt`, and `frodo640_cleanup_manifest.txt`. Production Frodo-640 is replaced only if a certified epsilon-derived candidate passes all hard constraints and is strictly better than the incumbent; Frodo-976 and Frodo-1344 must not be changed by this workflow.
