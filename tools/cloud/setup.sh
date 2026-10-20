#!/bin/bash
# Cloud-session setup for cambc builds (run first in every new cloud container; safe to re-run).
# The cambc CLI's own downloader rejects the cloud proxy's certificate (UnknownIssuer), so this
# fetches its toolchain with curl into the cache layout the CLI expects (results/cloud-parity.md).
set -euo pipefail
pip install --break-system-packages -q cambc==3.0.1 2>/dev/null || pip install -q cambc==3.0.1
C="$HOME/.cache/cambc"; mkdir -p "$C"; cd "$C"
if [ ! -x wasi-sdk-33/bin/clang ]; then
  curl -fsSL -o wasi.tgz https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-33/wasi-sdk-33.0-x86_64-linux.tar.gz
  mkdir -p wasi-sdk-33 && tar -xzf wasi.tgz -C wasi-sdk-33 --strip-components=1 && rm wasi.tgz
fi
if [ ! -x cmake-3.31.8-linux-x86_64/bin/cmake ]; then
  curl -fsSL -o cmake.tgz https://github.com/Kitware/CMake/releases/download/v3.31.8/cmake-3.31.8-linux-x86_64.tar.gz
  tar -xzf cmake.tgz && rm cmake.tgz
fi
if [ ! -x ninja-1.12.1-linux-x86_64/ninja ]; then
  curl -fsSL -o ninja.zip https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-linux.zip
  mkdir -p ninja-1.12.1-linux-x86_64 && python3 -m zipfile -e ninja.zip ninja-1.12.1-linux-x86_64/ && rm ninja.zip
  chmod +x ninja-1.12.1-linux-x86_64/ninja
fi
echo "cambc $(cambc --version 2>/dev/null | awk '{print $2}'), toolchain ready in $C"
