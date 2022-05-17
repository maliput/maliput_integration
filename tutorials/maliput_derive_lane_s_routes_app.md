\page maliput_derive_lane_s_routes_app maliput_derive_lane_s_routes application

# Get possible Routes

`maliput_derive_lane_s_routes` application builds a maliput::api::RoadGeometry and returns a set of maliput::api::LaneSRoute objects that go from the start waypoint to
end one. The backends that can be selected to build the RoadGeometry are `maliput_malidrive`, `maliput_multilane` and `maliput_dragway`.

## Routing alghorithm's parameters

The application provides two ways of setting up the routing process:
 - Using a configuration file:
    `--config_file` flag should be used and pass a yaml file path that contains:
  ```yaml
  # -*- yaml -*-
  ---
  # If using malidrive backend:
  xodr_file: TShapeRoad.xodr
  # If using multilane backend:
  yaml_file: 2x2_intersection.yaml

  max_length: 100
  waypoints:
    - [0.0, -2.0, 0]
    - [47.5, -49., 0]
  ```

 - Using the flags:
  ```bash
    --start_waypoint="{0.0, -2.0, 0}"
    --end_waypoint="{47.5, -49, 0}"
    --max_length=100
    # For malidrive backend
    --xodr_file_path=TShapeRoad.xodr
    # For multilane backend
    --yaml_file: 2x2_intersection.yaml
  ```

_Note: The configuration file method will be prioritized when flags are mixed up._

#### Waypoints and max_length properties:

 - **start_waypoint**: Intertial position coordinate that indicates the start point of the routing process.
 - **end_waypoint**: Intertial position coordinate that indicates the start point of the routing process.
 - **max_length**: The maximum length of the intermediate lanes between start and end waypoints.


#### Maliput backends' flags:
Depending on the maliput backend that is selected different flags related to the RoadGeometry building process will be active.
 - maliput_malidrive backend: See MALIDRIVE_PROPERTIES_FLAGS().
 - maliput_multilane backend: See MULTILANE_PROPERTIES_FLAGS().
 - maliput_dragway backend: See DRAGWAY_PROPERTIES_FLAGS().


_Note_: This application consumes methods that are publicly provided by `maliput` package to generate the routes, please refer to maliput::routing::DeriveLaneSRoutes() for further information if needed.

A description of all the available flags can be seen by running `maliput_derive_lane_s_routes --help`.

### Using maliput_malidrive backend

```bash
$ maliput_derive_lane_s_routes --maliput_backend=malidrive --xodr_file_path=TShapeRoad.xodr --max_length=100 --start_waypoint="{0.0, -2.0, 0.0}" --end_waypoint="{47.5, -49.0, 0.0}"
```
or
```bash
$ maliput_derive_lane_s_routes --maliput_backend=malidrive --config_file=<path_to_config_yaml_file>
```
with the configuration YAML file being:
  ```yaml
  # -*- yaml -*-
  ---
  xodr_file: TShapeRoad.xodr
  max_length: 100
  waypoints:
    - [0.0, -2.0, 0.0]
    - [47.5, -49., 0.0]
  ```

Output:
```
[INFO] Configuration loaded from flags as configuration file flag isn't used.
[INFO] Max length: 15
[INFO] Waypoints:
[INFO]   - {0, -2, 0}
[INFO]   - {47.5, -49, 0}
[INFO] Loading road network using malidrive backend implementation...
[INFO] xodr file path: TShapeRoad.xodr
[INFO] RoadNetwork loaded successfully.
[INFO] Start RoadPosition:
[INFO]   - Lane: 0_0_-1
[INFO]   - s,r,h: (0, -0.25, 0)
[INFO] End RoadPosition:
[INFO]   - Lane: 2_0_1
[INFO]   - s,r,h: (1, 0.75, 0)
[INFO] Number of routes: 1
Route 1 of 1:
- Lane: 0_0_-1
- Lane: 9_0_-1
- Lane: 2_0_1
  SRange: [45.99999999999975, 0.9999999999998996]

```


### Using maliput_multilane backend

Similarly, `maliput_multilane` can be used.

```bash
$ maliput_derive_lane_s_routes --maliput_backend=multilane --yaml_file=2x2_intersection.yaml --max_length=100 --start_waypoint="{-50.0, -2.0, 0.0}" --end_waypoint="{50.0, -2.0, 0.0}"
```
or

```
maliput_derive_lane_s_routes --maliput_backend=malidrive --config_file=<path_to_config_yaml_file>
```
with the configuration YAML file being:
  ```yaml
  # -*- yaml -*-
  ---
  yaml_file: 2x2_intersection.yaml
  max_length: 100
  waypoints:
    - [-50.0, -2.0, 0.0]
    - [50.0, -2.0, 0.0]
  ```

Output:
```
[INFO] Configuration loaded from flags as configuration file flag isn't used.
[INFO] Max length: 100
[INFO] Waypoints:
[INFO]   - {-50, -2, 0}
[INFO]   - {50, -2, 0}
[INFO] Loading road network using multilane backend implementation...
[INFO] yaml file path: 2x2_intersection.yaml
[INFO] RoadNetwork loaded successfully.
[INFO] Start RoadPosition:
[INFO]   - Lane: l:w_segment_0
[INFO]   - s,r,h: (9.375, -0.125, 0)
[INFO] End RoadPosition:
[INFO]   - Lane: l:e_segment_0
[INFO]   - s,r,h: (40.625, -0.125, 0)
[INFO] Number of routes: 1
Route 1 of 1:
- Lane: l:w_segment_0
  SRange: [9.375, 50]
- Lane: l:ew_intersection_segment_0
- SRange: [0, 40.625]
  Lane: l:e_segment_0
```

### Using maliput_dragway backend

Similarly, `maliput_dragway` can be used.
Have in mind that `maliput_dragway` backend doesn't support having multiples segments or branch points so it is expected that if the waypoints are located at different then lanes there won't be a possible route that connect them.

```bash
$ maliput_derive_lane_s_routes --maliput_backend=dragway -num_lanes=2 --length=50  --max_length=100 --start_waypoint="{0.0, 0.0, 0.0}" --end_waypoint="{25.0, 1.0, 0.0}"
```

or

```
maliput_derive_lane_s_routes --maliput_backend=malidrive --config_file=<path_to_config_yaml_file>
```
with the configuration YAML file being:
  ```yaml
  # -*- yaml -*-
  ---
  max_length: 100
  waypoints:
    - [0.0, 0.0, 0.0]
    - [25.0, 1.0, 0.0]
  ```



Output:
```
[INFO] Configuration loaded from flags as configuration file flag isn't used.
[INFO] Max length: 100
[INFO] Waypoints:
[INFO]   - {0, 0, 0}
[INFO]   - {25, 1, 0}
[INFO] Loading road network using dragway backend implementation...
[INFO] RoadNetwork loaded successfully.
[INFO] Start RoadPosition:
[INFO]   - Lane: Dragway_Lane_0
[INFO]   - s,r,h: (0, 1.85, 0)
[INFO] End RoadPosition:
[INFO]   - Lane: Dragway_Lane_1
[INFO]   - s,r,h: (25, -0.85, 0)
[INFO] Number of routes: 0
[ERROR] No routes found.

```

## More available options

As mentioned before, `maliput_derive_lane_s_routes` application has several arguments that can be used. All of them can be accessed by running `maliput_derive_lane_s_routes --help`.

Use `--log_level` to set the log output See possible values at maliput::common::logger::level. By default set to `unchanged`.
