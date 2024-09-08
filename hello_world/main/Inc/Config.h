#ifndef CONFIG_H
#define CONFIG_H

#define PIN_NUMBER GPIO_NUM_26

#define LED_GPIO_R GPIO_NUM_12
#define LED_GPIO_B GPIO_NUM_13
#define LED_GPIO_G GPIO_NUM_14

#define PWM_GPIO   GPIO_NUM_21
#define PWM_HZ     5000

#define BUFF_SIZE 10

#define MQTT_TOPIC_TEMP      "/home/sensors/temperature"
#define MQTT_TOPIC_HUMIDITY  "/home/sensors/humidity"
#define MQTT_TOPIC_LIGHT     "/home/sensors/light"

#define MQTT_TOPIC_LIGHT_COMMAND "/home/commands/light"


static const char* const TAG = "MQTT";
static const char* const TAG2 = "GreenHouse";

#define TURN_ON 1
#define TURN_OFF 0

#define TEMP_LOW_THRESHOLD 20
#define TEMP_HIGH_THRESHOLD 25

#define TEMP_TARGET 22.5

#define ESP_OK 0
#define ESP_ERROR 1

#endif /* CONFIG_H */
