# Epson Robot Client Library for Real-time Motion Control

[日本語](./readme_ja.md) / [English](./readme.md)

## 1. Introduction to the Client Library for Real-time Motion Control

This client library is for using the real-time motion control function in an Epson robot controller. You can use the following functions and create an application that uses path planning or trajectory planning calculated in an external device.

- Turn on the motor from an external device
- Control the robot using motion control values based on a path or trajectory planned in an external device

![](./docs/overview.png)

<sup>\*</sup> Epson does not provide the items listed below. You must prepare these items.

- External device: The external device to control the robot.
- Application: The application to control the robot.
- Ethernet cable: The Ethernet cable to connect the external device to the RC800-A.

When using this feature with ROS2, refer to the following README as well.

https://github.com/Epson-Robots/epson-robot-ros2.git

### 1.1 Precautions

- Epson does not provide a function to plan paths or trajectories. You must prepare and develop this yourself.
- Real-time motion control is an advanced function that operates the robot according to motion control values based on a path or trajectory calculated outside the Epson robot controller. Make sure you fully understand path planning/trajectory planning before using.

## 2. Required Equipment

- Epson industrial robot

    For the robots that can be used, see [Supported Models](#21-supported-models).
- Epson robot controller

    Real-time motion control can be used with the RC800 controller (Ver. 8.1.1.0 or higher). Purchase and enable the "Real-time Motion Control" controller option in advance. For details, refer to the following manual.  
    "Epson RC+ 8.0 User's Guide"
- Windows PC and USB cable

    Install Epson RC+ 8.0 and configure the settings related to real-time motion control in the controller.

    Connect this PC to the controller with the USB cable. The USB Type-B connector is located on the controller side for connecting to the PC.
- Epson RC+ 8.0 (Ver. 8.1.1.0 or higher)

    Epson RC+ 8.0 is a Windows application that provides an integrated development environment for Epson robots. You must configure the controller settings to use real-time motion control.
- Linux PC and Ethernet cable

    This equipment is required to use the client library. For the recommended environment, see [Recommended Environment for the Linux PC](#22-recommended-environment-for-the-linux-pc).

    You must connect this PC to the controller with the Ethernet cable.
- Other equipment required to use the industrial robot

    Make sure to connect the safeguard and emergency stop button to the controller.

### 2.1 Supported Models

Real-time motion control is available for the following models. The feature is available if the following version requirements are met.

- ROS2 package: 1.1.0 or higher
- Client library: 1.1.0 or higher
- RC800 controller: 8.1.2.0 or higher

For models marked with <sup>*</sup> in the list below, the feature is available if the following version requirements are met.

- ROS2 package: 1.0.0 or higher
- Client library: 1.0.0 or higher
- RC800 controller: 8.1.1.0 or higher

**SCARA robot**

|Series|Model|
|--|--|
|GX4 Series|GX4-C251S<sup>\*</sup>, GX4-C251C<br>GX4-C301S<sup>\*</sup>, GX4-C301SM, GX4-C301C, GX4-C301CM <br>GX4-C351S<sup>\*</sup>, GX4-C351SM, GX4-C351C, GX4-C351CM<br>GX4-C351S-L<sup>\*</sup>, GX4-C351C-L, GX4-C351S-R<sup>\*</sup>, GX4-C351C-R|
|GX8 Series|GX8-C452S, GX8-C452SR, GX8-C452C, GX8-C452CR<br>GX8-C453S, GX8-C453SR, GX8-C453C, GX8-C453CR<br>GX8-C552S, GX8-C552SR, GX8-C552C, GX8-C552CR<br>GX8-C553S, GX8-C553SR, GX8-C553C, GX8-C553CR<br>GX8-C652S, GX8-C652SR, GX8-C652C, GX8-C652CR<br>GX8-C653S, GX8-C653SR, GX8-C653C, GX8-C653CR|
|RS Series|RS4-C351S, RS4-C351C<br>RS6-C552S, RS6-C552C|

**6-axis robot**

|Series|Model|
|--|--|
|C8 Series|C8-C901S<sup>\*</sup>, C8-C901SR, C8-C901C, C8-C901CR<br>C8-C1401S<sup>\*</sup>, C8-C1401SR, C8-C1401C, C8-C1401CR|
|C12 Series|C12-C1401S<sup>\*</sup>, C12-C1401C|
|CX4 Series|CX4-A601S, CX4-A601SR, CX4-A601C, CX4-A601CR|
|CX7 Series|CX7-A701S, CX7-A701SR, CX7-A701C, CX7-A701CR<br>CX7-A901S, CX7-A901SR, CX7-A901C, CX7-A901CR|

### 2.2 Recommended Environment for the Linux PC

The recommended environment for the Linux PC that uses the client library is listed below. Operation is not guaranteed on all PCs.

- OS: Ubuntu22.04LTS
- CPU: Intel Core i7 or higher
- Memory: 16 GB or more

## 3. Building the Environment on the Linux PC to Use the Client Library

The client library requires Fast DDS 2.14.4 as a dependent library. Follow the steps below to install Fast DDS 2.14.4 and the client library.

For details on Fast DDS, refer to the following website:  
https://fast-dds.docs.eprosima.com/en/v2.14.4/index.html

1. Install the packages required to build the client library's dependent libraries.

    ```shell-session
    sudo apt install cmake g++ python3-pip wget git python3-colcon-common-extensions
    pip3 install -U vcstool
    ```

2. Install Asio.

    ```shell-session
    sudo apt install libasio-dev
    ```

3. Install TinyXML2.

    ```shell-session
    sudo apt install libtinyxml2-dev
    ```

4. Install OpenSSL.

    ```shell-session
    sudo apt install libssl-dev
    ```

5. Download the repository file used to install Fast DDS and its dependencies.

    ```shell-session
    mkdir ~/FastDDS
    cd FastDDS
    wget https://raw.githubusercontent.com/eProsima/Fast-DDS/2.14.4/fastrtps.repos
    mkdir src
    vcs import src < fastrtps.repos
    ```

6. Specify the foonathan_memory version.

   Specify the foonathan_memory version as shown below in CMakeLists.txt in the ~/FastDDS/src/foonathan_memory_vendor folder.
    ```shell-session
    find_package(foonathan_memory 1.3.1 QUIET)
    ```

7. Build Fast DDS.

    ```shell-session
    colcon build --cmake-args -DBUILD_SHARED_LIBS=OFF -DSECURITY=ON -DCMAKE_C_FLAGS=-fPIC -DCMAKE_CXX_FLAGS=-fPIC
    ```

8. Clone the repository in your home directory with git clone.

    ```shell-session
    git clone https://github.com/Epson-Robots/rtmc-client-library.git
    ```

9.  Build the client library.

    Specify the path used up to this point and link Fast DDS. If you created a directory or changed the install path when Fast DDS was built, you must change CMakeList.txt for this package.

    ```shell-session
    cd rtmc-client-library
    mkdir build
    cd build
    cmake ..
    make
    ```

## 4. Setting Up the Controller

Before you use the client library on the Linux PC, configure the settings related to real-time motion control in the controller.

For details on the configuration instructions, refer to the following manual:  
"Epson RC+ 8.0 User's Guide - Real-time Motion Control"

## 5. Using the Client Library

### 5.1 When Creating the Application

Include the following header files in the Linux PC application. These header files are included in the client library. Create the application on the Linux PC.

- include/rtmc/rtmc_client.h: This header file defines the API available in the client library
- include/rtmc/rtmc_definition.h: This header file defines values available as API arguments, values acquired with the API, and API return values as constants

### 5.2 When Running the Application

Follow the steps below for the Linux PC. Complete [Setting Up the Controller](#4-setting-up-the-controller) before these steps.

1. Set the IP address of the Linux PC to be on the same network as the controller.
2. Connect the Linux PC to the controller with the Ethernet cable.
3. Turn on the controller.
4. Wait until only the "AUTO" LED on the controller is flashing.

The controller is now able to accept requests from the Linux PC application. The API documented in [API List](#53-api-list) will function when [Request Acceptance Conditions] are satisfied.

### 5.3 API List

This section describes the API available in the client library. For the API return values, see [API Return Values](#54-api-return-values).

Set LD_LIBRARY_PATH and other environment variables so that the application can load the client library.

#### RTMC_Connect

This function starts communications for real-time motion control with the controller. The password is authenticated when communications are established.

When the controller's IP address or port number for built in messaging was changed, specify the destination robot controller by calling [Set_ComProperty](#set_comproperty) before you call this function.

[Syntax]

```shell-session
short RTMC_Connect(std::string password)
```

[Arguments]

- IN
    - password: Password for authentication
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Communications not established|
|Operation Mode |-|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

- Password authentication failed. Ensure that the password is correct.
- The controller option is disabled. Enable the controller option.

#### RTMC_DisConnect

This function disconnects communications for real-time motion control with the controller.

[Syntax]

```shell-session
short RTMC_DisConnect()
```

[Arguments]

None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |-|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

Communications with the controller were disconnected in an abnormal manner. Restart the controller.

#### RBCheck

This function checks if the robot model the Linux PC application is attempting to control is the same as the robot model set in the controller. The result of the check with this function is retained until the controller is turned off.

When this function is called successfully, the controller can accept requests by the following functions. For details of each function, see the corresponding item.

- [MotorOn](#motoron)
- [MotorOff](#motoroff)
- [PowerHigh](#powerhigh)
- [PowerLow](#powerlow)
- [Set_RTMCModeEnable](#set_rtmcmodeenable)
- [Set_RTMCModeDisable](#set_rtmcmodedisable)
- [Exec_RTMC](#exec_rtmc) <sup>\*</sup>
- [Set_Weight](#set_weight)
- [Set_Inertia](#set_inertia)

<sup>\*</sup> To enable the robot controller to accept requests with [Exec_RTMC](#exec_rtmc), call [Set_RTMCModeEnable](#set_rtmcmodeenable) first to enable the control mode for real-time motion control.

[Syntax]

```shell-session
short RBCheck(std::string rb_model)
```

[Arguments]

- IN
    - rb_model: The robot model the Linux PC application is attempting to control

        For the robot models, see [Supported Models](#21-supported-models).
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

The robot model specified in rb_model does not match the robot model set in the controller. Ensure that the robot model specified in rb_model is correct.

#### RBCheckResult

This function checks the result of the RBCheck function retained in the controller.

[Syntax]

```shell-session
short RBCheckResult()
```

[Arguments]

None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

The result of the matching robot model check retained in the controller is a mismatch.

#### Get_RBModel

This function gets the robot model set in the controller.

[Syntax]

```shell-session
short Get_RBModel(std::string* prb_model)
```

[Arguments]

- IN
    - None
- OUT
    - prb_model: The robot model name acquired from the controller

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|

#### Set_RTMCSendFormat

This function specifies the target to control with [Exec_RTMC](#exec_rtmc). This is called the motion control type. There are two types of motion control types:

- Pattern 1: Robot
- Pattern 2: Robot + standard I/O outputs (2 bytes)

The set motion control type is retained until the controller is turned off. The default motion control type is pattern 1.

[Syntax]

```shell-session
short Set_RTMCSendFormat(unsigned short sendformat)
```

[Arguments]

- IN
    - sendformat: The motion control type to set

        Specify one of the following values for the target to control with the Linux PC application.

        - Pattern 1: 0
        - Pattern 2: 1
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

Remote I/O is assigned to standard I/O. Change any standard I/O to which remote I/O was assigned to be not used. For details, refer to the following manual:  
"Epson RC+ 8.0 User's Guide - Setting Real-time Motion Control in Advance"

#### Get_RTMCSendFormat

This function gets the motion control type set in the controller. For the motion control type, see [Set_RTMCSendFormat](#set_rtmcsendformat).

[Syntax]

```shell-session
short Get_RTMCSendFormat(unsigned short* psendformat)
```

[Arguments]

- IN
    - None
- OUT
    - psendformat: The acquired motion control type

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|

#### Set_RTMCRecvFormat

This function specifies the format of the data to get as the result of [Exec_RTMC](#exec_rtmc). This is called the current value type. There are two types of current value types:

- Pattern 1: Robot
- Pattern 2: Robot + standard I/O inputs (3 bytes)

The set current value type is retained until the controller is turned off. The default current value type is pattern 1.

[Syntax]

```shell-session
short Set_RTMCRecvFormat(unsigned short recvformat)
```

[Arguments]

- IN
    - recvformat: The current value type to set

        Specify one of the following values for the target to control with the Linux PC application.

        - Pattern 1: 0
        - Pattern 2: 1
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_RTMCRecvFormat

This function gets the current value type set in the controller. For the current value type, see [Set_RTMCRecvFormat](#set_rtmcrecvformat) function.

[Syntax]

```shell-session
short Get_RTMCRecvFormat(unsigned short* precvformat)
```

[Arguments]

- IN
    - None
- OUT
    - precvformat: The acquired current value type

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|

#### Set_Weight

This function specifies the end effector weight for optimizing parameters related to robot motion. Set the end effector weight to an appropriate value with this function before you control the robot with [Exec_RTMC](#exec_rtmc).

This set value is retained in the controller even when the power is turned off.

[Syntax]

```shell-session
short Set_Weight(double set_weight)
```

[Arguments]

- IN
    - set_weight: End effector weight (unit: kg)
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|

#### Get_Weight

This function gets the set value for the end effector weight set in the controller.

[Syntax]

```shell-session
short Get_Weight(double* pget_weight)
```

[Arguments]

- IN
    - None
- OUT
    - pget_weight: The acquired end effector weight (unit: kg)

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|

#### Set_Inertia

This function specifies the load inertia including the end effector and workpiece and the eccentricity for optimizing parameters related to robot motion. Set the load inertia including the end effector and workpiece and the eccentricity to appropriate values with this function before you control the robot with [Exec_RTMC](#exec_rtmc).

This set value is retained in the controller even when the power is turned off.

[Syntax]

```shell-session
short Set_Inertia(double set_inertia,unsigned short set_eccentricity)
```

[Arguments]

- IN
    - set_inertia: Load inertia (unit: kgm^2)
    - set_eccentricity: Eccentricity (unit: mm, optional)
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|

#### Get_Inertia

This function gets the set values for the load inertia including the end effector and workpiece and the eccentricity set in the controller.

[Syntax]

```shell-session
short Get_Inertia(double* pget_inertia, unsigned short* pget_eccentricity)
```

[Arguments]

- IN
    - None
- OUT
    - pget_inertia: The acquired load inertia (unit: kgm^2)
    - pget_eccentricity: The acquired eccentricity (unit: mm, optional)

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop status |-|
|Controller error |-|

#### MotorON

This function turns on the motors in the robot connected to the controller.

[Syntax]

```shell-session
short MotorOn()
```

[Arguments]

- IN
    - None
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard| Close |
|Emergency Stop |Off|
|Controller error |None|

#### MotorOff

This function turns off the motors in the robot connected to the controller.

[Syntax]

```shell-session
short MotorOff()
```

[Arguments]

- IN
    - None
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard| Close |
|Emergency Stop |Off|
|Controller error |None|

#### Get_MotorStatus

This function gets the status of the motors in the robot connected to the controller.

[Syntax]

```shell-session
short Get_MotorStatus(bool* pmotor_stauts)
```

[Arguments]

- IN
    - None
- OUT
    - pmotor_statu: The acquired status of the motors
        - Motors turned on: true
        - Motors turned off: false

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### PowerHigh

This function sets the robot to high power mode. When this function is called successfully, high-speed robot motion is possible.

[Syntax]

```shell-session
short PowerHigh()
```

[Arguments]

- IN
    - None
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### PowerLow

This function sets the robot to low power mode. When this function is called successfully, the speed of robot motion is limited.

The power mode when the motors are turned on is low power mode.

[Syntax]

```shell-session
short PowerLow()
```

[Arguments]

- IN
    - None
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_PowerMode

This function gets the current power mode of the motors.

[Syntax]

```shell-session
short Get_PowerMode(bool* ppower_mode)
```

[Arguments]

- IN
    - None
- OUT
    - ppower_mode: The acquired power mode
        - High power mode: true
        - Low power mode: false

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Set_RTMCModeEnable

This function enables the control mode for real-time motion control. When this function is called successfully, the robot can be controlled with real-time motion control using [Exec_RTMC](#exec_rtmc).

The set control mode for real-time motion control is retained in the controller until one of the following statuses:

- An error occurred in the controller
- The controller was put in the emergency stop status
- The safeguard connected to the controller was activated
- Communications were disconnected
- [Set_RTMCModeDisable](#set_rtmcmodedisable) was called successfully

When any of the above conditions are satisfied, the control mode for real-time motion control is automatically disabled.

[Syntax]

```shell-session
short Set_RTMCModeEnable()
```

[Arguments]

- IN
    - None
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |-|
|Safeguard| Close |
|Emergency Stop |Off|
|Controller error |None|


[Causes of CLIENT_ERR Failures]

- The motors are turned off. Turn on the motors and call the function again.
- The robot is decelerating to a stop. Call the function again.

#### Set_RTMCModeDisable

This function disables the control mode for real-time motion control.

[Syntax]

```shell-session
short Set_RTMCModeDisable()
```

[Arguments]

- IN
    - None
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |-|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_RTMCMode

This function gets the current control mode for real-time motion control.

[Syntax]

```shell-session
short Get_RTMCMode(bool* prtmcmode)
```

[Arguments]

- IN
    - None
- OUT
    - prtmcmode: The acquired control mode for real-time motion control
        - Enabled: true
        - Disabled: false

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |-|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_CurrentJA

This function gets the current values of the robot joints.

[Syntax]

```shell-session
short Get_CurrentJA(std::vector<double>& pcurja)
```

[Arguments]

- IN
    - None
- OUT
    - pcurja: The acquired current values of the robot joints (unit: rad or mm)

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |-|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Exec_RTMC

This function controls the robot according to the motion control values. Inside the controller, the motion control values are first stored in a buffer, and then the motion control values in the buffer are read out at each control cycle of the controller to control the robot. If this function is called when the controller's buffer is full, it waits until the buffer becomes empty. For details on the buffer size setting, see [Set_BufferSize](#set_buffersize) and [Get_BufferSize](#get_buffersize).

[Syntax]

```shell-session
short Exec_RTMC(vector<double> jacmd, vector<double>& pcurja, unsigned short iocmd, unsigned int* pcurio, uint8_t* pbufstatus)
```

[Arguments]

- IN
    - jacmd: Motion control values (unit: rad or mm)
    - iocmd: Control values to standard I/O outputs

        To control standard I/O outputs with this argument, you must set the motion control type. For details, see [Set_RTMCSendFormat](#set_rtmcsendformat).

        Specify this argument with 2 bytes. For example, to turn on only the 15th bit, specify 32768 in the argument, which is decimal for "1000 0000 0000 0000."

        This argument is optional only when the motion control type is set to robot.
- OUT
    - pcurja: The current values of the robot joints (unit: rad or mm)
    - pcurio: The status of the standard I/O inputs

        To control standard I/O outputs with this argument, you must set the current value type. For details, see [Set_RTMCRecvFormat](#set_rtmcrecvformat).

        This argument can be acquired in 4 bytes. For example, when the 23rd bit is on, 8388608 can be acquired from this argument, which is the decimal number for "0000 0000 1000 0000 0000 0000 0000 0000."

        This argument is optional only when the current value type is set to robot.
    - pbufstatus: Buffer status (optional)
        - Buffer has free space: RTMC_BUFF_OK
        - Buffer is full: RTMC_BUFF_FULL

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |Completed|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Enabled|
|Safeguard| Close |
|Emergency Stop |Off|
|Controller error |None|

#### Reset

This function resets the controller to the default status.

This function resets emergency stop and error statuses in the controller. It also turns off all bits of the standard I/O outputs.

[Syntax]

```shell-session
short Reset()
```

[Arguments]

- IN
    - None
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Set_BufferSize

This function sets the size of the buffer that stores the motion control values sent with [Exec_RTMC](#exec_rtmc). The set buffer size is retained until the controller is turned off. The default buffer size is 2.

[Syntax]

```shell-session
short Set_BufferSize(unsigned short bufsize)
```

[Arguments]

- IN
    - bufsize: The buffer size to set (you can specify 2, 5, or 10)
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_BufferSize

This function gets the size of the buffer that stores the motion control values sent with [Exec_RTMC](#exec_rtmc).

[Syntax]

```shell-session
short Get_BufferSize(unsigned short* pbufsize)
```

[Arguments]

- IN
    - None
- OUT
    - pbufsize: The acquired buffer size

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Set_DigitalOutput_Bit

This function controls the specified standard I/O output by bit.

[Syntax]

```shell-session
short Set_DigitalOutput_Bit(uint8_t  bitnum, uint8_t bitdata)
```

[Arguments]

- IN
    - bitnum: Bit number of the output to control
    - bitdata: Control value
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |Close|
|Emergency Stop |Off|
|Controller error |None|


[Causes of CLIENT_ERR Failures]

Remote I/O is assigned to standard I/O. Change any standard I/O to which remote I/O was assigned to be not used. For details, refer to the following manual:  
"Epson RC+ 8.0 User's Guide - Setting Real-time Motion Control in Advance"

#### Set_DigitalOutput_Byte

This function controls the specified standard I/O outputs by byte.

[Syntax]

```shell-session
short Set_DigitalOutput_Byte(uint8_t  bytenum, uint8_t bytedata)
```

[Arguments]

- IN
    - bytenum: Byte number of the outputs to control
        - To control bits 0 to 7: 0
        - To control bits 8 to 15: 1
    - bytedata: Control values

        Specify this argument with 1 byte. For example, to turn on only the 2nd and 5th bits in the byte number with bits 0 to 7, specify 18 in the argument, which is the decimal number for "0001 0010."
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |Close|
|Emergency Stop |Off|
|Controller error |None|


[Causes of CLIENT_ERR Failures]

Remote I/O is assigned to standard I/O. Change any standard I/O to which remote I/O was assigned to be not used. For details, refer to the following manual:  
"Epson RC+ 8.0 User's Guide - Setting Real-time Motion Control in Advance"

#### Set_DigitalOutput_Word

This function simultaneously controls the specified standard I/O outputs in 2 bytes.

[Syntax]

```shell-session
short Set_DigitalOutput_Word(unsigned short worddata)
```

[Arguments]

- IN
    - worddata: Control values

        Specify this argument with 2 bytes. For example, to turn on only the 2nd and 12th bits, specify 4100 in the argument, which is the decimal number for "0001 0000 0000 0100."
- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |Close|
|Emergency Stop |Off|
|Controller error |None|


[Causes of CLIENT_ERR Failures]

Remote I/O is assigned to standard I/O. Change any standard I/O to which remote I/O was assigned to be not used. For details, refer to the following manual:  
"Epson RC+ 8.0 User's Guide - Setting Real-time Motion Control in Advance"

#### Get_DigitalOutput_Bit

This function gets the status of the specified standard I/O output bit number.

[Syntax]

```shell-session
short Get_DigitalOutput_Bit(uint8_t  bitnum, uint8_t* pbitdata)
```

[Arguments]

- IN
    - bitnum: Bit number of the output to get
- OUT
    - pbitdata: Status of the specified bit
        - OFF: 0
        - ON: 1

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_DigitalOutput_Byte

This function gets the status of the specified standard I/O output byte number.

[Syntax]

```shell-session
short Get_DigitalOutput_Byte(uint8_t  bytenum, uint8_t* pbytedata)
```

[Arguments]

- IN
    - bytenum: Byte number of the standard I/O outputs to get
        - To get bits 0 to 7: 0
        - To get bits 8 to 15: 1
- OUT
    - pbytedata: Statuses of the specified byte number

        This argument is acquired in 1 byte. For example, when the output byte number to get is bits 8 to 15 and bits 10 and 14 are on, 68 can be acquired from this argument, which is the decimal number for "0100 0100."

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_DigitalOutput_Word

This function gets the statuses of 2 bytes of standard I/O outputs.

[Syntax]

```shell-session
short Get_DigitalOutput_Word(unsigned short* pworddata)
```

[Arguments]

- IN
    - None
- OUT
    - pworddata: The statuses of 2 bytes of standard I/O outputs

        This argument is acquired in 2 bytes. For example, when bits 3 and 10 are on, 1032 can be acquired from this argument, which is the decimal number for "0000 0100 0000 1000."

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_DigitalInput_Bit

This function gets the status of the specified standard I/O input bit number.

[Syntax]

```shell-session
short Get_DigitalInput_Bit(uint8_t  bitnum, uint8_t* pbitdata)
```

[Arguments]

- IN
    - bitnum: Bit number of the input to get
- OUT
    - pbitdata: Status of the specified bit
        - OFF: 0
        - ON: 1

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_DigitalInput_Byte

This function gets the status of the specified standard I/O input byte number.

[Syntax]

```shell-session
short Get_DigitalInput_Byte(uint8_t  bytenum, uint8_t* pbytedata)
```

[Arguments]

- IN
    - bytenum: Byte number of the inputs to get
        - To get bits 0 to 7: 0
        - To get bits 8 to 15: 1
        - To get bits 16 to 23: 2
- OUT
    - pbytedata: Statuses of the specified byte number

        This argument is acquired in 1 byte. For example, when the input byte number to get is bits 16 to 23 and bits 18 and 22 are on, 68 can be acquired from this argument, which is the decimal number for "0100 0100."

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_DigitalInput_Word

This function gets the statuses of 2 bytes of standard I/O inputs.

[Syntax]

```shell-session
short Get_DigitalInput_Word(uint8_t wordnum, unsigned short* pworddata)
```

[Arguments]

- IN
    - wordnum: Word number of the standard I/O inputs to get
        - To get bits 0 to 15: 0
        - To get bits 16 to 31<sup>\*</sup>: 1
- OUT
    - pworddata: Statuses of the specified word number

        This argument is acquired in 2 bytes. For example, when the input word number to get is bits 16 to 31 and bits 17 and 22 are on, 66 can be acquired from this argument, which is the decimal number for "0000 000 0100 0010." <sup>\*</sup>

<sup>\*</sup> The values of bits 24 to 31 are 0 because these bits are outside the range of standard I/O inputs.

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |Auto|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |Disabled|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Get_RCStatus

This function gets the status information of the controller.

- Safeguard status
- Emergency stop status
- Operation Mode
- Error number/additional information 1/additional information 2/axis/error message<sup>\*</sup>
- Warning number/additional information 1/additional information 2/axis/warning message<sup>\*</sup>

You can call this function when the operation mode is not auto mode, but the above information will not be updated.

<sup>\*</sup> If no error or warning occurred, error and warning number/additional information 1/additional information 2/axis are 0. The error and warning messages are empty strings.

[Syntax]

```shell-session
short Get_RCStatus(RCSTATUS& prcstatus)
```

[Arguments]

- IN
    - None
- OUT
    - prcstatus: The acquired robot status information

The RCSTATUS structure is defined as follows:

|Member |Type |Description |Notes|
|--|--|--|--|
|safeGuard |uint8_t |Safeguard status |Door open: 1, door closed: 0|
|eStop |uint8_t |Emergency stop status |Emergency stop status: 1, emergency stop reset status: 0|
|operation_mode|unsigned short |Operation Mode |Program mode: 0, auto mode: 1, teach mode: 2|
|err_num|unsigned short |Error number |When no error occurred: 0|
|wrn_num|unsigned short |Warning number |When no warning occurred: 0|
|err_info|RCErrWrnInfo |Error details |-|
|wrn_info|RCErrWrnInfo |Warning details |-|

The RCErrWrnInfo structure is defined as follows:

|Member |Type |Description |Notes|
|--|--|--|--|
|add_info1 |int |Additional information 1 |When no error/warning occurred: 0|
|add_info2 |int |Additional information 2 |When no error/warning occurred: 0|
|jnt |unsigned short |Axis |When no error/warning occurred: 0|
|msg|std::string |Error/warning message |When no error/warning occurred: Empty string|

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Established|
|Operation Mode |-|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |-|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|

#### Set_ComProperty

This function sets the communications properties for real-time motion control. When the controller's IP address or port number for built in messaging was changed, specify the destination controller by setting its IP address and port number for built in messaging with this function before you call [RTMC_Connect](#rtmc_connect). Use this function when you also want to explicitly specify the NIC and port number on the Linux PC.

[Syntax]

```shell-session
short Set_ComProperty(COMPROPERTY set_com_property, COMPROPERTYOPT set_com_property_option)
```

[Arguments]

- IN
    - set_com_property: Communications properties for specifying the destination controller
    - set_com_property_option: Communications properties for explicitly specifying the NIC and port number on the Linux PC (optional)<sup>\*</sup>

The COMPROPERTY structure is defined as follows:

|Member |Type |Description |Notes|
|--|--|--|--|
|rc_ip_address |std::string |Destination controller's IP address |-|
|rc_builtinmsg_port|unsigned short |Destination controller's port number for built in messaging |-|

The COMPROPERTYOPT structure is defined as follows:

|Member |Type |Description |Notes|
|--|--|--|--|
|client_ip_address |std::string |Linux PC's IP Address |-|
|client_builtinmsg_port |unsigned short |Linux PC's port number for built in messaging |-|
|client_userdata_port |unsigned short |Linux PC's port number for user data |-|

<sup>\*</sup> Communications for real-time motion control uses different ports for user data and built in messaging (metadata).

- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Not established|
|Operation Mode |-|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |-|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

Communications are already established. Call this function before communications are established.

#### Get_ComProperty
This function gets the communications properties for real-time motion control.

[Syntax]

```shell-session
short Get_ComProperty(COMPROPERTY& pset_com_property, COMPROPERTYOPT& pset_com_property_option)
```

[Arguments]

- IN
    - None
- OUT
    - pset_com_property (optional)

        The acquired values of the communications properties for specifying the destination controller.

        For the definition of the COMPROPERTY structure, see [Set_ComProperty](#set_comproperty).

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Not established|
|Operation Mode |-|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |-|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

Communications are already established. Call this function before communications are established.

#### Set_SecProperty

This function sets the file properties for securing communications for real-time motion control. To secure communications for real-time motion control, call this function before you call [RTMC_Connect](#rtmc_connect). <sup>\*</sup>

<sup>\*</sup> You must also configure the robot controller for secure communications. For details on the configuration instructions, refer to the following manual.  
"Epson RC+ 8.0 User's Guide - Real-time Motion Control"

[Syntax]

```shell-session
short Set_SecProperty(SECPROPERTY secproperty)
```

[Arguments]

- IN
    - secproperty: The properties for securing communications for real-time motion control

The SECPROPERTY structure is defined as follows:

|Member |Type |Description |Notes|
|- |- |- |-|
|secuiry_communication_flag|bool |Secure communications enabled flag |Unsecure communications: false, secure communications: true, default: false|
|ca_cert_filepath |std::string |CA signing certificate |pem format|
|client_cert_filepath |std::string |Signing certificate for Linux PC application signed by CA |pem format|
|client_private_key |std::string |Private key for Linux PC application |pem format|
|client_governance_filepath |std::string |Governance document for Linux PC application |smime format|
|client_permissions_filepath |std::string |Permissions document for Linux PC application |smime format|

Specify each file with the absolute path to the file.

- OUT
    - None

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Not established|
|Operation Mode |-|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |-|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

Communications are already established. Call this function before communications are established.

#### Get_SecProperty

This function gets the property values for securing communications for real-time motion control.

[Syntax]

```shell-session
short Get_SecProperty(SECPROPERTY& psecproperty)
```

[Arguments]

- IN
    - None
- OUT
    - psecproperty: The acquired values of the file properties for securing communications for real-time motion control

        For the definition of the SECPROPERTY structure, see [Set_SecProperty](#set_secproperty).

[Request Acceptance Conditions]

|Item |Condition|
|--|--|
|Communications status |Not established|
|Operation Mode |-|
|Matching robot model check ([RBCheck](#rbcheck)) |-|
|Control mode for real-time motion control ([Set_RTMCModeEnable](#set_rtmcmodeenable)) |-|
|Safeguard |-|
|Emergency Stop |-|
|Controller error |-|


[Causes of CLIENT_ERR Failures]

Call failed because communications are already established. Call this function before communications are established.

### 5.4 API Return Values

The API return values are listed below.

- CLIENT_SUCCESS: The function call was successful.
- CLIENT_ERR: The function call failed.

    The cause of the failure depends on the API. Check [Causes of CLIENT_ERR Failures] for each API.

    CLIENT_ERR does not occur for the API that lack [Causes of CLIENT_ERR Failures].
- CLIENT_OPERATION_MODE_ERR: The function call failed because the controller is in auto mode.

    Change the operation mode to auto mode and call the function again.
- CLIENT_RBCHK_ERR: The function call failed because the robot model the Linux PC application is attempting to command is different from the robot model set in the controller.

    Complete the matching robot model check with [RBCheck](#rbcheck) first, and then call the function again.
- CLIENT_RTMC_MODE_ERR: The function call failed because the control mode for real-time motion control is enabled or disabled.

    After you set the control mode for real-time motion control to an appropriate value, call the function again.

    Check [Set_RTMCModeEnable](#set_rtmcmodeenable) and [Set_RTMCModeDisable](#set_rtmcmodedisable) for information on the control mode for real-time motion control.
- CLIENT_RCERR_STATUS_ERR: The function call failed because an error occurred in the controller.

    Take action for the error that occurred in the controller, and then call the function again.
- CLIENT_RCESTOP_STATUS_ERR: The function call failed because the controller entered the emergency stop status.

    Reset the emergency stop button, call [Reset](#reset), and then call the function again.
- CLIENT_RCSG_STATUS_ERR: The function call failed because the safeguard connected to the controller was opened.

    Close the safeguard, call [Reset](#reset), and then call the function again.
- CLIENT_COM_ERR: The function call failed because of a communications error.

    Check the following items, establish communications with the controller using [RTMC_Connect](#rtmc_connect), and then call the function again.

    - Is the Ethernet cable connected correctly?
    - Are the Linux PC and the controller on the same network?
    - Are secure communications and the destination controller set with [Set_ComProperty](#set_comproperty) in an appropriate manner?
- CLIENT_TIMEOUT_ERR: The function call timed out.

    Wait and call the function again because another function is being called.
- CLIENT_ARG_ERR: The function call failed because an argument value is not appropriate.

    Review the argument value, and then call the function again.
- CLIENT_UNDEF_ERR: The function call failed due to an unexpected error.

    End the Linux PC application and restart the controller. If the same error repeatedly occurs, contact support.

### 5.5 Steps from Starting to Ending Robot Motion Using the Client Library

To send requests to the robot and controller using the client library, you must change the control device to real-time motion control in advance.

For more details, refer to the following manual.  
"Epson RC+ 8.0 User's Guide - Real-time Motion Control"

The following code shows the minimum number of steps from starting to ending robot motion using the client library.

```C++

    RTMCClient user_client;
    std::vector<double> curja={0,0,0,0};
    std::vector<double> cmdja={0,0,0,0};

    // Start of preprocessing to start robot motion

    // Specify the password and establish communications with the robot controller.
    user_client.RTMC_Connect("usr_pass");

    // Check if the robot model the Linux PC application is attempting to control is the same as the robot model set in the robot controller.
    user_client.RBCheck("GX4C-251S");

    // Set the end effector weight (kg). In this example, the end effector weight is 3.2 kg.
    user_client.Set_Weight(3.2);

    // Set the load inertia including the end effector and workpiece (kgm^2) and the eccentricity (50 mm). In this example, the load inertia is 0.02 kgm^2 and the eccentricity is 50 mm.
    user_client.Set_Inertia(0.02,50);

    // Turn on motors.
    user_client.MotorOn();

     // Set the power mode to high.
    user_client.PowerHigh();

    // Get the robot's current joint angles.
    user_client.Get_CurrentJA(curja);
    cmdja=curja;

     // Enable the control mode for real-time motion control.
    user_client.SetRTMCModeEnable();

    // End of preprocessing to start robot motion

    for(int i=0; i<100; i++)
    {
      try{
          // Start of processing to control robot
          short ret=0;

         // Control robot to hold current posture with real-time motion control
         ret = user_client.Exec_RTMC(cmdja,curja);
         if(ret != CLIENT_SUCCESS) throw ret;

         // End of processing to control robot

      }catch(int err)
      {
         // Start of restart processing after controller error, emergency stop, or safeguard door opening

         if( err == CLIENT_RCERR_STATUS_ERR ||
             err == CLIENT_RCESTOP_STATUS_ERR ||
             err == CLIENT_RCSG_STATUS_ERR
           )
           {
              // After handling the exception, execute the following processing.

               // Reset error, emergency stop status, or safeguard.
               user_client.Reset();

               // Turn on motors.
               user_client.MotorOn();

               // Set the power mode to high.
               user_client.PowerHigh();

               // Get the robot's current joint angles.
               user_client.Get_CurrentJA(curja);
               cmdja=curja;

               // Enable the control mode for real-time motion control.
               ret = user_client.SetRTMCModeEnable();

               continue;

           }else{
            break;
           }

         // End of restart processing after robot controller error, emergency stop, or safeguard door opening
      }
    }

   // Start of processing to end robot motion

    // Disable the control mode for real-time motion control.
    user_client.SetRTMCModeDisable();

    // Turn off motors.
    user_client.MotorOff();

     // Disconnect communications with robot controller.
    user_client.RTMC_DisConnect();

    // End of processing to end robot motion

```

 To check the detailed steps, use the demo programs provided in this package. For instructions on how to use the demo program, see [Using the Demo Programs](#7-using-the-demo-programs).

### 5.6 Angular Acceleration and Angular Velocity

In robot control using real-time motion control, the robot operates according to the motion control values based on the path or trajectory planned externally (by a Linux PC application).

The maximum angular accelerations required for trajectory planning are found in joint_limits.yaml inside the config folder for each robot model in the following folder:

https://github.com/Epson-Robots/epson-robot-ros2/epson_robot_moveit_config/models

The maximum angular velocities are also found in epson_robot_property.xacro inside the urdf folder for each robot model in the following folder:

https://github.com/Epson-Robots/epson-robot-ros2/epson_robot_description/models

These files list the rated values. When moving the robot, adjust the maximum angular acceleration and the maximum angular velocity by taking into consideration the weight and load inertia of the end effector mounted to the robot.

**!!!CAUTION!!!**

Do not create a trajectory that exceeds the maximum joint accelerations in joint_limits.yaml or the maximum angular velocities in epson_robot_property.xacro.

#### 5.6.1 Adjusting the Maximum Angular Acceleration and the Maximum Angular Velocity

When moving the robot, you must adjust the maximum angular acceleration and the maximum angular velocity by taking into consideration the weight, load inertia, and eccentricity of the end effector mounted to the robot. You can calculate these values by multiplying the rated value by the adjustment ratios based on the end effector weight, load inertia, and eccentricity as shown in the following equation.

**Maximum Angular Acceleration Equation**

Max_acc'= Mac_acc * (Coef_weight * 0.001) * (Coef_eccentricity * 0.001)  * (Coef_inertia * 0.001) <sup>\*</sup>

- Max_acc' = Adjusted maximum angular acceleration
- Max_acc = Maximum angular acceleration
- Coef_weight = Adjustment ratio based on the end effector weight
- Coef_eccentricity = Adjustment ratio based on the eccentricity
- Coef_inertia = Adjustment ratio based on the load inertia

<sup>\*</sup> For the three expressions on the right side of the equation, you must consider the 4 axes of a SCARA robot and the 6 axes of a 6-axis robot when calculating the maximum angular velocity.

**Maximum Velocity Equation**

Max_vel'= Mac_vel * (Coef_weight * 0.001)

- Max_vel' = Adjusted maximum angular velocity
- Max_vel = Maximum angular velocity
- Coef_weight = Adjustment ratio based on the end effector weight

For the adjustment ratios of angular velocity and angular acceleration based on end effector weight, refer to the manual for your robot model.  
- "Robot Manual - Automatic Speed Correction at Weight Setting"
- "Robot Manual - Automatic Acceleration/Deceleration Correction at Weight Setting" (Refer to the values on the vertical axis of the graph.)

For the adjustment ratios of angular velocity based on load inertia, refer to the manual for your robot model.  
"Robot Manual - Automatic Acceleration/Deceleration Correction at Inertia Setting" (Refer to the values on the vertical axis of the graph.)

For the adjustment ratios of angular acceleration based on eccentricity, refer to the manual for your robot model.  
"Robot Manual - Automatic Acceleration/Deceleration Correction at Eccentricity Setting" (Refer to the values on the vertical axis of the graph.)

Check the adjustment ratios based on the values of the weight, load inertia, and eccentricity of the end effector mounted to the robot from the graphs. <sup>\*</sup>

When moving the robot, plan the trajectory so that the motion does not exceed the adjusted maximum angular acceleration and maximum acceleration.

<sup>\*</sup>

- The adjustment ratios vary depending on the robot model. Confirm the robot series, weight capacity, and arm length, and then confirm the adjustment ratios.
- Check the standard mode graphs.

Example: This example calculates the adjusted maximum angular acceleration assuming a SCARA robot to which an end effector is mounted with a weight of 4 kg, a load inertia of 0.03 kgm^2, and an eccentricity of 50 mm.

Since the end effector weight is 4 kg, the adjustment ratio for angular acceleration based on the end effector weight is 83 on "Example of Graph in Automatic Acceleration/Deceleration Correction at Weight Setting."

|Example of Graph in Automatic Acceleration/Deceleration Correction at Weight Setting|
|:--:|
|![](./docs/weight_example.png)|

Since the load inertia is 0.03 kgm^2, the adjustment ratio for angular acceleration based on the load inertia is 48 on "Example of Graph in Automatic Acceleration/Deceleration Correction at Inertia Setting."

|Example of Graph in Automatic Acceleration/Deceleration Correction at Inertia Setting|
|:--:|
|![](./docs/inertia_example.png)|

Since the eccentricity is 50 mm, the adjustment ratio for angular acceleration based on the eccentricity is 70 on "Example of Graph in Automatic Acceleration/Deceleration Correction at Eccentricity Setting."

|Example of Graph in Automatic Acceleration/Deceleration Correction at Eccentricity Setting|
|:--:|
|![](./docs/eccentricity_example.png)|

If the rated maximum angular acceleration of the robot is 35.00 [rad/s^2], then the adjusted maximum angular acceleration is the value calculated with the following equation:

35.00 * 0.83 * 0.7 = 20.34

For J4, the adjusted maximum angular acceleration is the value calculated with the following equation:

35.00 * 0.83 * 0.7 * 0.48 = 9.76

### 5.7 Free Running Distance and Free Running Time

For details on the free running distance and the free running time, refer to the following manuals: <sup>\*</sup>  
- "Robot Manual - Stopping Time and Stopping Distance at Emergency Stop"
- "Robot Manual - Stopping Time and Stopping Distance When Safeguard Is Open"

<sup>\*</sup>

- The free running distance and the free running time vary depending on the robot model. Confirm the robot series, weight capacity, and arm length, and then confirm the free running distance and free running time.
- Check the standard mode graphs.

## 6. Information on the Security Files

Use OpenSSL<sup>\*</sup> to create the files to set for ca_cert_filepath, client_cert_filepath, and client_private_key with [Set_SecProperty](#set_secproperty). For the files to set for client_governance_filepath and client_permissions_filepath, run create_secfile.py in the script folder of the client library to create the unsigned files. Use OpenSSL<sup>\*</sup> to sign these files.

When you run this script file, the following folders are created, and the secure files for both the Linux PC application and the controller are created. When you use the secure files created by this script with [Set_SecProperty](#set_secproperty), use the secure files for the Linux PC application.

```
[Folder in which the script is run]
├── [ca_cert]
│   ├── ca_cert_example.pem
│   └── ca_private_key_example.pem
├── [client_cert]
│   ├── client_cert_example.pem
│   ├── client_private_key_example.pem
│   ├── client_governance_example.smime
│   └── client_permissions_example.smime
└── [rc_cert]
    ├── rc_cert_example.pem
    ├── rc_private_key_example.pem
    ├── rc_governance_example.smime
    └── rc_permissions_example.smime
```

|Folder |File Name |Description|
|--|--|--|
|ca_cert |ca_cert_example.pem |Self-signed certificate of private CA (used by both the Linux PC application and the controller)|
|ca_cert |ca_private_key_example.pem |Private key of private CA|
|client_cert |client_cert_example.pem |Signing certificate for Linux application|
|client_cert |client_private_key_example.pem |Private key for Linux application|
|client_cert |client_governance_example.smime |Governance document for Linux application|
|client_cert |client_permissions_example.smime |Permissions document for Linux application|
|rc_cert |rc_cert_example.pem |Signing certificate for controller|
|rc_cert |rc_private_key_example.pem |Private key for controller|
|rc_cert |rc_governance_example.smime |Governance document for controller|
|rc_cert |rc_permissions_example.smime |Permissions document for controller|

<sup>\*</sup> OpenSSL is opensource software. For details, refer to the following website.  
https://openssl-library.org/


## 7. Using the Demo Programs

This package has three types of demo programs. Set LD_LIBRARY_PATH and other environment variables so that the demo program can load the client library.

- Standard demo: A demo program that moves only the J1 axis with real-time motion control.
- I/O demo: A demo program that controls the J1 axis and standard I/O outputs with real-time motion control.
- Secure demo: A demo program that moves only the J1 axis with real-time motion control over secure communications.

Run the demo programs as described below.

- Specify the robot model to control as the first command line argument.
- Specify the password set for the controller as the second command line argument. (You can omit the password if the controller is not password protected.)

The following example runs the standard demo for when the controller password is set to "user" and the robot model is the GX4-C251S.

```shell-session
./epson_rtmc_demo GX4-C251S user
```

When you run the secure demo, import the secure files contained in the demo/rc_cert folder into the controller first, and enable the secure communications settings in the controller. For detailed information on the secure communications settings in the controller, refer to the following manual:  
"Epson RC+ 8.0 User's Guide - Real-time Motion Control"
