#ifndef CONFIG_H
#define CONFIG_H


#define PIN_NUMBER GPIO_NUM_26

#define LED_GPIO_R GPIO_NUM_12
#define LED_GPIO_B GPIO_NUM_13
#define LED_GPIO_G GPIO_NUM_14

#define PWM_GPIO   GPIO_NUM_21


static const char* const TAG = "MQTT";

static const char* const TAG2 = "GreenHouse";
#define  EXAMPLE_ESP_WIFI_SSID "Sheiko"
#define  EXAMPLE_ESP_WIFI_PASS "10203070"

#define TEMP_TARGET 22.5

#endif /* CONFIG_H */
