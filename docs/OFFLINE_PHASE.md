# Offline Phase

The offline implementation is C17 and uses GMP/MPFR. For one Frodo epsilon it constructs the SDA lattice, runs exact l-infinity SVP, obtains `(p_0,...,p_{N-1},q)`, and checks positivity, `sum(p)=q`, compactness, cumulative-table consistency, SD, and RD. A valid vector is accepted unchanged; an invalid epsilon is rejected. The production path performs no fixed-q rounding or normalization.

```sh
./build-offline/generate_sdat --config offline/configs/frodo640.conf \
  --solver exact-linf-svp --epsilon VALUE
```

`exact-denominator-search` is an explicit research-only mode. The checked-in Frodo and Falcon online tables are frozen. A later phase will extend the pure-C offline path for both families; no Python generator is retained.

`offline/generated/` is an ignored work directory. Only its `.gitkeep` is tracked. The maintained C tools are `generate_sdat`, `verify_sdat`, and `export_tables`.
