/*
 * MINS: Efficient and Robust Multisensor-aided Inertial Navigation System
 * Copyright (C) 2019-2026 Woosik Lee
 * Copyright (C) 2019-2026 Guoquan Huang
 * Copyright (C) 2019-2026 MINS Contributors
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <memory>

#if ROS_AVAILABLE == 2
#include "core/ROS2Publisher.h"
#include "core/ROS2Subscriber.h"
#include "rclcpp/rclcpp.hpp"
#elif ROS_AVAILABLE == 1
#include "core/ROSPublisher.h"
#include "core/ROSSubscriber.h"
#include <ros/ros.h>
#endif
#include "core/SystemManager.h"
#include "options/Options.h"
#include "options/OptionsSystem.h"
#include "utils/Print_Logger.h"
#include "utils/State_Logger.h"
#include "utils/TimeChecker.h"
#include "utils/colors.h"
#include "utils/opencv_yaml_parse.h"

using namespace std;
using namespace mins;

// Main function
int main(int argc, char **argv) {

  // Ensure we have a path, if the user passes it then we should use it
  string config_path = "unset_path_to_config.yaml";
  argc > 1 ? config_path = argv[1] : string();

  // Launch our ros node
#if ROS_AVAILABLE == 2
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;

  options.allow_undeclared_parameters(true);
  options.automatically_declare_parameters_from_overrides(true);
  auto node = std::make_shared<rclcpp::Node>("mins_subscribe", options);
  node->get_parameter<std::string>("config_path", config_path);
#elif ROS_AVAILABLE == 1
  // ROS 1 branch (added 2026-11-09): same node as run_bag.cpp but the measurements come from live topics (ROSSubscriber) instead of a bag
  ros::init(argc, argv, "mins_subscribe");
  auto nh = make_shared<ros::NodeHandle>("~");
  nh->param<string>("config_path", config_path, config_path); // _config_path:=... or argv[1]
#endif
  std::cout << "Loading config from path: " << config_path << std::endl;
  // Load the config
  auto parser = make_shared<ov_core::YamlParser>(config_path);
#if ROS_AVAILABLE == 2
  parser->set_node(node);
#elif ROS_AVAILABLE == 1
  parser->set_node_handler(nh);
#endif
  shared_ptr<Options> op = make_shared<Options>();
  op->load_print(parser);
  shared_ptr<State_Logger> save = make_shared<State_Logger>(op);

  // Create our system
  shared_ptr<SystemManager> sys = make_shared<SystemManager>(op->est);
#if ROS_AVAILABLE == 2
  shared_ptr<ROS2Publisher> pub = make_shared<ROS2Publisher>(node, sys, op);
  shared_ptr<ROS2Subscriber> sub = make_shared<ROS2Subscriber>(node, sys, pub);
#elif ROS_AVAILABLE == 1
  shared_ptr<ROSPublisher> pub = make_shared<ROSPublisher>(nh, sys, op);
  shared_ptr<ROSSubscriber> sub = make_shared<ROSSubscriber>(nh, sys, pub);
#endif

  // Ensure we read in all parameters required
  if (!parser->successful()) {
    PRINT4(RED "unable to parse all parameters, please fix\n" RESET);
    exit(EXIT_FAILURE);
  }

#if ROS_AVAILABLE == 2
  // rclcpp::spin(node);
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();
  // Spin off to ROS
#elif ROS_AVAILABLE == 1
  // single-threaded spin: the callbacks (IMU, camera, wheels) are then run one after the other, so the filter is never fed from two threads
  ros::spin();
#endif

  // Final visualization
  sys->visualize_final();
  op->sys->save_timing ? save->save_timing_to_file(sys->tc_sensors->get_total_sum()) : void();
  save->check_files();

#if ROS_AVAILABLE == 2
  rclcpp::shutdown();
#elif ROS_AVAILABLE == 1
  ros::shutdown();
#endif
  // Done!
  return EXIT_SUCCESS;
}
