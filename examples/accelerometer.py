import adafruit_lis3dh
import board
import busio
import math
import neopixel
import time

TILTED = 6.5
SLIGHTLY_TILTED = 3.5

BRIGHTNESS = 2
RED = (BRIGHTNESS, 0, 0)
YELLOW = (BRIGHTNESS, BRIGHTNESS, 0)
GREEN = (0, BRIGHTNESS, 0)

def accelerometer():
    i2c = busio.I2C(board.ACCELEROMETER_SCL, board.ACCELEROMETER_SDA)
    return(adafruit_lis3dh.LIS3DH_I2C(i2c, address=25))

def balance_color(x, y, z):
    if (z < 0): return(RED)
    if (x > TILTED) or (y > TILTED): return ((BRIGHTNESS, 0, 0))
    if (x > SLIGHTLY_TILTED) or (y > SLIGHTLY_TILTED): return((BRIGHTNESS, BRIGHTNESS, 0))
    return((0, BRIGHTNESS, 0))

def balance_neo_pixel_number(x, y, z):
    if (x > 0): return(int((y + 10 + 2.5) / 5))

    return(9 - int((math.fabs(y + 10) + 2.5) / 5))

def balance_set_colors(neo_pixels, x, y, z):
    if z < 0:
        for i in range(10): neo_pixels[i] = RED
        return

    for i in range(10): neo_pixels[i] = 0

    color = balance_color(math.fabs(x), math.fabs(y), z)
    neo_pixel_number = balance_neo_pixel_number(x, y, z)

    if neo_pixel_number >= 0: neo_pixels[neo_pixel_number] = color

accelerometer = accelerometer()
neo_pixels = neopixel.NeoPixel(board.NEOPIXEL, 10, auto_write=False)

while True:
    x, y, z = accelerometer.acceleration

    balance_set_colors(neo_pixels, x, y, z)

    neo_pixels.show()

    time.sleep(0.25)
