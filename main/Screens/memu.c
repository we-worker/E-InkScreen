#include "memu.h"
#include "driver/gpio.h"
#include "ExamScheduling.h"
#include "Wifi/httpGet.h"
#include "Wifi/wifi.h"

uint8_t cursor_index=1;
uint8_t Screen_Mode=1;//1为主菜单
void memu_cursor(){
	uint8_t arrow[]={0, 0, 6, 0, 0, 0, 0, 0, 248, 0, 0, 0, 0, 63, 192, 0, 0, 0, 7, 254, 0, 0, 0, 0, 254, 127, 255, 254, 0, 63, 195, 255, 255, 252, 7, 248, 31, 255, 255, 224, 254, 0, 0, 0, 7, 63, 192, 0, 0, 0, 59, 248, 0, 0, 0, 1, 254, 0, 0, 0, 0, 14, 254, 0, 0, 0, 0, 115, 252, 0, 0, 0, 3, 131, 248, 1, 255, 255, 252, 7, 248, 31, 255, 255, 224, 15, 240, 255, 255, 255, 0, 15, 231, 0, 0, 0, 0, 31, 248, 0, 0, 0, 0, 63, 192, 0, 0, 0, 0, 62, 0, 0, 0, 0, 0, 112, 0, 0, 0};
	uint8_t memu_pos[]={109,159,209};
	EPD_init_Part();
	clearbuffer();   // 清空缓存(全白)
	DrawXbm_P(200,memu_pos[cursor_index-1],45,21,arrow);
	EPD_Dis_Part(50,250,200,300,0);
	deepsleep(); // 让屏幕进入休眠模式
}

void main_memu(){
	EPD_init_Full();                       // 全刷初始化，使用全刷波形
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	clearbuffer();   // 清空缓存(全白)
		fontscale = 2;
	DrawUTF(10, 150, "主菜单");
   DrawXline(0, 400, 40);
	DrawUTF(109, 100, "考试安排");
	DrawUTF(159, 100, "图片显示");
	DrawUTF(209, 100, "每日新闻");
	extern char ip_addr[16];
	DrawUTF(250, 100, "IP:");
	DrawUTF(250, 130, ip_addr);
    EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕

   deepsleep(); // 让屏幕进入休眠模式

}


void Read_keys(void *arg)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1<<IO0_GPIO) | (1<<IO1_GPIO) | (1<<IO2_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    while (1) {
        if (gpio_get_level(IO0_GPIO)==0) {
            // printf("下 pressed\n");
			cursor_index++;
			if (cursor_index > 3)
    			cursor_index = 1;
			memu_cursor();
        }
        if (gpio_get_level(IO1_GPIO)==0) {
            // printf("确定 pressed\n");
			if(Screen_Mode ==1){
				switch (cursor_index)
				{
				case 1:
					printf("考试安排\n");
					Screen_Mode=2;//2为考试安排
					ExamSchedue();
					break;
				case 3:
					get_news();
					Screen_Mode=3;
					break;
				default:
					break;
				}
			}else{
				main_memu();
				Screen_Mode=1;
			}
        }
        if (gpio_get_level(IO2_GPIO)==0) {
            // printf("上 pressed\n");
			cursor_index--;
			if (cursor_index < 1)
    			cursor_index = 3;
			memu_cursor();
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
