#!/bin/bash
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

cd "$ROOT/go2_ros2_ws"
python3 path_planner.py map.yaml
