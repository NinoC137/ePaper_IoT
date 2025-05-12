// #include "GUI_Driver.h"

// #include <stdarg.h>

// extern SemaphoreHandle_t GUILog_Mutex;

// TFT_eSPI tft = TFT_eSPI();

// // The scrolling area must be a integral multiple of TEXT_HEIGHT
// #define TEXT_HEIGHT 16          // Height of text to be printed and scrolled
// #define BOT_FIXED_AREA 0        // Number of lines in bottom fixed area (lines counted from bottom of screen)
// #define TOP_FIXED_AREA 240 + 16 // Number of lines in top fixed area (lines counted from top of screen)
// #define YMAX 480                // Bottom of screen area

// // The initial y coordinate of the top of the scrolling area
// uint16_t yStart = TOP_FIXED_AREA;
// // yArea must be a integral multiple of TEXT_HEIGHT
// uint16_t yArea = YMAX - TOP_FIXED_AREA - BOT_FIXED_AREA;
// // The initial y coordinate of the top of the bottom text line
// uint16_t yDraw = YMAX - BOT_FIXED_AREA - TEXT_HEIGHT;

// // Keep track of the drawing x coordinate
// uint16_t xPos = 0;

// // For the byte we read from the serial port
// byte data = 0;

// // A few test variables used during debugging
// bool change_colour = 1;
// bool selected = 1;

// // We have to blank the top line each time the display is scrolled, but this takes up to 13 milliseconds
// // for a full width line, meanwhile the serial buffer may be filling... and overflowing
// // We can speed up scrolling of short text lines by just blanking the character we drew
// int blank[19]; // We keep all the strings pixel lengths to optimise the speed of the top line blanking

// int scroll_line();

// void setupScrollArea(uint16_t tfa, uint16_t bfa);

// void scrollAddress(uint16_t vsp);

// void GUI_setup()
// {
//     tft.init();
//     tft.setRotation(0);

//     tft.fillScreen(TFT_BLACK);
//     //start-up log
//     tft.setTextColor(TFT_WHITE, TFT_BLUE);
//     tft.fillRect(0, 0, 320, 16, TFT_BLUE);
//     tft.drawCentreString("System information - Nino", 160, 0, 2);
    
//     //system debug log
//     tft.setTextColor(TFT_WHITE, TFT_BLUE);
//     tft.fillRect(0, 240, 320, 16, TFT_BLUE);
//     tft.drawCentreString(" System Log - Nino", 160, 240, 2);

//     // Change colour for scrolling zone text
//     tft.setTextColor(TFT_WHITE, TFT_BLACK);

//     // Setup scroll area
//     setupScrollArea(TOP_FIXED_AREA, BOT_FIXED_AREA);

//     for (byte i = 0; i < 18; i++)
//         blank[i] = 0;
// }

// void GUI_logPrint(std::string logStr)
// {
//     xSemaphoreTake(GUILog_Mutex, portMAX_DELAY);
//     for (char character : logStr)
//     {
//         char data = character;
//         // If it is a CR or we are near end of line then scroll one line
//         if (data == '\r' || xPos > 319)
//         {
//             xPos = 0;
//             yDraw = scroll_line(); // It can take 13ms to scroll and blank 16 pixel lines
//         }
//         if (data > 31 && data < 128)
//         {
//             xPos += tft.drawChar(data, xPos, yDraw, 2);
//             blank[(18 + (yStart - TOP_FIXED_AREA) / TEXT_HEIGHT) % 19] = xPos; // Keep a record of line lengths
//         }
//     }
//     xSemaphoreGive(GUILog_Mutex);
// }

// void GUI_sysPrint(int32_t x, int32_t y, const char* str, ...){
//     char buffer[256];
//     va_list args;
//     va_start(args, str);

//     vsnprintf(buffer, sizeof(buffer), str, args);
//     va_end(args);

//     tft.drawString(buffer, x, y, 2);
// }

// // ##############################################################################################
// // Call this function to scroll the display one text line
// // ##############################################################################################
// int scroll_line()
// {
//     int yTemp = yStart; // Store the old yStart, this is where we draw the next line
//     // Use the record of line lengths to optimise the rectangle size we need to erase the top line
//     tft.fillRect(0, yStart, blank[(yStart - TOP_FIXED_AREA) / TEXT_HEIGHT], TEXT_HEIGHT, TFT_BLACK);

//     // Change the top of the scroll area
//     yStart += TEXT_HEIGHT;
//     // The value must wrap around as the screen memory is a circular buffer
//     if (yStart >= YMAX - BOT_FIXED_AREA)
//         yStart = TOP_FIXED_AREA + (yStart - YMAX + BOT_FIXED_AREA);
//     // Now we can scroll the display
//     scrollAddress(yStart);
//     return yTemp;
// }

// // ##############################################################################################
// // Setup a portion of the screen for vertical scrolling
// // ##############################################################################################
// // We are using a hardware feature of the display, so we can only scroll in portrait orientation
// void setupScrollArea(uint16_t tfa, uint16_t bfa)
// {
//     tft.writecommand(ST7796_VSCRDEF); // Vertical scroll definition
//     tft.writedata(tfa >> 8);          // Top Fixed Area line count
//     tft.writedata(tfa);
//     tft.writedata((YMAX - tfa - bfa) >> 8); // Vertical Scrolling Area line count
//     tft.writedata(YMAX - tfa - bfa);
//     tft.writedata(bfa >> 8); // Bottom Fixed Area line count
//     tft.writedata(bfa);
// }

// // ##############################################################################################
// // Setup the vertical scrolling start address pointer
// // ##############################################################################################
// void scrollAddress(uint16_t vsp)
// {
//     tft.writecommand(ST7796_VSCRSADD); // Vertical scrolling pointer
//     tft.writedata(vsp >> 8);
//     tft.writedata(vsp);
// }

/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include <stdlib.h>

extern void EPD_2IN7_V2_SendCommand(UBYTE Reg);

/* Entry point ----------------------------------------------------------------*/
void epaper_setup()
{
    printf("EPD_2IN7_test Demo\r\n");
    DEV_Module_Init();

    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN7_V2_Init();
    EPD_2IN7_V2_Clear();
    EPD_2IN7_V2_SendCommand(0x3c);
    DEV_Delay_ms(500);

    //Create a new image cache
    UBYTE *BlackImage;
    /* you have to edit the startup_stm32fxxx.s file and set a big enough heap size */
    UWORD Imagesize = ((EPD_2IN7_V2_WIDTH % 8 == 0)? (EPD_2IN7_V2_WIDTH / 8 ): (EPD_2IN7_V2_WIDTH / 8 + 1)) * EPD_2IN7_V2_HEIGHT;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while (1);
    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN7_WIDTH, EPD_2IN7_HEIGHT, 270, WHITE);


#if 1   // show image for array   
    printf("show image for array\r\n");
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawBitMap(gImage_2in7);
    EPD_2IN7_V2_Display(BlackImage);
    DEV_Delay_ms(500);
#endif

#if 1  // Drawing on the image
    Paint_NewImage(BlackImage, EPD_2IN7_V2_WIDTH, EPD_2IN7_V2_HEIGHT, 90, WHITE);  	
    printf("Drawing\r\n");
    //1.Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    // 2.Drawing on the image
    printf("Drawing:BlackImage\r\n");
    Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);

    Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawCircle(45, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

    Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);

    Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);
    Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

    Paint_DrawString_CN(130, 0,"你好abc", &Font12CN, BLACK, WHITE);
    Paint_DrawString_CN(130, 20, "微雪电子", &Font24CN, WHITE, BLACK);

    EPD_2IN7_V2_Display_Base(BlackImage);
    DEV_Delay_ms(3000);
#endif

#if 1  // Fast Drawing on the image
    // Fast refresh
    printf("This is followed by a quick refresh demo\r\n");
    printf("First, clear the screen\r\n");
    EPD_2IN7_V2_Init();
    EPD_2IN7_V2_Clear();

    printf("e-Paper Init Fast\r\n");
    EPD_2IN7_V2_Init_Fast();
	Paint_NewImage(BlackImage, EPD_2IN7_V2_WIDTH, EPD_2IN7_V2_HEIGHT, 90, WHITE);  	
    printf("Drawing\r\n");
    //1.Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    printf("show bmp------------------------\r\n");
    Paint_Clear(WHITE);
    Paint_DrawBitMap(gImage_2in7);
    EPD_2IN7_V2_Display_Fast(BlackImage);
    DEV_Delay_ms(500);


    // 2.Drawing on the image
    Paint_Clear(WHITE);
    printf("Drawing:BlackImage\r\n");
    Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);

    Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawCircle(45, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

    Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);

    Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);
    Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

    Paint_DrawString_CN(130, 0,"你好abc", &Font12CN, BLACK, WHITE);
    Paint_DrawString_CN(130, 20, "微雪电子", &Font24CN, WHITE, BLACK);

    EPD_2IN7_V2_Display_Fast(BlackImage);
    DEV_Delay_ms(3000);

    

#endif

#if 1   //Partial refresh, example shows time    	
    // If you didn't use the EPD_2IN7_V2_Display_Base() function to refresh the image before,
    // use the EPD_2IN7_V2_Display_Base_color() function to refresh the background color, 
    // otherwise the background color will be garbled 
    EPD_2IN7_V2_Init();
    // EPD_2IN7_V2_Display_Base_color(WHITE);
	Paint_NewImage(BlackImage, 50, 120, 90, WHITE);
    
    printf("Partial refresh\r\n");
    Paint_SelectImage(BlackImage);
	Paint_SetScale(2);
    Paint_Clear(WHITE);
    
    PAINT_TIME sPaint_time;
    sPaint_time.Hour = 12;
    sPaint_time.Min = 34;
    sPaint_time.Sec = 56;
    UBYTE num = 15;
    for (;;) {
        sPaint_time.Sec = sPaint_time.Sec + 1;
        if (sPaint_time.Sec == 60) {
            sPaint_time.Min = sPaint_time.Min + 1;
            sPaint_time.Sec = 0;
            if (sPaint_time.Min == 60) {
                sPaint_time.Hour =  sPaint_time.Hour + 1;
                sPaint_time.Min = 0;
                if (sPaint_time.Hour == 24) {
                    sPaint_time.Hour = 0;
                    sPaint_time.Min = 0;
                    sPaint_time.Sec = 0;
                }
            }
        }
        
        Paint_Clear(WHITE);
		Paint_DrawRectangle(1, 1, 120, 50, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        Paint_DrawTime(10, 15, &sPaint_time, &Font20, WHITE, BLACK);

        num = num - 1;
        if(num == 0) {
            break;
        }
		printf("Part refresh...\r\n");
        EPD_2IN7_V2_Display_Partial(BlackImage, 60, 134, 110, 254); // Xstart must be a multiple of 8
        DEV_Delay_ms(500);
    }
#endif

#if 1 // show image for array
    free(BlackImage);
    printf("show Gray------------------------\r\n");
    Imagesize = ((EPD_2IN7_V2_WIDTH % 4 == 0)? (EPD_2IN7_V2_WIDTH / 4 ): (EPD_2IN7_V2_WIDTH / 4 + 1)) * EPD_2IN7_V2_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while (1);
    }
    EPD_2IN7_V2_Init_4GRAY();
    printf("4 grayscale display\r\n");
    Paint_NewImage(BlackImage, EPD_2IN7_V2_WIDTH, EPD_2IN7_V2_HEIGHT, 90, WHITE);
    Paint_SetScale(4);
    Paint_Clear(0xff);
    
    Paint_DrawPoint(10, 80, GRAY4, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, GRAY4, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, GRAY4, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(20, 70, 70, 120, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(20, 70, 70, 120, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(45, 95, 20, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, GRAY2, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(85, 95, 125, 95, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(10, 0, "waveshare", &Font16, GRAY4, GRAY1);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, GRAY3, GRAY1);
    Paint_DrawNum(10, 33, 123456789, &Font12, GRAY4, GRAY2);
    Paint_DrawNum(10, 50, 987654321, &Font16, GRAY1, GRAY4);
    Paint_DrawString_CN(150, 0,"你好abc", &Font12CN, GRAY4, GRAY1);
    Paint_DrawString_CN(150, 20,"你好abc", &Font12CN, GRAY3, GRAY2);
    Paint_DrawString_CN(150, 40,"你好abc", &Font12CN, GRAY2, GRAY3);
    Paint_DrawString_CN(150, 60,"你好abc", &Font12CN, GRAY1, GRAY4);
    Paint_DrawString_CN(10, 130, "微雪电子", &Font24CN, GRAY1, GRAY4);
    EPD_2IN7_V2_4GrayDisplay(BlackImage);
    DEV_Delay_ms(3000);

#endif

	printf("Clear...\r\n");
	EPD_2IN7_V2_Init();
    EPD_2IN7_V2_Clear();
	
    printf("Goto Sleep...\r\n");
    EPD_2IN7_V2_Sleep();
    free(BlackImage);
    BlackImage = NULL;
    DEV_Delay_ms(2000);//important, at least 2s
    // close 5V
    printf("close 5V, Module enters 0 power consumption ...\r\n");

}
