^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package maliput_integration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Forthcoming
-----------
* Adds Lane::ToSegmentPosition query. (`#123 <https://github.com/maliput/maliput_integration/issues/123>`_)
* Contributors: Franco Cipollone

0.1.2 (2022-08-16)
------------------
* Matches with BoundingRegion moving to maliput. (`#122 <https://github.com/maliput/maliput_integration/issues/122>`_)
* Contributors: Franco Cipollone

0.1.1 (2022-07-29)
------------------
* Matches change in create_road_network binding. (`#120 <https://github.com/maliput/maliput_integration/issues/120>`_)
* Adds triage workflow. (`#121 <https://github.com/maliput/maliput_integration/issues/121>`_)
* Improves README. (`#119 <https://github.com/maliput/maliput_integration/issues/119>`_)
* Contributors: Franco Cipollone

0.1.0 (2022-06-21)
------------------
* Updates package.xml file.
* Several changes for ros build farm compatibility (`#118 <https://github.com/maliput/maliput_integration/issues/118>`_)
* Uses compile definitions for the tests. (`#117 <https://github.com/maliput/maliput_integration/issues/117>`_)
* Updates license. (`#116 <https://github.com/maliput/maliput_integration/issues/116>`_)
* Removes dashing support. (`#115 <https://github.com/maliput/maliput_integration/issues/115>`_)
* Apps: Allows relative path to resource env var. (`#114 <https://github.com/maliput/maliput_integration/issues/114>`_)
* Extends maliput_query app with maliput_object related queries (`#112 <https://github.com/maliput/maliput_integration/issues/112>`_)
* Adds the RoadGeometry pointer when constructing IntersectionBooks. (`#111 <https://github.com/maliput/maliput_integration/issues/111>`_)
* Modified library dependancies from utilities to utility (`#110 <https://github.com/maliput/maliput_integration/issues/110>`_)
* Merge pull request `#109 <https://github.com/maliput/maliput_integration/issues/109>`_ from ToyotaResearchInstitute/voldivh/value_range_method_to_state
  Changed values() and ranges() method from Rules to states()
* [FEAT]: Changed values() and ranges() method from Rules to states()
* Adds example using dynamic rules (`#107 <https://github.com/maliput/maliput_integration/issues/107>`_)
* Adds BUILD_DOCS flag as opt-in flag (`#108 <https://github.com/maliput/maliput_integration/issues/108>`_)
* Matches with change in maliput's phase ring book loader. (`#106 <https://github.com/maliput/maliput_integration/issues/106>`_)
* Adds gflag for loading a RuleRegistry yaml file. (`#104 <https://github.com/maliput/maliput_integration/issues/104>`_)
* Adds workflow_dispatch to other workflows. (`#103 <https://github.com/maliput/maliput_integration/issues/103>`_)
* Adds CI badges (`#102 <https://github.com/maliput/maliput_integration/issues/102>`_)
* Replaces push by workflow_dispatch event in gcc build. (`#100 <https://github.com/maliput/maliput_integration/issues/100>`_)
* Includes malidrive's new parameter: max_linear_tolerance (`#99 <https://github.com/maliput/maliput_integration/issues/99>`_)
* Use maliput_drake instead of drake_vendor (`#98 <https://github.com/maliput/maliput_integration/issues/98>`_)
* maliput_measure_load_time: Adds tutorial. (`#97 <https://github.com/maliput/maliput_integration/issues/97>`_)
* Adds info about other maliput_to_string implemenatation apps. (`#95 <https://github.com/maliput/maliput_integration/issues/95>`_)
* maliput_derive_lane_s_route: Improve app + tutorial (`#94 <https://github.com/maliput/maliput_integration/issues/94>`_)
* Pairs with multilane::RoadGeometry going private. (`#96 <https://github.com/maliput/maliput_integration/issues/96>`_)
* Extends maliput_to_obj app to also generate urdf file. (`#93 <https://github.com/maliput/maliput_integration/issues/93>`_)
* Pairs with maliput_malidrive's load deprecation method. (`#91 <https://github.com/maliput/maliput_integration/issues/91>`_)
* Adds maliput_to_obj tutorial. (`#90 <https://github.com/maliput/maliput_integration/issues/90>`_)
* maliput_query: GetNumberOfLanes command. (`#89 <https://github.com/maliput/maliput_integration/issues/89>`_)
* maliput_query: Adds elapsed time information. (`#88 <https://github.com/maliput/maliput_integration/issues/88>`_)
* Updates maliput_query use using double dashes. (`#87 <https://github.com/maliput/maliput_integration/issues/87>`_)
* Adds maliput query tutorial (`#85 <https://github.com/maliput/maliput_integration/issues/85>`_)
* Adds maliput_to_string tutorial. (`#84 <https://github.com/maliput/maliput_integration/issues/84>`_)
* Enable doxygen check. (`#83 <https://github.com/maliput/maliput_integration/issues/83>`_)
* Removes references to InertialTolaneMappingConfig. (`#82 <https://github.com/maliput/maliput_integration/issues/82>`_)
* maliput_to_string app: Adds check_invariants flag. (`#81 <https://github.com/maliput/maliput_integration/issues/81>`_)
* Fixes maliput_to_string_with_plugin app. (`#80 <https://github.com/maliput/maliput_integration/issues/80>`_)
* Adds GetLaneLength command to maliput_query app. (`#79 <https://github.com/maliput/maliput_integration/issues/79>`_)
* maliput_query: Use the xodr file from gflag. (`#77 <https://github.com/maliput/maliput_integration/issues/77>`_)
* Set up linker properly when using clang in CI. (`#76 <https://github.com/maliput/maliput_integration/issues/76>`_)
* Enable foxy CI (`#74 <https://github.com/maliput/maliput_integration/issues/74>`_)
* Prepare for foxy (`#73 <https://github.com/maliput/maliput_integration/issues/73>`_)
* Info log road network loading status in apps. (`#75 <https://github.com/maliput/maliput_integration/issues/75>`_)
* Fix include style part 1: use <> maliput\_* includes (`#72 <https://github.com/maliput/maliput_integration/issues/72>`_)
* Use newer revision of pybind11 (`#71 <https://github.com/maliput/maliput_integration/issues/71>`_)
* rosdep update --include-eol-distros (`#70 <https://github.com/maliput/maliput_integration/issues/70>`_)
* Require OpenDRIVE file in RoadGeometryConfiguration (`#69 <https://github.com/maliput/maliput_integration/issues/69>`_)
* Pairs app with new flag in the malidrive::RoadGeometryConfiguration. (`#68 <https://github.com/maliput/maliput_integration/issues/68>`_)
* CI: Removes prereqs install for drake. (`#67 <https://github.com/maliput/maliput_integration/issues/67>`_)
* Upgrade ros-tooling to v0.2.1 (`#66 <https://github.com/maliput/maliput_integration/issues/66>`_)
* Uses maliput_documentation instead of maliput-documentation. (`#65 <https://github.com/maliput/maliput_integration/issues/65>`_)
* Use maliput_multilane instead of maliput-multilane. (`#64 <https://github.com/maliput/maliput_integration/issues/64>`_)
* Use maliput_dragway instead of maliput-dragway. (`#63 <https://github.com/maliput/maliput_integration/issues/63>`_)
* Updates standard_strictness_policy gflag. (`#62 <https://github.com/maliput/maliput_integration/issues/62>`_)
* Apps: Improve configuration options when loading a malidrive backend. (`#61 <https://github.com/maliput/maliput_integration/issues/61>`_)
* Switch ament_cmake_doxygen to main branch. (`#60 <https://github.com/maliput/maliput_integration/issues/60>`_)
* Optimizes scan-build run in CI. (`#59 <https://github.com/maliput/maliput_integration/issues/59>`_)
* Add changelog template (`#56 <https://github.com/maliput/maliput_integration/issues/56>`_)
* Replaces occurrences of dsim-repos-index by maliput_infrastructure. (`#58 <https://github.com/maliput/maliput_integration/issues/58>`_)
* Imports maliput_py to github actions. (`#57 <https://github.com/maliput/maliput_integration/issues/57>`_)
* Points to maliput_infrastructure instead of dsim-repos-index (`#55 <https://github.com/maliput/maliput_integration/issues/55>`_)
* Trigger PR clang builds on do-clang-test label (`#54 <https://github.com/maliput/maliput_integration/issues/54>`_)
* Restores scan-build workflow on label (`#53 <https://github.com/maliput/maliput_integration/issues/53>`_)
* Pairs with the change in RoadGeometry API (`#52 <https://github.com/maliput/maliput_integration/issues/52>`_)
* Moves disabled workflows to a different folder. (`#51 <https://github.com/maliput/maliput_integration/issues/51>`_)
* Adds app for measuring load time process (`#50 <https://github.com/maliput/maliput_integration/issues/50>`_)
* Allows the apps to select build policy for malidrive backend. (`#49 <https://github.com/maliput/maliput_integration/issues/49>`_)
* Refer to a specific clang version and use lld linker. (`#48 <https://github.com/maliput/maliput_integration/issues/48>`_)
* Update ros-tooling version in CI. (`#47 <https://github.com/maliput/maliput_integration/issues/47>`_)
* Fixes ubsan behavior in CI. (`#46 <https://github.com/maliput/maliput_integration/issues/46>`_)
* Removes Jenkins configuration. (`#45 <https://github.com/maliput/maliput_integration/issues/45>`_)
* Uses ament_cmake_flake8 package instead of pycodestyle. (`#44 <https://github.com/maliput/maliput_integration/issues/44>`_)
* Adds applications in cpp and python that uses the maliput plugin architecture (`#39 <https://github.com/maliput/maliput_integration/issues/39>`_)
* Improve application namespace consistency (`#42 <https://github.com/maliput/maliput_integration/issues/42>`_)
* Replaces GeoPosition by InertialPosition (`#38 <https://github.com/maliput/maliput_integration/issues/38>`_)
* Fixes clang Github CI workflow configuration. (`#40 <https://github.com/maliput/maliput_integration/issues/40>`_)
* Adds maliput_malidrive dependency to CI and try to check out same branches (`#36 <https://github.com/maliput/maliput_integration/issues/36>`_)
* Removes reference to maliput_malidrive/base/road_geometry.h because it is not installed anymore. (`#34 <https://github.com/maliput/maliput_integration/issues/34>`_)
* Adds maliput_derive_lane_s_route_app. (`#33 <https://github.com/maliput/maliput_integration/issues/33>`_)
* Unifies cmake install paths. (`#32 <https://github.com/maliput/maliput_integration/issues/32>`_)
* Adds maliput_query app. (`#30 <https://github.com/maliput/maliput_integration/issues/30>`_)
* Adds malidrive repository to scan_build workflow. (`#31 <https://github.com/maliput/maliput_integration/issues/31>`_)
* Adds maliput_malidrive backend implementation to the apps. (`#27 <https://github.com/maliput/maliput_integration/issues/27>`_)
* Adds scan_build job to Github Actions. (`#26 <https://github.com/maliput/maliput_integration/issues/26>`_)
* Disables tsan because not all backends support that build configuration. (`#29 <https://github.com/maliput/maliput_integration/issues/29>`_)
* Sets ACTIONS_ALLOW_UNSECURE_COMMANDS to true (`#28 <https://github.com/maliput/maliput_integration/issues/28>`_)
* Adds clang8, asan, ubsan and tsan to Github Actions. (`#25 <https://github.com/maliput/maliput_integration/issues/25>`_)
* Adds a template changelog. (`#23 <https://github.com/maliput/maliput_integration/issues/23>`_)
* Updates package.xml (`#22 <https://github.com/maliput/maliput_integration/issues/22>`_)
* Adds version number.
* Adds a template changelog.
* Fixes sanitizers variable. (`#20 <https://github.com/maliput/maliput_integration/issues/20>`_)
* Use GitHub Actions CI to build and test with gcc (`#19 <https://github.com/maliput/maliput_integration/issues/19>`_)
* Adds scan-build to jenkins configuration. (`#18 <https://github.com/maliput/maliput_integration/issues/18>`_)
* Parallelizes CI.
* Adds Undefined Behavior Sanitizer.
* Adds Address Sanitizer.
* Adds application to serialize dragway and multilane. (`#5 <https://github.com/maliput/maliput_integration/issues/5>`_)
* Generates URDF files for multilane and dragway implementation (`#4 <https://github.com/maliput/maliput_integration/issues/4>`_) (`#13 <https://github.com/maliput/maliput_integration/issues/13>`_)
* Pairs clang flags. (`#12 <https://github.com/maliput/maliput_integration/issues/12>`_)
* Generates OBJ file either from multilane or dragway implementation. (`#3 <https://github.com/maliput/maliput_integration/issues/3>`_)
* Changes namespace from utility to integration in yaml_to_obj.cc file. (`#2 <https://github.com/maliput/maliput_integration/issues/2>`_)
* Modifies DefaultCFlags. (`#9 <https://github.com/maliput/maliput_integration/issues/9>`_)
* Adapts files to multilane package's reorganization.
* Adapts files to dragway's reorganization. (`#7 <https://github.com/maliput/maliput_integration/issues/7>`_)
* Merge pull request `#1 <https://github.com/maliput/maliput_integration/issues/1>`_ from ToyotaResearchInstitute/francocipollone/move_maliput_integration_to_a_repo
  Brings maliput-integration package from maliput repository.
* Changes package from maliput-integration to maliput_integration
* Adds license to the package.xml file.
* Adds missing files to the repository. Minor fixes.
* Move maliput-utilities to maliput core (`#274 <https://github.com/maliput/maliput_integration/issues/274>`_)
* Move dragway_to_urdf to maliput-integration (`#305 <https://github.com/maliput/maliput_integration/issues/305>`_)
* Move yaml_to_obj to maliput-integration, remove multilane dependency (`#304 <https://github.com/maliput/maliput_integration/issues/304>`_)
* Adds maliput-integration package. (`#299 <https://github.com/maliput/maliput_integration/issues/299>`_)
* Initial commit
* Contributors: Agustin Alba Chicar, Chien-Liang Fok, Daniel Stonier, Franco, Franco Cipollone, Geoffrey Biggs, Steve Peters, Steven Peters, Voldivh
