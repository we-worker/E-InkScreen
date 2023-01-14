
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
	EPD_init_Full();                       // 全刷初始化，使用全刷波形
      vTaskDelay(3000 / portTICK_PERIOD_MS);
         extern uint8_t fontscale;
         fontscale=1;
          clearbuffer();   // 清空缓存(全白)

      get_news();

      //    EPD_Dis_Full((uint8_t *)image_data, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕
      // for (int i = 0; i < 3; i++)
      // {


      // vTaskDelay(10000 / portTICK_PERIOD_MS);
      // //模式切换时，需要多等一会几秒完全刷新好才行。
      // EPD_init_Part();
      // clearbuffer(); // 清空缓存(全白)
      // DrawUTF(26, 0, "这是局部刷新效果显示");         
      // DrawUTF(46, 0, "1234567890");    
      // // vTaskDelay(3000 / portTICK_PERIOD_MS);
      // EPD_Dis_Part(0, 100, 0, 300, 1); // 将缓存中的图像传给屏幕控制芯片局新屏幕
      // deepsleep();
      // vTaskDelay(3000 / portTICK_PERIOD_MS);
      // }
}
