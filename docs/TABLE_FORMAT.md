# Table format

Online runtime tables are `sdat_table` records from `online/common/sdat_types.h` and `online/common/sdat_tables.h`. Each table records scheme, parameter set, table kind, source method, support, denominator bit width, PMF/cumulative threshold storage type, denominator, availability, exact/heuristic flags, and memory metrics.

Committed manifests in `online/tables/frodo/` and `online/tables/falcon/` record scheme/parameter set, denominator `q`, support, table kind, hashes, storage type, validation flags, and provenance. If metadata is missing from an old source artifact, add a manifest rather than regenerating or changing table values.
