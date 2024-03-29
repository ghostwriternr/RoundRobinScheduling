#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
	int n=4;
    char NOI[2][20]={"10000","4000"};
    char Priority[2][20]={"10","5"};
    char SleepProb[2][20]={"30","70"};
    char SleepTime[2][20]={"1","3"};
    char exe[100]={"./process.out "};
    char command[100];
    int i=0;
    pid_t pid;
    // printf("Enter total number of processes to be spawned :\n");
    // scanf("%d",&n);
    // if(n<1)
    // 	return 1;
    // printf("\tFor CPU Bound Processes\n");
    // printf("Enter NOI, Priority, Sleep Time, Sleep Probabilty(on base 100) :\n");
    // scanf("%s%s%s%s",NOI[0],Priority[0],SleepTime[0],SleepProb[0]);
    // if(n>1)
    // {
    // 	printf("\tFor I/O Bound Processes\n");
    //     printf("Enter NOI, Priority, Sleep Time, Sleep Probabilty(on base 100) :\n");
    //     scanf("%s%s%s%s",NOI[1],Priority[1],SleepTime[1],SleepProb[1]);
    // }
    /**
		CPU BOUND Processes
	*/
    memset(command,'\0',100);
    strcpy(command,exe);
    strcat(command,NOI[0]);
    strcat(command," ");
    strcat(command,Priority[0]);
    strcat(command," ");
    strcat(command,SleepProb[0]);
    strcat(command," ");
    strcat(command,SleepTime[0]);
    for(i=0;i<(n+1)/2;i++)
    {
    	// printf("\t%d : %s\n",i,command);
        if(fork()==0)
            {
                execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", command, (void*)NULL);
            }
        sleep(1);
    }
    if(n<2)
    	return 0;
	/**
		I/O BOUND Processes
	*/
    strcpy(command,exe);
    strcat(command,NOI[1]);
    strcat(command," ");
    strcat(command,Priority[1]);
    strcat(command," ");
    strcat(command,SleepProb[1]);
    strcat(command," ");
    strcat(command,SleepTime[1]);
    for(;i<n;i++)    
    {
        if(fork()==0)
            {
                execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "bash", "-c", command, (void*)NULL);
            }
        sleep(1);
    }
    return 0;
}
