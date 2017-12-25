#include <pthread.h>
#include "msq_common.h"

int client_s_msqsnd(MsgType *msg_buff)
{
    return common_msqsnd(g_opponent_list[0].qid, msg_buff);
}

int client_s_msqrcv(MsgType *msg_buff)
{
    return common_msqrcv(g_opponent_list[0].qid, msg_buff);
}

// for client : common_msq_init (msq_id),   0 <= msq_id < g_opponent_num
void client_s_init(int msq_key)
{
    common_shared_mem_init();

    g_my_pid = (int)getpid();
    int i = 0;
    
    if (0 <= msq_key && msq_key < CLIENT_NUM)
    {
        g_my_type = IM_CLIENT;
        g_opponent_list[0].key = STARTING_KEY + msq_key;
        g_opponent_list[0].qid = msgget(STARTING_KEY + msq_key, 0666);
        g_opponent_list[0].pid = PID_TO_UNKNOWN;
        
        MsgType msg_buff;
        int nbyte = 9999;
        while (0 < nbyte)       // make it empty, do nothing
        {
            nbyte = client_s_msqrcv(&msg_buff);
        }
        msg_buff.pid_to = PID_TO_UNKNOWN;
        strcpy(msg_buff.mtext, "I join chatting");
        client_s_msqsnd(&msg_buff);
        // get response from server to set opponent pid
        while (nbyte <= 0)
        {
            nbyte = client_s_msqrcv(&msg_buff);
        }
        g_opponent_list[0].pid = msg_buff.pid_from;
    }
    else
    {
        printf("ERROR! client_s_init(msq_key) wrong value!\n");
        printf("you must input 0 ~ %d as id\n", CLIENT_NUM-1);
        exit(1);
    }
    
}

void print_logs();
void print_msg(MsgType *msg_buff);

void *new_thread_UI(void *arg)
{
    MsgType input;
    size_t leng = MESSAGE_LENG_MAX + 100;
    char *buff = (char *)malloc(leng);
    char *ptr;

    while (1)
    {

        int i, j;
        ptr = buff;
        getline((char **)&buff, &leng, stdin);
        buff[strlen(buff) - 1] = '\0';  // remove \n
        if(strlen(buff) == 0){continue;}
        if (buff[0] == '/')
        {
            if (buff[1] == 'w')
            {
                /*
                ptr = strtok(ptr, " ");
                ptr = strtok(NULL, " ");
                input.pid_to = atoi(ptr);

                
                ptr = &buff[i];
                strcpy(input.mtext, ptr);
                */
                sscanf(buff, "/w%d%s", &input.pid_to, input.mtext);
                const int len_buff = strlen(buff);
                const int len_scan = strlen(input.mtext);
                for (i = 0; i < len_buff; i++)
                {
                    for (j = 0; j < len_scan; j++)
                        if (buff[i + j] != input.mtext[j])
                            break;
                    if (j == len_scan)
                        break;
                }
                strcpy(input.mtext, buff + i);
                client_s_msqsnd(&input);
            }
            else if (buff[1] == 'l')
            {
                print_logs();
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
            //testprintf("public chat, %s\n", input.mtext);
            client_s_msqsnd(&input);
        }
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

void print_logs()
{
    int end = cur_pos;
    int start = (end + 1) % LOG_SAVE_MAX;
    int i = start;
    printf("\n\n====================== public chat log ======================\n");
    for (i = start; i < LOG_SAVE_MAX; i++)
    {
        printf("log%2d) %8d :\n\t%s\n", i, chat_log[i].pid_from, chat_log[i].mtext);
    }
    if (start != 0)
    {
        for (i = 0; i <= end; i++)
        {
            printf("log%2d) %8d :\n\t%s\n", i, chat_log[i].pid_from, chat_log[i].mtext);
        }
    }
    printf("\n====================== public chat log end ======================\n");
}

void print_msg(MsgType *msg_buff)
{
    if (msg_buff->pid_to == g_my_pid)
    {
        printf("%8d (/wispher):\n\t%s\n", msg_buff->pid_from, msg_buff->mtext);
    }
    else if (msg_buff->pid_to == PID_TO_ALL)    // not reading public chat message, we read shared memory
    {
        sscanf(msg_buff->mtext, "%d", &cur_pos);
        //printf("(pos%d)\n", cur_pos);
        MsgType *cur_msg = &chat_log[cur_pos];
        printf("%8d :\n\t%s\n", cur_msg->pid_from, cur_msg->mtext);
    }
    else if (msg_buff->pid_to == PID_TO_UNKNOWN)
    {
        printf("%8d (first connect):\n\t%s\n", msg_buff->pid_from, msg_buff->mtext);
    }
    else
    { // do nothing
     
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
    printf("start client #%d with pid %d\n", msq_key, g_my_pid);
    pthread_t tid;
    //pthread_mutex_init(&new_input, NULL);
    pthread_create(&tid, NULL, new_thread_UI, NULL);
    ori_thread_receiver();
}
