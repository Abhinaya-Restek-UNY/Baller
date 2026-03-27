from launch import LaunchDescription
from launch_ros.actions import Node

import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    bringup_dir = get_package_share_directory('baller_bringup')
    ekf_config_path = os.path.join(bringup_dir, 'config', 'ekf.yaml')

    # 1. Serial Node
    serial_node = Node(
        package="baller_serial",
        executable="baller_serial_node",
        output="screen",
        parameters=[{
            "serial_port": "/dev/ttyUSB0", # Added leading slash
            "baud_rate": 115200,
            "track_width_mm": 440.0,
            "wheel_diameter_mm": 55.0,
            "encoder_ppr": 400.0
        }]
    )

    # 2. Static TF (Connects the robot center to the IMU)
    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        arguments=['0', '0', '0', '0', '0', '0', 'base_link', 'imu_link']
    )

    # 3. EKF Node
    ekf_node = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[ekf_config_path]
    )

    return LaunchDescription([
        serial_node,
        static_tf,
        ekf_node
    ])
