#include "msq_common.h"

int find_que_id(int pid)
{
    int i = 0;
    for (i = 0; i < CLIENT_NUM; i++)
        if (g_opponent_list[i].pid == pid)
            return g_opponent_list[i].qid;
    return -1;
}

//server's msq send
int server_s_msqsnd(int pid, MsgType *msg_buff)
{
    int qid = find_que_id(pid);
    if (qid == -1)
        return -1;

    msg_buff->pid_to = pid;
    return common_msqsnd(qid, msg_buff);
}

//server's msq relay
int server_s_msqrel(int pid, MsgType *msg_buff)
{
    int qid = find_que_id(pid);
    if (qid == -1)
        return -1;

    // can't use common_msgsnd()   it changes pid_from
    msg_buff->mtype = g_my_type == IM_CLIENT ? MTYPE_TO_SERVER : MTYPE_TO_CLIENT;
    int msg_size = sizeof(*msg_buff) - sizeof(msg_buff->mtype);

    return msgsnd(qid, msg_buff, msg_size, IPC_NOWAIT);
}

//server's msq broadcast
void server_s_msqbrc(MsgType *msg_buff)
{
    msg_buff->pid_to = PID_TO_ALL;
    int i = 0;
    for (i = 0; i < CLIENT_NUM; i++)
        common_msqsnd(g_opponent_list[i].qid, msg_buff);
}

//server's msq receive and excute it away
int server_s_msqrcv_excute()
{
    MsgType msg_buff, reply;

    int client_no = 0, nbytes = 0;
    for (client_no = 0; client_no < CLIENT_NUM; client_no++)
    {
        nbytes = common_msqrcv(g_opponent_list[client_no].qid, &msg_buff);
        if (nbytes <= 0)
            continue;

        int err;
        switch (msg_buff.pid_to)
        {
        case PID_TO_UNKNOWN: // initial contact from client!
            printf("========================(here comes a new chatter)========================\n");
            g_opponent_list[client_no].pid = msg_buff.pid_from;
            reply.pid_from = g_my_pid;
            reply.pid_to = PID_TO_UNKNOWN; // to not change it, use msq_relay fuction

            // reply cur_pos
            sprintf(reply.mtext, "%d", cur_pos);
            server_s_msqrel(msg_buff.pid_from, &reply);
            // no "break;" new participatient message is public chat.

        case PID_TO_ALL: // public chat
            //printf("public chat from %d, %s\n", msg_buff.pid_from, msg_buff.mtext);

            cur_pos = (cur_pos + 1) % LOG_SAVE_MAX;
            
            chat_log[cur_pos].pid_from = msg_buff.pid_from;
            chat_log[cur_pos].pid_to = msg_buff.pid_to;
            chat_log[cur_pos].mtype = msg_buff.mtype;
            strcpy(chat_log[cur_pos].mtext, msg_buff.mtext);

            printf("log%2d-from%dto%d : %s\n", cur_pos, chat_log[cur_pos].pid_from, chat_log[cur_pos].pid_to, chat_log[cur_pos].mtext);

            sprintf(reply.mtext, "%d", cur_pos);
            server_s_msqbrc(&reply);
            break;

        default: // private wispher
            //printf("private wispher from %d to %d, %s\n", msg_buff.pid_from, msg_buff.pid_to, msg_buff.mtext);
            printf("wispher\t");
            err = server_s_msqrel(msg_buff.pid_to, &msg_buff);
            if (err != 0)
            {
                
                strcpy(reply.mtext, "wispher FAIL");
                server_s_msqsnd(msg_buff.pid_from, &reply);
                printf("FAIL\tfrom %d to %d : %s\n", msg_buff.pid_from, msg_buff.pid_to, msg_buff.mtext);
                printf("\tuser list : ");

                int i;
                for(i = 0; i < CLIENT_NUM; i++){
                    printf("%d,  ", g_opponent_list[i].pid);
                }
                printf("\n");
            }
            else
            {
                printf("SUCCESS\tfrom %d to %d : %s\n", msg_buff.pid_from, msg_buff.pid_to, msg_buff.mtext);
            }
            break;
        }
    }
}

void server_s_init()
{
    common_shared_mem_init();

    g_my_pid = (int)getpid();
    g_my_type = IM_SERVER;
    g_opponent_num = CLIENT_NUM;

    int i = 0;
    for (i = 0; i < CLIENT_NUM; i++)
    {
        g_opponent_list[i].key = STARTING_KEY + i;
        g_opponent_list[i].pid = PID_TO_UNKNOWN;
        g_opponent_list[i].qid = msgget(STARTING_KEY + i, IPC_CREAT | 0666);
    }

    
}

int main(int argc, char **argv)
{
    server_s_init();

    while (1)
    {
        server_s_msqrcv_excute();
    }
}
