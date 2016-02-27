#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

struct timetrack
{
    int pid;
    clock_t begin;
    float dispatched[10000];
    int dispatch_count;
    float enqueued[10000];
    int enqueue_count;
    float response;
    float turnaround;
} tt[400];

struct message
{
long mtype;
char mtext[10000];
} msg;
int msgid,msglen=10000,rstart=0,rend=0,wstart=0,wend=0,tt_count=0,process_count=1,total=1,valid=1;

struct process
{
    int pid;
    int Priority;
} ready_queue[100000],running,waiting_queue[100000];

void insert(char S[]);
void remove_entry(int pid);
void allocate(int isPR);
void enqueue(struct process q);
void notify(int pid);
void set_turnaround(int pid);
void set_enqueue(int pid);
void set_response(int pid);
float calculate_wait(int pid);
void set_dispatch(int pid);
int find_ind(int pid);
void io_handler();
void terminated_handler();

int main(int argc,char *argv[])
{
	signal(SIGUSR1,io_handler);
	signal(SIGUSR2,terminated_handler);
    if(argc!=2 || (strcmp(argv[1],"RR")!=0 && strcmp(argv[1],"PR")!=0))
        {
            printf("Incorrect Arguments!\n");
            return 1;
        }
    int timequanta=100000;
    if(argv[1][0]=='P')
        timequanta=200000;
    key_t key=1024;
    int status,i,j,running_pid,isPR=(timequanta==2000);//,issuccess;
    msgid=msgget(key,IPC_CREAT|0644);
    /**
    	Receive PID and Priority of first process
    */
    memset(msg.mtext,'\0',msglen);
    status = msgrcv(msgid, &msg, msglen, 200, 0);
    if(status==-1)
        return 1;
    insert(msg.mtext);
    /*
    	Send Scheduler PID to Received PID
    */
    memset(msg.mtext,'\0',msglen);
    sprintf(msg.mtext,"%d",getpid());
    msg.mtype=ready_queue[rend-1].pid;
    status=msgsnd(msgid,&msg,strlen(msg.mtext),0);
    // /*Send 'suspend' to Received PID*/
    // memset(msg.mtext,'\0',msglen);
    // strcpy(msg.mtext,"suspend");
    // msg.mtype=1000 + ready_queue[rend-1].pid;
    // while(status=msgsnd(msgid,&msg,strlen(msg.mtext),0)==-1);
    set_enqueue(ready_queue[0].pid);
    // int isdone=0,isio=0;
    while(process_count!=0)
    {
    	printf(" r : %d, w : %d, p : %d\n",rend-rstart,wend-wstart,process_count);
        if(rstart<rend){
            allocate(isPR); //schedule a process
            // status = msgrcv(msgid, &msg, msglen, 2000+running.pid, 0);
            printf("P%d %d is running\n",find_ind(running.pid),running.pid);
            valid=1;
            notify(running.pid);    //notify scheduled process
        }
        for(j=0;j<timequanta && valid==1;j++)
        	;
        if(j>=timequanta)
        	{
        		kill(SIGUSR1,running.pid);
        		printf("\ttimequanta expired\n");
        	}
        sleep(2);
        // isdone=0;
        // isio=0;
        // issuccess=0;
        /*
        	Receive PID Priority from msgQ (if any)
        */
        memset(msg.mtext,'\0',msglen);
        status = msgrcv(msgid, &msg, msglen, 200, IPC_NOWAIT);
        if(status!=-1)
            {
                process_count+=1;
                total+=1;
                insert(msg.mtext);
                /*
                	Send Scheduler PID
                */
                memset(msg.mtext,'\0',msglen);
                sprintf(msg.mtext,"%d",getpid());
                msg.mtype=ready_queue[rend-1].pid;
                status=msgsnd(msgid,&msg,strlen(msg.mtext),0);
                if(status==-1)
                    printf("timequanta sending to pid %ld failed\n",msg.mtype);
                set_enqueue(ready_queue[rend-1].pid);
                // /*Send 'suspend' to Received PID*/
                // memset(msg.mtext,'\0',msglen);
                // strcpy(msg.mtext,"suspend");
                // msg.mtype=ready_queue[rend-1].pid+1000;
                // while(status=msgsnd(msgid,&msg,strlen(msg.mtext),0)==-1);
            }
        // memset(msg.mtext,'\0',msglen);
        // status=-1;
        // while (status==-1) status = msgrcv(msgid, &msg, msglen, running.pid, 0);
        // if(msg.mtext[0]=='t')   //process terminated
        //     {
        //         isdone=1;
        //     }
        // else if(msg.mtext[0]=='i')  //process requested IO
        //     {
        //         isio=1;
        //         printf("P%d %d requests I/O\n",find_ind(running.pid),running.pid);
        //     }
        // else if(msg.mtext[0]=='e')  //time quanta expired
        //     {
        //         issuccess=1;
        //     }
        // if(isdone==1){
            // process_count-=1;
            // set_turnaround(running.pid);
        //     }
        // else{
                // /*Send 'suspend' to Received PID*/
                // memset(msg.mtext,'\0',msglen);
                // strcpy(msg.mtext,"suspend");
                // msg.mtype=1000 + running.pid;
                // while(status=msgsnd(msgid,&msg,strlen(msg.mtext),IPC_NOWAIT)==-1);
                // if(isio==1){
                //     waiting_queue[wend++]=running;
                // }
                // else
            if(valid==1)
                    {
                        enqueue(running);
                        set_enqueue(running.pid);
                    }
                // }
            do{
                for(i=wstart;i<wend;i++)
                    {
                        memset(msg.mtext,'\0',msglen);
                        status = msgrcv(msgid, &msg, msglen, waiting_queue[i].pid, IPC_NOWAIT);
                        if(status!=-1 && msg.mtext[0]=='d') //returned from I/O
                        {
                            ready_queue[rend++]=waiting_queue[i];
                            printf("P%d %d completes I/O\n",find_ind(waiting_queue[i].pid),waiting_queue[i].pid);
                            for(j=i;j<wend;j++)
                                waiting_queue[j]=waiting_queue[j+1];
                            wend--;
                            set_enqueue(ready_queue[rend-1].pid);
                        }
                    }
            }while(rstart>=rend && process_count!=0);
    }
    float res=0,turn=0,wait=0;
    FILE *fp=fopen("result.txt","w");
    fprintf(fp,"Process\tPID\t\tResponseT\tWaitingT\tTurnaroundT\t(in seconds)\n");
    for(i=0;i<total;i++)
        {
            fprintf(fp,"P%d\t\t%d\t%f\t%f\t%f\n", find_ind(tt[i].pid),tt[i].pid,tt[i].response,calculate_wait(tt[i].pid),tt[i].turnaround);
            res+=tt[i].response;
            turn+=tt[i].turnaround;
            wait+=calculate_wait(tt[i].pid);
        }
    fprintf(fp,"Average Values\t%f\t%f\t%f",res/tt_count,wait/tt_count,turn/tt_count);
    fclose(fp);
    return 0;
}

void io_handler()
{
	waiting_queue[wend]=running;
	wend+=1;
	printf("\tinside io handler w : %d\n",wend-wstart );
	valid=0;
    printf("P%d %d requests I/O\n",find_ind(running.pid),running.pid);
}

void terminated_handler()
{
        process_count-=1;
        valid=0;
        set_turnaround(running.pid);

}

int find_ind(int pid)
{
    int i=0;
    while (tt[i++].pid!=pid);
    return i;
}

float calculate_wait(int pid)
{
    int i=0,j,k;
    float ans=0,temp=0;
    for(;tt[i].pid!=pid;i++);
    for(j=0;j<tt[i].dispatch_count;j++)
        ans+=tt[i].dispatched[j] - tt[i].enqueued[j];
    return ans/CLOCKS_PER_SEC;
}

void set_dispatch(int pid)
{
    int i=0;
    for(;tt[i].pid!=pid;i++);
    tt[i].dispatched[tt[i].dispatch_count]=(float)clock();
    tt[i].dispatch_count++;
}

void set_enqueue(int pid)
{
    int i=0;
    for(;tt[i].pid!=pid;i++);
    tt[i].enqueued[tt[i].enqueue_count]=(float)clock();
    tt[i].enqueue_count++;
}

void set_turnaround(int pid)
{
    int i=0;
    for(;tt[i].pid!=pid;i++);
    tt[i].turnaround=((float)(clock()-tt[i].begin))/CLOCKS_PER_SEC;
}

void notify(int pid)
{
    kill(pid,SIGUSR2);
    printf("Notified %d\n",pid);
    set_dispatch(pid);
    set_response(pid);
}

void enqueue(struct process q)
{
    ready_queue[rend]=q;
    rend+=1;
}

void allocate(int isPR)
{
    int i,j;
    if(isPR==0)
    {
        running=ready_queue[rstart];
        for(i=rstart;i<rend;i++){
            ready_queue[i]=ready_queue[i+1];
        }
        rend--;
        return ;
    }
    int min_priority=100;
    for(i=rstart;i<rend;i++)
        if(ready_queue[i].Priority<min_priority)
            min_priority=ready_queue[i].Priority;
    for(i=rstart;i<rend;i++)
        if(ready_queue[i].Priority==min_priority)
            {
                running=ready_queue[i];
                remove_entry(running.pid);
                break;
            }
}

void set_response(int pid)
{
    int i=0;
    for(;tt[i].pid!=pid;i++);
    if(tt[i].response<0)    //check whether already set?
        tt[i].response=((float)(clock()-tt[i].begin))/CLOCKS_PER_SEC;
        //if(!i) tt[i].response/=10;
}

void remove_entry(int pid)
{
    int i,j;
    for(i=rstart;i<rend;i++)
    {
        if(ready_queue[i].pid==pid)
        {
            for(j=i;j<rend;j++)
                ready_queue[j]=ready_queue[j+1];
            rend--;
            return ;
        }
    }
}

void insert(char S[])
{
    int i,n=strlen(S),j;
    for(j=0,i=0;i<n;i++)
        {
            if(S[i]<='9' && S[i]>='0')
                j=j*10+S[i]-'0';
            else
                break;
        }
    ready_queue[rend].pid=j;
    tt[tt_count].dispatch_count=0;
    tt[tt_count].pid=j;
    tt[tt_count].begin=clock();
    tt[tt_count].response=-1;
    tt[tt_count].enqueue_count=0;
    tt_count++;
    for(j=0,i+=1;i<n;i++)
        {
            if(S[i]<='9' && S[i]>='0')
                j=j*10+S[i]-'0';
            else
                break;
        }
    ready_queue[rend].Priority=j;
    rend++;
    printf("inside insert r : %d\n",rend-rstart);
}
