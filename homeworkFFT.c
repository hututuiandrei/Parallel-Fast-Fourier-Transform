#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <complex.h>
#include <string.h>

#define INT_BITS 32
 
typedef double complex cplx;

cplx *input;
cplx *output;
const char * f_in;
const char * f_out;

int P;
long N;

pthread_barrier_t barrier;

unsigned int reverse_the_bits(unsigned int num, unsigned int cover_bits)
{
	if(cover_bits <= 1) {

		return num;
	}
    unsigned int count = (INT_BITS -1); 
    unsigned int tmp = num;  
	     
    num >>= 1;
    
    while(num)
    {
       tmp <<= 1;	      
       tmp |= num & 1;    
       num >>= 1;      
       count--;
    }
    
    tmp <<= count;
    tmp >>= INT_BITS - cover_bits;
    
    return tmp;
}

void* threadFunction(void *var)
{
	int thread_id = *(int*)var;

	for(int k = 1; k < N; k *= 2) {

		if(thread_id >= 0) {

			pthread_barrier_wait(&barrier);
		}

		int step = N / k / 2;

		int start, end;

		if(thread_id >= 0) {

			start = ceil(thread_id*step/P);
			end = ceil((thread_id + 1)*step/P);
		} else {

			start = 0;
			end = step;
		}

		for(int j = start; j < end; j += 2) {

			unsigned int j_cpy = j;
			unsigned int offset = reverse_the_bits(j_cpy, log2(step));

			for (int i = 0; i < N; i += 2 * step) {

				int log = log2(k);

				if(log % 2 == 0) {

					cplx t = cexp(-I * M_PI * i / N) * output[i + step + offset];

					if(i/2 + offset < N) 
						input[i / 2 + offset] = output[i + offset] + t;

				
					if((i + N)/2 + offset < N) 
						input[(i + N)/2 + offset] = output[i + offset] - t;
					
				} else {

					cplx t = cexp(-I * M_PI * i / N) * input[i + step + offset];

					if(i/2 + offset < N) 
						output[i / 2 + offset] = input[i + offset] + t;
					
					if((i + N)/2 + offset < N) 
						output[(i + N)/2 + offset] = input[i + offset] - t;
				}
			}
		}
		if(thread_id >= 0) {

			pthread_barrier_wait(&barrier);
		}

		for(int j = start + 1; j < end; j += 2) {

			unsigned int j_cpy = j;
			unsigned int offset = reverse_the_bits(j_cpy, log2(step));

			for (int i = 0; i < N; i += 2 * step) {

				int log = log2(k);

				if(log % 2 == 0) {

					cplx t = cexp(-I * M_PI * i / N) * output[i + step + offset];

					if(i/2 + offset < N) 
						input[i / 2 + offset] = output[i + offset] + t;
				
					if((i + N)/2 + offset < N) 
						input[(i + N)/2 + offset] = output[i + offset] - t;
					
				} else {

					cplx t = cexp(-I * M_PI * i / N) * input[i + step + offset];

					if(i/2 + offset < N) 
						output[i / 2 + offset] = input[i + offset] + t;
					
					if((i + N)/2 + offset < N) 
						output[(i + N)/2 + offset] = input[i + offset] - t;
				}
			}
		}
	}

	return NULL;
}

void iter_fft(int n) {

	if(P != 1) {

		pthread_barrier_init(&barrier, NULL, P);
		pthread_t tid[P];
		int thread_id[P];
		int i;
		for(i = 0;i < P; i++)
			thread_id[i] = i;

		for(i = 0; i < P; i++) {

			pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
		}

		for(i = 0; i < P; i++) {
			pthread_join(tid[i], NULL);
		}
		pthread_barrier_destroy(&barrier);
		
	} else {

		int no_threads = -1;

		threadFunction(&no_threads);
	}

	
	if((int)log2(n) % 2 == 0) {

		for (int i = 0; i < n; i++) input[i] = output[i];
	}
}

void getArgs(int argc, char **argv)
{

	if(argc < 4) {
		printf("Not enough paramters.");
		exit(1);
	}
	f_in = argv[1];
	f_out = argv[2];
	P = atoi(argv[3]);
}

void read_input() {

	FILE * fp_in = fopen(f_in, "r");

	if(fscanf(fp_in, "%ld", &N) > 0) {

		output = malloc(sizeof(*output) * N);
		input = malloc(sizeof(*input) * N);

		double temp;

		for(int i = 0; i < N; i++) {

			if(fscanf(fp_in, "%lf", &temp) > 0) {

				output[i] = temp;
				input[i] = temp;
			}
		}
	}
}
	
void write_output() {

	FILE * fp_out = fopen(f_out, "w");
	fprintf(fp_out, "%ld\n", N);

	for(int i = 0; i < N; i++) {

		fprintf(fp_out, "%lf %lf\n", creal(output[i]), cimag(output[i]));
	}	
}
 
int main(int argc, char *argv[])
{

	getArgs(argc, argv);

	read_input();

	iter_fft(N);

	write_output();

	free(input);
	free(output);
 
	return 0;
}