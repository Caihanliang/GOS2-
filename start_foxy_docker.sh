#!/bin/bash
set -e
xhost +local:docker >/dev/null 2>&1 || true

CONTAINER_NAME=go2-foxy-dev
IMAGE=go2-foxy-dev:latest
PROJECT=/home/holly/GO2-Patrol-Nav

# 若已存在则直接进入
if docker ps -a --format '{{.Names}}' | grep -qx "$CONTAINER_NAME"; then
  if [ "$(docker inspect -f '{{.State.Running}}' $CONTAINER_NAME)" != "true" ]; then
    docker start "$CONTAINER_NAME"
  fi
  docker exec -it "$CONTAINER_NAME" zsh
  exit 0
fi

docker run -it \
  --name "$CONTAINER_NAME" \
  --privileged \
  --network host \
  -e DISPLAY=$DISPLAY \
  -e QT_X11_NO_MITSHM=1 \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  -v "$HOME/.Xauthority:/root/.Xauthority:rw" \
  -v "$PROJECT:/workspace:rw" \
  -w /workspace \
  "$IMAGE" zsh
