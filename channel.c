#include "channel.h"

void resetsem(select_t * channel_list, size_t channel_count, sem_t* selectsem);

// Creates a new channel with the provided size and returns it to the caller
channel_t* channel_create(size_t size)
{
    /* IMPLEMENT THIS */
    channel_t * c = (channel_t *) malloc(sizeof(channel_t));
    c->buffer = buffer_create(size);
    pthread_mutex_init(&(c->mutex), NULL);
//    sem_init(&(c->slots), 0, (unsigned int) size);
//    sem_init(&(c->items), 0, 0);
    pthread_cond_init(&c->slotscond, NULL);
    pthread_cond_init(&c->itemscond, NULL);
    c->slotscount = (int) size;
    c->itemscount = 0;
    c->status = SUCCESS;
    c->semlist = list_create();
    return c;
}

void sembroadcast(channel_t *channel) {
    if (channel->semlist != NULL) {
        list_node_t * node = channel->semlist->head;
        while (node != NULL) {
            sem_post((sem_t*) node->data);
            node = node->next;
        }
        list_destroy(channel->semlist);
//        channel->semlist = NULL;
    }
}

// Writes data to the given channel
// This is a blocking call i.e., the function only returns on a successful completion of send
// In case the channel is full, the function waits till the channel has space to write the new data
// Returns SUCCESS for successfully writing data to the channel,
// CLOSED_ERROR if the channel is closed, and
// GENERIC_ERROR on encountering any other generic error of any sort
enum channel_status channel_send(channel_t *channel, void* data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    if (channel->status != SUCCESS) {
	if (channel->status == CLOSED_ERROR) {
	    pthread_mutex_unlock(&channel->mutex);
	    return channel->status;
	}
	pthread_mutex_unlock(&channel->mutex);
	return GENERIC_ERROR;
    }
//    sem_wait(&channel->slots);
    while (channel->slotscount == 0) {
	pthread_cond_wait(&channel->slotscond, &channel->mutex);
	if (channel->status == CLOSED_ERROR) {
	    pthread_mutex_unlock(&channel->mutex);
	    return channel->status;
	}
    }
    channel->slotscount--;
    channel->itemscount++;
    enum buffer_status bs = buffer_add(channel->buffer, data);
    (void) bs;
    pthread_cond_signal(&channel->itemscond);
//    sem_post(&channel->items);
    sembroadcast(channel);
    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Reads data from the given channel and stores it in the function's input parameter, data (Note that it is a double pointer)
// This is a blocking call i.e., the function only returns on a successful completion of receive
// In case the channel is empty, the function waits till the channel has some data to read
// Returns SUCCESS for successful retrieval of data,
// CLOSED_ERROR if the channel is closed, and
// GENERIC_ERROR on encountering any other generic error of any sort
enum channel_status channel_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    if (channel->status != SUCCESS) {
	if (channel->status == CLOSED_ERROR) {
	    pthread_mutex_unlock(&channel->mutex);
	    return channel->status;
	}
	pthread_mutex_unlock(&channel->mutex);
	return GENERIC_ERROR;
    }
//    sem_wait(&channel->items);
    while (channel->itemscount == 0) {
	pthread_cond_wait(&channel->itemscond, &channel->mutex);
	if (channel->status == CLOSED_ERROR) {
	    pthread_mutex_unlock(&channel->mutex);
	    return channel->status;
	}
    }
    channel->slotscount++;
    channel->itemscount--;
    enum buffer_status bs = buffer_remove(channel->buffer, data);
    (void) bs;
    pthread_cond_signal(&channel->slotscond);

    // post select sem
//    sem_post(&channel->slots);    
    sembroadcast(channel);
    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Writes data to the given channel
// This is a non-blocking call i.e., the function simply returns if the channel is full
// Returns SUCCESS for successfully writing data to the channel,
// CHANNEL_FULL if the channel is full and the data was not added to the buffer,
// CLOSED_ERROR if the channel is closed, and
// GENERIC_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_send(channel_t* channel, void* data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    if (channel->status != SUCCESS) {
	if (channel->status == CLOSED_ERROR) {
	    pthread_mutex_unlock(&channel->mutex);
	    return channel->status;
	}
	pthread_mutex_unlock(&channel->mutex);
	return GENERIC_ERROR;
    }
    if (channel->slotscount == 0) {
	pthread_mutex_unlock(&channel->mutex);
	return CHANNEL_FULL;
    }
    enum buffer_status bs = buffer_add(channel->buffer, data);
    (void) bs;
    channel->slotscount--;
    channel->itemscount++;
    pthread_cond_signal(&channel->itemscond);
    sembroadcast(channel);
    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Reads data from the given channel and stores it in the function's input parameter data (Note that it is a double pointer)
// This is a non-blocking call i.e., the function simply returns if the channel is empty
// Returns SUCCESS for successful retrieval of data,
// CHANNEL_EMPTY if the channel is empty and nothing was stored in data,
// CLOSED_ERROR if the channel is closed, and
// GENERIC_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    if (channel->status != SUCCESS) {
	if (channel->status == CLOSED_ERROR) {
	    pthread_mutex_unlock(&channel->mutex);
	    return channel->status;
	}
	pthread_mutex_unlock(&channel->mutex);
	return GENERIC_ERROR;
    }
    if (channel->itemscount == 0) {
	pthread_mutex_unlock(&channel->mutex);
	return CHANNEL_EMPTY;
    }
    enum buffer_status bs = buffer_remove(channel->buffer, data);
    (void) bs;
    channel->itemscount--;
    channel->slotscount++;
    pthread_cond_signal(&channel->slotscond);
    sembroadcast(channel);
    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Closes the channel and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the channel is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns SUCCESS if close is successful,
// CLOSED_ERROR if the channel is already closed, and
// GENERIC_ERROR in any other error case
enum channel_status channel_close(channel_t* channel)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    if (channel->status != SUCCESS) {
	if (channel->status == CLOSED_ERROR) {
	    pthread_mutex_unlock(&channel->mutex);
	    return channel->status;
	}
	pthread_mutex_unlock(&channel->mutex);
	return GENERIC_ERROR;
    }
    channel->status = CLOSED_ERROR;
    pthread_cond_broadcast(&channel->slotscond);
    pthread_cond_broadcast(&channel->itemscond);
    sembroadcast(channel);
    pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// GENERIC_ERROR in any other error case
enum channel_status channel_destroy(channel_t* channel)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    if (channel->status != CLOSED_ERROR) {
	pthread_mutex_unlock(&channel->mutex);
	return DESTROY_ERROR;
    }
    pthread_cond_destroy(&channel->slotscond);
    pthread_cond_destroy(&channel->itemscond);
    buffer_free(channel->buffer);
    channel->buffer = NULL;
    if (channel->semlist != NULL) {
	list_destroy(channel->semlist);
	free(channel->semlist);
	channel->semlist = NULL;
    }
    pthread_mutex_unlock(&channel->mutex);
    pthread_mutex_destroy(&channel->mutex);
    free(channel);
    return SUCCESS;
}

// Takes an array of channels (channel_list) of type select_t and the array length (channel_count) as inputs
// This API iterates over the provided list and finds the set of possible channels which can be used to invoke the required operation (send or receive) specified in select_t
// If multiple options are available, it selects the first option and performs its corresponding action
// If no channel is available, the call is blocked and waits till it finds a channel which supports its required operation
// Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
// In the event that a channel is closed or encounters any error, the error should be propagated and returned through select
// Additionally, selected_index is set to the index of the channel that generated the error
enum channel_status channel_select(select_t* channel_list, size_t channel_count, size_t* selected_index)
{
    /* IMPLEMENT THIS */
    enum channel_status st;
    sem_t selectsem;
    sem_init(&selectsem, 0, 0);
    for (int i = 0; i < channel_count; i++) {
        channel_t * c = channel_list[i].channel;
	pthread_mutex_lock(&c->mutex);
//	c->selectsem = &selectsem;
	list_insert(c->semlist, &selectsem);
	pthread_mutex_unlock(&c->mutex);
	if (channel_list[i].dir == SEND) {
	    if((st = channel_non_blocking_send(c, channel_list[i].data)) == SUCCESS) {
		*selected_index = (size_t) i;
		resetsem(channel_list, channel_count, &selectsem);
		sem_destroy(&selectsem);
		return st;
	    } else {
                if (st == CHANNEL_FULL) {
		    continue;
		} else {
		    *selected_index = (size_t) i;
		    break;
		}
	    }
	} else {
	    if ((st = channel_non_blocking_receive(c, &channel_list[i].data)) == SUCCESS) {
		*selected_index = (size_t) i;
		resetsem(channel_list, channel_count, &selectsem);
		sem_destroy(&selectsem);
		return st;
	    } else {
		if (st == CHANNEL_EMPTY) {
		    continue;
		} else {
		    *selected_index = (size_t) i;
		    break;
		}
	    }
	}
    }
    if (st != CHANNEL_FULL && st != CHANNEL_EMPTY) {
        resetsem(channel_list, channel_count, &selectsem);
	sem_destroy(&selectsem);
	return st;
    }
    do {
    sem_wait(&selectsem);
    for (int i = 0; i < channel_count; i++) {
	channel_t * c = channel_list[i].channel;
	pthread_mutex_lock(&c->mutex);
//	c->selectsem = &selectsem;
	list_insert(c->semlist, &selectsem);
	pthread_mutex_unlock(&c->mutex);
	if (channel_list[i].dir == SEND) {
	    if ((st = channel_non_blocking_send(c, channel_list[i].data)) == SUCCESS) {
		*selected_index = (size_t) i;
		resetsem(channel_list, channel_count, &selectsem);
		sem_destroy(&selectsem);
		return st;
	    } else {
		if (st == CHANNEL_FULL) {
		    continue;
		} else {
		    *selected_index = (size_t) i;
		    break;
		}
	    }
	} else {
	    if ((st = channel_non_blocking_receive(c, &channel_list[i].data)) == SUCCESS) {
		*selected_index = (size_t) i;
		resetsem(channel_list, channel_count, &selectsem);
		sem_destroy(&selectsem);
		return st;
	    } else {
		if (st == CHANNEL_EMPTY) {
		    continue;
		} else {
		    *selected_index = (size_t) i;
		    break;
		}
	    }
	}
    }
    } while (st == CHANNEL_FULL || st == CHANNEL_EMPTY);
    resetsem(channel_list, channel_count, &selectsem);
    sem_destroy(&selectsem);
    return st;
}

void resetsem (select_t* channel_list, size_t channel_count, sem_t* selectsem) {
    for (int i = 0; i < channel_count; i++) {
	channel_t * c = channel_list[i].channel;
	pthread_mutex_lock(&c->mutex);
	list_node_t * node = list_find(c->semlist, selectsem);
	if (node != NULL) {
	    list_remove(c->semlist, node);
	}
//	c->selectsem = NULL;
	pthread_mutex_unlock(&c->mutex);
    }
}