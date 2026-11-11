#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

/*
 * Go2 H264 视频流转发节点
 * 从组播 UDP 拉 H264 流，解码后发布为 ROS Image：/camera/image/raw
 */
class H264Repub : public rclcpp::Node {
public:
    H264Repub() : Node("h264_repub") {
        // 发布解码后的相机图像
        image_publisher_ = this->create_publisher<sensor_msgs::msg::Image>("/camera/image/raw", 10);

        // 组播网卡名（按实际修改，常见 eth0）
        std::string multicast_iface = "eth0";
        this->declare_parameter("multicast_iface", multicast_iface);
        this->get_parameter("multicast_iface", multicast_iface);

        // GStreamer：组播收 H264 → 解码 → BGR
        std::string gstreamer_str =
            "udpsrc address=230.1.1.1 port=1720 multicast-iface=" + multicast_iface +
            " ! application/x-rtp, media=video, encoding-name=H264 ! rtph264depay ! h264parse"
            " ! avdec_h264 ! videoconvert ! video/x-raw,width=1280,height=720,format=BGR ! appsink drop=1";
        cap.open(gstreamer_str, cv::CAP_GSTREAMER);

        if (!cap.isOpened()) {
            RCLCPP_ERROR(this->get_logger(), "无法打开视频流（请检查网卡名与组播地址）");
        } else {
            while (rclcpp::ok()) {
                cv::Mat frame;
                if (cap.read(frame)) {
                    auto ros_image = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toImageMsg();
                    ros_image->header.frame_id = "camera";
                    ros_image->header.stamp = this->get_clock()->now();
                    image_publisher_->publish(*ros_image);
                }
                cv::waitKey(10);
            }
        }
    }

private:
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_publisher_;
    cv::VideoCapture cap;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<H264Repub>();
    rclcpp::shutdown();
    return 0;
}
