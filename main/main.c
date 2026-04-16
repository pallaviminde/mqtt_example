#include <stdio.h>
#include "nvs_flash.h"
#include "wifi.h"
#include "app_mqtt.h"
#include "driver/gpio.h"

#define LED_GPIO 4

void app_main(void)
{

    nvs_flash_init();
 
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    wifi_init_sta();

}