/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>
#include <stdbool.h>


//==========================================================================
// VARIABLES AND STUFF
//==========================================================================
ADC_HandleTypeDef    g_AdcHandle;
DMA_HandleTypeDef	 g_DmaHandle;
uint32_t g_ADCValue;
int ADCCounter = 0;
int g_MeasurementNumber;
enum{ ADC_BUFFER_LENGTH = 8192 };
uint32_t g_ADCBuffer[ADC_BUFFER_LENGTH];

# define N 256
#define FN 3535

float Data [N];
float Data_Filtered [N];
float mes_Data[N];
float car_Data[N];
float ADC_1 [N];
float ADC_2 [N];
float data_f[N];
float hd [N];
float wn [N];
float hn [N];

float complex twiddle[N];			/*Create an empty complex array*/

//fc1 = (526500-3000/2)/1800001
//fc2 = (535500+3000/2)/1800001
//fsampling = 1.8Mhz
//N = 3535
//half_N = 1767
//fc1 = 0.16344956
//fc2 = 0.16687422
float fc1 = 0.2916666667;
float fc2 = 0.2983331676;
float wc1;
float wc2;
//==========================================================================

//==========================================================================
// FUNCTION DEFINITIONS
//==========================================================================
static void SystemClock_Config(void);
void ConfigureADC();
void ConfigureDMA();
void LCD_Intitialisation(void);
void LCD_SetupAxes(void);
void ClearDisplay(void);
void PlotFrequencyFunction(float F[N]);
void ProcessData(float S[N]);
void TwiddleInit(void);
//void FIRCoefficientsCalc(void);
//==========================================================================


//==========================================================================
// SAMPLING HANDLERS
//==========================================================================
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	g_ADCValue = HAL_ADC_GetValue(AdcHandle);

	//Place the ADC value into the correct array
	/*if (ADCCounter < N){
		ADC_1[ADCCounter] = g_ADCValue;
		ADCCounter ++;
	}
	else{
		ADC_2[ADCCounter - N] = g_ADCValue;
		ADCCounter ++;
	}*/
}

void DMA2_Stream4_IRQHandler()
{
    HAL_DMA_IRQHandler(&g_DmaHandle);
}

void ADC_IRQHandler()
{
    HAL_ADC_IRQHandler(&g_AdcHandle);
}
//==========================================================================


//==========================================================================
// MAIN
//==========================================================================
int main(void)
{
	// Start with initializing
    HAL_Init();
    // Initialize the system clock
    SystemClock_Config();
	//Now initialize the LCD and setup it's parameters and conditions
	LCD_Intitialisation();
	LCD_SetupAxes();

	TwiddleInit();
    // Setup the ADC and DMA and start it
    ConfigureADC();
    ConfigureDMA();
	HAL_ADC_Start_DMA(&g_AdcHandle, g_ADCBuffer, ADC_BUFFER_LENGTH);

	wc1 = 2*M_PI*fc1;
	wc2 = 2*M_PI*fc2;
	//FIRCoefficientsCalc();

    GPIO_InitTypeDef GPIO_InitStructure;
	__GPIOD_CLK_ENABLE();

	GPIO_InitStructure.Pin = GPIO_PIN_12;

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	/*for (;;)
	{
		int onTime = g_ADCValue;
		int offTime = 8192 - onTime;
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
		for (int i = 0; i < onTime; i++)
			asm("nop");

		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
		for (int i = 0; i < offTime; i++)
			asm("nop");
	}*/

	// Define a test AM signal
	float t = 0.0000005;		//1/2000000
	int mes_f = 20000;			//message freq
	int car_f = 531000;			//carrier freq
	for (int i = 0; i<N ; i++){
		mes_Data[i] = 100*sin(2*M_PI*(mes_f*t)*i)+100;
	}
	for (int i = 0; i<N ; i++){
		car_Data[i] = 100*sin(2*M_PI*(car_f*t)*i);
	}
	for (int i = 0; i<N ; i++){
		Data[i] = mes_Data[i]*car_Data[i];
	}

	ProcessData(Data);
	// Convolve the data with the filter
	/*for (int i = 0; i<N; i++){//Start by setting the filtered array to 0
				Data_Filtered[i] = 0;
			}
			for (int i = 0; i<N; i++){//first loop
				for (int j = 0; j<=i; j++){//second
					Data_Filtered[i] = 	Data_Filtered[i] + (hn[j] * Data[i-j]);
				}
			}
			for (int i = 0; i<N; i++){//scale the values a bit there
				Data_Filtered[i] = Data_Filtered[i]/(5*N);
			}

			for (int i = 0; i<N; i++){
				Data[i] = Data_Filtered[i] * 100;
			}*/
	//ProcessData(Data);

	while(1){

		//PlotFrequencyFunction(g_ADCValue);
	}
}

//==========================================================================


////////////////////////////// SUBROUTINES /////////////////////////////////
//==========================================================================
// SETUP THE CLOCK
//==========================================================================
static void SystemClock_Config(void)
{
	// Input Clock Freq = 8MHz (HSE)
	// Main PLL M div = 8 ==> 1MHz
	// Main PLL N Mult = 288 ==> 288MHz
	// Main PLL P div = 288/2 ==> 144MHz	(SYS CLK)
	// Main PLL Q div = 288/6 ==> 48MHz
	// SYSCLK source is PLLCLK
	// AHB Prescaler = 1 ==> 144MHz 	(HCLK)
	// APB1 Prescaler = 4 ==> 36MHz (APB1 Peripherals)
	//					  ==> 36MHz*2 (APB1 Timer CLK)
	// APB2 Prescaler = 2 ==> 72MHz (APB2 Peripherals /2)
	//					  ==> 144MHz (APB2 Timer CLK)
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 288;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 6;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
	SystemCoreClockUpdate();

	if (HAL_GetREVID() == 0x1001)
	__HAL_FLASH_PREFETCH_BUFFER_ENABLE();
}
//==========================================================================

//==========================================================================
// INITIALISE LCD
//==========================================================================
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
//==========================================================================

//==========================================================================
// SETUP LCD AXES
//==========================================================================
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
	BSP_LCD_DisplayStringAt(118,8,(uint8_t *)"5",LEFT_MODE);
	BSP_LCD_DisplayStringAt(118,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(182,8,(uint8_t *)"10",LEFT_MODE);
	BSP_LCD_DisplayStringAt(182,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(246,8,(uint8_t *)"15",LEFT_MODE);
	BSP_LCD_DisplayStringAt(246,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(299,8,(uint8_t *)"20",LEFT_MODE);
	BSP_LCD_DisplayStringAt(299,20,(uint8_t *)"|",LEFT_MODE);
	//Y-AXIS
	BSP_LCD_DisplayStringAt(31,35,(uint8_t *)"-80-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(31,50,(uint8_t *)"-70-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(31,65,(uint8_t *)"-60-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(31,80,(uint8_t *)"-50-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(31,95,(uint8_t *)"-40-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(31,110,(uint8_t *)"-30-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(31,125,(uint8_t *)"-20-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(31,140,(uint8_t *)"-10-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(45,155,(uint8_t *)"0-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,170,(uint8_t *)"10-",LEFT_MODE);
}
//==========================================================================

//==========================================================================
// CLEAR DISPLAY FOR NEW PLOT
//==========================================================================
void ClearDisplay(void){
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	for (int i = 0; i <= 320; i++){
		BSP_LCD_DrawVLine(i, 0,  240);
	}
	//LCD_SetupAxes();
	//BSP_LCD_FillRect(41,31,280,200);
}

//==========================================================================
// SETUP THE ADCS
//==========================================================================
ADC_HandleTypeDef g_AdcHandle;

void ConfigureADC()
{
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

    //total sampling time = 480 cycles + 12 bit res = 492 ADCCLK Cycles
    //ADCCLK is 72MHz at the moment (8MHz input freq from HSE)
    //13.88ns per clock cycle
    //thus total sampling time = 492 * 13.88n = 6.833us
    //sample rate = 1/6.833u = 146.341kHz

    //option 1
    //for sample rate of 2MHz
    //sampling time = 0.5us
    //with 13.88ns clock cycle => adc cycles must be 36 ie. 28

    //option 2
    //to choose largest sampling time (best practice) increase ADCCLK
    //thus for sampling time = 0.5us
    //with 56+12=68 ADCCLK Cycles
    //7.35294ns per clock cycle needed
    //136MHz ADCCLK required
    adcChannel.Channel = ADC_CHANNEL_11;
    adcChannel.Rank = 1;
    adcChannel.SamplingTime = ADC_SAMPLETIME_28CYCLES;
    adcChannel.Offset = 0;

    if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK)
    {
        asm("bkpt 255");
    }
}
//==========================================================================

//==========================================================================
// SETUP THE DMA
//==========================================================================
DMA_HandleTypeDef  g_DmaHandle;

void ConfigureDMA()
{
    __DMA2_CLK_ENABLE();
    g_DmaHandle.Instance = DMA2_Stream4;

    g_DmaHandle.Init.Channel  = DMA_CHANNEL_0;
    g_DmaHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    g_DmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    g_DmaHandle.Init.MemInc = DMA_MINC_ENABLE;
    g_DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    g_DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    g_DmaHandle.Init.Mode = DMA_CIRCULAR;
    g_DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;
    g_DmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    g_DmaHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    g_DmaHandle.Init.MemBurst = DMA_MBURST_SINGLE;
    g_DmaHandle.Init.PeriphBurst = DMA_PBURST_SINGLE;

    HAL_DMA_Init(&g_DmaHandle);

    __HAL_LINKDMA(&g_AdcHandle, DMA_Handle, g_DmaHandle);

    HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);
}
//==========================================================================

//==========================================================================
// CALCULATE TWIDDLE FACTORS
//==========================================================================
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
//==========================================================================

//==========================================================================
// PROCESS DATA WITH A FFT FUNCTION
//==========================================================================
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

	for (int i = 0; i < N/2; i++){
		data_f[i] = Y[i];
	}
	PlotFrequencyFunction(data_f);
}
//==========================================================================

//==========================================================================
// PLOT A GIVEN FREQUENCY FUNCTION
//==========================================================================
void PlotFrequencyFunction(float F[N]){
//Scale F to the desired values
	float FO [N];
	/*for (int i = 0; i<N;i++){
		FO[i] = F[i];
	}*/
	for (int i = 0; i < (N); i++){
		FO[i] =  20*log10(F[i]/N);
		//FO[i] =  (F[i]/(22));
		if (FO[i] > 200){
			FO[i] = 200;
		}
		if (FO[i] < 0){
			FO[i] = 0;
		}
	}
	/*if (FIRButton == 1){
		for (int i =0; i<N; i++){
			FO[i] = 1000*FO[i];
		}
	}*/

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
//==========================================================================

//==========================================================================
// CALCULATE FILTER COEFFICIENTS
//==========================================================================
/*void FIRCoefficientsCalc(void)
{
	for (int n = 0; n<1768; n++){
		if (n == 0){
			hd[1767-n] = 2*(fc2-fc1);
		}
		else{
			hd[1767-n] = (2*fc2*sin(n*wc2))/(n*wc2)-(2*fc1*sin(n*wc1))/(n*wc1);
		}
		wn[1767-n] = 0.54 + (0.46*cos((2*M_PI*n)/FN));

		hn[1767-n] = wn[1767-n] * hd[1767-n];

		//The data is mirrored on both sides of the arrray
		hd[1767+n] = hd[1767-n];
		wn[1767+n] = wn[1767-n];
		hn[1767+n] = hn[1767-n];
	}

}*/
//==========================================================================
