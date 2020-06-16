USB_VID = 0x239A
USB_PID = 0x8056
USB_PRODUCT = "STM32F412ZG Discovery Board - CPy"
USB_MANUFACTURER = "STMicroelectronics"
USB_DEVICES = "CDC,MSC,HID"

INTERNAL_FLASH_FILESYSTEM = 1

# QSPI_FLASH_FILESYSTEM = 1
# EXTERNAL_FLASH_DEVICE_COUNT = 1
# EXTERNAL_FLASH_DEVICES = N25Q128A
# LONGINT_IMPL = MPZ

MCU_SERIES = F4
MCU_VARIANT = STM32F412Zx
MCU_PACKAGE = LQFP144

LD_COMMON = boards/common_default.ld
LD_FILE = boards/STM32F412_fs.ld
