#include "wifi.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include <string.h>
#include "Screens\Todo.h"

char index_html[]="<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\" />\
    <title>表单标签</title>\
  </head>\
  <body>\
    <form action=\"/echo\" method=\"post\">\
      事项：<input name=\"words\" /><br />\
      <!--登陆按钮-->\
      <input type=\"submit\" value=\"登录\" />\
    </form>\
    <form action=\"/echo\" method=\"post\">\
        <input type=\"submit\" value=\"清空\" />\
      </form>\
  </body>\
</html>";

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[32];
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0) {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  =index_html

};

/* An HTTP POST handler */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        // printf(buf);
        ESP_LOGI(TAG, "====================================");

        list_add(buf);
    }

    // End response
    //TODO:无法完成302转跳，疑似是因为主函数还是再echo中。
    httpd_resp_send_chunk(req, NULL, 0);
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/.");
    httpd_resp_send(req, NULL, 0); // Response body can be empty
    ESP_LOGI(TAG, "转跳完成");
    return ESP_OK;
}

static const httpd_uri_t echo = {
    .uri       = "/echo",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = index_html
};

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    } else if (strcmp("/echo", req->uri) == 0) {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        httpd_register_uri_handler(server, &echo);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}



void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	if (event_id == WIFI_EVENT_AP_STACONNECTED)
	{
		wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
		ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
				 MAC2STR(event->mac), event->aid);
	}
	else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
	{
		wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
		ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
				 MAC2STR(event->mac), event->aid);
	}
}

void sta_start_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	esp_wifi_connect();
}

void got_ip_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	// No need to log, wifi driver logs automatically
	esp_netif_dns_info_t dns;
	if (esp_netif_get_dns_info(_esp_netif_sta, ESP_NETIF_DNS_MAIN, &dns) == ESP_OK)
	{
		dhcps_dns_setserver((const ip_addr_t *)&dns.ip);
		ESP_LOGI(TAG, "set dns to:" IPSTR, IP2STR(&dns.ip.u_addr.ip4));
	}
}

void wifi_init_softap(void)
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

	wifi_config_t wifi_config = {
		.ap = {
			.ssid = AP_WIFI_SSID,
			.ssid_len = strlen(AP_WIFI_SSID),
			.password = AP_WIFI_PASS,
			.max_connection = AP_MAX_STA_CONN,
			.authmode = WIFI_AUTH_WPA_WPA2_PSK},
	};
	if (strlen(AP_WIFI_PASS) == 0)
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

	wifi_config_t cfg1 = {
		.sta = {
			.ssid = STA_WIFI_SSID,
			.password = STA_WIFI_PASS,
			.threshold = {.rssi = 0, .authmode = WIFI_AUTH_WPA2_PSK},
			.pmf_cfg = {
				.capable = true,
				.required = false},
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg1));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, sta_start_handler, NULL));
	ESP_ERROR_CHECK(esp_wifi_start());

	esp_netif_ip_info_t ip_info;
	esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

	char ip_addr[16];
	inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
	ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

	ESP_LOGI(TAG, "wifi_init_softap finished. SSID:'%s' password:'%s'", AP_WIFI_SSID, AP_WIFI_PASS);
}



void wifi_init_all(void){
	   // Initialize networking stack
	ESP_ERROR_CHECK(esp_netif_init());

	// Create default event loop needed by the  main app
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Initialize NVS needed by Wi-Fi
	ESP_ERROR_CHECK(nvs_flash_init());

	// Initialize Wi-Fi including netif with default config
	_esp_netif_ap = esp_netif_create_default_wifi_ap();
	_esp_netif_sta = esp_netif_create_default_wifi_sta();

	// Initialise ESP32 in SoftAP mode
	wifi_init_softap();

	ip_addr_t dnsserver;
	// Enable DNS (offer) for dhcp server
	dhcps_offer_t dhcps_dns_value = OFFER_DNS;
	dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));
	// Set custom dns server address for dhcp server 默认跟随路由器 【推荐换成国内DNS】
	dnsserver.u_addr.ip4.addr = htonl(0xC0A80301);
	dnsserver.type = IPADDR_TYPE_V4;
	dhcps_dns_setserver(&dnsserver);

#if IP_NAPT
	// !!! 必须启动sta后再设置，不然ap无网络 !!! Set to ip address of softAP netif (Default is 192.168.4.1)
	u32_t napt_netif_ip = 0xC0A80401;
	ip_napt_enable(htonl(napt_netif_ip), 1);
#endif
    static httpd_handle_t server = NULL;
    server = start_webserver();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
}