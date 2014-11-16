#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <elf.h>
#include <syscall.h>
#include <coremap.h>
#include <synch.h>
#include <vnode.h>
//#include <math.h>
/* under dumbvm, always have 48k of user stack */
#define DUMBVM_STACKPAGES    12
paddr_t toppage_ptr;
static int vm_indicator = 0;

static unsigned int pagenumber;
static unsigned int pageleft;
struct coremap_entry * coremap;
//struct lock *mem_lock;
static int bootstrap = 0; 

void
vm_bootstrap(void)			//this function is to initialize coremap
{
	//mem_lock = lock_create("mem_lock");
	unsigned int i;
	//mem_lock = lock_create("mem_lock");
	struct coremap_entry * pages;
	paddr_t  lowest, highest;
	ram_getsize(&lowest, &highest);
	pagenumber = (unsigned int) ((highest-lowest) / PAGE_SIZE)-1 ;
	//pages = (struct coremap_entry*)PADDR_TO_KVADDR(lowest);		//don't use kmalloc, cast to kernel virtual address space
	
	pages = (struct coremap_entry*)PADDR_TO_KVADDR(lowest);
	//pages = (struct coremap_entry*)PADDR_TO_KVADDR(lowest);
	coremap = (struct coremap_entry*)PADDR_TO_KVADDR(lowest);
	 						//coremap points to the last entry
	toppage_ptr = lowest + (pagenumber * sizeof(struct coremap_entry));		//point to the top of the address space
	toppage_ptr = toppage_ptr + (paddr_t)(sizeof(struct coremap_entry));			//the first available physical address
	toppage_ptr &= PAGE_FRAME;			//round up to the whole chunk, avoid cutting in.
	toppage_ptr += 0x1000;
	
	unsigned int coremap_size = sizeof(struct coremap_entry);
	     /* Init the coremap */
        //pages = (struct coremap_entry *) PADDR_TO_KVADDR(lowest);		//kernel virtual address space
	for(i = 0; i < pagenumber; i++)
	{
		paddr_t temp;
		temp = toppage_ptr + (paddr_t)(4096 *i);
		pages->taken = 0;		//initially not taken
		pages->viraddr =  0;
		//kprintf("coremap:0x%x, temp:0x%x ",coremap-0x80000000,temp);
		//assert((unsigned int)temp>((unsigned int)coremap-(unsigned int)0x80000000));
		(paddr_t)(pages->phyaddr) = temp;	//!!!!!!!!!!!!!!!!!!!!!!!!! PROBLEM HERE, phyaddr is virtual address!!!
		//pages += coremap_size;	//get to next entry block
		pages = (struct coremap_entry*)((unsigned int)pages + (unsigned int)coremap_size);
		
	}
	
	pageleft = pagenumber;
	vm_indicator = 1;							//already have vm bootstrapped, this is an indicator
	bootstrap =1;
}

static
paddr_t
getppages(unsigned long npages)
{
	
	paddr_t phyaddr;
	unsigned long count = 0;
	int found = 0;
	int interrupt = splhigh();
	unsigned int coremap_size = sizeof(struct coremap_entry);
	if(vm_indicator == 0)				//first initialization before vm_bootstrap
	{
		phyaddr = ram_stealmem(npages);
	}
	else if(vm_indicator !=0)			//need to find unused phyaddr in coremap
	{
		if(npages>pageleft)			//if exceed the amount of total pages that can be used
			return 0;			
						
		else{
			struct coremap_entry * ptr, *temp;
			ptr = coremap;				//get the first coremap entry, then use ptr to increment 
			temp = coremap;				//get the first coremap entry for later to get chunk of address
			unsigned long i, j;
			for(i=0;i<pagenumber;i++)
			{
				if(ptr->taken == 0)		//can use this one
				{
					count++;
					if(count == npages)	//get enough pages
					{
						for (j=0; j<count; j++) 	//loop back to the first entry that find count == 0;
						{
							
							if(j>0)
		                                		ptr= (struct coremap_entry*)((unsigned int)ptr  - (unsigned int)coremap_size);
		                                	ptr->taken =1;
		                                	
		                       		}
		                      		
		                        	ptr->pid = curthread->pid;
						ptr->npage = count;
						                                                                                             
						phyaddr = ptr->phyaddr;//=========================return the coremap_entry pointer, which is the physical adress
						found =1;
						pageleft -= count;
						
						break;
					}
				}
				else
					count = 0;		//avoid fragmentation
					
				ptr = (struct coremap_entry*)((unsigned int)ptr + (unsigned int)coremap_size);		//increment to next address, point one more entry up
				
				
			}
		
			 if(found==0 )
			 {
			 	splx(interrupt);
				return 0;
			}
		}
		assert(phyaddr>(((unsigned int)coremap-(unsigned int)0x80000000)));
	}
	/*if(pageleft ==0)
	{
		splx(interrupt);
		return 0;
	}*/
	splx(interrupt);
	
//kprintf("phyaddr:0x%x, coremap:0x%x, coremap:0x%x\n",phyaddr,((unsigned int)coremap-(unsigned int)0x80000000),coremap);
	
	return phyaddr;
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t 
alloc_kpages(int npages)
{
   	paddr_t paddr;
   	vaddr_t vaddr;
        paddr = getppages(npages);
        if (paddr==0) 
                return 0;

        else
        {
        	vaddr = PADDR_TO_KVADDR(paddr);
        	return vaddr;
        }
}

void 
free_kpages(vaddr_t addr)
{
	/* nothing */
	struct coremap_entry *ptr;
	unsigned int coremap_size = sizeof(struct coremap_entry);
	unsigned long i,npages;
	//paddr_t phyaddr = KVADDR_TO_PADDR(addr);
	assert(addr >= MIPS_KSEG0);
	int s = splhigh();
		
	ptr = coremap;			//start to search from the bottom
	
	
	while((PADDR_TO_KVADDR(ptr->phyaddr)!= addr))	//loop until find the according coremap entry
		ptr= (struct coremap_entry*)((unsigned int)ptr  + (unsigned int)coremap_size);
		
	npages = ptr-> npage;	//out of the loop, means found the coremap entry that has available address,and this npage indicate how many pages this process has taken
	 
	for(i=0;i<npages;i++)
	{	
		ptr->taken = 0;					//start from that entry, make the whole chunk of adress available
		ptr= (struct coremap_entry*)((unsigned int)ptr  + (unsigned int)coremap_size); 	//now do operation on the next one until reach required npages		
		pageleft++;
		
		
	}
	
	splx(s);
}

int freepagetable(struct pagetable1  ptr/*[512]*/)
{
	unsigned int i,j;
	vaddr_t vaddr;
	for(i =0;i<512;i++)
	{
		
	  	if(ptr.pagetable2[i]!=NULL)
	  	{	
			for (j=0; j < 1024; j++)
		  	{	
		  		if(ptr.pagetable2[i]->pageentry[j].valid !=0)
		  		{
			  		free_kpages(PADDR_TO_KVADDR(ptr.pagetable2[i]->pageentry[j].phyaddr));	//free second level page
			  		//ptr.pagetable2[i]->pageentry[j]=NULL;
		  		}		
		  	}
			kfree(ptr.pagetable2[i]);				//now free first level pages
			ptr.pagetable2[i]=NULL;
	       	}
	        
	        
		
        }    	
        return 1;
}


int
vm_fault(int faulttype, vaddr_t faultaddress)
{
//kprintf("in vmfault Type: %d, faultaddress: 0x%x pid:%d\n",faulttype,faultaddress,curthread->pid);
if(faultaddress > (vaddr_t)0x80000000)
return EFAULT;
if(faultaddress == 0)
return EFAULT;
if((faultaddress & (vaddr_t)0x3)!=0)
return EFAULT;


	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
		paddr_t paddr;
		int i,j;
		u_int32_t ehi, elo;
		struct addrspace *as;
		int spl;

		spl = splhigh();

		faultaddress &= PAGE_FRAME;

		DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

		switch (faulttype) {
		    case VM_FAULT_READONLY:
			/* We always create pages read-write, so we can't get this */
			panic("dumbvm: got VM_FAULT_READONLY\n");
		    case VM_FAULT_READ:
		    case VM_FAULT_WRITE:
			break;
		    default:
			splx(spl);
			return EINVAL;
		}

		as = curthread->t_vmspace;
		if (as == NULL) {
			/*
			 * No address space set up. This is probably a kernel
			 * fault early in boot. Return EFAULT so as to panic
			 * instead of getting into an infinite faulting loop.
			 */
			return EFAULT;
		}

		/* Assert that the address space has been set up properly. */
		/*assert(as->as_vbase1 != 0);
		assert(as->as_pbase1 != 0);
		assert(as->as_npages1 != 0);
		assert(as->as_vbase2 != 0);
		assert(as->as_pbase2 != 0);
		assert(as->as_npages2 != 0);
		assert(as->as_stackpbase != 0);
		assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
		assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
		assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
		assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
		assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);

		vbase1 = as->as_vbase1;
		vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
		vbase2 = as->as_vbase2;
		vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
		stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
		stacktop = USERSTACK;*/
/*
		if (faultaddress >= vbase1 && faultaddress < vtop1) {
			paddr = (faultaddress - vbase1) + as->as_pbase1;
		}
		else if (faultaddress >= vbase2 && faultaddress < vtop2) {
			paddr = (faultaddress - vbase2) + as->as_pbase2;
		}
		else if (faultaddress >= stackbase && faultaddress < stacktop) {
			paddr = (faultaddress - stackbase) + as->as_stackpbase;
		}
		else {
			splx(spl);
			return EFAULT;
		}
*/		
		
		unsigned int index1 = (faultaddress & 0xffc00000) >>22;
		unsigned int index2 = (faultaddress & 0x003ff000) >>12;
//kprintf("index1: %d, index2: %d\n",index1,index2);
		if(curthread->t_vmspace->pagetable.pagetable2[index1] ==NULL)
		{
			curthread->t_vmspace->pagetable.pagetable2[index1]= kmalloc(sizeof(struct pagetable2));
		
			bzero(curthread->t_vmspace->pagetable.pagetable2[index1],sizeof(struct pagetable2));
			
		}
				
		struct pageentry pageentry = curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2];
		if(pageentry.valid==0)
		{
			curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].phyaddr = alloc_kpages(1);
			if(curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].phyaddr == ENOMEM){
				return ENOMEM;
			}
			
			curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].phyaddr=(paddr_t)((unsigned int)curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].phyaddr-(unsigned int)0x80000000);
//kprintf("paddr:%x coremap:%x\n",curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].phyaddr,coremap);
			assert((((unsigned int)coremap-(unsigned int)0x80000000))<curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].phyaddr);
			curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].valid =1;
			bzero((void*)PADDR_TO_KVADDR(curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].phyaddr),PAGE_SIZE);
		}	
		
		//searching for physichal address
		/* make sure it's page-aligned */
		
		paddr = curthread->t_vmspace->pagetable.pagetable2[index1]->pageentry[index2].phyaddr;
		assert((paddr & PAGE_FRAME)==paddr);
		for (i=0; i<NUM_TLB; i++) {
			TLB_Read(&ehi, &elo, i);
			if (elo & TLBLO_VALID) {
				continue;
			}
			ehi = faultaddress;
			elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
			DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
			TLB_Write(ehi, elo, i);
			splx(spl);
			return 0;
		}

		kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
		splx(spl);
		return EFAULT;
}

struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		return NULL;
	}

	as->as_vbase1 = 0;
	//as->as_pbase1 = 0;
	//as->as_npages1 = 0;
	as->as_vbase2 = 0;
	//as->as_pbase2 = 0;
	//as->as_npages2 = 0;
	as->as_stackpbase = 0;
	int i;
	
	//struct pageptr * pageptr = kmalloc(pagenum);		//initialize first level and second level pageptr
	//(as->pageptr) = kmalloc(512);

	for(i=0;i<512;i++)
	{
		as-> pagetable.pagetable2[i] = NULL;
		//as->pagetable += sizeof(struct pageptr);
	}
	//bzero((void*)as->pageptable,512);			//set to zero for as_define_region
	return as;
}

void
as_destroy(struct addrspace *as)
{
	freepagetable(as->pagetable);
	kfree(as);
}

void
as_activate(struct addrspace *as)
{
	int i, spl;

	(void)as;

	spl = splhigh();

	for (i=0; i<NUM_TLB; i++) {
		TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}
	
	splx(spl);
}

int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable)
{
	size_t npages; 
	/*size_t i;
	unsigned int i1,i2;
	vaddr_t * vptr1, *vptr2;*/
	/* Align the region. First, the base... */
	//sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	//vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	//sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

	//npages = sz / PAGE_SIZE;

	/* We don't use these - all pages are read-write */
	/*for(i = 0; i<npages; i++){
		i1 = (vaddr & 0xffc00000) >>22;
		vptr1 = (vaddr *)(as->pageptable.pagetable[i]);
		
		        if (vptr1 == NULL) {
		                vptr1 = alloc_kpages(1);
		                if (vptr1 == NULL)		//fail in allocating pages, no more pages
		                        return EFAULT;
		                        
		                bzero(*vaddr,1);
		                
		        }
	
	}*/
        
	if (as->as_vbase1 == 0) {
		as->as_vbase1 = vaddr;
	//	as->as_npages1 = npages;
		return 0;
	}

	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
	//	as->as_npages2 = npages;
		return 0;
	}

	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;
}

int
as_prepare_load(struct addrspace *as)
{
	//assert(as->as_pbase1 == 0);
	//assert(as->as_pbase2 == 0);
	assert(as->as_stackpbase == 0);

	/*as->as_pbase1 = getppages(as->as_npages1);
	if (as->as_pbase1 == 0) {
		return ENOMEM;
	}

	as->as_pbase2 = getppages(as->as_npages2);
	if (as->as_pbase2 == 0) {
		return ENOMEM;
	}

	as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
	if (as->as_stackpbase == 0) {
		return ENOMEM;
	}*/

	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	assert(as!= NULL);

	*stackptr = USERSTACK;
	return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *new;

	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}

	new->as_vbase1 = old->as_vbase1;
	//new->as_npages1 = old->as_npages1;
	new->as_vbase2 = old->as_vbase2;
	//new->as_npages2 = old->as_npages2;
	new->as_stackpbase = old->as_stackpbase;
	if (as_prepare_load(new)) {
		as_destroy(new);
		return ENOMEM;
	}

	//assert(new->as_pbase1 != 0);
	//assert(new->as_pbase2 != 0);
	//assert(new->as_stackpbase != 0);

	/*memmove((void *)PADDR_TO_KVADDR(new->as_pbase1),
		(const void *)PADDR_TO_KVADDR(old->as_pbase1),
		old->as_npages1*PAGE_SIZE);

	memmove((void *)PADDR_TO_KVADDR(new->as_pbase2),
		(const void *)PADDR_TO_KVADDR(old->as_pbase2),
		old->as_npages2*PAGE_SIZE);

	memmove((void *)PADDR_TO_KVADDR(new->as_stackpbase),
		(const void *)PADDR_TO_KVADDR(old->as_stackpbase),
		DUMBVM_STACKPAGES*PAGE_SIZE);*/
	int i,j;
	for(i=0;i<512;i++)
	{
		if(old->pagetable.pagetable2[i]!=NULL)
		{
			new->pagetable.pagetable2[i]= kmalloc(sizeof(struct pagetable2));
			bzero(new->pagetable.pagetable2[i],sizeof(struct pagetable2));
			for(j=0;j<1024;j++)
			{
				if(old->pagetable.pagetable2[i]->pageentry[j].valid ==1)
				{
					new->pagetable.pagetable2[i]->pageentry[j].phyaddr = alloc_kpages(1);
					
					
			
					if(new->pagetable.pagetable2[i]->pageentry[j].phyaddr == ENOMEM){
						return ENOMEM;
					}
			
					new->pagetable.pagetable2[i]->pageentry[j].phyaddr=(paddr_t)((unsigned int)new->pagetable.pagetable2[i]->pageentry[j].phyaddr-(unsigned int)0x80000000);					
					
					
					
					assert((((unsigned int)coremap-(unsigned int)0x80000000))<new->pagetable.pagetable2[i]->pageentry[j].phyaddr);
					assert((((unsigned int)coremap-(unsigned int)0x80000000))<old->pagetable.pagetable2[i]->pageentry[j].phyaddr);
					new->pagetable.pagetable2[i]->pageentry[j].valid = 1;
					memmove((void *)PADDR_TO_KVADDR(new->pagetable.pagetable2[i]->pageentry[j].phyaddr),
					(const void *)PADDR_TO_KVADDR(old->pagetable.pagetable2[i]->pageentry[j].phyaddr),
					PAGE_SIZE);	
				}
			}
		
		}
			

		
	}
	
	
	*ret = new;
	return 0;
}
