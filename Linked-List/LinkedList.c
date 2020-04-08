#include<stdlib.h>
#include<stdbool.h>
#include<stdio.h>

typedef struct node {
  int data;
  int key;
  struct node *next;
} node; // end of node

typedef struct SLL {
  node *head;
  node *tail;
} SLL; // end of SLL

bool isEmpty (SLL *list) {
  return list->head == NULL;
} // end of isEmpty

node *createNode (int key, int value) {
  //Create new node
  node *newNode = (node*)malloc(sizeof(node));
  //Assign values
  newNode->data = value;
  newNode->key = key;
  newNode->next = NULL;
  return newNode;
} // end of createNode

node *search (SLL *list, int key) {
  node *index = list->head;
  while (index != NULL) {
    // if we find the key, return value at that location
    if (index->key == key) {
      return index;
    } // end of if
    index = index->next;
  } // end of while

  // if we do not find anything then the while termiantes and we return -1
  return NULL;
} // end of search

void addFirst (SLL *list, int key, int value) {
  //Create new node
  node *newNode = createNode(key, value);

  //Point new node to previous head
  newNode->next = list->head;

  // if list is empty, update tail
  if(isEmpty(list)) {
    list->tail = newNode;
  }
  // update the head no matter the case
  list->head = newNode;
} // end of addFirst

void addLast (SLL *list, int key, int value) {
  // Create the new node.
  node *newNode = createNode(key, value);

  if(isEmpty(list)) {
    list->head = newNode;
  } // end of if
  else {
    list->tail->next=newNode;
  } // end of else
  list->tail = newNode;
} // end of addLast

bool addAfter (SLL *list, int key, int value, int searchKey) {
  node *location = search(list, searchKey);
  if (location != NULL) { // if we found it
    //Create new node
    node *newNode = createNode(key, value);
    //Point new node to next node;
    newNode->next = location->next;
    //Point old node to new node
    location->next = newNode;
    if (list->tail == location) { // if the point we are appending to is the last node, update tail
      list->tail = newNode;
    }
    return true;
  } // end of if
  return false;
} // end of addAfter

void printList (SLL *list) {
  node *index = list->head;
  int count = 1;
  if (index == NULL) { // if the linked list is empty.
    printf("\nThe linked list is currently empty.");
  }
  while (index != NULL) {
    printf("\nLocation %d: key is %d, value is %d.", count, index->key, index->data);
    index = index->next;
    count++;
  } // end of while
} // end of printList

bool updateNode (SLL *list, int key, int value) {
  node *index = search(list, key);
  if (index != NULL) { // if we found it
    // update value
    index->data = value;
    return true;
  } // end of if
  return false;
} // end of updateNode

bool removeNode (SLL *list, int key) {
  node *index = list->head;
  node *previndex = index;
  while (index != NULL) {
    if (index->key == key) { // if we find the key, return value at that location
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
      return true;
    } // end of if
    previndex = index;
    index = index->next;
  } // end of while
  return false;
} // end of removeNode

int main () {

  SLL list;
  list.head = NULL;
  list.tail = NULL;

  // Prints a simple greeting.
  printf("\n\nHello and welcome to Asher's Linked List Assignment.\n");
  printf("Please input your desired action.\n");
  // Declare variables
  int userInput;
  int key;
  int value;
  int searchKey;
  node *index;
  char commands[] = "\n[1] Add a node to the front of the list.\n[2] Add a node to the end of the list.\n"
                    "[3] Add a node at a given location.\n[4] Search for a given node.\n[5] Print a list of current nodes.\n"
                    "[6] Update the value stored in a given node.\n[7] Quit the program.\n[8] Remove a given node from the list.\n";

  // Handle UI
  while(userInput != 7) {
    // Get user input
    printf("%s\nAwaiting command...\n", commands);
    scanf("%d", &userInput);

    // Handle user input.
    switch (userInput) {
      case 1: // add to front
        printf("\nEnter key for the new node: ");
        scanf("%d", &key);
        printf("Enter value for the new node: ");
        scanf("%d", &value);
        addFirst(&list, key, value);
        printf("Node added successfully.\n");
        break;
      case 2: // add to end
        printf("\nEnter key for the new node: ");
        scanf("%d", &key);
        printf("Enter value for the new node: ");
        scanf("%d", &value);
        addLast(&list, key, value);
        printf("Node added successfully.\n");
        break;
      case 3: // add to given location
        printf("\nEnter key for the new node: ");
        scanf("%d", &key);
        printf("Enter value for the new node: ");
        scanf("%d", &value);
        printf("Enter key for the node you would like to append new node to: ");
        scanf("%d", &searchKey);
        value = addAfter(&list, key, value, searchKey);
        if (value == 1) { // if it works, do this
          printf("Node added successfully.\n");
        }
        else { // if it doesn't work, do this
          printf("Desired location does not exist in the Linked List.\n");
        }
        break;
      case 4: // search
        printf("\nEnter the key you would like to search for: ");
        scanf("%d", &key);
        index = search(&list, key);
        if (index == NULL) { // it wasn't found
          printf("This key is not currently in the Linked List.\n");
        } // end of if
        else { // it was found
          printf("Value associated with key %d is %d.\n", index->key, index->data);
        } // end of else
        break;
      case 5: // print
        printList(&list);
        printf("\n");
        break;
      case 6: // update
        printf("\nEnter key for the node you wish to update: ");
        scanf("%d", &key);
        printf("Enter the new value for the selected node: ");
        scanf("%d", &value);
        value = updateNode(&list, key, value);
        if (value == 1) { // if it works, do this
          printf("Node updated successfully.\n");
        }
        else { // if it doesn't work, do this
          printf("Node does not exist in the Linked List.\n");
        }
        break;
      case 8: // remove
        printf("\nEnter key for the node you wish to remove: ");
        scanf("%d", &key);
        value = removeNode(&list, key);
        if (value == 1) { // if it works, do this
          printf("Node removed successfully.\n");
        }
        else { // if it doesn't work, do this
          printf("Node does not exist in the Linked List.\n");
        }
        break;
    } // end of switch
  } // end of while

  return 0;
} // end of main
