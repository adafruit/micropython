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

import unittest
from nordicsemi.dfu.init_packet import *


class TestInitPacket(unittest.TestCase):
    def setUp(self):
        pass

    def test_generate_packet_a(self):
        init_packet_vars = {
            PacketField.DEVICE_TYPE: 1,
            PacketField.DEVICE_REVISION: 2,
            PacketField.APP_VERSION: 3,
            PacketField.REQUIRED_SOFTDEVICES_ARRAY: [1111, 2222, 3333, 4444],
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID: 2,
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH:
                "\xc9\xd3\xbfi\xf2\x1e\x88\xa01\x1e\r\xd2BSa\x12\xf8BW\x9b\xef&Z$\xbd\x02U\xfdD?u\x9e",
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_INIT_PACKET_ECDS:
                '1\xd7B8\x129\xaa\xc3\xe6\x8b\xe2\x01\xd11\x17\x01\x00\xae\x1e\x04\xf9~q\xcd\xbfv"\xdan\xc0f2\xd49' +
                '\xdc\xc7\xf8\xae\x16VV\x17\x90\xa3\x96\xadxPa\x0bs\xfe\xbdi]\xb2\x95\x81\x99\xe4\xb0\xcf\xe9\xda'
        }

        ip = Packet(init_packet_vars)
        data = ip.generate_packet()
        self.assertEqual(data, ("\x01\x00"  # Device type
                                "\x02\x00"  # Device revision
                                "\x03\x00\x00\x00"  # App version
                                "\x04\x00"  # Softdevice array length
                                "\x57\x04"  # Softdevice entry #1
                                "\xae\x08"  # Softdevice entry #2
                                "\x05\x0d"  # Softdevice entry #3
                                "\x5c\x11"  # Softdevice entry #4
                                "\x02\x00\x00\x00" # ext packet id
                                "\xc9\xd3\xbfi\xf2\x1e\x88\xa01\x1e\r\xd2BSa\x12"   # Firmware hash, part one
                                "\xf8BW\x9b\xef&Z$\xbd\x02U\xfdD?u\x9e"             # Firmware hash, part two
                                '1\xd7B8\x129\xaa\xc3\xe6\x8b\xe2\x01\xd11\x17\x01' # Init packet ECDS, part 1
                                '\x00\xae\x1e\x04\xf9~q\xcd\xbfv"\xdan\xc0f2\xd49'  # Init packet ECDS, part 2
                                '\xdc\xc7\xf8\xae\x16VV\x17\x90\xa3\x96\xadxPa\x0b' # Init packet ECDS, part 3
                                's\xfe\xbdi]\xb2\x95\x81\x99\xe4\xb0\xcf\xe9\xda'   # Init packet ECDS, part 4
                                )
                         )

    def test_generate_packet_b(self):
        init_packet_vars = {
            PacketField.DEVICE_TYPE: 1,
            PacketField.DEVICE_REVISION: 2,
            PacketField.APP_VERSION: 0xffeeffee,
            PacketField.REQUIRED_SOFTDEVICES_ARRAY: [1111, 2222, 3333],
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID: 1,
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH:
                "\xc9\xd3\xbfi\xf2\x1e\x88\xa01\x1e\r\xd2BSa\x12\xf8BW\x9b\xef&Z$\xbd\x02U\xfdD?u\x9e"
        }

        ip = Packet(init_packet_vars)
        data = ip.generate_packet()
        self.assertEqual(data, ("\x01\x00"  # Device type
                                "\x02\x00"  # Device revision
                                "\xee\xff\xee\xff"  # App version
                                "\x03\x00"  # Softdevice array length
                                "\x57\x04"  # Softdevice entry #1
                                "\xae\x08"  # Softdevice entry #2
                                "\x05\x0d"  # Softdevice entry #3
                                "\x01\x00\x00\x00" # ext packet id
                                "\xc9\xd3\xbfi\xf2\x1e\x88\xa01\x1e\r\xd2BSa\x12"  # Firmware hash, part one
                                "\xf8BW\x9b\xef&Z$\xbd\x02U\xfdD?u\x9e"  # Firmware hash, part two
                                )
                         )

    def test_generate_packet_c(self):
        init_packet_vars = {
            PacketField.DEVICE_TYPE: 1,
            PacketField.DEVICE_REVISION: 2,
            PacketField.APP_VERSION: 0xffeeffee,
            PacketField.REQUIRED_SOFTDEVICES_ARRAY: [1111, 2222, 3333],
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID: 0,
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16: 0xfaae
        }

        ip = Packet(init_packet_vars)
        data = ip.generate_packet()
        self.assertEqual(data, ("\x01\x00"  # Device type
                                "\x02\x00"  # Device revision
                                "\xee\xff\xee\xff"  # App version
                                "\x03\x00"  # Softdevice array length
                                "\x57\x04"  # Softdevice entry #1
                                "\xae\x08"  # Softdevice entry #2
                                "\x05\x0d"  # Softdevice entry #3
                                "\x00\x00\x00\x00" # ext packet id
                                "\xae\xfa"  # CRC-16 checksum for firmware
                                )
                         )


if __name__ == '__main__':
    unittest.main()
