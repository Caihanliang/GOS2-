#!/bin/bash
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

source /opt/ros/foxy/setup.bash
source "$ROOT/unitree_ros2/cyclonedds_ws/install/setup.bash"
source "$ROOT/go2_ros2_ws/install/setup.bash"
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

cd "$ROOT/go2_ros2_ws"
ros2 launch go2_core go2_startup.launch.py
