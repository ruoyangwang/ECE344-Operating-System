/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
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
//#include <queue.h>

struct semaphore *sem0;
struct semaphore *sem1;
struct semaphore *sem2;
struct semaphore *sem3;
struct semaphore *global;
//struct semaphore *check;

int indicator;
int totalcars;
//int anyone_turn_right;
//struct queue *q;
/*
 *
 * Constants
 *
 */

/*
 * Number of cars created.
 */

#define NCARS 20


/*
 *
 * Function Definitions
 *
 */
static void message(int msg_nr, int carnumber, int cardirection, int destdirection);
static void gostraight(unsigned long cardirection, unsigned long carnumber);
static void turnleft(unsigned long cardirection, unsigned long carnumber);
static void turnright(unsigned long cardirection, unsigned long carnumber);
static void approachintersection(void * unusedpointer, unsigned long carnumber);
int createcars(int nargs, char ** args);


static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}
 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
/*
for the functions making turns, firstly check if there are maximum 3 cars
at the entire intersection. If yes, then the car have to wait until there
are less than 3 cars. If no, skip the checking step.
*/

// Assembly code here to make a loop
checkcars1:
	if(totalcars>=3)
	goto checkcars1;

	P(global); 
	// when a car can go into the intersection, the total car num ++
	totalcars++; 
	if(totalcars>=3)
	{	
		//if the car enter the intersection exceeding the limit
		// (= 3), go straight.
		indicator=0;
		goto carrunning;
	}
	else
	{
		// the total car number is less than the limit when the car
		// enter the intersection, "unlock" the region.
		V(global);
		indicator =1;
		goto carrunning;
	}
	
	
	// check and lock the relative region1 and 2 of the current car
	carrunning:
		if(cardirection == 0){
			message(APPROACHING, carnumber, cardirection, 2);
			P(sem0);

			message(REGION1, carnumber, cardirection, 2);


			P(sem3);

			message(REGION2, carnumber, cardirection, 2);
			V(sem0);
			message(LEAVING, carnumber, cardirection, 2);
			V(sem3);
			
			
		}
		else if(cardirection == 1){
			message(APPROACHING, carnumber, cardirection, 3);
			P(sem1);

			message(REGION1, carnumber, cardirection, 3);
			
			
			
			P(sem0);

			message(REGION2, carnumber, cardirection, 3);
			V(sem1);
			message(LEAVING, carnumber, cardirection, 3);
			V(sem0);

		}
		else if(cardirection == 2){
			message(APPROACHING, carnumber, cardirection, 0);
			P(sem2);

			message(REGION1, carnumber, cardirection, 0);
			
			
			
			P(sem1);

			message(REGION2, carnumber, cardirection, 0);
			V(sem2);
			message(LEAVING, carnumber, cardirection, 0);
			V(sem1);	
		}
		else if(cardirection == 3){
			message(APPROACHING, carnumber, cardirection, 1);
			P(sem3);

			message(REGION1, carnumber, cardirection, 1);
			
			
			
			P(sem2);

			message(REGION2, carnumber, cardirection, 1);
			V(sem3);
			message(LEAVING, carnumber, cardirection, 1);
			V(sem2);
			
		}
	
	
	totalcars--;
	// if the critical region for the totalcar number is still "locked",
	// unlock it.
	if(indicator==0)
	V(global);
	
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
/*
Same functionality as described above.
*/
// Assembly code here to make a loop.
checkcars2:
	if(totalcars>=3)
	goto checkcars2;
	
	P(global);
	totalcars++;
	if(totalcars>=3)
	{
		indicator=0;
		goto carrunning2;
	}
	else
	{
		V(global);
		indicator =1;
		goto carrunning2;
	}
        // check and lock the relative region1, 2 and 3 of the current car
carrunning2:
        if(cardirection == 0){
        	message(APPROACHING, carnumber, cardirection, 1);
        	P(sem0);
        	message(REGION1, carnumber, cardirection, 1);
        	
        	
        	
        	P(sem3);
        	
        	message(REGION2, carnumber, cardirection, 1);
        	V(sem0);
        	
        	
        	P(sem2);
        	
        	message(REGION3, carnumber, cardirection, 1);
        	V(sem3);
        	message(LEAVING, carnumber, cardirection, 1);
        	V(sem2);
        	
        	
	}
	else if(cardirection == 1){
		message(APPROACHING, carnumber, cardirection, 2);
		P(sem1);

        	message(REGION1, carnumber, cardirection, 2);

        	P(sem0);
        	
        	message(REGION2, carnumber, cardirection, 2);
        	V(sem1);
        	
        	
        	P(sem3);
        	
        	
        	message(REGION3, carnumber, cardirection, 2);
        	V(sem0);
        	message(LEAVING, carnumber, cardirection, 2);
        	V(sem3);
        	
        	
        	
	}
	else if(cardirection == 2){
		message(APPROACHING, carnumber, cardirection, 3);
		P(sem2);
        	message(REGION1, carnumber, cardirection, 3);
        	
        	
        	
        	P(sem1);
        	
        	message(REGION2, carnumber, cardirection, 3);
        	V(sem2);
        	
        	
        	
        	P(sem0);
        	
        	message(REGION3, carnumber, cardirection, 3);
        	V(sem1);
        	message(LEAVING, carnumber, cardirection, 3);
        	V(sem0);
        	
        	
	}
	else if(cardirection == 3){
		message(APPROACHING, carnumber, cardirection, 0);
		P(sem3);

        	message(REGION1, carnumber, cardirection, 0);
        	
        	
        	
        	P(sem2);
        	
        	message(REGION2, carnumber, cardirection, 0);
        	V(sem3);
        	
        	
        	P(sem1);
        	
        	message(REGION3, carnumber, cardirection, 0);
        	V(sem2);
        	message(LEAVING, carnumber, cardirection, 0);
        	V(sem1);
        	
        	
	}
	totalcars--;
	if(indicator==0)
	V(global);
	
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
/*
Same functionality as described above.
*/
// Assembly code here to make a loop.
checkcars3:

	if(totalcars>=4)
	goto checkcars3;
	
	P(global);
	totalcars++;
	if(totalcars>=4)
	{
		indicator=0;
		goto carrunning3;
	}
	else
	{
		V(global);
		indicator =1;
		goto carrunning3;
	}
        // check and lock the relative region1 of the current car

carrunning3:
        if(cardirection == 0){
        	message(APPROACHING, carnumber, cardirection, 3);
        	P(sem0);

        	message(REGION1, carnumber, cardirection, 3);

        	message(LEAVING, carnumber, cardirection, 3);

        	V(sem0);
        	
        	
	}
	else if(cardirection == 1){
		message(APPROACHING, carnumber, cardirection, 0);
		P(sem1);
		
        	message(REGION1, carnumber, cardirection, 0);

        	message(LEAVING, carnumber, cardirection, 0);

        	V(sem1);
        	
        	
	}
	else if(cardirection == 2){
		message(APPROACHING, carnumber, cardirection, 1);
		P(sem2);
		

        	message(REGION1, carnumber, cardirection, 1);

        	message(LEAVING, carnumber, cardirection, 1);

        	V(sem2);
        	
        	
	}
	else if(cardirection == 3){
		message(APPROACHING, carnumber, cardirection, 2);
		P(sem3);
		
        	message(REGION1, carnumber, cardirection, 2);

        	message(LEAVING, carnumber, cardirection, 2);

        	V(sem3);
        	
        	
	}
	totalcars--;
	
	
	if(indicator==0)
	V(global);
	
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection;
	int destdirection;
        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;

        /*
         * cardirection and destdirection are set randomly.
         */

        cardirection = random() % 4;
	destdirection = random() % 4;// generating random dest direction
	
	// if the dest direction is same as coming direction, find another one
	while(destdirection == cardirection)
		destdirection = random() % 4;
	
	
	// now check where to go
	if(((cardirection - destdirection) == 1 )|| ((cardirection - destdirection) == -3 ))
		turnright(cardirection, carnumber);
	else if((cardirection - destdirection == 2) || (cardirection - destdirection == -2))
		gostraight(cardirection, carnumber);
	else if(((cardirection - destdirection) == 3)||((cardirection - destdirection) == -1))
		turnleft(cardirection, carnumber);


}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;
	totalcars=0;
	indicator=0;
	//anyone_turn_right = 0;
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
	sem0 = sem_create("NW",1);
	sem1 = sem_create("NE",1);
	sem2 = sem_create("SE",1);
	sem3 = sem_create("SW",1);
	global = sem_create("global",1);
	//check = sem_create("check",1);
	
        /*
         * Start NCARS approachintersection() threads.
         */

        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );

                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }
        }
        return 0;
}
