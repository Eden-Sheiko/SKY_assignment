/**
 * @file wifi.h
 * @brief Header file for WiFi station initialization and management.
 * 
 * This file contains the necessary includes, macros, and function 
 * declarations for setting up a WiFi station using ESP32. The WiFi 
 * station will connect to a predefined SSID and handle connection 
 * retries.
 * 
 * code was taken form esp32 offical github and ajusted 
 * https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c
 */

#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

/**
 * @brief The SSID of the WiFi network.
 */
#define EXAMPLE_ESP_WIFI_SSID      "Sheiko"

/**
 * @brief The password for the WiFi network.
 */
#define EXAMPLE_ESP_WIFI_PASS      "10203070"

/**
 * @brief Maximum number of connection retries before failure.
 */
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

/**
 * @brief Event bit for WiFi connected state.
 */
#define WIFI_CONNECTED_BIT BIT0

/**
 * @brief Event bit for WiFi failure state.
 */
#define WIFI_FAIL_BIT      BIT1

/**
 * @brief Initializes WiFi in station mode.
 * 
 * This function configures the ESP32 as a WiFi station and connects to the 
 * specified SSID. It handles reconnection attempts and raises events for 
 * connection success or failure.
 */
void wifi_init_sta(void);

#endif // WIFI_H
