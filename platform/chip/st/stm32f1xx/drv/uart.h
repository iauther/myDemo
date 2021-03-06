#ifndef __UART_Hx__
#define __UART_Hx__

#include "types.h"

enum {
    MODE_POLL=0,
    MODE_IT,
    MODE_DMA,
};

enum {
    UART_1=0,
    UART_2,
    UART_3,
    UART_4,

    UART_MAX
};


typedef void (*rx_fn_t)(U8 *data, U16 data_len);

typedef struct {
    rx_fn_t             rx;
    U8                  *buf;
    U16                 blen;
    U16                 dlen;       //data length
}uart_para_t;

typedef struct {
    U8                  mode;
    U8                  port;
    U8                  useDMA;
    U32                 baudrate;
    uart_para_t         para;
}uart_cfg_t;

handle_t uart_init(uart_cfg_t *cfg);

int uart_deinit(handle_t *h);

int uart_read(handle_t h, U8 *data, U32 len);

int uart_write(handle_t h, U8 *data, U32 len);

int uart_rw(handle_t h, U8 *data, U32 len, U8 rw);
#endif /*__ usart_H */
