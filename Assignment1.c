//Embedded Systems Course, Electrical and Computer Engineering AUTh, Angelos Vlachos

#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define QUEUESIZE 100
#define LOOP 10000
#define N 100
#define P 1000

//Global arrays to store timevals
struct timeval end[N*LOOP];
struct timeval start[N*LOOP];
int counter=0;
int counter1=0;

//Prod/Con Functions
void *producer (void *args);
void *consumer (void *args);

//Function to be put in FIFO
int retFunc(int i){
  for(int j=0;j<1000;j++){
  }
  return i;
  }


struct workFunction {
	  void * (*work)(void *);
	    void * arg;
};

//Struct queue
typedef struct {
	struct workFunction buf[QUEUESIZE];
  	long head, tail;
  	int full, empty;
  	pthread_mutex_t *mut;
  	pthread_cond_t *notFull, *notEmpty;
} queue;

//Queue Functions
queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, struct workFunction in);
void queueDel (queue *q, struct workFunction *out);

int main ()
{
	//create fifo and initialize threads
	queue *fifo;
  pthread_t *pro;
  pthread_t *con;

	pro = (pthread_t *)malloc(N*sizeof(pthread_t));
	con = (pthread_t*)malloc(P*sizeof(pthread_t));

  fifo = queueInit ();
  if (fifo ==  NULL) {
    fprintf (stderr, "main: Queue Init failed.\n");
    exit (1);
  }

  int i = 0;

	for(i = 0; i < N; i++){
		pthread_create(&pro[i], NULL, producer, fifo);
	}

	for(i = 0; i < P; i++){
		pthread_create(&pro[i], NULL, consumer, fifo);
  }

//Sync threads
	for(i = 0; i < N; i++){
		pthread_join(pro[i], NULL);
	}

	for(i = 0; i < P; i++){
		pthread_join(con[i], NULL);
	 }

  queueDelete (fifo);
	double mean=0.0;

	double *time = malloc((N*LOOP)*sizeof(double));
  //Calculate Time for each element
	for(i = 0; i < N*LOOP; i++){
		time[i]=((end[i].tv_sec - start[i].tv_sec)*1000.0+(end[i].tv_usec - start[i].tv_usec)/1000.0)*1000;
		mean+=time[i];
	}

	mean=mean/(N*LOOP);
	printf("MEAN: %f us.\n",mean);
	return 0;
}

void *producer (void *q)
{
	  queue *fifo;
	  int i=0;

	  struct workFunction func;
	  func.work=(void *)retFunc;



	  fifo = (queue *)q;

    //LOOP
  	for (i = 0; i < LOOP; i++) {
        //Put lock
		    pthread_mutex_lock (fifo->mut);
        //If full then wait
	      while (fifo->full) {
		        //printf ("producer: queue FULL.\n");
		          pthread_cond_wait (fifo->notFull, fifo->mut);
		    }
        //If not full get time, put argument and put to queue
	       gettimeofday(&start[counter], NULL);
	       func.arg=(void *)counter;
	       counter++;
	       //printf("producer: sent %d.\n",(int)func.work(func.arg));
	       queueAdd (fifo, func);
         //Unlock
	       pthread_mutex_unlock (fifo->mut);
	       pthread_cond_signal (fifo->notEmpty);
	  }
    	return (NULL);
}



void *consumer (void *q)
{
  queue *fifo;
	int i;
	struct workFunction *funcOut=malloc(sizeof(struct workFunction));

	fifo = (queue *)q;
  //Loop for consumer so all elements are taken
	for (i = 0; i < LOOP*N/P; i++) {
	//while(1){
	pthread_mutex_lock (fifo->mut);
	while (fifo->empty) {
		 //printf ("consumer: queue EMPTY.\n");
		 pthread_cond_wait (fifo->notEmpty, fifo->mut);
		}
    //Remove from fifo
		queueDel (fifo, funcOut);
    //Get time
		gettimeofday(&end[counter1], NULL);
		counter1++;
		pthread_mutex_unlock (fifo->mut);
		pthread_cond_signal (fifo->notFull);
		//printf ("consumer: recieved %d.\n",(int)funcOut->work(funcOut->arg));
    //Execute function taken
		int j = (int)funcOut->work(funcOut->arg);
		//usleep(200000);
	//	free(funcOut);
	}
	return (NULL);
	free(funcOut);
}

queue *queueInit (void)
{
  queue *q;
	q = (queue *)malloc (sizeof (queue));
	if (q == NULL) return (NULL);

	q->empty = 1;
	q->full = 0;
	q->head = 0;
	q->tail = 0;
	q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (q->mut, NULL);
	q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notFull, NULL);
	q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notEmpty, NULL);

	return (q);
}


void queueDelete (queue *q)
{
	pthread_mutex_destroy (q->mut);
	free (q->mut);
	pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
	free (q->notEmpty);
	free (q);
}

void queueAdd (queue *q, struct workFunction in)
{
	q->buf[q->tail] = in;
	q->tail++;
	if (q->tail == QUEUESIZE)
		q->tail = 0;
	if (q->tail == q->head)
		q->full = 1;
	q->empty = 0;

	return;
}
void queueDel (queue *q, struct workFunction *out)
{
	*out = q->buf[q->head];

	q->head++;
	if (q->head == QUEUESIZE)
		q->head = 0;
	if (q->head == q->tail)
		q->empty = 1;
	q->full = 0;

	return;
}
