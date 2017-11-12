# Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
#
# The information contained herein is property of Nordic Semiconductor ASA.
# Terms and conditions of usage are described in detail in NORDIC
# SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
#
# Licensees are granted free, non-transferable use of the information. NO
# WARRANTY of ANY KIND is provided. This heading must NOT be removed from
# the file.

import hashlib
import binascii

try:
    from ecdsa import SigningKey
    from ecdsa.curves import NIST256p
    from ecdsa.keys import sigencode_string
except Exception:
    print "Failed to import ecdsa, cannot do signing"

from nordicsemi.exceptions import InvalidArgumentException, IllegalStateException


class Signing(object):
    """
    Class for singing of hex-files
    """
    def gen_key(self, filename):
        """
        Generate a new Signing key using NIST P-256 curve
        """
        self.sk = SigningKey.generate(curve=NIST256p)

        with open(filename, "w") as sk_file:
            sk_file.write(self.sk.to_pem())

    def load_key(self, filename):
        """
        Load signing key (from pem file)
        """
        with open(filename, "r") as sk_file:
            sk_pem = sk_file.read()

        self.sk = SigningKey.from_pem(sk_pem)

        sk_hex = "".join(c.encode('hex') for c in self.sk.to_string())

    def sign(self, init_packet_data):
        """
        Create signature for init package using P-256 curve and SHA-256 as hashing algorithm
        Returns R and S keys combined in a 64 byte array
        """
        # Add assertion of init_packet
        if self.sk is None:
            raise IllegalStateException("Can't save key. No key created/loaded")

        # Sign the init-packet
        signature = self.sk.sign(init_packet_data, hashfunc=hashlib.sha256, sigencode=sigencode_string)
        return signature

    def verify(self, init_packet, signature):
        """
        Verify init packet
        """
        # Add assertion of init_packet
        if self.sk is None:
            raise IllegalStateException("Can't save key. No key created/loaded")

        vk = self.sk.get_verifying_key()

        # Verify init packet
        try:
            vk.verify(signature, init_packet, hashfunc=hashlib.sha256)
        except:
            return False

        return True

    def get_vk(self, output_type):
        """
        Get verification key (as hex, code or pem)
        """
        if self.sk is None:
            raise IllegalStateException("Can't get key. No key created/loaded")

        if output_type is None:
            raise InvalidArgumentException("Invalid output type for signature.")
        elif output_type == 'hex':
            return self.get_vk_hex()
        elif output_type == 'code':
            return self.get_vk_code()
        elif output_type == 'pem':
            return self.get_vk_pem()
        else:
            raise InvalidArgumentException("Invalid argument. Can't get key")

    def get_vk_hex(self):
        """
        Get the verification key as hex
        """
        if self.sk is None:
            raise IllegalStateException("Can't get key. No key created/loaded")

        vk = self.sk.get_verifying_key()
        vk_hexlify = binascii.hexlify(vk.to_string())

        vk_hex = "Verification key Qx: {0}\n".format(vk_hexlify[0:64])
        vk_hex += "Verification key Qy: {0}".format(vk_hexlify[64:128])

        return vk_hex

    def get_vk_code(self):
        """
        Get the verification key as code
        """
        if self.sk is None:
            raise IllegalStateException("Can't get key. No key created/loaded")

        vk = self.sk.get_verifying_key()
        vk_hex = binascii.hexlify(vk.to_string())

        vk_x_separated = ""
        vk_x_str = vk_hex[0:64]
        for i in xrange(0, len(vk_x_str), 2):
            vk_x_separated += "0x" + vk_x_str[i:i+2] + ", "
        vk_x_separated = vk_x_separated[:-2]

        vk_y_separated = ""
        vk_y_str = vk_hex[64:128]
        for i in xrange(0, len(vk_y_str), 2):
            vk_y_separated += "0x" + vk_y_str[i:i+2] + ", "
        vk_y_separated = vk_y_separated[:-2]

        vk_code = "static uint8_t Qx[] = {{ {0} }};\n".format(vk_x_separated)
        vk_code += "static uint8_t Qy[] = {{ {0} }};".format(vk_y_separated)

        return vk_code

    def get_vk_pem(self):
        """
        Get the verification key as PEM
        """
        if self.sk is None:
            raise IllegalStateException("Can't get key. No key created/loaded")

        vk = self.sk.get_verifying_key()
        vk_pem = vk.to_pem()

        return vk_pem
