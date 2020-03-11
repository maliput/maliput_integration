# `maliput-integration`

This package contains integration examples and tools that unify `maliput` core
and its possible backends.

## Build

1. Setup a development workspace as described [here](https://github.com/ToyotaResearchInstitute/dsim-repos-index/tree/master/README.md).

2. Bring up your development workspace:

```sh
cd path/to/my/workspace
source ./bringup
```

3. Build maliput-integration package and its dependencies:

  - If not building drake from source:

   ```sh
   colcon build --packages-up-to maliput-integration
   ```

  - If building drake from source:

   ```sh
   colcon build --cmake-args -DWITH_PYTHON_VERSION=3 --packages-up-to maliput-integration
   ```

# Applications

## `dragway_to_urdf`

## How to run it?

The executable `dragway_to_urdf` allows one create a URDF representation of a
dragway. To run `dragway_to_urdf`, execute:

    dragway_to_urdf \
          --dirpath=[dirpath] \
          --file_name_root=[file name root] \
          --lane_width=[lane width] \
          --length=[length of road in meters] \
          --num_lanes=[number of lanes] \
          --shoulder_width=[width of shoulder in meters]

For an explanation on what the above-mentioned parameters mean, execute:

    dragway_to_urdf --help

One the above command is executed, the following files should exist:

  1. `[dirpath]/[file name root].urdf` -- a [URDF](http://wiki.ros.org/urdf)
     description of the dragway.
  2. `[dirpath]/[file name root].obj` -- a
     [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) mesh of
     the dragway.
  3. `[dirpath]/[file name root].mtl` -- a material file that applies textures
     and colors to the above Wavefront OBJ file.
