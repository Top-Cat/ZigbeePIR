#include "esp_zigbee_core.h"
#include "esp_zigbee_cluster.h"
#include "esp_zigbee_attribute.h"

#include "esp_log.h"

#include "config.h"
#include "helpers.h"

#include "sensor.h"

uint16_t occupancyTimeoutSec = 60;
const char* swBuildId = SW_VERSION;
const char* dateCode = DATE_CODE;

void ZigbeeSensor::createBasicCluster(esp_zb_cluster_list_t* cluster_list) {
    esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(&basic_cfg);
    esp_zb_cluster_list_add_basic_cluster(cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    char zigbee_swid[16];
    fill_zcl_string(zigbee_swid, sizeof(zigbee_swid), swBuildId);

    esp_zb_cluster_add_attr(
        basic_cluster,
        ESP_ZB_ZCL_CLUSTER_ID_BASIC,
        ESP_ZB_ZCL_ATTR_BASIC_SW_BUILD_ID,
        ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,
        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,
        (void *)&zigbee_swid
    );

    char zigbee_datecode[50];
    fill_zcl_string(zigbee_datecode, sizeof(zigbee_datecode), dateCode);

    esp_zb_cluster_add_attr(
        basic_cluster,
        ESP_ZB_ZCL_CLUSTER_ID_BASIC,
        ESP_ZB_ZCL_ATTR_BASIC_DATE_CODE_ID,
        ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING,
        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,
        (void *)&zigbee_datecode
    );

    uint16_t stack_version = 0x30;

    esp_zb_cluster_add_attr(
        basic_cluster,
        ESP_ZB_ZCL_CLUSTER_ID_BASIC,
        ESP_ZB_ZCL_ATTR_BASIC_STACK_VERSION_ID,
        ESP_ZB_ZCL_ATTR_TYPE_U16,
        ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY,
        (void *)&stack_version
    );
}


void ZigbeeSensor::createIdentifyCluster(esp_zb_cluster_list_t* cluster_list) {
    esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_identify_cluster_create(&identify_cfg), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}

void ZigbeeSensor::createOccupancyCluster(esp_zb_cluster_list_t* cluster_list) {
    esp_zb_attribute_list_t *occupancy_cluster = esp_zb_occupancy_sensing_cluster_create(&occupancy_meas_cfg);
    esp_zb_cluster_list_add_occupancy_sensing_cluster(cluster_list, occupancy_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    esp_zb_cluster_add_attr(
        occupancy_cluster,
        ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING,
        ESP_ZB_ZCL_ATTR_OCCUPANCY_SENSING_PIR_OCC_TO_UNOCC_DELAY_ID,
        ESP_ZB_ZCL_ATTR_TYPE_U16,
        ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE,
        &occupancyTimeoutSec
    );
}

void ZigbeeSensor::createOnOffCluster(esp_zb_cluster_list_t* cluster_list) {
    esp_zb_attribute_list_t *on_off_cluster = esp_zb_on_off_cluster_create(&on_off_cfg);
    esp_zb_cluster_list_add_on_off_cluster(cluster_list, on_off_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}

void ZigbeeSensor::createTimeCluster(esp_zb_cluster_list_t* cluster_list) {
    time_t utc_time = 0;
    int32_t gmt_offset = 0;

    esp_zb_attribute_list_t *time_cluster_server = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_TIME);
    esp_zb_time_cluster_add_attr(time_cluster_server, ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID, (void *)&gmt_offset);
    esp_zb_time_cluster_add_attr(time_cluster_server, ESP_ZB_ZCL_ATTR_TIME_TIME_ID, (void *)&utc_time);
    esp_zb_time_cluster_add_attr(time_cluster_server, ESP_ZB_ZCL_ATTR_TIME_TIME_STATUS_ID, (void *)&_time_status);

    esp_zb_attribute_list_t *time_cluster_client = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_TIME);
    esp_zb_cluster_list_add_time_cluster(_cluster_list, time_cluster_server, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_time_cluster(_cluster_list, time_cluster_client, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
}

void ZigbeeSensor::createOtaCluster(esp_zb_cluster_list_t* cluster_list) {
    esp_zb_attribute_list_t *ota_cluster = esp_zb_ota_cluster_create(&ota_cluster_cfg);

    esp_zb_zcl_ota_upgrade_client_variable_t variable_config = {};
    variable_config.timer_query = ESP_ZB_ZCL_OTA_UPGRADE_QUERY_TIMER_COUNT_DEF;
    variable_config.hw_version = 1;
    variable_config.max_data_size = 223;

    uint16_t ota_upgrade_server_addr = 0xffff;
    uint8_t ota_upgrade_server_ep = 0xff;

    esp_zb_ota_cluster_add_attr(ota_cluster, ESP_ZB_ZCL_ATTR_OTA_UPGRADE_CLIENT_DATA_ID, (void *)&variable_config);
    esp_zb_ota_cluster_add_attr(ota_cluster, ESP_ZB_ZCL_ATTR_OTA_UPGRADE_SERVER_ADDR_ID, (void *)&ota_upgrade_server_addr);
    esp_zb_ota_cluster_add_attr(ota_cluster, ESP_ZB_ZCL_ATTR_OTA_UPGRADE_SERVER_ENDPOINT_ID, (void *)&ota_upgrade_server_ep);

    esp_zb_cluster_list_add_ota_cluster(_cluster_list, ota_cluster, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
}

static void findOTAServer(esp_zb_zdp_status_t zdo_status, uint16_t addr, uint8_t endpoint, void *user_ctx) {
    if (zdo_status == ESP_ZB_ZDP_STATUS_SUCCESS) {
        esp_zb_ota_upgrade_client_query_interval_set(*((uint8_t *)user_ctx), OTA_UPGRADE_QUERY_INTERVAL);
        esp_zb_ota_upgrade_client_query_image_req(addr, endpoint);
        ESP_LOGI("FIND_OTA", "Query OTA upgrade from server endpoint: %d after %d seconds", endpoint, OTA_UPGRADE_QUERY_INTERVAL);
    } else {
        ESP_LOGW("FIND_OTA", "No OTA Server found");
    }
}

void ZigbeeSensor::requestOTA() {
    esp_zb_zdo_match_desc_req_param_t req;
    uint16_t cluster_list[] = {ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE};

    req.addr_of_interest = 0x0000;
    req.dst_nwk_addr = 0x0000;
    req.num_in_clusters = 1;
    req.num_out_clusters = 0;
    req.profile_id = ESP_ZB_AF_HA_PROFILE_ID;
    req.cluster_list = cluster_list;
    esp_zb_lock_acquire(portMAX_DELAY);
    if (esp_zb_bdb_dev_joined()) {
        esp_zb_zdo_match_cluster(&req, findOTAServer, &_endpoint);
    }
    esp_zb_lock_release();
}

esp_zb_cluster_list_t* ZigbeeSensor::createClusters() {
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();

    createBasicCluster(cluster_list);
    createIdentifyCluster(cluster_list);
    createOccupancyCluster(cluster_list);
    createOnOffCluster(cluster_list);
    createOtaCluster(cluster_list);
    createTimeCluster(cluster_list);

    return cluster_list;
}

void ZigbeeSensor::zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) {
    if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING && message->attribute.id == ESP_ZB_ZCL_ATTR_OCCUPANCY_SENSING_PIR_OCC_TO_UNOCC_DELAY_ID) {
        uint16_t newTimeout = *(uint16_t *)message->attribute.data.value;

        if (newTimeout < 5 || newTimeout > 3600) {
            ESP_LOGW(TAG, "Invalid occupancy timeout rejected");
            return;
        }

        occupancyTimeoutSec = newTimeout;
        /*prefs.begin(NVS_NAMESPACE, false);
        prefs.putUShort(NVS_OCC_TIMEOUT, newTimeout);
        prefs.end();*/

        ESP_LOGI(TAG, "Occupancy timeout updated: %u seconds\n", newTimeout);
    } else if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF && message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) {
        bool newState = *(bool *)message->attribute.data.value;
        _on_light_change(newState);
        ESP_LOGI(TAG, "On off updated: %s\n", newState ? "ON" : "OFF");
    }
}

void ZigbeeSensor::onLightChange(void (*callback)(bool)) {
    _on_light_change = callback;
}

uint16_t ZigbeeSensor::getTimeout() {
    return occupancyTimeoutSec;
}

void ZigbeeSensor::init() {
    /*prefs.begin(NVS_NAMESPACE, false);
    occupancyTimeoutSec = prefs.getUShort(NVS_OCC_TIMEOUT, 60);
    prefs.end();*/

    ESP_LOGI(TAG, "Occupancy timeout loaded: %u seconds\n", occupancyTimeoutSec);
}

ZigbeeSensor::ZigbeeSensor(uint8_t endpoint) : ZigbeeDevice(ESP_ZB_HA_SIMPLE_SENSOR_DEVICE_ID, endpoint) {    
    basic_cfg = {
        .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,
        .power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE
    };
    identify_cfg = {
        .identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE
    };
    occupancy_meas_cfg = {
        .occupancy = ESP_ZB_ZCL_OCCUPANCY_SENSING_OCCUPANCY_UNOCCUPIED,
        .sensor_type = ESP_ZB_ZCL_OCCUPANCY_SENSING_OCCUPANCY_SENSOR_TYPE_PIR,
        .sensor_type_bitmap = (1 << ESP_ZB_ZCL_OCCUPANCY_SENSING_OCCUPANCY_SENSOR_TYPE_PIR)
    };
    on_off_cfg = {
        .on_off = ESP_ZB_ZCL_ON_OFF_ON_OFF_DEFAULT_VALUE
    };
    ota_cluster_cfg = {
        .ota_upgrade_file_version = FW_VERSION,
        .ota_upgrade_manufacturer = 0x1001,
        .ota_upgrade_image_type = 0x1011,
        .ota_min_block_reque = 0,
        .ota_upgrade_file_offset = 0,
        .ota_upgrade_downloaded_file_ver = ESP_ZB_ZCL_OTA_UPGRADE_DOWNLOADED_FILE_VERSION_DEF_VALUE,
        .ota_upgrade_server_id = 0,
        .ota_image_upgrade_status = 0
    };
}

bool ZigbeeSensor::setOccupancy(bool occupied) {
    esp_zb_zcl_status_t ret = ESP_ZB_ZCL_STATUS_SUCCESS;

    esp_zb_lock_acquire(portMAX_DELAY);
    ret = esp_zb_zcl_set_attribute_val(
        _endpoint,
        ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_OCCUPANCY_SENSING_OCCUPANCY_ID,
        &occupied,
        false
    );
    esp_zb_lock_release();

    if (ret != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set occupancy: 0x%x: %s", ret, esp_zb_zcl_status_to_name(ret));
        return false;
    }
    return true;
}

bool ZigbeeSensor::setOnOff(bool onOff) {
    esp_zb_zcl_status_t ret = ESP_ZB_ZCL_STATUS_SUCCESS;

    esp_zb_lock_acquire(portMAX_DELAY);
    ret = esp_zb_zcl_set_attribute_val(
        _endpoint,
        ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
        &onOff,
        false
    );
    esp_zb_lock_release();

    if (ret != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set on off: 0x%x: %s", ret, esp_zb_zcl_status_to_name(ret));
        return false;
    }
    return true;
}

bool ZigbeeSensor::report() {
    esp_zb_zcl_report_attr_cmd_t occupy_report_attr_cmd = {
        {
            .dst_addr_u = 0,
            .dst_endpoint = 0,
            .src_endpoint = _endpoint
        },
        ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
        ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING,
        {0, ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI, 0},
        ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
        ESP_ZB_ZCL_ATTR_OCCUPANCY_SENSING_OCCUPANCY_ID
    };

    esp_zb_zcl_report_attr_cmd_t onoff_report_attr_cmd = {
        {
            .dst_addr_u = 0,
            .dst_endpoint = 0,
            .src_endpoint = _endpoint
        },
        ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
        ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
        {0, ESP_ZB_ZCL_CMD_DIRECTION_TO_CLI, 0},
        ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
        ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID
    };

    esp_zb_lock_acquire(portMAX_DELAY);
    esp_err_t ret = esp_zb_zcl_report_attr_cmd_req(&occupy_report_attr_cmd);
    esp_err_t ret2 = esp_zb_zcl_report_attr_cmd_req(&onoff_report_attr_cmd);
    esp_zb_lock_release();

    return ret == ESP_OK && ret2 == ESP_OK;
}