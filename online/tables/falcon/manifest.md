# Falcon online tables

Falcon online base-table support is compiled into `online/common/sdat_tables.c`. The SDA Falcon table is marked heuristic because it originates from epsilon-BKZ research artifacts, not an exact `l_infinity` SVP proof. This directory keeps Falcon source/provenance artifacts that were committed as reviewed online inputs.

The canonical SDA PMF and cumulative thresholds remain unchanged in
`online/common/sdat_tables.c`. The production Gaussian0 mapper serializes that same PMF as
19 Falcon-style reverse-tail rows in `online/falcon/falcon_reverse_tail.c` (18 exact tails
and the official loop's final zero row). Its LE9 runtime-table SHA-256 is
`2d6a6d7a65aa8c86c276f134e88dfce5df85d2486c1a4b4cc16cb79fb2ae6fcd`; the canonical
PMF SHA-256 remains `15cb40167eda4761313ad83a6779b4657a7340625dac863a48843822e5802caa`.
