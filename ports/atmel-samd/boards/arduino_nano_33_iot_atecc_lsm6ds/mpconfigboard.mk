USB_VID = 0x2341
USB_PID = 0x8057
USB_PRODUCT = "Arduino Nano 33 IoT"
USB_MANUFACTURER = "Arduino"

CHIP_VARIANT = SAMD21G18A
CHIP_FAMILY = samd21

INTERNAL_FLASH_FILESYSTEM = 1
LONGINT_IMPL = NONE
CIRCUITPY_SMALL_BUILD = 1

SUPEROPT_GC = 0

FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_BusDevice
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_binascii
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_ATECC
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_Register
FROZEN_MPY_DIRS += $(TOP)/frozen/Adafruit_CircuitPython_LSM6DS
