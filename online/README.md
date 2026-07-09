# 在线 SDAT/CDT sampler

本目录把在线采样与离线搜索/生成完全分离。在线目标只编译 `online/common`、`online/reference`、`online/avx2`、`online/test`、`online/benchmark`，不链接 GMP、MPFR、fplll、Python 或任何离线 generator。

* Frodo-640/976/1344：本次审计只在 `generated/historical_sda_reference.csv` 和配置中找到论文 q（14534、7442、102），没有找到完整 PMF/cumulative 冻结表。因此在线 Frodo 表被显式标记为 unavailable，未编造 PMF。
* Falcon：使用 `generated/research/falcon/falcon_sdat_selected.{csv,h}` 和 certificate 中已选定的 epsilon-BKZ SDAT。该表是 base nonnegative SDAT，不是 exact-SVP 结果，也不是完整 Falcon Gaussian sampler。
* Reference：portable C17，固定长度全表扫描 CDT lookup；bounded uniform 使用 rejection sampling，不使用 modulo reduction。
* AVX2：sample-parallel batch lookup；Falcon 一次处理 4 个 72-bit 样本。尾部在 AVX2 源文件内用 scalar tail 处理。
* 72-bit 表示：`sdat_u72 { uint64_t lo; uint8_t hi; }`，`hi` 是 bits 64..71；9-byte 输入为 little-endian low64 + high8。结构 padding 不用于序列化。
* 常数时间说明：lookup 扫描固定 threshold 数量，不按随机值提前退出、不用随机值索引表；UniformBnd 的 rejection 次数可变，且只依赖公开随机候选，因此本 sampler 不声称整体固定周期。

构建：`cmake -S . -B build -DSDA_BUILD_BENCHMARKS=ON && cmake --build build --target test_sdat_online benchmark_sdat_online`。
测试：`ctest --test-dir build -R test_sdat_online --output-on-failure`。
benchmark：`./build/benchmark_sdat_online > generated/online/sdat_sampler_benchmark.csv`。
