# SDA-CDT full C workflow

This repository now provides a C17-only offline/online SDA-CDT workflow.  It replaces the previous C++17 benchmark-only implementation (`std::vector`, templates, and `rdtscp`) with C modules, CMake targets, generated fixtures, tests, and reproducible command-line tools.

## Dependencies

Required: GMP and MPFR development headers/libraries. Optional: FLINT for future heuristic LLL candidate generation. If FLINT is not enabled, `--solver lll` exits with `dependency unavailable`; `exact-linf` and `target-q` remain available.

Ubuntu example:

```sh
sudo apt-get install libgmp-dev libmpfr-dev
```

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Options: `-DSDA_ENABLE_FLINT=ON/OFF`, `-DSDA_ENABLE_SANITIZERS=ON/OFF`, `-DSDA_BUILD_BENCHMARKS=ON/OFF`, `-DSDA_MPFR_DEFAULT_PRECISION=512`.

## Tools

```sh
./build/generate_sdat --all
./build/generate_sdat --config config/frodo640.conf --solver exact-linf
./build/generate_sdat --config config/falcon.conf --solver lll
./build/generate_sdat --config config/falcon.conf --solver target-q --target-q 4696835740265763827900
./build/verify_sdat generated/sda_generated_tables.h
./build/benchmark_sampling --all
./build/benchmark_offline --all
```

## Solver modes

* `exact-linf`: exact recursive \(\ell_\infty\) enumeration for small upper-triangular SDA bases; reports nodes, leaves, pruning, best vector, and norm.
* `lll`: optional FLINT heuristic candidate mode only. Results must be labelled `heuristic LLL candidate`, never BKZ or exact SVP.
* `target-q`: deterministic constrained balanced largest-remainder rounding for a specified denominator. It is not an SVP result.

## Distributions and metrics

MPFR is used for high-precision finite-support discrete Gaussian probabilities, statistical distance, pointwise error, and multiplicative Rényi divergence. The code distinguishes finite support tables from infinite-support tails in reports; the checked-in fixture report notes that Frodo tables reproduce manuscript masses and Falcon is fixture/target-q derived from the repository's existing cumulative data.

## Online sampling

The sampler uses an RNG callback:

```c
typedef int (*sda_random_bytes_fn)(void *ctx, uint8_t *out, size_t out_len);
```

`uniform_bounded(q)` supports `q=1`, powers of two, `q <= 2^64`, and Falcon-size denominators above 64 bits using `__uint128_t`. Benchmark RNG is XorShift-style and is **benchmark-only**, not production cryptographic randomness.

For Frodo, magnitude is sampled first. A sign bit may be consumed for zero to keep a fixed randomness flow, but zero's sign is ignored and does not alter the zero probability.

## Bits and memory

* Accepted bits: \(\lceil \log_2 q \rceil\) per accepted bounded-uniform sample.
* Raw bits: accepted bits multiplied by rejection attempts; expected raw bits are \(b 2^b/q\).
* Ideal packed payload bits: fixed-width table payload \(N\lceil\log_2 q\rceil\).
* Native bytes: actual C array storage using `sizeof`-based `__uint128_t` arrays. Native benchmarks are not claimed to be bit-packed benchmarks.

## Reproducing paper/reference tables

Run `./build/generate_sdat --all` and `./build/verify_sdat generated/sda_generated_tables.h`. Frodo fixtures are explicit regression data matching the manuscript masses. The previous Falcon cumulative table contained a non-monotone tail entry; this workflow marks Falcon as target-q fixture data and not exact enumeration.

## Known limitations

Falcon exact enumeration is expected to be impractical in this compact implementation and is not represented as exact. FLINT LLL is optional and, when absent, reports dependency unavailable. Real bit-packed sampler storage is not implemented; reports separate ideal packed payload bits from native bytes.
