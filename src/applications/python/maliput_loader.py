#!/usr/bin/env python3
#
# Copyright 2020 Toyota Research Institute
#

"""Basic app for loading a dragway RoadNetwork as a plugin"""

import argparse

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


def ParseArgs():
    """Parse args"""
    my_parser = argparse.ArgumentParser()
    my_parser.add_argument('lib_name',
                           metavar='lib_name',
                           type=str,
                           help='Name of the .so library')
    my_parser.add_argument('-lib_name', action='store')
    my_parser.add_argument('-num_lanes', action='store')
    my_parser.add_argument('-length', action='store')
    my_parser.add_argument('-lane_width', action='store')
    my_parser.add_argument('-shoulder_width', action='store')
    my_parser.add_argument('-maximum_height', action='store')
    my_parser.add_argument('-opendrive_file', action='store')
    my_parser.add_argument('-linear_tolerance', action='store')
    my_parser.add_argument('-angular_tolerance', action='store')
    my_parser.add_argument('-scale_length', action='store')
    args = my_parser.parse_args()

    lib_name = args.lib_name
    args_dict = vars(args)
    del args_dict["lib_name"]

    params = dict()
    for key in args_dict:
        if args_dict[key] is None:
            continue
        params[key] = args_dict[key]
    print(lib_name)
    print(params)
    return lib_name, params


def main():
    """main function"""
    lib_name, params = ParseArgs()

    loader = LoadRoadNetworkPlugin(lib_name, params)
    rn = loader.GetRoadNetwork()
    rg = rn.road_geometry()
    print("\nRoad Geometry ID: ", rg.id().string())
    generate_string(rg)


if __name__ == "__main__":
    main()
