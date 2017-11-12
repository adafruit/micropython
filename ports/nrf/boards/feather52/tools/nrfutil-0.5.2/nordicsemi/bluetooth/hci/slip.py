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

logger = logging.getLogger(__name__)


class Slip(object):
    def __init__(self):
        self.SLIP_END = '\xc0'
        self.SLIP_ESC = '\xdb'
        self.SLIP_ESC_END = '\xdc'
        self.SLIP_ESC_ESC = '\xdd'

        self.started = False
        self.escaped = False
        self.stream = ''
        self.packet = ''

    def append(self, data):
        """
        Append a new
        :param data: Append a new block of data to do decoding on when calling decode.
        The developer may add more than one SLIP packet before calling decode.
        :return:
        """
        self.stream += data

    def decode(self):
        """
        Decodes a package according to http://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol
        :return Slip: A list of decoded slip packets
        """
        packet_list = list()

        for char in self.stream:
            if char == self.SLIP_END:
                if self.started:
                    if len(self.packet) > 0:
                        self.started = False
                        packet_list.append(self.packet)
                        self.packet = ''
                else:
                    self.started = True
                    self.packet = ''
            elif char == self.SLIP_ESC:
                self.escaped = True
            elif char == self.SLIP_ESC_END:
                if self.escaped:
                    self.packet += self.SLIP_END
                    self.escaped = False
                else:
                    self.packet += char
            elif char == self.SLIP_ESC_ESC:
                if self.escaped:
                    self.packet += self.SLIP_ESC
                    self.escaped = False
                else:
                    self.packet += char
            else:
                if self.escaped:
                    logging.error("Error in SLIP packet, ignoring error.")
                    self.packet = ''
                    self.escaped = False
                else:
                    self.packet += char

        self.stream = ''

        return packet_list

    def encode(self, packet):
        """
        Encode a packet according to SLIP.
        :param packet: A str array that represents the package
        :return: str array with an encoded SLIP packet
        """
        encoded = self.SLIP_END

        for char in packet:
            if char == self.SLIP_END:
                encoded += self.SLIP_ESC + self.SLIP_ESC_END
            elif char == self.SLIP_ESC:
                encoded += self.SLIP_ESC + self.SLIP_ESC_ESC
            else:
                encoded += char
        encoded += self.SLIP_END

        return encoded
