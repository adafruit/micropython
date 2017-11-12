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

# Nordic libraries
from nordicsemi.exceptions import NordicSemiException


# TODO: Create query function that maps query-result strings with functions
def query_func(question, default=False):
    """
    Ask a string question
    No input defaults to "no" which results in False
    """
    valid = {"yes": True, "y": True, "no": False, "n": False}
    if default is True:
        prompt = " [Y/n]"
    else:
        prompt = " [y/N]"

    while True:
        print "%s %s" % (question, prompt)
        choice = raw_input().lower()
        if choice == '':
            return default
        elif choice in valid:
            return valid[choice]
        else:
            print "Please respond with y/n"


def convert_uint16_to_array(value):
    """
    Converts a int to an array of 2 bytes (little endian)

    :param int value: int value to convert to list
    :return list[int]: list with 2 bytes
    """
    byte0 = value & 0xFF
    byte1 = (value >> 8) & 0xFF
    return [byte0, byte1]


def convert_uint32_to_array(value):
    """
    Converts a int to an array of 4 bytes (little endian)

    :param int value: int value to convert to list
    :return list[int]: list with 4 bytes
    """
    byte0 = value & 0xFF
    byte1 = (value >> 8) & 0xFF
    byte2 = (value >> 16) & 0xFF
    byte3 = (value >> 24) & 0xFF
    return [byte0, byte1, byte2, byte3]


def slip_parts_to_four_bytes(seq, dip, rp, pkt_type, pkt_len):
    """
    Creates a SLIP header.

    For a description of the SLIP header go to:
    http://developer.nordicsemi.com/nRF51_SDK/doc/7.2.0/s110/html/a00093.html

    :param int seq: Packet sequence number
    :param int dip: Data integrity check
    :param int rp: Reliable packet
    :param pkt_type: Payload packet
    :param pkt_len: Packet length
    :return: str with SLIP header
    """
    ints = [0, 0, 0, 0]
    ints[0] = seq | (((seq + 1) % 8) << 3) | (dip << 6) | (rp << 7)
    ints[1] = pkt_type | ((pkt_len & 0x000F) << 4)
    ints[2] = (pkt_len & 0x0FF0) >> 4
    ints[3] = (~(sum(ints[0:3])) + 1) & 0xFF

    return ''.join(chr(b) for b in ints)


def int32_to_bytes(value):
    """
    Converts a int to a str with 4 bytes

    :param value: int value to convert
    :return: str with 4 bytes
    """
    ints = [0, 0, 0, 0]
    ints[0] = (value & 0x000000FF)
    ints[1] = (value & 0x0000FF00) >> 8
    ints[2] = (value & 0x00FF0000) >> 16
    ints[3] = (value & 0xFF000000) >> 24
    return ''.join(chr(b) for b in ints)


def int16_to_bytes(value):
    """
    Converts a int to a str with 4 bytes

    :param value: int value to convert
    :return: str with 4 bytes
    """

    ints = [0, 0]
    ints[0] = (value & 0x00FF)
    ints[1] = (value & 0xFF00) >> 8
    return ''.join(chr(b) for b in ints)


def slip_decode_esc_chars(data):
    """Decode esc characters in a SLIP package.

    Replaces 0xDBDC with 0xCO and 0xDBDD with 0xDB.

    :return: str decoded data
    :type str data: data to decode
    """
    result = []
    while len(data):
        char = data.pop(0)
        if char == 0xDB:
            char2 = data.pop(0)
            if char2 == 0xDC:
                result.append(0xC0)
            elif char2 == 0xDD:
                result.append(0xDB)
            else:
                raise NordicSemiException('Char 0xDB NOT followed by 0xDC or 0xDD')
        else:
            result.append(char)
    return result


def slip_encode_esc_chars(data_in):
    """Encode esc characters in a SLIP package.

    Replace 0xCO  with 0xDBDC and 0xDB with 0xDBDD.

     :type str data_in: str to encode
     :return: str with encoded packet
    """
    result = []
    data = []
    for i in data_in:
        data.append(ord(i))

    while len(data):
        char = data.pop(0)
        if char == 0xC0:
            result.extend([0xDB, 0xDC])
        elif char == 0xDB:
            result.extend([0xDB, 0xDD])
        else:
            result.append(char)
    return ''.join(chr(i) for i in result)
