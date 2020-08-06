Listener API with Python
========================

A Python wrapper (pylisten) has been created to help with getting programs running more quickly.

Examples:
- **pylistener** a simple command line tool to connect and stream images. support for writing out images using PIL.
- **pysidelistener** a simple Qt based graphical program to connect and stream/view images. uses PySide2 for usage of the Qt libraries.

Executing under Linux:
- Install Pillow (latest PIL library) and PySide2 using pip
- Copy pylisten.so and liblisten.so to {listener_path}/src/python
- Execute: LD_LIBRARY_PATH=. python3 {clarius_python_example}.py
