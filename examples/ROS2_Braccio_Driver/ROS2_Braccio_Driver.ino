/**
 * ROS driver for controlling the Arduino Braccio++ with ROS2.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Braccio++.h>

#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <sensor_msgs/msg/joint_state.h>

#include <rosidl_runtime_c/string_functions.h>

#include <vector>
#include <algorithm>

/**************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************/

rcl_publisher_t joint_state_publisher;
sensor_msgs__msg__JointState joint_state_msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

/**************************************************************************************
 * DEFINES
 **************************************************************************************/

#define LED_PIN 13

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/

void setup()
{
  set_microros_transports();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  

  if (!Braccio.begin()) {
    error_loop();
  }
  Braccio.disengage();

  allocator = rcl_get_default_allocator();

  /* Create init_options. */
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  /* Create node. */
  RCCHECK(rclc_node_init_default(&node, "braccio_plusplus_node", "", &support));

  /* Create JointState publisher. */
  RCCHECK(rclc_publisher_init_default(
    &joint_state_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState),
    "/braccio_joint_state_actual"));

  /* Initialize JointState message. */
  rosidl_runtime_c__String__init  (&joint_state_msg.header.frame_id);
  rosidl_runtime_c__String__assign(&joint_state_msg.header.frame_id, "base");
    
  {
    static rosidl_runtime_c__String joint_state_msg_name[SmartServoClass::NUM_MOTORS];
    joint_state_msg.name.data = joint_state_msg_name;
    joint_state_msg.name.size = SmartServoClass::NUM_MOTORS;
    joint_state_msg.name.capacity = SmartServoClass::NUM_MOTORS;
    std::vector<std::string> const JOINT_NAMES = {"base", "shoulder", "elbow", "wrist_roll", "wrist_pitch", "pinch"};
    int i = 0;
    std::for_each(std::cbegin(JOINT_NAMES),
                  std::cend  (JOINT_NAMES),
                  [&joint_state_msg, &i](std::string const & joint_name)
                  {
                    rosidl_runtime_c__String__init  (&joint_state_msg.name.data[i]);
                    rosidl_runtime_c__String__assign(&joint_state_msg.name.data[i], joint_name.c_str());
                    i++;
                  });
  }

  static double joint_state_msg_position[SmartServoClass::NUM_MOTORS] = {0};
  joint_state_msg.position.data = joint_state_msg_position;
  joint_state_msg.position.size = SmartServoClass::NUM_MOTORS;
  joint_state_msg.position.capacity = SmartServoClass::NUM_MOTORS;

  /* Create Timer. */
  const unsigned int timer_timeout = 100;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  /* Create executor. */
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
}

void loop()
{
  delay(100);
  RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}

/**************************************************************************************
 * FUNCTIONS
 **************************************************************************************/

void error_loop()
{
  for (;;)
  {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(100);
  }
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (timer != NULL)
  {
    /* Retrieve current servo positions. */
    float current_servo_pos[SmartServoClass::NUM_MOTORS] = {0};
    Braccio.positions(current_servo_pos);
    /* Revert the array to fit with the names within the joint state message. */
    std::reverse(current_servo_pos, current_servo_pos + SmartServoClass::NUM_MOTORS);

    /* Populate header. */
    joint_state_msg.header.stamp.sec = millis() / 1000;
    joint_state_msg.header.stamp.nanosec = micros() * 1000;

    /* Populate data. */
    for (int i = 0; i < SmartServoClass::NUM_MOTORS; i++)
      joint_state_msg.position.data[i] = current_servo_pos[i];

    /* Publish message. */
    RCSOFTCHECK(rcl_publish(&joint_state_publisher, &joint_state_msg, NULL));
  }
}
