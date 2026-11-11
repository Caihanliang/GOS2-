#ifndef _ROS2_SPORT_CLIENT_
#define _ROS2_SPORT_CLIENT_
#include <iostream>
#include "nlohmann/json.hpp"
#include "unitree_api/msg/request.hpp"

/*
 * Go2 高层运动 API 封装（SportClient）
 *
 * 调用方式概要：
 * 1. 创建 unitree_api::msg::Request 对象
 * 2. 调用 SportClient 的某个方法填充 Request（写入 api_id 和 parameter）
 * 3. 发布到话题：/api/sport/request
 *
 * 对应官方文档：Unitree 运动服务 / sports_services
 * 本文件中的数字（如 1008）就是文档里的 api_id。
 */

#pragma pack(1)
// ---------- API 编号（与官方运动服务文档一一对应）----------
const int32_t ROBOT_SPORT_API_ID_DAMP = 1001;              // 阻尼模式
const int32_t ROBOT_SPORT_API_ID_BALANCESTAND = 1002;      // 平衡站立
const int32_t ROBOT_SPORT_API_ID_STOPMOVE = 1003;          // 停止移动
const int32_t ROBOT_SPORT_API_ID_STANDUP = 1004;           // 站起
const int32_t ROBOT_SPORT_API_ID_STANDDOWN = 1005;         // 趴下
const int32_t ROBOT_SPORT_API_ID_RECOVERYSTAND = 1006;     // 恢复站立
const int32_t ROBOT_SPORT_API_ID_EULER = 1007;             // 机身欧拉角姿态
const int32_t ROBOT_SPORT_API_ID_MOVE = 1008;              // 速度移动 (vx, vy, vyaw)
const int32_t ROBOT_SPORT_API_ID_SIT = 1009;               // 坐下
const int32_t ROBOT_SPORT_API_ID_RISESIT = 1010;           // 从坐下站起
const int32_t ROBOT_SPORT_API_ID_SWITCHGAIT = 1011;        // 切换步态
const int32_t ROBOT_SPORT_API_ID_TRIGGER = 1012;           // 触发
const int32_t ROBOT_SPORT_API_ID_BODYHEIGHT = 1013;        // 设置机身高度
const int32_t ROBOT_SPORT_API_ID_FOOTRAISEHEIGHT = 1014;   // 设置抬腿高度
const int32_t ROBOT_SPORT_API_ID_SPEEDLEVEL = 1015;        // 速度档位
const int32_t ROBOT_SPORT_API_ID_HELLO = 1016;             // 打招呼动作
const int32_t ROBOT_SPORT_API_ID_STRETCH = 1017;           // 伸展
const int32_t ROBOT_SPORT_API_ID_TRAJECTORYFOLLOW = 1018;  // 轨迹跟随
const int32_t ROBOT_SPORT_API_ID_CONTINUOUSGAIT = 1019;    // 连续步态
const int32_t ROBOT_SPORT_API_ID_CONTENT = 1020;           // Content 动作
const int32_t ROBOT_SPORT_API_ID_WALLOW = 1021;            // 打滚
const int32_t ROBOT_SPORT_API_ID_DANCE1 = 1022;            // 舞蹈1
const int32_t ROBOT_SPORT_API_ID_DANCE2 = 1023;            // 舞蹈2
const int32_t ROBOT_SPORT_API_ID_GETBODYHEIGHT = 1024;     // 获取机身高度
const int32_t ROBOT_SPORT_API_ID_GETFOOTRAISEHEIGHT = 1025;// 获取抬腿高度
const int32_t ROBOT_SPORT_API_ID_GETSPEEDLEVEL = 1026;     // 获取速度档位
const int32_t ROBOT_SPORT_API_ID_SWITCHJOYSTICK = 1027;    // 开关手柄控制
const int32_t ROBOT_SPORT_API_ID_POSE = 1028;              // 姿态模式
const int32_t ROBOT_SPORT_API_ID_SCRAPE = 1029;            // 刮地
const int32_t ROBOT_SPORT_API_ID_FRONTFLIP = 1030;         // 前空翻
const int32_t ROBOT_SPORT_API_ID_FRONTJUMP = 1031;         // 前跳
const int32_t ROBOT_SPORT_API_ID_FRONTPOUNCE = 1032;       // 前扑

// 轨迹点（用于 TrajectoryFollow）
typedef struct
{
    float timeFromStart;  // 相对起始时间
    float x;              // 位置 x
    float y;              // 位置 y
    float yaw;            // 航向
    float vx;             // 线速度 x
    float vy;             // 线速度 y
    float vyaw;           // 角速度
} PathPoint;

class SportClient
{
public:
    /** 阻尼模式，api_id=1001 */
    void Damp(unitree_api::msg::Request &req);

    /** 平衡站立，api_id=1002 */
    void BalanceStand(unitree_api::msg::Request &req);

    /** 停止移动，api_id=1003 */
    void StopMove(unitree_api::msg::Request &req);

    /** 站起，api_id=1004 */
    void StandUp(unitree_api::msg::Request &req);

    /** 趴下，api_id=1005 */
    void StandDown(unitree_api::msg::Request &req);

    /** 恢复站立，api_id=1006 */
    void RecoveryStand(unitree_api::msg::Request &req);

    /** 设置机身欧拉角，api_id=1007；参数 roll/pitch/yaw */
    void Euler(unitree_api::msg::Request &req, float roll, float pitch, float yaw);

    /**
     * 速度控制移动，api_id=1008（本项目导航最常用）
     * @param vx   前进速度 (m/s)
     * @param vy   横向速度 (m/s)
     * @param vyaw 转向角速度 (rad/s)
     */
    void Move(unitree_api::msg::Request &req, float vx, float vy, float vyaw);

    /** 坐下，api_id=1009 */
    void Sit(unitree_api::msg::Request &req);

    /** 从坐下恢复，api_id=1010 */
    void RiseSit(unitree_api::msg::Request &req);

    /** 切换步态，api_id=1011；d 为步态编号 */
    void SwitchGait(unitree_api::msg::Request &req, int d);

    /** 触发，api_id=1012 */
    void Trigger(unitree_api::msg::Request &req);

    /** 设置机身高度，api_id=1013 */
    void BodyHeight(unitree_api::msg::Request &req, float height);

    /** 设置抬腿高度，api_id=1014 */
    void FootRaiseHeight(unitree_api::msg::Request &req, float height);

    /** 速度档位，api_id=1015 */
    void SpeedLevel(unitree_api::msg::Request &req, int level);

    /** 打招呼，api_id=1016 */
    void Hello(unitree_api::msg::Request &req);

    /** 伸展，api_id=1017 */
    void Stretch(unitree_api::msg::Request &req);

    /** 轨迹跟随，api_id=1018 */
    void TrajectoryFollow(unitree_api::msg::Request &req, std::vector<PathPoint> &path);

    /** 开关手柄，api_id=1027 */
    void SwitchJoystick(unitree_api::msg::Request &req, bool flag);

    /** 连续步态，api_id=1019 */
    void ContinuousGait(unitree_api::msg::Request &req, bool flag);

    /** 打滚，api_id=1021 */
    void Wallow(unitree_api::msg::Request &req);

    /** Content，api_id=1020 */
    void Content(unitree_api::msg::Request &req);

    /** 姿态模式，api_id=1028 */
    void Pose(unitree_api::msg::Request &req, bool flag);

    /** 刮地，api_id=1029 */
    void Scrape(unitree_api::msg::Request &req);

    /** 前空翻，api_id=1030 */
    void FrontFlip(unitree_api::msg::Request &req);

    /** 前跳，api_id=1031 */
    void FrontJump(unitree_api::msg::Request &req);

    /** 前扑，api_id=1032 */
    void FrontPounce(unitree_api::msg::Request &req);

    /** 舞蹈1，api_id=1022 */
    void Dance1(unitree_api::msg::Request &req);

    /** 舞蹈2，api_id=1023 */
    void Dance2(unitree_api::msg::Request &req);
};

#endif
