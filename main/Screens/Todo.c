#include "Todo.h"

int list_pos=1;


void list_clear_all(){
	clearbuffer();   // 清空缓存(全白)
      vTaskDelay(1000 / portTICK_PERIOD_MS);
   list_pos=0;
   EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕

   deepsleep(); // 让屏幕进入休眠模式
}

//TODO不能局部刷新
void list_add(char *words){
// 	fontscale = 2; // 字体缩放系数(支持1和2,对图片也有效，用完记得重新改成1)
//    for (int i = 0; i < 2; i++)
//    {
//       vTaskDelay(3000 / portTICK_PERIOD_MS);
//       EPD_init_Part();
//       clearbuffer(); // 清空缓存(全白)
// 	  printf(words);
// 	  printf(words);
//       DrawUTF(300, 0, words);
//       EPD_Dis_Part(0, 300, 0, 300, (uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片局新屏幕
//    }
//       EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕

// 	deepsleep();                                          // 让屏幕进入休眠模式
	// clearbuffer();   // 清空缓存(全白)
   vTaskDelay(1000 / portTICK_PERIOD_MS);
   fontscale = 2; // 字体缩放系数(支持1和2,对图片也有效，用完记得重新改成1)
   // SetFont(FONT12);              // 选择字体，具体支持的字体见文档

	DrawUTF(130+30*list_pos, 0, words);
   list_pos+=1;
   // EPD.fontscale = 1;                // 字体缩放系数改回1
   //   EPD.DrawXbm_P(60, 0, 12, 12, (uint8_t *)city_icon); //绘制图片
   // printf("缓存图像绘制完毕，准备全刷 \n");
//    DrawYline(26, 150, 130);
   EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕

   // deepsleep(); // 让屏幕进入休眠模式

}