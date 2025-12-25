#pragma once

#include <list>

#include "esp_zigbee_type.h"

#include "endpoint.h"
#include "handlers.h"

#define INSTALLCODE_POLICY_ENABLE       false
#define ESP_ZB_PRIMARY_CHANNEL_MASK     ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK

class ZigbeeCore {
    public:
        ZigbeeCore();
        ~ZigbeeCore() {}
        void start();
        void registerEndpoint(ZigbeeDevice* device);
        void setChannelMask(uint32_t mask);
        void searchBindings();
        void deviceUpdate(esp_zb_zdo_signal_device_update_params_t* params);
        esp_err_t handle(esp_zb_core_action_callback_id_t callback_id, const void *message);

        bool connected = false;
        bool started = false;
    protected:
        std::list<ZigbeeDevice *> ep_objects;
    private:
        const char *TAG = "TC-ZC";

        esp_zb_ep_list_t* _zb_ep_list;
        ZigbeeHandlers* handlers;
        uint32_t _primary_channel_mask;
};

extern ZigbeeCore zigbeeCore;
