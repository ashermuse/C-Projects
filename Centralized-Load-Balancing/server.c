#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<stdbool.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

struct node {
  int nodeID;
  long double CPUUtil;
  int *sock;
  struct node *next;
}; // end of node

struct SLL {
  struct node *head;
  struct node *tail;
  int size;
  int currentID;
}; // end of SLL

bool isEmpty (struct SLL *list) {
  return list->head == NULL;
} // end of isEmpty

struct node *createNode (int nodeID, int *sock) {
  //Create new node
  struct node *newNode = (struct node*)malloc(sizeof(struct node));
  //Assign values
  newNode->sock = sock;
  newNode->nodeID = nodeID;
  newNode->CPUUtil = 1;
  newNode->next = NULL;
  return newNode;
} // end of createNode

struct node *searchByID (struct SLL *list, int nodeID) {
  struct node *index = list->head;
  while (index != NULL) {
    // if we find the key, return value at that location
    if (index->nodeID == nodeID) {
      return index;
    } // end of if
    index = index->next;
  } // end of while

  // if we do not find anything then the while terminates and we return -1
  return NULL;
} // end of searchByID

struct node *minCPUUtil (struct SLL *list) {
  struct node *index = list->head;
	struct node *minNode = index;
  while (index != NULL) {
    // if we find the key, return value at that location
    if (index->CPUUtil < minNode->CPUUtil) {
      minNode = index;
    } // end of if
    index = index->next;
  } // end of while

  // if we do not find anything then the while termiantes and we return -1
  return minNode;
} // end of minCPUUtil

void append(struct SLL *list, int nodeID, int *sock) {
  // Create the new node.
  struct node *newNode = createNode(nodeID, sock);

  if(isEmpty(list)) {
    list->head = newNode;
  } // end of if
  else {
    list->tail->next=newNode;
  } // end of else
  list->tail = newNode;
} // end of append

bool updateCPUUtil (struct SLL *list, int nodeID, double newUtil) {
  struct node *index = searchByID(list, nodeID);
  if (index != NULL) { // if we found it
    // update value
    index->CPUUtil = newUtil;
    return true;
  } // end of if
  return false;
} // end of updateNode

struct node* removeNode (struct SLL *list, int nodeID) {
  struct node *index = list->head;
  struct node *previndex = index;
  while (index != NULL) {
    if (index->nodeID == nodeID) { // if we find the key, return value at that location
      if (index->next==NULL && previndex == index) { // size 1 so must update head and tail
        list->tail = NULL;
        list->head = NULL;
      } // end of if
      else if (index->next == NULL) { // removing end so must update tail
        list->tail = previndex;
      } // end of elseif
      else if (previndex == index) { // removing first entry so must update head
        list->head = index->next;
      } // end of else if
      previndex->next = index->next;
      free(index);
      return previndex->next;
    } // end of if
    previndex = index;
    index = index->next;
  } // end of while
} // end of remover

void printList (struct SLL *list) {
  struct node *index = list->head;
  int count = 1;
  if (index == NULL) { // if the linked list is empty.
    printf("\nThe linked list is currently empty.");
  }
  else {
    printf("Current nodes in cluster: \n");
  }
  while (index != NULL) {
    printf("\nLocation %d: nodeID is %d, CPU Utilization is %Lf.", count, index->nodeID, index->CPUUtil);
    index = index->next;
    count++;
  } // end of while
} // end of printList

void *inputHandler(void *masterlist) {
  struct SLL *list = (struct SLL*)masterlist;
  char *command = (char*)malloc(sizeof(typeof(char))*128);
  char *message = (char*)malloc(sizeof(typeof(char))*128);
  char *buffer = (char*)malloc(sizeof(typeof(char))*128);
  long double CPUUtil;

  while(message[0] != '!' || message[1] != 'Q') {

    // Get command to be run.
    printf("Please input command to be run: ");
    fgets(command, 128, stdin);
    printf("Command is: %s\n", command);


    // Request CPU util from clients.
    struct node *index = list->head;
    message = "!C";
    while (index != NULL) {
      write(*index->sock , message , strlen(message));
      read(*index->sock , buffer, 1024);
      sscanf(buffer, "!C %Lf", &index->CPUUtil);
      printf("nodeID: %d, CPU Util: %Lf\n", index->nodeID, index->CPUUtil);
      index = index->next;
    } // end of while

    if (list->size < 1) { //If all client nodes disconnect
      printf("ERROR: All nodes have left the cluster.\n");
      break;
    }

    // Get min CPUUtil.
    struct node *worker = minCPUUtil(list);

    // Send the work to that worker.
    printf("Node %d selected. CPU utilizaiton: %Lf.\n", worker->nodeID, worker->CPUUtil);

    // Send file to run
    write(*worker->sock , command , strlen(command));
  }

  free(message);
  free(command);
  free(buffer);
  if (list->size < 1) {
    return (void *) 1;
  }
  return 0;
} // end of inputHandler

void *connectionHandler(void *masterlist) {
	//Get the socket descriptor
  struct SLL *list = (struct SLL*)masterlist;
	int sock = *list->tail->sock;
	int read_size;
	char *message = (char*)malloc(sizeof(typeof(char))*128), clientMessage[2000];

  // Connection accepted message sent to client.
  sprintf(message, "%d", list->currentID++);
  printf("\nID of new node is: %s\n", message);
  write(sock , message , strlen(message));
  printf("Connection handled.\n");

  free(message);
	return 0;
} // end of connectionHandler

int main(int argc , char *argv[]) {
	int socket_desc , client_sock , c , *new_sock, port;
	struct sockaddr_in server , client;
  struct SLL *list = (struct SLL *)malloc(sizeof(struct SLL));
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
  list->currentID = 0;

	// Handle port selection
  if (argc < 2) {
    printf("Port not specified. Please input desired port: ");
    scanf("%d", &port);
  }
  else {
    port = atoi(argv[1]);
    while(port == 0) {
      printf("\nThat is not a valid port. Please input a valid port: ");
      scanf("%d", &port);
    }
  }

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket.\n");
		return 1;
	}
	printf("Socket created successfully.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind
	if(bind(socket_desc,(struct sockaddr *)&server, sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	printf("Successfully bound socket.\n");

	//Listen
	listen(socket_desc, 5);
	printf("Waiting for incoming connections...\n\n");
	c = sizeof(struct sockaddr_in);

  pthread_t inputThread;
  if (pthread_create( &inputThread , NULL ,  inputHandler , (void*) list) == 1) {
    pthread_create( &inputThread , NULL ,  inputHandler , (void*) list);
  }

  // Handle connections
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		printf("Connection accepted.\n");

		pthread_t responseThread;
		new_sock = malloc(sizeof(int));
		*new_sock = client_sock;

    // Update the linked list with the new connection.
    append(list, list->currentID, new_sock);
    list->size++;

    // This creates the thread to handle a new connection.
		if( pthread_create( &responseThread , NULL ,  connectionHandler , (void*) list) < 0) {
			perror("could not create thread");
			return 1;
		}
    printList(list);
	}

	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}

	return 0;
} // end of main
