#define DO_EXPAND(VAL)  VAL ## 1
#define EXPAND(VAL)     DO_EXPAND(VAL)
#define CI              (EXPAND(FW_VERSION) != 1)

#if !defined(FW_VERSION) || !CI
#undef FW_VERSION
#define FW_VERSION 0x00000001
#endif

#if !defined(DATE_CODE) || !CI
#undef DATE_CODE
#define DATE_CODE "19700101"
#endif

#if !defined(SW_VERSION) || !CI
#undef SW_VERSION
#define SW_VERSION "0.0.0"
#endif

#define OCCUPANCY_SENSOR_ENDPOINT_NUMBER 10
#define HEARTBEAT_INTERVAL 60000000
#define HOLDOUT_US 10000000

#define WS2812_MAX 500

#define BUTTON_PIN  GPIO_NUM_9
#define SWITCH_PIN  GPIO_NUM_20
#define SENSOR_PIN  GPIO_NUM_18
#define LEDA_PIN    GPIO_NUM_23
#define LEDB_PIN    GPIO_NUM_22
#define LEDC_PIN    GPIO_NUM_21
#define WS2812_PIN  GPIO_NUM_1
#define TEMP_PIN    GPIO_NUM_2
#define SCL_PIN     GPIO_NUM_4
#define SDA_PIN     GPIO_NUM_5
