Cast API with Python
========================

A Python wrapper (pyclariuscast) has been created to help with getting programs running more quickly.

Examples:
- **pycaster** a simple command line tool to connect and stream images. support for writing out images using PIL.
- **pysidecaster** a simple Qt based graphical program to connect and stream/view images. uses PySide6 for usage of the Qt libraries.

Executing under Linux:
- Install Pillow (latest PIL library) and PySide6 using pip
- Copy the python programs to the extracted libs folder (where pycast.so and libcast.so are placed)
- Execute: python3 {clarius_python_example}.py
