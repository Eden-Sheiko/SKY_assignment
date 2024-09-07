#include "../Inc/Main_system.h"
#include "../Inc/dht11.h"
#include "../Inc/Config.h"
#include "../Inc/pid.h"

//#define DEBUG
//#define NDEBUG

#define MQTT_TOPIC_LIGHT_COMMAND "/home/commands/light"


#define EXAMPLE_ESP_WIFI_SSID      "Sheiko"
#define EXAMPLE_ESP_WIFI_PASS      "10203070"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTHmqtt
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

static esp_mqtt_client_handle_t client;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

//static const char *TAG = "wifi station";

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC_LIGHT_COMMAND, 0);
        ESP_LOGI(TAG, "Subscribed to light command, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
         ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);

    if (strncmp(event->topic, MQTT_TOPIC_LIGHT_COMMAND, event->topic_len) == 0) {
        int light_level = atoi(event->data);
        ESP_LOGI(TAG, "Received light command: %d", light_level);
        
        // Adjust lighting based on the received value
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, light_level);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://test.mosquitto.org",
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}


static esp_adc_cal_characteristics_t adc1_chars;

PIDController pid;
double temp = 0;               // Temperature (input)
double pid_output = 0;             // PID output
double TempSetpoint = TEMP_TARGET;      // Desired temperature


void app_main(void)
{
    DHT11_init(PIN_NUMBER);
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    


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

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    //ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    mqtt_app_start();

    pid.Kp = 2.0;  // Proportional gain
    pid.Ki = 0.01;  // Integral gain
    pid.Kd = 1.0;  // Derivative gain

    pid.tau = 0.02;  // Low-pass filter time constant (for derivative term)
    pid.T = 1.0;     // Sample time in seconds

    // Output limits (e.g., PWM limits from 0 to 100)
    pid.limMin = 0.0f;
    pid.limMax = 100.0f;

    // Integrator limits (to prevent integrator wind-up)
    pid.limMinInt = -10.0f;
    pid.limMaxInt = 10.0f;

    // Initialize the PID controller
    PIDController_Init(&pid);

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

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
        .freq_hz          = PWM_HZ,                   // Frequency of PWM signal (5kHz)
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



    while(1){ 
   
    ESP_LOGI(TAG2, "temperature is %d", DHT11_read().temperature);
    ESP_LOGI(TAG2, "Status code is %d\n", DHT11_read().status);
    ESP_LOGI(TAG2, "Humidity is %d\n", DHT11_read().humidity);

    temp = DHT11_read().temperature;
    if (temp < TEMP_LOW_THRESHOLD){

        ESP_LOGI(TAG2, "Toggling BLUE LED ON\n");
        gpio_set_level(LED_GPIO_B, TURN_ON);
        gpio_set_level(LED_GPIO_G, TURN_OFF);  
        gpio_set_level(LED_GPIO_R, TURN_OFF);    
        vTaskDelay(10 / portTICK_PERIOD_MS);  

    }else if (temp >= TEMP_LOW_THRESHOLD && temp <= TEMP_HIGH_THRESHOLD ){
        ESP_LOGI(TAG2, "Toggling Green LED on\n");
        gpio_set_level(LED_GPIO_G, TURN_ON);
        gpio_set_level(LED_GPIO_B, TURN_OFF);
        gpio_set_level(LED_GPIO_R, TURN_OFF); 
        vTaskDelay(10 / portTICK_PERIOD_MS);  

    }else{

        ESP_LOGI(TAG2, "Toggling RED LED ON\n");
        gpio_set_level(LED_GPIO_R, TURN_ON);
        gpio_set_level(LED_GPIO_B, TURN_OFF);  
        gpio_set_level(LED_GPIO_G, TURN_OFF);  
        vTaskDelay(10 / portTICK_PERIOD_MS);  

    }


    vTaskDelay(500 / portTICK_PERIOD_MS);    

    int adc_value = adc1_get_raw(ADC1_CHANNEL_4);
    ESP_LOGI(TAG2, "ADC Value: %d" ,adc_value);
    vTaskDelay(500/ portTICK_PERIOD_MS);

    char temp_str[10], hum_str[10], light_str[10];

    snprintf(temp_str, sizeof(temp_str), "%d", DHT11_read().temperature);
    snprintf(hum_str, sizeof(hum_str), "%d", DHT11_read().humidity);
    snprintf(light_str, sizeof(light_str), "%d", adc_value);  // Assuming ADC value represents light intensity

    // Publish temperature data
    esp_mqtt_client_publish(client, MQTT_TOPIC_TEMP, temp_str, 0, 1, 0);
    // Publish humidity data
    esp_mqtt_client_publish(client, MQTT_TOPIC_HUMIDITY, hum_str, 0, 1, 0);
    // Publish light data
    esp_mqtt_client_publish(client, MQTT_TOPIC_LIGHT, light_str, 0, 1, 0);
    

    pid_output = PIDController_Update(&pid, TempSetpoint, temp);
    ESP_LOGI(TAG2, "Temperature: %.2f, PID Output: %.2f\n", temp, pid_output);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, pid_output);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    
}


