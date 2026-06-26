
#define CAN_TX 20   // <-- adjust to your Waveshare board
#define CAN_RX 19    // <-- adjust to your Waveshare board

#define RECEIVE_RATE_MS 50 //How often to check for incoming CAN messages

// ==================================
#define EXAMPLE_I2C_ADDR    (ESP_IO_EXPANDER_I2C_CH422G_ADDRESS)
#define EXAMPLE_I2C_SDA_PIN 8         // I2C data line pins
#define EXAMPLE_I2C_SCL_PIN 9         // I2C clock line pin
// Extend IO Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

// Interval:
#define TRANSMIT_RATE_MS 1000
#define POLLING_RATE_MS 1000