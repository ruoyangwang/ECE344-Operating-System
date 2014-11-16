/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

struct lock * lock1 ;
struct lock * lock2 ;
int cat_indicate1, cat_indicate2, mouse_indicate1, mouse_indicate2;

/*
 * 
 * Function Definitions
 * 
 */

/* who should be "cat" or "mouse" */
static void
lock_eat(const char *who, int num, int bowl, int iteration)
{
	if(bowl==1)
	{
		lock1->bowl=0;						//bowl 1 is taken
		kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num, 
		        bowl, iteration);
		
		
		clocksleep(1);
		lock1->bowl=1;						//bowl1 is available 
		kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num, 
		        bowl, iteration);
		
		
	}
	else if(bowl==2)
	{	
     		lock2->bowl=0;						//bowl 2 is taken
		kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num, 
		        bowl, iteration);
		
		
		clocksleep(1);
		lock2->bowl=1;						//bowl 2 is available
		kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num, 
		        bowl, iteration);
		
	}
	
}

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer, 
        unsigned long catnumber)
{
        /*
         * Avoid unused variable warnings.
         */
   
         int icat=0,which_bowl;
         for(icat=0;icat<4;icat++)
         {
         	while(1)					//LET THE CAT WAIT UNTIL BOWL AVAILABLE
         	{
        start_again:
		 	if(((lock1->bowl==1&&lock2->bowl==1)||(lock1->bowl==1&&lock2->bowl==0))&&lock2->who!=2) //check if athe bowl1 is available and the other bowl is not taken by a mouse
		 	{
		 		cat_indicate1=1;
		 		if(mouse_indicate2==1||mouse_indicate1==1)
		 		{	
		 			cat_indicate1=0;
		 			goto start_again;
		 		}
		 			
		 		else
		 		{
			 		lock_acquire(lock1);
			 		lock1->who=1;				//indicate taken by a cat
			 		 lock_eat("cat",catnumber,1,icat);
			 		lock1->who=0;		//indicate no one is taken anymore
			 		cat_indicate1=0;		//indicate that cat at this bowl is leaving
					lock_release(lock1);
					break;
				}
			}
			else if((lock1->bowl==0&&lock2->bowl==1)&&lock1->who!=2)
			{
				cat_indicate2=1;
		 		if(mouse_indicate2==1||mouse_indicate1==1)
		 		{
		 			cat_indicate2=0;
		 			goto start_again;
		 		}
		 		else
		 		{
					lock_acquire(lock2);
					lock2->who=1;
			 		 lock_eat("cat",catnumber,2,icat);
			 		 lock2->who=0;
			 		 cat_indicate2=0;
					lock_release(lock2);
					break;
				}
			}
		}
		/*else if(lock1->bowl==0&&lock2->bowl==1)
		{
			lock_acquire(lock1);
         		 lock_eat("cat",catnumber,1,icat);
			lock_release(lock1);
		}*/
		
	 }	
	
        (void) unusedpointer;
        (void) catnumber;
}


/*int bowlcheck()
{
	if(bowl1==1||bowl2==1)
		return 3;
	else if(bowl1==1&&bowl2==0)
		return 1;
	else if(bowl2==1&&bowl1==0)
		return 2;
	else 
		return 0;
}*/

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
          unsigned long mousenumber)
{
        /*
         * Avoid unused variable warnings.
         */
          int imouse=0;
         for(imouse=0;imouse<4;imouse++)
         {
         	while(1)
         	{
         	start_again2:
		 	if(((lock1->bowl==1&&lock2->bowl==1)||(lock1->bowl==1&&lock2->bowl==0))&&lock2->who!=1) 	//if bowl1 is free then 
		 	{
		 		mouse_indicate1=1;					//indicate that mouse has came in
		 		if(cat_indicate1==1||cat_indicate2==1)			//to sychronize, check if a cat comes in the same time
		 		{
		 			mouse_indicate1=0;
		 			goto start_again2;
		 		}
		 			
		 		else						//if no cats come in, lock the bowl
		 		{
			 		lock_acquire(lock1);
			 		lock1->who=2;
			 		 lock_eat("mouse",mousenumber,1,imouse);
			 		lock1->who=0;
			 		mouse_indicate1=0;
					lock_release(lock1);
					break;
				}
			}
			else if((lock1->bowl==0&&lock2->bowl==1)&&lock1->who!=1)			//if bowl2 is free, then check bowl1,check who is taking bowl 1
			{
				mouse_indicate2=1;
		 		if(cat_indicate1==1||cat_indicate2==1)
		 		{
		 			mouse_indicate2=0;
		 			goto start_again2;
		 		}
		 		else
		 		{
					lock_acquire(lock2);
					lock2->who=2;
			 		 lock_eat("mouse",mousenumber,2,imouse);
			 		lock1->who=0;
			 		mouse_indicate2=0;
					lock_release(lock2);
					break;
				}
			}
		}
		
	 }	
        (void) unusedpointer;
        (void) mousenumber;
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
   	lock1 =	lock_create("lock1");
   	lock2 =	lock_create("lock2");
   
   	lock1->bowl=1;			//both bowl are available at first beginning
   	lock2->bowl=1;
   	
   	lock1->who=0;			//neither cat nor mouse is taking the bowl
   	lock2->who=0;
  	cat_indicate1=0;
  	cat_indicate2=0;
  	mouse_indicate1=0;		//neither cat nor mouse is entering and ready to eat
  	mouse_indicate2=0;
        /*
         * Start NCATS catlock() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catlock thread", 
                                    NULL, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mouselock thread", 
                                    NULL, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        return 0;
}

/*
 * End of catlock.c
 */
