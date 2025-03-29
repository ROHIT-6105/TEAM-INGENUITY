#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist.hpp"  // Changed from Float32 to Twist for movement control

class DistancePublisher : public rclcpp::Node {
public:
    DistancePublisher() : Node("distance_publisher") {
        // Subscribe to the /scan topic
        subscription_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
            "/scan", 10, std::bind(&DistancePublisher::scan_callback, this, std::placeholders::_1));
        
        // Changed to publish movement commands instead of distance
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
    }

private:
    void scan_callback(const sensor_msgs::msg::LaserScan::SharedPtr msg) {
        auto movement_msg = geometry_msgs::msg::Twist();
        
        // Analyze laser scan data
        size_t center_index = msg->ranges.size() / 2;
        float front_distance = msg->ranges[center_index];
        float left_distance = msg->ranges[center_index * 1.5];  // Left side
        float right_distance = msg->ranges[center_index / 2];    // Right side

        // Simple obstacle avoidance logic
        if (front_distance > 1.0) {  // If path is clear ahead
            movement_msg.linear.x = 0.3;  // Move forward
        } 
        else if (left_distance > right_distance) {  // If more space on left
            movement_msg.angular.z = 0.5;  // Turn left
        } 
        else {  // Otherwise turn right
            movement_msg.angular.z = -0.5;  // Turn right
        }

        // Publish the movement command
        publisher_->publish(movement_msg);
        RCLCPP_INFO(this->get_logger(), "Front: %.2f m, Left: %.2f m, Right: %.2f m", 
                   front_distance, left_distance, right_distance);
    }

    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr subscription_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;  // Changed to Twist
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DistancePublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}