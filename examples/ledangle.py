import adafruit_lis3dh
import board
import busio
import neopixel
import time

i2c = busio.I2C(board.ACCELEROMETER_SCL, board.ACCELEROMETER_SDA)
accelerometer = adafruit_lis3dh.LIS3DH_I2C(i2c, address=25)
neo_pixels = neopixel.NeoPixel(board.NEOPIXEL, 10, auto_write=False)

while True:
    x, y, z = accelerometer.acceleration

    if (x > 0):
        neo_pixels[2] = (2, 2, 2)
        neo_pixels[7] = (0, 0, 0)
    else:
        neo_pixels[2] = (0, 0, 0)
        neo_pixels[7] = (2, 2, 2)

    neo_pixels.show()

    time.sleep(0.25)
