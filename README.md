Zephyr Overlay
==============

> **NOTE:** This project was part of [Project Tempest](https://github.com/ZephyrVR/documents/blob/master/project-tempest.md) and is no longer actively maintained.

OpenVR overlay for Zephyr.

# Installation

The following process was performed on a Windows 10 PC. Your results may vary.

1. Clone this repository to your local environment.
2. Install [Qt 5.7](https://www.qt.io/).
3. If you wish to use Qt Creator, open the `zephyr.pro` file.
4. If you wish to use Visual Studio 2015, follow these steps:
	* Add `%QT-DIR%\5.7\msvc2015_64\bin` to your path.
	* In the cloned repository, run `qmake -spec win32-msvc2015 -tp vc`.
	* Open the Visual Studio project file.
5. Download [Boost](http://www.boost.org/users/download/).
6. Install Boost. You can follow the instructions included with Boost, but here's the gist:
	* Within the Boost folder, run `bootstrap` and then `.\b2`.
	* Run `bjam install address-model=64 --prefix="out" --with-system --with-date_time --with-random link=static runtime-link=shared threading=multi`
	* Optionally, combine the resulting libraries (in the `out` directory) using `lib.exe /OUT:boost.lib *`
	* Add the resulting libraries and directories to your project as necessary. `zephyr.pro` shows one example of this where `BOOT_ROOT` is an environment variable pointing to the Boot installation directory.
7. Compile the project.
8. Generate the necessary `.dll` files.
	* Add `%QT-DIR%\5.7\msvc2015_64\bin` to your path.
	* Run `winqtdeploy.bat`. Be sure to generate debug variants if applicable.
9. Run the project.

## Configuration
The above installation process covers getting the software to compile. To get a fully functioning instance of the overlay, you will need a custom version
of [Zephyr Login](https://github.com/ZephyrVR/login) (configured with a Zephyr app's API key, etc.). You'll also need to configure the overlay software with regards to API endpoints, etc.

Better support for configuration will come at a later date.

# License

This software is released under GPL 3.0.

This software uses portions of [OpenVR-AdvancedSettings](https://github.com/matzman666/OpenVR-AdvancedSettings).

This software uses OpenSSL, Socket.IO C++, and OpenVR. Each project's license can be found in their respective directories.
