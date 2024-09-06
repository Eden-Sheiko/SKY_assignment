#include "../Inc/Main_system.h"
#include "../Inc/dht11.h"
#include "../Inc/Config.h"
#include "driver/gpio.h"

#define LED_GPIO_R GPIO_NUM_12
#define LED_GPIO_B GPIO_NUM_13
#define LED_GPIO_G GPIO_NUM_14


static esp_adc_cal_characteristics_t adc1_chars;


void app_main(void)
{
    DHT11_init(PIN_NUMBER);

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);


    gpio_reset_pin(LED_GPIO_G);
    gpio_set_direction(LED_GPIO_G, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_GPIO_B);
    gpio_set_direction(LED_GPIO_B, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_GPIO_R);
    gpio_set_direction(LED_GPIO_R, GPIO_MODE_OUTPUT);


    

    
    while(1){ 
    printf("Temperature is %d \n", DHT11_read().temperature);
    printf("Humidity is %d\n", DHT11_read().humidity);
    printf("Status code is %d\n", DHT11_read().status);
    vTaskDelay(2000 / portTICK_PERIOD_MS);    

    int adc_value = adc1_get_raw(ADC1_CHANNEL_4);
    printf("ADC Value: %d", adc_value);
    printf("\n");
    vTaskDelay(500/ portTICK_PERIOD_MS);

    // Toggle the Green LED
    printf("Toggling Green LED ON\n");
    gpio_set_level(LED_GPIO_G, 1);  // Turn on
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 300ms

    printf("Toggling Green LED OFF\n");
    gpio_set_level(LED_GPIO_G, 0);  // Turn off
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 300ms

    printf("Toggling BLUE LED OFF\n");
    gpio_set_level(LED_GPIO_B, 1);  // Turn on
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 300ms

    printf("Toggling BLUE LED OFF\n");
    gpio_set_level(LED_GPIO_B, 0);  // Turn off
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 300ms


    printf("Toggling RED LED OFF\n");
    gpio_set_level(LED_GPIO_R, 1);  // Turn on
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 300ms

    printf("Toggling RED LED OFF\n");
    gpio_set_level(LED_GPIO_R, 0);  // Turn off
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait for 300ms
    }


    //return 0;
}
