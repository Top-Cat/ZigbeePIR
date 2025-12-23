#include "esp_zigbee_core.h"
#include "light_driver.h"

#define HA_ESP_LIGHT_ENDPOINT           10

static const char *TAG = "TC-ZB";
static char manufacturer_name[] = "\x02""TC";
static char model_identifier[] = "\x12""Kitchen PIR Sensor";

volatile bool occupancy_changed = false;
volatile bool button_pressed = false;
volatile bool switch_pressed = false;
bool occupancy_state = false;
bool zigbeeConnected = false;

uint64_t lastHeartbeat = 0;
uint64_t lastMotionUs = 0;
uint64_t holdOutUs = 0;
