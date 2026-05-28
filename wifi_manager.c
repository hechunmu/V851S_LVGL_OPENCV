#include "wifi_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/*  Shared state                                                       */
/* ------------------------------------------------------------------ */
static pthread_mutex_t  g_lock     = PTHREAD_MUTEX_INITIALIZER;
static pthread_t        g_tid      = 0;
static wifi_state_t     g_state    = WIFI_STATE_IDLE;
static wifi_ap_t        g_aps[WIFI_MAX_AP];
static int              g_ap_count = 0;
static char             g_connect_ssid[WIFI_SSID_MAX]    = "";
static char             g_connect_pass[256]              = "";
static char             g_connected_ssid[WIFI_SSID_MAX]  = "";

/* Large static buffers — never on thread stack */
static char g_scan_buf[65536];

/* ------------------------------------------------------------------ */
/*  Helpers                                                            */
/* ------------------------------------------------------------------ */
static int dbm_to_bars(int dbm)
{
    if (dbm >= -55) return 5;
    if (dbm >= -65) return 4;
    if (dbm >= -75) return 3;
    if (dbm >= -85) return 2;
    return 1;
}

static const char *wifi_iface(void)
{
    const char *e = getenv("WIFI_IFACE");
    return e ? e : "wlan0";
}

/* Run cmd, capture stdout into buf (up to len-1 bytes). Returns pclose() rc. */
static int run_cmd(const char *cmd, char *buf, int len)
{
    FILE *f = popen(cmd, "r");
    if (!f) { if (buf && len > 0) buf[0] = '\0'; return -1; }
    int n = 0;
    if (buf && len > 0) {
        n = (int)fread(buf, 1, len - 1, f);
        buf[n < 0 ? 0 : n] = '\0';
    }
    return pclose(f);
}

/* ------------------------------------------------------------------ */
/*  Parse wpa_cli scan_results                                         */
/*  Format: bssid\tfreq\tsignal\tflags\tssid                          */
/* ------------------------------------------------------------------ */
static int parse_wpa_scan_results(const char *txt, wifi_ap_t *out, int max)
{
    int count = 0;
    const char *p = txt;
    char line[256];

    /* skip header line */
    const char *nl = strchr(p, '\n');
    if (nl) p = nl + 1;

    while (*p && count < max) {
        nl = strchr(p, '\n');
        int ll = nl ? (int)(nl - p) : (int)strlen(p);
        if (ll >= (int)sizeof(line)) ll = (int)sizeof(line) - 1;
        memcpy(line, p, ll);
        line[ll] = '\0';
        p += ll + (nl ? 1 : 0);

        if (!line[0]) continue;

        /* Split by tab: bssid freq signal flags ssid */
        char *f[5] = { NULL, NULL, NULL, NULL, NULL };
        char *tok = line;
        for (int i = 0; i < 5; i++) {
            f[i] = tok;
            tok = strchr(tok, '\t');
            if (!tok) break;
            *tok++ = '\0';
        }
        if (!f[2] || !f[3] || !f[4]) continue;
        if (!f[4][0]) continue;   /* skip hidden SSIDs */

        wifi_ap_t ap;
        memset(&ap, 0, sizeof(ap));

        strncpy(ap.ssid, f[4], WIFI_SSID_MAX - 1);
        ap.bars    = dbm_to_bars(atoi(f[2]));
        ap.secured = (strstr(f[3], "WPA") != NULL ||
                      strstr(f[3], "WEP") != NULL);
        out[count++] = ap;
    }
    return count;
}

/* ------------------------------------------------------------------ */
/*  Parse iwlist scan (fallback)                                       */
/* ------------------------------------------------------------------ */
static int parse_iwlist_scan(const char *txt, wifi_ap_t *out, int max)
{
    int count = 0;
    wifi_ap_t cur;
    memset(&cur, 0, sizeof(cur));
    bool in_cell = false;

    const char *p = txt;
    char line[512];
    while (*p) {
        const char *nl = strchr(p, '\n');
        int ll = nl ? (int)(nl - p) : (int)strlen(p);
        if (ll >= (int)sizeof(line)) ll = (int)sizeof(line) - 1;
        memcpy(line, p, ll);
        line[ll] = '\0';
        p += ll + (nl ? 1 : 0);

        const char *t = line;
        while (*t == ' ' || *t == '\t') t++;

        if (strstr(t, "Cell ")) {
            if (in_cell && cur.ssid[0] && count < max) out[count++] = cur;
            memset(&cur, 0, sizeof(cur));
            in_cell = true;
            continue;
        }
        if (!in_cell) continue;

        if (strncmp(t, "ESSID:", 6) == 0) {
            const char *q = strchr(t + 6, '"');
            if (q) {
                q++;
                const char *e = strchr(q, '"');
                int l = e ? (int)(e - q) : (int)strlen(q);
                if (l >= WIFI_SSID_MAX) l = WIFI_SSID_MAX - 1;
                memcpy(cur.ssid, q, l);
                cur.ssid[l] = '\0';
            }
        } else if (strstr(t, "Signal level=")) {
            const char *s = strstr(t, "Signal level=");
            int dbm = (int)strtol(s + 13, NULL, 10);
            if (dbm == 0) {
                /* Quality=50/70 style */
                const char *q = strstr(t, "Quality=");
                if (q) {
                    int num = atoi(q + 8);
                    char *sl = strchr(q + 8, '/');
                    int den = sl ? atoi(sl + 1) : 70;
                    int pct = den > 0 ? num * 100 / den : 50;
                    dbm = -100 + pct / 2;
                }
            }
            cur.bars = dbm_to_bars(dbm);
        } else if (strstr(t, "Encryption key:on")) {
            cur.secured = true;
        }
    }
    if (in_cell && cur.ssid[0] && count < max) out[count++] = cur;
    return count;
}

/* ------------------------------------------------------------------ */
/*  Get currently connected SSID via wpa_cli status                   */
/* ------------------------------------------------------------------ */
static void fetch_connected_ssid(char *buf, int len)
{
    buf[0] = '\0';
    char cmd[160];
    snprintf(cmd, sizeof(cmd),
             "wpa_cli -i %s status 2>/dev/null | grep '^ssid=' | head -1",
             wifi_iface());
    char out[128] = "";
    run_cmd(cmd, out, sizeof(out));
    char *eq = strchr(out, '=');
    if (eq) {
        eq++;
        char *nl = strchr(eq, '\n');
        if (nl) *nl = '\0';
        strncpy(buf, eq, len - 1);
        buf[len - 1] = '\0';
    }
}

/* ------------------------------------------------------------------ */
/*  Scan thread                                                        */
/* ------------------------------------------------------------------ */
static void *scan_thread(void *arg)
{
    (void)arg;
    const char *iface = wifi_iface();
    char cmd[256];
    int count = 0;
    wifi_ap_t tmp[WIFI_MAX_AP];

    /* Bring interface up first */
    snprintf(cmd, sizeof(cmd),
             "ip link set %s up 2>/dev/null || ifconfig %s up 2>/dev/null || true",
             iface, iface);
    system(cmd);
    usleep(300000);   /* 300 ms settle */

    /* PRIMARY: wpa_cli scan + scan_results  (most reliable on OpenWrt) */
    snprintf(cmd, sizeof(cmd), "wpa_cli -i %s scan >/dev/null 2>&1", iface);
    system(cmd);
    sleep(3);   /* let driver collect beacons */

    snprintf(cmd, sizeof(cmd), "wpa_cli -i %s scan_results 2>/dev/null", iface);
    pthread_mutex_lock(&g_lock);
    /* use global buffer — safe because only one scan thread runs at a time */
    g_scan_buf[0] = '\0';
    pthread_mutex_unlock(&g_lock);

    run_cmd(cmd, g_scan_buf, sizeof(g_scan_buf));
    count = parse_wpa_scan_results(g_scan_buf, tmp, WIFI_MAX_AP);

    /* FALLBACK 1: iw dev scan */
    if (count == 0) {
        snprintf(cmd, sizeof(cmd), "iw dev %s scan 2>/dev/null", iface);
        run_cmd(cmd, g_scan_buf, sizeof(g_scan_buf));
        /* basic iw parse: SSID + signal lines */
        const char *p = g_scan_buf;
        wifi_ap_t cur; memset(&cur, 0, sizeof(cur));
        bool in_bss = false;
        char line[256];
        while (*p && count < WIFI_MAX_AP) {
            const char *nl = strchr(p, '\n');
            int ll = nl ? (int)(nl - p) : (int)strlen(p);
            if (ll >= (int)sizeof(line)) ll = (int)sizeof(line) - 1;
            memcpy(line, p, ll); line[ll] = '\0';
            p += ll + (nl ? 1 : 0);
            const char *t = line; while (*t == ' ' || *t == '\t') t++;
            if (strncmp(t, "BSS ", 4) == 0) {
                if (in_bss && cur.ssid[0]) tmp[count++] = cur;
                memset(&cur, 0, sizeof(cur)); in_bss = true;
            } else if (in_bss && strncmp(t, "SSID: ", 6) == 0 && t[6]) {
                strncpy(cur.ssid, t + 6, WIFI_SSID_MAX - 1);
            } else if (in_bss && strncmp(t, "signal: ", 8) == 0) {
                cur.bars = dbm_to_bars((int)strtod(t + 8, NULL));
            } else if (in_bss && (strncmp(t, "RSN:", 4) == 0 ||
                                   strncmp(t, "WPA:", 4) == 0)) {
                cur.secured = true;
            }
        }
        if (in_bss && cur.ssid[0] && count < WIFI_MAX_AP) tmp[count++] = cur;
    }

    /* FALLBACK 2: iwlist */
    if (count == 0) {
        snprintf(cmd, sizeof(cmd), "iwlist %s scan 2>/dev/null", iface);
        run_cmd(cmd, g_scan_buf, sizeof(g_scan_buf));
        count = parse_iwlist_scan(g_scan_buf, tmp, WIFI_MAX_AP);
    }

    /* Mark connected AP */
    char cur_ssid[WIFI_SSID_MAX] = "";
    fetch_connected_ssid(cur_ssid, sizeof(cur_ssid));
    for (int i = 0; i < count; i++)
        tmp[i].connected = (cur_ssid[0] &&
                            strcmp(tmp[i].ssid, cur_ssid) == 0);

    pthread_mutex_lock(&g_lock);
    memcpy(g_aps, tmp, count * sizeof(wifi_ap_t));
    g_ap_count = count;
    strncpy(g_connected_ssid, cur_ssid, WIFI_SSID_MAX - 1);
    g_state = WIFI_STATE_SCAN_DONE;
    pthread_mutex_unlock(&g_lock);
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Connect thread                                                     */
/* ------------------------------------------------------------------ */
static void *connect_thread(void *arg)
{
    (void)arg;
    const char *iface = wifi_iface();
    char ssid[WIFI_SSID_MAX], pass[256], cmd[512];

    pthread_mutex_lock(&g_lock);
    strncpy(ssid, g_connect_ssid, sizeof(ssid) - 1);
    strncpy(pass, g_connect_pass, sizeof(pass) - 1);
    pthread_mutex_unlock(&g_lock);

    /* Remove existing networks */
    snprintf(cmd, sizeof(cmd),
             "wpa_cli -i %s remove_network all >/dev/null 2>&1 || true", iface);
    system(cmd);

    snprintf(cmd, sizeof(cmd),
             "wpa_cli -i %s add_network >/dev/null 2>&1", iface);
    system(cmd);

    snprintf(cmd, sizeof(cmd),
             "wpa_cli -i %s set_network 0 ssid '\"%s\"' >/dev/null 2>&1",
             iface, ssid);
    system(cmd);

    if (pass[0]) {
        snprintf(cmd, sizeof(cmd),
                 "wpa_cli -i %s set_network 0 psk '\"%s\"' >/dev/null 2>&1",
                 iface, pass);
        system(cmd);
    } else {
        snprintf(cmd, sizeof(cmd),
                 "wpa_cli -i %s set_network 0 key_mgmt NONE >/dev/null 2>&1",
                 iface);
        system(cmd);
    }

    snprintf(cmd, sizeof(cmd),
             "wpa_cli -i %s enable_network 0 >/dev/null 2>&1", iface);
    system(cmd);

    /* Poll up to 15 s */
    char connected[WIFI_SSID_MAX] = "";
    for (int i = 0; i < 30; i++) {
        usleep(500000);
        fetch_connected_ssid(connected, sizeof(connected));
        if (connected[0] && strcmp(connected, ssid) == 0) break;
        connected[0] = '\0';
    }

    pthread_mutex_lock(&g_lock);
    if (connected[0]) {
        strncpy(g_connected_ssid, connected, WIFI_SSID_MAX - 1);
        g_state = WIFI_STATE_CONNECTED;
    } else {
        g_state = WIFI_STATE_FAILED;
    }
    pthread_mutex_unlock(&g_lock);
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */
void wifi_scan_async(void)
{
    pthread_mutex_lock(&g_lock);
    if (g_state == WIFI_STATE_SCANNING || g_state == WIFI_STATE_CONNECTING) {
        pthread_mutex_unlock(&g_lock);
        return;
    }
    g_state = WIFI_STATE_SCANNING;
    pthread_mutex_unlock(&g_lock);

    if (g_tid) { pthread_detach(g_tid); g_tid = 0; }
    pthread_create(&g_tid, NULL, scan_thread, NULL);
}

int wifi_get_results(wifi_ap_t *out, int max)
{
    pthread_mutex_lock(&g_lock);
    int n = g_ap_count < max ? g_ap_count : max;
    memcpy(out, g_aps, n * sizeof(wifi_ap_t));
    pthread_mutex_unlock(&g_lock);
    return n;
}

wifi_state_t wifi_get_state(void)
{
    pthread_mutex_lock(&g_lock);
    wifi_state_t s = g_state;
    pthread_mutex_unlock(&g_lock);
    return s;
}

const char *wifi_get_state_str(void)
{
    switch (wifi_get_state()) {
    case WIFI_STATE_IDLE:       return "Press Scan to search";
    case WIFI_STATE_SCANNING:   return "Scanning...";
    case WIFI_STATE_SCAN_DONE:  return "Scan complete";
    case WIFI_STATE_CONNECTING: return "Connecting...";
    case WIFI_STATE_CONNECTED:  return "Connected";
    case WIFI_STATE_FAILED:     return "Connection failed";
    default:                    return "";
    }
}

void wifi_connect_async(const char *ssid, const char *password)
{
    pthread_mutex_lock(&g_lock);
    strncpy(g_connect_ssid, ssid,     sizeof(g_connect_ssid) - 1);
    strncpy(g_connect_pass, password, sizeof(g_connect_pass) - 1);
    g_state = WIFI_STATE_CONNECTING;
    pthread_mutex_unlock(&g_lock);

    if (g_tid) { pthread_detach(g_tid); g_tid = 0; }
    pthread_create(&g_tid, NULL, connect_thread, NULL);
}

void wifi_get_connected_ssid(char *buf, int len)
{
    pthread_mutex_lock(&g_lock);
    strncpy(buf, g_connected_ssid, len - 1);
    buf[len - 1] = '\0';
    pthread_mutex_unlock(&g_lock);
}
