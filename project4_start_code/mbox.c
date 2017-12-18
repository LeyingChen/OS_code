#include "common.h"
#include "mbox.h"
#include "sync.h"
#include "scheduler.h"


typedef struct
{
    char msg[MAX_MESSAGE_LENGTH];
} Message;

typedef struct
{
    char name[MBOX_NAME_LENGTH+1];
    Message content[MAX_MBOX_LENGTH];
    int unused;
    int num_of_users;
    node_t send_queue;
    node_t recv_queue;
    uint32_t read_num;
    uint32_t write_num;
    //int msg_num;
    lock_t mtx;
    int full;
    int empty;
} MessageBox;


static MessageBox MessageBoxen[MAX_MBOXEN];
lock_t BoxLock;

/* Perform any system-startup
 * initialization for the message
 * boxes.
 */
void init_mbox(void)
{
    /* TODO */
    int i;
    for(i=0; i<MAX_MBOXEN; i++){
        MessageBoxen[i].name[0] = '\0';
        MessageBoxen[i].unused = 1;
        MessageBoxen[i].num_of_users = 0;
        queue_init(&MessageBoxen[i].send_queue);
        queue_init(&MessageBoxen[i].recv_queue);
        MessageBoxen[i].read_num = 0;
        MessageBoxen[i].write_num = 0;
        //MessageBoxen[i].msg_num = 0;
        lock_init(&MessageBoxen[i].mtx);
        MessageBoxen[i].full = 0;
        MessageBoxen[i].empty = 1;
    }
}

/* Opens the mailbox named 'name', or
 * creates a new message box if it
 * doesn't already exist.
 * A message box is a bounded buffer
 * which holds up to MAX_MBOX_LENGTH items.
 * If it fails because the message
 * box table is full, it will return -1.
 * Otherwise, it returns a message box
 * id.
 */
mbox_t do_mbox_open(const char *name)
{
    (void)name;
    /* TODO */
    mbox_t i;
    for(i=0; i<MAX_MBOXEN; i++){
        if(same_string(MessageBoxen[i].name, name)){
            MessageBoxen[i].num_of_users++;
            return i;
        }
    }

    for(i=0; i<MAX_MBOXEN; i++){
        if(MessageBoxen[i].unused == 1){
            lock_acquire(&MessageBoxen[i].mtx);
            MessageBoxen[i].unused = 0;
            MessageBoxen[i].num_of_users++;
            bcopy(name, MessageBoxen[i].name, strlen(name));
            current_running->using_box[i] = 1;
            lock_release(&MessageBoxen[i].mtx);
            return i;
        }
    }
}

/* Closes a message box
 */
void do_mbox_close(mbox_t mbox)
{
    (void)mbox;
    /* TODO */
    enter_critical();
    current_running->using_box[mbox] = 0;
    leave_critical();

    lock_acquire(&MessageBoxen[mbox].mtx);
    if(--MessageBoxen[mbox].num_of_users == 0){
        MessageBoxen[mbox].unused = 1;
        MessageBoxen[mbox].name[0] = '\0';
        MessageBoxen[mbox].empty = 1;
        queue_init(&MessageBoxen[mbox].send_queue);
        queue_init(&MessageBoxen[mbox].recv_queue);
    }
    lock_release(&MessageBoxen[mbox].mtx);
}

/* Determine if the given
 * message box is full.
 * Equivalently, determine
 * if sending to this mbox
 * would cause a process
 * to block.
 */
int do_mbox_is_full(mbox_t mbox)
{
    (void)mbox;
    /* TODO */
    return MessageBoxen[mbox].read_num + MAX_MBOX_LENGTH == MessageBoxen[mbox].write_num;
}

int do_mbox_is_empty(mbox_t mbox)
{
    (void)mbox;
    return MessageBoxen[mbox].read_num == MessageBoxen[mbox].write_num;
}

/* Enqueues a message onto
 * a message box.  If the
 * message box is full, the
 * process will block until
 * it can add the item.
 * You may assume that the
 * message box ID has been
 * properly opened before this
 * call.
 * The message is 'nbytes' bytes
 * starting at offset 'msg'
 */
void do_mbox_send(mbox_t mbox, void *msg, int nbytes)
{
    (void)mbox;
    (void)msg;
    (void)nbytes;
    /* TODO */
    enter_critical();
    if(do_mbox_is_full(mbox))
        block(&MessageBoxen[mbox].send_queue);
    leave_critical();

    lock_acquire(&MessageBoxen[mbox].mtx);
    int i;
    for(i=0; i<nbytes && i<MAX_MESSAGE_LENGTH; i++)
        MessageBoxen[mbox].content[MessageBoxen[mbox].write_num % MAX_MBOX_LENGTH].msg[i] = ((char *)msg)[i];

    MessageBoxen[mbox].write_num += 1;
    lock_release(&MessageBoxen[mbox].mtx);

    enter_critical();
    while(!is_empty(&MessageBoxen[mbox].recv_queue))
        unblock((pcb_t*)dequeue(&MessageBoxen[mbox].recv_queue));
    leave_critical();
}

/* Receives a message from the
 * specified message box.  If
 * empty, the process will block
 * until it can remove an item.
 * You may assume that the
 * message box has been properly
 * opened before this call.
 * The message is copied into
 * 'msg'.  No more than
 * 'nbytes' bytes will by copied
 * into this buffer; longer
 * messages will be truncated.
 */
void do_mbox_recv(mbox_t mbox, void *msg, int nbytes)
{
    (void)mbox;
    (void)msg;
    (void)nbytes;
    /* TODO */
    enter_critical();
    if(do_mbox_is_empty(mbox))
        block(&MessageBoxen[mbox].recv_queue);
    leave_critical();

    lock_acquire(&MessageBoxen[mbox].mtx);
    int i;
    for(i=0; i<nbytes && i<MAX_MESSAGE_LENGTH; i++)
        ((char*)msg)[i] = MessageBoxen[mbox].content[MessageBoxen[mbox].read_num % MAX_MBOX_LENGTH].msg[i];
    MessageBoxen[mbox].read_num++;
    lock_release(&MessageBoxen[mbox].mtx);

    enter_critical();
    while(!is_empty(&MessageBoxen[mbox].send_queue))
        unblock((pcb_t *)dequeue(&MessageBoxen[mbox].send_queue));
    leave_critical();
}
