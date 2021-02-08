// LEDs
// #define MICROPY_HW_LED_STATUS   (&pin_PA17)

#define MICROPY_HW_BOARD_NAME "Raspberry Pi Pico"
#define MICROPY_HW_MCU_NAME "rp2040"

#define DEFAULT_I2C_BUS_SDA (&pin_GPIO0)
#define DEFAULT_I2C_BUS_SCL (&pin_GPIO1)

#define DEFAULT_SPI_BUS_SCK (&pin_GPIO2)
#define DEFAULT_SPI_BUS_MOSI (&pin_GPIO3)
#define DEFAULT_SPI_BUS_MISO (&pin_GPIO4)

#define DEFAULT_UART_BUS_TX (&pin_GPIO8)
#define DEFAULT_UART_BUS_RX (&pin_GPIO9)
