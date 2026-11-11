# GO2-Patrol-Nav 复现操作手册（Docker + ROS 2 Foxy）

> 适用场景：宿主机为 **Ubuntu 24.04 + ROS 2 Jazzy**（或其它非 20.04 环境），通过 Docker 运行官方推荐的 **Ubuntu 20.04 / ROS 2 Foxy** 开发环境。  
> 项目路径以本文作者机器为例：`/home/holly/GO2-Patrol-Nav`。其他人请替换为自己的实际路径。  
> 整理日期：2026-11-10

---

## 0. 重要说明

### 0.1 为什么用 Docker

本项目文档要求：

- 操作系统：Ubuntu **20.04**
- ROS 2：**Foxy**
- Python：约 **3.8**

若宿主机是 22.04/24.04（Humble/Jazzy），直接在宿主机复现容易不兼容。  
推荐：宿主机只负责 Docker / 浏览器，**在 Foxy 容器内编译和运行 ROS 相关功能**。

### 0.2 本仓库没有“一键业务镜像”

- 可拉取的是基座镜像：`althack/ros2:foxy-full`
- 需本地用 `unitree_ros2/.devcontainer/Dockerfile-foxy` 构建开发镜像
- 代码与 `colcon` 编译产物在**挂载的项目目录**里，不自动打进镜像
- 别人复现：需要 **代码 + 构建镜像 + 容器内编译**，不是只 `docker pull` 就能完事

### 0.3 zsh 与 bash（必看）

开发镜像默认进入 **zsh**。ROS/colcon 环境脚本必须和 shell 匹配：

| 当前 Shell | 应使用 |
|---|---|
| zsh（容器默认） | `source .../setup.zsh` |
| bash | `source .../setup.bash` |

在 zsh 里 `source setup.bash` 会报类似错误：

```text
no such file or directory: .../cyclonedds_ws/setup.sh
not found: ".../cyclonedds_ws/local_setup.bash"
```

这不是编译坏了，是 shell 不匹配。

### 0.4 有无真机

| 目标 | 无真机 | 有真机 |
|---|---|---|
| 编译环境 | ✅ | ✅ |
| 路径规划 / 仿真 | ✅ | ✅ |
| 前端控制台页面 | ✅（可打开） | ✅ |
| SLAM 建图 / 真机巡检 | ❌ | ✅ |
| 控制台按钮完整联动 | 基本不可用 | 需再改脚本与终端启动方式 |

---

## 1. 宿主机准备

```bash
# 启动 Docker
sudo systemctl start docker
sudo systemctl enable docker

# 当前用户加入 docker 组（一次即可，然后重新登录或 newgrp）
sudo usermod -aG docker $USER
newgrp docker

docker version
```

图形相关（可选；纯编译可不做）：

```bash
# 若在图形桌面会话中
echo $DISPLAY          # 应类似 :0
xhost +local:docker    # 允许容器显示 GUI
```

> 若 `DISPLAY` 为空，RViz / 部分 GUI 打不开，但不影响编译。路径规划 GUI 也可能受影响。

---

## 2. 构建 Foxy 开发镜像

```bash
# 拉取基座
docker pull althack/ros2:foxy-full

# 构建本仓库开发镜像
cd /home/holly/GO2-Patrol-Nav/unitree_ros2/.devcontainer
docker build -t go2-foxy-dev:latest -f Dockerfile-foxy .
```

---

## 3. 启动并进入容器（挂载整个项目）

建议用下面命令创建容器（首次）：

```bash
xhost +local:docker 2>/dev/null || true

docker run -it \
  --name go2-foxy-dev \
  --privileged \
  --network host \
  -e DISPLAY=$DISPLAY \
  -e QT_X11_NO_MITSHM=1 \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  -v "$HOME/.Xauthority:/root/.Xauthority:rw" \
  -v /home/holly/GO2-Patrol-Nav:/workspace:rw \
  -w /workspace \
  go2-foxy-dev:latest zsh
```

之后日常：

```bash
docker start go2-foxy-dev
docker exec -it go2-foxy-dev zsh
```

另开一个进同一容器的终端：

```bash
docker exec -it go2-foxy-dev zsh
```

容器内项目根目录：`/workspace`（对应宿主机 `GO2-Patrol-Nav`）。

---

## 4. 容器内安装依赖（通常只需一次）

```bash
source /opt/ros/foxy/setup.zsh

apt update
apt install -y \
  ros-foxy-navigation2 \
  ros-foxy-nav2-bringup \
  ros-foxy-slam-toolbox \
  ros-foxy-pcl-ros \
  ros-foxy-tf-transformations \
  ros-foxy-cv-bridge \
  ros-foxy-rmw-cyclonedds-cpp \
  ros-foxy-rosidl-generator-dds-idl \
  python3-colcon-common-extensions \
  python3-rosdep \
  python3-pip \
  python3-opencv \
  libopencv-dev \
  build-essential cmake git wget

rosdep init || true
rosdep update
```

Python 依赖：

```bash
pip3 install -U pip
pip3 install torch==1.12.1 torchvision==0.13.1 \
  --index-url https://download.pytorch.org/whl/cpu
pip3 install opencv-python==4.5.5.64 numpy scipy pyyaml \
  aiortc aiohttp watchdog matplotlib transforms3d flask flask-cors

cd /workspace/go2_webrtc_connect
pip3 install -e .
```

---

## 5. 修改硬编码路径（通常只需一次）

作者机器路径为 `/home/lxc/...`，必须改成容器内路径。

```bash
cd /workspace
PROJECT_PATH=/workspace
PYTHON_PATH=$(which python3)

find go2_launch_sh -name "*.sh" -exec sed -i "s|~/go2_ros2_ws|$PROJECT_PATH/go2_ros2_ws|g" {} \;
find go2_launch_sh -name "*.sh" -exec sed -i "s|/home/lxc/go2_ros2_ws|$PROJECT_PATH/go2_ros2_ws|g" {} \;
find go2_launch_sh -name "*.sh" -exec sed -i "s|~/Target_Detect|$PROJECT_PATH/Target_Detect|g" {} \;
find go2_launch_sh -name "*.sh" -exec sed -i "s|/home/lxc/anaconda3/envs/video/bin/python|$PYTHON_PATH|g" {} \;

sed -i "s|/home/lxc/anaconda3/envs/target_detect/bin/python|$PYTHON_PATH|g" Target_Detect/launch/base.sh || true
sed -i "s|/home/lxc/anaconda3/envs/video/bin/python|$PYTHON_PATH|g" Target_Detect/launch/base.sh || true
sed -i "s|/home/lxc/Target_Detect|$PROJECT_PATH/Target_Detect|g" Target_Detect/launch/base.sh || true
sed -i "s|/home/lxc/go2_ros2_ws|$PROJECT_PATH/go2_ros2_ws|g" Target_Detect/launch/base.sh || true

# 真机 IP，按实际修改；无真机可先保持默认
ROBOT_IP=192.168.123.161
sed -i "s/ip=\"192.168.123.161\"/ip=\"$ROBOT_IP\"/g" go2_ros2_ws/path_runner.py

mkdir -p Target_Detect/img/{pred,true,false}
mkdir -p Target_Detect/model
mkdir -p go2_ros2_ws/maps logs
```

修改 Unitree setup 脚本中的工作空间路径：

```bash
cd /workspace/unitree_ros2
sed -i 's|$HOME/unitree_ros2|/workspace/unitree_ros2|g' setup.sh setup_local.sh setup_default.sh
```

> 注意：`setup_*.sh` 内部仍是 `source .../setup.bash`。在 **zsh** 下请不要直接 `source setup_local.sh`，改用下文第 7 节的 zsh 写法。

---

## 6. 编译 Unitree ROS2（cyclonedds_ws）

### 6.1 编译 cyclonedds（不要先 source ROS）

若当前 shell 已加载 ROS（`echo $ROS_DISTRO` 有输出），先清理：

```bash
unset ROS_DISTRO ROS_VERSION ROS_PYTHON_VERSION AMENT_PREFIX_PATH COLCON_PREFIX_PATH CMAKE_PREFIX_PATH
unset PYTHONPATH ROS_PACKAGE_PATH ROS_ROOT ROS_ETC_DIR
echo $ROS_DISTRO   # 应为空
```

或新开干净 shell：

```bash
# 在宿主机执行
docker exec -it go2-foxy-dev env -i \
  HOME=/root TERM=xterm \
  PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
  LD_LIBRARY_PATH=/opt/ros/foxy/lib \
  bash --noprofile --norc
```

然后编译：

```bash
cd /workspace/unitree_ros2/cyclonedds_ws
# 仓库中通常已有 src/cyclonedds、src/rmw_cyclonedds、src/unitree
export LD_LIBRARY_PATH=/opt/ros/foxy/lib
colcon build --packages-select cyclonedds
```

### 6.2 编译其余 Unitree 包（现在需要 source ROS）

```bash
source /opt/ros/foxy/setup.zsh
cd /workspace/unitree_ros2/cyclonedds_ws
colcon build
source install/setup.zsh

ros2 pkg list | grep -E 'unitree|cyclonedds'
# 期望看到：unitree_api unitree_go unitree_hg rmw_cyclonedds_cpp 等
```

---

## 7. 无真机时的环境加载方式（替代 setup_local.sh）

在 **zsh** 中每次新终端建议执行：

```bash
source /opt/ros/foxy/setup.zsh
source /workspace/unitree_ros2/cyclonedds_ws/install/setup.zsh
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces>
  <NetworkInterface name="lo" priority="default" multicast="default" />
</Interfaces></General></Domain></CycloneDDS>'
```

若坚持用 bash：

```bash
bash
source /workspace/unitree_ros2/setup_local.sh
```

---

## 8. 编译 go2_ros2_ws（不需要真机）

```bash
source /opt/ros/foxy/setup.zsh
source /workspace/unitree_ros2/cyclonedds_ws/install/setup.zsh
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

cd /workspace/go2_ros2_ws
rosdep install --from-paths src --ignore-src -r -y --skip-keys="cyclonedds"
colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.zsh

ros2 pkg list | grep go2
# 期望看到：go2_core go2_slam go2_navigation go2_perception 等
```

---

## 9. 无真机可运行的功能

### 9.1 路径规划

```bash
source /opt/ros/foxy/setup.zsh
source /workspace/unitree_ros2/cyclonedds_ws/install/setup.zsh
source /workspace/go2_ros2_ws/install/setup.zsh

cd /workspace/go2_ros2_ws
python3 path_planner.py map.yaml
```

操作：左键拖点 → 空格确认 → 右键撤销 → `Q` 保存（生成/更新 `commands.yaml`）。

### 9.2 路径仿真

```bash
cd /workspace/go2_ros2_ws
python3 path_simulate.py
```

### 9.3 前端控制台页面（可只看 UI）

在**宿主机**终端（或容器，因 `--network host` 端口互通）：

```bash
cd /home/holly/GO2-Patrol-Nav/go2_launch_sh
# 容器内则：cd /workspace/go2_launch_sh
python3 server.py
```

浏览器打开：

```text
http://127.0.0.1:5000/
```

说明：

- 页面文件来自本仓库 `go2_launch_sh/dashboard.html`
- 这是简单控制面板（按钮 + 识别截图），**不是** Foxglove 那种地图/摄像头/IMU 综合可视化页
- 按钮会调用 `gnome-terminal` 启动脚本；在 Docker 里常卡住，状态会一直显示「运行中...」
- 卡住时：`Ctrl+C` 停掉 `server.py` 再启动，浏览器强制刷新（Ctrl+Shift+R）
- 无真机时，建议**只用来看页面**，实际功能用 9.1 / 9.2 命令行

---

## 10. 有真机时补充步骤（概要）

1. VMware 网卡用**桥接**，电脑有线网卡静态 IP：如 `192.168.123.99/24`
2. 机器人默认 IP：`192.168.123.161`
3. `ping 192.168.123.161`
4. 修改 `unitree_ros2/setup.sh` 中网卡名，并使用：

```bash
# bash 下
source /workspace/unitree_ros2/setup.sh
# 或 zsh 下手动 source setup.zsh + 设置 CYCLONEDDS_URI 网卡名
```

5. `ros2 topic list` 应能看到机器人话题  
6. 再进行 SLAM / `path_runner.py` 真机巡检

SLAM 示例：

```bash
# 终端1
source ...（真机环境）
cd /workspace
./go2_launch_sh/1_slam.sh

# 终端2
rviz2 -d /workspace/go2_ros2_ws/src/go2_ros2_toolbox/go2_slam/config/slam.rviz
```

---

## 11. 日常最短命令清单

```bash
# 宿主机：进容器
docker start go2-foxy-dev
docker exec -it go2-foxy-dev zsh

# 容器内：加载环境
source /opt/ros/foxy/setup.zsh
source /workspace/unitree_ros2/cyclonedds_ws/install/setup.zsh
source /workspace/go2_ros2_ws/install/setup.zsh
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# 无真机主流程
cd /workspace/go2_ros2_ws
python3 path_planner.py map.yaml
python3 path_simulate.py
```

看前端：

```bash
cd /home/holly/GO2-Patrol-Nav/go2_launch_sh
python3 server.py
# 浏览器: http://127.0.0.1:5000/
```

---

## 12. 常见问题速查

| 现象 | 原因 / 处理 |
|---|---|
| `source setup.bash` 报找不到 `cyclonedds_ws/setup.sh` | 你在 zsh 里，改用 `setup.zsh` |
| `echo $ROS_DISTRO` 清不掉 | `exec zsh -f` 不会清空环境变量；用 `unset ROS_DISTRO ...` |
| 编译 cyclonedds 失败 | 编译前不要 source ROS |
| 控制台按钮全是「运行中...」且点不了 | 状态卡死；重启 `server.py` 并强刷浏览器 |
| 打开 `http://IP:5000/` 曾 404 | 旧版 `server.py` 不托管 HTML；现已增加 `/` 路由返回 `dashboard.html` |
| 别人那种地图+摄像头监控页 | 多半是 Foxglove 等工具，不是本仓库 `dashboard.html` |
| 目标检测 | 需要 `Target_Detect/model/best.pt`，仓库可能未自带 |
| `DISPLAY` 为空 | GUI/RViz 不可用；在图形桌面会话操作，或接受无 GUI |

---

## 13. 复现进度自检清单

- [ ] Docker 可用，镜像 `go2-foxy-dev:latest` 已构建
- [ ] 容器可进入，`/workspace` 指向项目
- [ ] 依赖已安装（apt + pip）
- [ ] 硬编码路径已修改
- [ ] `cyclonedds_ws` 编译成功，`ros2 pkg list` 能看到 unitree 包
- [ ] `go2_ros2_ws` 编译成功，`ros2 pkg list` 能看到 go2 包
- [ ] 无真机：`path_planner.py` / `path_simulate.py` 可跑
- [ ] 前端：`server.py` + 浏览器可打开控制面板
- [ ] （可选）真机网络通，`ros2 topic list` 有机器人话题

---

## 14. 相关文件

| 文件 | 作用 |
|---|---|
| `unitree_ros2/.devcontainer/Dockerfile-foxy` | Foxy 开发镜像构建 |
| `unitree_ros2/setup_local.sh` | 无真机环境（bash） |
| `unitree_ros2/setup.sh` | 真机环境（需改网卡名） |
| `go2_ros2_ws/` | 核心 ROS2 工作空间 |
| `go2_launch_sh/dashboard.html` | Web 控制面板页面 |
| `go2_launch_sh/server.py` | 控制面板后端（端口 5000） |
| `README.md` / `DEPLOYMENT.md` | 官方说明（面向 20.04 宿主机直装） |

---

**文档版本**：基于本机 Docker 复现实践整理  
**推荐无真机验证路径**：构建镜像 → 进容器 → 编译 → `path_planner.py` → `path_simulate.py` →（可选）打开控制台页面
