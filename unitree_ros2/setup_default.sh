#!/bin/bash
echo "Setup unitree ros2 environment with default interface"
source /opt/ros/foxy/setup.bash
source /workspace/unitree_ros2/cyclonedds_ws/install/setup.bash
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
