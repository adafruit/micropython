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
import nordicsemi.dfu.nrfhex as nrfhex
import nordicsemi.dfu.intelhex as intelhex


class TestnRFHex(unittest.TestCase):
    def setUp(self):
        script_abspath = os.path.abspath(__file__)
        script_dirname = os.path.dirname(script_abspath)
        os.chdir(script_dirname)

    def comparefiles(self, actual, wanted):
        actualfile = intelhex.IntelHex()
        actualfile.loadfile(actual, format="bin")

        wantedfile = intelhex.IntelHex()
        wantedfile.loadfile(wanted, format="bin")

        self.assertEqual(actualfile.minaddr(), wantedfile.minaddr())
        self.assertEqual(actualfile.maxaddr(), wantedfile.maxaddr())

        minaddress = actualfile.minaddr()
        maxaddress = actualfile.maxaddr()

        length = maxaddress - minaddress

        actualfile_data = actualfile.gets(minaddress, length)
        wantedfile_data = wantedfile.gets(minaddress, length)

        self.assertEqual(actualfile_data, wantedfile_data)

    def test_tobinfile_single_file_without_uicr_content(self):
        nrf = nrfhex.nRFHex("firmwares/bar.hex")
        nrf.tobinfile("firmwares/bar.bin")

        self.comparefiles("firmwares/bar.bin", "firmwares/bar_wanted.bin")

    def test_tobinfile_single_file_with_uicr_content(self):
        nrf = nrfhex.nRFHex("firmwares/foo.hex")
        nrf.tobinfile("firmwares/foo.bin")

        self.comparefiles("firmwares/foo.bin", "firmwares/foo_wanted.bin")

    def test_tobinfile_single_bin_file(self):
        nrf = nrfhex.nRFHex("firmwares/bar_wanted.bin")
        nrf.tobinfile("firmwares/bar.bin")

        self.comparefiles("firmwares/bar.bin", "firmwares/bar_wanted.bin")

    def test_tobinfile_two_hex_files(self):
        nrf = nrfhex.nRFHex("firmwares/foo.hex", "firmwares/bar.hex")
        nrf.tobinfile("firmwares/foobar.bin")

        self.comparefiles("firmwares/foobar.bin", "firmwares/foobar_wanted.bin")

    def test_tobinfile_one_hex_one_bin(self):
        nrf = nrfhex.nRFHex("firmwares/foo_wanted.bin", "firmwares/bar.hex")
        nrf.tobinfile("firmwares/foobar.bin")

        self.comparefiles("firmwares/foobar.bin", "firmwares/foobar_wanted.bin")

    def test_tobinfile_one_bin_one_hex(self):
        nrf = nrfhex.nRFHex("firmwares/foo.hex", "firmwares/bar_wanted.bin")
        nrf.tobinfile("firmwares/foobar.bin")

        self.comparefiles("firmwares/foobar.bin", "firmwares/foobar_wanted.bin")

    def test_tobinfile_two_bin(self):
        nrf = nrfhex.nRFHex("firmwares/foo_wanted.bin", "firmwares/bar_wanted.bin")
        nrf.tobinfile("firmwares/foobar.bin")

        self.comparefiles("firmwares/foobar.bin", "firmwares/foobar_wanted.bin")

    def test_sizes(self):
        nrf = nrfhex.nRFHex("firmwares/foo.hex", "firmwares/bar.hex")

        self.assertEqual(nrf.get_mbr_end_address(), 0x1000)
        self.assertEqual(nrf.minaddr(), 0x1000)
        self.assertEqual(nrf.size(), 73152)
        self.assertEqual(nrf.bootloadersize(), 13192)

        nrf = nrfhex.nRFHex("firmwares/s132_nrf52_mini.hex")

        self.assertEqual(nrf.get_mbr_end_address(), 0x3000)
        self.assertEqual(nrf.minaddr(), 0x3000)
        self.assertEqual(nrf.size(), 12288)
        self.assertEqual(nrf.bootloadersize(), 0)

    def test_get_softdevice_variant(self):
        nrf = nrfhex.nRFHex("firmwares/foo.hex")

        self.assertEqual(nrf.get_softdevice_variant(), "unknown")

        nrf = nrfhex.nRFHex("firmwares/s130_nrf51_mini.hex")

        self.assertEqual(nrf.get_softdevice_variant(), "s1x0")

        nrf = nrfhex.nRFHex("firmwares/s132_nrf52_mini.hex")

        self.assertEqual(nrf.get_softdevice_variant(), "s132")


if __name__ == '__main__':
    unittest.main()
