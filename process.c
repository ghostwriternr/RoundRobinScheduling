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
    int NOI,SleepProb,Priority,SleepTime; //SleepProb is on a base of 100 i.e 0.3->30
    int i=1,j,k,temp,status,count=0;
    key_t key=1025;
    extract(argv,&NOI,&SleepProb,&Priority,&SleepTime);
    msgid=msgget(key,IPC_CREAT|0644);
    memset(msg.mtext,'\0',msglen);
    sprintf(msg.mtext,"%d ",getpid());
    strcat(msg.mtext,argv[2]);
    msg.mtype=200;
	while(msgsnd(msgid,&msg,strlen(msg.mtext),0)==-1);
	printf("PID sent to scheduler\n");
    memset(msg.mtext,'\0',msglen);
	while(msgrcv(msgid, &msg, msglen, pid, 0)==-1);
    scheduler_pid=toint(msg.mtext);
    i=1;
    pause();
    while(i<=NOI)
    {
        printf("PID: %d, %d\n",pid,i);
        pr=rand()%100+1;
        if(pr<=SleepProb)
        {
        	count++;
            kill(scheduler_pid,SIGUSR1);
            printf("PID: %d Going for IO\n",pid);
            sleep(SleepTime);
            /**
            	inform scheduler via MSG Q
            */
            memset(msg.mtext,'\0',msglen);
            msg.mtext[0]='d';
            msg.mtype=getpid();
            while(msgsnd(msgid,&msg,strlen(msg.mtext),0)==-1);
			pause();
        }
    	i+=1;
	}
    kill(scheduler_pid,SIGUSR2);
    printf("\nProcess Terminated\n\n");
    getchar();
return 0;
}

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
	// printf("Suspended\n");
	pause();
	// printf("returning from suspension\n");
}

void notify_handler()
{
	// printf("Notified\n");
}