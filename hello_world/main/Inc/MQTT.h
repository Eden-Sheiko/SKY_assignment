#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"

extern esp_mqtt_client_handle_t client;

#define MQTT_TOPIC_LIGHT_COMMAND "/home/commands/light"

void mqtt_app_start(void);

#endif // MQTT_H
