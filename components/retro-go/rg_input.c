#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "rg_system.h"
#include "rg_input.h"
#include "rg_i2c.h"

static const char *TAG = "rg_input";

#define TDECK_I2C_SDA          GPIO_NUM_18
#define TDECK_I2C_SCL          GPIO_NUM_8
#define TDECK_I2C_PWR          GPIO_NUM_46
#define TDECK_KEYBOARD_ADDR    0x20

static int gamepad_state = 0;
static bool driver_initialized = false;

// Tving I2C-bussen til at nulstille hvis M5Launcher har låst den
static void force_i2c_bus_reset(void)
{
    gpio_reset_pin(TDECK_I2C_SDA);
    gpio_reset_pin(TDECK_I2C_SCL);
    gpio_set_direction(TDECK_I2C_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_direction(TDECK_I2C_SCL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(TDECK_I2C_SDA, 1);
    gpio_set_level(TDECK_I2C_SCL, 1);

    // Send 9 clock pulser for at låse hængte I2C enheder op
    for (int i = 0; i < 9; i++) {
        gpio_set_level(TDECK_I2C_SCL, 0);
        vTaskDelay(pdMS_TO_TICKS(1));
        gpio_set_level(TDECK_I2C_SCL, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

bool rg_input_init(void)
{
    if (driver_initialized) return true;

    ESP_LOGI(TAG, "Initializing T-Deck Input (M5Launcher Fix)...");

    // 1. Tænd for strøm til T-Deck periferi (GPIO 46)
    gpio_reset_pin(TDECK_I2C_PWR);
    gpio_set_direction(TDECK_I2C_PWR, GPIO_MODE_OUTPUT);
    gpio_set_level(TDECK_I2C_PWR, 1);
    vTaskDelay(pdMS_TO_TICKS(50)); // Vent på strømmen stabiliserer sig

    // 2. Fysisk reset af I2C bussen
    force_i2c_bus_reset();

    // 3. Trackball GPIOs (PULLUP for at fjerne støj / spøgelsestryk)
    const gpio_num_t tb_pins[] = {GPIO_NUM_0, GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3, GPIO_NUM_15};
    for (int i = 0; i < 5; i++) {
        gpio_reset_pin(tb_pins[i]);
        gpio_set_direction(tb_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(tb_pins[i], GPIO_PULLUP_ONLY);
    }

    // 4. Start Retro-Go I2C driver
    rg_i2c_init();

    // 5. Konfigurer TCA9555 tastatur-extender
    uint8_t cfg0[] = {0x06, 0xFF};
    uint8_t cfg1[] = {0x07, 0xFF};
    rg_i2c_write(TDECK_KEYBOARD_ADDR, -1, cfg0, 2);
    rg_i2c_write(TDECK_KEYBOARD_ADDR, -1, cfg1, 2);

    driver_initialized = true;
    return true;
}

bool rg_input_deinit(void)
{
    driver_initialized = false;
    return true;
}

bool rg_input_read_gamepad_raw(uint32_t *out)
{
    if (!driver_initialized) {
        rg_input_init();
    }

    uint32_t gamepad = 0;

    // --- A. TRACKBALL ---
    if (gpio_get_level(GPIO_NUM_3) == 0)  gamepad |= RG_KEY_UP;
    if (gpio_get_level(GPIO_NUM_15) == 0) gamepad |= RG_KEY_DOWN;
    if (gpio_get_level(GPIO_NUM_1) == 0)  gamepad |= RG_KEY_LEFT;
    if (gpio_get_level(GPIO_NUM_2) == 0)  gamepad |= RG_KEY_RIGHT;
    if (gpio_get_level(GPIO_NUM_0) == 0)  gamepad |= RG_KEY_A;

    // --- B. I2C TASTATUR ---
    uint8_t i2c_data[2] = {0xFF, 0xFF};
    if (rg_i2c_read(TDECK_KEYBOARD_ADDR, 0x00, i2c_data, 2)) {
        uint16_t key_pins = i2c_data[0] | (i2c_data[1] << 8);

        if (key_pins != 0xFFFF) {
            if (!(key_pins & (1 << 0))) gamepad |= RG_KEY_UP;
            if (!(key_pins & (1 << 1))) gamepad |= RG_KEY_DOWN;
            if (!(key_pins & (1 << 2))) gamepad |= RG_KEY_LEFT;
            if (!(key_pins & (1 << 3))) gamepad |= RG_KEY_RIGHT;
            if (!(key_pins & (1 << 4))) gamepad |= RG_KEY_A;
            if (!(key_pins & (1 << 5))) gamepad |= RG_KEY_B;
            if (!(key_pins & (1 << 6))) gamepad |= RG_KEY_SELECT;
            if (!(key_pins & (1 << 7))) gamepad |= RG_KEY_START;
            if (!(key_pins & (1 << 8))) gamepad |= RG_KEY_MENU;
        }
    }

    gamepad_state = gamepad;
    if (out) *out = gamepad;

    return true;
}

uint32_t rg_input_read_gamepad(void)
{
    uint32_t val = 0;
    rg_input_read_gamepad_raw(&val);
    return val;
}

int rg_input_get_battery_level(void)
{
    return 100;
}

void rg_input_wait_for_key(int key)
{
    while (!(rg_input_read_gamepad() & key)) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
