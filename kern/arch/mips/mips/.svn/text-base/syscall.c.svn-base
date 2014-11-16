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
	lock_acquire(curthread->lk);
/**/	int pid = (curthread->pid);
    lock_release(curthread->lk);
	return pid;
}



int
sys_waitpid(struct trapframe *tf, int *returnval)
{
	int pid = tf->tf_a0;
	if(tf->tf_a2 != 0)	// if the option is not valid.
		return EINVAL;
	if(tf->tf_a1 == NULL)	// if the status is not a valid pointer.
		return EFAULT;
		
	lock_acquire(curthread->lk);
	
	int i =0;
	for(i=0;i<20;i++){
		lock_acquire(curthread->child[i]->lk);					//acquire child lock to do operation
		if( curthread->child[i]->pid == pid)
		{
			if(curthread->child[i]->status == 0)			//child is still running
			{
				cv_wait(curthread->child[i]->Waitonexit, curthread->child[i]->lk);			//wait child to signal you, indicate that he finished
			}
            assert(curthread->child[i]->status == -1);
            tf->tf_a1 = curthread->child[i]->exitcode;		//child is not running
            cv_signal(curthread->child[i]->Waittoexit,curthread->child[i]->lk);	//signal child to indicate that get the exitcode
            lock_release(curthread->child[i]->lk);
            curthread->child[i] = NULL;
            break;	
		}
        lock_release(curthread->child[i]->lk);
	}
	
	lock_release(curthread->lk);
			
	*returnval = pid;
	return 0;
	
}



void
sys__exit(int exitcode) {
	
	lock_acquire(curthread->lk);
	//now check children
	int i =0;
	for(i = 0; i<20; i++)		//check for child
	{
        if (curthread->child[i] == NULL) continue;
		lock_acquire(curthread->child[i]->lk);
		
		if( curthread->child[i]->status != 0){
			// child exited already
			cv_signal(curthread->child[i]->Waittoexit, curthread->child[i]->lk);
		}
		else{
			curthread->child[i]->parent=NULL;
		}
		lock_release(curthread->child[i]->lk);
	
	}
	
	//NOW check parent
	if(curthread->parent != NULL)
	{						//PARENT STILL RUNNING
		curthread->exitcode = exitcode;
		curthread->status = -1;
		cv_signal(curthread->Waitonexit, curthread->lk);
		cv_wait(curthread->Waittoexit, curthread->lk);		//child will be locked by parent in waitpid()
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
		goto fail1;
	}
	
	memcpy(copy,tf,sizeof(struct trapframe)); //let the new trapframe pointer points on the old tf ,this put the old tf on heap, deep copy
	//
/**/	int i = as_copy(curthread->t_vmspace,&newspace);
	
	if(i!=0)			//if no memory
		goto fail2;
		//kprintf("returned here?\n");
	// as_activate(curthread->t_vmspace);
	int spl = splhigh();
	//kprintf("now start to create child\n");
/**/	*returnval = thread_fork(curthread->t_name, 
                        copy, (unsigned long)newspace, md_forkentry, &childthread);

	if(*returnval!=0){
		//kprintf("get in here?\n");
        assert("no memory");
		err = ENOMEM;
		tf->tf_a3 = 1;		//indicates a fail operation
		tf->tf_v0 =err;
        splx(spl);
		goto fail2;
	}
	childthread->parent = curthread;
	// (*returnval) = childthread->pid;
    splx(spl);
    
    // for (i = 0; i < 20; i ++) {
        // if (curthread->child[i] == NULL) {
            // curthread->child[i] = childthread;
            // break;
        // }
    // }
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
	struct trapframe temp;
	//struct addrspace *childspace;
	
	curthread->t_vmspace = (struct addrspace*) data;     //the same space as parent by casting
	//memcpy(local,tf,sizeof(struct trapframe));
    memmove(&temp, tf, sizeof(struct trapframe));
	// temp = *tf;
	// local = &temp;
	
	// int i =	as_copy(childspace, &curthread->t_vmspace);

	// as_destroy(childspace);
	// if(i!=0)
		// return;

	as_activate(curthread->t_vmspace);
	kfree(tf);
    temp.tf_epc += 4;			//increment PC
	temp.tf_v0 = 0;
	temp.tf_a3 = 0; 			//succeed , no failure 

	assert(curthread->t_vmspace!=NULL);
	mips_usermode(&temp);			//wrap to user space
	
	
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
 	    	err = sys_waitpid(tf, &retval);
 	    	break;
 	    	
 	    case SYS_getpid:
 	    	err = sys_getpid();
 	    	break;
 	    
 	    case SYS_write:
 	    	err = sys_write(tf->tf_a0,tf->tf_a1,tf->tf_a2);
 	    	break;
 	    
 	    case SYS_read:
 	    	err = sys_read(tf->tf_a0,tf->tf_a1,tf->tf_a2, &retval);
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


