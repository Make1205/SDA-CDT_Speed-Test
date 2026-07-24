# Online Phase

The online phase is dependency-free C17. It contains frozen Frodo and Falcon tables, portable reference samplers, AVX2 implementations, the Frodo dispatcher, and correctness tests. It does not depend on GMP/MPFR or offline generated files.

Frodo SDA denominators remain 14534, 7442, and 102 for Frodo-640, Frodo-976, and Frodo-1344. Falcon's current SDA table is also frozen.

```sh
cmake -S . -B build-online -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=OFF
cmake --build build-online -j
ctest --test-dir build-online --output-on-failure
```
