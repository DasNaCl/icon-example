
# Introduction
This repository is a fork of the [icon-example](https://gitlab.dkrz.de/skosukhin/icon-example/) project.
It contains example algorithms that are typical for weather and climate modelling on unstructured grids (e.g. for ICON model).
The aim of this repository is to demonstrate AnyDSL's heterogeneity.

# Requirements
The following software is assumed to be installed:

- Either clang++ or g++ supporting C++17
- [GNU Make](https://www.gnu.org/software/make/)
- [CMake](https://gitlab.kitware.com/cmake/cmake/) (VERSION>=3.4.3)
- [curl](https://curl.haxx.se/) (the cmake script downloads grid files)
- [AnyDSL](https://github.com/AnyDSL/anydsl)

Clone this repository inside the `anydsl/` folder.

# Building and Running

```bash
mkdir path/to/anydsl/iconex/build
cd "$_"  # the "$_" contains the argument to the previous command
cmake -DBACKEND=cpu \
      -DAnyDSL_runtime_DIR="$_"/../../runtime/build/share/anydsl/cmake/
make check
```

Supported backends:

- `cpu`
- `avx`
- `nvvm`
- `cuda`
- `amdgpu`

Note that for `cpu` and `avx` backends, you may choose a sensible number of threads via `-DNO_OF_THREADS=$(nproc)`.

# Benchmarks

Run the benchmarks using `make check`.
This will *always* run the `cpu` backend up to `$(nproc)` threads.
You may add more backends by specifying the `ICONEX_BENCH` environment variable, it's contents will be run after the cpu backend.
Suppose you want to run the benchmark script for `cpu`, `avx`, and `cuda`, the order of commands would look as follows:
```bash
ICONEX_BENCH="avx cuda" make check
```
The `CMakeLists.txt` is setup in a way to source this variable automagically according to the chosen backend.
For example, if you built for `avx` and want to bench this, it suffices to call `make check` without any environment variable tampering.

TODO: Insert graph. :-)

