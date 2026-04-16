#ifndef APP_MQTT_H  // Prevents multiple inclusion of this header file
#define APP_MQTT_H

/*
 * Function: mqtt_app_start
 * Initializes and starts the MQTT client.
 * Responsibilities:
 * - Configure MQTT connection
 * - Register event handlers
 * - Start communication with broker
 * - Initialize required peripherals (e.g., GPIO)
 */

void mqtt_app_start(void);

#endif // APP_MQTT_H