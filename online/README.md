# Online Original CDT and SDA_CDT

`online/` contains production runtime samplers, runtime table mappings, reference/AVX2 backend implementations, and online correctness tests. It does not contain performance benchmarks; benchmark sources and scripts live under the root `benchmark/` directory.

Frozen Frodo SDA tables:

* Frodo-640 q=14534
* Frodo-976 q=7442
* Frodo-1344 q=102

Original Frodo CDT uses the official power-of-two CDT tables with a 15-bit magnitude draw plus one sign bit. SDA_CDT uses `UniformBnd(q)` rejection sampling without modulo reduction. Falcon support here is limited to the frozen base-table online sampler; this directory does not implement full Falcon signing.

Build online correctness tests:

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-online -j
ctest --test-dir build-online --output-on-failure
```

Build benchmarks separately with `-DSDA_BUILD_BENCHMARKS=ON` and use `benchmark/scripts/`.

## Falcon base sampler

`online/falcon/falcon_base_sampler.{c,h}` implements only the portable half-Gaussian base sampler for support `{0,...,18}`. The Original path uses the official Falcon 72-bit CDT table already recorded in the production table metadata, and the SDA path uses the frozen Falcon SDA table with exact bounded-uniform rejection over its 72-bit denominator.

## Reference vs AVX2 reporting

Portable/reference samplers are the paper-primary comparison path. AVX2 remains available for correctness and future optimization work but should not be mixed with reference rows in the main speedup table.

## Frodo sampler framework

The Frodo runtime exposes a unified sampler dispatch layer with orthogonal dimensions: sampler kind (`original-cdt` or `sda-cdt`), backend (`reference` or `avx2`), frontend (`original-word`, `packed-bit`, or `word-oriented`), and parameter set (`frodo640`, `frodo976`, `frodo1344`).  Parameter descriptors reference the existing production tables; they do not copy or replace q values, CDFs, PMFs, thresholds, manifests, or hashes.

`reference` means portable C.  It may be inlined, unrolled, or auto-vectorized by the compiler and is therefore not labelled `scalar`.  Hand-written AVX2 remains in the AVX2 backend and is not used to justify paper-primary reference speedups.
