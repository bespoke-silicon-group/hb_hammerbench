#!/bin/bash
set -euo pipefail
N=${1:-32}
NITER=${2:-1}
PATTERN=${3:-0}
VARIANT=${4:-1}
DIR=$(cd "$(dirname "$0")" && pwd)
pushd "$DIR" >/dev/null
make -f Makefile.host all
if [ "$VARIANT" = "simple" ]; then
  echo "Running simple baseline..."
  ./host_baseline $N $NITER $PATTERN
else
  echo "Running optimized baseline variant $VARIANT..."
  ./host_baseline_opt $N $NITER $PATTERN $VARIANT
fi
popd >/dev/null
