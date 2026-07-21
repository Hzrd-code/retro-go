// Target definition - DET SKAL VÆRE PLUS FOR AT RETRO-GO TÆNDER FOR STRØMMEN
#define RG_TARGET_NAME             "T-DECK-PLUS"

// Storage
#define RG_STORAGE_ROOT             "/sd"
#define RG_STORAGE_SDSPI_HOST       SPI2_HOST
#define RG_STORAGE_SDSPI_SPEED      SDMMC_FREQ_DEFAULT

// GPIO Extender
#define RG_I2C_GPIO_DRIVER          2   
#define RG_I2C_GPIO_ADDR            T_DECK_KBD_ADDRESS

// Audio
#define RG_AUDIO_USE_INT_DAC        0
#define RG_AUDIO_USE_EXT_DAC        1

// Board-specific
#define T_DECK_BOARD_POWER          GPIO_NUM_10
#define T_DECK_RADIO_CS             GPIO_NUM_9
#define T_DECK_KBD_ADDRESS          0x20
#define T_DECK_KBD_MODE_RAW_CMD     0x00

// Sørger for at tænde for hovedstrømmen (GPIO 10) og tastatur-strømmen (GPIO 46)
#define RG_CUSTOM_PLATFORM_INIT()                             \
    gpio_set_direction(T_DECK_BOARD_POWER, GPIO_MODE_OUTPUT); \
    gpio_set_level(T_DECK_BOARD_POWER, 1);                    \
    gpio_set_direction(RG_GPIO_TP_RST, GPIO_MODE_OUTPUT);     \
    gpio_set_level(RG_GPIO_TP_RST, 1);                        \
    gpio_set_direction(RG_GPIO_SDSPI_CS, GPIO_MODE_OUTPUT);   \
    gpio_set_direction(T_DECK_RADIO_CS, GPIO_MODE_OUTPUT);    \
    gpio_set_direction(RG_GPIO_LCD_CS, GPIO_MODE_OUTPUT);     \
    gpio_set_level(RG_GPIO_SDSPI_CS, 1);                      \
    gpio_set_level(T_DECK_RADIO_CS, 1);                       \
    gpio_set_level(RG_GPIO_LCD_CS, 1);                        \
    gpio_set_pull_mode(RG_GPIO_SDSPI_MISO, GPIO_PULLUP_ONLY); \
    rg_task_delay(50);

// Video
#define RG_SCREEN_DRIVER            0   // ST7789
#define RG_SCREEN_HOST              SPI2_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_80M
#define RG_SCREEN_BACKLIGHT         1
#define RG_SCREEN_WIDTH             320
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0

// Input Matrix
#define RG_GAMEPAD_I2C_MAP { \
    {RG_KEY_UP,     .num = 0,  .level = 0},\
    {RG_KEY_DOWN,   .num = 1,  .level = 0},\
    {RG_KEY_LEFT,   .num = 2,  .level = 0},\
    {RG_KEY_RIGHT,  .num = 3,  .level = 0},\
    {RG_KEY_A,      .num = 4,  .level = 0},\
    {RG_KEY_B,      .num = 5,  .level = 0},\
    {RG_KEY_SELECT, .num = 6,  .level = 0},\
    {RG_KEY_START,  .num = 7,  .level = 0},\
}
#define RG_GAMEPAD_GPIO_MAP { \
    {RG_KEY_MENU, .num = GPIO_NUM_0, .pullup = 1, .level = 0},\
}

#define RG_RECOVERY_BTN             RG_KEY_MENU

// Trackball hardware-pins
#define RG_GPIO_TRACKBALL_UP    GPIO_NUM_10
#define RG_GPIO_TRACKBALL_DOWN  GPIO_NUM_15
#define RG_GPIO_TRACKBALL_LEFT  GPIO_NUM_11
#define RG_GPIO_TRACKBALL_RIGHT GPIO_NUM_14
#define RG_GPIO_TRACKBALL_CLICK GPIO_NUM_9

// Battery
#define RG_BATTERY_DRIVER           0

// I2C BUS
#define RG_GPIO_I2C_SDA             GPIO_NUM_18
#define RG_GPIO_I2C_SCL             GPIO_NUM_8

// SPI Display
#define RG_GPIO_LCD_MISO            GPIO_NUM_NC
#define RG_GPIO_LCD_MOSI            GPIO_NUM_41
#define RG_GPIO_LCD_CLK             GPIO_NUM_40
#define RG_GPIO_LCD_CS              GPIO_NUM_12
#define RG_GPIO_LCD_DC              GPIO_NUM_11
#define RG_GPIO_LCD_BCKL            GPIO_NUM_42

// SPI SD Card
#define RG_GPIO_SDSPI_MISO          GPIO_NUM_38
#define RG_GPIO_SDSPI_MOSI          GPIO_NUM_41
#define RG_GPIO_SDSPI_CLK           GPIO_NUM_40
#define RG_GPIO_SDSPI_CS            GPIO_NUM_39

// External I2S DAC
#define RG_GPIO_SND_I2S_BCK         GPIO_NUM_7
#define RG_GPIO_SND_I2S_WS          GPIO_NUM_6
#define RG_GPIO_SND_I2S_DATA        GPIO_NUM_5
#define RG_GPIO_TP_INT              GPIO_NUM_16
#define RG_GPIO_TP_RST              GPIO_NUM_46
#define RG_GPIO_KEYPAD_INTERRUPT    GPIO_NUM_16
