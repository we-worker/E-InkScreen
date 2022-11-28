/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "E-link/EPaperDrive.h"
#include "esp_littlefs.h"


 void app_main(void)
{

    vTaskDelay(2 / portTICK_PERIOD_MS);
    printf("Hello \n");
    fflush(stdout);
        printf("Restarting now.\n");
    // 驱动库的实例化，此处为使用软件SPI
    extern gpio_num_t CS ;
extern gpio_num_t RST ;
extern gpio_num_t DC ;
extern gpio_num_t BUSY_line ;
extern gpio_num_t CLK ;
extern gpio_num_t DIN ;
    EPaperDrive(CS, RST, DC, BUSY_line, CLK, DIN);

    // esp_vfs_littlefs_conf_t conf = {
    //     .base_path = "/littlefs",
    //     .partition_label = "littlefs",
    //     .format_if_mount_failed = true,
    //     .dont_mount = false,
    // };

    // // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    // esp_err_t ret = esp_vfs_littlefs_register(&conf);
       
       /*   墨水屏测试成功

        EPD_init_Full();              // 全刷初始化，使用全刷波形
        clearbuffer();                // 清空缓存(全白)
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    // EPD.fontscale = 2;                // 字体缩放系数(支持1和2,对图片也有效，用完记得重新改成1)
    // EPD.SetFont(FONT12);              // 选择字体，具体支持的字体见文档
    // EPD.DrawUTF(0, 0, "Hello World"); // 绘制字符串
    // EPD.DrawUTF(26, 0, "这是墨水屏"); // 绘制字符串
    // EPD.fontscale = 1;                // 字体缩放系数改回1
    //   EPD.DrawXbm_P(60, 0, 12, 12, (uint8_t *)city_icon); //绘制图片
    // printf("缓存图像绘制完毕，准备全刷 \n");
    DrawYline(0,300,100);
    EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕
    // EPD.deepsleep();                               // 让屏幕进入休眠模式
    // printf("全刷完毕");
       */
}
