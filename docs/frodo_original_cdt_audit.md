# Frodo Original CDT performance audit

This note records the audit for the anomalously low Frodo-640 Original CDT cycles/sample observed in the portable sample_n benchmark.  It is intentionally diagnostic: it does not change Frodo q values, PMFs, CDFs, thresholds, table hashes, manifests, sign convention, Falcon code, or AVX2 code.

## Call path audited

For the `official-original-scalar` benchmark rows, all three parameter sets use the same benchmark and wrapper layers:

```text
benchmark_frodo_sample_n.c main()
-> one(parameter_set, original_table, sda_table, sample_count, repetition)
-> run_impl("official-original-scalar", ...)
-> memcpy(out, words, sample_count * sizeof(uint16_t))
-> frodo_original_sample_n(out, sample_count, original_table)
-> parameter-table pointer dispatch
-> parameter-specific look_orig* helper
-> prnd = word >> 1; sign = word & 1
-> fixed branchless threshold scan using x > cdf[j]
-> frodo_apply_sign(magnitude, sign)
-> in-place output store
-> cksum(out, sample_count) after timing
```

The dispatch is symmetric in structure, but it deliberately uses three parameter-specific fixed-scan helpers rather than one runtime-count loop.  There is no function pointer dispatch, no generic fallback for Frodo-976 or Frodo-1344, and no `if (level == 640)` benchmark path.  The only parameter differences in the hot path are the table symbol and the unrolled threshold count: 13, 11, and 7 comparisons respectively.

## Original CDF parameters

| Parameter | Official CDF | Threshold count | Support max | Stored entries | Terminal 32767 scanned? | Compare-adds/sample | Element width | Native table bytes |
| --- | --- | ---: | ---: | ---: | --- | ---: | ---: | ---: |
| Frodo-640 | `{4643,13363,20579,25843,29227,31145,32103,32525,32689,32745,32762,32766,32767}` | 13 | 12 | 13 | yes | 13 | `uint16_t` | 26 |
| Frodo-976 | `{5638,15915,23689,28571,31116,32217,32613,32731,32760,32766,32767}` | 11 | 10 | 11 | yes | 11 | `uint16_t` | 22 |
| Frodo-1344 | `{9142,23462,30338,32361,32725,32765,32767}` | 7 | 6 | 7 | yes | 7 | `uint16_t` | 14 |

All three Original paths use `prnd = word >> 1`, `sign = word & 1`, strict official comparison `prnd > cdf[j]`, and sign application `((-sign) ^ magnitude) + sign` through `frodo_apply_sign`.

## Benchmark audit

The benchmark copies exactly `sample_count` prepared 16-bit words into the output buffer before timing Original CDT, transforms the whole buffer in place during timing, and checksums exactly `sample_count` outputs after timing.  The large batches are 5120, 7808, and 10752 for Frodo-640, Frodo-976, and Frodo-1344.  The denominator is always the same `sample_count` printed in the CSV row.  There is no inner repeat in the timed Original path, no metrics/accounting mixed into timing rows, and no AVX2 call for the `official-original-scalar` label.

The source words are deterministic but are allocated, filled, and copied at runtime per repetition, so the compiler cannot fold the benchmark result to constants.  The post-timing checksum prevents dead-code elimination of the output stores.

## Assembly and compiler behavior

The release assembly confirms the root cause: the three helpers are all inlined into `frodo_original_sample_n`, but GCC chooses a much more SIMD-friendly lowered form for the Frodo-640 fixed scan.  On the audit host, the Frodo-640 branch contains vector integer operations on packed words (`xmm`/`ymm`, including shifts, broadcasts, compares, and packed arithmetic) and a vectorized loop over samples.  Frodo-976 and Frodo-1344 also have unrolled compare expressions at the source level, but the generated hot path retains a less efficient scalar/partly scalar loop shape for this compiler/build.  No path is constant-folded, and no Original scalar path calls the AVX2 implementation.

Therefore the `scalar` label in this benchmark should be read as "optimized portable C": it is portable C source, but the compiler may auto-vectorize or otherwise lower it to SIMD instructions.

## Correctness audit

`online/tests/test_frodo_sample_n.c` now exhaustively checks all 65536 input words for each Frodo Original parameter set through both paths: the parameter-specific helper selected by pointer identity and a copied `sdat_table` that forces the generic fallback loop.  This confirms that the specialized helpers execute the same strict-compare CDF semantics as the generic implementation for Frodo-640, Frodo-976, and Frodo-1344.

## Classification

This audit classifies the anomaly as Case 1: a real compiler/build artifact in optimized portable C, not a benchmark denominator bug, not a missing threshold in Frodo-640, and not a Frodo-976/Frodo-1344 generic fallback asymmetry.  The paper should label these rows `optimized portable C`, not `strict scalar instruction implementation`.
