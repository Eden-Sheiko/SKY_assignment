#include "../Inc/Main_system.h"
#include "../Inc/dht11.h"
#include "../Inc/Config.h"
#include "../Inc/pid.h"
#include "../Inc/Wifi.h"
#include "../Inc/MQTT.h"

/*to add in the furture*/
//#define DEBUG
//#define NDEBUG

static esp_adc_cal_characteristics_t adc1_chars;
PIDController pid;
double temp = 0;               
double pid_output = 0;         
double TempSetpoint = TEMP_TARGET;  

void init_nvs(void);
void init_wifi(void);
void init_mqtt(void);
void init_pid(void);
void init_gpio(void);
void init_adc(void);

void app_main(void)
{
    /* include build in Error check + checksum */
    DHT11_init(PIN_NUMBER);
    
    /* inits Hardware + Software */
    init_nvs();
    init_wifi();
    init_mqtt();
    init_pid();
    init_gpio();
    init_adc();

    /* super loop */
    while (1) {
        ESP_LOGI(TAG2, "Temperature: %d", DHT11_read().temperature);
        ESP_LOGI(TAG2, "Status code: %d", DHT11_read().status);
        ESP_LOGI(TAG2, "Humidity: %d", DHT11_read().humidity);

        temp = DHT11_read().temperature;

        if (temp < TEMP_LOW_THRESHOLD) {
            ESP_LOGI(TAG2, "Toggling BLUE LED ON");
            gpio_set_level(LED_GPIO_B, TURN_ON);
            gpio_set_level(LED_GPIO_G, TURN_OFF);
            gpio_set_level(LED_GPIO_R, TURN_OFF);
        } else if (temp >= TEMP_LOW_THRESHOLD && temp <= TEMP_HIGH_THRESHOLD) {
            ESP_LOGI(TAG2, "Toggling GREEN LED ON");
            gpio_set_level(LED_GPIO_G, TURN_ON);
            gpio_set_level(LED_GPIO_B, TURN_OFF);
            gpio_set_level(LED_GPIO_R, TURN_OFF);
        } else {
            ESP_LOGI(TAG2, "Toggling RED LED ON");
            gpio_set_level(LED_GPIO_R, TURN_ON);
            gpio_set_level(LED_GPIO_B, TURN_OFF);
            gpio_set_level(LED_GPIO_G, TURN_OFF);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);

        int adc_value = adc1_get_raw(ADC1_CHANNEL_4);
        ESP_LOGI(TAG2, "ADC Value: %d", adc_value);
        vTaskDelay(500 / portTICK_PERIOD_MS);

        char temp_str[BUFF_SIZE], hum_str[BUFF_SIZE], light_str[BUFF_SIZE];
        snprintf(temp_str, sizeof(temp_str), "%d", DHT11_read().temperature);
        snprintf(hum_str, sizeof(hum_str), "%d", DHT11_read().humidity);
        snprintf(light_str, sizeof(light_str), "%d", adc_value);  

        esp_mqtt_client_publish(client, MQTT_TOPIC_TEMP, temp_str, 0, 1, 0);
        esp_mqtt_client_publish(client, MQTT_TOPIC_HUMIDITY, hum_str, 0, 1, 0);
        esp_mqtt_client_publish(client, MQTT_TOPIC_LIGHT, light_str, 0, 1, 0);

        pid_output = PIDController_Update(&pid, TempSetpoint, temp);
        ESP_LOGI(TAG2, "Temperature: %.2f, PID Output: %.2f", temp, pid_output);

        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, pid_output);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void init_wifi(void)
{
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}

void init_mqtt(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    mqtt_app_start();
}

void init_pid(void)
{
    pid.Kp = 2.0;
    pid.Ki = 0.01;
    pid.Kd = 1.0;
    pid.tau = 0.02;
    pid.T = 1.0;

    pid.limMin = 0.0f;
    pid.limMax = 100.0f;
    pid.limMinInt = -10.0f;
    pid.limMaxInt = 10.0f;

    PIDController_Init(&pid);
}

void init_gpio(void)
{
    gpio_reset_pin(PWM_GPIO);
    gpio_set_direction(PWM_GPIO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_GPIO_G);
    gpio_set_direction(LED_GPIO_G, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_GPIO_B);
    gpio_set_direction(LED_GPIO_B, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_GPIO_R);
    gpio_set_direction(LED_GPIO_R, GPIO_MODE_OUTPUT);
}

void init_adc(void)
{
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);

    static ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_7_BIT,
        .freq_hz          = PWM_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    static ledc_channel_config_t ledc_channel = {
        .gpio_num   = PWM_GPIO,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&ledc_channel);
}
