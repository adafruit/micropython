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

# Python libraries
import json
import binascii
import os

# Nordic libraries
from nordicsemi.exceptions import NotImplementedException
from nordicsemi.dfu.init_packet import PacketField
from nordicsemi.dfu.model import HexType, FirmwareKeys


class ManifestGenerator(object):
    def __init__(self, dfu_version, firmwares_data):
        """
        The Manifest Generator constructor. Needs a data structure to generate a manifest from.

        :type float dfu_version: The dfu version number to state in manifest
        :type dict firmwares_data: The firmwares data structure describing the Nordic DFU package
        """
        self.dfu_version = dfu_version
        self.firmwares_data = firmwares_data
        self.manifest = None

    def generate_manifest(self):
        self.manifest = Manifest()
        self.manifest.dfu_version = self.dfu_version

        for key in self.firmwares_data:
            firmware_dict = self.firmwares_data[key]

            if key == HexType.SD_BL:
                _firmware = SoftdeviceBootloaderFirmware()
                _firmware.bl_size = firmware_dict[FirmwareKeys.BL_SIZE]
                _firmware.sd_size = firmware_dict[FirmwareKeys.SD_SIZE]
            else:
                _firmware = Firmware()

            # Strip path, add only filename
            _firmware.bin_file = os.path.basename(firmware_dict[FirmwareKeys.BIN_FILENAME])
            _firmware.dat_file = os.path.basename(firmware_dict[FirmwareKeys.DAT_FILENAME])

            init_packet_data = InitPacketData()

            for init_packet_data_key in firmware_dict[FirmwareKeys.INIT_PACKET_DATA]:
                field = firmware_dict[FirmwareKeys.INIT_PACKET_DATA][init_packet_data_key]

                if init_packet_data_key == PacketField.APP_VERSION:
                    init_packet_data.application_version = field
                elif init_packet_data_key == PacketField.DEVICE_TYPE:
                    init_packet_data.device_type = field
                elif init_packet_data_key == PacketField.DEVICE_REVISION:
                    init_packet_data.device_revision = field
                elif init_packet_data_key == PacketField.REQUIRED_SOFTDEVICES_ARRAY:
                    init_packet_data.softdevice_req = field
                elif init_packet_data_key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID:
                    init_packet_data.ext_packet_id = field
                elif init_packet_data_key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_LENGTH:
                    init_packet_data.firmware_length = field
                elif init_packet_data_key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH:
                    init_packet_data.firmware_hash = binascii.hexlify(field)
                elif init_packet_data_key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16:
                    init_packet_data.firmware_crc16 = field
                elif init_packet_data_key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_INIT_PACKET_ECDS:
                    init_packet_data.init_packet_ecds = binascii.hexlify(field)
                else:
                    raise NotImplementedException(
                        "Support for init packet data type {0} not implemented yet.".format(init_packet_data_key))

                _firmware.init_packet_data = init_packet_data

            if key == HexType.APPLICATION:
                self.manifest.application = _firmware
            elif key == HexType.BOOTLOADER:
                self.manifest.bootloader = _firmware
            elif key == HexType.SOFTDEVICE:
                self.manifest.softdevice = _firmware
            elif key == HexType.SD_BL:
                self.manifest.softdevice_bootloader = _firmware
            else:
                raise NotImplementedException("Support for firmware type {0} not implemented yet.".format(key))

        return self.to_json()

    def to_json(self):
        def remove_none_entries(d):
            if not isinstance(d, dict):
                return d

            return dict((k, remove_none_entries(v)) for k, v in d.iteritems() if v is not None)

        return json.dumps({'manifest': self.manifest},
                          default=lambda o: remove_none_entries(o.__dict__),
                          sort_keys=True, indent=4,
                          separators=(',', ': '))


class InitPacketData(object):
    def __init__(self,
                 device_type=None,
                 device_revision=None,
                 application_version=None,
                 softdevice_req=None,
                 ext_packet_id=None,
                 firmware_length=None,
                 firmware_hash=None,
                 firmware_crc16=None,
                 init_packet_ecds=None
                 ):
        """
        The InitPacketData data model.

        :param int device_type:  device type
        :param int device_revision: device revision
        :param int application_version:  application version
        :param list softdevice_req: softdevice requirements
        :param int ext_packet_id: packet extension id
        :param int firmware_length: firmware length
        :param str firmware_hash: firmware hash
        :param int firmware_crc16: firmware CRC-16 calculated value
        :param str init_packet_ecds: Init packet signature
        :return: InitPacketData
        """
        self.device_type = device_type
        self.device_revision = device_revision
        self.application_version = application_version
        self.softdevice_req = softdevice_req
        self.ext_packet_id = ext_packet_id
        self.firmware_length = firmware_length
        self.firmware_hash = firmware_hash
        self.firmware_crc16 = firmware_crc16
        self.init_packet_ecds = init_packet_ecds


class Firmware(object):
    def __init__(self,
                 bin_file=None,
                 dat_file=None,
                 init_packet_data=None):
        """
        The firmware datamodel

        :param str bin_file: Firmware binary file
        :param str dat_file: Firmware .dat file (init packet for Nordic DFU)
        :param dict init_packet_data:  Initial packet data
        :return:
        """
        self.dat_file = dat_file
        self.bin_file = bin_file

        if init_packet_data:
            self.init_packet_data = InitPacketData(**init_packet_data)


class SoftdeviceBootloaderFirmware(Firmware):
    def __init__(self,
                 bin_file=None,
                 dat_file=None,
                 init_packet_data=None,
                 sd_size=None,
                 bl_size=None):
        """
        The SoftdeviceBootloaderFirmware data model

        :param str bin_file: Firmware binary file
        :param str dat_file: Firmware .dat file (init packet for Nordic DFU)
        :param int sd_size: The softdevice size
        :param int bl_size: The bootloader size
        :return: SoftdeviceBootloaderFirmware
        """
        super(SoftdeviceBootloaderFirmware, self).__init__(
            bin_file,
            dat_file,
            init_packet_data)
        self.sd_size = sd_size
        self.bl_size = bl_size


class Manifest:
    def __init__(self,
                 application=None,
                 bootloader=None,
                 softdevice=None,
                 softdevice_bootloader=None,
                 dfu_version=None):
        """
        The Manifest data model.

        :param dict application: Application firmware in package
        :param dict bootloader: Bootloader firmware in package
        :param dict softdevice: Softdevice firmware in package
        :param dict softdevice_bootloader: Combined softdevice and bootloader firmware in package
        :return: Manifest
        """
        self.softdevice_bootloader = \
            SoftdeviceBootloaderFirmware(**softdevice_bootloader) if softdevice_bootloader else None

        self.softdevice = Firmware(**softdevice) if softdevice else None
        self.bootloader = Firmware(**bootloader) if bootloader else None
        self.application = Firmware(**application) if application else None
        self.dfu_version = dfu_version

    @staticmethod
    def from_json(data):
        """
        Parses a manifest according to Nordic DFU package specification.

        :param str data: The manifest in string format
        :return: Manifest
        """
        kwargs = json.loads(data)
        return Manifest(**kwargs['manifest'])
