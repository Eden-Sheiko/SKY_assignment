#include "../Inc/Main_system.h"
#include "../Inc/dht11.h"
#include "../Inc/Config.h"
#include "../Inc/pid.h"

#define DEBUG

int update_lighting(double);

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe(event->client, "/topic/qos0", 0);
            esp_mqtt_client_publish(event->client, "/topic/qos0", "Hello from ESP32", 0, 1, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
}

static void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://test.mosquitto.org", // Update broker URI if needed
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}


static esp_adc_cal_characteristics_t adc1_chars;

PIDController myPID;
double temp = 0;               // Temperature (input)
double PIDOut = 0;             // PID output
double TempSetpoint = TEMP_TARGET;      // Desired temperature


void app_main(void)
{
    DHT11_init(PIN_NUMBER);

    myPID.Kp = 2.0;  // Proportional gain
    myPID.Ki = 0.01;  // Integral gain
    myPID.Kd = 1.0;  // Derivative gain

    myPID.tau = 0.02;  // Low-pass filter time constant (for derivative term)
    myPID.T = 1.0;     // Sample time in seconds

    // Output limits (e.g., PWM limits from 0 to 100)
    myPID.limMin = 0.0f;
    myPID.limMax = 100.0f;

    // Integrator limits (to prevent integrator wind-up)
    myPID.limMinInt = -10.0f;
    myPID.limMaxInt = 10.0f;

    // Initialize the PID controller
    PIDController_Init(&myPID);

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
    static ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,     // High speed mode
        .timer_num        = LEDC_TIMER_0,             // Timer 0
        .duty_resolution  = LEDC_TIMER_7_BIT,        // Set PWM to 7-bit resolution (0-8191)
        .freq_hz          = 5000,                     // Frequency of PWM signal (5kHz)
        .clk_cfg          = LEDC_AUTO_CLK             // Auto select clock source
    };
    ledc_timer_config(&ledc_timer);

    // Configure the PWM channel
    static ledc_channel_config_t ledc_channel = {
        .gpio_num   = PWM_GPIO,              // The GPIO pin (GPIO21)
        .speed_mode = LEDC_HIGH_SPEED_MODE, // High speed mode
        .channel    = LEDC_CHANNEL_0,       // Channel 0
        .timer_sel  = LEDC_TIMER_0,         // Use Timer 0
        .duty       = 0,                    // Initial duty cycle (0 = off)
        .hpoint     = 0
    };
    ledc_channel_config(&ledc_channel);



    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
    


    while(1){ 
   
    ESP_LOGI(TAG2, "temperature is %d", DHT11_read().temperature);
    ESP_LOGI(TAG2, "Status code is %d\n", DHT11_read().status);
    ESP_LOGI(TAG2, "Humidity is %d\n", DHT11_read().humidity);

    temp = DHT11_read().temperature;
    if (temp < 20){
        ESP_LOGI(TAG2, "Toggling BLUE LED ON\n");
        gpio_set_level(LED_GPIO_B, 1);
        gpio_set_level(LED_GPIO_G, 0);  
        gpio_set_level(LED_GPIO_R, 0);    
        vTaskDelay(10 / portTICK_PERIOD_MS);  
    }else if (temp >= 20 && temp <= 25 ){
        ESP_LOGI(TAG2, "Toggling Green LED on\n");
        gpio_set_level(LED_GPIO_G, 1);
        gpio_set_level(LED_GPIO_B, 0);
        gpio_set_level(LED_GPIO_R, 0); 
        vTaskDelay(10 / portTICK_PERIOD_MS);  
    }else{
        ESP_LOGI(TAG2, "Toggling RED LED ON\n");
        gpio_set_level(LED_GPIO_R, 1);
        gpio_set_level(LED_GPIO_B, 0);  
        gpio_set_level(LED_GPIO_G, 0);  
        vTaskDelay(10 / portTICK_PERIOD_MS);  
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);    

    int adc_value = adc1_get_raw(ADC1_CHANNEL_4);
    ESP_LOGI(TAG2, "ADC Value: %d" ,adc_value);
    vTaskDelay(500/ portTICK_PERIOD_MS);


    PIDOut = PIDController_Update(&myPID, TempSetpoint, temp);
    ESP_LOGI(TAG2, "Temperature: %.2f, PID Output: %.2f\n", temp, PIDOut);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, PIDOut);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    

}


