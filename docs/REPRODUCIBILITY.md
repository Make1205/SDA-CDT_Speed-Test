# Reproducibility

Regenerated research files are ignored under `offline/generated/`. To reproduce a small Frodo candidate and verification run:

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release
cmake --build build-offline --target generate_sdat verify_sdat -j
./build-offline/generate_sdat --config offline/configs/frodo640.conf --solver exact-denominator --reproducible
./build-offline/verify_sdat --all
```

For online benchmark smoke testing:

```sh
cmake -S . -B build-online -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-online --target benchmark_sdat_online -j
ONLINE_BENCH_SAMPLES=10000 ONLINE_BENCH_REPETITIONS=3 ./build-online/benchmark_sdat_online
```

Long Falcon BKZ and full formal benchmark runs are intentionally not part of the smoke path.
