# C Benchmarks

The repository has one canonical C17 Frodo executable, `benchmark_frodo`, and one formal
user entry point:

```sh
benchmark/scripts/run_frodo_benchmarks.sh
```

Falcon now mirrors the same canonical fused/block-staged methodology through `benchmark_falcon` and `benchmark/scripts/run_falcon_benchmarks.sh`.

## Frodo measurements

The runner launches Original and SDA as separate, sequential processes while using the
same executable, build, seed derivation, word stream, timer, counts, parameter rotation,
and CPU affinity. It creates exactly two raw CSV files:

* `frodo_full_sampler_raw.csv`: `full-sampler-fused` and
  `full-sampler-block-staged`;
* `frodo_stage_breakdown_raw.csv`: `block-stage-input` and
  `block-stage-mapping`.

The paper-primary speed measurement remains `full-sampler-fused`: `original-reference`
or the production scalar `sda-word-reference` (`branchless-map-before-accept`). The
block-staged sampler is an explanatory pipeline and does not replace production.

The default stage block size is 4096 outputs for every parameter set. Override it only
with the common `FRODO_STAGE_BLOCK_SIZE`; it is recorded in every row. Candidate and sign
workspaces are allocated once before timing and reused for every block. No block performs
allocation. Raw words are generated before timing, and all timed regions exclude RNG,
fill, allocation, checksum, CSV, and stats replay.

### Additive block stages

For Original, each block executes:

1. input: raw words to candidate/sign buffers, with one word per output;
2. mapping: candidate/sign buffers to signed CDT outputs.

For SDA, each block executes:

1. input: specialized extraction, `candidate < q` rejection, sign extraction, and
   accepted-output compaction until the block is full;
2. mapping: accepted candidate/sign buffers to signed CDT outputs.

The staged implementation uses the same parameter-specialized extraction and mapping
primitives as the reference samplers, preserves accepted order and accounting, and
handles a final partial block. It never materializes sample-count-sized intermediate
arrays.

An uninstrumented call measures the entire block loop. A separate instrumented call uses
one serialized timestamp sequence per block (`t0`, input, `t1`, mapping, `t2`) and stores
both raw stage cycle totals. `reconstructed_cycles_total` is exactly the paired input plus
mapping cycles; no scaling, subtraction, fitting, or redistribution is used. The runner
also reports, without altering raw rows:

```
stage_sum_gap_percent = 100 * (input_cycles + mapping_cycles) / staged_full_cycles - 100
staging_overhead_percent = 100 * staged_full_cycles / fused_full_cycles - 100
```

The first quantity exposes timer and block-loop representativeness; values over 3% are a
reason to inspect the environment, not a value to force to zero. The second measures
intermediate-buffer, block-boundary, and lost-fusion overhead and is not expected to be
zero.

The old standalone full-array frontend/mapping measurements and accept-before-map
diagnostic are no longer emitted by the formal runner. Their primitives and correctness
tests remain, but they cannot be confused with the additive staged breakdown.

## Pairing and aggregation

Rows pair on parameter set, implementation, process index, repetition, input seed, and
input stream ID. Fused, staged-total, input-stage, and mapping-stage rows have identical
checksums and attempts/rejections/source-word accounting.

For reporting, first compute the paired per-repetition reconstruction
`input_cycles_r + mapping_cycles_r`. Within each process take the median of the 31
repetition values, then take the median of the five process medians. Do not add a median
input stage to a median mapping stage: in general `median(A) + median(B)` is not
`median(A+B)`.

## Runner and build policy

Defaults are five sequential processes, 31 repetitions, five warm-ups, 1,048,576
accepted outputs, equal-size mode, and block size 4096. `FRODO_BENCH_CPU` pins every
process with `taskset` when available. The runner validates headers, 51 fields, status,
unique keys, matched four-row groups, seed/stream pairing, checksums, accounting, exact
stage reconstruction, and expected row counts. It writes a non-CSV companion report
`frodo_benchmark_validation.txt` with observed gaps and overheads; raw measured cycles
are never rewritten.

The portable reference sampler and benchmark driver use `-O3 -fno-lto`, disabled IPO,
and disabled compiler vectorization (`-fno-tree-vectorize -fno-tree-slp-vectorize` on
GCC). Compiler ID, version, flags, and configured Git commit are included in every row.
There is still no genuine SDA word-oriented AVX2 implementation.

## Falcon base-sampler measurements

Falcon measures only the existing nonnegative Gaussian0 base sampler over support 0..18;
it does not include samplerZ, BerExp, signs, centering, FFT sampling, or signing. Both
implementations consume the same precomputed stream of little-endian nine-byte candidates,
decoded exactly as `sdat_u72 { uint64_t lo; uint8_t hi; }`. Original maps every 72-bit
candidate through the official reverse-tail CDT. SDA compares the decoded candidate with
its exact 72-bit q, rejects first, and maps only accepted candidates, so its production
variant is `accept-before-map`.

Run only:

```sh
benchmark/scripts/run_falcon_benchmarks.sh
```

It launches separate sequential Original and SDA `benchmark_falcon` processes and writes
`falcon_full_sampler_raw.csv`, `falcon_stage_breakdown_raw.csv`, and the non-CSV
`falcon_benchmark_validation.txt`. Defaults match Frodo: 5 processes, 31 repetitions, 5
warm-ups, 1,048,576 outputs, and a common block size of 4096. The two raw CSV headers are
byte-for-byte identical to Frodo's 51-field schema.

The instrumented Falcon staged call additionally records an outer serialized interval.
`uncovered_cycles_total = instrumented_staged_cycles_total - reconstructed_cycles_total`
is reported separately and is never assigned back to either stage. The runner requires
raw input plus mapping cycles to equal the reconstruction exactly, requires reconstruction
not to exceed the outer interval, and emits warnings rather than altering gaps over 3% or
10%. `staging_difference_percent` is a staged-versus-fused difference, not a claim of pure
memory overhead, because SDA fused and staged control/data ordering may differ.

Falcon aggregation follows Frodo: compute reconstruction and stage shares within each
matched repetition, take the median of 31 repetitions within each process, then the median
of five process medians. Never substitute `median(input) + median(mapping)` for the median
of paired sums. Original-to-SDA cycle change is `100 * (SDA / Original - 1)`.
