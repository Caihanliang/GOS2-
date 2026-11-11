#!/bin/bash
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

cd "$ROOT/go2_ros2_ws"

# 无真机：仿真执行
python3 path_simulate.py

# 有真机时再取消下一行注释（并确认 path_runner.py 中 IP 正确）
# python3 path_runner.py
