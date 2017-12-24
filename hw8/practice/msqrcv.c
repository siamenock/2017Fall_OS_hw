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
    int que_id = msgget(key, 0666);
    int get_type;
    MsgType msg_buff;

    int msg_size = sizeof(msg_buff) - sizeof(msg_buff.mtype);
    
    ssize_t nbytes = 0;
    printf("Please insert wanted MSG type(1 or 2)\n");
    scanf("%d", &get_type);
    nbytes = msgrcv(que_id, &msg_buff, msg_size, get_type, IPC_NOWAIT);
    if (nbytes > 0)
        printf("%s\n", msg_buff.mtext);
    else if (errno == ENOMSG)
        printf("empty queue\n");
    return 0;
}