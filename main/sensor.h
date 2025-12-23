#include "esp_zigbee_type.h"

#include "zigbee/endpoint.h"

#define OTA_UPGRADE_QUERY_INTERVAL (1 * 60)

class ZigbeeSensor : public ZigbeeDevice {
    public:
        ZigbeeSensor(uint8_t endpoint);
        ~ZigbeeSensor() {}

        void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) override;

        void onLightChange(void (*callback)(bool));
        bool setOccupancy(bool occupied);
        bool setOnOff(bool onOff);
        void init();
        uint16_t getTimeout();

        void requestOTA();
        bool report();
    private:
        const char *TAG = "TC-ZBS";

        esp_zb_basic_cluster_cfg_t basic_cfg;
        esp_zb_identify_cluster_cfg_t identify_cfg;
        esp_zb_occupancy_sensing_cluster_cfg_t occupancy_meas_cfg;
        esp_zb_on_off_cluster_cfg_t on_off_cfg;
        esp_zb_ota_cluster_cfg_t ota_cluster_cfg;

        esp_zb_cluster_list_t* createClusters() override;
        void createBasicCluster(esp_zb_cluster_list_t* cluster_list);
        void createIdentifyCluster(esp_zb_cluster_list_t* cluster_list);
        void createOccupancyCluster(esp_zb_cluster_list_t* cluster_list);
        void createOnOffCluster(esp_zb_cluster_list_t* cluster_list);
        void createOtaCluster(esp_zb_cluster_list_t* cluster_list);
        void createTimeCluster(esp_zb_cluster_list_t* cluster_list);

        void (*_on_light_change)(bool);
};
