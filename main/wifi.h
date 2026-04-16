#ifndef WIFI_H // Prevents multiple inclusion of this header file
#define WIFI_H

/*
 * Function: wifi_init_sta
 * Initializes WiFi in station mode (STA) and connects
 * to the configured access point.
 * Notes:
 * - Typically handles connection, reconnection, and event processing
 * - Implementation is defined in wifi.c
 */

void wifi_init_sta(void);

#endif // WIFI_H