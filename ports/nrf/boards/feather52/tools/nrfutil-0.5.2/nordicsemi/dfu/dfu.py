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

# Python standard library
import os
import tempfile
import shutil
import logging
from time import time, sleep
from datetime import datetime, timedelta

# Nordic libraries
from nordicsemi.exceptions import *
from nordicsemi.dfu.package import Package
from nordicsemi.dfu.dfu_transport import DfuEvent
from nordicsemi.dfu.model import HexType
from nordicsemi.dfu.manifest import SoftdeviceBootloaderFirmware

logger = logging.getLogger(__name__)


class Dfu(object):
    """ Class to handle upload of a new hex image to the device. """

    def __init__(self, zip_file_path, dfu_transport):
        """
        Initializes the dfu upgrade, unpacks zip and registers callbacks.

        @param zip_file_path: Path to the zip file with the firmware to upgrade
        @type zip_file_path: str
        @param dfu_transport: Transport backend to use to upgrade
        @type dfu_transport: nordicsemi.dfu.dfu_transport.DfuTransport
        @return
        """
        self.zip_file_path = zip_file_path
        self.ready_to_send = True
        self.response_opcode_received = None

        self.temp_dir = tempfile.mkdtemp(prefix="nrf_dfu_")
        self.unpacked_zip_path = os.path.join(self.temp_dir, 'unpacked_zip')
        self.manifest = Package.unpack_package(self.zip_file_path, self.unpacked_zip_path)

        if dfu_transport:
            self.dfu_transport = dfu_transport

        self.dfu_transport.register_events_callback(DfuEvent.TIMEOUT_EVENT, self.timeout_event_handler)
        self.dfu_transport.register_events_callback(DfuEvent.ERROR_EVENT, self.error_event_handler)

    def __del__(self):
        """
        Destructor removes the temporary directory for the unpacked zip
        :return:
        """
        shutil.rmtree(self.temp_dir)

    def error_event_handler(self, log_message=""):
        """
        Event handler for errors, closes the transport backend.
        :param str log_message: The log message for the error.
        :return:
        """
        if self.dfu_transport.is_open():
            self.dfu_transport.close()

        logger.error(log_message)

    def timeout_event_handler(self, log_message):
        """
        Event handler for timeouts, closes the transport backend.
        :param log_message: The log message for the timeout.
        :return:
        """
        if self.dfu_transport.is_open():
            self.dfu_transport.close()

        logger.error(log_message)

    @staticmethod
    def _read_file(file_path):
        """
        Reads a file and returns the content as a string.

        :param str file_path: The path to the file to read.
        :return str: Content of the file.
        """
        buffer_size = 4096

        file_content = ""

        with open(file_path, 'rb') as binary_file:
            while True:
                data = binary_file.read(buffer_size)

                if data:
                    file_content += data
                else:
                    break

        return file_content

    def _wait_while_opening_transport(self):
        timeout = 10
        start_time = datetime.now()

        while not self.dfu_transport.is_open():
            timed_out = datetime.now() - start_time > timedelta(0, timeout)

            if timed_out:
                log_message = "Failed to open transport backend"
                raise NordicSemiException(log_message)

            sleep(0.1)


    def _dfu_send_image(self, program_mode, firmware_manifest):
        """
        Does DFU for one image. Reads the firmware image and init file.
        Opens the transport backend, calls setup, send and finalize and closes the backend again.
        @param program_mode: What type of firmware the DFU is
        @type program_mode: nordicsemi.dfu.model.HexType
        @param firmware_manifest: The manifest for the firmware image
        @type firmware_manifest: nordicsemi.dfu.manifest.Firmware
        @return:
        """

        if firmware_manifest is None:
            raise MissingArgumentException("firmware_manifest must be provided.")

        if self.dfu_transport.is_open():
            raise IllegalStateException("Transport is already open.")

        self.dfu_transport.open()
        self._wait_while_opening_transport()

        softdevice_size = 0
        bootloader_size = 0
        application_size = 0

        bin_file_path = os.path.join(self.unpacked_zip_path, firmware_manifest.bin_file)
        firmware = self._read_file(bin_file_path)

        dat_file_path = os.path.join(self.unpacked_zip_path, firmware_manifest.dat_file)
        init_packet = self._read_file(dat_file_path)

        if program_mode == HexType.SD_BL:
            if not isinstance(firmware_manifest, SoftdeviceBootloaderFirmware):
                raise NordicSemiException("Wrong type of manifest")
            softdevice_size = firmware_manifest.sd_size
            bootloader_size = firmware_manifest.bl_size
            firmware_size = len(firmware)
            if softdevice_size + bootloader_size != firmware_size:
                raise NordicSemiException(
                    "Size of bootloader ({} bytes) and softdevice ({} bytes)"
                    " is not equal to firmware provided ({} bytes)".format(
                    bootloader_size, softdevice_size, firmware_size))

        elif program_mode == HexType.SOFTDEVICE:
            softdevice_size = len(firmware)

        elif program_mode == HexType.BOOTLOADER:
            bootloader_size = len(firmware)

        elif program_mode == HexType.APPLICATION:
            application_size = len(firmware)

        start_time = time()
        logger.info("Starting DFU upgrade of type %s, SoftDevice size: %s, bootloader size: %s, application size: %s",
                    program_mode,
                    softdevice_size,
                    bootloader_size,
                    application_size)

        #logger.info("Sending DFU start packet, afterwards we wait for the flash on "
        #            "target to be initialized before continuing.")
        logger.info("Sending DFU start packet")
        self.dfu_transport.send_start_dfu(program_mode, softdevice_size, bootloader_size,
                                          application_size)

        logger.info("Sending DFU init packet")
        self.dfu_transport.send_init_packet(init_packet)

        logger.info("Sending firmware file")
        self.dfu_transport.send_firmware(firmware)

        self.dfu_transport.send_validate_firmware()

        self.dfu_transport.send_activate_firmware()

        end_time = time()
        logger.info("\nDFU upgrade took {0}s".format(end_time - start_time))

        self.dfu_transport.close()

    def dfu_send_images(self):
        """
        Does DFU for all firmware images in the stored manifest.
        :return:
        """
        if self.manifest.softdevice_bootloader:
            self._dfu_send_image(HexType.SD_BL, self.manifest.softdevice_bootloader)

        if self.manifest.softdevice:
            self._dfu_send_image(HexType.SOFTDEVICE, self.manifest.softdevice)

        if self.manifest.bootloader:
            self._dfu_send_image(HexType.BOOTLOADER, self.manifest.bootloader)

        if self.manifest.application:
            self._dfu_send_image(HexType.APPLICATION, self.manifest.application)
