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

# 3rd party libraries
from zipfile import ZipFile
import hashlib


# Nordic libraries
from nordicsemi.exceptions import NordicSemiException
from nordicsemi.dfu.nrfhex import *
from nordicsemi.dfu.init_packet import *
from nordicsemi.dfu.manifest import ManifestGenerator, Manifest
from nordicsemi.dfu.model import HexType, FirmwareKeys
from nordicsemi.dfu.crc16 import *

from signing import Signing


class Package(object):
    """
        Packages and unpacks Nordic DFU packages. Nordic DFU packages are zip files that contains firmware and meta-information
        necessary for utilities to perform a DFU on nRF5X devices.

        The internal data model used in Package is a dictionary. The dictionary is expressed like this in
         json format:

         {
            "manifest": {
                "bootloader": {
                    "bin_file": "asdf.bin",
                    "dat_file": "asdf.dat",
                    "init_packet_data": {
                        "application_version": null,
                        "device_revision": null,
                        "device_type": 5,
                        "firmware_hash": "asdfasdkfjhasdkfjashfkjasfhaskjfhkjsdfhasjkhf",
                        "softdevice_req": [
                            17,
                            18
                        ]
                    }
                }
        }

        Attributes application, bootloader, softdevice, softdevice_bootloader shall not be put into the manifest if they are null

    """

    DEFAULT_DEV_TYPE = 0xFFFF
    DEFAULT_DEV_REV = 0xFFFF
    DEFAULT_APP_VERSION = 0xFFFFFFFF
    DEFAULT_SD_REQ = [0xFFFE]
    DEFAULT_DFU_VER = 0.5
    MANIFEST_FILENAME = "manifest.json"

    def __init__(self,
                 dev_type=DEFAULT_DEV_TYPE,
                 dev_rev=DEFAULT_DEV_REV,
                 app_version=DEFAULT_APP_VERSION,
                 sd_req=DEFAULT_SD_REQ,
                 app_fw=None,
                 bootloader_fw=None,
                 softdevice_fw=None,
                 dfu_ver=DEFAULT_DFU_VER,
                 key_file=None):
        """
        Constructor that requires values used for generating a Nordic DFU package.

        :param int dev_type: Device type init-packet field
        :param int dev_rev: Device revision init-packet field
        :param int app_version: App version init-packet field
        :param list sd_req: Softdevice Requirement init-packet field
        :param str app_fw: Path to application firmware file
        :param str bootloader_fw: Path to bootloader firmware file
        :param str softdevice_fw: Path to softdevice firmware file
        :param float dfu_ver: DFU version to use when generating init-packet
        :param str key_file: Path to Signing key file (PEM)
        :return: None
        """
        self.dfu_ver = dfu_ver

        init_packet_vars = {}

        if dev_type is not None:
            init_packet_vars[PacketField.DEVICE_TYPE] = dev_type

        if dev_rev is not None:
            init_packet_vars[PacketField.DEVICE_REVISION] = dev_rev

        if app_version is not None:
            init_packet_vars[PacketField.APP_VERSION] = app_version

        if sd_req is not None:
            init_packet_vars[PacketField.REQUIRED_SOFTDEVICES_ARRAY] = sd_req

        self.firmwares_data = {}

        if app_fw:
            self.__add_firmware_info(HexType.APPLICATION,
                                     app_fw,
                                     init_packet_vars)

        if bootloader_fw:
            self.__add_firmware_info(HexType.BOOTLOADER,
                                     bootloader_fw,
                                     init_packet_vars)

        if softdevice_fw:
            self.__add_firmware_info(HexType.SOFTDEVICE,
                                     softdevice_fw,
                                     init_packet_vars)

        if key_file:
            self.dfu_ver = 0.8
            self.key_file = key_file

    def generate_package(self, filename, preserve_work_directory=False):
        """
        Generates a Nordic DFU package. The package is a zip file containing firmware(s) and metadata required
        for Nordic DFU applications to perform DFU onn nRF5X devices.

        :param str filename: Filename for generated package.
        :param bool preserve_work_directory: True to preserve the temporary working directory.
        Useful for debugging of a package, and if the user wants to look at the generated package without having to
        unzip it.
        :return: None
        """
        work_directory = self.__create_temp_workspace()

        if Package._is_bootloader_softdevice_combination(self.firmwares_data):
            # Removing softdevice and bootloader data from dictionary and adding the combined later
            softdevice_fw_data = self.firmwares_data.pop(HexType.SOFTDEVICE)
            bootloader_fw_data = self.firmwares_data.pop(HexType.BOOTLOADER)

            softdevice_fw_name = softdevice_fw_data[FirmwareKeys.FIRMWARE_FILENAME]
            bootloader_fw_name = bootloader_fw_data[FirmwareKeys.FIRMWARE_FILENAME]

            new_filename = "sd_bl.bin"
            sd_bl_file_path = os.path.join(work_directory, new_filename)

            nrf_hex = nRFHex(softdevice_fw_name, bootloader_fw_name)
            nrf_hex.tobinfile(sd_bl_file_path)

            softdevice_size = nrf_hex.size()
            bootloader_size = nrf_hex.bootloadersize()

            self.__add_firmware_info(HexType.SD_BL,
                                     sd_bl_file_path,
                                     softdevice_fw_data[FirmwareKeys.INIT_PACKET_DATA],
                                     softdevice_size,
                                     bootloader_size)

        for key in self.firmwares_data:
            firmware = self.firmwares_data[key]

            # Normalize the firmware file and store it in the work directory
            firmware[FirmwareKeys.BIN_FILENAME] = \
                Package.normalize_firmware_to_bin(work_directory, firmware[FirmwareKeys.FIRMWARE_FILENAME])

            # Calculate the hash for the .bin file located in the work directory
            bin_file_path = os.path.join(work_directory, firmware[FirmwareKeys.BIN_FILENAME])

            init_packet_data = firmware[FirmwareKeys.INIT_PACKET_DATA]

            if self.dfu_ver <= 0.5:
                firmware_hash = Package.calculate_crc16(bin_file_path)
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16] = firmware_hash
            elif self.dfu_ver == 0.6:
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID] = INIT_PACKET_USES_CRC16
                firmware_hash = Package.calculate_crc16(bin_file_path)
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16] = firmware_hash
            elif self.dfu_ver == 0.7:
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID] = INIT_PACKET_USES_HASH
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_LENGTH] = int(Package.calculate_file_size(bin_file_path))
                firmware_hash = Package.calculate_sha256_hash(bin_file_path)
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH] = firmware_hash
            elif self.dfu_ver == 0.8:
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID] = INIT_PACKET_EXT_USES_ECDS
                firmware_hash = Package.calculate_sha256_hash(bin_file_path)
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_LENGTH] = int(Package.calculate_file_size(bin_file_path))
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH] = firmware_hash
                temp_packet = self._create_init_packet(firmware)
                signer = Signing()
                signer.load_key(self.key_file)
                signature = signer.sign(temp_packet)
                init_packet_data[PacketField.NORDIC_PROPRIETARY_OPT_DATA_INIT_PACKET_ECDS] = signature

            # Store the .dat file in the work directory
            init_packet = self._create_init_packet(firmware)
            init_packet_filename = firmware[FirmwareKeys.BIN_FILENAME].replace(".bin", ".dat")

            with open(os.path.join(work_directory, init_packet_filename), 'wb') as init_packet_file:
                init_packet_file.write(init_packet)

            firmware[FirmwareKeys.DAT_FILENAME] = \
                init_packet_filename

        # Store the manifest to manifest.json
        manifest = self.create_manifest()

        with open(os.path.join(work_directory, Package.MANIFEST_FILENAME), "w") as manifest_file:
            manifest_file.write(manifest)

        # Package the work_directory to a zip file
        Package.create_zip_package(work_directory, filename)

        # Delete the temporary directory
        if not preserve_work_directory:
            shutil.rmtree(work_directory)

    @staticmethod
    def __create_temp_workspace():
        return tempfile.mkdtemp(prefix="nrf_dfu_")

    @staticmethod
    def create_zip_package(work_directory, filename):
        files = os.listdir(work_directory)

        with ZipFile(filename, 'w') as package:
            for _file in files:
                file_path = os.path.join(work_directory, _file)
                package.write(file_path, _file)

    @staticmethod
    def calculate_file_size(firmware_filename):
        b = os.path.getsize(firmware_filename)
        return b

    @staticmethod
    def calculate_sha256_hash(firmware_filename):
        read_buffer = 4096

        digest = hashlib.sha256()

        with open(firmware_filename, 'rb') as firmware_file:
            while True:
                data = firmware_file.read(read_buffer)

                if data:
                    digest.update(data)
                else:
                    break

        return digest.digest()

    @staticmethod
    def calculate_crc16(firmware_filename):
        """
        Calculates CRC16 has on provided firmware filename

        :type str firmware_filename:
        """
        data_buffer = b''
        read_size = 4096

        with open(firmware_filename, 'rb') as firmware_file:
            while True:
                data = firmware_file.read(read_size)

                if data:
                    data_buffer += data
                else:
                    break

        return calc_crc16(data_buffer, 0xffff)

    def create_manifest(self):
        manifest = ManifestGenerator(self.dfu_ver, self.firmwares_data)
        return manifest.generate_manifest()

    @staticmethod
    def _is_bootloader_softdevice_combination(firmwares):
        return (HexType.BOOTLOADER in firmwares) and (HexType.SOFTDEVICE in firmwares)

    def __add_firmware_info(self, firmware_type, filename, init_packet_data, sd_size=None, bl_size=None):
        self.firmwares_data[firmware_type] = {
            FirmwareKeys.FIRMWARE_FILENAME: filename,
            FirmwareKeys.INIT_PACKET_DATA: init_packet_data.copy(),
            # Copying init packet to avoid using the same for all firmware
            }

        if firmware_type == HexType.SD_BL:
            self.firmwares_data[firmware_type][FirmwareKeys.SD_SIZE] = sd_size
            self.firmwares_data[firmware_type][FirmwareKeys.BL_SIZE] = bl_size

    @staticmethod
    def _create_init_packet(firmware_data):
        p = Packet(firmware_data[FirmwareKeys.INIT_PACKET_DATA])
        return p.generate_packet()

    @staticmethod
    def normalize_firmware_to_bin(work_directory, firmware_path):
        firmware_filename = os.path.basename(firmware_path)
        new_filename = firmware_filename.replace(".hex", ".bin")
        new_filepath = os.path.join(work_directory, new_filename)

        if not os.path.exists(new_filepath):
            temp = nRFHex(firmware_path)
            temp.tobinfile(new_filepath)

        return new_filepath

    @staticmethod
    def unpack_package(package_path, target_dir):
        """
        Unpacks a Nordic DFU package.

        :param str package_path: Path to the package
        :param str target_dir: Target directory to unpack the package to
        :return: Manifest Manifest: Returns a manifest back to the user. The manifest is a parse datamodel
        of the manifest found in the Nordic DFU package.
        """

        if not os.path.isfile(package_path):
            raise NordicSemiException("Package {0} not found.".format(package_path))

        target_dir = os.path.abspath(target_dir)
        target_base_path = os.path.dirname(target_dir)

        if not os.path.exists(target_base_path):
            raise NordicSemiException("Base path to target directory {0} does not exist.".format(target_base_path))

        if not os.path.isdir(target_base_path):
            raise NordicSemiException("Base path to target directory {0} is not a directory.".format(target_base_path))

        if os.path.exists(target_dir):
            raise NordicSemiException(
                "Target directory {0} exists, not able to unpack to that directory.",
                target_dir)

        with ZipFile(package_path, 'r') as pkg:
            pkg.extractall(target_dir)

            with open(os.path.join(target_dir, Package.MANIFEST_FILENAME), 'r') as f:
                _json = f.read()
                """:type :str """

                return Manifest.from_json(_json)
