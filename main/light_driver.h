#include "led_strip.h"

void lightTask(void *pvParameters);
void setLevels(uint8_t amber, uint8_t warm_white, uint8_t cool_white, uint16_t count);

class LightDriver {
    public:
        void init();
        void setLevels(uint8_t amber, uint8_t warm_white, uint8_t cool_white, uint16_t count);
        void setPowerTarget(uint8_t target);
        void task();
    private:
        uint8_t powerTarget = 0;
        uint16_t driverLedCount = 1;

        led_strip_handle_t s_led_strip;
        float s_warm = 1, s_cold = 1, s_amber = 1;

        void setPower(uint8_t power);
};

extern LightDriver ledDriver;
