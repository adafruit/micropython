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

UART_HEADER_OCTET_COUNT = 4


class ThreeWireUartPacket(object):
    """
    This class encapsulate a three wire uart packet according to Bluetooth specification
    version 4.0 [Vol 4] part D.
    """
    def __init__(self):
        self.ack = None  # Acknowledgement number
        self.seq = None  # Sequence number
        self.di = None  # Data integrity present
        self.rp = None  # Reliable packet
        self.type = None  # Packet type
        self.length = None  # Payload Length
        self.checksum = None  # Header checksum
        self.payload = None  # Payload

    @staticmethod
    def decode(packet):
        """
        Decodes a packet from a str encoded array

        :param packet_bytes: A str encoded array
        :return: TheeWireUartPacket
        """

        decoded_packet = ThreeWireUartPacket()

        packet_bytes = bytearray(packet)

        decoded_packet.ack = (packet_bytes[0] & int('38', 16)) >> 3
        decoded_packet.seq = (packet_bytes[0] & int('07', 16))
        decoded_packet.di = (packet_bytes[0] & int('40', 16)) >> 6
        decoded_packet.rp = (packet_bytes[0] & int('80', 16)) >> 7
        decoded_packet.type = (packet_bytes[1] & int('0F', 16))
        decoded_packet.length = ((packet_bytes[1] & int('F0', 16)) >> 4) + (packet_bytes[2] * 16)

        checksum = packet_bytes[0]
        checksum = checksum + packet_bytes[1]
        checksum = checksum + packet_bytes[2]
        checksum &= int('FF', 16)
        decoded_packet.checksum = (~checksum + 1) & int('FF', 16)

        if decoded_packet.length > 0:
            decoded_packet.payload = packet_bytes[UART_HEADER_OCTET_COUNT:-1]

        return decoded_packet
