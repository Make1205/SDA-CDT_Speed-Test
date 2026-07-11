# Table Format

Production online table metadata is defined by `online/common/sdat_tables.c`, `online/common/sdat_tables.h`, and the manifests under `online/tables/`. Frodo production SDA denominators are frozen at q=14534 for Frodo-640, q=7442 for Frodo-976, and q=102 for Frodo-1344.

Table-size metadata distinguishes threshold payload bits from native C storage bytes. Research frontier files are not table-format authorities; only promoted production tables and manifests define runtime q, thresholds, hashes, and storage metadata.

Benchmark CSV schemas live with benchmark tooling under `benchmark/` and are not production table manifests.
