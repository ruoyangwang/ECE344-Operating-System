#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_getpid();
int sys_fork(struct trapframe *tf, int * i);
void sys__exit(int code);
int execv(const char *program, char **args);
#endif /* _SYSCALL_H_ */
