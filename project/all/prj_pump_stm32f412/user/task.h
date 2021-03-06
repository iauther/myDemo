#ifndef __TASK_Hx__
#define __TASK_Hx__

#ifdef OS_KERNEL
#include "RTE_Components.h"
#include "cmsis_os2.h"
#include "drv/delay.h"
#include "msg.h"

enum {
    TASK_COM=0,
    TASK_DEV,
    
    TASK_MAX
};

typedef struct {
    msg_t           *msg;
    int             msg_maxlen;
    
    osTimerId_t     tmr_id;
    osThreadId_t    thread_id;
    osThreadFunc_t  thread_fn;
}task_handle_t;

void task_com_fn(void *arg);
void task_dev_fn(void *arg);

handle_t task_new(osThreadFunc_t task, U32 stack_size);
handle_t task_create(int id, osThreadFunc_t task, U32 stack_size);
int task_msg_send(int task_id, U8 evt, U8 type, void *data, U16 len);
int task_msg_post(int task_id, U8 evt, U8 type, void *data, U16 len);

int task_init(void);
int task_start(void);
#endif

#endif

