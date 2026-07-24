# C Benchmarks

Four C17 benchmarks are maintained:

- `benchmark_frodo_sample_n`
- `benchmark_frodo_breakdown`
- `benchmark_falcon_base_sampler`
- `benchmark_falcon_breakdown`

Run them through:

```sh
benchmark/scripts/run_frodo_benchmarks.sh
benchmark/scripts/run_falcon_benchmarks.sh
```

Each runner configures a Release build with `SDA_BUILD_BENCHMARKS=ON`, builds only its two benchmark targets, runs them, and writes raw CSV under `build/benchmark-results/`. Environment variables accepted by the benchmark executables can be exported before invoking a runner. Statistical summarization is intentionally outside this lightweight repository.

The benchmarks use the frozen online tables and do not generate or modify table parameters. Frodo measures the one-dimensional sampler core; Falcon measures its base half-Gaussian sampler, not full signing.
