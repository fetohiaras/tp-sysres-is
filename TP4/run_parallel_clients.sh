#!/usr/bin/env bash
# Usage: ./run_clients_simple.sh <N>, default 5

set -e
N=${1:-5}
BIN=./build/ipc_client

tmpdir="$(mktemp -d)"
pids=()

for i in $(seq 1 "$N"); do
  echo "executed client #$i"
  (
    "$BIN" >"$tmpdir/out_$i.txt" 2>&1
  ) &
  pids+=($!)
done

for pid in "${pids[@]}"; do
  wait "$pid"
done

for i in $(seq 1 "$N"); do
  echo "---------- output of client #$i -------------"
  cat "$tmpdir/out_$i.txt"
done

echo "removing $tmpdir"
rm -rf "$tmpdir"
