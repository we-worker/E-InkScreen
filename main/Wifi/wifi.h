#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/inet.h"
#include "lwip/lwip_napt.h"
#include "nvs_flash.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>

// 热点名称 密码  可连接数量
#define AP_WIFI_SSID		"E-link"
#define AP_WIFI_PASS		"nbu12345"
#define AP_MAX_STA_CONN		4

// 路由器wifi名称 密码
#define STA_WIFI_SSID		"h3c_wifi"
#define STA_WIFI_PASS		"h3c_wifi"

static const char *TAG = "EWIFI";

static esp_netif_t* _esp_netif_sta = NULL;
static esp_netif_t* _esp_netif_ap = NULL;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

static void sta_start_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

static void got_ip_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void wifi_init_softap(void);

void wifi_init_all(void);