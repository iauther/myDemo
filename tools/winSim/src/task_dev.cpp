#include <windows.h>
#include "dev.h"
#include "win.h"
#include "log.h"
#include "pkt.h"
#include "data.h"
#include "task.h"

#define TIMER_MS            100
#define TIMEOUT             500
#define RETRIES             (TIMEOUT/TIMER_MS)

static BYTE dev_rx_buf[1000];
static int dev_exit_flag=0;
static int paras_tx_flag=0;
static UINT_PTR timerId;
static HANDLE devRxThreadHandle, devThreadHandle;


static void com_init(void)
{
    pkt_cfg_t cfg;

    curParas = DEFAULT_PARAS;
    cfg.retries = RETRIES;
    pkt_init(&cfg);
}

static U8 cmd_proc(void* data)
{
    cmd_t* c = (cmd_t*)data;
    //n950_send_cmd(c->cmd, c->para);

    return ERROR_NONE;
}


static U8 upgrade_proc(void* data)
{
    int r = 0;
    static U16 upg_pid = 0;
    upgrade_pkt_t* upg = (upgrade_pkt_t*)data;

    if (upg->pid == 0) {
        upg_pid = 0;
    }

    if (upg->pid != upg_pid) {
        return ERROR_FW_PKT_ID;
    }

    if (upg->pid == upg->pkts - 1) {
        //upgrade finished
    }
    else {
        upg_pid++;
    }

    return r;
}

const char* typeString[TYPE_MAX] = {
    "TYPE_CMD",
    "TYPE_STAT",
    "TYPE_ACK",
    "TYPE_SETT",
    "TYPE_PARAS",
    "TYPE_ERROR",
    "TYPE_UPGRADE",
};
static void error_print(U8 type, U8 error)
{
    switch (error) {
    case ERROR_NONE:
        log("___ pkt is ok\n");
        break;
    case ERROR_DEV_PUMP_RW:
        log("___ pump read/write error\n");
        break;
    case ERROR_DEV_MS4525_RW:
        log("___ ms4525 read/write error\n");
        break;
    case ERROR_DEV_E2PROM_RW:
        log("___ eeprom read/write error\n");
        break;
    case ERROR_PKT_TYPE:
        log("___ pkt type is wrong\n");
        break;
    case ERROR_PKT_MAGIC:
        log("___ pkt magic is wrong\n");
        break;
    case ERROR_PKT_LENGTH:
        log("___ pkt length is wrong\n");
        break;
    case ERROR_PKT_CHECKSUM:
        log("___ pkt checksum is wrong\n");
        break;
    case ERROR_DAT_TIMESET:
        log("___ para timeset is wrong\n");
        break;
    case ERROR_DAT_VACUUM:
        log("___ para vacuum is wrong\n");
        break;
    case ERROR_DAT_VOLUME:
        log("___ para volume is wrong\n");
        break;
    case ERROR_FW_INFO_VERSION:
        log("___ fw version is wrong\n");
        break;
    case ERROR_FW_INFO_BLDTIME:
        log("___ fw buildtime is wrong\n");
        break;
    case ERROR_FW_PKT_ID:
        log("___ fw pkt id is wrong\n");
        break;
    case ERROR_FW_PKT_LENGTH:
        log("___ fw pkt length is wrong\n");
        break;
    case ERROR_ACK_TIMEOUT:
        log("___ %s, ack is timeout\n", typeString[type]);
        break;
    case ERROR_UPG_FAILED:
        log("___ upgrade failed\n");
        break;
    default:
        break;
    }
}
static void pkt_print(pkt_hdr_t* p)
{
    log("_____ pkt.magic: 0x%08x\n", p->magic);
    log("_____ pkt.type:  0x%02x\n", p->type);
    log("_____ pkt.flag:  0x%02x\n", p->flag);
    log("_____ pkt.dataLen: %d\n",   p->dataLen);

    if (p->dataLen>0) {
        switch (p->type) {
        case TYPE_CMD:
        {
            if (p->dataLen==sizeof(cmd_t)) {
                cmd_t* cmd = (cmd_t*)p->data;
                log("_____ cmd.cmd: 0x%08x\n", cmd->cmd);
                log("_____ cmd.para: %d\n", cmd->para);
            }
        }
        break;
        case TYPE_ACK:
        {
            if (p->dataLen == sizeof(ack_t)) {
                ack_t* ack = (ack_t*)p->data;
                log("_____ ack.type: %d\n", ack->type);
                log("_____ ack.error: %d\n", ack->error);
            }
        }
        break;
        case TYPE_SETT:
        {
            if (p->dataLen == sizeof(setts_t)) {
                setts_t* setts = (setts_t*)p->data;
                log("_____ setts data\n");
            }
            else if (p->dataLen == sizeof(sett_t)) {
                sett_t* set = (sett_t*)p->data;
                log("_____ sett data\n");
            }
            else if (p->dataLen == sizeof(U8)) {
                U8* mode = (U8*)p->data;
                log("_____ mode: 0x%08x\n", *mode);
            }
        }
        break;
        case TYPE_PARAS:
        {
            if (p->dataLen == sizeof(paras_t)) {
                paras_t* paras = (paras_t*)p->data;
                log("_____ PARAS DATA\n");
            }
        }
        break;
        case TYPE_ERROR:
        {
            if (p->dataLen == sizeof(error_t)) {
                error_t* err = (error_t*)p->data;
                log("_____ err.type: %d\n", err->type);
                log("_____ err.error: %d\n", err->error);
            }
        }
        break;
        case TYPE_UPGRADE:
        {
            if (p->dataLen == sizeof(upgrade_pkt_t)) {
                upgrade_pkt_t* up = (upgrade_pkt_t*)p->data;
                log("_____ pkt.obj:  %d\n", up->obj);
                log("_____ pkt.pkts: %d\n", up->pkts);
                log("_____ pkt.pid:  %d\n", up->pid);
                log("_____ pkt.dataLen: %d\n", up->dataLen);
            }
        }
        break;
        }
    }
}


static U8 com_proc(pkt_hdr_t* p, U16 len)
{
    U8 ack = 0, err;





    if (p->askAck)  ack = 1;

    err = pkt_hdr_check(p, len);
    if (err == ERROR_NONE) {
        switch (p->type) {
        case TYPE_CMD:
        {
            err = cmd_proc(p);
        }
        break;

        case TYPE_ACK:
        {
            ack_t* ack = (ack_t*)p->data;
            pkt_ack_update(ack->type);
        }
        break;

        case TYPE_SETT:
        {
            if (p->dataLen > 0) {
                if (p->dataLen == sizeof(sett_t)) {
                    sett_t* sett = (sett_t*)p->data;
                    curParas.setts.sett[sett->mode] = *sett;
                }
                else if (p->dataLen == sizeof(curParas.setts.mode)) {
                    U8* m = (U8*)p->data;
                    curParas.setts.mode = *m;
                }
                else {
                    memcpy(&curParas.setts.sett, p->data, sizeof(curParas.setts));
                }
            }
            else {
                if (p->askAck) {
                    pkt_send_ack(p->type, 0); ack = 0;
                }
                pkt_send(TYPE_SETT, 0, &curParas.setts, sizeof(curParas.setts));
            }
        }
        break;

        case TYPE_PARAS:
        {
            if (p->dataLen > 0) {
                if (p->dataLen == sizeof(paras_t)) {
                    paras_t* paras = (paras_t*)p->data;
                    curParas = *paras;
                }
                else {
                    err = ERROR_PKT_LENGTH;
                }
            }
            else {
                if (p->askAck) {
                    pkt_send_ack(p->type, 0); ack = 0;
                }
                pkt_send(TYPE_PARAS, 0, &curParas, sizeof(curParas));
                paras_tx_flag = 1;
            }
        }
        break;

        case TYPE_UPGRADE:
        {
            if (p->askAck) {
                pkt_send_ack(p->type, 0); ack = 0;
            }
            err = upgrade_proc(p->data);
        }
        break;

        default:
        {
            err = ERROR_PKT_TYPE;
        }
        break;
        }
    }

    if (ack || (err && !ack)) {
        error_print(p->type, err);
        pkt_send_ack(p->type, err);
    }

    return err;
}

static int send_stat(void)
{
    stat_t stat;
                
    stat.temp = 88.9F;
    stat.aPres = 88.9F;
    stat.dPres = 88.9F;
    stat.pump.speed = 500;
    stat.pump.current = 650.0F;
    stat.pump.temp = 44.9F;
    stat.pump.fault = 0;
    stat.pump.cmdAck = 1;
    pkt_send(TYPE_STAT, 0, &stat, sizeof(stat));
    
    return 0;
}
static void timer_proc(void)
{
    U8  i;
    int r;

    if (!dev_is_opened()) {
        return;
    }

    if (!paras_tx_flag) {
        pkt_send(TYPE_PARAS, 1, &curParas, sizeof(curParas));
        paras_tx_flag = 1;
    }

    for (i = 0; i < TYPE_MAX; i++) {
        r = pkt_ack_check(i);
        if (r) {
            error_print(i, ERROR_ACK_TIMEOUT);
        }
    }

    send_stat();
}



static DWORD WINAPI dev_rx_thread(LPVOID lpParam)
{
    int rlen;
    pkt_hdr_t* p = (pkt_hdr_t*)dev_rx_buf;

    while (1) {
        rlen = dev_recv(p, sizeof(dev_rx_buf));
        if (rlen > 0) {
            com_proc(p, rlen);
        }

        if (dev_exit_flag) {
            break;
        }
    }

    return 0;
}


static void send_evt(int evt)
{
    PostThreadMessage(GetThreadId(devThreadHandle), evt, NULL, NULL);
}
static void CALLBACK timer_callback(HWND hwnd, UINT message, UINT_PTR iTimerID, DWORD dwTime)
{
    send_evt(WM_USER);
}


static DWORD WINAPI dev_thread(LPVOID lpParam)
{
    MSG msg;
    
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (dev_exit_flag) {
            break;
        }

        switch (msg.message) {

        case WM_USER:
        {
            timer_proc();
        }
        break;
        }


        
    }

    return 0;
}




int task_dev_start(void)
{
    dev_exit_flag = 0;

    timerId = SetTimer(NULL, 1, 100, timer_callback);
    devRxThreadHandle = CreateThread(NULL, 0, dev_rx_thread, NULL, 0, NULL);
    devThreadHandle = CreateThread(NULL, 0, dev_thread, NULL, 0, NULL);

    return 0;
}

int task_dev_stop(void)
{
    dev_exit_flag = 1;

    KillTimer(NULL, timerId);
    if (devRxThreadHandle) {
        WaitForSingleObject(devRxThreadHandle, INFINITE);
        CloseHandle(devRxThreadHandle);
        devRxThreadHandle = NULL;
    }

    if (devThreadHandle) {
        send_evt(WM_USER);
        WaitForSingleObject(devThreadHandle, INFINITE);
        CloseHandle(devThreadHandle);
        devThreadHandle = NULL;
    }
    

    return 0;
}

