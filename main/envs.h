#include "config.h"
#include "ds18b20.h"
#include "onewire_bus_impl_rmt.h"
#include "tsl2591.h"

#define LUX_HYSTERESIS 50
#define LUX_SAMPLES 5

class Envs {
    public:
        void setup();
        void task();

        bool getInhibit();
        void setInhibit(float _threshold);
    private:
        const char *TAG = "TC-ENV";
        float readings[LUX_SAMPLES];
        uint8_t luxIdx = 0;

        tsl2591_t light;
        bool lightFound = false;
        float threshold = 0;
        float lux = 0;
        bool inhibit = false;

        onewire_bus_handle_t bus = NULL;
        ds18b20_device_handle_t temp[2];
        uint8_t sensorCount = 0;

        void setupLight();
        void setupTemp();

        void handleTemperature();
        void handleLight();
        void checkLux();
};

extern Envs envs;

void sensor_task(void *pvParameters);
