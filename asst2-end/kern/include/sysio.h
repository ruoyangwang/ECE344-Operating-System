#ifndef _SYSIO_H_
#define _SYSIO_H_
#include <lib.h>
#include <types.h>
int sys_write(int filehandle, const char *buf, size_t size)
{
	/*if(filehandle == NULL)
		return EBADF;*/
	int i =0;
	for (i =0 ; i < size; i++){
		kprintf("%c",buf[i]);
		}
	return i;
}

int sys_read(int filehandle, void *buf, size_t size, int *retval)
{
	/*if(filehandle == NULL)
		return EBADF;*///it's a integer how the fuck can you compare with NULL?
	char i	= getch();	
	/*if(i==13)
	i= 10;	*/
	 
	copyout(&i,(userptr_t)buf, sizeof(int ));
	
	*retval = sizeof(char);
	return 0;
}










#endif
