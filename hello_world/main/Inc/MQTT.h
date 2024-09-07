/**
 * @file mqtt.h
 * @brief Header file for MQTT client initialization and configuration.
 * 
 * This file provides necessary includes, definitions, and function 
 * declarations for setting up and managing an MQTT client using ESP32. 
 * It defines a topic for sending light commands and starts the MQTT 
 * client.
 * 
 * code was taken form esp32 offical github and ajusted 
 */

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
#include "driver/ledc.h"       


/**
 * @brief MQTT client handle for managing MQTT connections and messages.
 */
extern esp_mqtt_client_handle_t client;

/**
 * @brief MQTT topic for sending light control commands.
 */
#define MQTT_TOPIC_LIGHT_COMMAND "/home/commands/light"

/**
 * @brief MQTT topic for sending temp control commands.
 */
#define MQTT_TOPIC_TEMP "/home/sensors/temperature"

/**
 * @brief MQTT topic for sending humidity control commands.
 */
#define MQTT_TOPIC_HUMIDITY "/home/sensors/humidity"


/**
 * @brief Initializes and starts the MQTT client.
 * 
 * This function configures the MQTT client, connects to the broker, 
 * and sets up event handlers for handling MQTT messages and connection
 * events.
 */
void mqtt_app_start(void);

#endif // MQTT_H
