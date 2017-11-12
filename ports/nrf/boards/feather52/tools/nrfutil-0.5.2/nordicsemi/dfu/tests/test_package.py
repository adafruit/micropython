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

import json
import os
import tempfile
import unittest
from zipfile import ZipFile
import shutil

from nordicsemi.dfu.package import Package


class TestPackage(unittest.TestCase):
    def setUp(self):
        self.work_directory = tempfile.mkdtemp(prefix="nrf_dfu_tests_")

    def tearDown(self):
        shutil.rmtree(self.work_directory, ignore_errors=True)

    def test_generate_package_application(self):
        self.p = Package(
            dev_type=1,
            dev_rev=2,
            app_version=100,
            sd_req=[0x1000, 0xfffe],
            app_fw="firmwares/bar.hex"
        )

        pkg_name = "mypackage.zip"

        self.p.generate_package(pkg_name, preserve_work_directory=False)
        expected_zip_content = ["manifest.json", "bar.bin", "bar.dat"]

        with ZipFile(pkg_name, 'r') as pkg:
            infolist = pkg.infolist()

            for file_information in infolist:
                self.assertTrue(file_information.filename in expected_zip_content)
                self.assertGreater(file_information.file_size, 0)

            # Extract all and load json document to see if it is correct regarding to paths
            pkg.extractall(self.work_directory)

            with open(os.path.join(self.work_directory, 'manifest.json'), 'r') as f:
                _json = json.load(f)
                self.assertEqual(u'bar.bin', _json['manifest']['application']['bin_file'])
                self.assertEqual(u'bar.dat', _json['manifest']['application']['dat_file'])
                self.assertTrue(u'softdevice' not in _json['manifest'])
                self.assertTrue(u'softdevice_bootloader' not in _json['manifest'])
                self.assertTrue(u'bootloader' not in _json['manifest'])

    def test_generate_package_sd_bl(self):
        self.p = Package(dev_type=1,
                         dev_rev=2,
                         app_version=100,
                         sd_req=[0x1000, 0xfffe],
                         softdevice_fw="firmwares/foo.hex",
                         bootloader_fw="firmwares/bar.hex")

        pkg_name = "mypackage.zip"

        self.p.generate_package(pkg_name, preserve_work_directory=False)

        expected_zip_content = ["manifest.json", "sd_bl.bin", "sd_bl.dat"]

        with ZipFile(pkg_name, 'r') as pkg:
            infolist = pkg.infolist()

            for file_information in infolist:
                self.assertTrue(file_information.filename in expected_zip_content)
                self.assertGreater(file_information.file_size, 0)

            # Extract all and load json document to see if it is correct regarding to paths
            pkg.extractall(self.work_directory)

            with open(os.path.join(self.work_directory, 'manifest.json'), 'r') as f:
                _json = json.load(f)
                self.assertEqual(u'sd_bl.bin', _json['manifest']['softdevice_bootloader']['bin_file'])
                self.assertEqual(u'sd_bl.dat', _json['manifest']['softdevice_bootloader']['dat_file'])

    def test_unpack_package_a(self):
        self.p = Package(dev_type=1,
                         dev_rev=2,
                         app_version=100,
                         sd_req=[0x1000, 0xffff],
                         softdevice_fw="firmwares/bar.hex",
                         dfu_ver=0.6)
        pkg_name = os.path.join(self.work_directory, "mypackage.zip")
        self.p.generate_package(pkg_name, preserve_work_directory=False)

        unpacked_dir = os.path.join(self.work_directory, "unpacked")
        manifest = self.p.unpack_package(os.path.join(self.work_directory, pkg_name), unpacked_dir)
        self.assertIsNotNone(manifest)
        self.assertEqual(u'bar.bin', manifest.softdevice.bin_file)
        self.assertEqual(0, manifest.softdevice.init_packet_data.ext_packet_id)
        self.assertIsNotNone(manifest.softdevice.init_packet_data.firmware_crc16)

    def test_unpack_package_b(self):
        self.p = Package(dev_type=1,
                         dev_rev=2,
                         app_version=100,
                         sd_req=[0x1000, 0xffff],
                         softdevice_fw="firmwares/bar.hex",
                         dfu_ver=0.7)
        pkg_name = os.path.join(self.work_directory, "mypackage.zip")
        self.p.generate_package(pkg_name, preserve_work_directory=False)

        unpacked_dir = os.path.join(self.work_directory, "unpacked")
        manifest = self.p.unpack_package(os.path.join(self.work_directory, pkg_name), unpacked_dir)
        self.assertIsNotNone(manifest)
        self.assertEqual(u'bar.bin', manifest.softdevice.bin_file)
        self.assertEqual(1, manifest.softdevice.init_packet_data.ext_packet_id)
        self.assertIsNone(manifest.softdevice.init_packet_data.firmware_crc16)
        self.assertIsNotNone(manifest.softdevice.init_packet_data.firmware_hash)

    def test_unpack_package_c(self):
        self.p = Package(dev_type=1,
                         dev_rev=2,
                         app_version=100,
                         sd_req=[0x1000, 0xffff],
                         softdevice_fw="firmwares/bar.hex",
                         key_file="key.pem")
        pkg_name = os.path.join(self.work_directory, "mypackage.zip")
        self.p.generate_package(pkg_name, preserve_work_directory=False)

        unpacked_dir = os.path.join(self.work_directory, "unpacked")
        manifest = self.p.unpack_package(os.path.join(self.work_directory, pkg_name), unpacked_dir)
        self.assertIsNotNone(manifest)
        self.assertEqual(u'bar.bin', manifest.softdevice.bin_file)
        self.assertEqual(2, manifest.softdevice.init_packet_data.ext_packet_id)
        self.assertIsNone(manifest.softdevice.init_packet_data.firmware_crc16)
        self.assertIsNotNone(manifest.softdevice.init_packet_data.firmware_hash)
        self.assertIsNotNone(manifest.softdevice.init_packet_data.init_packet_ecds)
        self.assertEqual(manifest.dfu_version, 0.8)


if __name__ == '__main__':
    unittest.main()
