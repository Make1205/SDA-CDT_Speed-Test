#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILD=${BUILD:-$ROOT/build-online}
CPU_COUNT=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)
DEFAULT_CORE=0
if [ "$CPU_COUNT" -gt 2 ]; then DEFAULT_CORE=2; fi
CORE=${FRODO_BENCH_CORE:-$DEFAULT_CORE}
OUT=${FRODO_BENCH_OUT:-$ROOT/offline/generated/online/frodo-stable-$(date -u +%Y%m%dT%H%M%SZ)}
mkdir -p "$OUT"
{
  echo timestamp_utc=$(date -u +%FT%TZ)
  echo kernel=$(uname -srmo)
  echo cpu_model=$(sed -n 's/^model name[[:space:]]*: //p' /proc/cpuinfo | head -1)
  if compgen -G '/sys/devices/system/cpu/cpu*/cpufreq/scaling_governor' >/dev/null; then
    echo governors=$(cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor 2>/dev/null | sort -u | tr '\n' ',' | sed 's/,$//')
  else echo governors=unavailable; fi
  if [ -r /sys/devices/system/cpu/intel_pstate/no_turbo ]; then echo intel_no_turbo=$(cat /sys/devices/system/cpu/intel_pstate/no_turbo); else echo turbo=unknown; fi
  if command -v taskset >/dev/null; then echo taskset=available core=$CORE cpu_count=$CPU_COUNT; else echo taskset=unavailable; fi
  git -C "$ROOT" rev-parse HEAD | sed 's/^/commit=/'
} | tee "$OUT/environment.txt"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DSDA_BUILD_BENCHMARKS=ON
cmake --build "$BUILD" --target benchmark_frodo_sample_n test_frodo_sample_n -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"
if command -v taskset >/dev/null; then
  taskset -c "$CORE" "$BUILD/benchmark_frodo_sample_n" > "$OUT/frodo_sample_n.csv"
else
  "$BUILD/benchmark_frodo_sample_n" > "$OUT/frodo_sample_n.csv"
fi
echo output_dir=$OUT
