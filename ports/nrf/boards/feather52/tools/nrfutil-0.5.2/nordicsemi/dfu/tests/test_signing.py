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

import binascii
import os
import shutil
import tempfile
import unittest

from nordicsemi.dfu.signing import Signing
from nordicsemi.dfu.init_packet import Packet, PacketField


class TestSinging(unittest.TestCase):
    def setUp(self):
        script_abspath = os.path.abspath(__file__)
        script_dirname = os.path.dirname(script_abspath)
        os.chdir(script_dirname)

    def test_gen_key(self):
        self.work_directory = tempfile.mkdtemp(prefix="nrf_signing_tests_")

        key_file_name = 'key.pem'
        key_file_path = os.path.join(self.work_directory, key_file_name)

        signing = Signing()
        signing.gen_key(key_file_path)

        self.assertTrue(os.path.exists(key_file_path))

        shutil.rmtree(self.work_directory, ignore_errors=True)

    def test_load_key(self):
        key_file_name = 'key.pem'

        signing = Signing()
        signing.load_key(key_file_name)

        self.assertEqual(64, len(binascii.hexlify(signing.sk.to_string())))

    def test_sign_and_verify(self):
        key_file_name = 'key.pem'

        signing = Signing()
        signing.load_key(key_file_name)

        init_packet_fields = {
            PacketField.DEVICE_TYPE: 0xFFFF,
            PacketField.DEVICE_REVISION: 0xFFFF,
            PacketField.APP_VERSION: 0xFFFFFFFF,
            PacketField.REQUIRED_SOFTDEVICES_ARRAY: [0xFFFE],
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID: 2,
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_LENGTH: 1234,
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH:
                '\xc9\xd3\xbfi\xf2\x1e\x88\xa01\x1e\r\xd2BSa\x12\xf8BW\x9b\xef&Z$\xbd\x02U\xfdD?u\x9e',
        }
        init_packet = Packet(init_packet_fields)
        init_packet_data = init_packet.generate_packet()

        signature = signing.sign(init_packet_data)

        self.assertTrue(signing.verify(init_packet_data, signature))

        init_packet_fields[PacketField.NORDIC_PROPRIETARY_OPT_DATA_INIT_PACKET_ECDS] = signature

        init_packet = Packet(init_packet_fields)
        init_packet_data = init_packet.generate_packet()

        self.assertFalse(signing.verify(init_packet_data, signature))

    def test_get_vk(self):
        key_file_name = 'key.pem'

        signing = Signing()
        signing.load_key(key_file_name)

        vk_str = signing.get_vk('hex')
        vk_hex = signing.get_vk_hex()
        self.assertEqual(vk_hex, vk_str)

        vk_str = signing.get_vk('code')
        vk_code = signing.get_vk_code()
        self.assertEqual(vk_code, vk_str)

        vk_str = signing.get_vk('pem')
        vk_pem = signing.get_vk_pem()
        self.assertEqual(vk_pem, vk_str)

    def test_get_vk_hex(self):
        key_file_name = 'key.pem'
        expected_vk_hex = "Verification key Qx: 658da2eddb981f697dae7220d68217abed3fb87005ec8a05b9b56bbbaa17f460\n" \
                          "Verification key Qy: 909baecdad7226c204b612b662ff4fccbd1b0c90841090d83a59cdad6c981d4c"

        signing = Signing()
        signing.load_key(key_file_name)

        vk_hex = signing.get_vk_hex()

        self.assertEqual(expected_vk_hex, vk_hex)

    def test_get_vk_code(self):
        key_file_name = 'key.pem'

        expected_vk_code = "static uint8_t Qx[] = { 0x65, 0x8d, 0xa2, 0xed, 0xdb, 0x98, 0x1f, 0x69, 0x7d, " \
                           "0xae, 0x72, 0x20, 0xd6, 0x82, 0x17, 0xab, 0xed, 0x3f, 0xb8, 0x70, 0x05, 0xec, " \
                           "0x8a, 0x05, 0xb9, 0xb5, 0x6b, 0xbb, 0xaa, 0x17, 0xf4, 0x60 };\n" \
                           "static uint8_t Qy[] = { 0x90, 0x9b, 0xae, 0xcd, 0xad, 0x72, 0x26, 0xc2, 0x04, " \
                           "0xb6, 0x12, 0xb6, 0x62, 0xff, 0x4f, 0xcc, 0xbd, 0x1b, 0x0c, 0x90, 0x84, 0x10, " \
                           "0x90, 0xd8, 0x3a, 0x59, 0xcd, 0xad, 0x6c, 0x98, 0x1d, 0x4c };"

        signing = Signing()
        signing.load_key(key_file_name)

        vk_code = signing.get_vk_code()

        self.assertEqual(expected_vk_code, vk_code)

    def test_get_vk_pem(self):
        key_file_name = 'key.pem'
        expected_vk_pem = "-----BEGIN PUBLIC KEY-----\n" \
                          "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEZY2i7duYH2l9rnIg1oIXq+0/uHAF\n" \
                          "7IoFubVru6oX9GCQm67NrXImwgS2ErZi/0/MvRsMkIQQkNg6Wc2tbJgdTA==\n" \
                          "-----END PUBLIC KEY-----\n"

        signing = Signing()
        signing.load_key(key_file_name)

        vk_pem = signing.get_vk_pem()

        self.assertEqual(expected_vk_pem, vk_pem)
