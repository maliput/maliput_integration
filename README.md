[![GCC](https://github.com/maliput/maliput_integration/actions/workflows/build.yml/badge.svg)](https://github.com/maliput/maliput_integration/actions/workflows/build.yml)

# maliput_integration

## Description

This package contains integration examples and tools that unify `maliput` core
and its possible backends.

**Note**: For full information about Maliput please visit [Maliput Documentation](https://maliput.readthedocs.io/en/latest/index.html).

## API Documentation

Refer to [Maliput Integration's Online API Documentation](https://maliput.readthedocs.io/en/latest/html/deps/maliput_integration/html/index.html).

## Applications

The integration examples are framed into applications, they can be found at [maliput_integration/applications](src/applications/).

Tutorials are provided to better understand how to get started with the examples. See[maliput_integration's tutorials](https://maliput.readthedocs.io/en/latest/html/deps/maliput_integration/html/integration_tutorials.html).


## Installation

### Supported platforms

Ubuntu Focal Fossa 20.04 LTS.

### Binary Installation on Ubuntu

See [Installation Docs](https://maliput.readthedocs.io/en/latest/installation.html#binary-installation-on-ubuntu).

### Source Installation on Ubuntu

#### Prerequisites

```
sudo apt install python3-rosdep python3-colcon-common-extensions
```

#### Build

1. Create colcon workspace if you don't have one yet.
    ```sh
    mkdir colcon_ws/src -p
    ```

2. Clone this repository in the `src` folder
    ```sh
    cd colcon_ws/src
    git clone https://github.com/maliput/maliput_integration.git
    ```

3. Install package dependencies via `rosdep`
    ```
    export ROS_DISTRO=foxy
    ```
    ```sh
    rosdep update
    rosdep install -i -y --rosdistro $ROS_DISTRO --from-paths src
    ```

4. Build the package
    ```sh
    colcon build --packages-up-to maliput_integration
    ```

    **Note**: To build documentation a `-BUILD_DOCS` cmake flag is required:
    ```sh
    colcon build --packages-select maliput_integration --cmake-args " -DBUILD_DOCS=On"
    ```
    More info at [Building Documentation](https://maliput.readthedocs.io/en/latest/developer_guidelines.html#building-the-documentation).

For further info refer to [Source Installation on Ubuntu](https://maliput.readthedocs.io/en/latest/installation.html#source-installation-on-ubuntu)


### For development

It is recommended to follow the guidelines for setting up a development workspace as described [here](https://maliput.readthedocs.io/en/latest/developer_setup.html).

## Contributing

Please see [CONTRIBUTING](https://maliput.readthedocs.io/en/latest/contributing.html) page.

## License

[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://github.com/maliput/maliput_integration/blob/main/LICENSE)
