
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "E-link/EPaperDrive.h"
#include "E-link/FileSystem.h"
#include "Screens/ExamScheduling.h"

void app_main(void)
{

   vTaskDelay(2 / portTICK_PERIOD_MS);
   printf("Hello \n");
   fflush(stdout);
   printf("Restarting now.\n");
   // 驱动库的实例化，此处为使用软件SPI
   extern gpio_num_t CS;
   extern gpio_num_t RST;
   extern gpio_num_t DC;
   extern gpio_num_t BUSY_line;
   extern gpio_num_t CLK;
   extern gpio_num_t DIN;


   EPaperDrive(CS, RST, DC, BUSY_line, CLK, DIN);

   LittlefsBegin();

   // /*   墨水屏测试成功

   EPD_init_Full(); // 全刷初始化，使用全刷波形

   ExamSchedue();
   /* code */
   // deepsleep();                                          // 让屏幕进入休眠模式

   // for (int i = 0; i < 2; i++)
   // {
   //    vTaskDelay(3000 / portTICK_PERIOD_MS);
   //    EPD_init_Part();
   //    clearbuffer(); // 清空缓存(全白)
   //    DrawUTF(26, 0, "这是局部刷新效果显示");
   //    DrawUTF(46, 0, "1234567890");
   //    // EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕
   //    EPD_Dis_Part(0, 100, 0, 300, (uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片局新屏幕
   // }

   // EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕

   deepsleep(); // 让屏幕进入休眠模式
                // printf("全刷完毕");
}
