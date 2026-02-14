#include "sensor.h"

#include "envs.h"

// Defined in main.cpp
extern ZigbeeSensor zbOccupancySensor;

void Envs::handleTemperature() {
    ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));

    float averageTemp = 0, currentTemp = 0;
    for (uint8_t i = 0; i < sensorCount; i++) {
        ESP_ERROR_CHECK(ds18b20_get_temperature(temp[i], &currentTemp));
        averageTemp += (currentTemp / sensorCount);
    }
    zbOccupancySensor.setTemperature(averageTemp);
}

void Envs::setupTemp() {
    onewire_bus_config_t busConfig = {
        .bus_gpio_num = TEMP_PIN,
        .flags = {
            .en_pull_up = false
        }
    };
    onewire_bus_rmt_config_t rmtConfig = {
        .max_rx_bytes = 10
    };

    ESP_ERROR_CHECK(onewire_new_bus_rmt(&busConfig, &rmtConfig, &bus));

    onewire_device_iter_handle_t iter = NULL;
    ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));

    onewire_device_t next;
    esp_err_t result = ESP_OK;
    do {
        result = onewire_device_iter_get_next(iter, &next);
        if (result == ESP_OK) {
            ds18b20_config_t dsCfg = {};
            onewire_device_address_t addr;

            if (ds18b20_new_device_from_enumeration(&next, &dsCfg, &temp[sensorCount]) == ESP_OK) {
                ds18b20_get_device_address(temp[sensorCount], &addr);
                ESP_LOGD(TAG, "Found sensor[%d], address: %016llX\n", sensorCount, addr);
                sensorCount++;
            } else {
                ESP_LOGD(TAG, "Found unknown device, address: %016llX\n", next.address);
            }
        }
    } while (result != ESP_ERR_NOT_FOUND);
    ESP_ERROR_CHECK(onewire_del_device_iter(iter));
    ESP_LOGD(TAG, "Found %d temperature sensors\n", sensorCount);
}

void Envs::setupLight() {
    ESP_ERROR_CHECK(i2cdev_init());

    ESP_ERROR_CHECK(tsl2591_init_desc(&light, I2C_NUM_0, SDA_PIN, SCL_PIN));
    esp_err_t res = tsl2591_init(&light);

    if (res == ESP_OK) {
        lightFound = true;

        ESP_ERROR_CHECK(tsl2591_set_power_status(&light, TSL2591_POWER_ON));
        ESP_ERROR_CHECK(tsl2591_set_als_status(&light, TSL2591_ALS_ON));
        ESP_ERROR_CHECK(tsl2591_set_gain(&light, TSL2591_GAIN_MEDIUM));
        ESP_ERROR_CHECK(tsl2591_set_integration_time(&light, TSL2591_INTEGRATION_300MS));
    }
}

void Envs::checkLux() {
    if (!inhibit && lux > threshold) {
        inhibit = true;
    } else if (inhibit && lux < threshold - LUX_HYSTERESIS) {
        inhibit = false;
    }
}

void Envs::handleLight() {
    if (!lightFound) return;

    float luxLocal;

    if (tsl2591_get_lux(&light, &luxLocal) == ESP_OK) {
        readings[luxIdx] = luxLocal;
        luxIdx = (luxIdx + 1) % LUX_SAMPLES;

        float luxSum = 0;
        for (uint8_t i = 0; i < LUX_SAMPLES; i++) {
            luxSum += readings[i];
        }

        lux = luxSum / LUX_SAMPLES;
        ESP_LOGV(TAG, "Light value: %.2f, Smoothed: %.2f\n", luxLocal, lux);

        checkLux();
        zbOccupancySensor.setIlluminance(lux);
    }
}

bool Envs::getInhibit() {
    return inhibit;
}

void Envs::setInhibit(float _threshold) {
    threshold = _threshold;
    checkLux();
}

void sensor_task(void *pvParameters) {
    envs.setup();

    while (true) {
        envs.task();
    }
}

void Envs::setup() {
    setupTemp();
    setupLight();
}

void Envs::task() {
    handleTemperature();
    handleLight();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
}

Envs envs;
