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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>
			
/*===========================================================*/
/*Initialization
/*===========================================================*/

int N = 128;				/*128 points in FFT*/
#define M_PI acos(-1.0)
#define I sqrt(-1)



int main(void)
{
	/*Twiddle Factors*/
	float twiddle_real[N];
	float twiddle_imag[N];
	int i;
	for (i = 0; i< N; i++){
		twiddle_real[i] = 0;
		twiddle_imag[i] = 0;
	}
	for (i=0;i< 2*N;i++){
		float complex twiddle = expf(-((I*2*M_PI*i)/N));
		twiddle_real[i] = creal(twiddle);
		twiddle_imag[i] = cimag(twiddle);
	}

	for(;;);
}
