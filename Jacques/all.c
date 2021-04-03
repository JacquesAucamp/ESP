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

/*Variables and things*/
int N = 128;				/*128 points in FFT*/
#define M_PI acos(-1.0)
#define I sqrt(-1)
int Y[N];					/*to store incoming signal data*/
float YT[N];					/*Temporary storage*/
float YSorted[N];				/*Sorted Signal data*/
float YF[N];					/*Final FFTd data*/
float YT_spliced[N];
float Signal_spliced[N];
int twdAreal;
int twdAimag;
int twdBreal;
int twdBimag;
float YT_real[N];
float YT_imag[N];

/*Fake Input Signal*/
float Signal[N] = {0,0.25,0,75,1,0.75,0.5,0.25,0,-0.25,-0.5,-0.75,-1,-0.75,-0.5,-0.25,0,0.25,0,75,1,0.75,0.5,0.25,0,-0.25,-0.5,-0.75,-1,-0.75,-0.5,-0.25,0,0.25,0,75,1,0.75,0.5,0.25,0,-0.25,-0.5,-0.75,-1,-0.75,-0.5,-0.25,0,0.25,0,75,1,0.75,0.5,0.25,0,-0.25,-0.5,-0.75,-1,-0.75,-0.5,-0.25,0,0.25,0,75,1,0.75,0.5,0.25,0,-0.25,-0.5,-0.75,-1,-0.75,-0.5,-0.25,0,0.25,0,75,1,0.75,0.5,0.25,0,-0.25,-0.5,-0.75,-1,-0.75,-0.5,-0.25,0,0.25,0,75,1,0.75,0.5,0.25,0,-0.25,-0.5,-0.75,-1,-0.75,-0.5,-0.25,0,0.25,0,75,1,0.75,0.5,0.25,0,-0.25,-0.5,-0.75,-1,-0.75,-0.5,-0.25,0,0.25,0.5,0.75,1,0.75,0.5,0.25};

int oddeven(int arr[]){				/*Divide an array by it's indexes. Odd and even*/
	int L = len(arr);				/*length of the array*/
	int z[L];						/*empty array*/
	int i;
	for (i = 0; i< N; i++){
		z[i] = 0;
	}
	for (i=0;i<L/2;i++){			/*Only need to loop through half the*/
		z[i] = arr[2*i];			/*length. Will place every even sample at the sfirst*/
		z[L-i-1] = arr[L-(2*i)-1];	/*half and every odd sample at the second half*/
	}
	return z;
}

int main(void)
{
	/*Twiddle Factors*/
	/*Now we make a database  of the twiddle factors*/
	float twiddle_real[N];			/*Create an empty complex array*/
	float twiddle_imag[N];
	int i;
	for (i = 0; i< N; i++){
		twiddle_real[i] = 0;
		twiddle_imag[i] = 0;
	}
	for (i=0;i< 2*N;i++){
		float complex twiddle = expf(-((I*2*M_PI*i)/N));	/*Twiddle factor formula*/
		twiddle_real[i] = creal(twiddle);
		twiddle_imag[i] = cimag(twiddle);
	}

	/*Place Data in Desired order*/
	for (i=0;i=N;i++){
		YT[i] = Signal[i];		/*start by making a temp or a working array*/
	}

	for (i = 0; i< (log2(N))-1 ; i++){		/*loop for each subdivision*/
		int p = pow(2,i);					/*a division factor*/
		int j;
		int k;
		for (j = 0; j<N; j += N/p){			/*divide the subdivisions up*/
			for (k=j; k=j+(N/p)+(N/p);j += N/p){	/*Had to add extra loop here*/
				YT_spliced[k] = YT[k];			/*Please check logic*/
				Signal_spliced[k] = Signal[k];
			}
			YT_spliced = oddeven(Signal_spliced);
		}
		for (i=0;i=N;i++){
			Signal[i] = YT[i];
		}
	}
	for (i=0;i=N;i++){
		YSorted[i] = Signal[i];
	}

	/*FFT Algorithm*/


	for (i = 0; i< N; i++){		/*Make YT a empty complex array*/
		YT_real[i] = 0;
		YT_imag[i] = 0;
	}
	/*First we need to loop for each layer of the butterfly diagram*/
	for (i=0; i<log2(N); i++){
		int s = pow(-2,i);					/*the skipping factor*/
		int j;
		for (j=0; j<N; j++){
			if (i == 0 || j%((int)pow(2,i))){	/*Make the  skipping factor + or -*/
				s = s * -1;
			}
			/*Now we need to get the correct twiddle factors*/
			if (s>0){						/*If we retrieving from bellow*/
				twdAreal = 1;
				twdAimag = 0;
				twdBreal = twiddle_real[(int)(j*(N/(pow(2,i+1))))];
				twdBimag = twiddle_imag[(int)(j*(N/(pow(2,i+1))))];
			}
			else{							/*else retrieve from above*/
				twdBreal = 1;
				twdBimag = 0;
				twdAreal = twiddle_real[(int)(j*(N/(pow(2,i+1))))];
				twdAimag = twiddle_imag[(int)(j*(N/(pow(2,i+1))))];
			}
			YT_real[j] = (twdAreal*YSorted[j])+(twdBreal*YSorted[j+s]);
			YT_imag[j] = (twdAimag*YSorted[j])+(twdBimag*YSorted[j+s]);
		}
		for (i=0;i=N;i++){
			YSorted[i] = YT_real[i]+YT_imag[i];
		}
	}
	int j;
	for (j=0;j=(N/2)+1;j++){
		YF[j] = YSorted[j];
	}
}
