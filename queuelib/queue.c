#include "queue.h"
#include "../processlib/process.h"
#include <stdlib.h>
#include <assert.h>



void node_init(node_item_t * node, unsigned int id)
{
    node->id = id;
    node->inQueue = false;
    node->ahead = NULL;
    node->behind = NULL;
}

bool node_in_queue(node_item_t * node)
{
    return node->inQueue;
}

fifo_queue_t * queue_create(unsigned capacity)
{
    if (capacity == 0) {
        return NULL;
    }
    fifo_queue_t * newQueue_ptr = malloc(sizeof(fifo_queue_t));
    if (newQueue_ptr == NULL) {
        return NULL;
    }
    newQueue_ptr->capacity = capacity;
    newQueue_ptr->usage = 0;
    newQueue_ptr->front = NULL;
    newQueue_ptr->front = NULL;
    return newQueue_ptr;
}

void queue_destroy(fifo_queue_t * queue)
{
    // assume queue is not null

    assert(queue->usage == 0);

    free(queue);
}

node_item_t * queue_pop(fifo_queue_t * queue)
{
    if (queue->usage > 0) { // nonempty
        if (queue->usage == 1) { // only one element
            node_item_t * onlyElement = queue->front;
            queue->front = NULL;
            queue->back = NULL;
            queue->usage = 0;

            // no longer in queue
            onlyElement->inQueue = false;

            return onlyElement;
        } else { // more than one element
            // once we pop front, the second element becomes new front
            node_item_t * currentFront = queue->front;
            node_item_t * currentSecond = queue->front->behind;

            // remove front element
            queue->front = currentSecond;

            // this new front element has nothing ahead
            currentSecond->ahead = NULL;

            queue->usage--;

            // no longer in queue
            currentFront->inQueue = false;

            return currentFront;
        }
        
    } 
    return NULL;
}

node_item_t * queue_top(fifo_queue_t * queue)
{
    if (queue->usage > 0) { // non-empty
        return queue->front;
    }
    return NULL;
}

int queue_push(fifo_queue_t * queue, node_item_t * node)
{
    // make sure we aren't enqueuing a node that already belongs to another queue.
    assert(!node_in_queue(node));
    
    if (queue->usage < queue->capacity) {
        // queue is empty
        if (queue->usage == 0) {
            queue->front = node;
            queue->back = node;
            queue->usage++;
        } else { // queue is not empty
            // get current last queue element
            node_item_t * currentLast = queue->back;

            // place our new node behind it
            currentLast->behind = node;
            node->ahead = currentLast;

            // queue has new caboose
            queue->back = node;

            queue->usage++;
        }

        node->inQueue = true;
        
        return 0;
    } 
    
    return -1;
}

node_item_t * queue_remove(fifo_queue_t * queue, unsigned int id)
{
    if (queue->usage > 0) {
        node_item_t * curr = queue->front;

        while (curr != NULL) {
            // check if current node has target id
            if (curr->id == id) {
                if (curr == queue->front) { // check by pointer equality
                    queue->front = curr->behind;
                }
                if (curr == queue->back) {
                    queue->back = curr->ahead;
                }
                // connect ahead and behind nodes
                if (curr->ahead != NULL) {
                    curr->ahead->behind = curr->behind;
                }
                if (curr->behind != NULL) {
                    curr->behind->ahead = curr->ahead;
                }
                

                curr->inQueue = false;
                queue->usage--;

                return curr;
            }

            curr = curr->behind;
        }
    }

    return NULL;
}


int 
queue_count(fifo_queue_t * queue)
{
    return queue->usage;
}

void queue_print(fifo_queue_t * queue)
{
    node_item_t * curr = queue->front;
    while (curr != NULL) {
        printf("%d ", curr->id);
        curr = curr->behind;
    }
    printf("\n");
}

bool queue_is_empty(fifo_queue_t * queue)
{
    return (queue->usage == 0);
}