#include "app_mqtt.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "driver/gpio.h"
#include <string.h>

#define LED_GPIO 4

static const char *TAG = "MQTT";

// Prevent multiple task creation
static bool task_started = false;


// =========================
// MQTT Publish Task (JSON)
// =========================
void mqtt_publish_task(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t) pvParameters;

    while (1)
    {
        cJSON *root = cJSON_CreateObject();

        cJSON_AddStringToObject(root, "device", "ESP32S3");

        int led_state = gpio_get_level(LED_GPIO);
        cJSON_AddNumberToObject(root, "status", led_state);

        char *json_data = cJSON_PrintUnformatted(root);

        esp_mqtt_client_publish(client, "iot/device/data", json_data, 0, 1, 0);

        ESP_LOGI(TAG, "Published: %s", json_data);

        cJSON_Delete(root);
        free(json_data);

        vTaskDelay(pdMS_TO_TICKS(5000)); // 5 sec delay
    }
}


// =========================
// MQTT Event Handler
// =========================
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected");

        // Subscribe to control topic
        esp_mqtt_client_subscribe(client, "iot/device/control", 0);

        // Start publish task only once
        if (!task_started)
        {
            xTaskCreate(mqtt_publish_task, "mqtt_pub_task", 4096, client, 5, NULL);
            task_started = true;
        }
        break;

    case MQTT_EVENT_DATA:
{
    ESP_LOGI(TAG, "Received Data: %.*s", event->data_len, event->data);

    cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);

    if (root == NULL)
    {
        ESP_LOGE(TAG, "JSON parsing failed");
        break;
    }

    // ✅ Get status instead of command
    cJSON *status = cJSON_GetObjectItem(root, "status");

    if (cJSON_IsNumber(status))
    {
        gpio_set_level(LED_GPIO, status->valueint);
        ESP_LOGI(TAG, "LED set to: %d", status->valueint);
    }

    cJSON_Delete(root);
    break;
}

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT disconnected");
        break;

    default:
        break;
    }
}


// =========================
// MQTT Init Function
// =========================
void mqtt_app_start(void)
{
    // Configure LED GPIO
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com",
        .credentials.client_id = "esp32s3_unique_001",

        // Stability settings
        .session.keepalive = 60,
        .network.timeout_ms = 10000,
        .network.reconnect_timeout_ms = 5000,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    esp_mqtt_client_start(client);
}