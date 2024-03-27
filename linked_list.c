#include <stdlib.h>
#include "linked_list.h"

// Creates and returns a new list
list_t* list_create()
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_t* l = (list_t *) malloc(sizeof(list_t));
    l->head = NULL;
    l->tail = NULL;
    l->count = 0;
    return l;
}

// Destroys a list
void list_destroy(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    list_node_t * current = list->head;
    while (current != NULL) {
	list_node_t * next = current->next;
	free(current);
	current = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
//    free(list);
}

// Returns head of the list
list_node_t* list_head(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns tail of the list
list_node_t* list_tail(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns next element in the list
list_node_t* list_next(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns prev element in the list
list_node_t* list_prev(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns end of the list marker
list_node_t* list_end(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns data in the given list node
void* list_data(list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return NULL;
}

// Returns the number of elements in the list
size_t list_count(list_t* list)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    return list->count;
}

// Finds the first node in the list with the given data
// Returns NULL if data could not be found
list_node_t* list_find(list_t* list, void* data)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    if (list == NULL) {
	return NULL;
    }
    list_node_t * current = list->head;
    while (current != NULL) {
	if (data == current->data) {
	    return current;
	}
	current = current->next;
    }
    return NULL;
}

// Inserts a new node in the list with the given data
// Returns new node inserted
list_node_t* list_insert(list_t* list, void* data)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    if (list_find(list, data) != NULL) {
	return NULL;
    } else {
	list_node_t * node = (list_node_t *) malloc(sizeof(list_node_t));
	node->data = data;
	node->next = list->head;
	node->prev = NULL;
	if (list->head == NULL) {
	    list->head = node;   
	    list->tail = node;
	} else {
	    list->head->prev = node;
	    list->head = node;
	}
	list->count++;
	return node;
    }
    return NULL;
}

// Removes a node from the list and frees the node resources
void list_remove(list_t* list, list_node_t* node)
{
    /* IMPLEMENT THIS IF YOU WANT TO USE LINKED LISTS */
    if (node != NULL) {
	if (list->count == 1) {
	    list->head = NULL;
	    list->tail = NULL;
	    list->count--;
	} else {
	    if (node == list->head) {
		list->head = node->next;
		list->head->prev = NULL;
	    } else if (node->next != NULL) {
	        node->next->prev = node->prev;
	        node->prev->next = node->next;
	    } else if (node == list->tail) {
		list->tail = node->prev;
		node->prev->next = NULL;
	    }
	    list->count--;
	}
	free(node);
    }
}