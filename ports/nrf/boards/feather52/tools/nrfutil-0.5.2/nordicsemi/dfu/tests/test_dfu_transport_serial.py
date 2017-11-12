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

import logging
import os
import unittest

# Nordic Semiconductor imports
import sys
from nordicsemi.dfu.dfu_transport import DfuEvent
from nordicsemi.dfu import crc16
from nordicsemi.dfu.init_packet import PacketField, Packet
from nordicsemi.dfu.model import HexType
from nordicsemi.dfu.dfu_transport_serial import DfuTransportSerial


def setup_logging():
    root = logging.getLogger()
    root.setLevel(logging.DEBUG)

    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.DEBUG)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    root.addHandler(ch)


@unittest.skip('Ignoring these tests since they take too much time to run.')
class TestDfuTransportSerial(unittest.TestCase):
    DEVKEY_PORT = "NORDICSEMI_PCA10028_1_PORT"

    def setUp(self):
        setup_logging()

        # Assert that environment variables are setUp before starting tests.
        # TODO: create generic functionality for fetching environment variables that map
        # TODO: communication ports to PCA versions
        # TODO: setup target nRF5X device to a given state (bootloader+sd+application)
        if self.DEVKEY_PORT not in os.environ:
            self.fail("Environment variable {0} not found. "
                      "Must specify serial port with development kit connected."
                      .format(self.DEVKEY_PORT))

        self.transport = DfuTransportSerial(os.environ[self.DEVKEY_PORT],
                                            baud_rate=38400,
                                            flow_control=True)

    def tearDown(self):
        if self.transport and self.transport.is_open():
            self.transport.close()

    def test_open_close(self):
        self.transport.open()
        self.assertTrue(self.transport.is_open())
        self.transport.close()
        self.assertFalse(self.transport.is_open())

    def test_dfu_methods(self):
        def timeout_callback(log_message):
            logging.debug("timeout_callback. Message: %s", log_message)

        def progress_callback(progress, log_message, done):
            logging.debug("Log message: %s, Progress: %d, done: %s", log_message, progress, done)

        def error_callback(log_message=""):
            logging.error("Log message: %s", log_message)

        self.transport.register_events_callback(DfuEvent.TIMEOUT_EVENT, timeout_callback)
        self.transport.register_events_callback(DfuEvent.PROGRESS_EVENT, progress_callback)
        self.transport.register_events_callback(DfuEvent.ERROR_EVENT, error_callback())

        firmware = ''
        test_firmware_path = os.path.join("firmwares", "pca10028_nrf51422_xxac_blinky.bin")

        with open(test_firmware_path, 'rb') as f:
            while True:
                data = f.read()

                if data:
                    firmware += data
                else:
                    break

        crc = crc16.calc_crc16(firmware, 0xffff)

        self.transport.open()

        # Sending start DFU command to target
        self.transport.send_start_dfu(HexType.APPLICATION,
                                      app_size=len(firmware),
                                      softdevice_size=0,
                                      bootloader_size=0)

        # Sending DFU init packet to target
        init_packet_vars = {
            PacketField.DEVICE_TYPE: 1,
            PacketField.DEVICE_REVISION: 2,
            PacketField.APP_VERSION: 0xfffa,
            PacketField.REQUIRED_SOFTDEVICES_ARRAY: [0x005a],
            PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16: crc
        }
        pkt = Packet(init_packet_vars)
        self.transport.send_init_packet(pkt.generate_packet())

        # Sending firmware to target
        self.transport.send_firmware(firmware)

        # Validating firmware
        self.transport.send_validate_firmware()
        self.transport.send_activate_firmware()
        self.transport.close()
