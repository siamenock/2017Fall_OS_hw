#include "msq_common.h"

MsgType log[LOG_SAVE_MAX]; // TODO : move it in shared memory

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

    int i = 0, nbytes = 0;
    for (i = 0; i < CLIENT_NUM; i++)
    {
        nbytes = common_msqrcv(g_opponent_list[i].qid, &msg_buff);
        if (nbytes <= 0)
            continue;

        switch (msg_buff.pid_to)
        {
        case PID_TO_UNKNOWN: // initial contact from client!
            g_opponent_list[i].pid = msg_buff.pid_from;
            reply.pid_from = g_my_pid;
            reply.pid_to = PID_TO_UNKNOWN; // to not change it, use msq_relay fuction

            // TODO: reply something about shared memory
            strcpy(reply.mtext, "sth about shared memory");
            server_s_msqrel(msg_buff.pid_from, &reply);
            // no "break;" new participatient message is public chat.

        case PID_TO_ALL: // public chat
            // TODO : register this msg into log
            // TODO : broadcast new chat log pos to read in shared mem
            //        (not sending message itself through message queue)
            strcpy(reply.mtext, "new message!"); // change it!
            server_s_msqbrc(&reply);
            break;

        default: // private wispher
            int error = server_s_msqrel(msg_buff.pid_to, &msg_buff);
            if (error != 0)
            {
                strcpy(reply.mtext, "fail to send wispher!");
                server_s_msqsnd(msg_buff.pid_from, &reply);
            }
            break;
        }
    }
}

void server_s_init()
{
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

    // TODO : shared mem
}



int main(int argc, char**argv){
    server_s_init();

    while(1){
        server_s_msqrcv_excute();
    }
}
