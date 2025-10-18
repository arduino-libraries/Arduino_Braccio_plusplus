<img src="https://content.arduino.cc/website/Arduino_logo_teal.svg" height="100" align="right" />

`Arduino_Braccio_plusplus`
==========================

[![Check Arduino status](https://github.com/arduino-libraries/Arduino_Braccio_plusplus/actions/workflows/check-arduino.yml/badge.svg)](https://github.com/arduino-libraries/Arduino_Braccio_plusplus/actions/workflows/check-arduino.yml)
[![Compile Examples status](https://github.com/arduino-libraries/Arduino_Braccio_plusplus/actions/workflows/compile-examples.yml/badge.svg)](https://github.com/arduino-libraries/Arduino_Braccio_plusplus/actions/workflows/compile-examples.yml)
[![Spell Check status](https://github.com/arduino-libraries/Arduino_Braccio_plusplus/actions/workflows/spell-check.yml/badge.svg)](https://github.com/arduino-libraries/Arduino_Braccio_plusplus/actions/workflows/spell-check.yml)

This library allows you to control and interact with the 6 DOF Braccio++ robot arm.

## How-to-update precompiled `liblvgl`
```bash
git clone https://github.com/lvgl/lvgl
git clone https://github.com/arduino-libraries/Arduino_Braccio_plusplus
cd Arduino_Braccio_plusplus/extras
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
```

## :mag_right: Resources

* [How to install a library](https://www.arduino.cc/en/guide/libraries)
* [Help Center](https://support.arduino.cc/)
* [Forum](https://forum.arduino.cc)

## :bug: Bugs & Issues

If you want to report an issue with this library, you can submit it to the [issue tracker](https://github.com/arduino-libraries/Arduino_Braccio_plusplus/issues) of this repository. Remember to include as much detail as you can about your hardware set-up, code and steps for reproducing the issue. Make sure you're using an original Arduino board.

## :technologist: Development

There are many ways to contribute:

* Improve documentation and examples
* Fix a bug
* Test open Pull Requests
* Implement a new feature
* Discuss potential ways to improve this library

You can submit your patches directly to this repository as Pull Requests. Please provide a detailed description of the problem you're trying to solve and make sure you test on real hardware.

## :yellow_heart: Support the project

This open-source code is maintained by Arduino with the help of the community. We invest a considerable amount of time in testing code, optimizing it and introducing new features. Please consider [buying original Arduino boards](https://store.arduino.cc/) to support our work on the project.
