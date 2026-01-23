#include "esp_log.h"
#include "driver/ledc.h"
#include "config.h"

#include "light_driver.h"

LightDriver ledDriver;

void LightDriver::setPowerTarget(uint8_t target) {
    powerTarget = target;
}

void LightDriver::setLevels(uint8_t amber, uint8_t warm_white, uint8_t cool_white, uint16_t count) {
    s_warm = warm_white / 255.0;
    s_cold = cool_white / 255.0;
    s_amber = amber / 255.0;

    for (uint16_t i = driverLedCount; i > count; i--) {
        ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, i - 1, 0, 0, 0));
    }
    driverLedCount = count;
}

void LightDriver::setAnimation(FadeAnimation _animation) {
    animation = _animation;
}

void setLevels(uint8_t amber, uint8_t warm_white, uint8_t cool_white, uint16_t count, uint8_t animation) {
    ledDriver.setLevels(amber, warm_white, cool_white, count);
    ledDriver.setAnimation((FadeAnimation) animation);
}

const double &min(const double &a, const double &b) {
    return (b < a) ? b : a;
}

const double &max(const double &a, const double &b) {
    return (b > a) ? b : a;
}

uint8_t LightDriver::calculateFromEnds(uint8_t power, uint8_t i) {
    double travel = (power / 255.0) * ((driverLedCount / 2.0) + fadeWidth);
    double delta = travel - min(i, driverLedCount - 1 - i);
    return max(0.0, min(255.0, 255.0 * delta / fadeWidth));
}

void LightDriver::setPower(uint8_t power) {
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, power * 32));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

    for (uint16_t i = 0; i < driverLedCount; i++) {
        uint8_t newPower = power;
        switch (animation) {
            case FadeAnimation::FROM_ENDS:
                newPower = calculateFromEnds(power, i);
                break;
            case FadeAnimation::ROWS:
                newPower = i % 2 == 0 ?
                    (power > 128 ? 255 : power * 2) :
                    (power > 128 ? (power - 128) * 2 : 0);
                break;
            case FadeAnimation::BASIC:
            default:
                break;
        }
        ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, i, s_cold * newPower, s_amber * newPower, s_warm * newPower));
    }
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

void LightDriver::init() {
    // Debug LED
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 4000,
        .clk_cfg          = LEDC_AUTO_CLK,
        .deconfigure = false
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = LEDC_PIN,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0,
        .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = {
            .output_invert = false
        }
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // Led strip
    led_strip_config_t led_strip_conf = {
        .strip_gpio_num = WS2812_PIN,
        .max_leds = WS2812_MAX,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
        .flags = {
            .invert_out = false
        }
    };
    led_strip_rmt_config_t rmt_conf = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 48,
        .flags = {
            .with_dma = false
        }
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_strip_conf, &rmt_conf, &s_led_strip));
}

void lightTask(void *pvParameters) {
    ledDriver.task();
}

void LightDriver::task() {
    ledDriver.init();
    uint8_t power = 0;
    uint8_t speed = 3;
    uint8_t refresh = 10;

    while (true) {
        if (power < powerTarget) {
            power += speed;
        } else if (power > powerTarget) {
            power -= speed;
        } else {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            if (refresh-- > 1) continue;
            refresh = 10;
        }

        ledDriver.setPower(power);
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}