# Repository layout

`online/` is the runtime boundary. It contains `common/` table definitions and bounded-uniform helpers, `frodo/` scalar/reference code, `falcon/` AVX2 code, committed final table artifacts under `tables/`, plus `tests/` and `benchmarks/`.

`offline/` is the research and generation boundary. `common/` is the C library for SDA distributions, exact `l_infinity` enumeration, metrics, RNG test helpers, and table validation. `scripts/` contains generation/export/verification tools. `configs/` contains Frodo and Falcon input configs. `tests/` and `benchmarks/` exercise the offline library. `generated/` is ignored and receives regenerated artifacts.

`docs/` contains human documentation. `third_party/` is reserved for vendored or submodule dependencies; no third-party tree is currently tracked there.
