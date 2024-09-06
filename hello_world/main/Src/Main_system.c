#include "../Inc/Main_system.h"
#include "../Inc/dht11.h"
#include "../Inc/Config.h"

static esp_adc_cal_characteristics_t adc1_chars;


void app_main(void)
{
    DHT11_init(PIN_NUMBER);

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);

    gpio_reset_pin(PWM_GPIO);
    gpio_set_direction(PWM_GPIO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_GPIO_G);
    gpio_set_direction(LED_GPIO_G, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_GPIO_B);
    gpio_set_direction(LED_GPIO_B, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_GPIO_R);
    gpio_set_direction(LED_GPIO_R, GPIO_MODE_OUTPUT);

    // Configure the PWM timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,     // High speed mode
        .timer_num        = LEDC_TIMER_0,             // Timer 0
        .duty_resolution  = LEDC_TIMER_13_BIT,        // Set PWM to 13-bit resolution (0-8191)
        .freq_hz          = 5000,                     // Frequency of PWM signal (5kHz)
        .clk_cfg          = LEDC_AUTO_CLK             // Auto select clock source
    };
    ledc_timer_config(&ledc_timer);

    // Configure the PWM channel
    ledc_channel_config_t ledc_channel = {
        .gpio_num   = PWM_GPIO,              // The GPIO pin (GPIO21)
        .speed_mode = LEDC_HIGH_SPEED_MODE, // High speed mode
        .channel    = LEDC_CHANNEL_0,       // Channel 0
        .timer_sel  = LEDC_TIMER_0,         // Use Timer 0
        .duty       = 0,                    // Initial duty cycle (0 = off)
        .hpoint     = 0
    };
    ledc_channel_config(&ledc_channel);



    
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


        for (int duty = 0; duty <= 8191; duty += 256) {
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(30 / portTICK_PERIOD_MS);
        }


            // Gradually decrease brightness (duty cycle from 8191 to 0)
        for (int duty = 8191; duty >= 0; duty -= 256) {
            ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
            vTaskDelay(30 / portTICK_PERIOD_MS);
        }
    }



}
