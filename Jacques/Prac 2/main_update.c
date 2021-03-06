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
#define FN 95

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

//The values to be used in the filter
float fc = 0.052325;
float wc;
//we will have the following arrays
float hd [N];
float wn [N];
float hn [N];

float Data [N];
float Data_Filtered [N];

ADC_HandleTypeDef g_AdcHandle;

//----------------------------------------------------------------------------

//Private Functions-----------------------------------------------------------
void SystemClock_Config(void);
void LCD_Intitialisation(void);
void LCD_SetupAxes(void);
void LCD_SetupTimeAxes(void);
void PlotFunction(float F[N]);
void PlotTimeFunction(float T[N]);
void PlotTimeFIRFunction(float FIR[N]);
void ProcessData(float S[N]);
void TwiddleInit(void);
void ADCInit(void);
void ClearDrawSpace(void);
void FIRCoefficientsCalc(void);





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

	//which axes do you need
	//LCD_SetupAxes();
	//LCD_SetupTimeAxes();
	LCD_SetupTimeFIRAxes();

		//Now lets Define the Twiddle Factors
	TwiddleInit();
		//Now let's Initialise the ADC
	ADCInit();
	HAL_ADC_Start(&g_AdcHandle);
    HAL_ADC_Start_IT(&g_AdcHandle);

    //Now we need to define the FIR coefficents
     wc = 2 * M_PI * fc;
     FIRCoefficientsCalc();
 	/*for (int i = 0; i < N; i++){
 		hn[i] = (hn[i] *1000)+50;
 		float buff = hn[i];
 	}*/



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

	//PlotFunction(hn);

	//Update the input data
	if (flag_1 == 1){
		flag_1 = 0;
		for (int i = 0;i<N;i++){
			Data[i] = ADC_1[i];
		}
		__enable_irq();
	}
	else if (flag_2 == 1){
		flag_2 = 0;
		for (int i = 0;i<N;i++){
			Data[i] = ADC_2[i];
		}
		__enable_irq();
	}

	//Convolve the input with filter
	for (int i = 0; i<N; i++){//Start by setting the filtered array to 0
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
	//that's it

	//if button x pressed do one of following:
	//PlotTimeFunction(Data_Filtered);			//Plot Time Domain Output
	//ProcessData(Data_Filtered);					//Plot Freq Domain Output
	PlotTimeFIRFunction(hn);					//Plot Time Domain FIR Coef
	//ProcessData(hn);							//Plot Freq Domain FIR Coef

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
	BSP_LCD_DisplayStringAt(118,8,(uint8_t *)"5",LEFT_MODE);
	BSP_LCD_DisplayStringAt(118,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(182,8,(uint8_t *)"10",LEFT_MODE);
	BSP_LCD_DisplayStringAt(182,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(246,8,(uint8_t *)"15",LEFT_MODE);
	BSP_LCD_DisplayStringAt(246,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(299,8,(uint8_t *)"20",LEFT_MODE);
	BSP_LCD_DisplayStringAt(299,20,(uint8_t *)"|",LEFT_MODE);
	//Y-AXIS
	//Label every 5dB
	BSP_LCD_DisplayStringAt(31,35,(uint8_t *)"-10-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,50,(uint8_t *)"-5-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(46,65,(uint8_t *)"0-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(46,80,(uint8_t *)"5-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,95,(uint8_t *)"10-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,110,(uint8_t *)"15-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,125,(uint8_t *)"20-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,140,(uint8_t *)"25-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,155,(uint8_t *)"30-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,170,(uint8_t *)"35-",LEFT_MODE);
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
	//for (int i = 0; i<N;i++){
		//FO[i] = F[i];
	//}
	for (int i = 0; i < (N); i++){
		FO[i] =  3*20*log10(F[i]/(N));
		//FO[i] =  (F[i]/(22));
	}

//Now plot the data
	int j = 61;
	for (int i = 0; i < (N/2); i++){
		//We are making every 2 pixels 1 data bin.
		//Add a loop here to make sure it uses up the full available space
		while (j < (i+1)*(256/(N/2)) + 61){
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_DrawVLine(j, 41,  200);
			BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
			if (F[i] >=1){
				BSP_LCD_DrawVLine(j, 41,  F[i]);
			}
			j = j+1;
		}
	}

}
//////Setup axes for time domain
void LCD_SetupTimeAxes(void)
{
	BSP_LCD_Clear(LCD_COLOR_BLACK);  //Clear and set the colour
	BSP_LCD_SetTextColor(LCD_COLOR_RED); //Set the axis colour
	BSP_LCD_DrawHLine(60,40,260); //Draw the axis
	BSP_LCD_DrawVLine(60,40,200);

	//AXIS LABELS
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(1,200,(uint8_t *)"Ampl.",LEFT_MODE);
	BSP_LCD_DisplayStringAt(70,0,(uint8_t *)"Time (ms)",CENTER_MODE);
	//AXIS VALUES
	BSP_LCD_SetFont(&Font12);
	//X-AXIS
	//Label every 1ms
	BSP_LCD_DisplayChar(55,18,0x30);
	BSP_LCD_DisplayStringAt(84,8,(uint8_t *)"1",LEFT_MODE);
	BSP_LCD_DisplayStringAt(84,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(108,8,(uint8_t *)"2",LEFT_MODE);
	BSP_LCD_DisplayStringAt(108,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(132,8,(uint8_t *)"3",LEFT_MODE);
	BSP_LCD_DisplayStringAt(132,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(156,8,(uint8_t *)"4",LEFT_MODE);
	BSP_LCD_DisplayStringAt(156,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(180,8,(uint8_t *)"5",LEFT_MODE);
	BSP_LCD_DisplayStringAt(180,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(204,8,(uint8_t *)"6",LEFT_MODE);
	BSP_LCD_DisplayStringAt(204,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(228,8,(uint8_t *)"7",LEFT_MODE);
	BSP_LCD_DisplayStringAt(228,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(252,8,(uint8_t *)"8",LEFT_MODE);
	BSP_LCD_DisplayStringAt(252,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(276,8,(uint8_t *)"9",LEFT_MODE);
	BSP_LCD_DisplayStringAt(276,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(300,8,(uint8_t *)"10",LEFT_MODE);
	BSP_LCD_DisplayStringAt(300,20,(uint8_t *)"|",LEFT_MODE);
	//Y-AXIS
	//Label every 5dB
	BSP_LCD_DisplayStringAt(31,35,(uint8_t *)"-10-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,50,(uint8_t *)"-5-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(46,65,(uint8_t *)"0-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(46,80,(uint8_t *)"5-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,95,(uint8_t *)"10-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,110,(uint8_t *)"15-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,125,(uint8_t *)"20-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,140,(uint8_t *)"25-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,155,(uint8_t *)"30-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,170,(uint8_t *)"35-",LEFT_MODE);
}
///// Plot the time domain
void PlotTimeFunction(float T[N]){

//Scale F to the desired values
	float TO [N];
	/*for (int i = 0; i<N;i++){
		FO[i] = F[i];
	}*/
	for (int i = 0; i < (N); i++){
		TO[i] =  1000*(T[i])+50;
		//FO[i] =  (F[i]/(22));
	}

//Now plot the data
	int j = 61;
	for (int i = 0; i < (N/2); i++){
		//We are making every 2 pixels 1 data bin.
		//Add a loop here to make sure it uses up the full available space
		while (j < (i+1)*(256/(N/2)) + 61){
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_DrawVLine(j, 41,  200);
			BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
			if (TO[i] >=1){
				BSP_LCD_DrawVLine(j, 41,  TO[i]);
			}
			j = j+1;
		}
	}

}
//// Setup FIR Plot Axes
void LCD_SetupTimeFIRAxes(void)
{
	BSP_LCD_Clear(LCD_COLOR_BLACK);  //Clear and set the colour
	BSP_LCD_SetTextColor(LCD_COLOR_ORANGE); //Set the axis colour
	BSP_LCD_DrawHLine(60,40,260); //Draw the axis
	BSP_LCD_DrawVLine(60,40,200);

	//AXIS LABELS
	BSP_LCD_SetFont(&Font16);
	BSP_LCD_DisplayStringAt(1,200,(uint8_t *)"Ampl.",LEFT_MODE);
	BSP_LCD_DisplayStringAt(1,180,(uint8_t *)"(m)",LEFT_MODE);
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_DisplayStringAt(70,0,(uint8_t *)"Coefficient",CENTER_MODE);
	//AXIS VALUES
	BSP_LCD_SetFont(&Font8);
	//X-AXIS
	//Label every 10 samples
	BSP_LCD_DisplayChar(55,18,0x30);
	BSP_LCD_DisplayStringAt(75,8,(uint8_t *)"10",LEFT_MODE);
	BSP_LCD_DisplayStringAt(75,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(95,8,(uint8_t *)"20",LEFT_MODE);
	BSP_LCD_DisplayStringAt(95,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(115,8,(uint8_t *)"30",LEFT_MODE);
	BSP_LCD_DisplayStringAt(115,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(135,8,(uint8_t *)"40",LEFT_MODE);
	BSP_LCD_DisplayStringAt(135,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(155,8,(uint8_t *)"50",LEFT_MODE);
	BSP_LCD_DisplayStringAt(155,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(175,8,(uint8_t *)"60",LEFT_MODE);
	BSP_LCD_DisplayStringAt(175,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(195,8,(uint8_t *)"70",LEFT_MODE);
	BSP_LCD_DisplayStringAt(195,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(215,8,(uint8_t *)"80",LEFT_MODE);
	BSP_LCD_DisplayStringAt(215,20,(uint8_t *)"|",LEFT_MODE);
	BSP_LCD_DisplayStringAt(235,8,(uint8_t *)"90",LEFT_MODE);
	BSP_LCD_DisplayStringAt(235,20,(uint8_t *)"|",LEFT_MODE);


	BSP_LCD_SetFont(&Font12);
	//Y-AXIS
	//Label every 5dB
	//BSP_LCD_DisplayStringAt(31,35,(uint8_t *)"-10-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(32,56,(uint8_t *)"-20-",LEFT_MODE);
	//BSP_LCD_DisplayStringAt(46,65,(uint8_t *)"0-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(46,75,(uint8_t *)"0-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,94,(uint8_t *)"20-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,113,(uint8_t *)"40-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,132,(uint8_t *)"60-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(39,151,(uint8_t *)"80-",LEFT_MODE);
	//BSP_LCD_DisplayStringAt(39,155,(uint8_t *)"30-",LEFT_MODE);
	BSP_LCD_DisplayStringAt(32,170,(uint8_t *)"100-",LEFT_MODE);

	//BSP_LCD_DisplayStringAt(31,35,(uint8_t *)"-10-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(39,50,(uint8_t *)"-5-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(46,65,(uint8_t *)"0-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(46,80,(uint8_t *)"5-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(39,95,(uint8_t *)"10-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(39,110,(uint8_t *)"15-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(39,125,(uint8_t *)"20-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(39,140,(uint8_t *)"25-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(39,155,(uint8_t *)"30-",LEFT_MODE);
		//BSP_LCD_DisplayStringAt(39,170,(uint8_t *)"35-",LEFT_MODE);
}
///// Plot Time Domain FIR
void PlotTimeFIRFunction(float FIR[N]){

//Scale F to the desired values
	float FIRO [N];
	/*for (int i = 0; i<N;i++){
		FO[i] = F[i];
	}*/
	for (int i = 0; i < (N); i++){
		FIRO[i] =  1000*(FIR[i])+50;
		//FO[i] =  (F[i]/(22));
	}

//Now plot the data
	int j = 61;
	for (int i = 0; i < (N/2); i++){
		//We are making every 2 pixels 1 data bin.
		//Add a loop here to make sure it uses up the full available space
		while (j < (i+1)*(256/(N/2)) + 61){
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_DrawVLine(j, 41,  200);
			BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
			if (FIRO[i] >=-0.03){
				BSP_LCD_DrawVLine(j, 41,  FIRO[i]);
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

	    adcChannel.Channel = ADC_CHANNEL_11;
	    adcChannel.Rank = 1;
	    adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	    adcChannel.Offset = 0;

	    if (HAL_ADC_ConfigChannel(&g_AdcHandle, &adcChannel) != HAL_OK)
	    {
	        asm("bkpt 255");
	    }
}

///////////Filter coefficients
void FIRCoefficientsCalc(void)
{
	for (int n = 0; n<48; n++){
		if (n == 0){
			hd[47-n] = 2*fc;
		}
		else{
			hd[47-n] = (2*fc*sin(n*wc))/(n*wc);
		}
		wn[47-n] = 0.54 + (0.46*cos((2*M_PI*n)/FN));

		hn[47-n] = wn[47-n] * hd[47-n];

		//The data is mirrored on both sides of the arrray
		hd[47+n] = hd[47-n];
		wn[47+n] = wn[47-n];
		hn[47+n] = hn[47-n];
	}

}


/////////////////////////////////////END///////////////////////////////////////

