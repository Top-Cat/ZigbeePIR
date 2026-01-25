#include "esp_zigbee_type.h"

#include "zigbee/endpoint.h"
#include "prefs.h"
#include "led_enum.h"

#define MANUFACTURER_CODE        0x1234

#define MS_LED_CLUSTER_ID        0xFC10
#define ATTR_AMBER_LEVEL_ID      0x0001
#define ATTR_WARM_WHITE_LEVEL_ID 0x0002
#define ATTR_COOL_WHITE_LEVEL_ID 0x0003
#define ATTR_LED_COUNT_ID        0x0010
#define ATTR_ANIMATION_ID        0x0011
#define ATTR_SPEED_ID            0x0012

#define MS_LUX_CLUSTER_ID         0xFC11
#define ATTR_INHIBIT_THRESHOLD_ID 0x0001

#define OTA_UPGRADE_QUERY_INTERVAL (1 * 60)
#define NVS_NAMESPACE         "config"
#define NVS_OCC_TIMEOUT       "occ_timeout"
#define NVS_MAN_TIMEOUT       "man_timeout"
#define NVS_AMBER             "amber"
#define NVS_WARM_WHITE        "warm"
#define NVS_COOL_WHITE        "cool"
#define NVS_LED_COUNT         "count"
#define NVS_ANIMATION         "anim"
#define NVS_SPEED             "speed"
#define NVS_INHIBIT_THRESHOLD "inhibit"

class ZigbeeSensor : public ZigbeeDevice {
    public:
        ZigbeeSensor(uint8_t endpoint);
        ~ZigbeeSensor() {}

        void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) override;

        void onLightChange(void (*callback)(bool));
        bool setOccupancy(bool occupied);
        bool setTemperature(float temperature);
        bool setIlluminance(float illuminance);
        bool setOnOff(bool onOff);
        void init();
        uint16_t getTimeout();
        uint16_t getManualHoldout();

        void onConnect();
        void requestOTA();
        bool report(bool occupancy);
    private:
        const char* TAG = "TC-ZBS";
        const char* manufacturer_name = "TC";
        const char* model_identifier = "Kitchen PIR Sensor";
        bool reportTemperature = false;
        bool reportIlluminance = false;

        uint16_t occupancyTimeoutSec = 60;
        uint16_t manualTimeoutSec    = 60;
        uint8_t amberLevel           = 0;
        uint8_t warmWhiteLevel       = 0;
        uint8_t coolWhiteLevel       = 0;
        uint16_t ledCount            = 1;
        uint8_t animation            = 0;
        uint8_t speed                = 0;
        uint16_t inhibitThreshold    = 1;

        Preferences prefs;

        esp_zb_basic_cluster_cfg_t basic_cfg;
        esp_zb_identify_cluster_cfg_t identify_cfg;
        esp_zb_occupancy_sensing_cluster_cfg_t occupancy_meas_cfg;
        esp_zb_on_off_cluster_cfg_t on_off_cfg;
        esp_zb_ota_cluster_cfg_t ota_cluster_cfg;
        esp_zb_temperature_meas_cluster_cfg_t temperature_cfg;
        esp_zb_illuminance_meas_cluster_cfg_t lux_cfg;

        esp_zb_cluster_list_t* createClusters() override;
        void createBasicCluster(esp_zb_cluster_list_t* cluster_list);
        void createIdentifyCluster(esp_zb_cluster_list_t* cluster_list);
        void createOccupancyCluster(esp_zb_cluster_list_t* cluster_list);
        void createOnOffCluster(esp_zb_cluster_list_t* cluster_list);
        void createOtaCluster(esp_zb_cluster_list_t* cluster_list);
        void createTimeCluster(esp_zb_cluster_list_t* cluster_list);
        void createTemperatureCluster(esp_zb_cluster_list_t* cluster_list);
        void createIlluminanceCluster(esp_zb_cluster_list_t* cluster_list);
        void createCustomClusters(esp_zb_cluster_list_t* cluster_list);

        void (*_on_light_change)(bool);
};
