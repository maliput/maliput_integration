[![gcc](https://github.com/ToyotaResearchInstitute/maliput_integration/actions/workflows/build.yml/badge.svg)](https://github.com/ToyotaResearchInstitute/maliput_integration/actions/workflows/build.yml)


# `maliput_integration`

This package contains integration examples and tools that unify `maliput` core
and its possible backends.

## Build

1. Setup a development workspace as described [here](https://github.com/ToyotaResearchInstitute/maliput_documentation/blob/main/docs/installation_quickstart.rst).

2. Bring up your development workspace:

```sh
cd path/to/my/workspace
source ./bringup
```

3. Build maliput_integration package and its dependencies:

  - If not building drake from source:

   ```sh
   colcon build --packages-up-to maliput_integration
   ```

  - If building drake from source:

   ```sh
   colcon build --cmake-args -DWITH_PYTHON_VERSION=3 --packages-up-to maliput_integration
   ```

# Applications

Check the [applications' tutorials](tutorials) folder for information about the `maliput_integration`'s applications.
