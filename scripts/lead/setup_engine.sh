#!/bin/bash
# setup_engine.sh: cambc workspaces for engine 1.0.3 (cambc 3.0.5, primary) and 1.0.2 (cambc 3.0.4, history) in a claude.ai sandbox.
# Tested 23 Oct 2026 07:52-08:00 BST. The sandbox reaches 2026a.battlecode.co.uk with no settings change; github.com
# answers through the proxy, but cambc's own downloader rejects the proxy certificate, so the three toolchain archives
# come down with curl (which trusts /root/.ccr/ca-bundle.crt) and go into cambc's cache under the names it probes.
set -eu
export SSL_CERT_FILE=/root/.ccr/ca-bundle.crt CAMBC_NO_UPDATE_CHECK=1
H=${HOME_DIR:-/home/claude}
C=/root/.cache/cambc
mkdir -p $H/dl $C

pip install --break-system-packages -q cambc==3.0.4          # engine 1.0.2 -> /usr/local/bin/cambc
python3 -m venv $H/v305 && $H/v305/bin/pip install -q cambc==3.0.5   # engine 1.0.3 -> $H/v305/bin/cambc

fetch() { [ -s "$H/dl/$2" ] || curl -sS -L --max-time 600 -o "$H/dl/$2" "$1"; }
fetch https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-33/wasi-sdk-33.0-x86_64-linux.tar.gz wasi.tgz
fetch https://github.com/Kitware/CMake/releases/download/v3.31.8/cmake-3.31.8-linux-x86_64.tar.gz cmake.tgz
fetch https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-linux.zip ninja.zip
echo "630615d8e98ac33eba7fbe472626dff5c899c85af3c024585ae109166a6909d0  $H/dl/cmake.tgz" | sha256sum -c -
echo "6f98805688d19672bd699fbbfa2c2cf0fc054ac3df1f0e6a47664d963d530255  $H/dl/ninja.zip" | sha256sum -c -
mkdir -p $C/wasi-sdk-33 $C/cmake-3.31.8-linux-x86_64 $C/ninja-1.12.1-linux-x86_64
[ -x $C/wasi-sdk-33/bin/clang ] || tar -xzf $H/dl/wasi.tgz -C $C/wasi-sdk-33 --strip-components=1
[ -x $C/cmake-3.31.8-linux-x86_64/bin/cmake ] || tar -xzf $H/dl/cmake.tgz -C $C/cmake-3.31.8-linux-x86_64 --strip-components=1
[ -x $C/ninja-1.12.1-linux-x86_64/ninja ] || (cd $C/ninja-1.12.1-linux-x86_64 && unzip -q -o $H/dl/ninja.zip && chmod +x ninja)

mkdir -p $H/ws305 $H/ws
(cd $H/ws305 && $H/v305/bin/cambc init . --yes)   # SDK 3.0.5 and the 40-map pool from the platform
(cd $H/ws && cambc init . --yes)                  # SDK 3.0.4

# smoke test: one release game per engine; the summary's match.engineVersion names the engine
(cd $H/ws305 && $H/v305/bin/cambc run starter-cpp starter-cpp --map 00ambush --release --statistics /tmp/e103.json --json | tail -3)
(cd $H/ws && cambc run starter-cpp starter-cpp --map 00ambush --release --statistics /tmp/e102.json --json | tail -3)
python3 -c "import json;[print(p, json.load(open(p))['match']['engineVersion']) for p in ('/tmp/e103.json','/tmp/e102.json')]"
# Bots: copy random2/bots/<name> into <ws>/bots/<name>; build with: cambc build <name> --release
