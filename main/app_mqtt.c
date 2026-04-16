#include "app_mqtt.h"     
#include "esp_log.h"      // Logging
#include "mqtt_client.h"  // MQTT client
#include "cJSON.h"       // JSON handling
#include "driver/gpio.h"   // GPIO control
#include <string.h>       // String handling

// GPIO used for LED control
#define LED_GPIO 4

// Logging tag
static const char *TAG = "MQTT";

// Prevent multiple task creation
// Flag to ensure publish task is created only once
static bool task_started = false;

// MQTT Publish Task (JSON)
/*
 * Function: mqtt_publish_task
 * FreeRTOS task that periodically publishes device data to MQTT broker.
 * Publishes JSON:
 * {
 *   "device": "ESP32S3",
 *   "status": <LED state>
 * }
 * pvParameters: MQTT client handle
 */
void mqtt_publish_task(void *pvParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t) pvParameters;

    while (1)
    {
        // Create JSON object
        cJSON *root = cJSON_CreateObject();

        // Add device name and LED status to JSON
        cJSON_AddStringToObject(root, "device", "ESP32S3");

        // Read LED state from GPIO and add to JSON
        int led_state = gpio_get_level(LED_GPIO);

        // Add LED state to JSON
        cJSON_AddNumberToObject(root, "status", led_state);

        // Convert JSON object to string
        char *json_data = cJSON_PrintUnformatted(root);

        // Publish data to MQTT topic
        esp_mqtt_client_publish(client, "iot/device/data", json_data, 0, 1, 0);

        ESP_LOGI(TAG, "Published: %s", json_data);

        // Free allocated memory
        cJSON_Delete(root);
        free(json_data);

        // Delay for 5 seconds before next publish
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
    // Log received payload (not null-terminated)
    ESP_LOGI(TAG, "Received Data: %.*s", event->data_len, event->data);

    // Parse JSON safely using length
    cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);

    if (root == NULL)
    {
        ESP_LOGE(TAG, "JSON parsing failed");
        break;
    }

    // Extract "status" field from JSON
    cJSON *status = cJSON_GetObjectItem(root, "status");

    // Validate and apply GPIO control
    if (cJSON_IsNumber(status))
    {
        // Set LED state (0 = OFF, 1 = ON)
        gpio_set_level(LED_GPIO, status->valueint);
        ESP_LOGI(TAG, "LED set to: %d", status->valueint);
    }

    // Free JSON object memory
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

/*
 * Function: mqtt_app_start
 * Initializes MQTT client and starts communication.
 * Steps:
 * 1. Configure GPIO for LED
 * 2. Setup MQTT client configuration
 * 3. Register event handler
 * 4. Start MQTT client
 */
void mqtt_app_start(void)
{
    // Configure LED GPIO
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    /* ----------- MQTT Configuration ----------- */
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.hivemq.com",
        // Unique client ID for broker identification
        .credentials.client_id = "esp32s3_unique_001",

        // Stability settings
        .session.keepalive = 60,     // Keepalive interval (seconds)
        .network.timeout_ms = 10000, // Network timeout (ms)
        .network.reconnect_timeout_ms = 5000,   // Reconnect timeout (ms)
    };

    // Initialize MQTT client
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    // Register event handler for all MQTT events
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    // Start MQTT client
    esp_mqtt_client_start(client);
}