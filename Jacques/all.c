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
void PlotFunction(float F[N]);





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
	float Y [N];
	float t = 0.000025;//1/5000; //Minimum timestep
	int f = 500; //Define the signal frequency
	//Now assign it to a variable
	for (int i = 0; i<N ; i++){
		Y[i] = 2000*sin(2*M_PI*(f*t)*i)+2000;
		//Y[i] = 10*i;
	}

	float YOG [N];
	for (int i = 0; i<N; i++){
		YOG[i] = Y[i];
	}



	//Now lets Define the Twiddle Factors
	float twiddle_real[N];			/*Create an empty complex array*/
	float twiddle_imag[N];
	for (int i = 0; i< N; i++){
		twiddle_real[i] = 0;
		twiddle_imag[i] = 0;
	}
	float complex twiddle;
	for (int i=0; i< N; i++){
		twiddle = expf(-((I*2*M_PI*i)/N));	/*Twiddle factor formula*/
		twiddle_real[i] = creal(twiddle);
		twiddle_imag[i] = cimag(twiddle);
	//	twiddle_real[i] = cos((2*M_PI*i)/N);
	//	twiddle_imag[i] = sin((2*M_PI*i)/N);
	}



	//PlotFunction(Y);

	//Now let's sort the data for butterfly processing
	float Ytemp [N];

	for (int i = 0; i < (log2(N)); i++){
		int p = pow(2,i);
		float oddeven_arr[N/p];
				//oddeven function now
		for (int j = 0; j < N; j = j + (N/p)){
			for (int k = j; k < j+(N/p); k++){//Divide up the data
				oddeven_arr[k] = Y[k];

			}

			int L = N/p;//sizeof(oddeven_arr);
			float z[L];						/*empty array*/
			for (int k = j; k < L/2; k++){			/*Only need to loop through half the*/
				z[k] = oddeven_arr[2*k];			/*length. Will place every even sample at the sfirst*/
				z[L-k-1] = oddeven_arr[L-(2*k)-1];	/*half and every odd sample at the second half*/
			}
			//for (int k =0; k < L/2; k++){
				//z[L-k-1] = oddeven_arr[L-(2*k)-1];
			//}

			for (int k = j; k < j + (N/p); k++){//Return the ordered sata
				Ytemp[k] = z[k];
			}
		}
		for (int k = 0; k < N; k++){
			Y[k] = Ytemp[k];
		}
	}


	//PlotFunction(Y);






	//Now let's do the Butterfly processing
	//start by declaring the variables and arrays that will be needed
	//float YSorted [16] = {1.0, 1.0000000000000013, 1.0000000000000007, 1.0000000000000029, 0.9999999999999997, 1.0000000000000007, 0.9999999999999986, 1.0000000000000009, 2.0, 2.0, 2.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.7071067811865475, 1.707106781186548, 1.707106781186548, 1.7071067811865475, 0.29289321881345254, 0.29289321881344976, 0.2928932188134521, 0.2928932188134524, 1.7071067811865472, 1.7071067811865475, 1.707106781186547, 1.7071067811865461, 0.2928932188134523, 0.2928932188134513, 0.29289321881345276, 0.29289321881345365};
	float YSorted [sizeof(Y)/sizeof(float)];
	for (int k = 0; k < sizeof(Y)/sizeof(float); k++){
		YSorted[k] = Y[k];
	}
	float YT_real [N];
	float YT_imag [N];
	float Ytwiddled [N];
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
				twdAimag = 1;
				twdBreal = twiddle_real[((int)(j*(N/(pow(2,i+1))))%N)];
				twdBimag = twiddle_imag[((int)(j*(N/(pow(2,i+1))))%N)];
			}
			else{							/*else retrieve from above*/
				twdBreal = 1;
				twdBimag = 1;
				twdAreal = twiddle_real[((int)(j*(N/(pow(2,i+1))))%N)];
				twdAimag = twiddle_imag[((int)(j*(N/(pow(2,i+1))))%N)];
			}
			YT_real[j] = (twdAreal*YSorted[j])+(twdBreal*YSorted[j+s]);
			YT_imag[j] = (twdAimag*YSorted[j])+(twdBimag*YSorted[j+s]);
		}
		for (int i=0; i<N; i++){
			Ytwiddled[i] = YT_real[i]+YT_imag[i];
		}
	}
	for (int j=0; j<N; j++){
		Y[j] = 20.0*Ytwiddled[j];
	}

	PlotFunction(Y);


//Now we will enter into the main loop of the program
while(1){


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
void PlotFunction(float F[N]){
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
//Scale F to the desired values
	float FO [N];
	for (int i = 0; i < (N); i++){
		FO[i] =  (F[i]/(22*N));
	}
//Now plot the data
	int j = 41;
	for (int i = 0; i < (N); i++){
		//We are making every 2 pixels 1 data bin.
		//Add a loop here to make sure it uses up the full available space
		BSP_LCD_DrawVLine(j, 31,  FO[i]);
		j = j+1;
		BSP_LCD_DrawVLine(j, 31,  FO[i]);
		j = j+1;


		//BSP_LCD_DrawVLine(j, 31,  FO[i]);
		//j = j+1;

	}

}

/////////////////////////////////////END///////////////////////////////////////
