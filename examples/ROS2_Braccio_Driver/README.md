:floppydisk: `ROS2_Braccio_Driver`
==================================
#### How-to-install
* Install [ROS2 Galactic](https://docs.ros.org/en/galactic/Installation/Ubuntu-Install-Debians.html)
* Install [colcon](https://colcon.readthedocs.io/en/released/user/installation.html)
* Install/Setup [Micro-ROS](https://micro.ros.org/docs/tutorials/core/teensy_with_arduino)

#### Compile and Upload
```bash
arduino-cli compile -b arduino:mbed_nano:nanorp2040connect -v examples/ROS2_Braccio_Driver
arduino-cli upload -b arduino:mbed_nano:nanorp2040connect -v examples/ROS2_Braccio_Driver -p /dev/ttyACM0
```
#### Run Micro-ROS Agent
```bash
cd microros_ws
source /opt/ros/galactic/setup.bash
source install/local_setup.bash
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0
```
#### View available ROS topics
```bash
source /opt/ros/galactic/setup.bash
ros2 topic list
  /braccio_joint_state_actual
  /parameter_events
  /rosout
```
#### View published JointStates
```bash
source /opt/ros/galactic/setup.bash
ros2 topic echo /braccio_joint_state_actual
...
    header:
    stamp:
        sec: 44
        nanosec: 1567475040
    frame_id: base
    name: [base, shoulder, elbow, wrist_roll, wrist_pitch, pinch]
    position: [90.32624816894531, 156.71249389648438, 156.3975067138672, 157.34249877929688, 24.885000228881836, 153.7987518310547]
    velocity: []
    effort: []
...
```
