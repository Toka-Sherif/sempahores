#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

sem_t many_counters,mutex,countsem,sem_size,sem_prod,sem_con;
int count=0,writecounter=0;
int buffer_size;
int *buffer;
int front = -1, rear =-1;
int  deqeu_ret;


int isFull()
{
    if( (front == rear + 1) || (front == 0 && rear == buffer_size-1)) return 1;
    return 0;
}

int isEmpty()
{
    if(front == -1) return 1;
    return 0;
}

int enqueue (int element)
{
    int index;
    if(isFull()) {

    }
    else
    {
        if(front == -1) front = 0;
        index =rear;
        rear = (rear + 1) % buffer_size;
        buffer[rear] = element;

    }
    return rear;
}


int  dequeue ()
{
    int index;
    if(isEmpty()) {
        return(-1);
    } else {
        deqeu_ret = buffer[front];
        index = front;
        if (front == rear){
            front = -1;
            rear = -1;
        }
        else {
            front = (front + 1) % buffer_size;
        }

    }
    return index;
}


void * writer(void *arg){
    int data = ((int)arg);

    // Random Message Sending
    int num = (rand() % ( 20- 1 + 1)) + 1;

    sleep(num);
    printf("  \n Counter Thread [%d] : Received a Message \n",data);

    sem_wait(&countsem);
    writecounter++;
    if (writecounter >= 1 ) {
        printf("\n  Counter Thread [%d] : Waiting To Write \n",data);
        sem_wait(&many_counters);
    }


    /*mutual execlusion between mcounters*/
    sem_post(&countsem);
    sem_wait(&mutex);
    /*critical section on count variable*/
        count++;
        printf(" \nCounter Thread [%d] : Now adding to Counter,counter Value=%d \n",data,count);
   /*end of critical section on count variable*/

    sem_post(&mutex);
    writecounter--;
    sem_post(&countsem);


    /*end of mutual execlusion between mcounters*/
    sem_post(&many_counters);
}

void * reader(void * arg){
    int timenumber = 0;
    int * data = ((int*)arg);
    int value=0,index=0;

     while(timenumber<data[0]){

        sleep(4);
        printf("\n Mointer thread: waiting to read counter\n");
        sem_wait(&mutex);
        /*mutual execlusion between mcounter and mMonitor*/
        if(count >= 1){

         printf(" \n Mointer thread: reading a count value of %d\n",count);

            value = count;
            count = 0;
            if(sem_trywait(&sem_size) <= -1)
             {

             printf(" \n  Monitor Thread : Buffer Full !!\n"); 		     sem_wait(&sem_size);
             };
            sem_wait(&sem_prod);
            /*Critical Section of buffer*/
            index = enqueue (value);
            printf("\n Monitor Thread : writing to buffer at position %d\n",index);
            /*end of critical section of buffer */
            sem_post(&sem_prod);
            sem_post(&sem_con);
            timenumber+=value;
        }
/*end of mutual execlusion of mcounter and mMonitor*/
        sem_post(&many_counters);
        sem_post(&mutex);
     }
}

void * consumer(void * arg){

    int finalsend=0;
    int index;
    int  * data = ((int)arg);

    while (finalsend<data[0])
    {
    sleep(10);
    sem_wait(&sem_con);
    sem_wait(&sem_prod);
    	/*critical section */
    if(isEmpty()==0){
        index = dequeue();
        printf(" \n Collector Thread : reading from buffer at position  %d\n",index);
        finalsend+=deqeu_ret;
    }else{
        printf("\n Collector Thread : nothing is in buffer !\n");
    }
	/*end of critical section */
    sem_post(&sem_prod);
    sem_post(&sem_size);
}
}

int main(){

    int mcount,i;
    printf("Enter The number of counters : ");
    scanf("%d",&mcount);
    printf("Enter The Size of Buffer : ");
    scanf("%d",&buffer_size);
    pthread_t mMoniter,mCollector;
    pthread_t *mCounter = malloc (mcount*sizeof(pthread_t));

    // name of the thread , 0 for the shared threads, value = 1
    sem_init(&mutex,0,1);
    sem_init(&many_counters,0,1);
    sem_init(&countsem,0,1);

    sem_init(&sem_size,0,buffer_size);
    sem_init(&sem_prod,0,1);
    sem_init(&sem_con,0,0);

    buffer = malloc(buffer_size*sizeof(int));

    for (i =0;i<mcount;i++){
        pthread_create(&mCounter[i], NULL,writer, (void *)(i));
    }

     pthread_create(&mMoniter, NULL,reader, (void *)(&mcount));
     pthread_create(&mCollector, NULL,consumer, (void *)(&mcount));

    for (i =0;i<mcount;i++){
         pthread_join(mCounter[i], NULL);
    }

     pthread_join(mMoniter, NULL);
     pthread_join(mCollector,NULL);

    return 0;

}
