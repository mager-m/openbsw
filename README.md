# Eclipse OpenBSW


## Build Status 🚀

[![Build S32k and posix platform](https://github.com/eclipse-openbsw/openbsw/actions/workflows/build.yml/badge.svg?branch=main&event=push)](https://github.com/eclipse-openbsw/openbsw/actions/workflows/build.yml)

## Code Coverage 

| Code Coverage            | Status                                                                 |
|--------------------------|------------------------------------------------------------------------|
| Line Coverage            | [![Line Coverage](https://eclipse-openbsw.github.io/openbsw/coverage_badges/line_coverage_badge.svg)](https://github.com/eclipse-openbsw/openbsw/actions/workflows/code-coverage.yml) |
| Function Coverage        | [![Function Coverage](https://eclipse-openbsw.github.io/openbsw/coverage_badges/function_coverage_badge.svg)](https://github.com/eclipse-openbsw/openbsw/actions/workflows/code-coverage.yml) |

## Overview

Eclipse OpenBSW is an open source SDK to build professional, high quality embedded software products. 
It is a software stack specifically designed and developed for automotive applications.

This repository provides the complete code, documentation and a reference example
that works out of the box without any specific hardware requirements (any POSIX platform)
allowing developers to get up and running quickly.

## Target Audience

* **Open Source Enthusiasts**: Enthusiasts and hobbyists passionate about automotive technology
  and interested in contributing to open source projects, collaborating with like-minded
  individuals and exploring new ideas and projects in the automotive domain.

* **Embedded Systems Developers**: Developers specializing in embedded systems programming,
  microcontroller firmware development and real-time operating systems (RTOS), who are interested
  in automotive applications.

* **Automotive Engineers**: Professionals working in the automotive industry, including engineers,
  designers and technicians, who are interested in developing and improving automotive
  technologies, systems and components.

* **Students and Researchers**: Students, researchers, and academic institutions interested in
  learning about automotive technologies, conducting research, and exploring innovative solutions
  in automotive areas.

## Getting Started

To get started, we recommend to compile our reference application for one of the supported platforms
using the docker image we provide including all the necessary tools. Therefore, you can simply run
the development service in the docker compose in the root of the repo, call cmake with the correct
options and build the generated project.

> [!NOTE]
> 
> In case your local user already uses `UID`/`GID` of `1000` you can skip the `DOCKER_UID` and
> `DOCKER_GID` variables, since this is the default. Otherwise, you need it to make sure you have
> proper access to your local files.
> 
> In case you want to use a custom history file for the commands you run in the container you can
> also set the `DOCKER_HISTORY` variable, which defaults to the `~/.docker_history` file.
>
> Note, that we bind mount your current working directory into the container and use it as working
> directory. This makes sure, that you will have the same paths inside and outside of the container
> when for example loading a generated elf into a debugger or following symlinks created within the
> container.

```
host> DOCKER_UID=$(id -u) DOCKER_GID=$(id -g) docker compose run --build development
docker> cmake --preset posix
docker> cmake --build --preset posix
```

### Building for Arduino Nano ESP32

Requires [ESP-IDF v5.5.x](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32s3/get-started/index.html)
with `IDF_PATH` set and ESP-IDF tools (`cmake`, `ninja`, `xtensa-esp32s3-elf-gcc`) in PATH.

The ESP32 build has two phases — OpenBSW compilation, then linking with ESP-IDF runtime:

```bash
# Phase 1: Compile OpenBSW (link failure at the end is expected)
cmake --preset esp32-freertos
cmake --build build/esp32-freertos --config RelWithDebInfo

# Archive application objects
xtensa-esp32s3-elf-ar rcs build/esp32-freertos/libappReferenceApp.a \
  build/esp32-freertos/executables/referenceApp/application/CMakeFiles/app.referenceApp.dir/RelWithDebInfo/src/**/*.obj

# Phase 2: Build ESP-IDF wrapper (produces flashable binary)
cd executables/referenceApp/platforms/esp32/idf_project
cmake -G Ninja -S . -B build -DIDF_TARGET=esp32s3 \
  -DSDKCONFIG_DEFAULTS=sdkconfig.defaults -DPYTHON_DEPS_CHECKED=1 -DCCACHE_ENABLE=0
cmake --build build

# Flash (hold B1 button, plug USB, release B1 to enter bootloader)
idf.py -p <PORT> flash
idf.py -p <PORT> monitor    # press Enter, type 'help'
```

See [ESP32 platform documentation](doc/dev/platforms/esp32/index.rst) for full details,
pin configuration, and VS Code task setup.

## Feature Overview

### Implemented Features

| Feature | Description | POSIX Support | S32K148 Support | ESP32 Support | New? |
| --- | --- | --- | --- | --- | --- |
| Modular design | Based on each project's needs, required software modules can easily be included or excluded. | Yes | Yes | Yes | |
| Application Lifecycle Management | The order in which Applications/Features are brought up/down is easily organised. | Yes | Yes | Yes | |
| Console | A console is provided for diagnostic and development purposes. | In a terminal interface | Via UART | Via USB Serial/JTAG | |
| Commands | Commands can easily be added to the console to aid development, test and debugging. | Yes | Yes | Yes | |
| Logging | Diagnostic logging is implemented per software component. | Yes | Yes | Yes | |
| CAN | Support for CAN bus communication | If ``SocketCAN`` is supported | Yes | Via TWAI (ext. transceiver) | |
| Sensors and actuators integration | ADC, PWM & GPIO | | Yes | Yes (ADC, LEDC, GPIO) | |
| UDS, DoCAN | Diagnostics over CAN | If ``SocketCAN`` is supported | Yes | Yes | |
| Ethernet | Basic TCP and UDP support | Yes | Yes | No (no PHY) | |
| Storage | Persistent data storage on EEPROM and Flash | Yes | Yes | Yes (NVS-based) | |

## Roadmap

See [GitHub Issues](https://github.com/eclipse-openbsw/openbsw/issues?q=is%3Aissue%20state%3Aopen%20label%3Aenhancement).

## Documentation

The [documentation](https://eclipse-openbsw.github.io/openbsw)
describes Eclipse OpenBSW in detail and provides simple setup guides to build and use it.

## Contributing

It is expected that this repository will be used as a starting point for many custom developments.
You may wish to contribute back some of your work to this repository.
For more details see [CONTRIBUTING](CONTRIBUTING.md).

## Legals

Distributed under the [Apache 2.0 License](LICENSE).

Also see [NOTICE](NOTICE.md).
