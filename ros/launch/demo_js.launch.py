from launch import LaunchDescription
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    package_share = FindPackageShare('saw_socket_streamer_ros')
    demo_config = PathJoinSubstitution([package_share, 'share', 'demo_js.json'])

    return LaunchDescription([
        Node(
            package='saw_socket_streamer_ros',
            executable='demo_js',
            name='socket_streamer_demo_js',
            parameters=[{
                'topic': 'demo/measured_js',
            }],
            output='screen',
        ),
        Node(
            package='saw_socket_streamer_ros',
            executable='socket_streamer_ros_bridge',
            name='socket_streamer_ros_bridge',
            arguments=[
                '--json-config', demo_config,
                '--ros-namespace', 'demo',
            ],
            output='screen',
        ),
    ])
