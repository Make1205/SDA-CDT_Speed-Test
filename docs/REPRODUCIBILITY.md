# Reproducibility

## Correctness

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-online -j
ctest --test-dir build-online --output-on-failure
```

If GMP and MPFR are installed, offline targets and tests are enabled by the same CMake project. `verify_sdat --all` intentionally fails in a clean checkout when no production tables exist in `offline/generated/`.

## Benchmarks

The maintained targets are `benchmark_frodo` and `benchmark_falcon`. Run their shell wrappers:

```sh
benchmark/scripts/run_frodo_benchmarks.sh
benchmark/scripts/run_falcon_benchmarks.sh
```

Outputs are raw CSV under `build/benchmark-results/`; they are transient and ignored. No Python summarizer or generated summary is part of this repository.

For canonical Frodo measurements, use only `benchmark/scripts/run_frodo_benchmarks.sh`.
It runs Original and SDA sequentially through the same `benchmark_frodo` executable and
writes `frodo_full_sampler_raw.csv`, `frodo_stage_breakdown_raw.csv`, and the non-CSV
validation report. Keep the build, affinity, counts, mode, and stage block size fixed.
Pair the four raw measurements by parameter set, implementation, process index,
repetition, input seed, and stream ID; reconstruct stages per repetition before taking
process and cross-process medians.

For canonical Falcon base-sampler measurements, use only
`benchmark/scripts/run_falcon_benchmarks.sh`. It runs separate Original and SDA processes
through `benchmark_falcon`, uses the same 51-field schema as Frodo, and produces only
`falcon_full_sampler_raw.csv`, `falcon_stage_breakdown_raw.csv`, plus a text validation
report. Falcon here means the nonnegative Gaussian0 base sampler, not samplerZ or signing.
Original and SDA mapping rows invoke the same separately compiled Falcon-style reverse-tail
kernel over the same three-24-bit-limb representation and fixed 19-row loop. The SDA table
is an exact reverse-tail serialization of the frozen PMF; SDA's q rejection remains in its
input stage. The CSV `table_hash` identifies the runtime table, not merely the distribution
name.
