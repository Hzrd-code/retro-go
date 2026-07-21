#include "rg_input.h"
#include "rg_system.h"
#include "rg_i2c.h"
#include <driver/gpio.h>

#define T_DECK_KBD_ADDRESS 0x20

static uint32_t gamepad_state = 0;
static bool input_task_running = false;

bool rg_input_init(void)
{
    // 1. Tænd for strømmen og reset-pinnen til tastatur-chippen (GPIO 46)
    gpio_reset_pin(GPIO_NUM_46);
    gpio_set_direction(GPIO_NUM_46, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_46, 1);
    rg_task_delay(50);

    // 2. Start Retro-Go's indbyggede I2C-driver
    rg_i2c_init();

    // 3. Konfigurer trackball retnings-pins direkte på ESP32-S3
    gpio_reset_pin(GPIO_NUM_10); gpio_set_direction(GPIO_NUM_10, GPIO_MODE_INPUT); gpio_set_pull_mode(GPIO_NUM_10, GPIO_PULLUP_ONLY);
    gpio_reset_pin(GPIO_NUM_15); gpio_set_direction(GPIO_NUM_15, GPIO_MODE_INPUT); gpio_set_pull_mode(GPIO_NUM_15, GPIO_PULLUP_ONLY);
    gpio_reset_pin(GPIO_NUM_11); gpio_set_direction(GPIO_NUM_11, GPIO_MODE_INPUT); gpio_set_pull_mode(GPIO_NUM_11, GPIO_PULLUP_ONLY);
    gpio_reset_pin(GPIO_NUM_14); gpio_set_direction(GPIO_NUM_14, GPIO_MODE_INPUT); gpio_set_pull_mode(GPIO_NUM_14, GPIO_PULLUP_ONLY);
    gpio_reset_pin(GPIO_NUM_9);  gpio_set_direction(GPIO_NUM_9,  GPIO_MODE_INPUT); gpio_set_pull_mode(GPIO_NUM_9,  GPIO_PULLUP_ONLY);

    input_task_running = true;
    RG_LOGI("T-Deck Input hardware initialized successfully.\n");
    return true;
}

void rg_input_deinit(void)
{
    input_task_running = false;
}

uint32_t rg_input_read_gamepad(void)
{
    gamepad_state = 0;
    uint8_t data[2] = {0xFF, 0xFF};

    // Læs I2C-data fra din TCA9555 chip på adresse 0x20
    if (rg_i2c_read(T_DECK_KBD_ADDRESS, 0x00, data, 2)) {
        if (!(data[0] & (1 << 4))) gamepad_state |= RG_KEY_A;      // Enter / Knap A
        if (!(data[0] & (1 << 5))) gamepad_state |= RG_KEY_B;      // Space / Knap B
        if (!(data[0] & (1 << 6))) gamepad_state |= RG_KEY_SELECT; // Select
        if (!(data[0] & (1 << 7))) gamepad_state |= RG_KEY_START;  // Start
    }

    // Læs de direkte fysiske retnings-pins fra trackball'en
    if (!gpio_get_level(GPIO_NUM_10)) gamepad_state |= RG_KEY_UP;
    if (!gpio_get_level(GPIO_NUM_15)) gamepad_state |= RG_KEY_DOWN;
    if (!gpio_get_level(GPIO_NUM_11)) gamepad_state |= RG_KEY_LEFT;
    if (!gpio_get_level(GPIO_NUM_14)) gamepad_state |= RG_KEY_RIGHT;
    if (!gpio_get_level(GPIO_NUM_9))  gamepad_state |= RG_KEY_MENU; // Trackball-klik = Menu

    return gamepad_state;
}

bool rg_input_key_is_pressed(rg_key_t key)
{
    return (rg_input_read_gamepad() & key) == key;
}

rg_battery_t rg_input_read_battery(void)
{
    rg_battery_t battery = { .voltage = 4.0f, .percentage = 80, .is_charging = false };
    return battery;
}

const char *rg_input_get_key_name(rg_key_t key)
{
    switch (key) {
        case RG_KEY_UP:     return "Up";
        case RG_KEY_RIGHT:  return "Right";
        case RG_KEY_DOWN:   return "Down";
        case RG_KEY_LEFT:   return "Left";
        case RG_KEY_SELECT: return "Select";
        case RG_KEY_START:  return "Start";
        case RG_KEY_MENU:   return "Menu";
        case RG_KEY_A:      return "A";
        case RG_KEY_B:      return "B";
        default:            return "Unknown";
    }
}
