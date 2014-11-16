#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <synch.h>
#include <sysio.h>
//#include </../lib/copyinout.c>
//#include <sys_execv.h>
/*
 * System call handler.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception.S) is passed in.
 *
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. In addition, the system call number is
 * passed in the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, like an ordinary function call, and the a3 register is
 * also set to 0 to indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/lib/libc/syscalls.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * Since none of the OS/161 system calls have more than 4 arguments,
 * there should be no need to fetch additional arguments from the
 * user-level stack.
 *
 * Watch out: if you make system calls that have 64-bit quantities as
 * arguments, they will get passed in pairs of registers, and not
 * necessarily in the way you expect. We recommend you don't do it.
 * (In fact, we recommend you don't use 64-bit quantities at all. See
 * arch/mips/include/types.h.)
 */

//struct lock*lock;
int sys_getpid()
{
	
/**/	int pid = (curthread->pid);
	return pid;
}



int
sys_waitpid(int a0,int *a1, int a2, int *returnval)
{
	int pid = a0;
	if(a2 != 0)	// if the option is not valid.
		return EINVAL;
	if(a1 == NULL)	// if the status is not a valid pointer.
		return EFAULT;
	
	struct thread *temp;
	lock_acquire(curthread->lk);
	int i =0;
	for(i=0;i<20;i++){
		lock_acquire(curthread->child[i]->lk);
		temp = curthread->child[i];					//acquire child lock to do operation
		if( curthread->child[i]->pid == pid)
		{
			if(curthread->child[i]->status == 0)			//child is still running
			{
				if(curthread->child[i]->waiting_parent == 1){	//child has been waiting
					cv_signal(curthread->child[i]->Waittoexit, curthread->child[i]->lk);
					*(int *)(a1) = curthread->child[i]->exitcode;
					cv_wait(curthread->child[i]->Waitonexit, curthread->child[i]->lk);//wait child to signal you, indicate that he finished
				}
				else{	// child is not waiting. just wait for child now.
					curthread->waiting_child = 1;
					
					cv_wait(curthread->child[i]->Waitonexit, curthread->child[i]->lk);//wait child to signal you, indicate that he finished
					*(int *)(a1) = curthread->child[i]->exitcode;
					cv_signal(curthread->child[i]->Waittoexit,curthread->child[i]->lk);//signal child to indicate that get the exitcode
				}
			}
		    	curthread->child[i] = NULL;
		    	
		    	break;	
		}
        	lock_release(curthread->child[i]->lk);
	}
	//kprintf("in here!!!");
	lock_release(curthread->lk);
	//lock_release(temp->lk);		
	*returnval = pid;
	return 0;
	
}



void
sys__exit(int exitcode) {
	
	lock_acquire(curthread->lk);
	//now check children
	int i =0;
	curthread->waiting_child = 2;
	for(i = 0; i < 20; i++)		//check for child
	{
        	if (curthread->child[i] == NULL) // if no child in this element
        		continue;
        		
		lock_acquire(curthread->child[i]->lk);			//PROBLEMS ARE HERE!!!
		//kprintf("Exittingchild pid is %d,self %d, parent %d\n", curthread->child[i]->pid,curthread->pid, curthread->parent->pid);
		if( curthread->child[i]->status == 1){
			// child exited already
			;
			//cv_signal(curthread->child[i]->Waittoexit, curthread->child[i]->lk);
		}
		else{	// child status == 0, child didnt exit
			curthread->child[i]->parent=NULL;
		}
		lock_release(curthread->child[i]->lk);
	
	}
	
	//NOW check parent
	if(curthread->parent != NULL)
	{//kprintf("Exitting pid is %d,Child pid is %d\n", curthread-> pid,curthread->parent->pid);	//PARENT STILL RUNNING
		curthread->exitcode = exitcode;
		curthread->status = 1;
		
		if(curthread->parent->waiting_child == 0){//if parent is not waiting, or parent didnt exit yet
			curthread->waiting_parent = 1;
			cv_wait(curthread->Waittoexit, curthread->lk);
			cv_signal(curthread->Waitonexit, curthread->lk);
			assert(lock_do_i_hold(curthread->lk) == 1);	
			//cv_wait(curthread->Waittoexit, curthread->lk);	
		}
		else if(curthread->parent->waiting_child == 1){// if parent is waiting, or parent didnt exit yet
			cv_signal(curthread->Waitonexit, curthread->lk);
			assert(lock_do_i_hold(curthread->lk) == 1);
			cv_wait(curthread->Waittoexit, curthread->lk);		//child will be locked by parent in waitpid()
		}
	}
	lock_release(curthread->lk);	
	thread_exit();	
}

int sys_fork(struct trapframe *tf, int * returnval) //fork a new thread
{
	
	int err;
	/*if(numthreads>=20)
		return EAGAIN ;			//limit the thread number to 20*/
		
	struct thread * childthread ;			//create a child thread
	struct trapframe *copy = kmalloc(sizeof(struct trapframe)) ;	//create trapframe for later to move to the new one
	struct addrspace *newspace ;	//allocate new address space for the process
	
	
	if (copy == NULL) {		//the heap is full?
		err = ENOMEM;
		return err;
	}
	
	memcpy(copy,tf,sizeof(struct trapframe)); //let the new trapframe pointer points on the old tf ,this put the old tf on heap, deep copy
	//
/**/	int i = as_copy(curthread->t_vmspace,&newspace);
	
	if(i!=0)			//if no memory
		goto fail2;
		//kprintf("returned here?\n");
	as_activate(curthread->t_vmspace);
	
	//kprintf("now start to create child\n");
/**/	*returnval = thread_fork(curthread->t_name, copy, (unsigned long)newspace, md_forkentry, &childthread);

	if(*returnval!=0){
		//kprintf("get in here?\n");
		err = ENOMEM;
		tf->tf_a3 = 1;		//indicates a fail operation
		tf->tf_v0 =err;
		goto fail2;
	}
	
	(*returnval) = childthread->pid;
    for (i = 0; i < 20; i ++) {
        if (curthread->child[i] == NULL) {
            curthread->child[i] = childthread;
            break;
        }
    }
    assert(i<20);
	return 0;
	
fail2:	kfree(copy);
fail1:	return err;
}

void
md_forkentry(struct trapframe *tf, unsigned long data)			//this function is to actually store into child pro
{
	/*
	 * This function is provided as a reminder. You need to write
	 * both it and the code that calls it.
	 *
	 * Thus, you can trash it and do things another way if you prefer.
	 */
	struct trapframe *local ,temp;
	struct addrspace *childspace;
	
	childspace = (struct addrspace*) data;     //the same space as parent by casting
	//memcpy(local,tf,sizeof(struct trapframe));
	temp = *tf;
	local = &temp;
	
	int i =	as_copy(childspace, &curthread->t_vmspace);
	//as_destroy(childspace);
	if(i!=0)
		return;

	as_activate(curthread->t_vmspace);
	kfree(tf);
	local->tf_epc += 4;			//increment PC
	local->tf_v0 = 0;
	local->tf_a3 = 0; 			//succeed , no failure 

	assert(curthread->t_vmspace!=NULL);
	mips_usermode(local);			//wrap to user space
	
	
	(void)tf;
}


void
mips_syscall(struct trapframe *tf)
{
	int callno;
	int32_t retval;
	int err;

	
	assert(curspl==0);

	callno = tf->tf_v0;
	int a2;
	
	
	//lock =lock_create("lock");
	/*
	 * Initialize retval to 0. Many of the system calls don't
	 * really return a value, just 0 for success and -1 on
	 * error. Since retval is the value returned on success,
	 * initialize it to 0 by default; thus it's not necessary to
	 * deal with it except for calls that return other values, 
	 * like write.
	 */

	retval = 0;

	switch (callno) {
	    case SYS_reboot:
		err = sys_reboot(tf->tf_a0);
		break;

	    /* Add stuff here */
	    //    NEED TO ADD SYS_getpid, SYS__exit, SYS_fork, SYS_waitpid, SYS_execv
 	    case SYS__exit:
 	    	sys__exit(tf->tf_a0);
 	    	break;
 	    	
 	 /*  case SYS_execv:
 	    	err = sys_execv(name, arg[]);
 	    	break;*/
 	    	
 	    case SYS_fork:
 	    	err = sys_fork(tf,&retval);
 	    	break;
 	    	
 	   case SYS_waitpid:
 	    	err = sys_waitpid(tf->tf_a0, tf->tf_a1,tf->tf_a2,&retval);
 	    	break;
 	    	
 	    case SYS_getpid:
 	    	err = sys_getpid();
 	    	break;
 	    
 	    case SYS_write:
 	    	err = sys_write(tf->tf_a0,tf->tf_a1,tf->tf_a2);
 	    	break;
 	    
 	    case SYS_read:
 	    	err = sys_read(tf->tf_a0,tf->tf_a1,tf->tf_a2,&retval);
 	   
 	    	break;
 	    	
	    default:
		kprintf("Unknown syscall %d\n", callno);
		err = ENOSYS;
		break;
	}


	if (err) {
		/*
		 * Return the error code. This gets converted at
		 * userlevel to a return value of -1 and the error
		 * code in errno.
		 */
		tf->tf_v0 = err;
		tf->tf_a3 = 1;      /* signal an error */
	}
	else {
		/* Success. */
		tf->tf_v0 = retval;
		tf->tf_a3 = 0;      /* signal no error */
	}
	
	/*
	 * Now, advance the program counter, to avoid restarting
	 * the syscall over and over again.
	 */
	
	tf->tf_epc += 4;

	/* Make sure the syscall code didn't forget to lower spl */
	assert(curspl==0);
}


