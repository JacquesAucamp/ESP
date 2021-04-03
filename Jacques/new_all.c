////////////////////////////START//////////////////////////////////////////////
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <complex.h>
#include <string.h>

///////////////////DECLARATIONS////////////////////////////////////////////////

int N = 8; // The order of the FFT

#define M_PI acos(-1.0)
#define I sqrt(-1)

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
	for (int i = 0; i<N ; i++){
		Y[i] = 2000*sin(2*M_PI*(f*t)*i)+2000;
	}



	//Now lets Define the Twiddle Factors
	float twiddle_real[N];			/*Create an empty complex array*/
	float twiddle_imag[N];
	for (int i = 0; i< N; i++){
		twiddle_real[i] = 0;
		twiddle_imag[i] = 0;
	}
	for (int i=0;i< 2*N;i++){
		float complex twiddle = expf(-((I*2*M_PI*i)/N));	/*Twiddle factor formula*/
		twiddle_real[i] = creal(twiddle);
		twiddle_imag[i] = cimag(twiddle);
	}


	//Now let's sort the data for butterfly processing
	float Ytemp [N];
	//float oddeven_arr[N];
	for (int i = 0; i < N; i++){
		Ytemp[i] = Y[i];
	}
	for (int i = 0; i < (int)(log2(N)-1); i++){
		int p = pow(2,i);
		float oddeven_arr[N/p];
		//oddeven function now
		for (int j = 0; j < N; j += (int)(N/p)){
			for (int k = j; k < (int)(j+(N/p)); k++){
				oddeven_arr[k] = Y[k];
			}
			int L = sizeof(oddeven_arr)/sizeof(float);
			float z[L];						/*empty array*/
			for (int i = 0; i< N; i++){
				z[i] = 0;
			}
			for (int i = 0; i < L/2; i++){			/*Only need to loop through half the*/
				z[i] = oddeven_arr[2*i];			/*length. Will place every even sample at the sfirst*/
				z[L-i-1] = oddeven_arr[L-(2*i)-1];	/*half and every odd sample at the second half*/
			}
			for (int k = j; k < (int)(j+(N/p)); k++){
				Ytemp[k] = z[k];
			}
		}
		for (int k = 0; k < sizeof(Ytemp)/sizeof(float); k++){
			Y[k] = Ytemp[k];
		}
	}
	/////////YEAH this will come later

	//Now let's do the Butterfly processing
	//start by declaring the variables and arrays that will be needed
	//float YSorted [16] = {1.0, 1.0000000000000013, 1.0000000000000007, 1.0000000000000029, 0.9999999999999997, 1.0000000000000007, 0.9999999999999986, 1.0000000000000009, 2.0, 2.0, 2.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.7071067811865475, 1.707106781186548, 1.707106781186548, 1.7071067811865475, 0.29289321881345254, 0.29289321881344976, 0.2928932188134521, 0.2928932188134524, 1.7071067811865472, 1.7071067811865475, 1.707106781186547, 1.7071067811865461, 0.2928932188134523, 0.2928932188134513, 0.29289321881345276, 0.29289321881345365};
	float YSorted [sizeof(Y)/sizeof(double)];
	for (int k = 0; k < sizeof(Y)/sizeof(float); k++){
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
		int s = pow(-2,i);					/*the skipping factor*/
		for (int j=0; j<N; j++){
			if (j%((int)pow(2,i))){	/*Make the  skipping factor + or -*/
				s = s * -1;
			}
			/*Now we need to get the correct twiddle factors*/
			if (s>0){						/*If we retrieving from bellow*/
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
			YSorted[i] = YT_real[i];
		}
	}
	for (int j=0; j<N; j++){
		Y[j] = YSorted[j];
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
	float FO [N];
	for (int i = 0; i<(N/2); i++){
		FO[i] =  (20*F[i]);
	}
//Now plot the data
	int j = 41;
	for (int i = 0; i < (N/2); i++){
		//We are making every 2 pixels 1 data bin.
		//Add a loop here to make sure it uses up the full available space
		while (j <= 2*((i+1)*(256/N))){
			BSP_LCD_DrawVLine(j, 31, (int) FO[i]);
			j = j+1;
		}
	}

}
//////////OddEven
//This function will divide an array by it's index.
//Odd indexes first, even indexes second

/////////////////////////////////////END///////////////////////////////////////

