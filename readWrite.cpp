#include <semaphore.h>
#include <pthread.h>
#include <chrono>
#include <bits/stdc++.h>
using namespace std;

void *reader(void *);
void *writer(void *);

int readcount = 0,writecount = 0;
sem_t x,rsem,wsem;
pthread_t r[10],w[10];

typedef struct{
    int val;
}myarg_t;

void *reader(void *i){

	sem_wait(&x);
	sem_wait(&rsem);
	readcount++;
	if(readcount == 1)
		sem_wait(&wsem);
	sem_post(&rsem);
	sem_post(&x);
	myarg_t *args = (myarg_t *)i;
	cout << "reader: " << args->val << " is reading\n";
	this_thread::sleep_for(chrono::milliseconds(3000));
	cout << "reader: " << args->val << " finished reading\n";

	sem_wait(&rsem);
	readcount--;
	if(readcount==0)
		sem_post(&wsem);
	sem_post(&rsem);
}

void *writer(void *i){
	sem_wait(&x);
	sem_wait(&wsem);
	sem_post(&x);
	myarg_t *args = (myarg_t *)i;
	cout << "writer: " << args->val << " is writing\n";
	this_thread::sleep_for(chrono::milliseconds(2000));
	cout << "writer: " << args->val << " finished writing\n";

	sem_post(&wsem);
}

int main(){
	sem_init(&x,0,1);
	sem_init(&wsem,0,1);
	sem_init(&rsem,0,1);
	
	vector<myarg_t> v;
	for(int i = 0; i < 10; i++){
		myarg_t args = {i};
		v.push_back(args);
	}
	pthread_create(&r[0],NULL,reader,&v[0]);
	pthread_create(&w[0],NULL,writer,&v[0]);
	pthread_create(&r[1],NULL,reader,&v[1]);
	pthread_create(&r[2],NULL,reader,&v[2]);
	pthread_create(&r[3],NULL,reader,&v[3]);
	pthread_create(&w[3],NULL,writer,&v[3]);
	pthread_create(&r[4],NULL,reader,&v[4]);

	pthread_join(r[0],NULL);
	pthread_join(w[0],NULL);
	pthread_join(r[1],NULL);
	pthread_join(r[2],NULL);
	pthread_join(r[3],NULL);
	pthread_join(w[1],NULL);
	pthread_join(r[4],NULL);

	return(0);
}  