//#include <unistd.h>
//#include <stdio.h>
//void
//main()
//{
//char c;
//printf("input = ");
//readchar();
	//printchar("H");
//printf("xxxxxxx\n");	
//int pid = fork();
//printf("pid is %d\n", pid);
//printf("the input char is\n");	
/*int i=0;
if (pid==0) {
	printf ("i am the child1\n");
	for (i=0; i<10000; i++);
	exit(9);
	printf(" I have successfully exited!\n");
}
*/
//else{
/*
if(pid!=0){
	printf("I am the parent1\n");
	printf("pid%d\n", pid);
	int status;
	waitpid(pid, &status, 0);

    printf("my status1 is %d\n", status);
}
*/
//}
#include <unistd.h>
#include <stdio.h>

/*
 * reboot - shut down system and reboot it.
 * Usage: reboot
 *
 * Just calls reboot() with the RB_REBOOT flag.
 */

int
main()
{
	char p = 'F';
	int ch = 0;
	printf("Hello world!Hello printf!\n");

	//fork_test();

	wait_test();

	//run_pro_test();

	//read_test();

	return 0;
}

void read_test(void){

	int ch;
	printf("input a thing:");
	ch = getchar();
	//ch = sys_readchar();
	printf("the thing is %c \n",ch);
	return;
}

void fork_test(void){

		printf("Begin the fork test!! \n");

		int childpid;

		int j = 0;

		childpid = fork();
		childpid = fork();
		childpid = fork();
		childpid = fork();

		for(j = 0; j < 5; j++){
			printf("This is process %d \n", getpid());
			int p = 0;
			for(p = 0; p<200000;p++);
		}

		return;
}

void wait_test(void){

	int child_pid;

	child_pid = fork();

	 printf("the return child pid is %d \n",child_pid);
     if (child_pid == 0)
     { // child
    	  	  	  	  printf("I am the child. My ID is %d,  I start counting numbers to infinity.\n",getpid());
    	  	  	  	  int i = 0;
    	  	  	  	  for(i = 0 ; i < 9000000; i++)
    	  	  	  		  ;
                      exit(9);
                      printf("This code should not be triggered in test!!\n");
     }
      else
      {
    	  	  // parent, child_pid value is > 0

    	  	  int status;
              // wait on child
              printf("I am the parent. I am waiting for my child pid %d .\n", child_pid);
              waitpid(child_pid, &status, 0);
              printf("I am the parent. I got the exit code for the child is %d .\n", status);
      }
      return;
}

void run_pro_test(void){

    	char  *filename = "/testbin/add";
        char  *args[4];
        pid_t  pid;

        args[0] = "add";
        args[1] = "5";
        args[2] = "12";
        args[3] = NULL;

        pid = fork();
        if (pid == 0){
        	execv(filename, args);
        }
	return;
}
