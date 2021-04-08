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
# define N 256
#define M_PI acos(-1.0)
//#define I sqrt(-1)

//int N = 128; // The order of the FFT
float complex twiddle[N];			/*Create an empty complex array*/
//float twiddle_imag[N];		//Used to store the twiddle factors
float ADC_1 [N];
float ADC_2 [N];
int flag_1 = 0;
int flag_2 = 0;

float Signal [N];
float t = 0.000025;//1/4000; //Minimum timestep
int f = 5000; //Define the signal frequency

int adc_value;
int ADCCounter = 0;



ADC_HandleTypeDef g_AdcHandle;

//----------------------------------------------------------------------------

//Private Functions-----------------------------------------------------------
void SystemClock_Config(void);
void LCD_Intitialisation(void);
void LCD_SetupAxes(void);
void PlotFunction(float F[N]);
void ProcessData(float S[N]);
void TwiddleInit(void);
void ADCInit(void);
void ClearDrawSpace(void);





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
    		ADCCounter ++;
    	}
    	else{
    		ADC_2[ADCCounter - N] = adc_value;
    		ADCCounter ++;
    	}

    	if (ADCCounter == N){
    		flag_1 = 1;
    		__disable_irq();

    	}
    	if (ADCCounter == (2*N)){
    		flag_2 = 1;
    		ADCCounter = 0;
    		__disable_irq();
    		//f = f+0.1 ;
    	}


    	/*if (flag_1 == 1){
				//Now assign it to a variable
			for (int i = 0; i<N ; i++){
				Signal[i] = 2000*sin(2*M_PI*(f*t)*i)+2000;
			}
			for (int i = 0; i < N; i++){
				ADC_1[i] = ADC_1[i] + Signal[i];
			}
    	}
    	if (flag_2 == 1){
				//Now assign it to a variable
			for (int i = 0; i<N ; i++){
				Signal[i] = 2000*sin(2*M_PI*(f*t)*i)+2000;
			}
			for (int i = 0; i < N; i++){
				ADC_2[i] = ADC_2[i] + Y[i];
			}
    	}*/


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
	HAL_ADC_Start(&g_AdcHandle);
    HAL_ADC_Start_IT(&g_AdcHandle);





//Now we will enter into the main loop of the program
while(1){
	//Let's define a testing input signal
		/*float Y [N] ;//= {0.3535,0.3535,0.6464,1.0607,0.3535,-1.0607,-1.3535,-0.3535};
		float t = 0.000025;//1/4000; //Minimum timestep
		int f = 12000; //Define the signal frequency
		//Now assign it to a variable
		for (int i = 0; i<N ; i++){
			Y[i] = 2000*sin(2*M_PI*(f*t)*i)+2000;
		}
		//flag_1 = 1;
		for (int i = 0; i < N; i++){
			ADC_1[i] = Y[i];
			ADC_2[i] = Y[i];
		}*/


	if (flag_1 == 1){
		//PlotFunction(ADC_1);

		ProcessData(ADC_1);
		flag_1 = 0;
		//f = f +10;
		__enable_irq();
		//HAL_Delay(10);
	}
	else if (flag_2 == 1){

		ProcessData(ADC_2);
		flag_2 = 0;

		__enable_irq();
	}


	}
}







/////////////////////////SUBROUTINES///////////////////////////////////////////
/////////////////////CLOCK
//The system clock initialisation process
//It's a bit long and there is a lot going on there
void SystemClock_Config(void){
	/* @brief System Clock Configuration
	* The system Clock is configured as follow :
	* System Clock source = PLL (HSE)
	* SYSCLK(Hz) = 168000000
	* HCLK(Hz) = 168000000
	* AHB Prescaler = 1
	* APB1 Prescaler = 4
	* APB2 Prescaler = 2
	* HSE Frequency(Hz) = HSE_VALUE
	* PLL_M = (HSE_VALUE/1000000u)
	* PLL_N = 336
	* PLL_P = 2
	* PLL_Q = 7
	* VDD(V) = 3.3
	* Main regulator output voltage = Scale1 mode
	* Flash Latency(WS) = 5
	* @param None
	* @retval None
	*/
	{
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
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
	}
}

///////////LCD_Initialise
void LCD_Intitialisation(void)
{
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER,LCD_FRAME_BUFFER);
	BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER,LCD_FRAME_BUFFER);
	BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
	BSP_LCD_DisplayOn();
	BSP_LCD_SelectLayer(LCD_BACKGROUND_LAYER);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

}
/////////LCD AXES setup
void LCD_SetupAxes(void)

{
	BSP_LCD_Clear(LCD_COLOR_BLACK);  //Clear and set the colour
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE); //Set the axis colour
	BSP_LCD_DrawHLine(60,40,260); //Draw the axis
	BSP_LCD_DrawVLine(60,40,200);

	//AXIS LABELS
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(1,200,(uint8_t *)"V(dB)",LEFT_MODE);
	BSP_LCD_DisplayStringAt(70,0,(uint8_t *)"f(kHz)",CENTER_MODE);
	//AXIS VALUES
	BSP_LCD_SetFont(&Font12);
	//X-AXIS
	//Label every 5kHz
	BSP_LCD_DisplayChar(55,18,0x30);
	BSP_LCD_DisplayStringAt(124,18,(uint8_t *)"5",LEFT_MODE);
	BSP_LCD_DisplayStringAt(188,18,(uint8_t *)"10",LEFT_MODE);
	BSP_LCD_DisplayStringAt(252,18,(uint8_t *)"15",LEFT_MODE);
	BSP_LCD_DisplayStringAt(305,18,(uint8_t *)"20",LEFT_MODE);
	//Y-AXIS
	//Label every 20dB
	BSP_LCD_DisplayStringAt(45,35,(uint8_t *)"20",LEFT_MODE);
	BSP_LCD_DisplayStringAt(45,50,(uint8_t *)"40",LEFT_MODE);
	BSP_LCD_DisplayStringAt(45,65,(uint8_t *)"60",LEFT_MODE);
	BSP_LCD_DisplayStringAt(45,80,(uint8_t *)"80",LEFT_MODE);
	BSP_LCD_DisplayStringAt(35,95,(uint8_t *)"100",LEFT_MODE);
	BSP_LCD_DisplayStringAt(35,110,(uint8_t *)"120",LEFT_MODE);
	BSP_LCD_DisplayStringAt(35,125,(uint8_t *)"140",LEFT_MODE);
	BSP_LCD_DisplayStringAt(35,140,(uint8_t *)"160",LEFT_MODE);
	BSP_LCD_DisplayStringAt(35,155,(uint8_t *)"180",LEFT_MODE);
	BSP_LCD_DisplayStringAt(35,170,(uint8_t *)"200",LEFT_MODE);
}
//////////Calculate the Twiddle factors
void TwiddleInit(void){
	float TWDI;
	float TWDR;
	float complex TWD;
	for (int i = 0; i< N; i++){
		twiddle[i] = 0;
	}
	//float complex twiddle;
	for (int i=0; i< N; i++){
		//twiddle[i] = expf(-((I*2*M_PI*i)/N));	/*Twiddle factor formula*/
	//	twiddle_real[i] = creal(twiddle);
	//	twiddle_imag[i] = cimag(twiddle);
		TWDR = cos((2*M_PI*i)/N);
		TWDI = - sin((2*M_PI*i)/N);
		TWD = TWDR + (I * TWDI);
		twiddle[i] = TWD;
	}
}
//////////Clear the drawing space to allow for a new plot
void ClearDrawSpace(void){
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	for (int i = 61; i <= 300; i++){
		BSP_LCD_DrawVLine(i, 41,  200);
	}
	//LCD_SetupAxes();
	//BSP_LCD_FillRect(41,31,280,200);
}
//////////Plot a given function
void PlotFunction(float F[N]){

//Scale F to the desired values
	float FO [N];
	for (int i = 0; i < (N); i++){
		FO[i] =  3*20*log10(F[i]/(N));
		//FO[i] =  (F[i]/(22));
	}
//Let's clear the display
	//ClearDrawSpace();
	//BSP_LCD_SetTextColor(LCD_COLOR_RED);
//Now plot the data
	int j = 61;
	for (int i = 0; i < (N/2); i++){
		//We are making every 2 pixels 1 data bin.
		//Add a loop here to make sure it uses up the full available space
		while (j < (i+1)*(256/(N/2)) + 61){
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_DrawVLine(j, 41,  200);
			BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
			if (FO[i] >=1){
				BSP_LCD_DrawVLine(j, 41,  FO[i]);
			}
			j = j+1;
		}


	}

}
///////////Process the data with an FFT algortihm
void ProcessData(float S[N]){
	//__disable_irq();
	float Y [N];
	for (int i = 0; i<N;i++){
		Y[i] = S[i];
	}
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
		float complex YSorted [N];
		for (int k = 0; k < N; k++){
			YSorted[k] = Y[k];
		}
		float complex YT[N];
		float complex twdA;
		float complex twdB;

		for (int i = 0; i< N; i++){		/*Make YT a empty complex array*/
			YT[i] = 0;
		}
			/*First we need to loop for each layer of the butterfly diagram*/
		for (int i=0; i<log2(N); i++){
			int s = -pow(2,i);					/*the skipping factor*/
			for (int j=0; j<N; j++){
				if (j % ((int)(pow(2,i))) == 0){	/*Make the  skipping factor + or -*/
					s = s * -1;
				}
					/*Now we need to get the correct twiddle factors*/
				int P = (int)(j*(N/(pow(2,i+1))))%N;
				if (s > 0){						/*If we retrieving from bellow*/
					twdA = 1;
					twdB = twiddle[P];
				}
				else{	/*else retrieve from above*/
					twdA = twiddle[P];
					twdB = 1;
				}
				YT[j] = (twdA*YSorted[j])+(twdB*YSorted[j+s]);
			}
			for (int i=0; i<N; i++){
				YSorted[i] = YT[i];
			}
		}
		for (int j=0; j<N; j++){
			Y[j] = abs(creal(YSorted[j])) + abs(cimag(YSorted[j]));
		}

		PlotFunction(Y);


}
////////////Initialise the ADC
void ADCInit(void){
	GPIO_InitTypeDef gpioInit;

	    __GPIOC_CLK_ENABLE();
	    __ADC1_CLK_ENABLE();

	    gpioInit.Pin = GPIO_PIN_1;			//PA0
	    gpioInit.Mode = GPIO_MODE_ANALOG;
	    gpioInit.Pull = GPIO_NOPULL;
	    HAL_GPIO_Init(GPIOC, &gpioInit);

	    HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	    HAL_NVIC_EnableIRQ(ADC_IRQn);

	    ADC_ChannelConfTypeDef adcChannel;

	    g_AdcHandle.Instance = ADC1;

	    //g_AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
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

	    adcChannel.Channel = ADC_CHANNEL_11;
	    adcChannel.Rank = 1;
	    adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	    adcChannel.Offset = 0;

	    if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK)
	    {
	        asm("bkpt 255");
	    }
}




/////////////////////////////////////END///////////////////////////////////////
