#include "ExamScheduling.h"

   extern uint8_t fontscale;
   extern uint8_t EPDbuffer[400 * 300 / 8];

void ExamSchedue(){
	clearbuffer();   // 清空缓存(全白)
   vTaskDelay(1000 / portTICK_PERIOD_MS);
   fontscale = 2; // 字体缩放系数(支持1和2,对图片也有效，用完记得重新改成1)
   // SetFont(FONT12);              // 选择字体，具体支持的字体见文档
   DrawUTF(0, 80, "考试安排"); // 绘制字符串
   DrawUTF(26, 0, "电力电子：    1月3--9:00"); // 绘制字符串
   DrawUTF(52, 0, "计算机控制： 1月4--9:00"); // 绘制字符串
   DrawUTF(78, 0, "PLC控制：     1月4--13:00"); // 绘制字符串
   DrawUTF(104, 0, "现代控制：    1月5--9:00"); // 绘制字符串
   DrawUTF(130, 0, "电力系统：    1月5--13:00"); // 绘制字符串

   // EPD.fontscale = 1;                // 字体缩放系数改回1
   //   EPD.DrawXbm_P(60, 0, 12, 12, (uint8_t *)city_icon); //绘制图片
   // printf("缓存图像绘制完毕，准备全刷 \n");
   DrawYline(26, 150, 130);
   EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕
}