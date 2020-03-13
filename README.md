Clarius Listener API
====================

This repository contains all related items for the Clarius Listen API

Release Cycle:
- A new API gets published when Clarius releases a new App
- APIs must be updated to the latest version, there is no backwards or forwards compatibility with older/newer Apps due to a probe firmware check that the library performs upon a connection

Features:
- Obtain greyscale and color Doppler **images** in real-time over the wireless network
- Obtain greyscale and color Doppler **image data** (as opposed to proper images) in real-time over the wireless network
- Obtain **9-dof IMU** data in real-time over the wireless network **
- Obtain **raw data** such as IQ and RF signals once imaging is frozen **
- Set **dimensions** of output images
- Work over a **wireless LAN** or on the **probe's WiFi** network
- Notifications for imaging **freeze and button presses**
- Ability to **freeze** imaging, change basic **imaging parameters**, and change **imaging modes**

** separate software licensing may be required for features to function

Constraints:
- Can only run while the **Clarius App is launched** and connected to a probe, can be the same or different mobile device/PC
- Probe, mobile device, and PC/device must be on the **same wireless network**
- Raw data can only be captured while **imaging is frozen**

Primary platforms include:
- **Linux**
- **Windows**
- **macOS**
- **iOS**
- **Android**

Repository structure:
- **lib**                 packaged binaries (platform of choice should be extracted into same folder)
- **src/include**         API headers
- **src/example**         example programs

Example programs:
- **console** a simple standalone command-line program that must be run with proper input arguments. The Windows version currently requires the boost c++ libraries to be installed for program argument parsing. Images cannot be viewed, however data/images can be captured. A Linux makefile and a Visual Studio solution have been created to help with compilation.
- **qt** a graphical program that allows real-time viewing of the ultrasound stream and implements more functionality than the console program. A Qt Creator project file has been created to help with compilation. A valid compiler and Qt binaries should be installed in order for a proper kit to be defined within the IDE.
- **qt_plugin** a graphical program that uses a separate API defined with Qt C++ calls that optimizes image viewing by rendering directly to an OpenGL context.
