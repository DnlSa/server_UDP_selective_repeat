
#include "basic.h"



int correct_send() { // funzione che imposto in modo che ogni pacchetto abbbia la stessa probabilita di perdersi
// il seme randomico deve essere calcolato su la base dei microsecondi e non
// con time(NULL) in quanto quest ultima quantizza fino al secondo e non al microsecondo.
	struct timeval t1;
	gettimeofday(&t1 , NULL);
	srand(t1.tv_usec*t1.tv_usec);
	int randint = rand()%100+1;
	if(LOST_PROB <randint) {
		return 1;
	}
	return 0;
}


int get_timeout(int sockfd){
	// Restituisce l'attuale valore del timeout della socket
	struct timeval time;
	int dimension = sizeof(time);
	time.tv_sec = 0;
	time.tv_usec = 0;
	if(getsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&time, &dimension) < 0) {
		perror("getsockopt error");
		exit(-1);
	}
	//printf("timeout: %ld\n",time.tv_usec);
	return time.tv_usec;
}


void set_timeout(int sockfd, int timeout) {
	// Imposta il timeout della socket in microsecondi
	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = timeout;
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&time, sizeof(time)) < 0) {
		perror("setsockopt set_timeout");
		exit(-1);
	}
}


void decrease_timeout(int sockfd){
	// Decrementa il valore attuale del timeout in caso di timeout adattativo
	int timeout;
	timeout = get_timeout(sockfd);
	if(timeout >= MIN_TIMEOUT + TIME_UNIT){
		set_timeout(sockfd, timeout - TIME_UNIT);
	}
}

void increase_timeout(int sockfd){
	//Incrementa l'attuale valore del timeout in caso di timeout adattativo
	int timeout;
	timeout = get_timeout(sockfd);
	if(timeout <= MAX_TIMEOUT - TIME_UNIT){
		set_timeout(sockfd, timeout + TIME_UNIT);
	}
}
