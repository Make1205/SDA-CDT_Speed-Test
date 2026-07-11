# Offline Phase

The offline phase owns table generation, exact-SVP/SDA solving, PMF/CDF/threshold export, certificate production, and verification. It does not contain performance benchmarks; benchmark sources and scripts live under `benchmark/`.

Frodo production tables are frozen for the default workflow: Frodo-640 q=14534, Frodo-976 q=7442, and Frodo-1344 q=102. Future epsilon-driven table research must run outside the default production workflow and must not overwrite reviewed production artifacts.

Offline verification entry points remain `generate_sdat`, `verify_sdat`, and the offline correctness tests. Generated research traces, candidate CSV files, solver logs, and temporary certificates belong in ignored workspaces and are not production inputs.
