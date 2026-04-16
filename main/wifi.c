#include "wifi.h"
#include "app_mqtt.h"   // For mqtt_app_start()

#include "freertos/FreeRTOS.h" // FreeRTOS functions and types
#include "freertos/task.h"     // FreeRTOS task management
#include "esp_wifi.h"          // WiFi functions and types
#include "esp_event.h"         // Event handling
#include "esp_log.h"           // Logging
#include "nvs_flash.h"        // Non-volatile storage (required for WiFi)
#include "esp_netif.h"       // Network interface (required for WiFi)
 
// SSID and password of WiFi network
#define WIFI_SSID "V2036"    
#define WIFI_PASS "pallavi@12324"

// Logging tag
static const char *TAG = "wifi";

// Flag to ensure MQTT starts only once
static bool mqtt_started = false;

/*
 * Function: wifi_event_handler
 * Handles WiFi and IP events:
 * - Connect when WiFi starts
 * - Retry connection on disconnect
 * - Start MQTT after getting IP address
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    /* ----------- WiFi Start Event ----------- */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // Connect to configured WiFi network
        esp_wifi_connect();
    } 

    /* ----------- WiFi Disconnected ----------- */
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // Retry connection automatically
        ESP_LOGI(TAG, "Retrying WiFi...");
        esp_wifi_connect();
    } 
    /* ----------- Got IP Address ----------- */
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
{
    ESP_LOGI(TAG, "WiFi Connected, Got IP!");
    // Start MQTT only once after successful connection
    if (!mqtt_started)
    {
        // Small delay to ensure network stability (optional)
        vTaskDelay(pdMS_TO_TICKS(2000));  // optional but helpful
        mqtt_app_start();   // Start MQTT communication
        mqtt_started = true;
    }
}
}
/*
 * Function: wifi_init_sta
 * Initializes WiFi in station mode (STA).
 * Steps:
 * 1. Initialize network interface
 * 2. Create default event loop
 * 3. Initialize WiFi driver
 * 4. Register event handlers
 * 5. Configure WiFi credentials
 * 6. Start WiFi
 */
void wifi_init_sta(void)
{
    /* ----------- Network Initialization ----------- */
    // Initialize TCP/IP stack
    esp_netif_init();  
    // Create event loop
    esp_event_loop_create_default();
    // Create WiFi STA interface
    esp_netif_create_default_wifi_sta();

    /* ----------- WiFi Driver Init ----------- */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

     /* ----------- Event Handler Registration ----------- */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    // Register handler for all WiFi events
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_any_id);

    // Register handler for IP acquisition event
    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_got_ip);
    /* ----------- WiFi Configuration ----------- */
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    // Set WiFi mode to station
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Apply WiFi credentials
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    // Start WiFi
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}