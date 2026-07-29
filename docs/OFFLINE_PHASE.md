# Offline Phase

The offline implementation is C17 and uses GMP/MPFR. For one Frodo epsilon it constructs the SDA lattice, runs exact l-infinity SVP, obtains `(p_0,...,p_{N-1},q)`, and checks positivity, `sum(p)=q`, compactness, cumulative-table consistency, SD, and RD. A valid vector is accepted unchanged; an invalid epsilon is rejected. The production path performs no fixed-q rounding or normalization.

```sh
./build-offline/generate_sdat --config offline/configs/frodo640.conf \
  --solver exact-linf-svp --epsilon VALUE
```

`exact-denominator-search` is an explicit research-only mode. The checked-in Frodo and Falcon online tables are frozen. A later phase will extend the pure-C offline path for both families; no Python generator is retained.

`offline/generated/` is an ignored work directory. Only its `.gitkeep` is tracked. The maintained C tools are `generate_sdat`, `verify_sdat`, and `export_tables`.

Deterministic random-epsilon search uses MPFR-computed paper bounds and a
SplitMix64 stream derived independently for each parameter set:

```sh
./build-offline/generate_sdat --all --random-epsilon --seed 1 --max-trials 256
./build-offline/verify_sdat --generated offline/generated/frodo640
```

Falcon requests the `bkz20` backend and fail explicitly when the external
`fplll` executable is unavailable; there is no rounding fallback.

## Expanded epsilon search

`generate_sdat --config CONFIG --expanded-search --epsilon-search logarithmic-grid --compare-frozen` runs the three MPFR-derived ranges `[L,U]`, `[L/2,min(2U,0.95)]`, and `[L/4,min(4U,0.95)]`. Frodo uses 256/512/1024 points and Falcon uses 16/32/64 BKZ-20 points. `--stop-on-frozen-match` stops only after an exact `q`, PMF, and cumulative-table match. A single custom range uses `--epsilon-min`, `--epsilon-max`, `--epsilon-points`, and `--epsilon-search` (`logarithmic-grid`, `linear-grid`, or `random`).

Expanded-search artifacts are ignored files under `offline/generated/<parameter>/expanded-search/`. They record whether each first epsilon lies in the paper interval, exact candidate plateaus, canonical integer-basis hashes, independent quality rankings, and probability-distance rankings against the frozen online table. Quality metrics remain reporting-only and the solver PMF is never rounded or normalized. Validate an artifact set with `verify_sdat --search-dir DIR`.
