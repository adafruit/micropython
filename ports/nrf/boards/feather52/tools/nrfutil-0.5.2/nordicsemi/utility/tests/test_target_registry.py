# Copyright (c) 2015, Nordic Semiconductor
# All rights reserved.
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
# * Neither the name of Nordic Semiconductor ASA nor the names of its
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

import os
import unittest
from nordicsemi.utility.target_registry import TargetRegistry, EnvTargetDatabase
from nordicsemi.utility.target_registry import FileTargetDatabase


class TestTargetRegistry(unittest.TestCase):
    def setUp(self):
        script_abspath = os.path.abspath(__file__)
        script_dirname = os.path.dirname(script_abspath)
        os.chdir(script_dirname)

        # Setup the environment variables
        os.environ["NORDICSEMI_TARGET_1_SERIAL_PORT"] = "COM1"
        os.environ["NORDICSEMI_TARGET_1_PCA"] = "PCA10028"
        os.environ["NORDICSEMI_TARGET_1_DRIVE"] = "D:\\"
        os.environ["NORDICSEMI_TARGET_1_SEGGER_SN"] = "1231233333"

        os.environ["NORDICSEMI_TARGET_2_SERIAL_PORT"] = "COM2"
        os.environ["NORDICSEMI_TARGET_2_PCA"] = "PCA10028"
        os.environ["NORDICSEMI_TARGET_2_DRIVE"] = "E:\\"
        os.environ["NORDICSEMI_TARGET_2_SEGGER_SN"] = "3332222111"

    def test_get_targets_from_file(self):
        target_database = FileTargetDatabase("test_targets.json")
        target_repository = TargetRegistry(target_db=target_database)

        target = target_repository.find_one(target_id=1)
        assert target is not None
        assert target["drive"] == "d:\\"
        assert target["serial_port"] == "COM7"
        assert target["pca"] == "PCA10028"
        assert target["segger_sn"] == "123123123123"

        target = target_repository.find_one(target_id=2)
        assert target is not None
        assert target["drive"] == "e:\\"
        assert target["serial_port"] == "COM8"
        assert target["pca"] == "PCA10028"
        assert target["segger_sn"] == "321321321312"

    def test_get_targets_from_environment(self):
        target_database = EnvTargetDatabase()
        target_repository = TargetRegistry(target_db=target_database)

        target = target_repository.find_one(target_id=1)
        assert target is not None
        assert target["drive"] == "D:\\"
        assert target["serial_port"] == "COM1"
        assert target["pca"] == "PCA10028"
        assert target["segger_sn"] == "1231233333"

        target = target_repository.find_one(target_id=2)
        assert target is not None
        assert target["drive"] == "E:\\"
        assert target["serial_port"] == "COM2"
        assert target["pca"] == "PCA10028"
        assert target["segger_sn"] == "3332222111"


if __name__ == '__main__':
    unittest.main()
