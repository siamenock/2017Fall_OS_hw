#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct{
    long mtype;     /* message type, must be > 0 */
    char mtext[20]; /* message data */
} MsgType;

int main(void){
    key_t key = 4071;
    int que_id = msgget(key, IPC_CREAT | 0666);
    MsgType first_msg, second_msg;

    int msg_size = sizeof(first_msg) - sizeof(first_msg.mtype);

    first_msg.mtype = 1;
    strcpy(first_msg.mtext, "hello world");
    msgsnd(que_id, &first_msg, msg_size, IPC_NOWAIT);

    second_msg.mtype = 2;
    strcpy(second_msg.mtext, "bye world");
    msgsnd(que_id, &second_msg, msg_size, IPC_NOWAIT);
}