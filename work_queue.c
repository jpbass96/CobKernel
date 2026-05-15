#include "work_queue.h"
#include "util.h"
#include "types.h"
#include "arm.h"
#include "malloc.h"



struct work_queue_entry *create_work_queue_entry() {
    struct work_queue_entry *q;

    q = kmalloc(sizeof(struct work_queue_entry));
    if (q == NULL) {
        LOG_ERROR("Could not allocate required memory for work queue\n\r");
        return NULL;
    }

    //initialize submission and completion semaphores.  Submission queue
    //starts empty so the semaphore must be taken. Completion
    //queue starts at 1 so work cna be posted.
    arm64_init_semaphore(&(q->submission_sem));
    arm64_take_semaphore_exclusive(&(q->submission_sem));
    arm64_init_semaphore(&(q->completion_sem));
    q->result = RESULT_INVALID;

    return q;
}

//for now assume that the entire queue + buffer is allocated from one malloc
void delete_work_queue_entry(struct work_queue_entry *q) {
    kfree(q);
}

//posts some work to a queue. blocks if the queue is not available.
void post_work_blocking(struct work_queue_entry *q, int (*next_task)(void *), void *params) {
    LOG_DEBUG("Taking completion semaphore\n\r");
    arm64_take_semaphore_exclusive(&(q->completion_sem));
    //LOG_DEBUG
    q->next_task = next_task;
    q->params = params;
    LOG_DEBUG("Putting submission semaphore\n\r");
    arm64_put_semaphore_exclusive(&(q->submission_sem));
}

void wait_work_done(struct work_queue_entry *q) {
    //take completino sem, but do not intend to post any work.
    //Instead immediately free the sem so that we know its done.
    //This only works if there is precisely one thread that will
    //try to post work. Otherwise another submitter thread
    //might submit work before you get results
    LOG_DEBUG("Waiting work done\n\r");
    arm64_take_semaphore_exclusive(&(q->completion_sem));
    arm64_put_semaphore_exclusive(&(q->completion_sem));
    LOG_DEBUG("work is done done\n\r");
}

void execute_work(struct work_queue_entry *q) {

    LOG_DEBUG("Completer waiting for submission\n\r");
    arm64_take_semaphore_exclusive(&(q->submission_sem));
    LOG_DEBUG("Completer got submission\n\r");
    q->result = q->next_task(q->params);
 
    arm64_put_semaphore_exclusive(&(q->completion_sem));
}