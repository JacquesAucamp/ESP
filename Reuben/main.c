////////////////////////////START//////////////////////////////////////////////
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stdlib.h"
#include "math.h"
#include "stdio.h"

///////////////////DECLARATIONS////////////////////////////////////////////////

int N = 256; // The order of the FFT

void SystemClock_Config(void);
void LCD_Intitialisation(void);
void LCD_SetupAxes(void);
void PlotFunction(double F[N]);



///////////////////////////////////////////////////////////////////////////////

int main(void){
	//SO we start by initialsing
HAL_Init();
	//Intialise the system clock next
SystemClock_Config();
	//Now initialise the LCD and setup it's parameters and conditions
LCD_Intitialisation();
LCD_SetupAxes();

//Let's define a testing input signal
double Y [N];
double t = 0.000025;//1/5000; //Minimum timestep
int f = 500; //Define the signal frequency
//Now assign it to a variable
for (int i = 0; i<N ; i = i+1){
	Y[i] = 2000*sin(2*3.1415167*(f*t)*i)+2000;
}

//Now we will enter into the main loop of the program
while(1){
	PlotFunction(Y);
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
/////////LCD AXES setup
void LCD_SetupAxes(void)
{
	BSP_LCD_Clear(LCD_COLOR_BLACK);  //Clear and set the colour
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN); //Set the axis colour
	BSP_LCD_DrawHLine(40,30,260); //Draw the axis
	BSP_LCD_DrawVLine(40,30,200);
	BSP_LCD_DisplayChar(5,140,0x56); //Add the V dB
	BSP_LCD_DisplayChar(5,120,0x64);
	BSP_LCD_DisplayChar(20,120,0x42);
	BSP_LCD_DisplayChar(125,0,0x66); //Add the f KHz
	BSP_LCD_DisplayChar(150,0,0x4B);
	BSP_LCD_DisplayChar(165,0,0x48);
	BSP_LCD_DisplayChar(180,0,0x7A);
}
//////////Plot a given function
void PlotFunction(double F[N]){
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
//Scale F to the desired values
	for (int i = 0; i<(N/2); i = i+1){
		F[i] =  F[i]/22;
	}
//Now plot the data
	int j = 41;
	for (int i = 0; i <(N/2); i = i+1){
		//We are making every 2 pixels 1 data bin.
		BSP_LCD_DrawVLine(j, 31,(int) F[i]);
		j = j+1;
		BSP_LCD_DrawVLine(j, 31, (int) F[i]);
		j=j+1;
	}

}


/////////////////////////////////////END///////////////////////////////////////

