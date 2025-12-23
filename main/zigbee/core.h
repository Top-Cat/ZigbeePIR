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
        esp_err_t handle(esp_zb_core_action_callback_id_t callback_id, const void *message);
    protected:
        std::list<ZigbeeDevice *> ep_objects;
    private:
        esp_zb_ep_list_t* _zb_ep_list;
        ZigbeeHandlers* handlers;
};

extern ZigbeeCore zigbeeCore;
