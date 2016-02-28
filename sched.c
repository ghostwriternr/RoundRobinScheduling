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
    float waiting;
    float response;
    float turnaround;
    clock_t dispatch;
    clock_t begin,prev_enqueue;
} tt[400];

struct message
{
	long mtype;
	char mtext[10000];
} msg;

int msgid,msglen=10000,rstart=0,rend=0,wstart=0,wend=0;
int tt_count=0,process_count=1,total=1,valid=1;

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
void suspend(int pid);
void set_turnaround(int pid);
void set_enqueue(int pid);
void set_response(int pid);
void set_dispatch(int pid);
int find_ind(int pid);
void io_handler();
void terminated_handler();
void check_io_returns();
void check_new_processes();

int main(int argc,char *argv[])
{
	signal(SIGUSR1,io_handler);
	signal(SIGUSR2,terminated_handler);
    int status,i,j,running_pid,isPR=0;
    if(argc!=2 || (strcmp(argv[1],"RR")!=0 && strcmp(argv[1],"PR")!=0))
        {
            printf("Incorrect Arguments!\n");
            return 1;
        }
    int timequanta=500;
    if(argv[1][0]=='P')
        {
        	timequanta=200;
        	isPR=1;
        }
    key_t key=1025;
    msgid=msgget(key,IPC_CREAT|0644);
    /**
    	Receive PID and Priority of first process
    */
    memset(msg.mtext,'\0',msglen);
    status = msgrcv(msgid, &msg, msglen, 200, 0);
    if(status==-1)
        return 1;
    insert(msg.mtext);
    printf("%d Very first Process\n",ready_queue[rend-1].pid);
    /*
    	Send Scheduler PID to Received PID
    */
    memset(msg.mtext,'\0',msglen);
    sprintf(msg.mtext,"%d",getpid());
    msg.mtype=ready_queue[rend-1].pid;
    status=msgsnd(msgid,&msg,strlen(msg.mtext),0);
    set_enqueue(ready_queue[0].pid);
    while(process_count!=0)
    {
        if(rstart<rend){
            // printf("before dispatch r : %d, w : %d, p : %d\n",rend-rstart,wend-wstart,process_count);
        	allocate(isPR); //schedule a process
            notify(running.pid);    //notify scheduled process
            // printf("Quanta : %d, P%d %d is running\n",timequanta,find_ind(running.pid),running.pid);
        }
        usleep(50);
        for(j=0;valid==1;j++)
		{
	        /*
	        	Receive PID Priority from msgQ (if any)
	        */
	        check_new_processes();
	        check_io_returns();
			if(j>=timequanta)
	    	{
	    		if(valid)
	    			suspend(running.pid);
	    		break;
			}
	    }
	    // printf("j final %d, tq %d\n",j,timequanta);
        do{
	    	check_new_processes();
	    	check_io_returns();
        }while(rstart>=rend && process_count!=0);
    	valid=1;
    }
    float res=0,turn=0,wait=0;
    FILE *fp=fopen("result.txt","w");
    fprintf(fp,"Process\t\tPID\tResponseT\tWaitingT\tTurnaroundT\t(in seconds)\n");
    for(i=0;i<total;i++)
        {
            fprintf(fp,"P%d\t\t%d\t%f\t%f\t%f\n", find_ind(tt[i].pid),tt[i].pid,tt[i].response,tt[i].waiting,tt[i].turnaround);
            res+=tt[i].response;
            turn+=tt[i].turnaround;
            wait+=tt[i].waiting;
        }
    fprintf(fp,"Average Values\t\t%f\t%f\t%f",res/tt_count,wait/tt_count,turn/tt_count);
    fclose(fp);
    return 0;
}

void io_handler()
{
	waiting_queue[wend]=running;
	wend+=1;
	// printf("\tinside io handler w : %d\n",wend-wstart );
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

// float calculate_wait(int pid)
// {
//     int i=0,j,k;
//     float ans=0,temp=0;
//     for(;tt[i].pid!=pid;i++);
//     for(j=0;j<tt[i].dispatch_count;j++)
//         ans+=tt[i].dispatched[j] - tt[i].enqueued[j];
//     return ans/CLOCKS_PER_SEC;
// }

void set_dispatch(int pid)
{
    int i=0;
    for(;tt[i].pid!=pid;i++);
    // tt[i].dispatched[tt[i].dispatch_count]=(float)clock();
    // tt[i].dispatch_count++;
    tt[i].waiting+=((float)(clock()-tt[i].prev_enqueue))/CLOCKS_PER_SEC;
}

void set_enqueue(int pid)
{
    int i=0;
    for(;tt[i].pid!=pid;i++);
    // tt[i].enqueued[tt[i].enqueue_count]=(float)clock();
    // tt[i].enqueue_count++;
    tt[i].prev_enqueue=clock();
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

void suspend(int pid)
{
	kill(pid,SIGUSR1);
    enqueue(running);
    set_enqueue(running.pid);
	// printf("Suspended %d\n", pid);
}
void enqueue(struct process q)
{
    ready_queue[rend]=q;
    rend+=1;
}

void allocate(int isPR)
{
    int i,j;
    if(isPR == 0)
    {
        running = ready_queue[rstart];
        printf("Ready Queue %d, ",ready_queue[rstart].pid);
        for( i = rstart; i < rend-1; i++ ){
	        ready_queue[i] = ready_queue[i+1];
        	printf("%d, ",ready_queue[i].pid);
        }
        printf("\n");
        rend = rend-1;
        return ;
    }
    int min_priority = 100;
    for(i = rstart;i < rend;i++)
        if(ready_queue[i].Priority < min_priority)
            min_priority = ready_queue[i].Priority;
    for(i = rstart;i < rend;i++)
        if(ready_queue[i].Priority == min_priority)
            {
                running = ready_queue[i];
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
    tt[tt_count].pid=j;
    tt[tt_count].begin=clock();
    tt[tt_count].response=-1;
    tt[tt_count].waiting=0;
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
    // printf("inside insert r : %d\n",rend-rstart);
}

void check_io_returns()
{
	int i,status,j;
    for(i=wstart;i<wend;i++)
    {
        memset(msg.mtext,'\0',msglen);
        status = msgrcv(msgid, &msg, msglen, waiting_queue[i].pid, IPC_NOWAIT);
        // printf("expecting io return of %d\n",waiting_queue[i].pid);
        if(status!=-1 && msg.mtext[0]=='d') //returned from I/O
        {
            ready_queue[rend++]=waiting_queue[i];
            printf("P%d %d completes I/O\n",find_ind(waiting_queue[i].pid),waiting_queue[i].pid);//sleep(1);
            for(j=i;j < wend-1;j++)
                waiting_queue[j]=waiting_queue[j+1];
            wend--;
            set_enqueue(ready_queue[rend-1].pid);
        }
    }

}

void check_new_processes()
{
	int status;
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
            while(msgsnd(msgid,&msg,strlen(msg.mtext),0)==-1);
            if(status==-1)
                printf("Scheduler PID sending to pid %ld failed\n",msg.mtype);
        	printf("process %d added\n",ready_queue[rend-1].pid);
        	set_enqueue(ready_queue[rend-1].pid);
        }
}