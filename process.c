#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>

struct message
{
	long mtype;
	char mtext[10000];
} msg;
int msglen=10000,msgid;

int toint(char S[]);
// void pause_fun();
void notify_handler();
void suspend_handler();
void extract(char *S[], int *NOI,int *SleepProb,int *Priority,int *SleepTime);

int done_waiting,scheduler_pid;

int main(int argc, char *argv[])
{
	signal(SIGUSR1,suspend_handler);
	signal(SIGUSR2,notify_handler);

    srand(time(NULL)); //Seeding srand()

    int pid=(int)getpid(),flag=1,pr;
    printf("PID %d\n",pid);
    int NOI,SleepProb,Priority,SleepTime; //SleepProb is multiplied by a factor of 100 i.e 0.3->30
    int i=1,j,k,timequanta=1000,temp,status;
    key_t key=1024;
    extract(argv,&NOI,&SleepProb,&Priority,&SleepTime);
    msgid=msgget(key,IPC_CREAT|0644);
    memset(msg.mtext,'\0',msglen);
    sprintf(msg.mtext,"%d ",getpid());
    strcat(msg.mtext,argv[2]);
    msg.mtype=200;
	status=msgsnd(msgid,&msg,100,0);
    memset(msg.mtext,'\0',msglen);
	status = msgrcv(msgid, &msg, msglen, pid, 0);
    scheduler_pid=toint(msg.mtext);
    i=0;
    pause();
    printf("hola\n");
    while(i<NOI)//flag)
    {
        // do {
        //     status = msgrcv(msgid, &msg, msglen, 1000+pid, 0);
        // } while(status==-1);
        // pause_fun();
        // for(j=1;j<=timequanta;j++)
        // {
            i+=1;
            // if(i>NOI)
            // {
                // flag=0;
                // memset(msg.mtext,'\0',msglen);
                // msg.mtext[0]='t';
                // msg.mtype=getpid();
                // status=msgsnd(msgid,&msg,strlen(msg.mtext),0);
			    // kill(scheduler_pid,SIGUSR2);
       //          break;
            // }
            printf("PID: %d, %d\n",pid,i);
            pr=rand()%100+1;
            if(pr<=SleepProb)
            {
                /**
                	send I/O signal to scheduler
                */
                // memset(msg.mtext,'\0',msglen);
                // msg.mtext[0]='i';
                // msg.mtype=getpid();
                // status=msgsnd(msgid,&msg,strlen(msg.mtext),0);
                kill(scheduler_pid,SIGUSR1);
                printf("PID: %d Going for IO\n",pid);
                sleep(SleepTime);
                /**
                	inform scheduler via MSG Q
                */
                memset(msg.mtext,'\0',msglen);
                msg.mtext[0]='d';
                msg.mtype=getpid();
                status=msgsnd(msgid,&msg,strlen(msg.mtext),0);
                pause();
                break;
            }
        // }
        // if(j>timequanta)
        // {
        //     memset(msg.mtext,'\0',msglen);
        //     msg.mtext[0]='e';   //time quanta expired successfully
        //     msg.mtype=getpid();
        //     status=msgsnd(msgid,&msg,strlen(msg.mtext),0);
        // }
    }
    kill(scheduler_pid,SIGUSR2);
    sleep(100);
return 0;
}

void handler()
{
    done_waiting = 1;
}

// void pause_fun()
// {
//     done_waiting = 0;
//     msg.mtype=2000+getpid();
//     strcpy(msg.mtext,"ACK");
//     if(msgsnd(msgid,&msg,10,0)==-1);
//     signal(SIGINT, handler);
//     while ( !done_waiting );
// }

int toint(char S[])
{
    int i,j,n=strlen(S);
    for(i=0,j=0;i<n;i++)
        if(S[i]<='9' && S[i]>='0')
            j=j*10 + S[i]-'0';
        else
            break;
    return j;
}

void extract(char *S[], int *NOI,int *SleepProb,int *Priority,int *SleepTime)
{
    *NOI=toint(S[1]);
    *Priority=toint(S[2]);
    *SleepProb=toint(S[3]);
    *SleepTime=toint(S[4]);
}

void suspend_handler()
{
	signal(SIGUSR1,suspend_handler);
	signal(SIGUSR2,notify_handler);
	pause();
}

void notify_handler()
{
	signal(SIGUSR1,suspend_handler);
	signal(SIGUSR2,notify_handler);
}