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
int Y[N];					/*to store data*/
int twdAreal;
int twdAimag;
int twdBreal;
int twdBimag;
float YT_real[N];
float YT_imag[N];

int oddeven(int arr[]){
	int L = len(arr);
	int z[L];
	int i;
	for (i = 0; i< N; i++){
		z[i] = 0;
	}
	for (i=0;i<L/2;i++){
		z[i] = arr[2*i];
		z[L-i-1] = arr[L-(2*i)-1];
	}
	return z;
}

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

	/*Place Data in Desired order*/
	int YT[N];
	for (i = 0; i< (log2(N))-1 ; i++){
		int p = pow(2,i);
		int j;
		for (j = 0; j<N; j += N/p){
			YT[j:(j+(N/p))] = oddeven(Y[j:(j+(N/p))]);
		}
		Y = YT;
	}
	int YSorted[] = Y;
	for(;;);

	/*FFT Algorithm*/

	int i;
	for (i = 0; i< N; i++){
		YT_real[i] = 0;
		YT_imag[i] = 0;
	}
	for (i=0; i<log2(N); i++){
		int s = pow(-2,i);
		int j;
		for (j=0; j<N; j++){
			if (i == 0 || j%(pow(2,i))){
				s = s * -1;
			}
			if (s>0){
				twdAreal = 1;
				twdAimag = 0;
				twdBreal = twiddle_real[j*(N/(pow(2,i+1)))];
				twdBimag = twiddle_imag[j*(N/(pow(2,i+1)))];
			}
			else{
				twdBreal = 1;
				twdBimag = 0;
				twdAreal = twiddle_real[j*(N/(pow(2,i+1)))];
				twdAimag = twiddle_imag[j*(N/(pow(2,i+1)))];
			}
			YT_real[j] = (twdAreal*YSorted[j])+(twdBreal*YSorted[j+s]);
			YT_imag[j] = (twdAimag*YSorted[j])+(twdBimag*YSorted[j+s]);
		}
	YSorted = YT_real[j]+YT_imag[j];
	}
}
