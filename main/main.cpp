#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "main.h"
#include "driver/gpio.h"

#include "config.h"
#include "sensor.h"
#include "zigbee/handlers.h"
#include "zigbee/core.h"

ZigbeeSensor zbOccupancySensor = ZigbeeSensor(10);

void IRAM_ATTR pirISR() {
    occupancy_changed = true;
}

void IRAM_ATTR buttonISR() {
    button_pressed = true;
}

void IRAM_ATTR switchISR() {
    switch_pressed = true;
}

void setOnOff(bool onOff) {
    gpio_set_level(LED_PIN, onOff);
}

// TODO: Replace LED with WS2812B driver
// TODO: Seperate occupancy timeout and light timeout
void setOccupied(bool newVal) {
  occupancy_state = newVal;
  setOnOff(occupancy_state);
  zbOccupancySensor.setOccupancy(occupancy_state);
  zbOccupancySensor.setOnOff(occupancy_state);
  zbOccupancySensor.report();
}

static esp_err_t deferred_driver_init(void) {
    light_driver_init(LIGHT_DEFAULT_OFF);
    return ESP_OK;
}

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask) {
    ESP_RETURN_ON_FALSE(esp_zb_bdb_start_top_level_commissioning(mode_mask) == ESP_OK, , TAG, "Failed to start Zigbee commissioning");
}

// TODO: Add more handlers from arduino-esp32/zigbee
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
    uint32_t *p_sg_p       = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;   
    esp_zb_app_signal_type_t sig_type = (esp_zb_app_signal_type_t)*p_sg_p;
    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Deferred driver initialization %s", deferred_driver_init() ? "failed" : "successful");
            ESP_LOGI(TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Start network steering");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                ESP_LOGI(TAG, "Device rebooted");
            }
        } else {
            /* commissioning failed */
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
            zigbeeConnected = true;
        } else {
            ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
        }
        break;
    default:
        ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                 esp_err_to_name(err_status));
        break;
    }
}

void handlePIR() {
    if (occupancy_changed) {
        occupancy_changed = false;

        if (gpio_get_level(SENSOR_PIN)) {
            lastMotionUs = esp_timer_get_time();

            if (!occupancy_state && (esp_timer_get_time() < HOLDOUT_US || esp_timer_get_time() - holdOutUs >= HOLDOUT_US)) {
                setOccupied(true);
                ESP_LOGI(TAG, "Occupancy detected");
            }
        }
    }

    uint16_t occupancyTimeoutSec = zbOccupancySensor.getTimeout();
    if (occupancy_state && esp_timer_get_time() - lastMotionUs >= occupancyTimeoutSec * 1000000ULL) {
        setOccupied(false);
        ESP_LOGI(TAG, "Occupancy cleared");
    }
}

void handleResetButton() {
    if (!button_pressed)
        return;

    button_pressed = false;

    uint64_t pressStart = esp_timer_get_time();
    while (gpio_get_level(BUTTON_PIN) == 0) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        if (esp_timer_get_time() - pressStart > 3000000) {
            ESP_LOGW(TAG, "Resetting Zigbee to factory and rebooting in 1s.");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            esp_zb_factory_reset();
        }
    }
}

void handleHeartbeat() {
    if (esp_timer_get_time() - lastHeartbeat <= HEARTBEAT_INTERVAL)
        return;

    lastHeartbeat = esp_timer_get_time();
    if (zigbeeConnected) {
        zbOccupancySensor.report();
    } else {
        ESP_LOGI(TAG, "Zigbee not connected, attempting reconnect...");
        zigbeeCore.start();
    }
}

void handleSwitch() {
    if (!switch_pressed)
        return;

    switch_pressed = false;

    // De-bounce, went low, still low
    if (gpio_get_level(SWITCH_PIN))
        return;

    ESP_LOGI(TAG, "Switch pressed");

    if (occupancy_state) {
        holdOutUs = esp_timer_get_time();
    } else {
        lastMotionUs = esp_timer_get_time();
    }

    setOccupied(!occupancy_state);
}

static void main_task(void *pvParameters) {
    handlePIR();
    handleSwitch();
    handleResetButton();
    handleHeartbeat();

    // Can't sleep as we're a zigbee router
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

extern "C" void app_main(void) {
    gpio_config_t gpioConfig = {
        .pin_bit_mask = (1ULL << BUTTON_PIN) | (1ULL << SWITCH_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&gpioConfig);

    gpioConfig.pin_bit_mask = 1ULL << SENSOR_PIN;
    gpioConfig.intr_type = GPIO_INTR_ANYEDGE;
    gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&gpioConfig);

    gpioConfig.pin_bit_mask = (1ULL << LED_PIN) | (1ULL << WS2812_PIN);
    gpioConfig.intr_type = GPIO_INTR_DISABLE;
    gpioConfig.mode = GPIO_MODE_OUTPUT;
    gpio_config(&gpioConfig);

    zigbeeCore.registerEndpoint(&zbOccupancySensor);
    zigbeeCore.start();

    xTaskCreate(main_task, "Main", 4096, NULL, 4, NULL);

    if (gpio_get_level(SENSOR_PIN)) {
        occupancy_changed = true;
    }
    ESP_LOGI(TAG, "app_main complete");
}
