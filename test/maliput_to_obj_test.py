# BSD 3-Clause License
#
# Copyright (c) 2022, Woven Planet. All rights reserved.
# Copyright (c) 2020-2022, Toyota Research Institute. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of the copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Attempts to generate a Wavefront OBJ file from each non-blacklisted
*.yaml file from parent directory, returning a non-zero exit code if any
file fails.
"""

import glob
import subprocess
import os
import unittest

_THIS_FILE = os.path.abspath(__file__)


class TestYamlObjing(unittest.TestCase):

    def setUp(self):
        self._maliput_to_obj = os.path.join(os.getcwd(), "maliput_to_obj")
        self.assertTrue(os.path.exists(self._maliput_to_obj),
                        self._maliput_to_obj + " not found")

    def test_yaml_files(self):
        yaml_dir = os.environ.get("MULTILANE_RESOURCE_ROOT")

        yaml_files = glob.glob(os.path.join(yaml_dir, '*.yaml'))
        # NB:  Blacklist is empty now, but still here in case it is needed
        # again in the future.
        blacklist = []
        test_yaml_files = [f for f in yaml_files
                           if not any([b in f for b in blacklist])]
        self.assertTrue(len(test_yaml_files) > 0)

        for yf in test_yaml_files:
            subprocess.check_call([
                self._maliput_to_obj,
                "-file_name_root", yf,
                "-file_name_root", "/dev/null",
            ])

    def test_dragway_creation(self):
        subprocess.check_call([
            self._maliput_to_obj,
            "-file_name_root", "/dev/null",
        ])
