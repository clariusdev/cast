Cast API with Python
========================

A Python wrapper (pycast) has been created to help with getting programs running more quickly.

Examples:
- **pycaster** a simple command line tool to connect and stream images. support for writing out images using PIL.
- **pysidecaster** a simple Qt based graphical program to connect and stream/view images. uses PySide2 for usage of the Qt libraries.

Executing under Linux:
- Install Pillow (latest PIL library) and PySide2 using pip
- Copy the python programs to the extracted libs folder (where pycast.so and libcast.so are placed)
- Execute: LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. python3 {clarius_python_example}.py
