#include<pthread.h>
#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>
#include<unistd.h>

typedef struct queue {
  int capacity;
  int head;
  int tail;
  int *buff;
} queue;

typedef struct attributes {
  int producerSleepTime;
  int consumerSleepTime;
  int numProducers;
  int numConsumers;
  bool continueExecution;
  size_t currentLine;
  pthread_cond_t *producerCondition;
  pthread_cond_t *consumerCondition;
  pthread_mutex_t *producerMutex;
  pthread_mutex_t *consumerMutex;
  queue *buffer;
} attrs;

bool isEmpty (queue *buffer) {
  return buffer->head == -1;
} // end of isEmpty

bool isFull (queue *buffer) {
  return (buffer->tail+1)%buffer->capacity==buffer->head;
}

/*
  ProduceElement is run by threads. It simulates producing a value
  and appending it to an array based on a test configuration's capacity.
*/
void *produceElement (void *args) {
  attrs *attr = (attrs *)args;
  int producedValue;

  while(attr->continueExecution) {
    // Create a random value. Created here to act more like simulated user input.
    producedValue = rand() % 100;

    // Get a lock
    pthread_mutex_lock(attr->producerMutex);

    // Check if buffer is full
    if(isFull(attr->buffer)) {
      printf("Buffer is full. Waiting for consumer.\n");
      pthread_cond_wait(attr->producerCondition, attr->producerMutex);
    } // end of isfull check
    // If our alloted time is up, leave the loop to close the thread
    if(!attr->continueExecution) {
      break;
    }

    // 'Produce' the tail element
    if(isEmpty(attr->buffer)) { //
      attr->buffer->head = 0;
    }
    attr->buffer->tail = (attr->buffer->tail+1)%attr->buffer->capacity;
    attr->buffer->buff[attr->buffer->tail] = producedValue;

    // Unlock producers
    pthread_mutex_unlock(attr->producerMutex);

    // Signal that we have produced an item
    pthread_cond_signal(attr->consumerCondition);

    //sleep for specified time. You originally said random based on specified but
    //nick has told me that you told him that you do not want it to be random
    sleep(rand() % (attr->producerSleepTime+1));
  } // end of while
  // For proper exiting, we must signal the thread stuck in cond_wait
  // as well as unlock the mutex locks to ensure all threads are quitting and
  // there is no infinite waiting.
  pthread_cond_signal(attr->producerCondition);
  pthread_cond_signal(attr->consumerCondition);
  pthread_mutex_unlock(attr->producerMutex);
  pthread_mutex_unlock(attr->consumerMutex);
  pthread_exit(NULL);
} // end of produceElement

/*
  ConsumeElement is run by threads. It simulates conusming a value
  by changing the pointers of a circular array.
*/
void *consumeElement (void *args) {
  attrs *attr = (attrs *)args;

  while(attr->continueExecution) {
    // Lock consumers
    pthread_mutex_lock(attr->consumerMutex);

    // Check to see if the queue is full.
    if(isEmpty(attr->buffer)) {
      printf("Buffer is empty. Waiting for producer.\n");
      pthread_cond_wait(attr->consumerCondition, attr->producerMutex);
    } // end of isempty check
    // If our alloted time is up, leave the loop to close the thread
    if(!attr->continueExecution) {
      break;
    }

    // 'Consume' the front element.
    if (attr->buffer->head == attr->buffer->tail) { // if size 1
      attr->buffer->head = -1;
      attr->buffer->tail = -1;
    }
    else {
      attr->buffer->head = (attr->buffer->head+1)%attr->buffer->capacity;
    }

    // Unlock consumers
    pthread_mutex_unlock(attr->consumerMutex);

    // Signal that we have consumed an item
    pthread_cond_signal(attr->producerCondition);

    //sleep for specified time. You originally said random based on specified but
    //nick has told me that you told him that you do not want it to be random
    sleep(attr->consumerSleepTime);
  } // end of while
  // For proper exiting, we must signal the thread stuck in cond_wait
  // as well as unlock the mutex locks to ensure all threads are quitting and
  // there is no infinite waiting.
  pthread_cond_signal(attr->producerCondition);
  pthread_cond_signal(attr->consumerCondition);
  pthread_mutex_unlock(attr->producerMutex);
  pthread_mutex_unlock(attr->consumerMutex);
  pthread_exit(NULL);
} // end of consumeElement

/*
  Setup function reads a line of the CSV file and places the values in their
  proper locations. It additionally handles reseting values modified during the
  course of execution. It returns a bool that alerts when there are no more
  lines to be read and therefor no more tests to execute.
*/
bool setup(FILE *file, attrs *attr) {
  ssize_t read;
  char *line;

  // Reads the next line. If that line does not exist, return to main and
  // signal that all tests are complete.
  if ((read = getline(&line, &attr->currentLine, file)) == -1) {
    free(line);
    return false;
  }

  // Parse the line read from the file into their respective variables
  sscanf(line, "%d, %d, %d, %d, %d", &attr->buffer->capacity,
         &attr->producerSleepTime, &attr->consumerSleepTime,
         &attr->numProducers, &attr->numConsumers);

  // Update the boolean that controls the runtime of the threads
  attr->continueExecution = true;

  // Prepare the queue bsaed on the desired size
  attr->buffer->buff = (int*)malloc(attr->buffer->capacity*sizeof(int));
  attr->buffer->head = -1;
  attr->buffer->tail = -1;

  free(line);
  return true;
}

/*
  Begins execution of the test based on parameters set out from the CSV and
  read by the setup function.
*/
void runTest(attrs*attr, int executionTime) {
  // Create threads
  pthread_t producers[attr->numProducers];
  pthread_t consumers[attr->numConsumers];

  //Start producers
  for(int i=0; i<attr->numProducers; i++) {
    pthread_create(&producers[i], NULL, produceElement, (void*)attr);
  }

  //Start consumers
  for(int i=0; i<attr->numConsumers; i++) {
    pthread_create(&consumers[i], NULL, consumeElement, (void*)attr);
  }

  // Run for the input amount of time before telling other threads to end
  sleep(executionTime);
  attr->continueExecution = false;
  // Wait for producers to terminate. We must all signal conditions to ensure
  // none of the threads are left behind.
  for(int i=0; i<attr->numProducers; i++) {
    pthread_join(producers[i], NULL);
  }
  // Wait for consumers to terminate. We must all signal conditions to ensure
  // none of the threads are left behind.
  for(int i=0; i<attr->numConsumers; i++) {
    pthread_join(consumers[i], NULL);
  }
}

int main (int argc, char *argv[]) {

  FILE *file;
  char path[64];
  file = fopen(argv[1], "r");

  // Ensure that we have a working path
  while (file == NULL) {
    printf("File not found. Please reinput path.\n"); // should be "./testConf.txt"
    scanf("%s", path);
    file = fopen(path, "r");
  }

  // Allocate memory for the attributes and queue structures. These will be
  // reused across all tests so will be freed at the end of main.
  attrs *attr = (attrs*)malloc(sizeof(attrs));
  attr->buffer = (queue*)malloc(sizeof(queue));

  // Create and initialize lock and store their addresses.
  // This is shared across all tests so is done in the main function.
  pthread_mutex_t producerMutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t consumerMutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t producerCondition = PTHREAD_COND_INITIALIZER;
  pthread_cond_t consumerCondition = PTHREAD_COND_INITIALIZER;
  attr->producerCondition = &producerCondition;
  attr->consumerCondition = &consumerCondition;
  attr->producerMutex = &producerMutex;
  attr->consumerMutex = &consumerMutex;

  /*
    The execution time is the second element of the input by the user in the
    command line. Atoi converts it into a number. If it cannot be converted
    (uesr inputs a letter, for example), then it goes into the loop
    where Sscanf converts the value into an integer and stores it
    in the executiontime variable for later use.
  */
  int executionTime = atoi(argv[2]);
  while (executionTime == 0) {
    printf("Execution time is 0. Please reinput a valid execution time.\n");
    scanf("%d", &executionTime);
  }

  // While there are more test cases
  int testCount = 0;
  while(setup(file, attr)) {
    // Tell the user information about this test iteration.
    testCount++;
    printf("\nBeginning Test %d: BufferSize = %d, ProducerSleepTime = %d, ConsumerSleepTime = %d,"
           " numProducers = %d, numConsumers = %d.\n", testCount, attr->buffer->capacity,
           attr->producerSleepTime, attr->consumerSleepTime,
           attr->numProducers, attr->numConsumers);
           // Begin the test
           runTest(attr, executionTime);

           // Free up the array size as it changes for each test case
           free(attr->buffer->buff);
  }

  // Cleanup
  fclose(file);
  free(attr->buffer);
  free(attr);

  return 0;
} // end of main
