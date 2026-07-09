# 在线 Original CDT 与 SDA_CDT

术语约定：SDAT 是 SDA table，即离线生成并冻结的整数概率表；SDA_CDT 是使用 SDAT table 的在线 sampler。SDAT table 本身不是 sampler，也不是在线算法。Original CDT 是原始 Frodo/Falcon 使用 power-of-two random domain 的在线 sampler。

本目录只属于 online stage。Offline 代码可以依赖 GMP、MPFR、fplll、Python；online target 不链接这些库，也不在运行时解析 CSV/header/certificate 或调用离线 generator。

* Frodo SDAT：使用论文 Table 5 / Table 6 exact integer masses and cumulative thresholds。Frodo-640 q=14534，Frodo-976 q=7442，Frodo-1344 q=102；三者均表示非负 magnitude distribution，sign 在本层之外处理。
* Original Frodo CDT：使用 `generated/original_baseline_tables.h` 中的 power-of-two CDT table，denominator 为 2^15，draw bits 为 15。
* Original Falcon base CDT：使用 Falcon reference `sign.c` 的 `gaussian0_sampler` constants。该官方 base sampler 读取 72 random bits，拆为三个 24-bit limbs，并按 reverse-tail thresholds 统计 `threshold > random` 的个数作为非负 base magnitude。
* Falcon SDAT：使用 `generated/research/falcon/falcon_sdat_selected.{csv,h}` 和 certificate 中已选定的 epsilon-BKZ SDAT。Falcon SDAT 不是 exact-SVP 结果，也不是完整 Falcon Gaussian sampler。
* Reference 实现是 portable C17，不使用 AVX intrinsics。AVX2 实现是 sample-parallel batch path：Frodo Original CDT/SDA_CDT 使用 8 个 32-bit lane；Falcon SDA_CDT 使用 4 个 72-bit lane；Original Falcon AVX2 path 保持官方 reverse-tail 比较语义。
* Original CDT 从 power-of-two random domain 直接抽取 k bits，不加入 arbitrary-bound rejection。SDA_CDT 使用 `UniformBnd(q)` rejection sampling，不使用 modulo reduction。
* lookup-only benchmark 只测对应 table 的 fixed full scan；end-to-end sampler benchmark 才包含 RNG、bit masking、rejection/refill 和 output store。两者不能混称。
* Benchmark 使用相同 deterministic randombytes callback、sample count、batch size、compiler flags 和 cycle timer；AVX2 不保证所有参数和 batch size 都更快，必须报告正负 speedup。
* Falcon 这里只比较 base sampler；不集成 Bernoulli exponential、center/sigma outer transformation、签名流程或 full Gaussian rejection process。

构建：`cmake -S . -B build -DSDA_BUILD_BENCHMARKS=ON && cmake --build build --target test_sdat_online benchmark_sdat_online`。
测试：`ctest --test-dir build -R test_sdat_online --output-on-failure`。
benchmark：`ONLINE_BENCH_SAMPLES=1000000 ./build/benchmark_sdat_online > generated/online/sampler_benchmark.csv`。
