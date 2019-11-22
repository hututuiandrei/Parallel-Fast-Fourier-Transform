#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <complex.h>
#include <math.h>

typedef double complex cplx;

int N;
int P;
cplx *input;
cplx *output;
const char * f_in;
const char * f_out;

void* threadFunction(void *var)
{
	int thread_id = *(int*)var;

	int start = ceil(thread_id*N/P);
	int end = ceil((thread_id + 1)*N/P);

	for (int k = start; k < end; k++) { 
	 // For each output element
		cplx sum = 0.0;
		for (int t = 0; t < N; t++) {
		  // For each input element
			double angle = 2 * M_PI * t * k / N;
			sum += input[t] * cexp(-angle * I);
		}
		output[k] = sum;
	}	
	return NULL;
}

void show(const char * s, cplx buf[]) {

	printf("%s", s);
	for (int i = 0; i < N; i++)
		if (!cimag(buf[i]))
			printf("%g ", creal(buf[i]));
		else
			printf("(%g, %g) ", creal(buf[i]), cimag(buf[i]));
	printf("\n");
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
	if(fscanf(fp_in, "%d", &N) > 0) {
		double temp;

		input = malloc(sizeof(*input) * N);
		output = malloc(sizeof(*input) * N);

		for(int i = 0; i < N; i++) {

			if(fscanf(fp_in, "%lf", &temp) > 0) {
				
				input[i] = temp;
			}
		}
	}
}
	
void write_output() {

	FILE * fp_out = fopen(f_out, "w");
	fprintf(fp_out, "%d\n", N);

	for(int i = 0; i < N; i++) {

		fprintf(fp_out, "%lf %lf\n", creal(output[i]), cimag(output[i]));

	}	
}

int main(int argc, char *argv[])
{

	getArgs(argc, argv);
	read_input();

	int i;

	pthread_t tid[P];
	int thread_id[P];
	for(i = 0;i < P; i++)
		thread_id[i] = i;

	for(i = 0; i < P; i++) {
		pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
	}

	for(i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}
	write_output();

	free(input);
	free(output);
 
	return 0;
}