# sawSocketStreamer ROS

This package provides a ROS2 bridge executable for sawSocketStreamer.
It accepts a regular sawSocketStreamer JSON configuration file, creates
an `mtsSocketStreamer` component with its `Required` interface, and asks
`mts_ros_crtk_bridge_required` to populate the matching ROS CRTK
provided interface.

Only CRTK-compatible commands and events are bridged.  Other entries in
the sawSocketStreamer JSON file are ignored by the CRTK bridge.

Example:

```sh
ros2 run saw_socket_streamer_ros socket_streamer_ros_bridge \
  --ros-args -- \
  --json-config streamer.json \
  --ros-namespace PSM1
```

## Example

Terminal 1, start the demo ROS2 node publishing `demo/measured_js`:

```sh
ros2 run saw_socket_streamer_ros demo_js
```

Terminal 2, start the ROS2 to UDP/JSON bridge:

```sh
ros2 run saw_socket_streamer_ros socket_streamer_ros_bridge --ros-args -- \
  --json-config $(ros2 pkg prefix saw_socket_streamer_ros)/share/saw_socket_streamer_ros/share/demo_js.json \
  --ros-namespace demo
```

Terminal 3, display the received UDP/JSON `measured_js`:

```sh
nc -lu 127.0.0.1 48054
```

The demo node and bridge can also be started together:

```sh
ros2 launch saw_socket_streamer_ros demo_js.launch.py
```
