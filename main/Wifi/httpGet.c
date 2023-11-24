#include "wifi.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include <string.h>
#include "httpGet.h"
#include "esp_http_client.h"
#include "esp_tls.h"

#include "E-link/EPaperDrive.h"

// http状态行为定义
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	static char *output_buffer; // Buffer to store response of http request from event handler
	static int output_len;		// Stores number of bytes read

	switch (evt->event_id)
	{
	case HTTP_EVENT_ERROR:
		ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		/*
		 *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
		 *  However, event handler can also be used in case chunked encoding is used.
		 */
		if (!esp_http_client_is_chunked_response(evt->client))
		{
			// If user_data buffer is configured, copy the response into the buffer
			if (evt->user_data)
			{
				memcpy(evt->user_data + output_len, evt->data, evt->data_len);
			}
			else
			{
				if (output_buffer == NULL)
				{
					output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client));
					output_len = 0;
					if (output_buffer == NULL)
					{
						ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
						return ESP_FAIL;
					}
				}
				memcpy(output_buffer + output_len, evt->data, evt->data_len);
			}
			output_len += evt->data_len;
		}
		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
		if (output_buffer != NULL)
		{
			// Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
			// ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
			free(output_buffer);
			output_buffer = NULL;
			output_len = 0;
		}
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
		int mbedtls_err = 0;
		esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
		if (err != 0)
		{
			if (output_buffer != NULL)
			{
				free(output_buffer);
				output_buffer = NULL;
				output_len = 0;
			}
			ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
			ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
		}
		break;
	}
	return ESP_OK;
}

void show_news(char *words){

	EPD_init_Full();                       // 全刷初始化，使用全刷波形
	clearbuffer();   // 清空缓存(全白)
	fontscale = 2;
	DrawUTF(0, 115, "睡前消息列表");
   DrawXline(0, 400, 40);
   fontscale = 1; // 字体缩放系数(支持1和2,对图片也有效，用完记得重新改成1)
	DrawUTF(50, 0, words);
//    list_pos+=1;
   // EPD.fontscale = 1;                // 字体缩放系数改回1

    EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕

	
   deepsleep(); // 让屏幕进入休眠模式

}


void get_news()
{
	char local_response_buffer[3048] = {0};


	esp_http_client_config_t config = {
	.host = "darkarc.dns.army:10002",
	.path = "/",
	.event_handler = _http_event_handler,
	.user_data = local_response_buffer};

	// 初始化一个客户端
	esp_http_client_handle_t client = esp_http_client_init(&config);
	esp_http_client_set_url(client, "http://darkarc.dns.army:10002/news");
    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
	char *p = strstr(local_response_buffer, "</html>"); // 查找字符串中的"1.在华"
	if (p) {
    	strcpy(local_response_buffer, p+9); // 将后面的部分复制到buf的开头
	}
	printf("get:%s\n",local_response_buffer);
	// 数据会保存到之前注册在http-client对象中的数据缓存区中
	esp_http_client_cleanup(client);
	if(strlen(local_response_buffer)>20)
		show_news(local_response_buffer);
}