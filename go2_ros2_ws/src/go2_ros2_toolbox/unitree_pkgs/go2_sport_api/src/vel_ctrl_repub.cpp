#include "rclcpp/rclcpp.hpp"
#include <iostream>

#include "common/ros2_sport_client.h"
#include "unitree_api/msg/request.hpp"

#include "unitree_go/msg/sport_mode_state.hpp"
#include <sensor_msgs/msg/joy.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>

/*
 * 手柄速度 → Unitree 运动 API 转发节点
 *
 * 订阅：/joy（手柄）
 * 发布：/api/sport/request（unitree_api/msg/Request）
 *
 * 核心调用：
 *   sport_req.Move(req, vx, vy, vyaw);
 *   req_puber->publish(req);
 */

rclcpp::Publisher<unitree_api::msg::Request>::SharedPtr req_puber;
rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_suber;

unitree_api::msg::Request req;
SportClient sport_req;
bool new_cmd = false;

float vx;
float vyaw;
float vy;
int count_down = 0;

rclcpp::Node::SharedPtr nh;

bool checkObstacle = true;
float joySpeed = 0;
float joySpeedRaw = 0;
float joySpeedYaw = 0;
float joySpeedLateral = 0;

float PI = 3.141592653589397;
float maxSpeedYaw = 1.4;       // 最大转向速度
float maxSpeedLateral = 0.5;   // 最大横向速度

// 手柄回调：把摇杆轴映射成 vx / vy / vyaw
void joystickHandler(const sensor_msgs::msg::Joy::ConstSharedPtr joy)
{
  joySpeedRaw = joy->axes[4];
  joySpeed = joySpeedRaw;
  joySpeedLateral = joy->axes[3] * maxSpeedLateral;
  joySpeedYaw = joy->axes[0] * maxSpeedYaw;

  if (joySpeed > 1.0) joySpeed = 1.0;
  if (joySpeed < -1.0) joySpeed = -1.0;
  if (joySpeedLateral > maxSpeedLateral) joySpeedLateral = maxSpeedLateral;
  if (joySpeedLateral < -maxSpeedLateral) joySpeedLateral = -maxSpeedLateral;

  if (joy->axes[4] == 0) joySpeed = 0;

  // 轴5：是否启用避障检查（保留字段，当前主循环未深度使用）
  if (joy->axes[5] > -0.1) {
    checkObstacle = true;
  } else {
    checkObstacle = false;
  }

  new_cmd = true;
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    nh = rclcpp::Node::make_shared("vel_cmd_repub");

    // 订阅手柄
    joy_suber = nh->create_subscription<sensor_msgs::msg::Joy>("/joy", 5, joystickHandler);
    // 发布到 Unitree 高层运动接口
    req_puber = nh->create_publisher<unitree_api::msg::Request>("/api/sport/request", 10);

    rclcpp::Rate rate(100);  // 100 Hz 发送
    bool status = rclcpp::ok();
    while (status) {
        rclcpp::spin_some(nh);
        vx = joySpeed;
        vyaw = joySpeedYaw;
        vy = joySpeedLateral;
        std::cout << "vx: " << vx << ", vy: " << vy << ", vyaw: " << vyaw << std::endl;

        // 填充并发布 Move 请求（api_id=1008）
        sport_req.Move(req, vx, vy, vyaw);
        req_puber->publish(req);

        rate.sleep();
        status = rclcpp::ok();
    }

    rclcpp::shutdown();
    return 0;
}
