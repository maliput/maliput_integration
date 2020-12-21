#!/usr/bin/env python3
#
# Copyright 2020 Toyota Research Institute
#

"""Basic app for loading a dragway RoadNetwork as a plugin"""

from maliput.plugin import (
    LoadRoadNetworkPlugin
)

# rn = RoadNetwork()
loader = LoadRoadNetworkPlugin("libmaliput_dragway_road_network.so")
rn = loader.GetRoadNetwork()
rg = rn.road_geometry()
print("Road Geometry ID: ", rg.id().string())
print("Number of junctions: ", rg.num_junctions())
