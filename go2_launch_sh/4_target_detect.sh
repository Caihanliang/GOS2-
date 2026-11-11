#!/bin/bash
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

cd "$ROOT/Target_Detect"
bash launch/base.sh
