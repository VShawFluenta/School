/* 	
	A Simple Simulation of Airline Checkin System
	CSC 360 Programming Assignment2
	Authored by Huan Wang
 */
 
#include "acs.h"
#include "queue.h"

struct timeval init_time;
int queue_length[NQUEUE];
int queue_status[NQUEUE]; // BUSY or IDLEs
int q_signal[NQUEUE]; // IF winner is selected already
double overall_waiting_time;
Queue real_queue[NQUEUE];

pthread_mutex_t waiting_time_mtex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t queue_mtex[NQUEUE];
pthread_cond_t queue_cond[NQUEUE];

pthread_cond_t clerk_cond[NCLERK]; //customers using this convar to awake clerk job is done
pthread_mutex_t clerk_mtex[NCLERK];

int main(int argc, char *args[]) {
	
	FILE *fp = NULL;
	char *pline = NULL;
	size_t len = 0;
	int i, count = -1, nCustomers = 0;
	struct info_node * info_list;
	pthread_t * thread_idList = NULL;
	int clerkNum[NCLERK];

	if(!thread_lock_init_()){
		perror("Lock init failed");
		exit(EXIT_FAILURE);
	}
	
	for(i = 0; i < NQUEUE; i++){
		initQueue(&real_queue[i]);
	}

	if((fp = fopen(args[1], "r")) == NULL){
		
		fprintf(stdout, "File Specified Not Exist\n");
		exit(EXIT_FAILURE);
	}
	
	while(getline(&pline, &len, fp) > 0){
		if(count == -1){
			pline[strlen(pline) - 1] = '\0';
			if((nCustomers = atoi(pline)) < 1){
				fprintf(stderr, "wrong customer number\n");
				exit(EXIT_FAILURE);
			}
			info_list = (struct info_node *)malloc(sizeof(struct info_node) * nCustomers);
			count++;
		}
		else{
			info_list[count].user_id = atoi(strtok(pline, ":"));
			info_list[count].priority = atoi(strtok(NULL, ","));
			info_list[count].arrival_time = atoi(strtok(NULL, ","));
			info_list[count++].service_time = atoi(strtok(NULL, ","));
		}
	}
	fclose(fp);
	qsort(info_list, nCustomers, sizeof(struct info_node), q_comp);
	
	gettimeofday(&init_time, NULL); // simulation starts
	
	thread_idList = (pthread_t *)malloc((nCustomers + NCLERK)*sizeof(pthread_t));
	for(i = 0; i < (nCustomers + NCLERK); i++){
		if(i < NCLERK) {
			clerkNum[i] = i;
			if(pthread_create(thread_idList + i, NULL, clerk_entry, (void *)&clerkNum[i]) != 0){
				perror("pthread create failed");
				exit(EXIT_FAILURE);
			}
			continue;
		}
		else{
			if(pthread_create(thread_idList + i, NULL, customer_entry, (void *)&info_list[i-NCLERK]) != 0){
				perror("pthread create failed");
				exit(EXIT_FAILURE);
			}
		}
	}
	for(i = NCLERK; i < nCustomers + NCLERK; i++){
		pthread_join(thread_idList[i], NULL);
	}

	printf("All jobs done...\n\nThe average waiting time for all customers in system is: %.2f seconds\n",\
	overall_waiting_time / nCustomers);
	exit(EXIT_SUCCESS);
	return 0;
}

void * customer_entry(void * cus_info) {
	
	int * p_queue_picked = NULL;
	int queue_num, clerk_id;
	struct info_node * p_myInfo = (struct info_node *) cus_info;
	double cur_seconds = 0.0, queue_enter_time = 0.0;
	//printf("I am cus %d, with arrival time: %d, service time: %d\n", p_myInfo->user_id, p_myInfo->arrival_time, p_myInfo->service_time);
	usleep(p_myInfo->arrival_time * 100000);
	fprintf(stdout, "A customer arrives: customer ID %2d. \n", p_myInfo->user_id);
	//p_queue_picked = findShortestQueue();
	
	queue_num = p_myInfo->priority;
	
	pthread_mutex_lock(&queue_mtex[queue_num]);
	{
		fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n",\
		queue_num, queue_length[queue_num]);
		
		enQueue(&real_queue[queue_num], p_myInfo->user_id);
		queue_enter_time = getCurSeconds();
		queue_length[queue_num]++;
		
		while(TRUE){
			pthread_cond_wait(&queue_cond[queue_num], &queue_mtex[queue_num]);
			if(iCameFirst(p_myInfo->user_id, queue_num) && q_signal[queue_num] == TRUE){
				deQueue(&real_queue[queue_num]);
				queue_length[queue_num]--;
				q_signal[queue_num] = FALSE;
				break;
			}
		}
	}
	pthread_mutex_unlock(&queue_mtex[queue_num]);
	usleep(10);
	if((clerk_id = getClerkAwakingMe(queue_num)) == -1){
		fprintf(stderr, "getClerkAwakingMe failed\n");
		pthread_exit(NULL);
	}
	
	cur_seconds = getCurSeconds();
	pthread_mutex_lock(&waiting_time_mtex);
	overall_waiting_time += (cur_seconds - queue_enter_time);
	pthread_mutex_unlock(&waiting_time_mtex);
	
	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n",\
	cur_seconds, p_myInfo->user_id, clerk_id);
	
	usleep(p_myInfo->service_time * 100000);
	
	cur_seconds = getCurSeconds();
	fprintf(stdout, "-->>> A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n",\
	cur_seconds, p_myInfo->user_id, clerk_id);
	
	//pthread_mutex_lock(&queue_mtex[queue_num]);
	//pthread_mutex_unlock(&queue_mtex[queue_num]);
	
	pthread_cond_signal(&clerk_cond[clerk_id]);
	
	pthread_exit(NULL);
	
	return NULL;
}

void *clerk_entry(void * clerkNum) {
	
	int queue_num;
	int clerk_num = *((int *)clerkNum);
	while(TRUE){
		queue_num = findQ_toServe(clerk_num);
		if(queue_num == -1){
			usleep(10);
			//printf("clerk %d, continue\n", clerk_num);
			continue;
		}
		pthread_mutex_lock(&queue_mtex[queue_num]);
		pthread_cond_broadcast(&queue_cond[queue_num]);
		q_signal[queue_num] = TRUE;
		pthread_mutex_unlock(&queue_mtex[queue_num]);
		
		pthread_mutex_lock(&clerk_mtex[clerk_num]);
		pthread_cond_wait(&clerk_cond[clerk_num], &clerk_mtex[clerk_num]);
		pthread_mutex_unlock(&clerk_mtex[clerk_num]);
	}
	
	pthread_exit(NULL);
	
	return NULL;
}

int findQ_toServe(int clerk_num){
	
	// Check business class-----------------
	
	while (1) {
		pthread_mutex_lock(&queue_mtex[0]);
		if (queue_status[0] != IDLE){
			pthread_mutex_unlock(&queue_mtex[0]);
			usleep(10);
			continue;
		}
		if (queue_length[0] > 0){
			queue_status[0] = clerk_num;
			pthread_mutex_unlock(&queue_mtex[0]);
			return 0;
		}
		else{
			pthread_mutex_unlock(&queue_mtex[0]);
			break;
		}
	}
	
	// Economy class -----------------------------------------
	
	while (1) {
		pthread_mutex_lock(&queue_mtex[1]);
		if (queue_status[1] != IDLE ){
			pthread_mutex_unlock(&queue_mtex[1]);
			usleep(10);
			continue;
		}
		if (queue_length[1] > 0){
			queue_status[1] = clerk_num;
			pthread_mutex_unlock(&queue_mtex[1]);
			return 1;
		}
		else{
			pthread_mutex_unlock(&queue_mtex[1]);
			return -1;
		}
	}
	
	return -1;
}

int q_comp(const void *p1, const void *p2){

	struct info_node *pnode1 = (struct info_node *)p1;
	struct info_node *pnode2 = (struct info_node *)p2;
	
	return pnode1->arrival_time - pnode2->arrival_time;
}

uint8_t thread_lock_init_(){
	int i;
	for(i = 0; i < NQUEUE; i++){
		if(pthread_mutex_init(&queue_mtex[i], NULL) != 0){
			return FALSE;
		}
		if(pthread_cond_init(&queue_cond[i], NULL) != 0){
			return FALSE;
		}
		queue_status[i] = IDLE;
	}
	for(i = 0; i < NCLERK; i++){

		if(pthread_mutex_init(&clerk_mtex[i], NULL) != 0){
			return FALSE;
		}
		if(pthread_cond_init(&clerk_cond[i], NULL) != 0){
			return FALSE;
		}
	}

	return TRUE;
}

int iCameFirst(int cusId, int queueNum){
	
	if(cusId == topOfQueue(&real_queue[queueNum])){
		return TRUE;
	}
	
	return FALSE;
}

int getClerkAwakingMe(qnum){
	
	int clerk_id;
	
	pthread_mutex_lock(&queue_mtex[qnum]);
	{
	
		clerk_id = queue_status[qnum];
		if(clerk_id < -1 || clerk_id > (NCLERK - 1)){
			pthread_mutex_unlock(&queue_mtex[qnum]);
			return -1;
		}
		queue_status[qnum] = IDLE;
	}
	pthread_mutex_unlock(&queue_mtex[qnum]);

	return clerk_id;
}

double getCurSeconds() {
	
	double cur_sysTime = 0.0;
	struct timeval now_time;
	
	cur_sysTime = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);
	
	gettimeofday(&now_time, NULL);
	cur_sysTime = (now_time.tv_sec + (double) now_time.tv_usec / 1000000) - cur_sysTime;
	
	return cur_sysTime;
}
