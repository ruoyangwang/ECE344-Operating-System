#ifndef	_COREMAP_ENTRY_H_
#define _COREMAP_ENTRY_H_



struct coremap_entry
{
	int 	pid;
	vaddr_t viraddr;
	paddr_t phyaddr;
	int taken;
	unsigned long npage;
};

#endif
