#ifndef _work_queue_h
#define _work_queue_h

#include "types.h"
#include "arm.h"
//#include "util.h"

#define RESULT_SUCCESS 0
#define RESULT_INVALID 0xFFFF
#define RESULT_FAIL 1

//Num args must be defined at compile time
#define INIT_TASK_HANDLER_1ARG(function, arg_type)  int _## function ##_task_handler(void *params) { return function(*(arg_type*)params);}
#define INIT_TASK_HANDLER_2ARG(function, arg_type)  int _## function ##_task_handler(void *params) { return function(*(arg_type*)params, *(arg_type*)((void*)params + sizeof(arg_type)));}
#define INIT_TASK_HANDLER_3ARG(function, arg_type)  int _## function ##_task_handler(void *params) { return function(*(arg_type*)params, *(arg_type*)((void*)params + sizeof(arg_type)), *(arg_type*)((void*)params + sizeof(arg_type)*2) );}
#define INIT_TASK_HANDLER_4ARG(function, arg_type)  int _## function ##_task_handler(void *params) { return function(*(arg_type*)params, *(arg_type*)((void*)params + sizeof(arg_type)), *(arg_type*)((void*)params + sizeof(arg_type)*2), *(arg_type*)((void*)params + sizeof(arg_type)*3));}
#define INIT_TASK_HANDLE(function, NUM_ARGS, arg_type)  INIT_TASK_HANDLER_##NUM_ARGS##ARG(function, arg_type)
#define DECLARE_TASK_HANDLE(function, NUM_ARGS, arg_type) int _## function ##_task_handler(void *params)
#define QUEUE_WORK(q, function, params) post_work_blocking(q, _## function ##_task_handler, params)

//work queue has a single task in it.
//Semaphore is put by the main thread. Semaphore is taken by the worker thread when the task is done. Semaphore initializes as 0.
//params is a pointer containing parameters for the task. Its size and contents are task implementation defined
struct work_queue_entry{
  arm64_sem submission_sem;
  arm64_sem completion_sem;
  int (*next_task)(void *);
  void *params;
  int result;
};

//allocates all memory needed for work queue and returns pointer
struct work_queue_entry *create_work_queue_entry();
void delete_work_queue_entry(struct work_queue_entry *q);
void post_work_blocking(struct work_queue_entry *q, int (*next_task)(void *), void *params);
void wait_work_done(struct work_queue_entry *q);
void execute_work(struct work_queue_entry *q);


#endif