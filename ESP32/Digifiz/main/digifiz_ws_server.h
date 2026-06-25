#ifndef DIGIFIZ_WS_SERVER
#define DIGIFIZ_WS_SERVER
#include <inttypes.h>

/**
 * @file digifiz_ws_server.h
 * @brief Wi-Fi connection and websocket update status interface.
 */

/**
 * @brief Start Wi-Fi connection workflow for Digifiz networking.
 */
void digifiz_wifi_connect(void);

/**
 * @brief Get status flag indicating whether OTA/config update is in progress.
 *
 * @return uint8_t Non-zero when update flow is active, 0 otherwise.
 */
uint8_t get_update_in_progress(void);

#endif
