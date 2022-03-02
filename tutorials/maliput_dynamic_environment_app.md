\page maliput_dynamic_environment_app maliput_dynamic_environment application

# Maliput Dynamic Environment

`maliput_dynamic_environment` application simulates a dynamic environment where the multiple states of the rules change in a time basis. The backends that can be selected are `maliput_malidrive`, `maliput_multilane` and `maliput_dragway`.

```bash
  maliput_dynamic_environment \
    --maliput_backend=malidrive \
    --phase_duration=1 \
    --timeout=20 \
    --xodr_file_path=SingleRoadPedestrianCrosswalk.xodr \
    --road_rule_book_file=SingleRoadPedestrianCrosswalk.yaml \
    --traffic_light_book_file=SingleRoadPedestrianCrosswalk.yaml \
    --rule_registry_file=SingleRoadPedestrianCrosswalk.yaml \
    --phase_ring_book_file=SingleRoadPedestrianCrosswalk.yaml \
    --intersection_book_file=SingleRoadPedestrianCrosswalk.yaml
```

**The full list of available commands can be obtained by running:** `maliput_dynamic_environment --help`

Depending on the maliput backend that is selected different flags related to the RoadGeometry building process will be active.
 - maliput_malidrive backend: See MALIDRIVE_PROPERTIES_FLAGS().
 - maliput_multilane backend: See MULTILANE_PROPERTIES_FLAGS().
 - maliput_dragway backend: See DRAGWAY_PROPERTIES_FLAGS().


## Current Phase changing through time

`maliput_dynamic_environment` app implements a basic system that, by using the `maliput`'s api, is capable of iterating the `Phase`s of all the `PhaseRing`s that are defined for the given `maliput::api::RoadNetwork`.

The application allows setting the desired phase duration if needed.

The output of the application shows the information about the static rules (rules with onle one state) and, the information of the current phases as time pass.

Command:
```bash
  maliput_dynamic_environment \
    --maliput_backend=malidrive \
    --phase_duration=1 \
    --timeout=20 \
    --xodr_file_path=SingleRoadPedestrianCrosswalk.xodr \
    --road_rule_book_file=SingleRoadPedestrianCrosswalk.yaml \
    --traffic_light_book_file=SingleRoadPedestrianCrosswalk.yaml \
    --rule_registry_file=SingleRoadPedestrianCrosswalk.yaml \
    --phase_ring_book_file=SingleRoadPedestrianCrosswalk.yaml \
    --intersection_book_file=SingleRoadPedestrianCrosswalk.yaml
```
Output:
```
INFO] Loading road network using malidrive backend implementation...
[INFO] RoadNetwork loaded successfully.
Static DiscreteValueRules
	Discrete Value Rule: Direction-Usage Rule Type/1_0_-1 | State: WithS
	Discrete Value Rule: Direction-Usage Rule Type/1_0_1 | State: AgainstS
	Discrete Value Rule: Direction-Usage Rule Type/1_1_-1 | State: WithS
	Discrete Value Rule: Direction-Usage Rule Type/1_1_1 | State: AgainstS
	Discrete Value Rule: Direction-Usage Rule Type/1_2_-1 | State: WithS
	Discrete Value Rule: Direction-Usage Rule Type/1_2_1 | State: AgainstS
	Discrete Value Rule: Direction-Usage Rule Type/1_3_-1 | State: WithS
	Discrete Value Rule: Direction-Usage Rule Type/1_3_1 | State: AgainstS
	Discrete Value Rule: Vehicle Usage Rule Type/1_0_-1 | State: NonPedestrians
	Discrete Value Rule: Vehicle Usage Rule Type/1_0_1 | State: NonPedestrians
	Discrete Value Rule: Vehicle Usage Rule Type/1_1_-1 | State: NonPedestrians
	Discrete Value Rule: Vehicle Usage Rule Type/1_1_1 | State: NonPedestrians
	Discrete Value Rule: Vehicle Usage Rule Type/1_2_-1 | State: NonPedestrians
	Discrete Value Rule: Vehicle Usage Rule Type/1_2_1 | State: NonPedestrians
	Discrete Value Rule: Vehicle Usage Rule Type/1_3_-1 | State: NonPedestrians
	Discrete Value Rule: Vehicle Usage Rule Type/1_3_1 | State: NonPedestrians
	Discrete Value Rule: Vehicle-Stop-In-Zone-Behavior Rule Type/EastToWest | State: DoNotStop
	Discrete Value Rule: Vehicle-Stop-In-Zone-Behavior Rule Type/WestToEast | State: DoNotStop
Static RangeValueRules
	Range Value Rule: Speed-Limit Rule Type/1_0_-1_1 | State: [0, 11.1111]
	Range Value Rule: Speed-Limit Rule Type/1_0_1_1 | State: [0, 11.1111]
	Range Value Rule: Speed-Limit Rule Type/1_1_-1_1 | State: [0, 11.1111]
	Range Value Rule: Speed-Limit Rule Type/1_1_1_1 | State: [0, 11.1111]
	Range Value Rule: Speed-Limit Rule Type/1_2_-1_1 | State: [0, 11.1111]
	Range Value Rule: Speed-Limit Rule Type/1_2_1_1 | State: [0, 11.1111]
	Range Value Rule: Speed-Limit Rule Type/1_3_-1_1 | State: [0, 11.1111]
	Range Value Rule: Speed-Limit Rule Type/1_3_1_1 | State: [0, 11.1111]
Time: 0.25
PhaseRingId: PedestrianCrosswalkIntersection | Current Phase: AllGoPhase
	Discrete Value Rule: Right-Of-Way Rule Type/WestToEast | State: Go
	Discrete Value Rule: Right-Of-Way Rule Type/EastToWest | State: Go
	BulbUniqueId: WestFacing-WestFacingBulbs-GreenBulb | State: On
	BulbUniqueId: WestFacing-WestFacingBulbs-YellowBulb | State: Off
	BulbUniqueId: EastFacing-EastFacingBulbs-GreenBulb | State: On
	BulbUniqueId: EastFacing-EastFacingBulbs-RedBulb | State: Off
	BulbUniqueId: WestFacing-WestFacingBulbs-RedBulb | State: Off
	BulbUniqueId: EastFacing-EastFacingBulbs-YellowBulb | State: Off
...
...
Time: 1.001
PhaseRingId: PedestrianCrosswalkIntersection | Current Phase: AllStopPhase
	Discrete Value Rule: Right-Of-Way Rule Type/WestToEast | State: Stop
	Discrete Value Rule: Right-Of-Way Rule Type/EastToWest | State: Stop
	BulbUniqueId: WestFacing-WestFacingBulbs-GreenBulb | State: Off
	BulbUniqueId: WestFacing-WestFacingBulbs-YellowBulb | State: Off
	BulbUniqueId: EastFacing-EastFacingBulbs-GreenBulb | State: Off
	BulbUniqueId: EastFacing-EastFacingBulbs-RedBulb | State: On
	BulbUniqueId: WestFacing-WestFacingBulbs-RedBulb | State: On
	BulbUniqueId: EastFacing-EastFacingBulbs-YellowBulb | State: Off
...
...
Time: 2.001
PhaseRingId: PedestrianCrosswalkIntersection | Current Phase: AllGoPhase
	Discrete Value Rule: Right-Of-Way Rule Type/WestToEast | State: Go
	Discrete Value Rule: Right-Of-Way Rule Type/EastToWest | State: Go
	BulbUniqueId: WestFacing-WestFacingBulbs-GreenBulb | State: On
	BulbUniqueId: WestFacing-WestFacingBulbs-YellowBulb | State: Off
	BulbUniqueId: EastFacing-EastFacingBulbs-GreenBulb | State: On
	BulbUniqueId: EastFacing-EastFacingBulbs-RedBulb | State: Off
	BulbUniqueId: WestFacing-WestFacingBulbs-RedBulb | State: Off
	BulbUniqueId: EastFacing-EastFacingBulbs-YellowBulb | State: Off

```

As expected, the available `Phases` iterates on a time basis defined by the `--phase_duration` flag.
The Right-Of-Way Rules and the Traffic Lights' bulbs, change their state in tandem according to what the PhaseRingBook information, for this particular RoadNetwork, describes.
