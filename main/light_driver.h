#include "led_strip.h"
#include "led_enum.h"

void lightTask(void *pvParameters);

class LightDriver {
    public:
        void init();
        void setLevels(uint8_t amber, uint8_t warm_white, uint8_t cool_white);
        void setCount(uint16_t count);
        void setSpeed(uint8_t speed);
        void setPowerTarget(uint8_t target);
        void setAnimation(FadeAnimation animation);
        void setOccupancyState(bool state);
        void task();
    private:
        float speed = 3;
        uint8_t powerTarget = 0;
        uint16_t driverLedCount = 1;
        FadeAnimation animation = FadeAnimation::BASIC;
        uint8_t fadeWidth = 10;
        uint8_t sparkleSteps = 24;

        led_strip_handle_t s_led_strip;
        float s_warm = 1, s_cold = 1, s_amber = 1;

        void setPower(uint8_t power);
        uint8_t calculateBasic(uint8_t power, uint16_t i);
        uint8_t calculateRows(uint8_t power, uint16_t i);
        uint8_t calculateFromEnds(uint8_t power, uint16_t i);
        uint8_t calculateFromCenter(uint8_t power, uint16_t i);
        uint8_t calculateSparkle(uint8_t power, uint16_t i);
        uint8_t calculateFromLeft(uint8_t power, uint16_t i);
        uint8_t calculateFromRight(uint8_t power, uint16_t i);

        static constexpr uint8_t (LightDriver::*animations[])(uint8_t, uint16_t) = {
            &LightDriver::calculateBasic,
            &LightDriver::calculateRows,
            &LightDriver::calculateFromEnds,
            &LightDriver::calculateFromCenter,
            &LightDriver::calculateSparkle,
            &LightDriver::calculateFromLeft,
            &LightDriver::calculateFromRight
        };
};

extern LightDriver ledDriver;
