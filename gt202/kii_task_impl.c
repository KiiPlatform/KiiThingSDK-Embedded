#include <mqx.h>
#include "main.h"
#include "kii_task_impl.h"

typedef struct _data {
    const char* name;
    KII_TASK_ENTRY entry;
    void* param;
} _data_t;

void kii_task1(uint_32 temp)
{
    _data_t* data = (_data_t*)temp;
    data->entry(data->param);
    free(data);
}

void kii_task2(uint_32 temp)
{
    _data_t* data = (_data_t*)temp;
    data->entry(data->param);
    free(data);
}

static int use_task1 = 0;

kii_task_code_t task_create_cb
    (const char* name,
     KII_TASK_ENTRY entry,
     void* param)
{
    int ret = 0;
    _data_t* data = NULL;
    uint_32 task = KII_TASK1;

    data = (_data_t*)malloc(sizeof(_data_t));
    data->name = name;
    data->entry = entry;
    data->param = param;

    if (use_task1 == 1)
    {
        task = KII_TASK2;
    }
    else
    {
        use_task1 = 1;
    }

    _task_create(0, task,(uint_32) data);

    if(ret == 0)
    {
        return KII_TASKC_OK;
    }
    else
    {
        return KII_TASKC_FAIL;
    }
}

void delay_ms_cb(unsigned int msec)
{
    _time_delay(msec);
}
/* vim:set ts=4 sts=4 sw=4 et fenc=UTF-8 ff=unix: */
