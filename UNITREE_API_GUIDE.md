# Unitree 接口文档怎么调用（结合本项目）

本文用本仓库真实代码，说明 `unitree_ros2` / 运动服务 API **怎么被调用**，而不是只罗列文档名词。

---

## 1. 先分清两条控狗通道

本项目里其实有 **两套** 控制方式：

| 通道 | 用在哪 | 怎么发指令 |
|---|---|---|
| **A. ROS2 + unitree_ros2** | SLAM/导航/`go2_base`/`go2_sport_api` | 发 ROS 话题 `/api/sport/request` |
| **B. WebRTC** | `path_runner.py`、`display_video.py` | 走 WebRTC DataChannel，不经过 unitree_ros2 话题 |

你看的「接口文档」（运动服务 api_id、SportModeState 等）主要对应 **通道 A**。  
`path_runner.py` 是通道 B，api_id 名字类似，但传输层不同。

```text
通道 A（ROS2）:
  Nav2 /cmd_vel  →  go2_base / vel_ctrl_repub
                 →  SportClient.Move() 填 Request
                 →  发布 /api/sport/request
                 →  机器人执行

通道 B（WebRTC）:
  path_runner.py →  conn.datachannel.pub_sub.publish_request_new(...)
                 →  机器人执行
```

---

## 2. 官方接口文档对应到代码的什么

官方「运动服务」文档里每个动作有一个 **api_id**，例如：

- `1008` = Move（速度移动）
- `1003` = StopMove
- `1004` = StandUp
- …

本项目把这些编号写在：

`go2_ros2_ws/src/go2_ros2_toolbox/unitree_pkgs/go2_sport_api/include/common/ros2_sport_client.h`

封装类是 `SportClient`：你调用 `Move/StandUp/...`，它帮你写好：

1. `req.header.identity.api_id = 某个编号`
2. 需要参数时，把 JSON 写进 `req.parameter`

消息类型来自 `unitree_ros2` 包：

- `unitree_api/msg/Request`：你发给机器人的控制请求
- `unitree_go/msg/SportModeState`：机器人回传的高层状态

---

## 3. 一次完整调用长什么样（最重要）

以 **Move（前进/转向）** 为例，项目里真实写法在 `go2_base.cpp`：

```cpp
// 1) 准备空请求
unitree_api::msg::Request req;

// 2) 用 SportClient 按文档 api_id=1008 填好请求
//    内部会设置：
//    req.header.identity.api_id = 1008
//    req.parameter = {"x": vx, "y": vy, "z": vyaw}
sport_client_->Move(req, msg->linear.x, msg->linear.y, msg->angular.z);

// 3) 发布到官方约定话题
sport_pub_->publish(req);   // 话题: /api/sport/request
```

`SportClient::Move` 实现（简化理解）：

```cpp
void SportClient::Move(Request &req, float vx, float vy, float vyaw) {
    nlohmann::json js;
    js["x"] = vx;
    js["y"] = vy;
    js["z"] = vyaw;
    req.parameter = js.dump();                 // 文档里的参数 JSON
    req.header.identity.api_id = 1008;         // 文档里的 api_id
}
```

所以：**读文档 = 查 api_id + 参数字段；写代码 = SportClient 填 Request + publish。**

---

## 4. 读状态怎么调用

文档里的高层状态，对应订阅：

```text
/sportmodestate
或
/lf/sportmodestate   # 低频
```

消息类型：`unitree_go/msg/SportModeState`  
里面有：`position`、`velocity`、`imu_state`、`mode`、`gait_type` 等。

有真机时验证：

```bash
source /opt/ros/foxy/setup.zsh
source /workspace/unitree_ros2/cyclonedds_ws/install/setup.zsh
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
# 真机还要设好网卡 CYCLONEDDS_URI

ros2 topic list
ros2 topic echo /sportmodestate
```

官方例程：`unitree_ros2/example/.../read_motion_state`、`go2_sport_client`。

---

## 5. 本项目里谁在调用这些接口

| 文件 | 做什么 |
|---|---|
| `go2_core/src/go2_base.cpp` | 订阅 `/cmd_vel`，转成 `Move/StopMove`，发 `/api/sport/request`；并把位姿转成 `/odom` 与 TF |
| `go2_sport_api/.../vel_ctrl_repub.cpp` | 订阅 `/joy`，同样 `Move` 后发 `/api/sport/request` |
| `ros2_sport_client.h/.cpp` | 接口文档的代码封装（api_id 表） |
| `path_runner.py` | **不是**这条 ROS 接口，是 WebRTC 版 Move |

导航链路（通道 A）：

```text
Nav2 发 /cmd_vel
    → go2_base 收到
    → SportClient.Move(...)
    → /api/sport/request
    → 狗走路
```

巡检脚本链路（通道 B）：

```text
path_planner 生成 commands.yaml
    → path_runner 读 YAML
    → WebRTC SPORT_MOD + api_id=Move
    → 狗走路
```

---

## 6. 你自己写一个调用（模板）

```cpp
#include "common/ros2_sport_client.h"
#include "unitree_api/msg/request.hpp"

// 假设已有 publisher: pub
unitree_api::msg::Request req;
SportClient client;

// 站起
client.StandUp(req);
pub->publish(req);

// 前进 0.3 m/s，同时轻微转向
client.Move(req, 0.3f, 0.0f, 0.1f);
pub->publish(req);

// 停止
client.StopMove(req);
pub->publish(req);
```

Python/WebRTC 对照（`path_runner.py`）：

```python
await conn.datachannel.pub_sub.publish_request_new(
    RTC_TOPIC["SPORT_MOD"],
    {"api_id": SPORT_CMD["Move"], "parameter": {"x": vx, "y": vy, "z": vyaw}},
)
```

两边都是：**api_id + parameter**，只是一个走 ROS Topic，一个走 WebRTC。

---

## 7. 文档应该怎么读（实用顺序）

1. `unitree_ros2/README _zh.md`：环境、话题名、状态字段  
2. 官方运动服务页：每个 api_id 的含义和参数  
   https://support.unitree.com/home/zh/developer/sports_services  
3. 对照本仓库 `ros2_sport_client.h`：编号和函数名  
4. 对照 `go2_base.cpp`：项目里真实怎么发  

---

## 8. 无真机时你能练什么

- 能读懂：api_id、Request、SportClient、`/api/sport/request`  
- 能跑：路径规划/仿真（不依赖这套真机接口）  
- 不能验证：`ros2 topic echo /sportmodestate` 和真机动作  

有真机后，最小验证：

1. 网络通 + source unitree 环境  
2. `ros2 topic echo /sportmodestate`  
3. 运行官方 `go2_sport_client` 或启动本项目 `go2_base`，看 `/api/sport/request` 是否有数据  

---

**一句话**：接口文档里的 api_id，在本项目里被 `SportClient` 填进 `unitree_api/msg/Request`，再发布到 `/api/sport/request`；`go2_base` 把导航的 `/cmd_vel` 转成这个调用。
