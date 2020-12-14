// SPDX-FileCopyrightText: 2014 MicroPython & CircuitPython contributors (https://github.com/adafruit/circuitpython/graphs/contributors)
//
// SPDX-License-Identifier: MIT

#ifndef MICROPY_INCLUDED_EXTMOD_MODWEBSOCKET_H
#define MICROPY_INCLUDED_EXTMOD_MODWEBSOCKET_H

#define FRAME_OPCODE_MASK 0x0f
enum {
    FRAME_CONT, FRAME_TXT, FRAME_BIN,
    FRAME_CLOSE = 0x8, FRAME_PING, FRAME_PONG
};

#endif // MICROPY_INCLUDED_EXTMOD_MODWEBSOCKET_H
