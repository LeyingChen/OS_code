/* queue.c */
/* Do not change this file */

#include "common.h"
#include "queue.h"

void queue_init(node_t * queue){
  queue->prev = queue->next = queue;
}

node_t *dequeue(node_t * queue){
  node_t *item;
  
  item = queue->next;
  if (item == queue) {
    /* The queue is empty */
    item = NULL;
  } 
  else {
    /* Remove item from the queue */
    item->prev->next = item->next;
    item->next->prev = item->prev;
  }
  return item;
}

void enqueue(node_t * queue, node_t * item){
    item->prev = queue->prev;
    item->next = queue;
    item->prev->next = item;
    item->next->prev = item;
}

int is_empty(node_t *queue){
  if( queue->next == queue )
    return 1;
  else
    return 0;
}

node_t *peek(node_t *queue){
  if( queue->next == queue )
    return NULL;
  else
    return queue->next;
}

void enqueue_sort(node_t *q, node_t *item, node_lte comp){
    node_t *iq;
    for(iq = q->next; iq && iq!=q; iq=iq->next){
        if(comp(item, iq)){
            enqueue(iq, item);
            return;
        }
    }
    enqueue(q, item);
}

/*node_t *dequeue_pri(node_t *queue){
     node_t *item;
     pcb_t* temp;
     temp = (pcb_t*)peek(queue);
     for(item = peek(queue); item && item!=queue; item = item->next){
          if(temp->priority < ((pcb_t *)item)->priority)
               temp = (pcb_t *)item;
     }
     temp->node.prev->next = temp->node.next;
     temp->node.next->prev = temp->node.prev;
     temp->node.next = NULL;
     temp->node.prev = NULL;
     return (node_t *)temp;
}*/
