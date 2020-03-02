#define MICROPY_HW_BOARD_NAME "Arduino Nano 33 IoT"
#define MICROPY_HW_MCU_NAME "samd21g18"

#define MICROPY_HW_LED_STATUS   (&pin_PA17)

#define MICROPY_PORT_A        (0)
#define MICROPY_PORT_B        (0)
#define MICROPY_PORT_C        (0)

#define DEFAULT_I2C_BUS_SCL (&pin_PB09)
#define DEFAULT_I2C_BUS_SDA (&pin_PB08)

#define DEFAULT_SPI_BUS_SCK (&pin_PA17)
#define DEFAULT_SPI_BUS_MOSI (&pin_PA16)
#define DEFAULT_SPI_BUS_MISO (&pin_PA19)

#define DEFAULT_UART_BUS_RX (&pin_PB23)
#define DEFAULT_UART_BUS_TX (&pin_PB22)

// USB is always used internally so skip the pin objects for it.
#define IGNORE_PIN_PA24     1
#define IGNORE_PIN_PA25     1

#define NANO_IOT_ATECC 1
