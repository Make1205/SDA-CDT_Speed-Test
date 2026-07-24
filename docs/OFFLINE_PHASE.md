# Offline Phase

The production Frodo workflow is epsilon-driven: construct the SDA lattice for
one epsilon, run exact l-infinity SVP, and either accept the solver's unmodified
`(p_0,...,p_{N-1},q)` after all structural, compactness, SD, and RD checks or
reject that epsilon.  `generate_sdat --all` performs a logarithmic epsilon grid
of those independent calls.  A single instance can be requested with:

```
generate_sdat --config offline/configs/frodo640.conf \
  --solver exact-linf-svp --epsilon VALUE
```

`exact-denominator-search` and its fixed-q rounding are research-only and must
be selected explicitly; they are not part of the paper production path.

The offline phase owns table generation, exact-SVP/SDA solving, PMF/CDF/threshold export, certificate production, and verification. It does not contain performance benchmarks; benchmark sources and scripts live under `benchmark/`.

Frodo production tables are frozen for the default workflow: Frodo-640 q=14534, Frodo-976 q=7442, and Frodo-1344 q=102. Future epsilon-driven table research must run outside the default production workflow and must not overwrite reviewed production artifacts.

Offline verification entry points remain `generate_sdat`, `verify_sdat`, and the offline correctness tests. Generated research traces, candidate CSV files, solver logs, and temporary certificates belong in ignored workspaces and are not production inputs.
