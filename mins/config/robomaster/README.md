# MINS profiles of the RoboMaster S1

Settings of MINS (camera + IMU + wheels) used with a DJI RoboMaster S1 driven by a Raspberry Pi 5.
Each folder is a complete MINS profile (the 11 `config_*.yaml` files that MINS reads from `config.yaml`).
Every file starts with a header (FILE, PURPOSE, ROLE, LOCATION, BUILD, RUN, TEST) and every setting that differs from the
MINS default is commented. Code that records, replays and evaluates is in the project repository
(`corentinlarre-prog/robomaster`, folder `scripts/`).

| Folder (in `mins/config/`) | Robot / sensors | Notes |
|---|---|---|
| `robomaster/` | first robot: robot camera + robot IMU + wheels | gimbal in safe mode on this robot |
| `robomaster_r3/` | Banku (third robot): robot camera + robot IMU + wheels | own camera intrinsics, camera tilt estimated online, mask of a cable in the field of view (`mask_cable.png`) |
| `robomaster_d455/` | Intel D455 camera + D455 IMU, wheels off | D455 on the gimbal |
| `robomaster_d455_wheels/` | D455 on the gimbal + wheels | `T_imu_wheel` valid only while the gimbal is aligned with the chassis |
| `robomaster_d455_chassis/` | D455 mounted upside down on the chassis + wheels | `T_imu_wheel` computed for this mounting (recompute it for every recording); `imu_only_init: true`; `imu_thresh: 0.15` (see the comments of `config_init.yaml`) |
| `robomaster_robimu_d455cam/` | ROBOT IMU (`/imu/data`, 50 Hz) + D455 camera mounted upside down on the chassis + wheels | the `T_imu_cam` in the file is a placeholder: compute the real one and the clock offset for every recording with `scripts/robimu_d455cam_calc.py` of the project repository, then replay with `scripts/d455_replay.py` (`PROFILE=robomaster_robimu_d455cam T_IMU_CAM=...`); the other files are those of `robomaster_r3/` |

## Use

```bash
# copy a profile folder into the MINS workspace, then replay a recording (roscore running)
cp -r mins/config/robomaster_r3 ~/mins_sandbox_ws/src/mins/mins/config/
rosrun mins bag _config_path:=<folder>/config.yaml
```

Check at start-up that MINS prints no `unable to parse` or `invalid boolean` warning: the YAML reader (OpenCV) is
fragile and a comment on the same line as a boolean makes the reading fail without stopping MINS.

## Live mode under ROS 1

`mins/src/run_subscribe.cpp` now has a ROS 1 branch (it used to exit immediately without ROS 2): the node `subscribe` reads the live topics of the profile (IMU, camera, wheels) instead of a bag.
Build with `catkin build mins` (only the `subscribe` executable changes), run with `rosrun mins subscribe _config_path:=<folder>/config.yaml`; see `scripts/live_run.py` in the
robomaster repository (`corentinlarre-prog/robomaster`) for a script that copies a profile, starts the node on the robot or on a bag played in real time, and summarises the trajectory.
At one core and 1.5 GHz the node drops part of the camera updates (the subscriber queue is 10 images) but keeps up with real time.

The tuning values of the D455 profiles are not validated (see the Technical Document, chapter on the D455).
