/**
 * @file main_system.h
 * @brief Main system header file for ESP32 project.
 * 
 * This file includes necessary headers for ESP32 system initialization, WiFi setup, 
 * MQTT client configuration, and FreeRTOS task management.
 */

#ifndef MAIN_SYSTEM_H
#define MAIN_SYSTEM_H

#include <stdio.h>              
#include <inttypes.h>           
#include <string.h>             
#include <stdlib.h>             
#include <stdint.h>            
#include "sdkconfig.h"          
#include "esp_system.h"         
#include "nvs_flash.h"          
#include "esp_wifi.h"          
#include "esp_event.h"          
#include "esp_netif.h"          
#include "esp_mac.h"            
#include "esp_log.h"           
#include "esp_chip_info.h"     
#include "esp_flash.h"          
#include "driver/adc.h"         
#include "esp_adc_cal.h"        
#include "driver/gpio.h"       
#include "driver/ledc.h"        
#include "mqtt_client.h"        
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"      
#include "freertos/semphr.h"    
#include "freertos/queue.h"     
#include "lwip/sockets.h"       
#include "lwip/dns.h"           
#include "lwip/netdb.h"         
#include "lwip/err.h"           
#include "lwip/sys.h"           

/**
 * @brief Main application entry point.
 * 
 * This function is called when the system starts and initializes 
 * all required components (WiFi, MQTT, tasks, etc.).
 */
void app_main(void);

#endif /* MAIN_SYSTEM_H */
