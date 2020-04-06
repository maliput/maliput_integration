# `maliput_integration`

This package contains integration examples and tools that unify `maliput` core
and its possible backends.

## Build

1. Setup a development workspace as described [here](https://github.com/ToyotaResearchInstitute/dsim-repos-index/tree/master/README.md).

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

## `maliput_to_urdf`

## How to run it?

The executable `maliput_to_urdf` allows to create URDF files from different road geometry implementations.
If a valid filepath of an YAML file is passed, a multilane RoadGeometry will be created. Otherwise,
the following arguments will help to carry out a dragway implementation:
 -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
To run `maliput_to_urdf`, execute:

    maliput_to_urdf \
          --yaml_file=[yaml file for multilane road geometry.]
          --dirpath=[dirpath] \
          --file_name_root=[file name root] \
          --lane_width=[lane width] \
          --length=[length of road in meters] \
          --num_lanes=[number of lanes] \
          --shoulder_width=[width of shoulder in meters] \
          --maximum_height=[maximum modelled height above the road surface in meters] \
          --log_level=[log level threshold]

For an explanation on what the above-mentioned parameters mean, execute:

    maliput_to_urdf --help

One the above command is executed, the following files should exist:

  1. `[dirpath]/[file name root].urdf` -- a [URDF](http://wiki.ros.org/urdf)
     description of the dragway.
  2. `[dirpath]/[file name root].obj` -- a
     [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) mesh of
     the dragway.
  3. `[dirpath]/[file name root].mtl` -- a material file that applies textures
     and colors to the above Wavefront OBJ file.

## `maliput_to_obj`

## How to run it?

The executable `maliput_to_obj` allows to create obj files from different road geometry implementations.
If a valid filepath of an YAML file is passed, a multilane RoadGeometry will be created. Otherwise,
the following arguments will help to carry out a dragway implementation:
 -num_lanes, -length, -lane_width, -shoulder_width, -maximum_height.
To run `maliput_to_obj`, execute:

    maliput_to_obj \
          --yaml_file=[yaml file for multilane road geometry.]
          --dirpath=[dirpath] \
          --file_name_root=[file name root] \
          --lane_width=[lane width] \
          --length=[length of road in meters] \
          --num_lanes=[number of lanes] \
          --shoulder_width=[width of shoulder in meters] \
          --maximum_height=[maximum modelled height above the road surface in meters] \
          --max_grid_unit=[maximum size of a grid unit in the rendered mesh covering the road surface] \
          --min_grid_resolution=[minimum number of grid-units in either lateral or longitudinal direction] \
          --draw_elevation_boounds=[whether to draw the elevation bounds] \
          --simplify_mesh_threshold=[optional tolerance for mesh simplification, in meters] \
          --log_level=[log level threshold]

For an explanation on what the above-mentioned parameters mean, execute:

    maliput_to_obj --help

One the above command is executed, the following files should exist:

  1. `[dirpath]/[file name root].obj` -- a
     [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) mesh of
     the road geometry.
  3. `[dirpath]/[file name root].mtl` -- a material file that applies textures
     and colors to the above Wavefront OBJ file.