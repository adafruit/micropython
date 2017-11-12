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
import unittest
from nordicsemi.bluetooth.hci.slip import Slip
from nordicsemi.bluetooth.hci import codec


class TestInitPacket(unittest.TestCase):
    def setUp(self):
        pass

    def test_decode_packet(self):
        # TODO: extend this test, this tests only a small portion of the slip/hci decoding
        # These are packets read from Device Monitoring Studio
        # during communication between serializer application and firmware
        read_packets = [
            " C0 10 00 00 F0 C0 C0 D1 6E 00 C1 01 86 00 00 00 00 17 63 C0",
            " C0 D2 DE 02 4E 02 1B 00 FF FF 01 17 FE B4 9A 9D E1 B0 F8 02"
            " 01 06 11 07 1B C5 D5 A5 02 00 A9 B7 E2 11 A4 C6 00 FE E7 74"
            " 09 09 49 44 54 57 32 31 38 48 5A BB C0",
            " C0 D3 EE 00 3F 02 1B 00 FF FF 01 17 FE B4 9A 9D E1 AF 01 F1 62 C0",
            " C0 D4 DE 02 4C 02 1B 00 FF FF 01 17 FE B4 9A 9D E1 B1 F8 02 01 06"
            " 11 07 1B C5 D5 A5 02 00 A9 B7 E2 11 A4 C6 00 FE E7 74 09 09 49 44 54 57 32 31 38 48 6E C8 C0"
        ]

        slip = Slip()
        output = list()

        for uart_packet in read_packets:
            hex_string = uart_packet.replace(" ", "")
            hex_data = hex_string.decode("hex")
            slip.append(hex_data)

        packets = slip.decode()

        for packet in packets:
            output.append(codec.ThreeWireUartPacket.decode(packet))

        self.assertEqual(len(output), 5)

        packet_index = 0
        self.assertEqual(output[packet_index].seq, 0)

        packet_index += 1
        self.assertEqual(output[packet_index].seq, 1)

        packet_index += 1
        self.assertEqual(output[packet_index].seq, 2)

        packet_index += 1
        self.assertEqual(output[packet_index].seq, 3)

        packet_index += 1
        self.assertEqual(output[packet_index].seq, 4)
