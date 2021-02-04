Clarius Cast API
================

This repository contains all related items for the Clarius Cast API

# Features

- Obtain greyscale and color Doppler **images** (cartesian data) in real-time over the wireless network
- Obtain greyscale and color Doppler **raw images** (polar co-ordinate data) in real-time over the wireless network
- Obtain **9-DOF IMU** data in real-time over the wireless network **
- Obtain **raw data** such as IQ and RF signals once imaging is frozen **
- Set **dimensions** of output images
- Work over a **wireless LAN** or on the **probe's WiFi** network
- Notifications for imaging **freeze and button presses**
- Ability to **freeze** imaging, change basic **imaging parameters**, and change **imaging modes**

** separate software licensing may be required for features to function

# Constraints

- Must be executed while the **Clarius App is running** and connected to a probe, can be the same or different mobile device/PC
- Probe, mobile device, and PC/device must be on the **same wireless network**
- Raw data can only be captured while **imaging is frozen**

# Releases

- New API binaries and source code get published when Clarius releases a new App
- APIs must be updated to the latest version, there is no backwards or forwards compatibility with older/newer Apps due to a probe firmware check that the library performs upon a connection
- Binaries can be obtained in the [releases](https://github.com/clariusdev/cast/releases) section of the GitHub repositories

# Architecture

The Cast API communicates with the _Clarius Scanner_ directly, and makes use of TCP technologies to create a secondary connection to the device that is able to receive images.

                                      +-----------------------+
                                      | Mobile Device         |
    +---------+                       |    +-------------+    |
    |         |   Primary Connection  |    |   Clarius   |    |
    |  Probe  +-----------------------|--->+     App     |    |
    |         |                       |    +-------------+    |
    |         |    Cast Connection    |                       |
    |         +<--------------+       +-----------------------+
    |         |               |
    +---------+               |       +-----------------------+
                              |       | PC                    |
                              |       |    +-------------+    |
                              |       |    | Cast        |    |
                              +-------|--->+ Application |    |
                                      |    |             |    |
                                      |    +-------------+    |
                                      |                       |
                                      +-----------------------+

# Supported Platforms

- **Windows**: Windows 10 only
- **Linux**: tested on Ubuntu 20.04 and later
- **macOS**: tested on macOS 10.15 and later

# Repository

Structure:
- **src/include**         API headers
- **src/example**         example programs
- **src/python**          python examples (import pycast modules from release package)

Example Programs:
- **caster** a simple standalone command-line program that must be run with proper input arguments. The Windows version currently requires the boost c++ libraries to be installed for program argument parsing. Images cannot be viewed, however data/images can be captured. A Linux makefile and a Visual Studio solution have been created to help with compilation.
- **caster_qt** a graphical program that allows real-time viewing of the ultrasound stream and implements more functionality than the console program. A Qt Creator project file has been created to help with compilation. A valid compiler and Qt binaries should be installed in order for a proper kit to be defined within the IDE.
- **cast_plugin_example** a graphical program that uses a separate API defined with Qt C++ calls that optimizes image viewing by rendering directly to an OpenGL context.

Typical Usage:
```
init(callbacks, dimensions)
connect(network_params)

while (input)
  performAction(input)
 
imageCallback(image)
{
  processImage(image)
}
```
# Network Information

The Clarius App will display the network information on a top bar above the image for any scanner licensed for the Cast API. The port and IP address are required to make a connection through the Cast API.

|<img height="250px" alt="connection in progress" src="blob/1.png"/>|<img height="250px" alt="connected" src="blob/2.png"/>|

Licenced scanners also have the option for forcing the port by adjusting the Clarius Cast Permission setting within the App to "Research". This provides a potentially more streamlined method when automating connections from the custom software.

|<img height="250px" alt="setting" src="blob/3.png"/>|

Since most connections will be made using the scanner's own Wi-Fi network, it is important to ensure the computer running the Cast program is on the same network. Scanner networks are typically prefixed with "DIRECT-", and the password is available for 60 seconds on the mobile device clipboard once the Clarius App has connected.

# Notes

- When running under Windows, execution may require temporarily disabling the firewall defender or adding an exception for the executable - the latter is recommended. This is due to the use of randomized ports that the API makes use of for streaming images.
