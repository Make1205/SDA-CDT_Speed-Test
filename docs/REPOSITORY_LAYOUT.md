# Repository layout

`online/` is the dependency-free runtime boundary. It contains common table and bit-reader code, Frodo and Falcon samplers, committed frozen tables, and tests.

`offline/` is the C generation boundary. `common/` contains distributions, exact `l_infinity` enumeration, metrics, and table validation. `scripts/` contains the three C generation/export/verification tools, `configs/` contains Frodo and Falcon inputs, and `tests/` exercises the library. `generated/` is ignored working space.

`benchmark/` contains four maintained C benchmarks and two shell runners. `docs/` contains concise project documentation.
