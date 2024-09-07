/**
 * @file main_system.h
 * @brief Main system header file for ESP32 project.
 * 
 * This file includes necessary headers for ESP32 system initialization, WiFi setup, 
 * MQTT client configuration, and FreeRTOS task management.
 */

#ifndef MAIN_SYSTEM_H
#define MAIN_SYSTEM_H

#include <stdio.h>              // For basic printf commands
#include <inttypes.h>           // For fixed-width integer types
#include <string.h>             // For string handling
#include <stdlib.h>             // For memory management functions
#include <stdint.h>             // For standard integer types
#include "sdkconfig.h"          // SDK configuration
#include "esp_system.h"         // ESP system functions (esp_init, esp_err_t)
#include "nvs_flash.h"          // Non-volatile storage functions
#include "esp_wifi.h"           // WiFi functions (esp_wifi_init)
#include "esp_event.h"          // Event handling (WiFi and system events)
#include "esp_netif.h"          // Network interface initialization
#include "esp_mac.h"            // MAC address management
#include "esp_log.h"            // Logging functions (ESP_LOG)
#include "esp_chip_info.h"      // Chip information
#include "esp_flash.h"          // Flash memory functions
#include "driver/adc.h"         // ADC driver for reading analog inputs
#include "esp_adc_cal.h"        // ADC calibration
#include "driver/gpio.h"        // GPIO control
#include "driver/ledc.h"        // LED control (LEDC)
#include "mqtt_client.h"        // MQTT client setup and communication
#include "freertos/FreeRTOS.h"  // FreeRTOS task management, delays, mutexes, and semaphores
#include "freertos/task.h"      // Task creation and management
#include "freertos/semphr.h"    // Semaphore functions
#include "freertos/queue.h"     // Queue functions for inter-task communication
#include "lwip/sockets.h"       // Lightweight IP socket functions
#include "lwip/dns.h"           // DNS handling
#include "lwip/netdb.h"         // Network database functions
#include "lwip/err.h"           // Lightweight IP error handling
#include "lwip/sys.h"           // System functions for lightweight IP applications

/**
 * @brief Main application entry point.
 * 
 * This function is called when the system starts and initializes 
 * all required components (WiFi, MQTT, tasks, etc.).
 */
void app_main(void);

#endif /* MAIN_SYSTEM_H */
