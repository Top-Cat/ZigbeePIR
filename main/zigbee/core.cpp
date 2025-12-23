#include "nvs_flash.h"
#include "esp_zigbee_core.h"

#include "core.h"

ZigbeeCore zigbeeCore;

static void esp_zb_task(void *pvParameters) {
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_stack_main_loop();
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message) {
    return zigbeeCore.handle(callback_id, message);
}

ZigbeeCore::ZigbeeCore() {
    handlers = new ZigbeeHandlers(&ep_objects);
    _zb_ep_list = esp_zb_ep_list_create();
}

esp_err_t ZigbeeCore::handle(esp_zb_core_action_callback_id_t callback_id, const void *message) {
    return handlers->handle(callback_id, message);
}

void ZigbeeCore::start() {
    ESP_ERROR_CHECK(nvs_flash_init());

    esp_zb_platform_config_t config = {
        .radio_config = {
            .radio_mode = ZB_RADIO_MODE_NATIVE,
            .radio_uart_config = {}
        },
        .host_config = {
            .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE,
            .host_uart_config = {}
        }
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    esp_zb_cfg_t zb_nwk_cfg = {
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ROUTER,
        .install_code_policy = INSTALLCODE_POLICY_ENABLE,
        .nwk_cfg = {
            .zczr_cfg = {
                .max_children = 10
            }
        }
    };
    esp_zb_init(&zb_nwk_cfg);

    ESP_ERROR_CHECK(esp_zb_device_register(_zb_ep_list));
    esp_zb_core_action_handler_register(zb_action_handler);
    ESP_ERROR_CHECK(esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK));
    // esp_zb_aps_data_indication_handler_register();

    xTaskCreate(esp_zb_task, "Zigbee", 4096, NULL, 5, NULL);
}

void ZigbeeCore::registerEndpoint(ZigbeeDevice* device) {
    ep_objects.push_back(device);

    ESP_ERROR_CHECK(esp_zb_ep_list_add_ep(_zb_ep_list, device->_cluster_list, device->_ep_config));
}
