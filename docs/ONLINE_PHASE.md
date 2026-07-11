# Online phase

The online phase executes table lookup and bounded uniform sampling from reviewed SDA-CDT/original-CDT table data. Build it with:

```sh
cmake -S . -B build-online -DSDA_BUILD_BENCHMARKS=ON
cmake --build build-online --target test_sdat_online benchmark_sdat_online -j
ctest --test-dir build-online -R test_sdat_online --output-on-failure
```

The online phase uses `online/common`, `online/frodo`, `online/falcon`, and committed metadata in `online/tables`. It does not depend on `offline/generated`.
