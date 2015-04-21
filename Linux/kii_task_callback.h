#ifndef KII_TASK_CALLBACK
#define KII_TASK_CALLBACK

typedef enum kii_task_code_t {
    KII_TASKC_OK,
    KII_TASKC_FAIL
} kii_task_code_t;

typedef (void)(*KII_TASK_ENTRY)(void* value);

typedef kii_task_code_t
(KII_TASK_CREATE*)
    (const char* name,
     KII_TASK_ENTRY entry,
     void* param,
     unsigned char* stk_start,
     unsigned int stk_size,
     unsigned int prio);

#endif
