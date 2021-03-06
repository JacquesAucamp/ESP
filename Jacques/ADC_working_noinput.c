////////////////////////////START//////////////////////////////////////////////
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>
#include <stdbool.h>



///////////////////DECLARATIONS////////////////////////////////////////////////

//Private Variables ----------------------------------------------------------
int N = 256; // The order of the FFT
float twiddle_real[256];			/*Create an empty complex array*/
float twiddle_imag[256];		//Used to store the twiddle factors
float ADC_1 [256];
float ADC_2 [256];
bool flag_1 = 0;
bool flag_2 = 0;

int adc_value;
int ADCCounter = 0;

#define M_PI acos(-1.0)
#define I sqrt(-1)

ADC_HandleTypeDef g_AdcHandle;

//----------------------------------------------------------------------------

//Private Functions-----------------------------------------------------------
void SystemClock_Config(void);
void LCD_Intitialisation(void);
void LCD_SetupAxes(void);
void PlotFunction(float F[N]);
void ProcessData(float Y[N]);
void TwiddleInit(void);
void ADCInit(void);





    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
    {
    	//Poll for end of ADC conversion
        if (HAL_ADC_PollForConversion(&g_AdcHandle, 1000000) == HAL_OK)
         {
             adc_value = HAL_ADC_GetValue(&g_AdcHandle);//Read ADC value
         }


    	//Place the ADC value into the correct array
    	if (ADCCounter < N){
    		ADC_1[ADCCounter] = adc_value;
    		flag_1 = 1;
    		ADCCounter ++;
    	}
    	else {
    		ADC_1[ADCCounter - N] = adc_value;
    		flag_2 = 1;
    		ADCCounter ++;
    	}
    	if (ADCCounter == 2*N){
    		ADCCounter = 0;
    	}
    	/*if (flag_1 == 1){
			ProcessData(ADC_1);
			flag_1 = 0;
		}
		else if (flag_2 == 1){
			ProcessData(ADC_2);
			flag_2 = 0;
		}*/
    	//HAL_ADC_Start_IT(&g_AdcHandle);

    }

    void ADC_IRQHandler()
    {
        HAL_ADC_IRQHandler(&g_AdcHandle);
    }






///////////////////////////////////////////////////////////////////////////////

int main(void){
		//SO we start by initialsing
	HAL_Init();
		//Intialise the system clock next
	SystemClock_Config();
		//Now initialise the LCD and setup it's parameters and conditions
	LCD_Intitialisation();
	LCD_SetupAxes();
		//Now lets Define the Twiddle Factors
	TwiddleInit();
		//Now let's Initialise the ADC
	ADCInit();
    HAL_ADC_Start_IT(&g_AdcHandle);





//Now we will enter into the main loop of the program
while(1){
	//Let's define a testing input signal
	/*	float Y [N];
		float t = 0.000025;//1/4000; //Minimum timestep
		int f = 10000; //Define the signal frequency
		//Now assign it to a variable
		for (int i = 0; i<N ; i++){
			Y[i] = 2000*sin(2*M_PI*(f*t)*i)+2000;
			//Y[i] = 10*i;
		}
		flag_1 = 1;
		for (int i = 0; i < N; i++){
			ADC_1[i] = Y[i];
			ADC_2[i] = Y[i];
		}*/


	if (flag_1 == 1){
		ProcessData(ADC_1);
		flag_1 = 0;
	}
	else if (flag_2 == 1){
		ProcessData(ADC_2);
		flag_2 = 0;
	}


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
	/*RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 160;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 3;*/
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
//////////Calculate the Twiddle factors
void TwiddleInit(void){
	for (int i = 0; i< N; i++){
		twiddle_real[i] = 0;
		twiddle_imag[i] = 0;
	}
	//float complex twiddle;
	for (int i=0; i< N; i++){
	//	twiddle = expf(-((I*2*M_PI*i)/N));	/*Twiddle factor formula*/
	//	twiddle_real[i] = creal(twiddle);
	//	twiddle_imag[i] = cimag(twiddle);
		twiddle_real[i] = cos((2*M_PI*i)/N);
		twiddle_imag[i] = sin((2*M_PI*i)/N);
	}
}
//////////Plot a given function
void PlotFunction(float F[N]){

//Scale F to the desired values
	float FO [N];
	for (int i = 0; i < (N); i++){
		FO[i] =  2*20*log10(F[i]/(N));
		//FO[i] =  (F[i]/(22));
	}
//Let's clear the display
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_FillRect(41,31,260,200);
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
//Now plot the data
	int j = 41;
	for (int i = 0; i < (N/2); i++){
		//We are making every 2 pixels 1 data bin.
		//Add a loop here to make sure it uses up the full available space
		while (j < (i+1)*(256/(N/2)) + 41){
			BSP_LCD_DrawVLine(j, 31,  FO[i]);
			j = j+1;
		}




		//BSP_LCD_DrawVLine(j, 31,  FO[i]);
		//j = j+1;

	}

}
///////////Process the data with an FFT algortihm
void ProcessData(float Y[N]){


	float Ytemp [N]; //Create a temporary or working array
		for (int i = 0; i < (log2(N))-1; i++){
			int p = pow(2,i);
			float oddeven_arr[N/p];
					//oddeven function now
			for (int j = 0; j < N; j = j + (N/p)){
				int n = 0;
				for (int k = j; k < j+(N/p); k++){//Divide up the data
					oddeven_arr[n] = Y[k];
					n++;
				}
				int L = N/p;//sizeof(oddeven_arr);
				float z[L];						/*empty array*/
				for (int k = 0; k < L/2; k++){			/*Only need to loop through half the*/
					z[k] = oddeven_arr[2*k];			/*length. Will place every even sample at the sfirst*/
					z[L-k-1] = oddeven_arr[L-(2*k)-1];	/*half and every odd sample at the second half*/
				}
				n = 0;
				for (int k = j; k < j + (N/p); k++){//Return the ordered sata
					Ytemp[k] = z[n];
					n++;
				}
			}
			for (int k = 0; k < N; k++){
				Y[k] = Ytemp[k];
			}
		}
		//PlotFunction(Y);

		//Now let's do the Butterfly processing
		//start by declaring the variables and arrays that will be needed
		float YSorted [N];
		for (int k = 0; k < N; k++){
			YSorted[k] = Y[k];
		}
		float YT_real [N];
		float YT_imag [N];
		int twdAreal;
		int twdAimag;
		int twdBreal;
		int twdBimag;

		for (int i = 0; i< N; i++){		/*Make YT a empty complex array*/
			YT_real[i] = 0;
			YT_imag[i] = 0;
		}
			/*First we need to loop for each layer of the butterfly diagram*/
		for (int i=0; i<log2(N); i++){
			int s = -pow(2,i);					/*the skipping factor*/
			for (int j=0; j<N; j++){
				if (j % ((int)(pow(2,i))) == 0){	/*Make the  skipping factor + or -*/
					s = s * -1;
				}
					/*Now we need to get the correct twiddle factors*/
				if (s > 0){						/*If we retrieving from bellow*/
					twdAreal = 1;
					twdAimag = 0;
					twdBreal = twiddle_real[((int)(j*(N/(pow(2,i+1))))%N)];
					twdBimag = twiddle_imag[((int)(j*(N/(pow(2,i+1))))%N)];
				}
				else{							/*else retrieve from above*/
					twdBreal = 1;
					twdBimag = 0;
					twdAreal = twiddle_real[((int)(j*(N/(pow(2,i+1))))%N)];
					twdAimag = twiddle_imag[((int)(j*(N/(pow(2,i+1))))%N)];
				}
				YT_real[j] = (twdAreal*YSorted[j])+(twdBreal*YSorted[j+s]);
				YT_imag[j] = (twdAimag*YSorted[j])+(twdBimag*YSorted[j+s]);
			}
			for (int i=0; i<N; i++){
				YSorted[i] = YT_real[i]+YT_imag[i];
			}
		}
		for (int j=0; j<N; j++){
			Y[j] = YSorted[j];
		}

		PlotFunction(Y);


}
////////////Initialise the ADC
void ADCInit(void){
	GPIO_InitTypeDef gpioInit;

	    __GPIOC_CLK_ENABLE();
	    __ADC1_CLK_ENABLE();

	    gpioInit.Pin = GPIO_PIN_1;
	    gpioInit.Mode = GPIO_MODE_ANALOG;
	    gpioInit.Pull = GPIO_NOPULL;
	    HAL_GPIO_Init(GPIOC, &gpioInit);

	    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	    HAL_NVIC_EnableIRQ(ADC_IRQn);

	    ADC_ChannelConfTypeDef adcChannel;

	    g_AdcHandle.Instance = ADC1;

	    g_AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
	    g_AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
	    g_AdcHandle.Init.ScanConvMode = DISABLE;
	    g_AdcHandle.Init.ContinuousConvMode = ENABLE;
	    g_AdcHandle.Init.DiscontinuousConvMode = DISABLE;
	    g_AdcHandle.Init.NbrOfDiscConversion = 0;
	    g_AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	    g_AdcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
	    g_AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	    g_AdcHandle.Init.NbrOfConversion = 1;
	    g_AdcHandle.Init.DMAContinuousRequests = ENABLE;
	    g_AdcHandle.Init.EOCSelection = DISABLE;

	    HAL_ADC_Init(&g_AdcHandle);

	    adcChannel.Channel = ADC_CHANNEL_10;
	    adcChannel.Rank = 1;
	    adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	    adcChannel.Offset = 0;

	    if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK)
	    {
	        asm("bkpt 255");
	    }
}




/////////////////////////////////////END///////////////////////////////////////
