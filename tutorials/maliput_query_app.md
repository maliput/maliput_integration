\page maliput_query_app maliput_query application

# Queries to a RoadGeometry

`maliput_query` application has several commands that can be selected to perform queries to the maliput::api::RoadGeometry. The backends that can be selected are `maliput_malidrive`, `maliput_multilane` and `maliput_dragway`.

```bash
  maliput_query -- <command> <arg1> <arg2> ... <argN>
```

The application helps you to obtain information of the maliput::api::RoadGeometry trough queries like:
```
GetLaneBounds <lane_id> <s>: To obtains the segment and lane bounds of lane_id at s position.

LaneToInertialPosition <lane_id> <s> <r> <h>: To obtain the InertialPosition for an (s, r, h) LanePosition in a Lane, identified by lane_id.

ToLanePosition <lane_id> <x> <y> <z>: To obtains the LanePosition in a Lane, identified by lane_id, that is closest, in the world frame, to an (x, y, z) InertialPosition.
```

**The full list of available commands can be obtained by running:** `maliput_query --help`

Depending on the maliput backend that is selected different flags related to the RoadGeometry building process will be active.
 - maliput_malidrive backend: See MALIDRIVE_PROPERTIES_FLAGS().
 - maliput_multilane backend: See MULTILANE_PROPERTIES_FLAGS().
 - maliput_dragway backend: See DRAGWAY_PROPERTIES_FLAGS().


## Lane position to Inertial position.
Using `maliput_query` we can obtain information about the `Inertial`-Frame coordinate that matches a given maliput::api::LanePosition.

```bash
maliput_query -- LaneToInertialPosition <lane_id> <s> <r> <h>
```

### Using maliput_malidrive backend

```bash
$ maliput_query --maliput_backend=malidrive --xodr_file_path=TShapeRoad.xodr --linear_tolerance=0.05 -- LaneToInertialPosition 1_0_1 5 0 0
```

Output:
```
[INFO] Loading road network using malidrive backend implementation...
[INFO] RoadNetwork loaded successfully.
Geometry Loaded
(1_0_1)->ToInertialPosition(lane_position: (s = 5, r = 0, h = 0))
              : Result: inertial_position:(x = 59, y = 1.75, z = 0)
              : Result round_trip inertial_position(x = 59, y = 1.75, z = 0), with distance: 0
              : RoadPosition: (lane: 1_0_1, lane_pos: (s = 5, r = 0, h = 0))

```

_Note: To increase verbosity the log level could be adjusted to `TRACE` by adding the `--log_level=trace` flag._

### Using maliput_multilane backend

Similarly, `maliput_multilane` can be used.

```bash
$  maliput_query --maliput_backend=multilane --yaml_file=2x2_intersection.yaml -- LaneToInertialPosition l:ew_intersection_segment_0 0 0 0
```

Output:
```
[INFO] Loading road network using multilane backend implementation...
[INFO] RoadNetwork loaded successfully.
Geometry Loaded
(l:ew_intersection_segment_0)->ToInertialPosition(lane_position: (s = 0, r = 0, h = 0))
              : Result: inertial_position:(x = -9.375, y = -1.875, z = 0)
              : Result round_trip inertial_position(x = -9.375, y = -1.875, z = 0), with distance: 0
              : RoadPosition: (lane: l:ew_intersection_segment_0, lane_pos: (s = 0, r = 0, h = 0))

```

### Using maliput_dragway backend

Similarly, `maliput_dragway` can be used.

```bash
$ maliput_query --maliput_backend=dragway --num_lanes=3 --length=5 --lane_width=3 --shoulder_width=0.5 --maximum_height=4 -- LaneToInertialPosition Dragway_Lane_0 5 0 0
```

Output:
```
[INFO] Loading road network using dragway backend implementation...
[INFO] RoadNetwork loaded successfully.
Geometry Loaded
(Dragway_Lane_0)->ToInertialPosition(lane_position: (s = 5, r = 0, h = 0))
              : Result: inertial_position:(x = 5, y = -3, z = 0)
              : Result round_trip inertial_position(x = 5, y = -3, z = 0), with distance: 0
              : RoadPosition: (lane: Dragway_Lane_0, lane_pos: (s = 5, r = 0, h = 0))
```

## More available options

`maliput_query` application has several arguments that can be used. All of them can be accessed by running `maliput_query --help`.

Use `--log_level` to set the log output See possible values at maliput::common::logger::level. By default set to `unchanged`.
