#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_zigbee_core.h"
#include "../helpers.h"

#include "endpoint.h"

ZigbeeDevice::ZigbeeDevice(esp_zb_ha_standard_devices_t deviceId, uint8_t endpoint) {
    _endpoint = endpoint;
    _cluster_list = createClusters();
    _ep_config = {
        .endpoint = _endpoint,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = (uint16_t) deviceId,
        .app_device_version = 0
    };
}

void ZigbeeDevice::zbReadTimeCluster(const esp_zb_zcl_attribute_t *attribute) {
    if (attribute->id == ESP_ZB_ZCL_ATTR_TIME_TIME_ID && attribute->data.type == ESP_ZB_ZCL_ATTR_TYPE_UTC_TIME) {
        ESP_LOGV(PTAG, "Time attribute received");
        ESP_LOGV(PTAG, "Time: %lld", *(uint32_t *)attribute->data.value);
        _read_time = *(uint32_t *)attribute->data.value;
        xSemaphoreGive(lock);
    } else if (attribute->id == ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID && attribute->data.type == ESP_ZB_ZCL_ATTR_TYPE_S32) {
        ESP_LOGV(PTAG, "Timezone attribute received");
        ESP_LOGV(PTAG, "Timezone: %d", *(int32_t *)attribute->data.value);
        _read_timezone = *(int32_t *)attribute->data.value;
        xSemaphoreGive(lock);
    }
}


bool ZigbeeDevice::setTime(tm time) {
    esp_zb_zcl_status_t ret = ESP_ZB_ZCL_STATUS_SUCCESS;
    time_t utc_time = mktime(&time);
    ESP_LOGD(PTAG, "Setting time to %lld", utc_time);
    esp_zb_lock_acquire(portMAX_DELAY);
    ret = esp_zb_zcl_set_attribute_val(_endpoint, ESP_ZB_ZCL_CLUSTER_ID_TIME, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, ESP_ZB_ZCL_ATTR_TIME_TIME_ID, &utc_time, false);
    esp_zb_lock_release();
    if (ret != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(PTAG, "Failed to set time: 0x%x: %s", ret, esp_zb_zcl_status_to_name(ret));
        return false;
    }
    return true;
}

bool ZigbeeDevice::setTimezone(int32_t gmt_offset) {
    esp_zb_zcl_status_t ret = ESP_ZB_ZCL_STATUS_SUCCESS;
    ESP_LOGD(PTAG, "Setting timezone to %d", gmt_offset);
    esp_zb_lock_acquire(portMAX_DELAY);
    ret =  esp_zb_zcl_set_attribute_val(_endpoint, ESP_ZB_ZCL_CLUSTER_ID_TIME, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID, &gmt_offset, false);
    esp_zb_lock_release();
    if (ret != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(PTAG, "Failed to set timezone: 0x%x: %s", ret, esp_zb_zcl_status_to_name(ret));
        return false;
    }
    return true;
}

tm ZigbeeDevice::getTime(uint8_t endpoint, int32_t short_addr, esp_zb_ieee_addr_t ieee_addr) {
    esp_zb_zcl_read_attr_cmd_t read_req;
    memset(&read_req, 0, sizeof(read_req));

    if (short_addr >= 0) {
        read_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
        read_req.zcl_basic_cmd.dst_addr_u.addr_short = (uint16_t)short_addr;
    } else {
        read_req.address_mode = ESP_ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
        memcpy(read_req.zcl_basic_cmd.dst_addr_u.addr_long, ieee_addr, sizeof(esp_zb_ieee_addr_t));
    }

    uint16_t attributes[] = {ESP_ZB_ZCL_ATTR_TIME_TIME_ID};
    read_req.attr_number = ZB_ARRAY_LENGTH(attributes);
    read_req.attr_field = attributes;

    read_req.clusterID = ESP_ZB_ZCL_CLUSTER_ID_TIME;

    read_req.zcl_basic_cmd.dst_endpoint = endpoint;
    read_req.zcl_basic_cmd.src_endpoint = _endpoint;

    _read_time = 0;

    ESP_LOGV(PTAG, "Reading time from endpoint %d", endpoint);
    esp_zb_zcl_read_attr_cmd_req(&read_req);

    if (xSemaphoreTake(lock, ZB_CMD_TIMEOUT) != pdTRUE) {
        ESP_LOGE(PTAG, "Error while reading time");
        return tm();
    }

    struct tm *timeinfo = localtime(&_read_time);
    if (timeinfo) {
        setTime(*timeinfo);

        _time_status |= 0x02;
        esp_zb_lock_acquire(portMAX_DELAY);
        esp_zb_zcl_set_attribute_val(_endpoint, ESP_ZB_ZCL_CLUSTER_ID_TIME, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, ESP_ZB_ZCL_ATTR_TIME_TIME_STATUS_ID, &_time_status, false);
        esp_zb_lock_release();

        return *timeinfo;
    } else {
        ESP_LOGE(PTAG, "Error while converting time");
        return tm();
    }
}

int32_t ZigbeeDevice::getTimezone(uint8_t endpoint, int32_t short_addr, esp_zb_ieee_addr_t ieee_addr) {
    esp_zb_zcl_read_attr_cmd_t read_req;
    memset(&read_req, 0, sizeof(read_req));

    if (short_addr >= 0) {
        read_req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
        read_req.zcl_basic_cmd.dst_addr_u.addr_short = (uint16_t)short_addr;
    } else {
        read_req.address_mode = ESP_ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
        memcpy(read_req.zcl_basic_cmd.dst_addr_u.addr_long, ieee_addr, sizeof(esp_zb_ieee_addr_t));
    }

    uint16_t attributes[] = {ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID};
    read_req.attr_number = ZB_ARRAY_LENGTH(attributes);
    read_req.attr_field = attributes;

    read_req.clusterID = ESP_ZB_ZCL_CLUSTER_ID_TIME;

    read_req.zcl_basic_cmd.dst_endpoint = endpoint;
    read_req.zcl_basic_cmd.src_endpoint = _endpoint;

    _read_timezone = 0;

    ESP_LOGV(PTAG, "Reading timezone from endpoint %d", endpoint);
    esp_zb_zcl_read_attr_cmd_req(&read_req);

    //Wait for response or timeout
    if (xSemaphoreTake(lock, ZB_CMD_TIMEOUT) != pdTRUE) {
        ESP_LOGE(PTAG, "Error while reading timezone");
        return 0;
    }
    setTimezone(_read_timezone);
    return _read_timezone;
}
