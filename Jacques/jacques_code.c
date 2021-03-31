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

/* FFT settings */
#define SAMPLES                    256             /* 128 real party and 128 imaginary parts */
#define FFT_SIZE                SAMPLES / 2        /* FFT size is always the same size as we have samples, so 128 in our case */
 
#define FFT_BAR_MAX_HEIGHT        120             /* 120 px on the LCD */			

int * OddEven(arr):
	L = len(arr)
	int z[L] = {0};

	for i in range(0,int(len(arr)/2)):
			z[i] = arr[2*i];
			z[L-i-1] = arr[L-(2*i)-1]
	return z;


int main(void)
{
	for(;;);
	
	/*System Initialization*/
	SystemInit();
	
	/* Initialize LED's on board */
	TM_DISCO_LedInit();
	
	/* Initialize LCD */
	TM_ILI9341_Init();
	TM_ILI9341_Rotate(TM_ILI9341_Orientation_Landscape_1);
	
	/* Initialize ADC, PA0 is used */
	TM_ADC_Init(ADC1, ADC_Channel_0);
	
	
}
