#pragma once

#include "time.h"

#include "esp_zigbee_type.h"
#include "zcl/esp_zigbee_zcl_common.h"
#include "zcl/esp_zigbee_zcl_core.h"

#define ZB_CMD_TIMEOUT 10000
#define ZB_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

class ZigbeeDevice {
    public:
        ZigbeeDevice(esp_zb_ha_standard_devices_t deviceId, uint8_t endpoint = 10);
        ~ZigbeeDevice() {}

        void zbReadTimeCluster(const esp_zb_zcl_attribute_t *attribute);
    protected:
        uint8_t _endpoint;
        esp_zb_endpoint_config_t _ep_config;
        esp_zb_cluster_list_t* _cluster_list;
        uint8_t _time_status;

        struct tm getTime(uint8_t endpoint = 1, int32_t short_addr = 0x0000, esp_zb_ieee_addr_t ieee_addr = {0});
        int32_t getTimezone(uint8_t endpoint = 1, int32_t short_addr = 0x0000, esp_zb_ieee_addr_t ieee_addr = {0});

        virtual esp_zb_cluster_list_t* createClusters() {
            return NULL;
        }
        virtual void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) {}
        virtual void zbAttributeRead(uint16_t cluster_id, const esp_zb_zcl_attribute_t *attribute, uint8_t src_endpoint, esp_zb_zcl_addr_t src_address) {}
    private:
        const char *PTAG = "TC-ZBD";
        time_t _read_time;
        int32_t _read_timezone;
        SemaphoreHandle_t lock;

        bool setTime(tm time);
        bool setTimezone(int32_t gmt_offset);

    friend class ZigbeeCore;
    friend class ZigbeeHandlers;
};
