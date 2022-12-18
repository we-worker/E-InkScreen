#include "EPaperDrive.h"
#include "string.h"
#include "WAVEFORM_SETTING_LUT.h"
// #include "EPD_drive_gpio.h"

uint8_t UNICODEbuffer[200];
uint8_t EPDbuffer[400 * 300 / 8] = {0};
char fontname[] = "/littlefs/font12";

gpio_num_t CS = GPIO_NUM_5;
gpio_num_t RST = GPIO_NUM_7;
gpio_num_t DC = GPIO_NUM_6;
gpio_num_t BUSY_line = GPIO_NUM_8;
gpio_num_t CLK = GPIO_NUM_4;
gpio_num_t DIN = GPIO_NUM_10;

uint8_t fontscale = 1;
uint8_t fontwidth = 12;
uint8_t fontheight = 12;

int xDot = 400;
int yDot = 300;

int EPD_Type = 6;

int WX29 = 0; // SSD1608
int WF29 = 1;
int OPM42 = 2; // SSD1619
int WF58 = 3;
int WF29BZ03 = 4;
int C154 = 5;
int DKE42_3COLOR = 6; // SSD1619
int DKE29_3COLOR = 7; // SSD1680
int WF42 = 8;
int WF32 = 9;
int WFT0290CZ10 = 10;    // UC8151C
int GDEY042Z98 = 11;     // SSD1683
int HINKE0266A15A0 = 12; // SSD1675

void EPaperDrive(gpio_num_t CS, gpio_num_t RST, gpio_num_t DC, gpio_num_t BUSY, gpio_num_t CLK, gpio_num_t DIN)
{

    gpio_config_t io_conf_output = {};
    io_conf_output.intr_type = GPIO_INTR_DISABLE,                                                   // 不启用gpio中断
        io_conf_output.mode = GPIO_MODE_OUTPUT,                                                     // 推挽输出模式
        io_conf_output.pin_bit_mask = BIT64(CS) | BIT64(DC) | BIT64(RST) | BIT64(CLK) | BIT64(DIN), // 设置goio，可以同时设置多个
        io_conf_output.pull_down_en = GPIO_PULLDOWN_DISABLE,                                        // 不下拉
        io_conf_output.pull_up_en = GPIO_PULLUP_DISABLE,                                            // 不上拉
        gpio_config(&io_conf_output);

    io_conf_output.intr_type = GPIO_INTR_DISABLE,            // 不启用gpio中断
        io_conf_output.mode = GPIO_MODE_INPUT,               // 推挽输出模式
        io_conf_output.pin_bit_mask = BIT64(BUSY),           // 设置goio，可以同时设置多个
        io_conf_output.pull_down_en = GPIO_PULLDOWN_DISABLE, // 不下拉
        io_conf_output.pull_up_en = GPIO_PULLUP_DISABLE,     // 不上拉
        gpio_config(&io_conf_output);

    gpio_set_level(RST, 1);

    printf("GPIO_init\n");
}

void driver_delay_xms(unsigned long xms)
{
    vTaskDelay(1 / portTICK_PERIOD_MS);
}

void SPI_Write(uint8_t value)
{

    EPD_CLK_0;
    vTaskDelay(1 / portTICK_PERIOD_MS);
    for (int i = 0; i < 8; i++)
    {
        // 高位在前发送方式    根据升级器件特性定
        if ((value & 0x80) == 0x80)
            EPD_DIN_1;
        else
            EPD_DIN_0;
        // vTaskDelay(1 / portTICK_PERIOD_MS); //等待数据稳定  根据实际时钟调整
        EPD_CLK_1; // 上升沿发送数据
        // vTaskDelay(1 / portTICK_PERIOD_MS);//CLK高电平保持一段时间 这个可以不需要 根据具体的spi时钟来确定
        EPD_CLK_0;          // 把时钟拉低实现为下一次上升沿发送数据做准备
        value = value << 1; // 发送数据的位向前移动一位
    }
}

/*
void DrawCircle(int x, int y, int r, bool fill)
{
    if (fill == 0)
    {
        for (int i = 0; i < 360; i++)
        {
            SetPixel(round(cos(i) * r + x), round(sin(i) * r) + y);
        }
    }
    else
    {
        for (int j = 0; j < r; j++)
        {

            for (int i = 0; i < 360; i++)
            {
                SetPixel(std::round(cos(i) * j + x), round(sin(i) * j) + y);
            }
        }
    }
}
*/

void DrawBox(uint8_t x, int y, int w, int h)
{

    for (int i = x; i < x + h; i++)
    {
        DrawXline(y, y + w - 1, i);
    }
}
void DrawEmptyBox(int x, int y, int w, int h)
{
    DrawXline(y, y + w, x);
    DrawXline(y, y + w, x + h);
    DrawYline(x, x + h, y);
    DrawYline(x, x + h, y + w);
}

int getIcon(int weathercodeindex)
{
    if (weathercodeindex == 0)
        return 12; // 晴
    if (weathercodeindex == 1)
        return 58; // 多云
    if (weathercodeindex == 2)
        return 58; // 少云
    if (weathercodeindex == 3)
        return 58; // 晴间多云
    if (weathercodeindex == 4)
        return 54; // 阴
    if (weathercodeindex >= 5 && weathercodeindex <= 18)
        return 0;
    if (weathercodeindex >= 19 && weathercodeindex <= 32)
        return 19; // 雨
    if (weathercodeindex >= 33 && weathercodeindex <= 36)
        return 16; // 雪
    if (weathercodeindex >= 37 && weathercodeindex <= 40)
        return 16; // 雪(雨夹雪)
    if (weathercodeindex == 41)
        return 37; // 雾(薄雾)
    if (weathercodeindex == 42)
        return 37; // 雾
    if (weathercodeindex == 43)
        return 37; // 雾(霾)
    return 17;     // 阵雨夹雪
}
/*
void DrawWeatherChart(int xmin, int xmax, int ymin, int ymax, int point_n, int show_n, char * tmax, char * tmin, char * code_d, char * code_n, char * text_d, char * text_n, char * date, char * week) //绘制天气温度变化曲线
{
    if (tmax == ",,,,,")
    {
        tmax = "5,5,5,5,5,5";
        tmin = "2,2,2,2,2,2";
        code_d = "0,0,0,0,0,0";
        code_n = "0,0,0,0,0,0";
        text_n = "晴,晴,晴,晴,晴,晴";
        text_d = "晴,晴,晴,晴,晴,晴";
        date = "1-1,1-2,1-3,1-4,1-5,1-6";
        week = "1,1,1,1,1,1";
    }
    char * code_d_a[point_n], code_n_a[point_n], text_d_a[point_n], text_n_a[point_n], date_a[point_n], week_a[point_n];
    int tmax_a[point_n], tmin_a[point_n];
    int min_data = 999, max_data = -999;
    int tmin_x_cord[point_n], tmax_x_cord[point_n]; //将数值转成屏幕坐标

    char * temp_min[point_n];
    char * temp_max[point_n];
    int j = 0, k = 0, l = 0;
    //分割tmax和tmin
    // Serial.println(tmax); Serial.println(tmin);
    // Serial.println(code_d);
    for (int i = 0; i < tmin.length(); i++)
    {
        temp_min[j] += tmin[i];

        if (tmin.charAt(i) == char(','))
            j++;
    }
    for (int i = 0; i < tmax.length(); i++)
    {

        temp_max[k] += tmax[i];

        if (tmax.charAt(i) == char(','))
            k++;
    }
    j = 0;
    //分割code_d
    for (int i = 0; i < code_d.length(); i++)
    {
        code_d_a[j] += code_d[i];
        if (code_d.charAt(i) == char(','))
            j++;
    }
    //分割code_n
    j = 0;
    for (int i = 0; i < code_n.length(); i++)
    {
        code_n_a[j] += code_n[i];
        if (code_n.charAt(i) == char(','))
            j++;
    }
    //分割text_d
    j = 0;
    for (int i = 0; i < text_d.length(); i++)
    {
        if (text_d.charAt(i) == char(','))
            j++;
        else
            text_d_a[j] += text_d[i];
    }
    //分割text_n
    j = 0;
    for (int i = 0; i < text_n.length(); i++)
    {
        if (text_n.charAt(i) == char(','))
            j++;
        else
            text_n_a[j] += text_n[i];
    }
    //分割week_n
    j = 0;
    for (int i = 0; i < week.length(); i++)
    {
        if (week.charAt(i) == char(','))
            j++;
        else
            week_a[j] += week[i];
    }
    j = 0;
    for (int i = 0; i < date.length(); i++)
    {
        if (date.charAt(i) == char(','))
            j++;
        else
            date_a[j] += date[i];
    }
    for (int i = 0; i < point_n; i++)
    {
        tmax_a[i] = temp_max[i].toInt(); // Serial.printf("max:%d\n",tmax_a[i]);
        tmin_a[i] = temp_min[i].toInt(); // Serial.printf("min:%d\n",tmin_a[i]);
    }
    //找出计算最大最小值
    for (int i = 0; i < show_n; i++)
    {
        if (tmax_a[i] > max_data)
            max_data = tmax_a[i];
        if (tmax_a[i] < min_data)
            min_data = tmax_a[i];
        if (tmin_a[i] > max_data)
            max_data = tmin_a[i];
        if (tmin_a[i] < min_data)
            min_data = tmin_a[i];
    }
    //转换坐标
    if ((max_data - min_data) != 0)
    {
        for (int i = 0; i < show_n; i++)
        {
            tmin_x_cord[i] = xmax - ((xmax - xmin) * (tmin_a[i] - min_data) / (max_data - min_data));
            tmax_x_cord[i] = xmax - ((xmax - xmin) * (tmax_a[i] - min_data) / (max_data - min_data));
        }
    }
    int dy = (ymax - ymin) / (show_n - 1);


    //画折线
    for (int i = 0; i < show_n - 1; i++)
    {
        DrawLine(tmin_x_cord[i], ymin + dy * i, tmin_x_cord[i + 1], ymin + dy * (i + 1));
        DrawLine(tmax_x_cord[i], ymin + dy * i, tmax_x_cord[i + 1], ymin + dy * (i + 1));

        // DrawLine(tmin_x_cord[i]-1,ymin+dy*i,tmin_x_cord[i+1]-1,ymin+dy*(i+1));
        // DrawLine(tmax_x_cord[i]-1,ymin+dy*i,tmax_x_cord[i+1]-1,ymin+dy*(i+1));
    }
    //画圆圈，添加标注
    for (int i = 0; i < show_n; i++)
    {
        DrawCircle(tmin_x_cord[i], ymin + dy * i, 3, 1);
        DrawCircle(tmax_x_cord[i], ymin + dy * i, 3, 1);
        SetFont(FONT12);
        fontscale = 1;

        DrawUTF(tmax_x_cord[i] - 14, ymin + dy * i - 3 * (char *(tmax_a[i]).length() + 1), char *(tmax_a[i]) + "°");
        DrawUTF(tmin_x_cord[i] + 2, ymin + dy * i - 3 * (char *(tmax_a[i]).length() + 1), char *(tmin_a[i]) + "°");

        if (EPD_Type != WF32)
        {
            SetFont(ICON32);
            DrawUTF(xmin - 44, ymin + dy * i - 16, char *(char(getIcon(code_d_a[i].toInt()))));
            SetFont(FONT12);
            DrawUTF(xmin - 56, ymin + dy * i - 2 * text_d_a[i].length(), text_d_a[i]);
            DrawUTF(xmin - 82, ymin + dy * i - 2 * week_a[i].length(), week_a[i]);

            SetFont(ICON32);
            DrawUTF(xmax + 14, ymin + dy * i - 16, char *(char(getIcon(code_n_a[i].toInt()))));
            SetFont(FONT12);
            DrawUTF(xmax + 46, ymin + dy * i - 2 * text_n_a[i].length(), text_n_a[i]);
        }
        else
        {
            // SetFont(ICON32);DrawUTF(xmin-35,ymin+dy*i-16,char *(char(getIcon(code_d_a[i].toInt()))));
            SetFont(FONT12);
            DrawUTF(xmax + 14, ymin + dy * i - 2 * text_d_a[i].length(), text_d_a[i]);
            SetFont(ICON32);
            DrawUTF(xmax + 26, ymin + dy * i - 16, char *(char(getIcon(code_d_a[i].toInt()))));
            SetFont(FONT12);
            DrawUTF(xmin - 28, ymin + dy * i - 2 * week_a[i].length(), week_a[i]);
        }

        // Serial.print("code_d:");
        // Serial.println(code_d_a[i]);
        // Serial.print("txt_n:");
        // Serial.println(text_n_a[i]);
    }
    if (EPD_Type != WF32)
        Inverse(xmin - 83, xmin - 69, 0, 400);
}
*/
void DrawXline(int start, int end, int x)
{
    for (int i = start; i <= end; i++)
    {
        SetPixel(x, i);
    }
}
void DrawYline(int start, int end, int y)
{
    for (int i = start; i <= end; i++)
    {
        SetPixel(i, y);
    }
}
/*
void DrawLine(int xstart, int ystart, int xend, int yend)
{

    int dx = abs(xend - xstart);
    int dy = abs(yend - ystart);
    if (dx >= dy)
    {
        if (xend > xstart)
        {
            for (int i = xstart; i <= xend; i++)
            {
                SetPixel(i, round((i - xstart) * (yend - ystart) / (xend - xstart)) + ystart);
            }
        }
        else
        {
            for (int i = xstart; i >= xend; i--)
            {
                SetPixel(i, round((i - xstart) * (yend - ystart) / (xend - xstart)) + ystart);
            }
        }
    }

    else
    {
        if (yend > ystart)
        {
            for (int i = ystart; i <= yend; i++)
            {
                SetPixel(round((i - ystart) * (xend - xstart) / (yend - ystart)) + xstart, i);
            }
        }
        else
        {
            for (int i = ystart; i >= yend; i--)
            {
                SetPixel(round((i - ystart) * (xend - xstart) / (yend - ystart)) + xstart, i);
            }
        }
    }
}
*/

void Inverse(int xStart, int xEnd, int yStart, int yEnd)
{
    for (int i = 0; i < (xEnd - xStart); i++)
    {
        for (int j = 0; j < (yEnd - yStart); j++)
        {
            InversePixel(xStart + i, yStart + j);
        }
    }
}

void DrawUTF_wh(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *code)
{
    int charcount;
    charcount = UTFtoUNICODE((uint8_t *)code);
    DrawUnicodeStr(x, y, width, height, charcount, (uint8_t *)UNICODEbuffer);
}
void DrawUTF(int16_t x, int16_t y, char *code)
{
    // char * buffer=code;
    // code.toCharArray(buffer, 200);
    DrawUTF_wh(x, y, fontwidth, fontheight, (uint8_t *)code);
}
int UTFtoUNICODE(uint8_t *code)
{
    int i = 0;
    int charcount = 0;
    while (code[i] != '\0')
    {
        // Serial.println("current codei");
        // Serial.println(code[i],HEX);
        //   Serial.println(code[i]&0xf0,HEX);
        if (code[i] <= 0x7f) // ascii
        {

            UNICODEbuffer[charcount * 2] = 0x00;
            UNICODEbuffer[charcount * 2 + 1] = code[i];
            // Serial.println("english or number");
            // Serial.println(UNICODEbuffer[charcount*2],HEX);
            //  Serial.println(UNICODEbuffer[charcount*2+1],HEX);
            i++;
            charcount++;
        }
        else if ((code[i] & 0xe0) == 0xc0)
        {

            UNICODEbuffer[charcount * 2 + 1] = (code[i] << 6) + (code[i + 1] & 0x3f);
            UNICODEbuffer[charcount * 2] = (code[i] >> 2) & 0x07;
            i += 2;
            charcount++;
            // Serial.println("two bits utf-8");
        }
        else if ((code[i] & 0xf0) == 0xe0)
        {

            UNICODEbuffer[charcount * 2 + 1] = (code[i + 1] << 6) + (code[i + 2] & 0x7f);
            UNICODEbuffer[charcount * 2] = (code[i] << 4) + ((code[i + 1] >> 2) & 0x0f);

            // Serial.println("three bits utf-8");
            //  Serial.println(UNICODEbuffer[charcount*2],HEX);
            //  Serial.println(UNICODEbuffer[charcount*2+1],HEX);
            i += 3;
            charcount++;
        }
        else
        {
            return 0;
        }
    }
    UNICODEbuffer[charcount * 2] = '\0';
    return charcount;
}
void DrawUnicodeChar(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *code)
{

    int offset;
    int sizeofsinglechar;
    if (height % 8 == 0)
        sizeofsinglechar = (height / 8) * width;
    else
        sizeofsinglechar = (height / 8 + 1) * width;
    offset = (code[0] * 0x100 + code[1]) * sizeofsinglechar;
    // TODO
    FILE *f = fopen(fontname, "r");
    if (f == NULL)
    {
        printf("Failed to open file for writing\n");
        return;
    }
    fseek(f, offset, SEEK_SET);
    char zi[sizeofsinglechar];
    fread(zi, sizeofsinglechar, 1, f);
    fclose(f);
    // printf("read_over\n");
    // printf("zi 偏移 %d,为%d\n", offset, zi[sizeofsinglechar]);

    if (offset < 0xff * sizeofsinglechar && FontIndex < 10)
    {
        drawXbm(x, y, width, height, (uint8_t *)zi);
    }
    else
    {
        drawXbm(x, y, width, height, (uint8_t *)zi);
    }

    // SPIFFS.end();
}

void DrawUnicodeStr(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t strlength, uint8_t *code)
{
    int ymax = yDot;

    ymax = xDot;
    // printf("start_Draw_Unicode_str\n");

    CurrentCursor = 0;
    uint8_t sizeofsinglechar;
    if (height % 8 == 0)
        sizeofsinglechar = (height / 8) * width;
    else
        sizeofsinglechar = (height / 8 + 1) * width;
    int ymove = 0;
    int xmove = 0;
    strlength *= 2;
    int i = 0;
    while (i < strlength)
    {
        int offset;
        offset = (code[i] * 0x100 + code[i + 1]) * sizeofsinglechar;
        if (offset < 0xff * sizeofsinglechar && fontscale == 1)
        {

            DrawUnicodeChar(x + xmove, y + ymove, width, height, (uint8_t *)code + i);
            ymove += CurrentCursor + 1;
            if ((y + ymove + width / 2) >= ymax - 1)
            {
                xmove += height + 1;
                ymove = 0;
                CurrentCursor = 0;
            }
        }
        else if (offset < 0xff * sizeofsinglechar && fontscale == 2)
        {
            DrawUnicodeChar(x + xmove, y + ymove, width, height, (uint8_t *)code + i);
            ymove += CurrentCursor + 2;
            if ((y + ymove + width) >= ymax - 1)
            {
                xmove += height + 1;
                ymove = 0;
                CurrentCursor = 0;
            }
        }
        else if (fontscale == 2)
        {
            DrawUnicodeChar(x + xmove, y + ymove, width, height, (uint8_t *)code + i);
            ymove += width * 2;
            if ((y + ymove + width * 2) >= ymax - 1)
            {
                xmove += height * 2 + 2;
                ymove = 0;
                CurrentCursor = 0;
            }
        }
        else
        {
            DrawUnicodeChar(x + xmove, y + ymove, width, height, (uint8_t *)code + i);
            ymove += width;
            if ((y + ymove + width) >= ymax - 1)
            {
                xmove += height + 1;
                ymove = 0;
                CurrentCursor = 0;
            }
        }
        i++;
        i++;
    }
}
void drawXbm(int16_t xMove, int16_t yMove, int16_t width, int16_t height, uint8_t *xbm)
{
    // TODO
    int16_t heightInXbm = (height + 7) / 8;
    uint8_t Data = 0;
    for (int16_t x = 0; x < width; x++)
    {
        for (int16_t y = 0; y < height; y++)
        {
            if (y & 7)
            {
                Data <<= 1; // Move a bit
            }
            else
            { // Read new Data every 8 bit
                Data = xbm[(y / 8) + x * heightInXbm];
            }
            // if there is a bit draw it
            if (((Data & 0x80) >> 7))
            {
                if (fontscale == 1)
                {
                    SetPixel(xMove + y, yMove + x);
                    // printf("写字开始\n");
                    CurrentCursor = x;
                }
                if (fontscale == 2)
                {
                    SetPixel(xMove + y * 2, yMove + x * 2);
                    SetPixel(xMove + y * 2 + 1, yMove + x * 2);
                    SetPixel(xMove + y * 2, yMove + x * 2 + 1);
                    SetPixel(xMove + y * 2 + 1, yMove + x * 2 + 1);
                    CurrentCursor = x * 2;
                }
                // if(fontscale==2) {SetPixel(xMove + y*2, yMove + x*2);CurrentCursor=x*2;}
            }
        }
    }
}

void DrawXbm_P(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const uint8_t *xbm)
{
    int16_t heightInXbm = (height + 7) / 8;
    uint8_t Data = 0;
    // uint8_t temp[heightInXbm*width];
    // memcpy_P(temp, xbm, heightInXbm*width);

    for (int16_t x = 0; x < width; x++)
    {
        for (int16_t y = 0; y < height; y++)
        {
            if (y & 7)
            {
                Data <<= 1; // Move a bit
            }
            else
            { // Read new Data every 8 bit
                Data = *((char *)(xbm + (y / 8) + x * heightInXbm));
            }
            // if there is a bit draw it
            if (((Data & 0x80) >> 7))
            {
                if (fontscale == 1)
                {
                    SetPixel(xMove + y, yMove + x);
                    CurrentCursor = x;
                }
                if (fontscale == 2)
                {
                    SetPixel(xMove + y * 2, yMove + x * 2);
                    SetPixel(xMove + y * 2 + 1, yMove + x * 2);
                    SetPixel(xMove + y * 2, yMove + x * 2 + 1);
                    SetPixel(xMove + y * 2 + 1, yMove + x * 2 + 1);
                }
            }
        }
    }
}
void DrawXbm_p_gray(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const uint8_t *xbm, uint8_t level)
{
    int16_t heightInXbm = (height + 1) / 2;
    uint8_t Data = 0;

    for (int16_t x = 0; x < width; x++)
    {
        for (int16_t y = 0; y < height; y++)
        {
            if (y % 2 != 0)
            {
                Data <<= 4; // Move a bit
            }
            else
            { // Read new Data every 8 bit
                Data = *((char *)(xbm + (y / 2) + x * heightInXbm));
            }
            // if there is a bit draw it
            if (((Data & 0xf0) >> 4 == level))
            {
                SetPixel(xMove + y, yMove + x);
                CurrentCursor = x;
            }
        }
    }
}
void DrawXbm_spiff_gray(int16_t xMove, int16_t yMove, int16_t width, int16_t height, uint8_t level)
{

    FILE *f = fopen("/pic.xbm", "r");

    int16_t heightInXbm = (height + 1) / 2;
    uint8_t Data = 0;

    for (int16_t x = 0; x < width; x++)
    {
        for (int16_t y = 0; y < height; y++)
        {
            if (y % 2 != 0)
            {
                Data <<= 4; // Move a bit
            }
            else
            { // Read new Data every 8 bit
                Data = ~fgetc(f);
            }
            // if there is a bit draw it
            if (((Data & 0xf0) >> 4 == level))
            {
                SetPixel(xMove + y, yMove + x);
                CurrentCursor = x;
            }
        }
    }
    fclose(f);
}
void SetPixel(int16_t x, int16_t y)
{

    int16_t temp = x;
    x = y;
    y = yDot - 1 - temp;
    if (x < xDot && y < yDot)
        EPDbuffer[x / 8 + y * xDot / 8] &= ~(0x80 >> x % 8);
}
void InversePixel(int16_t x, int16_t y)
{

    int16_t temp = x;
    x = y;
    y = yDot - 1 - temp;
    if (x < xDot && y < yDot)
        EPDbuffer[x / 8 + y * xDot / 8] ^= (0x80 >> x % 8);
}
void clearbuffer()
{

    if (EPD_Type == WF29BZ03)
    {
        for (int i = 0; i < (xDot * yDot / 4); i++)
            EPDbuffer[i] = 0xff;
    }
    else
        for (int i = 0; i < (xDot * yDot / 8); i++)
            EPDbuffer[i] = 0xff;
}

bool ReadBusy(void)
{
    while(1)
    {

        if (READ_EPD_BUSY == 0)
        {
            // Serial.println("Busy is Low \r\n");
            return 1;
        }
        
        vTaskDelay(2 / portTICK_PERIOD_MS);
        // Serial.println("epd is Busy");
    }
    return 0;
}
bool ReadBusy_long(void)
{
    unsigned long i = 0;
    for (i = 0; i < 2000; i++)
    {
        //  println("isEPD_BUSY = %d\r\n",isEPD_CS);
        if (EPD_Type == WX29 || EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
        {
            if (READ_EPD_BUSY == 0)
            {
                // Serial.println("Busy is Low \r\n");
                return 1;
            }
        }
        if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF29BZ03 || EPD_Type == C154 || EPD_Type == WF42 || EPD_Type == WFT0290CZ10)
        {
            if (READ_EPD_BUSY != 0)
            {
                // Serial.println("Busy is H \r\n");
                return 1;
            }
        }
        vTaskDelay(2 / portTICK_PERIOD_MS);
        // Serial.println("epd is Busy");
    }
    return 0;
}
void EPD_WriteCMD(uint8_t command)
{
    /*if(EPD_Type==WX29||EPD_Type==OPM42||EPD_Type==DKE42_3COLOR||EPD_Type==DKE29_3COLOR)
  {
  ReadBusy();
  }   */
    ReadBusy();
    EPD_CS_0;
    EPD_DC_0; // command write
    SPI_Write(command);
    EPD_CS_1;
}
void EPD_WriteData(uint8_t Data)
{

    EPD_CS_0;
    EPD_DC_1;
    SPI_Write(Data);
    EPD_CS_1;
}

void EPD_WriteCMD_p1(uint8_t command, uint8_t para)
{
    /*if(EPD_Type==WX29||EPD_Type==OPM42||EPD_Type==DKE42_3COLOR||EPD_Type==DKE29_3COLOR)
 {
  ReadBusy();
 }*/
    // ReadBusy();
    EPD_CS_0;
    EPD_DC_0; // command write
    SPI_Write(command);
    EPD_DC_1; // command write
    SPI_Write(para);
    EPD_CS_1;
}

void deepsleep(void)
{
    if (EPD_Type == WX29 || EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
    {
        ReadBusy_long();
        EPD_WriteCMD_p1(0x10, 0x01);
        // EPD_WriteCMD_p1(0x22,0xc0);//power off
        // EPD_WriteCMD(0x20);
    }
    if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF29BZ03 || EPD_Type == C154 || EPD_Type == WF42 || EPD_Type == WFT0290CZ10)
    {
        ReadBusy_long();
        EPD_WriteCMD(0x50);
        EPD_WriteData(0xf7); // border floating
        EPD_WriteCMD(0x02);  // power off
        // ReadBusy();
        EPD_WriteCMD(0x07); // sleep
        EPD_WriteData(0xa5);
    }
}

void EPD_Write(uint8_t *value, uint8_t Datalen)
{

    uint8_t i = 0;
    uint8_t *ptemp;
    ptemp = value;

    ReadBusy();

    EPD_CS_0;
    EPD_DC_0; // When DC is 0, write command

    SPI_Write(*ptemp); // The first uint8_t is written with the command value
    ptemp++;
    EPD_DC_1; // When DC is 1, write Data
    for (i = 0; i < Datalen - 1; i++)
    { // sub the Data
        SPI_Write(*ptemp);
        ptemp++;
    }
    ReadBusy();
    EPD_CS_1;
}

void EPD_WriteDispRam(unsigned int XSize, unsigned int YSize, uint8_t *Dispbuff, unsigned int offset, uint8_t label)
{

    int i = 0, j = 0;

        ReadBusy();
        EPD_DC_0; // command write
        EPD_CS_0;
        SPI_Write(0x24);

    EPD_DC_1; // Data write

    if (label != 1)
    {
        for (i = 0; i < YSize; i++)
        {
            for (j = 0; j < XSize; j++)
            {
                SPI_Write(label);
            }
        }
    }
    else
    {
        Dispbuff += offset;
        for (i = 0; i < YSize; i++)
        {
            for (j = 0; j < XSize; j++)
            {
                SPI_Write(*Dispbuff);
                Dispbuff++;
            }
            Dispbuff += xDot / 8 - XSize;
        }
    }

    EPD_CS_1;
}
void EPD_WriteDispRam_RED(unsigned int XSize, unsigned int YSize, uint8_t *Dispbuff, unsigned int offset, uint8_t label)
{

    int i = 0, j = 0;
    if (EPD_Type == WF29BZ03)
    {
        EPD_WriteCMD(0x10);
        EPD_CS_0;
        EPD_DC_1;
        for (i = 0; i < (YSize * 2); i++)
        {
            for (j = 0; j < (XSize * 2); j++)
            {
                SPI_Write(*Dispbuff);
                Dispbuff++;
            }
        }
        EPD_CS_1;
        return;
    }

    if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF42)
    {
        EPD_WriteCMD(0x10);
        EPD_CS_0;
        EPD_DC_1;
        for (i = 0; i < YSize; i++)
        {
            for (j = 0; j < XSize; j++)
            {
                SPI_Write(0x00);
            }
        }
        EPD_CS_1;
    }
    if (EPD_Type == WX29 || EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
    {
        ReadBusy();
        EPD_DC_0; // command write
        EPD_CS_0;
        SPI_Write(0x26);
    }

    if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF42)
    {
        EPD_CS_0;
        EPD_DC_0; // command write
        SPI_Write(0x13);
    }
    EPD_DC_1; // Data write
    // Serial.printf("Xsize=%dYsize=%d\r\n",XSize,YSize);

    if (label != 1)
    {
        for (i = 0; i < YSize; i++)
        {
            for (j = 0; j < XSize; j++)
            {
                SPI_Write(label);
            }
        }
    }
    else
    {
        Dispbuff += offset;
        for (i = 0; i < YSize; i++)
        {
            for (j = 0; j < XSize; j++)
            {
                SPI_Write(~*Dispbuff);
                Dispbuff++;
            }
            Dispbuff += xDot / 8 - XSize;
        }
    }

    EPD_CS_1;
}
void EPD_WriteDispRam_Old(unsigned int XSize, unsigned int YSize, uint8_t *Dispbuff, unsigned int offset, uint8_t label)
{

    int i = 0, j = 0;
    if (EPD_Type == WF29BZ03)
    {
        EPD_WriteCMD(0x10);
        EPD_CS_0;
        EPD_DC_1;
        for (i = 0; i < (YSize * 2); i++)
        {
            for (j = 0; j < (XSize * 2); j++)
            {
                SPI_Write(*Dispbuff);
                Dispbuff++;
            }
        }
        EPD_CS_1;
        return;
    }

    /*if(EPD_Type==WF29||EPD_Type==WF58||EPD_Type==WF42)
  {
      EPD_WriteCMD(0x10);
      EPD_CS_0;
      EPD_DC_1;
     for(i=0;i<YSize;i++){
    for(j=0;j<XSize;j++){
      SPI_Write(0x00);
       }
     }
    EPD_CS_1;
  }*/
    else if (EPD_Type == WX29 || EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
    {
        ReadBusy();
        EPD_DC_0; // command write
        EPD_CS_0;
        SPI_Write(0x26);
    }

    else if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF42 || EPD_Type == WFT0290CZ10)
    {
        ReadBusy();
        EPD_CS_0;
        EPD_DC_0; // command write
        SPI_Write(0x10);
    }
    EPD_DC_1; // Data write
    // Serial.printf("Xsize=%dYsize=%d\r\n",XSize,YSize);

    if (label != 1)
    {
        for (i = 0; i < YSize; i++)
        {
            for (j = 0; j < XSize; j++)
            {
                SPI_Write(label);
            }
        }
    }
    else
    {

        Dispbuff += offset;
        for (i = 0; i < YSize; i++)
        {
            for (j = 0; j < XSize; j++)
            {
                SPI_Write(*Dispbuff);
                Dispbuff++;
            }
            Dispbuff += xDot / 8 - XSize;
        }
    }
    ReadBusy_long();

    EPD_CS_1;
}

void EPD_SetRamArea(uint16_t Xstart, uint16_t Xend,
                    uint8_t Ystart, uint8_t Ystart1, uint8_t Yend, uint8_t Yend1)
{
    if (EPD_Type == WX29 || EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
    {
        uint8_t RamAreaX[3]; // X start and end
        uint8_t RamAreaY[5]; // Y start and end
        RamAreaX[0] = 0x44;  // command
        RamAreaX[1] = Xstart / 8;
        RamAreaX[2] = Xend / 8;
        RamAreaY[0] = 0x45; // command
        RamAreaY[1] = Ystart;
        RamAreaY[2] = Ystart1;
        RamAreaY[3] = Yend;
        RamAreaY[4] = Yend1;
        EPD_Write(RamAreaX, sizeof(RamAreaX));
        EPD_Write(RamAreaY, sizeof(RamAreaY));
        // Serial.printf("set ram area%d %d %d %d %d %d %d %d\n",RamAreaX[0],RamAreaX[1],RamAreaX[2],RamAreaY[0],RamAreaY[1],RamAreaY[2],RamAreaY[3],RamAreaY[4]);
    }
    if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF42 || EPD_Type == WF29BZ03 || EPD_Type == WFT0290CZ10)
    {
        EPD_WriteCMD(0x91); // enter partial refresh mode
        EPD_WriteCMD(0x90);
        if (EPD_Type == WF42)
        {
            EPD_WriteData(Xstart / 256);
            EPD_WriteData(Xstart % 256);
            EPD_WriteData(Xend / 256);
            EPD_WriteData(Xend % 256);
            EPD_WriteData(Ystart);
            EPD_WriteData(Ystart1);
            EPD_WriteData(Yend);
            EPD_WriteData(Yend1);
            EPD_WriteData(0x0);
        }
        /*else if (EPD_Type == WFT0290CZ10)
        {
            EPD_WriteData(Xstart);   // x-start
            EPD_WriteData(Xend - 1); // x-end

            EPD_WriteData(Ystart / 256);
            EPD_WriteData(Ystart % 256); // y-start

            EPD_WriteData(Yend / 256);
            EPD_WriteData(Yend % 256 - 1); // y-end
            EPD_WriteData(0x28);
        }*/
        else
        {
            EPD_WriteData(Xstart);
            EPD_WriteData(Xend);
            EPD_WriteData(Ystart);
            EPD_WriteData(Ystart1);
            EPD_WriteData(Yend);
            EPD_WriteData(Yend1);
            EPD_WriteData(0x0);
        }
    }
}

void EPD_SetRamPointer(uint16_t addrX, uint8_t addrY, uint8_t addrY1)
{
    uint8_t RamPointerX[2]; // default (0,0)
    uint8_t RamPointerY[3];
    // Set RAM X address counter
    RamPointerX[0] = 0x4e;
    RamPointerX[1] = addrX;
    // RamPointerX[1] = 0xff;
    // Set RAM Y address counter
    RamPointerY[0] = 0x4f;
    RamPointerY[1] = addrY;
    RamPointerY[2] = addrY1;
    // RamPointerY[1] = 0x2b;
    // RamPointerY[2] = 0x02;

    EPD_Write(RamPointerX, sizeof(RamPointerX));
    EPD_Write(RamPointerY, sizeof(RamPointerY));

    // Serial.printf("0x4e: %d , 0x4f: %d %d \n",RamPointerX[1],RamPointerY[1],RamPointerY[2]);
}

void EPD_Init(void)
{
    printf("初始化中..type=%d\n", EPD_Type);
    EPD_RST_0;
    driver_delay_xms(10);
    EPD_RST_1;

    printf("初始化DKE42_3COLOR\n");
    EPD_WriteCMD(0x74);  //
    EPD_WriteData(0x54); //
    EPD_WriteCMD(0x7E);  //
    EPD_WriteData(0x3B); //
    EPD_WriteCMD(0x01);  //
    EPD_WriteData(0x2B); //
    EPD_WriteData(0x01);
    EPD_WriteData(0x00); //

    EPD_WriteCMD(0x0C);  //
    EPD_WriteData(0x8B); //
    EPD_WriteData(0x9C); //
    EPD_WriteData(0xD6); //
    EPD_WriteData(0x0F); //

    EPD_WriteCMD(0x3A);  //
    EPD_WriteData(0x21); //
    EPD_WriteCMD(0x3B);  //
    EPD_WriteData(0x06); //
    EPD_WriteCMD(0x3C);  //
    EPD_WriteData(0x03); //

    EPD_WriteCMD(0x11);  // data enter mode
    EPD_WriteData(0x01); // 01 –Y decrement, X increment,

    EPD_WriteCMD(0x2C);  //
    EPD_WriteData(0x00); // fff

    EPD_WriteCMD(0x37);  //
    EPD_WriteData(0x00); //
    EPD_WriteData(0x00); //
    EPD_WriteData(0x00); //
    EPD_WriteData(0x00); //
    EPD_WriteData(0x80); //

    EPD_WriteCMD(0x21);  //
    EPD_WriteData(0x40); //
    EPD_WriteCMD(0x22);
    EPD_WriteData(0xc7); // c5forgraymode//
}
void EPD_Set_Contrast(uint8_t vcom)
{
    if (EPD_Type == OPM42)
    {
        EPD_WriteCMD(0x2C);  //
        EPD_WriteData(vcom); // fff
    }
}

void EPD_Update(void)
{
    if (EPD_Type == OPM42)
    {
        EPD_WriteCMD(0x20);
    }
    if (EPD_Type == DKE42_3COLOR)
    {
        EPD_WriteCMD(0x22); // Display Update Control
        EPD_WriteData(0xC7);
        EPD_WriteCMD(0x20); // Activate Display Update Sequence
    }
    if (EPD_Type == WX29)
    {
        EPD_WriteCMD_p1(0x22, 0x04);
        EPD_WriteCMD(0x20);
    }
    if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF29BZ03 || EPD_Type == C154 || EPD_Type == WF42 || EPD_Type == WFT0290CZ10)
    {
        EPD_WriteCMD(0x12);
        ReadBusy();
    }
    if (EPD_Type == DKE29_3COLOR)
    {
        EPD_WriteCMD(0x22); // Display Update Control
        EPD_WriteData(0xc7);
        EPD_WriteCMD(0x20); // Activate Display Update Sequence
        ReadBusy();
    }
    else if (EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
    {
        EPD_WriteCMD(0x22); // Display Update Control
        EPD_WriteData(0xC7);
        EPD_WriteCMD(0x20); // Activate Display Update Sequence
        ReadBusy();
    }
}
void EPD_Update_Part(void)
{
        EPD_WriteCMD(0x20);
}

void EPD_init_Full(void)
{
    EPD_Init();

    EPD_Write((uint8_t *)LUTDefault_full_opm42, sizeof(LUTDefault_full_opm42));
}

void EPD_init_Part(void)
{

    EPD_Init();
    EPD_WriteCMD(0x21);
    EPD_WriteData(0x00);
    EPD_Write((uint8_t *)LUTDefault_part_dke42, sizeof(LUTDefault_part_dke42));
}

void EPD_Transfer_Full_BW(uint8_t *DisBuffer, uint8_t Label)
{
    if (EPD_Type == WX29 || EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == WFT0290CZ10 || EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
    {
        unsigned int yStart = 0;
        unsigned int yEnd = yDot - 1;
        unsigned int xStart = 0;
        unsigned int xEnd = xDot - 1;
        unsigned long temp = yStart;

        yStart = yDot - 1 - yEnd;
        yEnd = yDot - 1 - temp;
        EPD_SetRamPointer(xStart / 8, yEnd % 256, yEnd / 256);
        EPD_SetRamArea(xStart, xEnd, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);

        if (Label == 2)
        {
            EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0x00); // white
        }
        else if (Label == 3)
        {
            EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0xff); // black
        }
        else if (Label == 4)
        {
            EPD_WriteDispRam_Old(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 1); // black
        }
        else
        {
            EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 1);
        }
        // EPD_Update();
        // ReadBusy_long();
        // EPD_WriteDispRam(xDot/8, yDot, (uint8_t *)DisBuffer,0,1);
    }
    else if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF29BZ03 || EPD_Type == C154 || EPD_Type == WF42 || EPD_Type == WFT0290CZ10)
    {
        if (Label == 2)
        {
            EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0xff); // white
        }
        else if (Label == 3)
        {
            EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0x00); // black
        }
        else if (Label == 4)
        {
            EPD_WriteDispRam_Old(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 1); // black
        }
        else
        {
            EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 1); // white
        }
        // EPD_Update();
    }
}
void EPD_Transfer_Full_RED(uint8_t *DisBuffer, uint8_t Label)
{
    if (EPD_Type == WX29 || EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
    {
        unsigned int yStart = 0;
        unsigned int yEnd = yDot - 1;
        unsigned int xStart = 0;
        unsigned int xEnd = xDot - 1;
        unsigned long temp = yStart;

        yStart = yDot - 1 - yEnd;
        yEnd = yDot - 1 - temp;
        EPD_SetRamPointer(xStart / 8, yEnd % 256, yEnd / 256);
        EPD_SetRamArea(xStart, xEnd, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);

        if (Label == 2)
        {
            EPD_WriteDispRam_RED(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0x00); // white
        }
        else if (Label == 3)
        {
            EPD_WriteDispRam_RED(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0xff); // black
        }
        else
        {
            EPD_WriteDispRam_RED(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 1);
        }
        // EPD_Update();
        // ReadBusy_long();
        // EPD_WriteDispRam_Old(xDot/8, yDot, (uint8_t *)DisBuffer,0,1);
    }

    if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF29BZ03 || EPD_Type == C154 || EPD_Type == WF42 || EPD_Type == WFT0290CZ10)
    {
        if (Label == 2)
        {
            EPD_WriteDispRam_RED(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0xff); // white
        }
        else if (Label == 3)
        {
            EPD_WriteDispRam_RED(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0x00); // black
        }
        else
        {
            EPD_WriteDispRam_RED(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 1); // white
        }
        // EPD_Update();
    }
}
void EPD_Dis_Full(uint8_t *DisBuffer, uint8_t Label)
{
    // int nowtime = millis();

    unsigned int yStart = 0;
    unsigned int yEnd = yDot - 1;
    unsigned int xStart = 0;
    unsigned int xEnd = xDot - 1;
    unsigned long temp = yStart;

    yStart = yDot - 1 - yEnd;
    yEnd = yDot - 1 - temp;

    EPD_SetRamPointer(xStart / 8, yEnd % 256, yEnd / 256);
    EPD_SetRamArea(xStart, xEnd, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);

    if (Label == 2)
    {
        EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0x00); // white
    }
    else if (Label == 3)
    {
        EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 0xff); // black
    }
    else
    {
        EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 1);
    }
    // nowtime = millis() - nowtime;
    EPD_Update();
    // nowtime = millis() - nowtime;

    // int updatatime = nowtime;
    // Serial.printf("开始全刷 \n");
    ReadBusy_long();
    ReadBusy_long();
    ReadBusy_long();
    ReadBusy_long();

    EPD_SetRamPointer(xStart / 8, yEnd % 256, yEnd / 256);
    EPD_WriteDispRam(xDot / 8, yDot, (uint8_t *)DisBuffer, 0, 1);
}

void EPD_Dis_Part(int xStart, int xEnd, int yStart, int yEnd, uint8_t Label)
{

    int temp1 = xStart, temp2 = xEnd;
    xStart = yStart;
    xEnd = yEnd;
    yEnd = yDot - temp1 - 2;
    yStart = yDot - temp2 - 3;

    unsigned int Xsize = xEnd - xStart;
    unsigned int Ysize = yEnd - yStart + 1;
    if (Xsize % 8 != 0)
    {
        Xsize = Xsize + (8 - Xsize % 8);
    }
    Xsize = Xsize / 8;
    unsigned int offset = yStart * xDot / 8 + xStart / 8;

    unsigned long temp = yStart;
    yStart = yDot - 1 - yEnd;
    yEnd = yDot - 1 - temp;

    printf("Xsize:%d,Ysize:%d\n",Xsize,Ysize);
    EPD_SetRamArea(xStart, xEnd, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);
    EPD_SetRamPointer(xStart / 8, yEnd % 256, yEnd / 256);
    if (Label == 2)
        EPD_WriteDispRam(xEnd - xStart, yEnd - yStart + 1, (uint8_t *)EPDbuffer, offset, 0x00);
    else if (Label == 3)
        EPD_WriteDispRam(xEnd - xStart, yEnd - yStart + 1, (uint8_t *)EPDbuffer, offset, 0xff);
    else
        EPD_WriteDispRam(Xsize, Ysize, (uint8_t *)EPDbuffer, offset, 1);

    EPD_Update_Part();
    ReadBusy_long();
    ReadBusy_long();
    ReadBusy_long();
    ReadBusy_long();

    

    EPD_WriteDispRam(Xsize, Ysize, (uint8_t *)EPDbuffer, offset, 1);
}
void EPD_Transfer_Part(int xStart, int xEnd, int yStart, int yEnd, uint8_t *DisBuffer, uint8_t Label)
{
    if (EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == GDEY042Z98)
    {
        int temp1 = xStart, temp2 = xEnd;
        xStart = yStart;
        xEnd = yEnd;
        yEnd = yDot - temp1 - 2;
        yStart = yDot - temp2 - 3;
    }
    unsigned int Xsize = xEnd - xStart;
    unsigned int Ysize = yEnd - yStart + 1;
    if (Xsize % 8 != 0)
    {
        Xsize = Xsize + (8 - Xsize % 8);
    }
    Xsize = Xsize / 8;
    unsigned int offset = yStart * xDot / 8 + xStart / 8;
    if (EPD_Type == WX29 || EPD_Type == OPM42 || EPD_Type == DKE42_3COLOR || EPD_Type == DKE29_3COLOR || EPD_Type == GDEY042Z98 || EPD_Type == HINKE0266A15A0)
    {

        unsigned long temp = yStart;
        yStart = yDot - 1 - yEnd;
        yEnd = yDot - 1 - temp;

        EPD_SetRamArea(xStart, xEnd, yEnd % 256, yEnd / 256, yStart % 256, yStart / 256);
        EPD_SetRamPointer(xStart / 8, yEnd % 256, yEnd / 256);
        if (Label == 2)
            EPD_WriteDispRam(xEnd - xStart, yEnd - yStart + 1, (uint8_t *)DisBuffer, offset, 0x00);
        else if (Label == 3)
            EPD_WriteDispRam(xEnd - xStart, yEnd - yStart + 1, (uint8_t *)DisBuffer, offset, 0xff);
        else
            EPD_WriteDispRam(Xsize, Ysize, (uint8_t *)DisBuffer, offset, 1);
        // EPD_Update_Part();
        ReadBusy();
        // EPD_WriteDispRam_Old(Xsize, Ysize,(uint8_t *)DisBuffer,offset,1);
        // ReadBusy();
        // EPD_SetRamArea(xStart,xEnd,yEnd%256,yEnd/256,yStart%256,yStart/256);
        // EPD_SetRamPointer(xStart/8,yEnd%256,yEnd/256);
        // EPD_WriteDispRam(Xsize, Ysize,(uint8_t *)DisBuffer,offset,1);
    }

    else if (EPD_Type == WF29 || EPD_Type == WF58 || EPD_Type == WF42 || EPD_Type == WF29BZ03 || EPD_Type == WFT0290CZ10)
    {

        EPD_SetRamArea(xStart, xEnd, yStart / 256, yStart % 256, yEnd / 256, yEnd % 256);
        if (Label == 2)
            EPD_WriteDispRam(xEnd - xStart, yEnd - yStart + 1, (uint8_t *)DisBuffer, offset, 0xff);
        else if (Label == 3)
            EPD_WriteDispRam(xEnd - xStart, yEnd - yStart + 1, (uint8_t *)DisBuffer, offset, 0x00);
        else
            EPD_WriteDispRam(Xsize, Ysize, (uint8_t *)DisBuffer, offset, 1);
        // EPD_Update_Part();
    }
}
