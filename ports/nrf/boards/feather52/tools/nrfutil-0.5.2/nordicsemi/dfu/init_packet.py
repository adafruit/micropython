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

from enum import Enum
import struct


INIT_PACKET_USES_CRC16 = 0
INIT_PACKET_USES_HASH = 1
INIT_PACKET_EXT_USES_ECDS = 2


class PacketField(Enum):
    DEVICE_TYPE = 1
    DEVICE_REVISION = 2
    APP_VERSION = 3
    REQUIRED_SOFTDEVICES_ARRAY = 4
    OPT_DATA = 5
    NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID = 6
    NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_LENGTH = 7
    NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH = 8
    NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16 = 9
    NORDIC_PROPRIETARY_OPT_DATA_INIT_PACKET_ECDS = 10


class Packet(object):
    """
    Class that implements the INIT packet format.
    http://developer.nordicsemi.com/nRF51_SDK/doc/7.1.0/s110/html/a00065.html
    """

    UNSIGNED_SHORT = "H"
    UNSIGNED_INT = "I"
    UNSIGNED_CHAR = "B"
    CHAR_ARRAY = "s"

    def __init__(self, init_packet_fields):
        """

            :param init_packet_fields: Dictionary with packet fields
        """
        self.init_packet_fields = init_packet_fields

    def generate_packet(self):
        """
        Generates a binary packet from provided init_packet_fields provided in constructor.
        This version includes the extended data

        :return str: Returns a string representing the init_packet (in binary)

        """
        # Create struct format string based on keys that are
        # present in self.init_packet_fields
        format_string = self.__generate_struct_format_string()
        args = []

        # If you got error message AttributeError: 'int' object has no attribute 'value'.
        # Uncomment line 84 and comment out line 85 and run 'python setup.py install'
        #for key in sorted(self.init_packet_fields.keys(), key=lambda x: x):
        for key in sorted(self.init_packet_fields.keys(), key=lambda x: x.value):
            # Add length to fields that required that
            if key in [PacketField.REQUIRED_SOFTDEVICES_ARRAY,
                       PacketField.OPT_DATA]:
                args.append(len(self.init_packet_fields[key]))
                args.extend(self.init_packet_fields[key])
            else:
                args.append(self.init_packet_fields[key])

        return struct.pack(format_string, *args)

    def __generate_struct_format_string(self):
        format_string = "<"  # Use little endian format with standard sizes for python,
        # see https://docs.python.org/2/library/struct.html

        # If you got error message AttributeError: 'int' object has no attribute 'value'.
        # Uncomment line 102 and comment out line 103 and run 'python setup.py install'
        #for key in sorted(self.init_packet_fields.keys(), key=lambda x: x):
        for key in sorted(self.init_packet_fields.keys(), key=lambda x: x.value):
            if key in [PacketField.DEVICE_TYPE,
                       PacketField.DEVICE_REVISION,
                       ]:
                format_string += Packet.UNSIGNED_SHORT

            elif key in [PacketField.APP_VERSION]:
                format_string += Packet.UNSIGNED_INT
            elif key in [PacketField.REQUIRED_SOFTDEVICES_ARRAY]:
                array_elements = self.init_packet_fields[key]
                format_string += Packet.UNSIGNED_SHORT  # Add length field to format packet

                for _ in range(len(array_elements)):
                    format_string += Packet.UNSIGNED_SHORT
            elif key in [PacketField.OPT_DATA]:
                format_string += Packet.UNSIGNED_SHORT  # Add length field to optional data
                format_string += "{0}{1}".format(len(self.init_packet_fields[key]), Packet.CHAR_ARRAY)
            elif key in [PacketField.NORDIC_PROPRIETARY_OPT_DATA_EXT_PACKET_ID]:
                format_string += Packet.UNSIGNED_INT  # Add the extended packet id field
            elif key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_LENGTH:
                format_string += Packet.UNSIGNED_INT  # Add the firmware length field
            elif key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_HASH:
                format_string += "32{0}".format(Packet.CHAR_ARRAY)  # SHA-256 requires 32 bytes
            elif key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_FIRMWARE_CRC16:
                format_string += Packet.UNSIGNED_SHORT
            elif key == PacketField.NORDIC_PROPRIETARY_OPT_DATA_INIT_PACKET_ECDS:
                format_string += "64{0}".format(Packet.CHAR_ARRAY)  # ECDS based on P-256 using SHA-256 requires 64 bytes

        return format_string
