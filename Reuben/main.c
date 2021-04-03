////////////////////////////START//////////////////////////////////////////////
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

///////////////////DECLARATIONS////////////////////////////////////////////////

void SystemClock_Config(void);
void LCD_Intitialisation(void);

///////////////////////////////////////////////////////////////////////////////

int main(void){
	//SO we start by initialsing
HAL_Init();
	//Intialise the system clock next
SystemClock_Config();
	//Now initialise the LCD and setup it's parameters and conditions
LCD_Intitialisation();

int x = 10;
int y = 10;
int l = 100;

//Now we will enter into the main loop of the program
while(1){
	BSP_LCD_DrawHLine(x,y,l);
	BSP_LCD_DrawVLine(x,y,l);
	//BSP_DrawChar(x,y,"R");
	BSP_LCD_DisplayChar(x,y,0x52);


	}
}

/////////////////////////SUBROUTINES///////////////////////////////////////////
/////////////////////CLOCK
//The system clock initialisation process
//It's a bit long and there is a lot going on there
void SystemClock_Config(void){
RCC_ClkInitTypeDef RCC_ClkInitStruct;
RCC_OscInitTypeDef RCC_OscInitStruct;
// Enable Power Control clock
__PWR_CLK_ENABLE();
// The voltage scaling allows optimizing the power consumption when the
// device is clocked below the maximum system frequency, to update the
// voltage scaling value regarding system frequency refer to product
// datasheet.
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
// Enable HSE Oscillator and activate PLL with HSE as source
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
RCC_OscInitStruct.HSEState = RCC_HSE_ON;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
// This assumes the HSE_VALUE is a multiple of 1MHz. If this is not
// your case, you have to recompute these PLL constants.
RCC_OscInitStruct.PLL.PLLM = (HSE_VALUE/1000000u);
RCC_OscInitStruct.PLL.PLLN = 336;
RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
RCC_OscInitStruct.PLL.PLLQ = 7;
HAL_RCC_OscConfig(&RCC_OscInitStruct);
// Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
// clocks dividers
RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

///////////LCD_Initialise
void LCD_Intitialisation(void)
{
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER,LCD_FRAME_BUFFER);
	BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER,LCD_FRAME_BUFFER);
	BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
	BSP_LCD_DisplayOn();
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	BSP_LCD_SelectLayer(LCD_BACKGROUND_LAYER);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);

}

