# Table Format

Production online table metadata is defined by `online/common/sdat_tables.c`, `online/common/sdat_tables.h`, and the manifests under `online/tables/`. Frodo production SDA denominators are frozen at q=14534 for Frodo-640, q=7442 for Frodo-976, and q=102 for Frodo-1344.

Table-size metadata distinguishes threshold payload bits from native C storage bytes. Research frontier files are not table-format authorities; only promoted production tables and manifests define runtime q, thresholds, hashes, and storage metadata.

Benchmark CSV schemas live with benchmark tooling under `benchmark/` and are not production table manifests.

## Threshold convention

The paper stores `SDAT[i] = C_i - 1` and increments the support index when
`u > SDAT[i]`.  This repository stores the ordinary cumulative value `C_i`
and uses `u >= C_i`; the two forms are exactly equivalent, not an off-by-one
error.  The frozen original Frodo tables instead use the official
upper-inclusive thresholds and therefore compare `x > threshold`.  Generic,
specialized, and AVX2 SDA paths all compare their stored cumulative values with
`>=`.

Falcon has no C production generator in this repository.  Its current online
SDA table is a frozen experimental benchmark table.  The legacy Python/BKZ
research artifacts are not a production generator and must not be used to
round, normalize, or automatically replace Falcon's frozen `p` or `q`.
