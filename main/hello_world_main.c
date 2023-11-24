
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "E-link/EPaperDrive.h"
#include "E-link/FileSystem.h"
#include "Screens/ExamScheduling.h"
#include "Screens/Todo.h"
#include "Wifi/wifi.h"
#include "Wifi/httpGet.h"
#include "Screens/memu.h"


void app_main(void)
{
      vTaskDelay(2 / portTICK_PERIOD_MS);
      printf("Hello \n");
      fflush(stdout);
      // 驱动库的实例化，此处为使用软件SPI
      extern gpio_num_t CS;
      extern gpio_num_t RST;
      extern gpio_num_t DC;
      extern gpio_num_t BUSY_line;
      extern gpio_num_t CLK;
      extern gpio_num_t DIN;
      extern uint8_t EPDbuffer[400 * 300 / 8]; ///>屏幕图像
      // 初始化wifi
      wifi_init_all();
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      EPaperDrive(CS, RST, DC, BUSY_line, CLK, DIN);
      LittlefsBegin();
     // ExamSchedue();
	// EPD_init_Full();                       // 全刷初始化，使用全刷波形
      // vTaskDelay(3000 / portTICK_PERIOD_MS);
      //    extern uint8_t fontscale;
      //    fontscale=1;
      //     clearbuffer();   // 清空缓存(全白)

      //get_news();
      
      main_memu();
      xTaskCreate(Read_keys, "button_task", 2048, NULL, 10, NULL);
}
