# Offline phase

The offline phase builds target distributions, searches denominators/candidates, verifies metrics, and exports reviewed online tables.

Minimal example:

```sh
cmake -S . -B build-offline -DCMAKE_BUILD_TYPE=Release
cmake --build build-offline --target generate_sdat verify_sdat -j
./build-offline/generate_sdat --config offline/configs/frodo640.conf --solver exact-denominator --reproducible
./build-offline/verify_sdat --all
```

Frodo exact paths include exact-denominator search and SDA-specialized exact `l_infinity` SVP/certificate code in `offline/common/sda_exact_linf*.c`. Falcon research generation in `offline/scripts/generate_falcon_sdat.py` is heuristic epsilon-BKZ/`l_2` work and is kept separate from exact `l_infinity` SVP claims.

All regenerated outputs should be written below `offline/generated/`.
