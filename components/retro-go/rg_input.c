#include "rg_input.h"
#include "rg_system.h"
#include "rg_i2c.h"
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define T_DECK_KBD_ADDRESS 0x20

volatile uint32_t gamepad_state = 0;
static bool input_task_running = false;
static esp_adc_cal_characteristics_t *adc_chars;

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

    // 4. Konfigurer den rigtige Batteri ADC-driver til T-Deck
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, adc_chars);

    input_task_running = true;
    return true;
}

void rg_input_deinit(void)
{
    input_task_running = false;
}

uint32_t rg_input_read_gamepad_raw(void)
{
    uint32_t state = 0;
    uint8_t data = {0xFF, 0xFF};

    // Læs I2C-data fra din TCA9555 chip på adresse 0x20
    if (rg_i2c_read(T_DECK_KBD_ADDRESS, 0x00, data, 2)) {
        if (!(data & (1 << 4))) state |= RG_KEY_A;      // Enter / Knap A
        if (!(data & (1 << 5))) state |= RG_KEY_B;      // Space / Knap B
        if (!(data & (1 << 6))) state |= RG_KEY_SELECT; // Select
        if (!(data & (1 << 7))) state |= RG_KEY_START;  // Start
    }

    // Læs de direkte fysiske retnings-pins fra trackball'en
    if (!gpio_get_level(GPIO_NUM_10)) state |= RG_KEY_UP;
    if (!gpio_get_level(GPIO_NUM_15)) state |= RG_KEY_DOWN;
    if (!gpio_get_level(GPIO_NUM_11)) state |= RG_KEY_LEFT;
    if (!gpio_get_level(GPIO_NUM_14)) state |= RG_KEY_RIGHT;
    if (!gpio_get_level(GPIO_NUM_9))  state |= RG_KEY_MENU; // Trackball-klik = Menu

    return state;
}

uint32_t rg_input_read_gamepad(void)
{
    gamepad_state = rg_input_read_gamepad_raw();
    return gamepad_state;
}

bool rg_input_key_is_pressed(rg_key_t key)
{
    return (rg_input_read_gamepad() & key) == key;
}

bool rg_input_wait_for_key(rg_key_t key_mask, bool pressed, int timeout_ms)
{
    rg_task_delay(timeout_ms);
    return true;
}

rg_battery_t rg_input_read_battery(void)
{
    rg_battery_t battery = { .voltage = 4.0f, .percentage = 80, .is_charging = false };
    
    // Læs den ægte spænding fra T-Deck hardwaren
    uint32_t raw = adc1_get_raw(ADC1_CHANNEL_3);
    uint32_t mv = esp_adc_cal_raw_to_voltage(raw, adc_chars) * 2; // Gange 2 pga. spændingsdeler
    
    battery.voltage = mv * 0.001f;
    battery.percentage = ((battery.voltage - 3.5f) / (4.2f - 3.5f)) * 100.0f;
    
    if (battery.percentage > 100) battery.percentage = 100;
    if (battery.percentage < 0) battery.percentage = 0;
    
    return battery;
}

const char *rg_input_get_key_name(rg_key_t key)
{
    return "Key";
}

void rg_input_setup_gamepad(void) {}
