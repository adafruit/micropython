#ifndef NRFX_CONFIG_H__
#define NRFX_CONFIG_H__

// Power
#define NRFX_POWER_ENABLED                         1
#define NRFX_POWER_CONFIG_IRQ_PRIORITY             7

// SPI
#define NRFX_SPIM_ENABLED                          1

#ifdef NRF52840_XXAA
#define NRFX_SPIM3_ENABLED                         1
#else
#define NRFX_SPIM2_ENABLED                         1
#endif

#define NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY      7
#define NRFX_SPIM_MISO_PULL_CFG                    1

// QSPI
#define NRFX_QSPI_ENABLED                          1

// TWI aka. I2C
#define NRFX_TWIM_ENABLED                          1
#define NRFX_TWIM0_ENABLED                         1

#define NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY      7
#define NRFX_TWIM_DEFAULT_CONFIG_FREQUENCY         NRF_TWIM_FREQ_400K
#define NRFX_TWIM_DEFAULT_CONFIG_HOLD_BUS_UNINIT   0

// UART
#define NRFX_UART_ENABLED                          1
#define NRFX_UART0_ENABLED                         1

#define NRFX_UART_DEFAULT_CONFIG_IRQ_PRIORITY      7
#define NRFX_UART_DEFAULT_CONFIG_HWFC              NRF_UART_HWFC_DISABLED
#define NRFX_UART_DEFAULT_CONFIG_PARITY            NRF_UART_PARITY_EXCLUDED
#define NRFX_UART_DEFAULT_CONFIG_BAUDRATE          NRF_UART_BAUDRATE_115200

// PWM
#define NRFX_PWM0_ENABLED                          1
#define NRFX_PWM1_ENABLED                          1
#define NRFX_PWM2_ENABLED                          1

#ifdef NRF_PWM3
#define NRFX_PWM3_ENABLED                          1
#else
#define NRFX_PWM3_ENABLED                          0
#endif

#endif
