#!/usr/bin/env python3

import json
import socket

import rclpy
from rclpy.node import Node
from rosidl_runtime_py.convert import message_to_ordereddict
from rosidl_runtime_py.set_message import set_message_fields
from sensor_msgs.msg import JointState


class MeasuredJSServoJPBridge(Node):

    def __init__(self):
        super().__init__('measured_js_servo_jp')

        self.declare_parameter('measured_js_topic', 'measured_js')
        self.declare_parameter('servo_jp_topic', 'servo_jp')
        self.declare_parameter('udp_send_ip', '127.0.0.1')
        self.declare_parameter('udp_send_port', 48054)
        self.declare_parameter('udp_listen_ip', '0.0.0.0')
        self.declare_parameter('udp_listen_port', 48055)
        self.declare_parameter('period', 0.001)

        measured_js_topic = self.get_parameter('measured_js_topic').value
        servo_jp_topic = self.get_parameter('servo_jp_topic').value
        self._udp_destination = (
            self.get_parameter('udp_send_ip').value,
            self.get_parameter('udp_send_port').value,
        )

        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._socket.bind((
            self.get_parameter('udp_listen_ip').value,
            self.get_parameter('udp_listen_port').value,
        ))
        self._socket.setblocking(False)

        self._servo_jp = self.create_publisher(JointState, servo_jp_topic, 10)
        self.create_subscription(JointState, measured_js_topic, self.measured_js_cb, 10)
        self.create_timer(self.get_parameter('period').value, self.receive_udp)

        self.get_logger().info(
            f'{measured_js_topic} -> UDP {self._udp_destination[0]}:{self._udp_destination[1]}')
        self.get_logger().info(
            f'UDP {self._socket.getsockname()[0]}:{self._socket.getsockname()[1]} -> {servo_jp_topic}')

    def measured_js_cb(self, msg):
        ros = message_to_ordereddict(msg)
        payload = {
            'measured_js': {
                'Position': ros['position'],
            }
        }
        data = json.dumps(payload, separators=(',', ':')).encode('utf-8')
        self._socket.sendto(data, self._udp_destination)

    def receive_udp(self):
        while True:
            try:
                data, _ = self._socket.recvfrom(65535)
            except BlockingIOError:
                return

            try:
                payload = json.loads(data.decode('utf-8'))
                servo_jp = payload['servo_jp']
            except (UnicodeDecodeError, json.JSONDecodeError, KeyError, TypeError) as exc:
                self.get_logger().warning(f'ignoring UDP packet: {exc}')
                continue

            msg = JointState()
            position = [float(value) for value in servo_jp.get('Goal', servo_jp.get('position', []))]
            fields = {
                'header': {
                    'stamp': self.get_clock().now().to_msg(),
                },
                'position': position,
            }
            set_message_fields(msg, fields)

            self._servo_jp.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    node = MeasuredJSServoJPBridge()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
