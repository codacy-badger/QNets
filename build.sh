#!/bin/sh

. ./config.sh
mkdir -p build && cd build
cmake -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" -DUSER_CXX_FLAGS="${CXX_FLAGS}" -DUSE_COVERAGE="${USE_COVERAGE}" -DUSE_OPENMP="${USE_OPENMP}" -DGSL_ROOT_DIR="${GSL_ROOT}" ..

if [ "$1" = "" ]; then
  make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null)
else
  make -j$1
fi
