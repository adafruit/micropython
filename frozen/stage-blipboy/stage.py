import time
import array
import digitalio
import audiopwmio as audioio
import struct
try:
    import audiocore
except ImportError:
    audiocore = audioio

import _stage


FONT = (b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        b'P\x01\xd4\x05\xf5\x17\xed\x1e\xd5\x15\xd0\x01P\x01\x00\x00'
        b'P\x01\xd0\x01\xd5\x15\xed\x1e\xf5\x17\xd4\x05P\x01\x00\x00'
        b'P\x01\xd0\x05\x95\x17\xfd\x1f\x95\x17\xd0\x05P\x01\x00\x00'
        b'P\x01\xd4\x01\xb5\x15\xfd\x1f\xb5\x15\xd4\x01P\x01\x00\x00'
        b'T\x05\xf9\x1b\xdd\x1d}\x1f\xd9\x19\xa9\x1aT\x05\x00\x00'
        b'T\x05\xf9\x1b]\x1d\xdd\x1dY\x19\xa9\x1aT\x05\x00\x00P\x01\xd0\x01'
        b'\xe5\x16\xfd\x1f\xe4\x06t\x07\x14\x05\x00\x00P\x01\xd5\x15'
        b']\x1d\x95\x15\xf4\x07\xe4\x06T\x05\x00\x00\x14\x05y\x1b'
        b'\xfd\x1f\xf9\x1b\xe4\x06\xd0\x01@\x00\x00\x00P\x01\xf4\x06'
        b'\xad\x1b\xed\x1b\xf9\x1a\xa4\x06P\x01\x00\x00@\x00\xd0\x01'
        b'\xf4\x06\xfd\x1a\xa4\x06\x90\x01@\x00\x00\x00@\x15\xd0\x1a'
        b'\xb4\x1b\xed\x1b\xfd\x06\xad\x01U\x00\x00\x00T\x05\xf5\x17'
        b'\xbd\x1a]\x19m\x1bm\x1aU\x15\x00\x00\x00\x15D\x1f\xd9\x1f\xe4\x07'
        b'\x94\x01Y\x06\x05\x01\x00\x00T\x05\xbd\x1a\xfd\x1a\xfd\x1a'
        b'\xf4\x06\x90\x01@\x00\x00\x00\x15\x15m\x1e\xfd\x1f\xf5\x16'
        b'\xb4\x06\xf4\x06T\x05\x00\x00P\x01\x04\x04\x04\x04P\x01'
        b'\xf4\x06\xb4\x06P\x01\x00\x00P\x05t\x1b]\x1a\x1d\x15]\x1d\xf4\x07'
        b'P\x01\x00\x00T\x00\x10\x01\x10\x05T\x1bm\x1ai\x05\x14\x00\x00\x00'
        b'T\x05\xf4\x06\x90\x01\xf4\x06\xb9\x1a\xa9\x1aT\x05\x00\x00'
        b'T\x05\xf5\x17\xdd\x1d\xdd\x1d\xf5\x17\xe4\x06T\x05\x00\x00'
        b'U\x15\xad\x1e\xfd\x1f\xad\x1e\xd5\x15\xa9\x1aU\x15\x00\x00'
        b'P\x01\xe4\x06t\x07\xe4\x06\xd0\x01\xd0\x05\xd0\x06P\x05'
        b'P\x05\xd4\x17\xa5\x1d\xf9\x16\xb9\x06\xa5\x05T\x01\x00\x00'
        b'U\x15\xfd\x1f\xbd\x1f\xad\x1e\xbd\x1f\xfd\x1fU\x15\x00\x00'
        b'U\x15\xf9\x1a\xb4\x06\xf9\x1a\xf9\x1a\xa5\x16T\x05\x00\x00'
        b'\x14\x05e\x16y\x1b\xd4\x05y\x1be\x16\x14\x05\x00\x00T\x15\xf5\x1f'
        b'\x9d\x19\xf5\x1d\xd4\x1d\xd0\x1dP\x15\x00\x00\x00\x00P\x01'
        b'\xe4\x06\xf4\x07\xe4\x06P\x01\x00\x00\x00\x00U\x15\xdd\x1d'
        b'\xdd\x1d\x99\x19U\x15\xdd\x1dU\x15\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00U\x15\xdd\x1dU\x15\x00\x00\x00\x00\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00P\x01\xd0\x01'
        b'\xd0\x01\x90\x01P\x01\xd0\x01P\x01\x00\x00T\x05t\x07d\x06T\x05'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x14\x05u\x17\xed\x1et\x07'
        b'\xed\x1eu\x17\x14\x05\x00\x00T\x15\xf5\x1b\x99\x05\xf5\x17'
        b'\x94\x19\xf9\x17U\x05\x00\x00\x15\x14\x1d\x1dU\x07\xd0\x01'
        b't\x15\x1d\x1d\x05\x15\x00\x00T\x01\xe4\x05u\x07\xdd\x01'
        b']\x17\xe5\x1dT\x14\x00\x00P\x01\xd0\x01\x90\x01P\x01'
        b'\x00\x00\x00\x00\x00\x00\x00\x00@\x05P\x06\x90\x01\xd0\x01'
        b'\x90\x01P\x06@\x05\x00\x00T\x00d\x01\x90\x01\xd0\x01\x90\x01d\x01'
        b'T\x00\x00\x00\x00\x00\x14\x05t\x07\xd0\x01t\x07\x14\x05'
        b'\x00\x00\x00\x00P\x01\x90\x01\xd5\x15\xf9\x1b\xd5\x15\x90\x01'
        b'P\x01\x00\x00\x00\x00\x00\x00\x00\x00P\x01\xd0\x01\x90\x01'
        b'P\x01\x00\x00\x00\x00\x00\x00U\x15\xf9\x1bU\x15\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00P\x01\xd0\x01'
        b'P\x01\x00\x00\x00\x04\x00\x1d@\x07\xd0\x01t\x00\x1d\x00'
        b'\x04\x00\x00\x00T\x05\xe5\x16Y\x1a\xdd\x1di\x19\xe5\x16'
        b'T\x05\x00\x00@\x01\xd0\x01\xe4\x01\xd0\x01\xd0\x01\xe4\x06'
        b'T\x05\x00\x00T\x05\xf9\x17U\x1d\xf4\x17Y\x05\xfd\x1fU\x15\x00\x00'
        b'T\x05\xf5\x17]\x1d\x94\x07]\x1d\xf5\x17T\x05\x00\x00P\x00t\x00'
        b']\x05]\x17\xfd\x1fU\x17@\x05\x00\x00U\x15\xfd\x1b]\x05\xfd\x1b'
        b'U\x1d\xf9\x1bU\x05\x00\x00T\x15\xf5\x1b]\x05\xfd\x1b]\x1d\xf9\x1b'
        b'T\x05\x00\x00U\x15\xfd\x1fU\x19\xd0\x06d\x01t\x00T\x00\x00\x00'
        b'T\x05\xf5\x17]\x1d\xf5\x17]\x1d\xf5\x17T\x05\x00\x00T\x05\xf9\x1b'
        b']\x1d\xf9\x1fT\x1d\xf9\x17U\x05\x00\x00\x00\x00P\x01\xd0\x01P\x01'
        b'\xd0\x01P\x01\x00\x00\x00\x00\x00\x00P\x01\xd0\x01P\x01'
        b'\xd0\x01\x90\x01P\x01\x00\x00\x00\x05@\x07\xd0\x01t\x00'
        b'\xd0\x01@\x07\x00\x05\x00\x00\x00\x00U\x15\xf9\x1bT\x05'
        b'\xf9\x1bU\x15\x00\x00\x00\x00\x14\x00t\x00\xd0\x01@\x07'
        b'\xd0\x01t\x00\x14\x00\x00\x00T\x05\xe5\x17]\x1d\xd5\x16'
        b'P\x05\xd0\x01P\x01\x00\x00T\x05\xb5\x17\xdd\x1d\x9d\x1b'
        b'Y\x15\xf5\x06T\x05\x00\x00P\x00\xe4\x01Y\x07]\x1d\xed\x1e]\x1d'
        b'\x15\x15\x00\x00U\x01\xfd\x05]\x07\xed\x16]\x1d\xfd\x17'
        b'U\x05\x00\x00T\x05\xf5\x06]\x01\x1d\x14]\x1d\xf5\x17T\x05\x00\x00'
        b'U\x01\xbd\x05]\x17\x1d\x1d]\x1d\xfd\x16U\x05\x00\x00U\x05\xfd\x06'
        b']\x01\xfd\x01]\x15\xfd\x1bU\x15\x00\x00U\x15\xfd\x1b]\x15]\x00'
        b'\xbd\x01]\x01\x15\x00\x00\x00T\x15\xf5\x1b]\x05\xdd\x1f'
        b'Y\x1d\xf5\x1bT\x15\x00\x00\x15\x15\x1d\x1d]\x1d\xfd\x1f'
        b']\x1d\x1d\x1d\x15\x15\x00\x00T\x05\xe4\x06\xd0\x01\xd0\x01'
        b'\xd0\x01\xe4\x06T\x05\x00\x00\x00\x15\x00\x1d\x00\x1d\x05\x1d'
        b']\x19\xf5\x17T\x05\x00\x00\x15\x14\x1d\x1d]\x07\xfd\x01'
        b']\x07\x1d\x1d\x15\x14\x00\x00\x15\x00\x1d\x00\x1d\x00\x1d\x00'
        b']\x15\xfd\x1fU\x15\x00\x00\x05\x14\x1d\x1dm\x1e\xdd\x1d'
        b']\x1d\x1d\x1d\x15\x15\x00\x00\x05\x15\x1d\x1dm\x1d\xdd\x1d'
        b']\x1e\x1d\x1d\x15\x14\x00\x00T\x01\xb5\x05]\x17\x1d\x1d'
        b']\x1d\xe5\x17T\x05\x00\x00U\x05\xfd\x16]\x19]\x1d\xfd\x17]\x05'
        b'\x15\x00\x00\x00T\x01\xb5\x05]\x17\x1d\x1d]\x1e\xe5\x07'
        b'T\x1d\x00\x15U\x05\xfd\x16]\x19]\x1d\xfd\x07]\x1d\x15\x15\x00\x00'
        b'T\x05\xf5\x07]\x01\xe5\x06T\x1d\xf9\x17U\x05\x00\x00U\x15\xf9\x1b'
        b'\xd5\x15\xd0\x01\xd0\x01\xd0\x01P\x01\x00\x00\x15\x15\x1d\x1d'
        b'\x1d\x1d\x19\x1du\x19\xd4\x17P\x05\x00\x00\x05\x14\x1d\x1d'
        b'\x19\x19u\x17d\x06\xd0\x01@\x00\x00\x00\x15\x15\x1d\x1d'
        b'\x1d\x1d]\x1d\xd9\x19u\x17\x14\x05\x00\x00\x05\x14\x1d\x1d'
        b't\x07\xd0\x01t\x07\x1d\x1d\x05\x14\x00\x00\x15\x15\x1d\x1d'
        b'\x19\x19u\x17\x94\x05\xd0\x01P\x01\x00\x00U\x15\xf9\x1b'
        b'U\x07\xd0\x01t\x15\xf9\x1bU\x15\x00\x00T\x05\xf4\x06t\x01t\x00'
        b't\x01\xf4\x06T\x05\x00\x00\x05\x00\x1d\x00t\x00\xd0\x01'
        b'@\x07\x00\x1d\x00\x14\x00\x00T\x05\xe4\x07P\x07@\x07P\x07\xe4\x07'
        b'T\x05\x00\x00@\x00\xd0\x01t\x07\x19\x19\x04\x04\x00\x00'
        b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00U\x15\xf9\x1b'
        b'U\x15\x00\x00P\x00\xb4\x01\xd4\x06P\x07@\x01\x00\x00'
        b'\x00\x00\x00\x00\x00\x00T\x15\xe5\x1f]\x1d]\x1d\xf5\x1f'
        b'T\x15\x00\x00\x15\x00]\x05\xfd\x16]\x1d]\x1d\xfd\x17U\x05\x00\x00'
        b'\x00\x00T\x05\xe5\x07]\x05]\x1d\xf5\x16T\x05\x00\x00\x00\x15T\x1d'
        b'\xe5\x1f]\x1d]\x1d\xf5\x1fT\x15\x00\x00\x00\x00T\x05'
        b'\xf5\x17\xad\x1e]\x15\xf5\x07T\x05\x00\x00@\x15P\x1e'
        b'\xd4\x15\xf4\x07\xd4\x05\xd0\x01\xd0\x01P\x01\x00\x00T\x15'
        b'\xe5\x1f]\x1d\xf5\x1fT\x1d\xf9\x16U\x05\x15\x00]\x05\xfd\x16]\x1d'
        b'\x1d\x1d\x1d\x1d\x15\x15\x00\x00P\x01\xd0\x01P\x01\xd0\x01'
        b'\xd0\x01\xd0\x01P\x01\x00\x00@\x05@\x07@\x05@\x07E\x07]\x07'
        b'\xe5\x05T\x01\x15\x00\x1d\x14]\x1d\xfd\x06]\x19\x1d\x1d'
        b'\x15\x14\x00\x00T\x00t\x00t\x00t\x00d\x05\xd4\x07P\x05\x00\x00'
        b'\x00\x00U\x05\xfd\x17\xdd\x19\xdd\x1d]\x1d\x15\x15\x00\x00'
        b'\x00\x00U\x05\xfd\x17]\x19\x1d\x1d\x1d\x1d\x15\x15\x00\x00'
        b'\x00\x00T\x05\xe5\x17]\x1d]\x1d\xf5\x17T\x05\x00\x00\x00\x00U\x05'
        b'\xfd\x17]\x1d]\x1d\xfd\x17]\x05\x15\x00\x00\x00T\x15\xf5\x1f]\x1d'
        b']\x1d\xf5\x1fT\x1d\x00\x15\x00\x00U\x05\xdd\x16}\x1d]\x04\x1d\x00'
        b'\x15\x00\x00\x00\x00\x00T\x15\xe5\x1f\xad\x05\x94\x1e\xfd\x16'
        b'U\x05\x00\x00T\x00u\x05\xfd\x07t\x01t\x01\xd4\x07P\x05\x00\x00'
        b'\x00\x00\x15\x15\x1d\x1d\x1d\x1d]\x1d\xe5\x1fT\x15\x00\x00'
        b'\x00\x00\x05\x14\x1d\x1d\x19\x19u\x17\xd4\x05P\x01\x00\x00'
        b'\x00\x00\x15\x15]\x1d\xdd\x1d\xd9\x19u\x17T\x05\x00\x00'
        b'\x00\x00\x15\x15m\x1e\xd4\x05\xd4\x05m\x1e\x15\x15\x00\x00'
        b'\x00\x00\x15\x15\x1d\x1d]\x1d\xe5\x1fT\x1d\xfd\x17U\x05'
        b'\x00\x00U\x15\xfd\x1f\xa4\x15\x95\x06\xfd\x1fU\x15\x00\x00'
        b'@\x05\x90\x07\xd0\x01t\x01\xd0\x01\x90\x07@\x05\x00\x00'
        b'P\x01\x90\x01\xd0\x01\xd0\x01\xd0\x01\x90\x01P\x01\x00\x00'
        b'T\x00\xb4\x01\xd0\x01P\x07\xd0\x01\xb4\x01T\x00\x00\x00'
        b'\x00\x00T\x00u\x15\xd9\x19U\x17@\x05\x00\x00\x00\x00U\x15\xfd\x1f'
        b'\xed\x1e\xbd\x1f\xed\x1e\xfd\x1fU\x15\x00\x00')

PALETTE = (b'\xf8\x1f\x00\x00\xcey\xff\xff\xf8\x1f\x00\x19\xfc\xe0\xfd\xe0'
           b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00')


def color565(r, g, b):
    """Convert 24-bit RGB color to 16-bit."""
    return (r & 0xf8) << 8 | (g & 0xfc) << 3 | b >> 3


def collide(ax0, ay0, ax1, ay1, bx0, by0, bx1=None, by1=None):
    """Return True if the two rectangles intersect."""
    if bx1 is None:
        bx1 = bx0
    if by1 is None:
        by1 = by0
    return not (ax1 < bx0 or ay1 < by0 or ax0 > bx1 or ay0 > by1)


class Audio:
    """Play sounds."""
    last_audio = None

    def __init__(self, speaker_pin, mute_pin=None):
        if mute_pin:
            self.mute_pin = digitalio.DigitalInOut(mute_pin)
            self.mute_pin.switch_to_output(value=0)
        else:
            self.mute_pin = None
        self.audio = audioio.PWMAudioOut(speaker_pin)

    def play(self, audio_file, loop=False):
        """
        Start playing an open file ``audio_file``. If ``loop`` is ``True``,
        repeat until stopped. This function doesn't block, the sound is
        played in the background.
        """
        self.stop()
        wave = audiocore.WaveFile(audio_file)
        self.audio.play(wave, loop=loop)

    def stop(self):
        """Stop playing whatever sound is playing."""
        self.audio.stop()

    def mute(self, value=True):
        """Enable or disable all sounds."""
        if self.mute_pin:
            self.mute_pin.value = not value


class BMP16:
    """Read 16-color BMP files."""

    def __init__(self, filename):
        self.filename = filename
        self.colors = 0

    def read_header(self):
        """Read the file's header information."""

        if self.colors:
            return
        with open(self.filename, 'rb') as f:
            f.seek(10)
            self.data = int.from_bytes(f.read(4), 'little')
            f.seek(18)
            self.width = int.from_bytes(f.read(4), 'little')
            self.height = int.from_bytes(f.read(4), 'little')
            f.seek(46)
            self.colors = int.from_bytes(f.read(4), 'little')

    def read_palette(self):
        """Read the color palette information."""

        palette = array.array('H', (0 for i in range(16)))
        with open(self.filename, 'rb') as f:
            f.seek(self.data - self.colors * 4)
            for color in range(self.colors):
                buffer = f.read(4)
                c = color565(buffer[0], buffer[1], buffer[2])
                palette[color] = ((c << 8) | (c >> 8)) & 0xffff
        return palette

    def read_data(self, buffer=None):
        """Read the image data."""
        line_size = self.width >> 1
        if buffer is None:
            buffer = bytearray(line_size * self.height)

        with open(self.filename, 'rb') as f:
            f.seek(self.data)
            index = (self.height - 1) * line_size
            for line in range(self.height):
                chunk = f.read(line_size)
                buffer[index:index + line_size] = chunk
                index -= line_size
        return buffer


def read_blockstream(f):
    while True:
        size = f.read(1)[0]
        if size == 0:
            break
        for i in range(size):
            yield f.read(1)[0]


class EndOfData(Exception):
    pass


class LZWDict:
    def __init__(self, code_size):
        self.code_size = code_size
        self.clear_code = 1 << code_size
        self.end_code = self.clear_code + 1
        self.codes = []
        self.clear()

    def clear(self):
        self.last = b''
        self.code_len = self.code_size + 1
        self.codes[:] = []

    def decode(self, code):
        if code == self.clear_code:
            self.clear()
            return b''
        elif code == self.end_code:
            raise EndOfData()
        elif code < self.clear_code:
            value = bytes([code])
        elif code <= len(self.codes) + self.end_code:
            value = self.codes[code - self.end_code - 1]
        else:
            value = self.last + self.last[0:1]
        if self.last:
            self.codes.append(self.last + value[0:1])
        if (len(self.codes) + self.end_code + 1 >= 1 << self.code_len and
            self.code_len < 12):
                self.code_len += 1
        self.last = value
        return value


def lzw_decode(data, code_size):
    dictionary = LZWDict(code_size)
    bit = 0
    try:
        byte = next(data)
        try:
            while True:
                code = 0
                for i in range(dictionary.code_len):
                    code |= ((byte >> bit) & 0x01) << i
                    bit += 1
                    if bit >= 8:
                        bit = 0
                        byte = next(data)
                yield dictionary.decode(code)
        except EndOfData:
            while True:
                next(data)
    except StopIteration:
        return
    finally:
        del dictionary


class GIF16:
    def __init__(self, filename):
        self.filename = filename

    def read_header(self):
        with open(self.filename, 'rb') as f:
            header = f.read(6)
            if header not in {b'GIF87a', b'GIF89a'}:
                raise ValueError("Not GIF file")
            self.width, self.height, flags, self.background, self.aspect = (
                struct.unpack('<HHBBB', f.read(7)))
            self.palette_size = 1 << ((flags & 0x07) + 1)
        if not flags & 0x80 or self.palette_size > 16:
            raise ValueError("16-color GIF expected")

    def read_palette(self):
        palette = array.array('H', (0 for i in range(16)))
        with open(self.filename, 'rb') as f:
            f.seek(13)
            for color in range(self.palette_size):
                buffer = f.read(3)
                c = color565(buffer[2], buffer[1], buffer[0])
                palette[color] = ((c << 8) | (c >> 8)) & 0xffff
        return palette

    def read_data(self, buffer=None):
        line_size = (self.width + 1) >> 1
        if buffer is None:
            buffer = bytearray(line_size * self.height)
        with open(self.filename, 'rb') as f:
            f.seek(13 + self.palette_size * 3)
            while True: # skip to first frame
                block_type = f.read(1)[0]
                if block_type == 0x2c:
                    break
                elif block_type == 0x21: # skip extension
                    extension_type = f.read(1)[0]
                    while True:
                        size = f.read(1)[0]
                        if size == 0:
                            break
                        f.seek(1, size)
                elif block_type == 0x3b:
                    raise ValueError("no frames")
            x, y, w, h, flags = struct.unpack('<HHHHB', f.read(9))
            if flags & 0x80:
                raise ValueError("local palette")
            if flags & 0x40:
                raise ValueError("interlaced")
            if w != self.width or h != self.height or x != 0 or y != 0:
                raise ValueError("partial frame")
            min_code_size = f.read(1)[0]
            x = 0
            y = 0
            for decoded in lzw_decode(read_blockstream(f), min_code_size):
                for pixel in decoded:
                    if x & 0x01:
                        buffer[(x >> 1) + y * line_size] |= pixel
                    else:
                        buffer[(x >> 1) + y * line_size] = pixel << 4
                    x += 1
                    if (x >= self.width):
                        x = 0
                        y += 1
        return buffer


class Bank:
    """
    Store graphics for the tiles and sprites.

    A single bank stores exactly 16 tiles, each 16x16 pixels in 16 possible
    colors, and a 16-color palette. We just like the number 16.

    """

    def __init__(self, buffer=None, palette=None):
        self.buffer = buffer
        self.palette = palette

    @classmethod
    def from_bmp16(cls, filename):
        """Read the bank from a BMP file."""
        return cls.from_image(filename)


    @classmethod
    def from_image(cls, filename):
        """Read the bank from an image file."""
        if filename.lower().endswith(".gif"):
            image = GIF16(filename)
        elif filename.lower().endswith(".bmp"):
            image = BMP16(filename)
        else:
            raise ValueError("Unsupported format")
        image.read_header()
        if image.width != 16 or image.height != 256:
            raise ValueError("Image size not 16x256")
        palette = image.read_palette()
        buffer = image.read_data()
        return cls(buffer, palette)


class Grid:
    """
    A grid is a layer of tiles that can be displayed on the screen. Each square
    can contain any of the 16 tiles from the associated bank.
    """

    def __init__(self, bank, width=8, height=8, palette=None, buffer=None):
        self.x = 0
        self.y = 0
        self.z = 0
        self.stride = (width + 1) & 0xfe
        self.width = width
        self.height = height
        self.bank = bank
        self.palette = palette or bank.palette
        self.buffer = buffer or bytearray(self.stride * height)
        self.layer = _stage.Layer(self.stride, self.height, self.bank.buffer,
                                  self.palette, self.buffer)

    def tile(self, x, y, tile=None):
        """Get or set what tile is displayed in the given place."""

        if not 0 <= x < self.width or not 0 <= y < self.height:
            return 0
        index = (y * self.stride + x) >> 1
        b = self.buffer[index]
        if tile is None:
            return b & 0x0f if x & 0x01 else b >> 4
        if x & 0x01:
            b = b & 0xf0 | tile
        else:
            b = b & 0x0f | (tile << 4)
        self.buffer[index] = b

    def move(self, x, y, z=None):
        """Shift the whole layer respective to the screen."""

        self.x = x
        self.y = y
        if z is not None:
            self.z = z
        self.layer.move(int(x), int(y))


class WallGrid(Grid):
    """
    A special grid, shifted from its parents by half a tile, useful for making
    nice-looking corners of walls and similar structures.
    """

    def __init__(self, grid, walls, bank, palette=None):
        super().__init__(bank, grid.width + 1, grid.height + 1, palette)
        self.grid = grid
        self.walls = walls
        self.update()
        self.move(self.x - 8, self.y - 8)

    def update(self):
        for y in range(self.height):
            for x in range(self.width):
                t = 0
                bit = 1
                for dy in (-1, 0):
                    for dx in (-1, 0):
                        if self.grid.tile(x + dx, y + dy) in self.walls:
                            t |= bit
                        bit <<= 1
                self.tile(x, y, t)


class Sprite:
    """
    A sprite is a layer containing just a single tile from the associated bank,
    that can be positioned anywhere on the screen.
    """

    def __init__(self, bank, frame, x, y, z=0, rotation=0, palette=None):
        self.bank = bank
        self.palette = palette or bank.palette
        self.frame = frame
        self.rotation = rotation
        self.x = x
        self.y = y
        self.z = z
        self.layer = _stage.Layer(1, 1, self.bank.buffer, self.palette)
        self.layer.move(x, y)
        self.layer.frame(frame, rotation)
        self.px = x
        self.py = y

    def move(self, x, y, z=None):
        """Move the sprite to the given place."""

        self.x = x
        self.y = y
        if z is not None:
            self.z = z
        self.layer.move(int(x), int(y))

    def set_frame(self, frame=None, rotation=None):
        """
        Set the current graphic and rotation of the sprite.

        The possible values for rotation are: 0 - none, 1 - 90 degrees
        clockwise, 2 - 180 degrees, 3 - 90 degrees counter-clockwise, 4 -
        mirrored, 5 - 90 degrees clockwise and mirrored, 6 - 180 degrees and
        mirrored, 7 - 90 degrees counter-clockwise and mirrored.  """

        if frame is not None:
            self.frame = frame
        if rotation is not None:
            self.rotation = rotation
        self.layer.frame(self.frame, self.rotation)

    def update(self):
        pass

    def _updated(self):
        self.px = int(self.x)
        self.py = int(self.y)


class Text:
    """Text layer. For displaying text."""

    def __init__(self, width, height, font=None, palette=None, buffer=None):
        self.width = width
        self.height = height
        self.font = font or FONT
        self.palette = palette or PALETTE
        self.buffer = buffer or bytearray(width * height)
        self.layer = _stage.Text(width, height, self.font,
                                 self.palette, self.buffer)
        self.column = 0
        self.row = 0
        self.x = 0
        self.y = 0
        self.z = 0

    def char(self, x, y, c=None, hightlight=False):
        """Get or set the character at the given location."""
        if not 0 <= x < self.width or not 0 <= y < self.height:
            return
        if c is None:
            return chr(self.buffer[y * self.width + x])
        c = ord(c)
        if hightlight:
            c |= 0x80
        self.buffer[y * self.width + x] = c

    def move(self, x, y, z=None):
        """Shift the whole layer respective to the screen."""
        self.x = x
        self.y = y
        if z is not None:
            self.z = z
        self.layer.move(int(x), int(y))

    def cursor(self, x=None, y=None):
        """Move the text cursor to the specified row and column."""
        if y is not None:
            self.row = min(max(0, y), self.width - 1)
        if x is not None:
            self.column = min(max(0, x), self.height - 1)

    def text(self, text, hightlight=False):
        """Display text starting at the current cursor location."""
        for c in text:
            if ord(c) >= 32:
                self.char(self.column, self.row, c, hightlight)
                self.column += 1
            if self.column >= self.width or c == '\n':
                self.column = 0
                self.row += 1
                if self.row >= self.height:
                    self.row = 0

    def clear(self):
        """Clear all text from the layer."""
        for i in range(self.width * self.height):
            self.buffer[i] = 0


class Stage:
    """
    Represents what is being displayed on the screen.

    The ``display`` parameter is displayio.Display representing an initialized
    display connected to the device.

    The ``fps`` specifies the maximum frame rate to be enforced.
    """
    buffer = bytearray(512)

    def __init__(self, display, fps=6):
        self.layers = []
        self.display = display
        self.width = display.width
        self.height = display.height
        self.last_tick = time.monotonic()
        self.tick_delay = 1 / fps

    def tick(self):
        """Wait for the start of the next frame."""
        self.last_tick += self.tick_delay
        wait = max(0, self.last_tick - time.monotonic())
        if wait:
            time.sleep(wait)
        else:
            self.last_tick = time.monotonic()

    def render_block(self, x0=0, y0=0, x1=None, y1=None):
        """Update a rectangle of the screen."""
        if x1 is None:
            x1 = self.width
        if y1 is None:
            y1 = self.height
        layers = [l.layer for l in self.layers]
        _stage.render(x0, y0, x1, y1, layers, self.buffer, self.display)

    def render_sprites(self, sprites):
        """Update the spots taken by all the sprites in the list."""
        layers = [l.layer for l in self.layers]
        for sprite in sprites:
            x0 = max(0, min(self.width - 1, min(sprite.px, int(sprite.x))))
            y0 = max(0, min(self.height - 1, min(sprite.py, int(sprite.y))))
            x1 = max(1, min(self.width, max(sprite.px, int(sprite.x)) + 16))
            y1 = max(1, min(self.height, max(sprite.py, int(sprite.y)) + 16))
            if x0 == x1 or y0 == y1:
                continue
            _stage.render(x0, y0, x1, y1, layers, self.buffer,
                          self.display)
            sprite._updated()
