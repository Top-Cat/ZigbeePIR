#include "esp_zigbee_core.h"
#include "light_driver.h"
#include "ds18b20.h"
#include "onewire_bus_impl_rmt.h"

#define HA_ESP_LIGHT_ENDPOINT           10

static const char *TAG = "TC-ZB";

volatile bool occupancy_changed = false;
volatile bool button_pressed = false;
volatile bool switch_pressed = false;
bool occupancy_state = false;

uint64_t lastHeartbeat = 0;
uint64_t lastMotionUs = 0;
uint64_t holdOutUs = 0;

onewire_bus_handle_t bus = NULL;
ds18b20_device_handle_t temp[2];
uint8_t sensorCount = 0;
