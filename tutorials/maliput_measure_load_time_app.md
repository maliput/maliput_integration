\page maliput_measure_load_time_app maliput_measure_load_time application

# Measure load time

`maliput_measure_load_time` application allows to measure the loading process a maliput::api::RoadGeometry. The backends that can be selected to build the RoadGeometry are `maliput_malidrive`, `maliput_multilane` and `maliput_dragway`.


Depending on the maliput backend that is selected different flags related to the RoadGeometry building process will be active.
 - maliput_malidrive backend: See MALIDRIVE_PROPERTIES_FLAGS().
 - maliput_multilane backend: See MULTILANE_PROPERTIES_FLAGS().
 - maliput_dragway backend: See DRAGWAY_PROPERTIES_FLAGS().

A description of all the available flags can be seen by running `maliput_measure_load_time --help`.


### Using maliput_malidrive backend

```bash
maliput_measure_load_time --maliput_backend=malidrive --xodr_file_path=TShapeRoad.xodr --iterations=5
```

Output:
```
[INFO] Building RoadNetwork 1 of 5.
[INFO] Building RoadNetwork 2 of 5.
[INFO] Building RoadNetwork 3 of 5.
[INFO] Building RoadNetwork 4 of 5.
[INFO] Building RoadNetwork 5 of 5.
[INFO] 	Mean time was: 0.0454088s out of 5 iterations.

```

### Using maliput_multilane backend

Similarly, `maliput_multilane` can be used.

```bash
$ maliput_measure_load_time --maliput_backend=multilane --yaml_file=2x2_intersection.yaml --iterations=5
```

Output:
```
[INFO] Building RoadNetwork 1 of 5.
[INFO] Building RoadNetwork 2 of 5.
[INFO] Building RoadNetwork 3 of 5.
[INFO] Building RoadNetwork 4 of 5.
[INFO] Building RoadNetwork 5 of 5.
[INFO] 	Mean time was: 0.0256615s out of 5 iterations.

```

### Using maliput_dragway backend

Similarly, `maliput_dragway` can be used.

```bash
$ maliput_measure_load_time --maliput_backend=dragway --num_lanes=10 --length=1000 --iterations=5

```

Output:
```
[INFO] Building RoadNetwork 1 of 5.
[INFO] Building RoadNetwork 2 of 5.
[INFO] Building RoadNetwork 3 of 5.
[INFO] Building RoadNetwork 4 of 5.
[INFO] Building RoadNetwork 5 of 5.
[INFO] 	Mean time was: 2.15518e-05s out of 5 iterations.

```

## More available options

As mentioned before, `maliput_measure_load_time` application has several arguments that can be used. All of them can be accessed by running `maliput_measure_load_time --help`.

Use `--log_level` to set the log output See possible values at maliput::common::logger::level. By default set to `unchanged`.
