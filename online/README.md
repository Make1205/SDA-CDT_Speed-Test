# 在线 Original CDT 与 SDA_CDT

术语约定：SDAT 是 SDA table，即离线生成并冻结的整数概率表；SDA_CDT 是使用 SDAT table 的在线 sampler。SDAT table 本身不是 sampler，也不是在线算法。

本目录只属于在线阶段。Offline 代码可以依赖 GMP、MPFR、fplll、Python；online target 不链接这些库，也不在运行时解析 CSV/header/certificate 或调用离线 generator。

* Original CDT：使用原始 power-of-two CDT table。Frodo original CDT table 来自 `generated/original_baseline_tables.h`，denominator 为 2^15；在线 lookup 省略 terminal q threshold，但保留 PMF/cumulative artifact 验证。
* SDA_CDT：使用冻结 SDAT table，先执行 `UniformBnd(q)` rejection，再对 SDA cumulative table 做 fixed full scan。Frodo 三套论文 SDAT 仍缺少完整 historical-paper PMF/cumulative，因此标记 unresolved；Falcon 使用 `generated/research/falcon/falcon_sdat_selected.{csv,h}` 和 certificate 中已选定的 epsilon-BKZ SDAT。
* Falcon SDAT 是 base nonnegative SDA table，不是 exact-SVP 结果，也不是完整 Falcon Gaussian sampler。本目录不集成 Bernoulli exponential、center/sigma transformation、签名流程或 full Gaussian rejection process。
* Reference 实现是 portable C17，不使用 AVX intrinsics。AVX2 实现是 sample-parallel batch path：Frodo original CDT 使用 8 个 32-bit lane；Falcon SDA_CDT 使用 4 个 72-bit lane。scalar tail 和 fallback 由统计字段单独报告。
* Original CDT 从 power-of-two random domain 直接抽取 k bits，不加入 arbitrary-bound rejection。SDA_CDT 使用 rejection sampling，不使用 modulo reduction。
* lookup-only benchmark 只测对应 table 的 fixed full scan；end-to-end sampler benchmark 才包含 RNG、bit masking、rejection 和 output store。两者不能混称。
* AVX2 不保证所有参数和 batch size 都更快，benchmark 必须报告正负 speedup。

构建：`cmake -S . -B build -DSDA_BUILD_BENCHMARKS=ON && cmake --build build --target test_sdat_online benchmark_sdat_online`。
测试：`ctest --test-dir build -R test_sdat_online --output-on-failure`。
benchmark：`ONLINE_BENCH_SAMPLES=1000000 ./build/benchmark_sdat_online > generated/online/sampler_benchmark.csv`。
