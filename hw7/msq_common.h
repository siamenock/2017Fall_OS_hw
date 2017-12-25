#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define testprintf printf
#define SHM_KEY 4070
#define STARTING_KEY 4071
#define CLIENT_NUM 3
#define LOG_SAVE_MAX 100
#define MESSAGE_LENG_MAX 1024

typedef struct
{
    long mtype;
    int pid_from, pid_to;
    char mtext[MESSAGE_LENG_MAX];
} MsgType;

enum
{   //MsgType.pid_to
    PID_TO_UNKNOWN = 0,
    PID_TO_ALL = 1,

    //MsgType.mtype
    IM_SERVER,
    IM_CLIENT,
    MTYPE_TO_SERVER = IM_SERVER,
    MTYPE_TO_CLIENT = IM_CLIENT,
    //MTYPE_FOR_INITIALIZE
};

typedef struct
{
    int pid, qid, key;
} PidKeySet;

int g_my_type; // need for msq protocol. must be initialized
int g_my_pid;  // need for msq protocol. must be initialized
int g_opponent_num = 1;                // CLIENT_NUM for server
PidKeySet g_opponent_list[CLIENT_NUM]; // client use only 1

MsgType* chat_log = NULL;   // for shared memory
int cur_pos = -1;           // shared mem pos


// no-wait-receive for my MsqType. automatically set mtype.
int common_msqrcv(int que_id, MsgType *msg_buff)
{
    int msg_size = sizeof(*msg_buff) - sizeof(msg_buff->mtype);
    int get_type = g_my_type;
    return (int)msgrcv(que_id, msg_buff, msg_size, get_type, IPC_NOWAIT);
}

// no-wait-send for my MsqType.
// auto set :  pid_from, mtype.
// you need to set  [msg_buff->pid_to]
int common_msqsnd(int que_id, MsgType *msg_buff)
{
    msg_buff->pid_from = g_my_pid;
    msg_buff->mtype = g_my_type == IM_CLIENT ? MTYPE_TO_SERVER : MTYPE_TO_CLIENT;

    int msg_size = sizeof(*msg_buff) - sizeof(msg_buff->mtype);
    return msgsnd(que_id, msg_buff, msg_size, IPC_NOWAIT);
}

void common_shared_mem_init(){
    
    int shm_id = shmget((key_t) SHM_KEY, sizeof(MsgType) * LOG_SAVE_MAX, 0666|IPC_CREAT);
    void* mem =  shmat(shm_id, (void*)0, 0);
    if(mem == (void*) -1){
        printf("common_shmget_shmat() failed!\n");
        exit(1);
    }
    chat_log = (MsgType *) mem;
    cur_pos = CLIENT_NUM -1;   // then it will start from 0
}


