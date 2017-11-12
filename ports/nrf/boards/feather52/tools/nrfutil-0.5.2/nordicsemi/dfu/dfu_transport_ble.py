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
from time import sleep
from datetime import datetime, timedelta
import abc
import logging

# Nordic libraries
from nordicsemi.exceptions import NordicSemiException, IllegalStateException
from nordicsemi.dfu.util import int16_to_bytes
from nordicsemi.dfu.dfu_transport import DfuTransport, DfuEvent

logger = logging.getLogger(__name__)


# BLE DFU OpCodes :
class DfuOpcodesBle(object):
    """ DFU opcodes used during DFU communication with bootloader

        See http://developer.nordicsemi.com/nRF51_SDK/doc/7.2.0/s110/html/a00949.html#gafa9a52a3e6c43ccf00cf680f944d67a3
        for further information
    """
    INVALID_OPCODE = 0
    START_DFU = 1
    INITIALIZE_DFU = 2
    RECEIVE_FIRMWARE_IMAGE = 3
    VALIDATE_FIRMWARE_IMAGE = 4
    ACTIVATE_FIRMWARE_AND_RESET = 5
    SYSTEM_RESET = 6
    REQ_PKT_RCPT_NOTIFICATION = 8
    RESPONSE = 16
    PKT_RCPT_NOTIF = 17


class DfuErrorCodeBle(object):
    """ DFU error code used during DFU communication with bootloader

        See http://developer.nordicsemi.com/nRF51_SDK/doc/7.2.0/s110/html/a00949.html#gafa9a52a3e6c43ccf00cf680f944d67a3
        for further information
    """
    SUCCESS = 1
    INVALID_STATE = 2
    NOT_SUPPORTED = 3
    DATA_SIZE_EXCEEDS_LIMIT = 4
    CRC_ERROR = 5
    OPERATION_FAILED = 6

    @staticmethod
    def error_code_lookup(error_code):
        """
        Returns a description lookup table for error codes received from peer.

        :param int error_code: Error code to parse
        :return str: Textual description of the error code
        """
        code_lookup = {DfuErrorCodeBle.SUCCESS: "SUCCESS",
                       DfuErrorCodeBle.INVALID_STATE: "Invalid State",
                       DfuErrorCodeBle.NOT_SUPPORTED: "Not Supported",
                       DfuErrorCodeBle.DATA_SIZE_EXCEEDS_LIMIT: "Data Size Exceeds Limit",
                       DfuErrorCodeBle.CRC_ERROR: "CRC Error",
                       DfuErrorCodeBle.OPERATION_FAILED: "Operation Failed"}

        return code_lookup.get(error_code, "UNKOWN ERROR CODE")

# Service UUID. For further information, look at the nRF51 SDK documentation V7.2.0:
# http://developer.nordicsemi.com/nRF51_SDK/doc/7.2.0/s110/html/a00071.html#ota_spec_number
UUID_DFU_SERVICE = '000015301212EFDE1523785FEABCD123'
# Characteristic UUID
UUID_DFU_PACKET_CHARACTERISTIC = '000015321212EFDE1523785FEABCD123'
UUID_DFU_CONTROL_STATE_CHARACTERISTIC = '000015311212EFDE1523785FEABCD123'
# Descriptor UUID
UUID_CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR = 0x2902

# NOTE:  If packet receipt notification is enabled, a packet receipt
#        notification will be received for each 'num_of_packets_between_notif'
#        number of packets.
#
# Configuration tip: Increase this to get lesser notifications from the DFU
# Target about packet receipts. Make it 0 to disable the packet receipt
# notification

NUM_OF_PACKETS_BETWEEN_NOTIF = 10
DATA_PACKET_SIZE = 20


class DfuTransportBle(DfuTransport):

    def __init__(self):
        super(DfuTransportBle, self).__init__()

    def open(self):
        super(DfuTransportBle, self).open()

    def is_open(self):
        return super(DfuTransportBle, self).is_open()

    def close(self):
        super(DfuTransportBle, self).close()

    def _wait_for_condition(self, condition_function, expected_condition_value=True, timeout=10,
                            waiting_for="condition"):
        """
        Waits for condition_function to be true
        Will timeout after 60 seconds

        :param function condition_function: The function we are waiting for to return true
        :param str timeout_message: Message that should be logged
        :return:
        """

        start_time = datetime.now()

        while condition_function() != expected_condition_value:
            timeout_message = "Timeout while waiting for {0}.".format(waiting_for)
            timed_out = datetime.now() - start_time > timedelta(0, timeout)
            if timed_out:
                self._send_event(DfuEvent.TIMEOUT_EVENT, log_message=timeout_message)
                raise NordicSemiException(timeout_message)

            if not self.is_open():
                log_message = "Disconnected from device while waiting for {0}.".format(waiting_for)
                raise IllegalStateException(log_message)

            sleep(0.1)

        if self.get_last_error() != DfuErrorCodeBle.SUCCESS:
            error_message = "Error occoured while waiting for {0}. Error response {1}."
            error_code = DfuErrorCodeBle.error_code_lookup(self.get_last_error())
            error_message = error_message.format(waiting_for, error_code)
            self._send_event(DfuEvent.ERROR_EVENT, log_message=error_message)
            raise NordicSemiException(error_message)

    @abc.abstractmethod
    def send_packet_data(self, data):
        """
        Send data to the packet characteristic

        :param str data: The data to be sent
        :return:
        """
        pass

    @abc.abstractmethod
    def send_control_data(self, opcode, data=""):
        """
        Send data to the control characteristic

        :param int opcode: The opcode for the operation command sent to the control characteristic
        :param str data: The data to be sent
        :return:
        """
        pass

    @abc.abstractmethod
    def get_received_response(self):
        """
        Returns True if the transport layer has received a response it expected

        :return: bool
        """
        pass

    def clear_received_response(self):
        """
        Clears the received response status, sets it to False.

        :return:
        """
        pass

    @abc.abstractmethod
    def is_waiting_for_notification(self):
        """
        Returns True if the transport layer is waiting for a notification

        :return: bool
        """
        pass

    def set_waiting_for_notification(self):
        """
        Notifies the transport layer that it should wait for notification

        :return:
        """
        pass

    @abc.abstractmethod
    def get_last_error(self):
        """
        Returns the last error code

        :return: DfuErrorCodeBle
        """
        pass

    def _start_dfu(self, program_mode, image_size_packet):
        logger.debug("Sending 'START DFU' command")
        self.send_control_data(DfuOpcodesBle.START_DFU, chr(program_mode))
        logger.debug("Sending image size")
        self.send_packet_data(image_size_packet)
        self._wait_for_condition(self.get_received_response, waiting_for="response for START DFU")
        self.clear_received_response()

    def send_start_dfu(self, program_mode, softdevice_size=0, bootloader_size=0, app_size=0):
        super(DfuTransportBle, self).send_start_dfu(program_mode, softdevice_size, bootloader_size, app_size)
        image_size_packet = DfuTransport.create_image_size_packet(softdevice_size, bootloader_size, app_size)

        self._send_event(DfuEvent.PROGRESS_EVENT, progress=0, log_message="Setting up transfer...")

        try:
            self._start_dfu(program_mode, image_size_packet)
        except IllegalStateException:
            # We got disconnected. Try to send Start DFU again in case of buttonless dfu.
            self.close()
            self.open()

            if not self.is_open():
                raise IllegalStateException("Failed to reopen transport backend.")

            self._start_dfu(program_mode, image_size_packet)

    def send_init_packet(self, init_packet):
        super(DfuTransportBle, self).send_init_packet(init_packet)
        init_packet_start = chr(0x00)
        init_packet_end = chr(0x01)

        logger.debug("Sending 'INIT DFU' command")
        self.send_control_data(DfuOpcodesBle.INITIALIZE_DFU, init_packet_start)

        logger.debug("Sending init data")
        for i in range(0, len(init_packet), DATA_PACKET_SIZE):
            data_to_send = init_packet[i:i + DATA_PACKET_SIZE]
            self.send_packet_data(data_to_send)

        logger.debug("Sending 'Init Packet Complete' command")
        self.send_control_data(DfuOpcodesBle.INITIALIZE_DFU, init_packet_end)
        self._wait_for_condition(self.get_received_response, timeout=60, waiting_for="response for INITIALIZE DFU")
        self.clear_received_response()

        if NUM_OF_PACKETS_BETWEEN_NOTIF:
            packet = int16_to_bytes(NUM_OF_PACKETS_BETWEEN_NOTIF)
            logger.debug("Send number of packets before device sends notification")
            self.send_control_data(DfuOpcodesBle.REQ_PKT_RCPT_NOTIFICATION, packet)

    def send_firmware(self, firmware):
        def progress_percentage(part, complete):
            """
                Calculate progress percentage
                :param int part: Part value
                :param int complete: Completed value
                :return: int: Percentage complete
                """
            return min(100, (part + DATA_PACKET_SIZE) * 100 / complete)

        super(DfuTransportBle, self).send_firmware(firmware)
        packets_sent = 0
        last_progress_update = -1  # Last packet sequence number when an update was fired to the event system
        bin_size = len(firmware)
        logger.debug("Send 'RECEIVE FIRMWARE IMAGE' command")
        self.send_control_data(DfuOpcodesBle.RECEIVE_FIRMWARE_IMAGE)

        for i in range(0, bin_size, DATA_PACKET_SIZE):
            progress = progress_percentage(i, bin_size)

            if progress != last_progress_update:
                self._send_event(DfuEvent.PROGRESS_EVENT, progress=progress, log_message="Uploading firmware")
                last_progress_update = progress

            self._wait_for_condition(self.is_waiting_for_notification, expected_condition_value=False,
                                     waiting_for="notification from device")

            data_to_send = firmware[i:i + DATA_PACKET_SIZE]

            log_message = "Sending Firmware bytes [{0}, {1}]".format(i, i + len(data_to_send))
            logger.debug(log_message)

            packets_sent += 1

            if NUM_OF_PACKETS_BETWEEN_NOTIF != 0:
                if (packets_sent % NUM_OF_PACKETS_BETWEEN_NOTIF) == 0:
                    self.set_waiting_for_notification()

            self.send_packet_data(data_to_send)

        self._wait_for_condition(self.get_received_response, waiting_for="response for RECEIVE FIRMWARE IMAGE")
        self.clear_received_response()

    def send_validate_firmware(self):
        super(DfuTransportBle, self).send_validate_firmware()
        logger.debug("Sending 'VALIDATE FIRMWARE IMAGE' command")
        self.send_control_data(DfuOpcodesBle.VALIDATE_FIRMWARE_IMAGE)
        self._wait_for_condition(self.get_received_response, waiting_for="response for VALIDATE FIRMWARE IMAGE")
        self.clear_received_response()
        logger.info("Firmware validated OK.")

    def send_activate_firmware(self):
        super(DfuTransportBle, self).send_activate_firmware()
        logger.debug("Sending 'ACTIVATE FIRMWARE AND RESET' command")
        self.send_control_data(DfuOpcodesBle.ACTIVATE_FIRMWARE_AND_RESET)
