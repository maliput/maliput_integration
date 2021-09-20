\page maliput_to_obj_app maliput_to_obj application

# Generating OBJ files

`maliput_to_obj` application allows to create an OBJ file from a maliput::api::RoadGeometry. The backends that can be selected to build the RoadGeometry are `maliput_malidrive`, `maliput_multilane` and `maliput_dragway`.

The application possesses flags to modify the OBJ mesh output. For example: the maximum size of a grid unit or whether to draw the elevation boundaries of lanes.

```
--max_grid_unit
--min_grid_resolution
--draw_elevation_bounds
--simplify_mesh_threshold
```

Depending on the maliput backend that is selected different flags related to the RoadGeometry building process will be active.
 - maliput_malidrive backend: See MALIDRIVE_PROPERTIES_FLAGS().
 - maliput_multilane backend: See MULTILANE_PROPERTIES_FLAGS().
 - maliput_dragway backend: See DRAGWAY_PROPERTIES_FLAGS().

A description of all the available flags can be seen by running `maliput_to_obj --help`.

_Note_: This application consumes methods that are publicly provided by `maliput` package to generate OBJ files, so please refer to maliput::utility::GenerateObjFile() for further information if needed.

Let's use the default values of the OBJ builder configuration flags except for `--max_grid_unit` and `--draw_elevation_bounds`, which will be disabled to better see the meshes`.
```
$ maliput_to_obj --max_grid_unit=1 --draw_elevation_bounds=false --dirpath="." --file_name_root=maliput_to_obj_tutorial
```

As result two files are expected to be created in the selected `dirpath`:
 - maliput_to_obj_tutorial.obj
 - maliput_to_obj_tutorial.mtl

Then we can open `maliput_to_obj_tutorial.obj` using any free software for visualizing `obj` files, for example using **[MeshLab](https://www.meshlab.net/)**.
```bash
$ meshlab maliput_to_obj_tutorial.obj
```

### Using maliput_malidrive backend

```bash
$ maliput_to_obj --maliput_backend=malidrive --xodr_file_path=TShapeRoad.xodr --omit_nondrivable_lanes=true --max_grid_unit=1 --draw_elevation_bounds=false --dirpath="." --file_name_root=maliput_to_obj_tutorial
```

Output:
```
[INFO] Loading road network using malidrive backend implementation...
[INFO] RoadNetwork loaded successfully.
[INFO] OBJ files location: /home/maliput_ws.
[INFO] Generating OBJ...
[INFO] OBJ creation has finished.

```
Mesh visualization using MeshLab:

```bash
$ meshlab maliput_to_obj_tutorial.obj
```

@image html TShapeRoadObj.png  width=800px

### Using maliput_multilane backend

Similarly, `maliput_multilane` can be used.

```bash
$ maliput_to_obj --maliput_backend=multilane --yaml_file=2x2_intersection.yaml  --max_grid_unit=1 --draw_elevation_bounds=false --dirpath="." --file_name_root=maliput_to_obj_tutorial
```

Output:
```
[INFO] Loading road network using multilane backend implementation...
[INFO] RoadNetwork loaded successfully.
[INFO] OBJ files location: /home/maliput_ws.
[INFO] Generating OBJ...
[INFO] OBJ creation has finished.

```
Mesh visualization using MeshLab:

```bash
$ meshlab maliput_to_obj_tutorial.obj
```

@image html 2x2IntersectionObj.png  width=800px

### Using maliput_dragway backend

Similarly, `maliput_dragway` can be used.

```bash
$ maliput_to_obj --maliput_backend=dragway --num_lanes=3 --length=5 --lane_width=3 --shoulder_width=0.5 --max_grid_unit=1 --draw_elevation_bounds=false --dirpath="." --file_name_root=maliput_to_obj_tutorial
```

Output:
```
[INFO] Loading road network using dragway backend implementation...
[INFO] RoadNetwork loaded successfully.
[INFO] OBJ files location: /home/maliput_ws.
[INFO] Generating OBJ...
[INFO] OBJ creation has finished.

```
Mesh visualization using MeshLab:

```bash
$ meshlab maliput_to_obj_tutorial.obj
```

@image html DragwayObj.png  width=800px

## More available options

As mentioned before, `maliput_to_obj` application has several arguments that can be used. All of them can be accessed by running `maliput_to_obj --help`.

Use `--log_level` to set the log output See possible values at maliput::common::logger::level. By default set to `unchanged`.
