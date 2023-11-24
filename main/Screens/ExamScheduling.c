#include "ExamScheduling.h"

   extern uint8_t fontscale;
   extern uint8_t EPDbuffer[400 * 300 / 8];

void ExamSchedue(){
	EPD_init_Full();                       // 全刷初始化，使用全刷波形

   clearbuffer();   // 清空缓存(全白)
   fontscale = 2; // 字体缩放系数(支持1和2,对图片也有效，用完记得重新改成1)
   // SetFont(FONT12);              // 选择字体，具体支持的字体见文档
   DrawUTF(0, 80, "考试安排"); // 绘制字符串
   DrawUTF(26, 0, "电力系统：    2月21--14:00"); // 绘制字符串
   DrawUTF(52, 0, "PLC控制：     2月21--18:00"); // 绘制字符串
   DrawUTF(78, 0, "电力电子：    2月22--14:00"); // 绘制字符串
   DrawUTF(104, 0, "计算机控制： 2月23--09:00"); // 绘制字符串
   DrawUTF(130, 0, "现代控制：    2月26--09:00"); // 绘制字符串

   // EPD.fontscale = 1;                // 字体缩放系数改回1
   //   EPD.DrawXbm_P(60, 0, 12, 12, (uint8_t *)city_icon); //绘制图片
   // printf("缓存图像绘制完毕，准备全刷 \n");
   DrawYline(26, 150, 130);
   EPD_Dis_Full((uint8_t *)EPDbuffer, 1); // 将缓存中的图像传给屏幕控制芯片全刷屏幕
   vTaskDelay(1000 / portTICK_PERIOD_MS);

   // deepsleep(); // 让屏幕进入休眠模式
}

