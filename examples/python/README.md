# Cast API with Python

A Python wrapper (`pyclariuscast`) has been created to help with getting programs running more quickly.

Examples:
- **pycaster**: a command line tool to connect and stream images. Support for writing out images using PIL.
- **pysidecaster**: a Qt-based graphical program to connect and stream/view images. Uses PySide6 for usage of the Qt libraries.
- **pycontrast**: a Qt-based graphical program to stream images and adjust low-level acquisition parameters: transmit and receive frequency, aperture, pulse, voltages, TGC, noise floor and dynamic range. Uses PySide6.
- **pyimu**: a Qt 3D program that rotates a probe model with the IMU orientation streamed from the probe. Uses PySide6, and needs `scanner.obj` and `scanner.mtl` next to it.
- **pyimucapture**: a command line tool that records the IMU stream to a CSV file for offline analysis.

Setup:
1. Download the SDK zip for your platform from the [releases](https://github.com/clariusdev/cast/releases) and extract it.
2. Copy `pyclariuscast` from the `python3XX` folder matching your Python version (e.g. `python312` for Python 3.12) next to the library: `libcast.so` on Linux, `libcast.dylib` on macOS.
3. Copy the example programs into the same folder.
4. Install the dependencies using pip: PySide6 for the graphical examples, and Pillow (latest PIL library) to save images from pycaster.
5. On Linux, use Ubuntu 22.04: the SDK needs `libicu70`.
6. On macOS, give the module the path to the library next to it, then sign it again:
   ```
   install_name_tool -add_rpath @loader_path pyclariuscast.so
   codesign --force --sign - pyclariuscast.so
   ```

Execute from that folder: python3 `<example file>.py`
