#!/usr/bin/env bash
# Build an integration candidate from v2a plus each listed builder's source diff, in order.
# usage: bash scripts/integrate.sh <out-bot> <builder> [builder ...]
#   e.g. bash scripts/integrate.sh v3a w10 w13 w12
# Each builder's change is taken as `diff -ruN v2a/src <builder>/src` (salt.h excluded) and applied with `patch -p1`.
# A rejected hunk stops the script with the .rej files listed, so conflicts are resolved by hand, never silently.
set -euo pipefail
cd "$(dirname "$0")/.."
out=$1; shift
[ -e "bots/$out" ] && { echo "bots/$out exists; remove it first"; exit 1; }
mkdir -p patches "bots/$out"
cp -r bots/v2a/src "bots/$out/src"
rm -f "bots/$out/src/salt.h"
for b in "$@"; do
  (cd bots && diff -ruN -x salt.h -x '*.orig' -x '*.rej' v2a/src "$b/src") > "patches/$b.patch" || true
  echo "== $b: $(grep -c '^+++ ' "patches/$b.patch") files"
  if ! (cd "bots/$out" && patch -p1 --forward --no-backup-if-mismatch < "../../patches/$b.patch"); then
    echo "CONFLICT applying $b; rejects:"; find "bots/$out/src" -name '*.rej'; exit 2
  fi
done
CAMBC="${CAMBC:-$(command -v cambc || echo "$APPDATA/Python/Python314/Scripts/cambc.exe")}"
"$CAMBC" build --release "$out"
echo "built bots/$out from v2a + $*"
