#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>


#define MAX_Y 4000
#define MAX_X MAX_Y
#define MAX_THREAD_NUM 40


void GDB_Break(){}

typedef struct DistributionPlan {
	int thread_pos;
	int col_start, col_end;
} DistributionPlan;

DistributionPlan* MakeInitialPlan(int size, int thread_num);
int ReturnThreadNumFromArgvs(int argc, char** argv);


bool debug_mode = false;
pthread_t threads[MAX_THREAD_NUM] = {0,};
void MakeMatrixFile(char filename[]);
void ReadMatrixFile(char filename[], int mat[MAX_Y][MAX_X]);
void DistributeHalf(DistributionPlan* plan);
void* ExecutePlan(void* void_plan);

FILE* fp_a;
FILE* fp_b;

int A[MAX_Y][MAX_X];
int B[MAX_Y][MAX_X];
long long int C[MAX_Y][MAX_X + 1];

DistributionPlan* MakeInitialPlan(int size, int thread_num){
	DistributionPlan* plan = malloc(sizeof(DistributionPlan));
	plan->col_start 	= 0;
	plan->col_end		= size;
	plan->thread_pos	= thread_num -1;

	threads[thread_num -1] = pthread_self();

	return plan;
}

int ReturnThreadNumFromArgvs(int argc, char** argv){
	/*
	if (2 < argc){
		if( strcmp(argv[2], "debug") == 0){
			debug_mode = true;
		}
	}*/
	if (1 < argc){
		return atoi(argv[1]);
	} else {
		return 1;
	}
}

void MakeMatrixFile(char filename[]) {
	FILE* fp_temp =  fopen(filename, "w");
	int x, y, val;
	for (y = 0; y < MAX_Y; y++) {
		for (x = 0; x < MAX_X; x++) {
			val = rand() % 10000;	// 0 ~ 9999
			fprintf(fp_temp, "%6d ", val);
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
		perror("ReadMatrixFile() failed!\n");
		exit(1);
	}

	int x, y;
	for (y = 0; y < MAX_Y; y++) {
		for (x = 0; x < MAX_X; x++) {
			fscanf(fp_temp, "%d", &(mat[y][x]));
		}
	}

}


/*
void DistributeHalf(DistributionPlan* plan) {
	int width = plan->col_start - plan->col_end;
	

	DistributionPlan * qlan = (DistributionPlan*) malloc(sizeof(DistributionPlan));
	*qlan =  *plan;		//qlan is new plan for new thread
	plan->col_start		+= width / 2;
	qlan->col_end		=  plan->col_start;
	qlan->distribute_to	/= 2;
	plan->distribute_to	-= qlan->distribute_to;
	qlan->thread_pos	-= plan->thread_pos - plan->distribute_to;
	
	pthread_create(&(threads[qlan->thread_pos]), NULL, ExecutePlan, (void*) qlan);	
}
*/
void* ExecutePlan(void* void_plan) {
	DistributionPlan * plan = (DistributionPlan*) void_plan;

	//if(debug_mode){printf("start thread%2d : %5d~%5d\n", plan->thread_pos, plan->col_start, plan->col_end);}
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
	//free(plan);
}

int main(int argc, char** argv) {
	int thread_num = 1;
	thread_num = ReturnThreadNumFromArgvs(argc, argv);
	srand(0);	// fixed
	if(debug_mode){ printf("start reading a.txt\n"); }
	ReadMatrixFile("a.txt", A);
	if(debug_mode){ printf("start reading b.txt\n"); }
	ReadMatrixFile("b.txt", B);
	
	if(debug_mode){
		printf("thread_num = %d\n", thread_num);
		//printf("plan->col_start %d, plan->col_end %d, plan->thread_pos %d, plan->distribute_to %d\n", plan->col_start, plan->col_end, plan->thread_pos, plan->distribute_to);
	}


	DistributionPlan plans[MAX_THREAD_NUM];
	
	//MAIN PART. start TIMER here
	time_t start, end;
	time(&start);

	int i, gap;
	gap = MAX_Y / thread_num;
	if(debug_mode) { printf("gap == %d\n",gap);}
	for(i=0; i < thread_num -1; i++){
		plans[i].col_start	= gap * i;
		plans[i].col_end	= gap * (i+1);
		plans[i].thread_pos	= i;
		//if(debug_mode) {printf("plan->col_start %d, plan->col_end %d, plan->thread_pos %d, plan->distribute_to %d\n", plans[i].col_start, plans[i].col_end, plans[i].thread_pos, plans[i].distribute_to);}
		pthread_create(&threads[i], NULL, ExecutePlan, &plans[i]);
	}
	plans[i].col_start	= gap * i;
	plans[i].col_end	= MAX_Y;	
	plans[i].thread_pos	= i;

	//if(debug_mode) {printf("plan->col_start %d, plan->col_end %d, plan->thread_pos %d, plan->distribute_to %d\n", plans[i].col_start, plans[i].col_end, plans[i].thread_pos, plans[i].distribute_to);}
	pthread_create(&threads[i], NULL, ExecutePlan, &plans[i]);

	//TODO: check end
	long long int  sum = 0, j;
	for (i = 0; i < thread_num; i++){
		void* ret;
		pthread_join(threads[i], &ret);
		
		for(j = plans[i].col_start; j < plans[i].col_end; j++){
			//if(debug_mode) {printf("thread[%3d] joined, rowsum = %d\n", i, C[j][MAX_X]);}
			sum += C[j][MAX_X];
		}
		//if(debug_mode) {printf("thread[%3d] joined, current sum = %10lld\n", i, sum);}
	}
	time(&end);
	double time_cost = difftime(end, start);
	printf("thread_num ==%2d\t time_cost == %fsec\t", thread_num, time_cost);
	printf("total sum == %lld\n", sum);

	return 0;
}









