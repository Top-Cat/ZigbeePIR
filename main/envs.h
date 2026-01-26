#include "config.h"
#include "ds18b20.h"
#include "onewire_bus_impl_rmt.h"
#include "tsl2591.h"

#define LUX_HYSTERESIS 50
#define LUX_SAMPLES 10

bool getInhibit();
void setInhibit(float _threshold);

void sensor_task(void *pvParameters);
