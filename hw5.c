#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>


#define MAX_Y 4000
#define MAX_X MAX_Y
#define MAX_THREAD_NUM 40

pthread_t threads[MAX_THREAD_NUM] = {0,};

typedef struct DestributionPlan {
	int thread_pos;
	int destribute_to;
	int col_start, col_end;
} DestributionPlan;


FILE* fp_a;
FILE* fp_b;
//FILE* fp_c;

int A[MAX_Y][MAX_X];
int B[MAX_Y][MAX_X];
int C[MAX_Y][MAX_X + 1];

void MakeMatrixFile(char filename[]) {
	FILE* fp_temp =  fopen(filename, "w");
	int x, y, val;
	for (y = 0; y < MAX_Y; y++) {
		for (x = 0; x < MAX_X; x++) {
			val = rand() % 10;	// 0 ~ 9
			fprintf(fp_temp, "%d ", val);
		}
		fprintf(fp_temp, "\n");
	}
	fclose(fp_temp);
};

void ReadMatrixFile(char filename[], int mat[MAX_Y][MAX_X]) {
	FILE* fp_temp = NULL;
	fp_temp = fopen(filename, "r");
	if (fp_temp == NULL) {
		MakeMatrixFile(filename);
		fp_temp = fopen(filename, "r");
	}
	if (fp_temp == NULL) {
		perror("ReadMatrixFile(%s) failed!\n", filename);
		exit(1);
	}

	int x, y;
	for (y = 0; y < MAX_Y; y++) {
		for (x = 0; x < MAX_X; x++) {
			fscanf(fp_temp, "%d", &(mat[y][x]));
		}
	}

}

void DestributeHalf(DestributionPlan plan) {
	int cols = plan.col_start - plan.col_end;
	DestributionPlan qlan = plan;		//qlan is new plan for new thread
	qlan.col_end		-= cols / 2;
	plan.col_start		+= cols / 2;
	qlan.destribute_to	/= 2;
	plan.destribute_to	-= qlan.destribute_to;
	qlan.thread_pos		+= plan.destribute_to;
	
	pthread_create();
	
}

void* Plan2Thread(DestributionPlan* plan) {
	threads[(int)plan->thread_pos] = pthread_self();

	while (plan->destribute_to == 1 ) {	//
		DestrbuteHalf(plan);
	}
	int x, y, i;
	for (y = plan->col_start; y < plan->col_end; y++) {
		C[y][MAX_X] = 0;			//row sum
		for (x = 0; x < MAX_X; x++) {
			C[y][x] = 0;			
			for (i = 0; i < MAX_X; i++) {
				C[y][x] += A[y][i] * B[i][x];
			}						// C[y][x] calc fin
			C[y][MAX_X] += C[y][x];	
		}							// row sum calc fin
	}								// my job fin

}

int main(int argc, char** argv) {
	
	srand(0);	// fixed
	ReadMatrixFile("a.txt", A);
	ReadMatrixFile("b.txt", B);
	
	int thread_num = 1;
	// set thread_num here


	DestributionPlan plan;
	plan.col_start	= 0;
	plan.col_end = MAX_Y;
	plan.destribute_to = thread_num;
	plan.thread_pos = 0;

	Plan2Thread(&plan);

	return 1;
}
