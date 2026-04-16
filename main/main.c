#include <stdio.h>
#include "nvs_flash.h"    // Non-volatile storage (required for WiFi)
#include "wifi.h"        // WiFi initialization 
#include "app_mqtt.h"    // MQTT application logic
#include "driver/gpio.h"  // GPIO control

// GPIO used for LED control
#define LED_GPIO 4

/*
 * Function: app_main
 * Main entry point of ESP-IDF application.
 * Steps:
 * 1. Initialize NVS (required for WiFi and networking)
 * 2. Configure LED GPIO as output
 * 3. Start WiFi in station mode
 */

void app_main(void)
{
    /* ----------- NVS Initialization ----------- */
    // Initialize flash storage (required for WiFi stack)
    nvs_flash_init();

    /* ----------- GPIO Setup ----------- */
    // Reset LED pin to default state
    gpio_reset_pin(LED_GPIO);

    // Configure LED pin as output
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    /* ----------- WiFi Initialization ----------- */
    // Start WiFi connection (station mode)
    wifi_init_sta();

}