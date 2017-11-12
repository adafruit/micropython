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

import copy
import json
import unittest

from nordicsemi.dfu.init_packet import PacketField
from nordicsemi.dfu.manifest import ManifestGenerator, Manifest
from nordicsemi.dfu.model import HexType
from nordicsemi.dfu.package import FirmwareKeys


class TestManifest(unittest.TestCase):
    def setUp(self):
        self.firmwares_data_a = {}

        init_packet_data_a = {
            PacketField.DEVICE_TYPE: 1,
            PacketField.DEVICE_REVISION: 2,
            PacketField.APP_VERSION: 1000,
            PacketField.REQUIRED_SOFTDEVICES_ARRAY: [22, 11],
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID: 2,
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_LENGTH: 1234,
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH:
                '\xc9\xd3\xbfi\xf2\x1e\x88\xa01\x1e\r\xd2BSa\x12\xf8BW\x9b\xef&Z$\xbd\x02U\xfdD?u\x9e',
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_INIT_PACKET_ECDS:
                '1\xd7B8\x129\xaa\xc3\xe6\x8b\xe2\x01\xd11\x17\x01\x00\xae\x1e\x04\xf9~q\xcd\xbfv"\xdan\xc0f2\xd49' +
                '\xdc\xc7\xf8\xae\x16VV\x17\x90\xa3\x96\xadxPa\x0bs\xfe\xbdi]\xb2\x95\x81\x99\xe4\xb0\xcf\xe9\xda'
        }

        self.firmwares_data_a[HexType.APPLICATION] = {
            FirmwareKeys.BIN_FILENAME: "app_fw.bin",
            FirmwareKeys.DAT_FILENAME: "app_fw.dat",
            FirmwareKeys.INIT_PACKET_DATA: init_packet_data_a,
            FirmwareKeys.ENCRYPT: False}

        self.firmwares_data_a[HexType.SD_BL] = {
            FirmwareKeys.BIN_FILENAME: "sd_bl_fw.bin",
            FirmwareKeys.DAT_FILENAME: "sd_bl_fw.dat",
            FirmwareKeys.INIT_PACKET_DATA: copy.copy(init_packet_data_a),  # Fake the hash
            FirmwareKeys.BL_SIZE: 50,
            FirmwareKeys.SD_SIZE: 90
        }

        self.firmwares_data_b = {}

        init_packet_data_b = {
            PacketField.DEVICE_TYPE: 1,
            PacketField.DEVICE_REVISION: 2,
            PacketField.APP_VERSION: 1000,
            PacketField.REQUIRED_SOFTDEVICES_ARRAY: [22, 11],
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16: 0xfaae
        }

        self.firmwares_data_b[HexType.APPLICATION] = {
            FirmwareKeys.BIN_FILENAME: "app_fw.bin",
            FirmwareKeys.DAT_FILENAME: "app_fw.dat",
            FirmwareKeys.INIT_PACKET_DATA: init_packet_data_b
        }

        self.firmwares_data_b[HexType.BOOTLOADER] = {
            FirmwareKeys.BIN_FILENAME: "bootloader_fw.bin",
            FirmwareKeys.DAT_FILENAME: "bootloader_fw.dat",
            FirmwareKeys.INIT_PACKET_DATA: copy.copy(init_packet_data_b),  # Fake the hash
        }

        self.firmwares_data_c = {}

        init_packet_data_c = {
            PacketField.DEVICE_TYPE: 1,
            PacketField.DEVICE_REVISION: 2,
            PacketField.APP_VERSION: 1000,
            PacketField.REQUIRED_SOFTDEVICES_ARRAY: [22, 11],
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16: 0xfaae
        }

        self.firmwares_data_c[HexType.SOFTDEVICE] = {
            FirmwareKeys.BIN_FILENAME: "softdevice_fw.bin",
            FirmwareKeys.DAT_FILENAME: "softdevice_fw.dat",
            FirmwareKeys.INIT_PACKET_DATA: init_packet_data_c
        }

    def test_generate_manifest(self):
        r = ManifestGenerator(0.5, self.firmwares_data_a)

        _json = json.loads(r.generate_manifest())

        # Test for presence of attributes in document
        self.assertIn('manifest', _json)

        manifest = _json['manifest']
        self.assertIn('application', manifest)

        application = manifest['application']
        self.assertIn('init_packet_data', application)
        self.assertIn('dat_file', application)
        self.assertIn('bin_file', application)

        init_packet_data = application['init_packet_data']
        self.assertIn('firmware_hash', init_packet_data)
        self.assertIn('softdevice_req', init_packet_data)
        self.assertIn('device_revision', init_packet_data)
        self.assertIn('device_type', init_packet_data)
        self.assertIn('application_version', init_packet_data)

        # Test for values in document
        self.assertEqual("app_fw.bin", application['bin_file'])
        self.assertEqual("app_fw.dat", application['dat_file'])

        self.assertEqual(2, init_packet_data['ext_packet_id'])
        self.assertEqual(1234, init_packet_data['firmware_length'])
        self.assertEqual('c9d3bf69f21e88a0311e0dd242536112f842579bef265a24bd0255fd443f759e',
                         init_packet_data['firmware_hash'])
        self.assertEqual('31d742381239aac3e68be201d131170100ae1e04f97e71cdbf7622da6ec06632d439dcc7f8ae1656561790a396ad7850610b73febd695db2958199e4b0cfe9da',
                         init_packet_data['init_packet_ecds'])
        self.assertEqual(1000, init_packet_data['application_version'])
        self.assertEqual(1, init_packet_data['device_type'])
        self.assertEqual(2, init_packet_data['device_revision'])
        self.assertEqual([22, 11], init_packet_data['softdevice_req'])

        # Test softdevice_bootloader
        bl_sd = manifest['softdevice_bootloader']
        self.assertIsNotNone(bl_sd)
        self.assertEqual(90, bl_sd['sd_size'])
        self.assertEqual(50, bl_sd['bl_size'])

        # Test for values in document
        self.assertEqual("sd_bl_fw.bin", bl_sd['bin_file'])
        self.assertEqual("sd_bl_fw.dat", bl_sd['dat_file'])

    def test_manifest_a(self):
        r = ManifestGenerator(0.5, self.firmwares_data_a)
        m = Manifest.from_json(r.generate_manifest())
        self.assertIsNotNone(m)
        self.assertIsNotNone(m.application)
        self.assertEqual("app_fw.bin", m.application.bin_file)
        self.assertEqual("app_fw.dat", m.application.dat_file)
        self.assertIsNone(m.bootloader)
        self.assertIsNone(m.softdevice)
        self.assertIsNotNone(m.softdevice_bootloader)
        self.assertEqual(90, m.softdevice_bootloader.sd_size)
        self.assertEqual(50, m.softdevice_bootloader.bl_size)
        self.assertEqual("sd_bl_fw.bin", m.softdevice_bootloader.bin_file)
        self.assertEqual("sd_bl_fw.dat", m.softdevice_bootloader.dat_file)

    def test_manifest_b(self):
        r = ManifestGenerator("0.5", self.firmwares_data_b)
        m = Manifest.from_json(r.generate_manifest())
        self.assertIsNotNone(m)
        self.assertIsNotNone(m.application)
        self.assertEqual("app_fw.bin", m.application.bin_file)
        self.assertEqual("app_fw.dat", m.application.dat_file)
        self.assertIsNotNone(m.bootloader)
        self.assertEqual("bootloader_fw.bin", m.bootloader.bin_file)
        self.assertEqual("bootloader_fw.dat", m.bootloader.dat_file)
        self.assertIsNone(m.softdevice)
        self.assertIsNone(m.softdevice_bootloader)
        self.assertEqual(0xfaae, m.application.init_packet_data.firmware_crc16)
        self.assertEqual(0xfaae, m.bootloader.init_packet_data.firmware_crc16)


    def test_manifest_c(self):
        r = ManifestGenerator("0.5", self.firmwares_data_c)
        m = Manifest.from_json(r.generate_manifest())
        self.assertIsNotNone(m)
        self.assertIsNone(m.application)
        self.assertIsNone(m.bootloader)
        self.assertIsNotNone(m.softdevice)
        self.assertEqual('softdevice_fw.bin', m.softdevice.bin_file)
        self.assertEqual('softdevice_fw.dat', m.softdevice.dat_file)
        self.assertIsNone(m.softdevice_bootloader)
        self.assertEqual(0xfaae, m.softdevice.init_packet_data.firmware_crc16)

if __name__ == '__main__':
    unittest.main()
