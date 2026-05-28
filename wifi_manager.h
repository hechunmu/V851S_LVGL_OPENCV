#pragma once
#include <stdbool.h>

#define WIFI_MAX_AP    24
#define WIFI_SSID_MAX  64

typedef struct {
    char ssid[WIFI_SSID_MAX];
    int  bars;       /* signal strength 1-5 */
    bool secured;    /* WPA/WPA2 */
    bool connected;
} wifi_ap_t;

typedef enum {
    WIFI_STATE_IDLE,
    WIFI_STATE_SCANNING,
    WIFI_STATE_SCAN_DONE,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_FAILED,
} wifi_state_t;

/* Trigger async scan (non-blocking) */
void wifi_scan_async(void);

/* Copy latest scan results, returns count */
int  wifi_get_results(wifi_ap_t *out, int max);

/* Get current state */
wifi_state_t wifi_get_state(void);
const char  *wifi_get_state_str(void);

/* Trigger async connect (non-blocking).  password="" for open networks. */
void wifi_connect_async(const char *ssid, const char *password);

/* Get SSID of currently connected network (empty string if none) */
void wifi_get_connected_ssid(char *buf, int len);
