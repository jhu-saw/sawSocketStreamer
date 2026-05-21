#!/usr/bin/env python3

import math

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState


class DemoJS(Node):

    def __init__(self):
        super().__init__('socket_streamer_demo_js')
        self.declare_parameter('topic', 'demo/measured_js')
        self.declare_parameter('joint_name', 'joint_1')
        self.declare_parameter('amplitude', 1.0)
        self.declare_parameter('frequency', 0.25)
        self.declare_parameter('period', 0.01)

        self._topic = self.get_parameter('topic').value
        self._joint_name = self.get_parameter('joint_name').value
        self._amplitude = self.get_parameter('amplitude').value
        self._frequency = self.get_parameter('frequency').value
        period = self.get_parameter('period').value

        self._publisher = self.create_publisher(JointState, self._topic, 10)
        self._start = self.get_clock().now()
        self.create_timer(period, self.publish_measured_js)

        self.get_logger().info(f'publishing {self._topic}')

    def publish_measured_js(self):
        now = self.get_clock().now()
        elapsed = (now - self._start).nanoseconds * 1.0e-9
        position = self._amplitude * math.sin(2.0 * math.pi * self._frequency * elapsed)

        message = JointState()
        message.header.stamp = now.to_msg()
        message.name = [self._joint_name]
        message.position = [position]

        self._publisher.publish(message)


def main(args=None):
    rclpy.init(args=args)
    node = DemoJS()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
