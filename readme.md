# SDA-CDT Benchmark
This repository contains the performance benchmarking code for the paper **"Simultaneous Diophantine Approximation for Compact Discrete Gaussian Sampling"**.

It evaluates the speed (CPU cycles) and memory usage of the proposed SDA-CDT sampler against the standard CDT sampler used in lattice-based cryptography schemes like FrodoKEM and Falcon.

## 🚀 Features
- Real Data Testing: Uses exact probability tables and parameters derived from Frodo-640, Frodo-976, Frodo-1344, and Falcon.

- High Precision: Measures execution time using CPU ``` rdtscp ``` instructions for cycle-accurate results.

- Optimized Implementation:

    - ses XorShift128+ for fast random number generation.

    - Implements Lemire's algorithm for fast uniform sampling (avoiding expensive modulo divisions).

    - Compile-time loop unrolling and type shrinking (e.g., ```uint8_t``` for Frodo-1344).


## 🛠️ Prerequisites
- A C++ compiler supporting C++17 (GCC or Clang recommended).

- x86_64 CPU (for ```__rdtscp``` instruction support).

## 💻 How to Build and Run
You can compile the project using g++ directly. Ensure you enable optimizations (```-O3```) and native architecture flags (```-march=native```) to get accurate benchmarking results.

**Using CMake**

```
mkdir build && cd build
cmake ..
make
./benchmark
```

## 📄 Reference
If you use this code, please cite the paper:
```
@misc{cryptoeprint:2025/1342,
      author = {Ke Ma and Jiabo Wang and Shanxiang Lyu and Junzuo Lai and Zsolt Lángi},
      title = {Simultaneous Diophantine Approximation for Compact Discrete Gaussian Sampling},
      howpublished = {Cryptology {ePrint} Archive, Paper 2025/1342},
      year = {2025},
      url = {https://eprint.iacr.org/2025/1342}
}
```