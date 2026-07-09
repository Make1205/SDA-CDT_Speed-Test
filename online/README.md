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

## 一键 formal/quick benchmark

一键脚本路径：`scripts/run_online_benchmarks.sh`。默认 formal 配置为 `warmup_samples=100000`、`repetitions=21`、`samples_per_repetition=1000000`、`batch_sizes=1,4,8,16,64,256,1024`，并使用 Release build：

```sh
./scripts/run_online_benchmarks.sh --formal --cpu 0
```

quick 模式只用于开发自检，不可用于论文或标准化文档：

```sh
./scripts/run_online_benchmarks.sh --quick --cpu 0 --batch-sizes 1,8 --samples 10000 --repetitions 3
```

可用参数也可通过环境变量指定：`ONLINE_BENCH_CPU`、`ONLINE_BENCH_REPETITIONS`、`ONLINE_BENCH_SAMPLES`、`ONLINE_BENCH_WARMUP`、`ONLINE_BENCH_BATCH_SIZES`、`ONLINE_BENCH_OUTPUT_DIR`。脚本会配置并构建 `benchmark_sdat_online` 和 `test_sdat_online`，先运行 `ctest --test-dir build-bench -R test_sdat_online --output-on-failure`，正确性 precheck 失败时停止 benchmark。

脚本默认写入 timestamp 子目录：`generated/online/formal/<timestamp>/`，并维护 `generated/online/formal/latest` 与 `generated/online/formal/latest.txt`。主要 artifact 包括：`benchmark_platform.txt`、`benchmark_config.txt`、`lookup_raw.csv`、`lookup_summary.csv`、`end_to_end_raw.csv`、`end_to_end_summary.csv`、四个独立 speedup CSV、`table_memory.csv`、`randomness_cost.csv`、`benchmark_summary_zh.txt` 和 `run_log.txt`。

benchmark 类型严格区分：`lookup-only` 只测合法随机整数之后的 table lookup，不包含 RNG、bit masking、rejection 或 lane refill；`end-to-end` 才测完整 online base sampler。论文主结果应使用 `end-to-end`；`lookup-only` 只用于解释 table scan、SIMD、table size 和 compare width 成本。

四类 comparison dimension 分别输出为独立 CSV：

1. `speedup_original_avx2_vs_ref.csv`：Original CDT sampler 中 AVX2 vs Reference。
2. `speedup_sda_avx2_vs_ref.csv`：SDA_CDT sampler 中 AVX2 vs Reference。
3. `speedup_sda_vs_original_ref.csv`：Reference 实现下 SDA_CDT vs Original CDT。
4. `speedup_sda_vs_original_avx2.csv`：AVX2 实现下 SDA_CDT vs Original CDT。

脚本会检测 AVX2、RDTSCP、`constant_tsc`/`invariant_tsc`，尝试用 `taskset` 绑定固定 logical CPU，并在 `benchmark_platform.txt` 中记录 affinity 是否成功。Reference target 不使用 `-mavx2`；AVX2 target 单独使用 `-mavx2`。SDAT 仍然只表示冻结 table；SDA_CDT 才是使用 SDAT 的 online sampler。
