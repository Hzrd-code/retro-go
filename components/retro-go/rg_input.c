#include "rg_input.h"
#include <driver/i2c.h>
#include <esp_log.h>

#define TAG "T_DECK_KEYBOARD"

#define TDECK_KB_I2C_ADDR  0x55
#define TDECK_KB_GPIO_SDA  18
#define TDECK_KB_GPIO_SCL  17

void rg_input_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TDECK_KB_GPIO_SDA,
        .scl_io_num = TDECK_KB_GPIO_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    ESP_LOGI(TAG, "T-Deck I2C tastatur driver klar til slave-pooling.");
}

uint32_t rg_input_read(void) {
    uint32_t active_keys = 0;
    uint8_t scancode = 0;

    // Læs direkte fra I2C-slaven (præcis som koden fra Robert Grizzell lægger op til)
    esp_err_t err = i2c_master_read_from_device(I2C_NUM_0, TDECK_KB_I2C_ADDR, &scancode, 1, pdMS_TO_TICKS(5));
    
    if (err != ESP_OK || scancode == 0 || scancode == '\0') {
        return 0;
    }

    // Oversæt det modtagne tegn fra tastaturets buffer til spil-knapper
    switch (scancode) {
        case 'w': case 'W': active_keys |= RG_KEY_UP; break;
        case 's': case 'S': active_keys |= RG_KEY_DOWN; break;
        case 'a': case 'A': active_keys |= RG_KEY_LEFT; break;
        case 'd': case 'D': active_keys |= RG_KEY_RIGHT; break;
        case 'j': case 'J': active_keys |= RG_KEY_A; break;
        case 'k': case 'K': active_keys |= RG_KEY_B; break;
        case 13:  case '\n': active_keys |= RG_KEY_START; break;
        case ' ': active_keys |= RG_KEY_SELECT; break;
        case 'm': case 'M': active_keys |= RG_KEY_MENU; break;
        default: break;
    }

    return active_keys;
}
