/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>
//#include <copyinout.h>
/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(void * ptr, unsigned long nargs)
{
	char **args = ptr;		//cast to characters point
	char progname[128];
	strcpy(progname, args[0]);	
	unsigned long temp_p[nargs+1];
	size_t *actual;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);	//ptr[0] is the program name
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vaddr_t abcdefg;
	//int i = sizeof(nargs);
	if(nargs>2){
	kprintf("taking args arg[0]%s arg[1]%s\n", args[0],args[1]);
	
		int j = 0;
		for(j= nargs-1;j>=0;j--)
		{
			int ptrlen = strlen(args[j])+1;
			int offset =4-(ptrlen%4);
			if(ptrlen%4==0)
				offset = 0;
		kprintf("here ptrlen:%d,offset:%d stackptr: 0x%x\n",ptrlen,offset,stackptr);
			stackptr = (vaddr_t)((unsigned int)stackptr - (unsigned int)(ptrlen + offset));
		
				kprintf("here ptrlen:%d,offset:%d stackptr: 0x%x data:%s \n",ptrlen,offset,stackptr,args[j]);
				
			copyoutstr(args[j],(userptr_t)stackptr,(size_t)(ptrlen),actual);		//copy the string from kernel to user level
			if(*(int*)actual != (ptrlen))
				kprintf("ERROR stackptr: 0x%x stackptr+len: 0x%x actual:%d prelen:%d\n ",stackptr,(unsigned int)(stackptr)+(unsigned int)ptrlen+(unsigned int)1,*(int*)actual,ptrlen);
			kprintf("here?\n");
			temp_p[j] = stackptr;kprintf("here?\n");
		
			kprintf("addr stored: 0x%x value: %s\n",stackptr,stackptr);
											//move to another block
		
		}
	
		temp_p[nargs]= (int )NULL;
		for(j = nargs; j>=0; j--)
		{	
			stackptr-=4;
			//copyout(&temp_p[j],stackptr,(size_t)temp_p[j]);				//copy the address from kernel to user level
			vaddr_t t = stackptr;
			*(unsigned long*)t=temp_p[j];
			kprintf("addr stored: 0x%x value: 0x%x\n",t,*(unsigned int*)t);
			//t++;
			//*(unsigned long*)t=NULL;
		}
		abcdefg = stackptr;
	}/*else if(nargs ==2){
		
	
	}*/else{abcdefg = NULL;}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Warp to user mode. */
	
	
	
	md_usermode(nargs /*argc*/, (userptr_t)abcdefg /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}

