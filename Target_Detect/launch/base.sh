#!/bin/bash
set -e
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PYTHON=python3

mkdir -p "$ROOT/Target_Detect/img/pred" \
         "$ROOT/Target_Detect/img/true" \
         "$ROOT/Target_Detect/img/false"

# 目标检测监控（需准备 Target_Detect/model/best.pt）
$PYTHON "$ROOT/Target_Detect/monitor_detect_target.py"

# 有真机视频流时再启用：
# $PYTHON "$ROOT/go2_ros2_ws/display_video.py"
