#include "nvs_flash.h"
#include "esp_zigbee_core.h"
#include "esp_log.h"

#include "core.h"

extern "C" {
    #include "zboss_api.h"
    extern zb_ret_t zb_nvram_write_dataset(zb_nvram_dataset_types_t t);
}

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
    _primary_channel_mask = ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK;
    _zb_ep_list = esp_zb_ep_list_create();
}

esp_err_t ZigbeeCore::handle(esp_zb_core_action_callback_id_t callback_id, const void *message) {
    return handlers->handle(callback_id, message);
}

const char *APSDE_TAG = "APSDE";
bool zb_apsde_data_indication_handler(esp_zb_apsde_data_ind_t ind) {
    ESP_LOGD(APSDE_TAG, "APSDE INDICATION - Received APSDE-DATA indication, status: %d", ind.status);
    ESP_LOGD(APSDE_TAG,
        "APSDE INDICATION - dst_endpoint: %d, src_endpoint: %d, dst_addr_mode: %d, src_addr_mode: %d, cluster_id: 0x%04x, asdu_length: %d", ind.dst_endpoint,
        ind.src_endpoint, ind.dst_addr_mode, ind.src_addr_mode, ind.cluster_id, ind.asdu_length
    );
    ESP_LOGD(APSDE_TAG,
        "APSDE INDICATION - dst_short_addr: 0x%04x, src_short_addr: 0x%04x, profile_id: 0x%04x, security_status: %d, lqi: %d, rx_time: %d", ind.dst_short_addr,
        ind.src_short_addr, ind.profile_id, ind.security_status, ind.lqi, ind.rx_time
    );

    if (ind.status == 0x00) {
        // Catch bind/unbind requests to update the bound devices list
        if (ind.cluster_id == 0x21 || ind.cluster_id == 0x22) {
            zigbeeCore.searchBindings();
        }
    } else {
        ESP_LOGE(APSDE_TAG, "APSDE INDICATION - Invalid status of APSDE-DATA indication, error code: %d", ind.status);
    }
    return false;  //False to let the stack process the message as usual
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
    esp_zb_aps_data_indication_handler_register(zb_apsde_data_indication_handler);

    xTaskCreate(esp_zb_task, "Zigbee", 4096, NULL, 5, NULL);
}

void ZigbeeCore::registerEndpoint(ZigbeeDevice* device) {
    ep_objects.push_back(device);

    ESP_ERROR_CHECK(esp_zb_ep_list_add_ep(_zb_ep_list, device->_cluster_list, device->_ep_config));
}

void ZigbeeCore::setChannelMask(uint32_t mask) {
    _primary_channel_mask = mask;
    esp_zb_set_channel_mask(_primary_channel_mask);
    zb_nvram_write_dataset(ZB_NVRAM_COMMON_DATA);
    ESP_LOGV(TAG, "Channel mask set to 0x%08x", mask);
}

void ZigbeeCore::searchBindings() {
    // NOT IMPLMENTED - Recall devices from binding table
    ESP_LOGW(TAG, "searchBindings not implemented");
}

void ZigbeeCore::deviceUpdate(esp_zb_zdo_signal_device_update_params_t* params) {
    // NOT IMPLMENTED - Used for updating bindings
    ESP_LOGW(TAG, "deviceUpdate not implemented");
}
