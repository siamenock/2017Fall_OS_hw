#include <pthread.h>
#include "msq_common.h"

int client_s_msqsnd(MsgType *msg_buff)
{
    msg_buff->pid_to = g_opponent_list[0].pid;
    return common_msqsnd(g_opponent_list[0].qid, msg_buff);
}

int client_s_msqrcv (MsgType *msg_buff)
{
    return common_msqrcv(g_opponent_list[0].qid, msg_buff);
}


// for client : common_msq_init (msq_id),   0 <= msq_id < g_opponent_num
void client_s_init(int msq_key)
{
    g_my_pid = (int)getpid();
    int i = 0;

    if (0 <= msq_key && msq_key < g_opponent_num)
    {
        g_my_type = IM_CLIENT;
        g_opponent_list[0].key = STARTING_KEY + msq_key;
        g_opponent_list[i].qid = msgget(STARTING_KEY + msq_key, 0666);

        MsgType msg_buff;
        strcpy(msg_buff.mtext, "join chatting");
        client_s_msqsnd(&msg_buff);
        // TODO: get response from server and get shared memory address and connect
    }
    else
    {
        printf("ERROR! client_s_init(msq_key) wrong value!\n");
        printf("you must input 0 ~ %d as id\n", CLIENT_NUM);
        exit(1);
    }
    
}






void print_msg(MsgType *msg_buff);

void *new_thread_UI(void *arg)
{
    MsgType input;
    char buff[MESSAGE_LENG_MAX + 100];
    char *ptr;

    while (1)
    {
        ptr = buff;
        scanf("%s", buff);

        if (buff[0] == '/')
        {
            if (buff[1] == 'w')
            {
                ptr = strtok(ptr, " ");
                input.pid_to = atoi(ptr);
                ptr = strtok(ptr, " ");
                strcpy(input.mtext, ptr);
            }
            else if (buff[1] == 'l')
            {
                // TODO: show chat log
                continue;
            }
            else
            {
                printf("no cmd like that\n");
                continue;
            }
        }
        else
        {
            input.pid_to = PID_TO_ALL;
            strcpy(input.mtext, ptr);
        }

        client_s_msqsnd(&input);
    }
}

void ori_thread_receiver()
{
    int nbyte = -1;
    MsgType msg_buff;
    while (1)
    {
        nbyte = client_s_msqrcv(&msg_buff);
        if (nbyte <= 0)
            continue;

        print_msg(&msg_buff);
    }
}

void print_msg(MsgType *msg_buff)
{
    if (msg_buff->pid_to == g_my_pid)
    {
        printf("%8d (/wispher):\n\t%s\n\n", msg_buff->pid_from, msg_buff->mtext);
    }
    else if (msg_buff->pid_to == PID_TO_ALL)
    {
        printf("%8d :\n\t%s\n\n", msg_buff->pid_from, msg_buff->mtext);
    }
    else if (msg_buff->pid_to == PID_TO_UNKNOWN)
    {
        printf("%8d (system):\n\t%s\n\n", msg_buff->pid_from, msg_buff->mtext);
    }
    else
    {
        printf("server send me a wrong message. pdi_to : %d\n", msg_buff->pid_to);
        printf("%8d :\n\t%s\n\n", msg_buff->pid_from, msg_buff->mtext);
        printf("\n\n");
    }
}

int main(int argc, char **argv)
{
    if (argc != 1 + 1)
    {
        printf("only 1 argument as messeage queue key is allowed");
        exit(1);
    }
    int msq_key = atoi(argv[1]);
    client_s_init(msq_key);

    pthread_t tid;
    //pthread_mutex_init(&new_input, NULL);
    pthread_create(&tid, NULL, new_thread_UI, NULL);
}
