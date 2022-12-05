
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

   //初始化wifi
   wifi_init_all();

   EPaperDrive(CS, RST, DC, BUSY_line, CLK, DIN);

   LittlefsBegin();

   // /*   墨水屏测试成功

   EPD_init_Full(); // 全刷初始化，使用全刷波形

   ExamSchedue();
   // list_add("words=ssssds");





}
