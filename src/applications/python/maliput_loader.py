#!/usr/bin/env python3
#
# Copyright 2020 Toyota Research Institute
#

"""Basic app for loading a dragway RoadNetwork as a plugin"""

from maliput.plugin import (
    LoadRoadNetworkPlugin
)


def generate_string(road_geometry):
    """Print data from the `road_geometry`"""
    num_junctions = road_geometry.num_junctions()
    print("Number of junctions: ", )
    for j_index in range(num_junctions):
        junction = road_geometry.junction(j_index)
        print("\tJunction ", j_index)
        num_segments = junction.num_segments()
        print("\t\tNumber of segments ", num_segments)
        for s_index in range(num_segments):
            segment = junction.segment(s_index)
            num_lanes = segment.num_lanes()
            print("\t\tSegment ", s_index)
            print("\t\t\tNumber of lanes: ", num_lanes)
            for l_index in range(num_lanes):
                lane = segment.lane(l_index)
                print("\t\t\tLane id: ", lane.id().string())
                print("\t\t\t\tlength: ", lane.length())


def main():
    params = {
        "num_lanes": "12",
        "length": "14.",
        "lane_width": "4.",
        "shoulder_width": "3.3",
        "maximum_height": "5.3"
    }

    loader = LoadRoadNetworkPlugin("libmaliput_dragway_road_network.so", params)
    rn = loader.GetRoadNetwork()
    rg = rn.road_geometry()
    print("\nRoad Geometry ID: ", rg.id().string())
    generate_string(rg)

if __name__ == "__main__":
  main()
