#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "rg_system.h"
#include "rg_input.h"
#include "rg_i2c.h"

static const char *TAG = "rg_input";

// --- T-DECK SPECIFIK HARDWARE-KONFIGURATION ---
#define TDECK_I2C_SDA          GPIO_NUM_18
#define TDECK_I2C_SCL          GPIO_NUM_8
#define TDECK_I2C_PWR          GPIO_NUM_46
#define TDECK_KEYBOARD_ADDR    0x20

static QueueHandle_t input_queue = NULL;
static TaskHandle_t input_task_handle = NULL;
static int gamepad_state = 0;
static bool driver_initialized = false;

// Tving I2C-bussen til at nulstille (Løser blokeret I2C fra M5Launcher)
static void tdeck_force_i2c_bus_reset(void)
{
    gpio_reset_pin(TDECK_I2C_SDA);
    gpio_reset_pin(TDECK_I2C_SCL);
    gpio_set_direction(TDECK_I2C_SDA, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_direction(TDECK_I2C_SCL, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(TDECK_I2C_SDA, 1);
    gpio_set_level(TDECK_I2C_SCL, 1);

    for (int i = 0; i < 9; i++) {
        gpio_set_level(TDECK_I2C_SCL, 0);
        vTaskDelay(pdMS_TO_TICKS(1));
        gpio_set_level(TDECK_I2C_SCL, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// T-Deck Hardware Initialisering
static void tdeck_init_hardware(void)
{
    // 1. Tænd strøm til I2C/skærm/periferi (GPIO 46)
    gpio_reset_pin(TDECK_I2C_PWR);
    gpio_set_direction(TDECK_I2C_PWR, GPIO_MODE_OUTPUT);
    gpio_set_level(TDECK_I2C_PWR, 1);
    vTaskDelay(pdMS_TO_TICKS(30));

    // 2. Fysisk reset af I2C bussen
    tdeck_force_i2c_bus_reset();

    // 3. Modstande (PULL-UPs) på trackball for at fjerne spøgelses-bevægelser
    const gpio_num_t tb_pins[] = {GPIO_NUM_0, GPIO_NUM_1, GPIO_NUM_2, GPIO_NUM_3, GPIO_NUM_15};
    for (int i = 0; i < 5; i++) {
        gpio_reset_pin(tb_pins[i]);
        gpio_set_direction(tb_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(tb_pins[i], GPIO_PULLUP_ONLY);
    }

    // 4. Initialiser Retro-Go I2C og konfigurer tastatur-chippen (TCA9555)
    rg_i2c_init();
    uint8_t cfg0[] = {0x06, 0xFF};
    uint8_t cfg1[] = {0x07, 0xFF};
    rg_i2c_write(TDECK_KEYBOARD_ADDR, -1, cfg0, 2);
    rg_i2c_write(TDECK_KEYBOARD_ADDR, -1, cfg1, 2);
}

bool rg_input_read_gamepad_raw(uint32_t *out)
{
    if (!driver_initialized) {
        tdeck_init_hardware();
        driver_initialized = true;
    }

    uint32_t gamepad = 0;

    // --- A. LÆS TRACKBALL ---
    if (gpio_get_level(GPIO_NUM_3) == 0)  gamepad |= RG_KEY_UP;
    if (gpio_get_level(GPIO_NUM_15) == 0) gamepad |= RG_KEY_DOWN;
    if (gpio_get_level(GPIO_NUM_1) == 0)  gamepad |= RG_KEY_LEFT;
    if (gpio_get_level(GPIO_NUM_2) == 0)  gamepad |= RG_KEY_RIGHT;
    if (gpio_get_level(GPIO_NUM_0) == 0)  gamepad |= RG_KEY_A;

    // --- B. LÆS TASTATUR VIA I2C (TCA9555) ---
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

    if (out) {
        *out = gamepad;
    }

    return true;
}

static void rg_input_task(void *arg)
{
    rg_input_event_t event;
    uint32_t last_state = 0;

    while (1) {
        uint32_t current_state = 0;
        rg_input_read_gamepad_raw(&current_state);
        gamepad_state = current_state;

        // Generer input-events hvis knaptilstanden ændrer sig
        uint32_t changed = current_state ^ last_state;
        if (changed && input_queue) {
            for (int i = 0; i < 32; i++) {
                uint32_t mask = (1 << i);
                if (changed & mask) {
                    event.key = mask;
                    event.pressed = (current_state & mask) ? true : false;
                    xQueueSend(input_queue, &event, 0);
                }
            }
        }

        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

bool rg_input_init(void)
{
    if (driver_initialized) {
        return true;
    }

    ESP_LOGI(TAG, "Initializing Retro-Go Input for T-Deck...");

    tdeck_init_hardware();

    input_queue = xQueueCreate(16, sizeof(rg_input_event_t));

    xTaskCreatePinnedToCore(rg_input_task, "rg_input_task", 3072, NULL, 5, &input_task_handle, 1);

    driver_initialized = true;
    ESP_LOGI(TAG, "Input Subsystem Ready.");
    return true;
}

bool rg_input_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing Input Subsystem...");
    if (input_task_handle) {
        vTaskDelete(input_task_handle);
        input_task_handle = NULL;
    }
    if (input_queue) {
        vQueueDelete(input_queue);
        input_queue = NULL;
    }
    driver_initialized = false;
    return true;
}

uint32_t rg_input_read_gamepad(void)
{
    return gamepad_state;
}

bool rg_input_get_event(rg_input_event_t *event, uint32_t timeout_ms)
{
    if (!input_queue) return false;
    return xQueueReceive(input_queue, event, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
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
