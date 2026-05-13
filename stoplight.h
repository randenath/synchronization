/*
 * Samantha Foley & Rohan Hari
 *
 * CS 441/541 : Synchronization Project
 */
#include "support.h"
#include "semaphore_support.h"

/*****************************
 * Defines
 *****************************/


/*****************************
 * Structures
 *****************************/
typedef struct {
	int queue[100];
	int front;
	int rear;
	semaphore_t lock;
	semaphore_t turn_sem[100];
} approach_queue_t;

approach_queue_t north_queue;
approach_queue_t south_queue;
approach_queue_t east_queue;
approach_queue_t west_queue;


typedef struct {
	double min_time;
	double max_time;
	double total_time;
	int num_trips;
} car_stats_t;

car_stats_t car_stats[100];

/*****************************
 * Global Variables
 *****************************/
semaphore_t quadrant_nw;
semaphore_t quadrant_ne;
semaphore_t quadrant_sw;
semaphore_t quadrant_se;
semaphore_t stats_lock;

/*
 * Time to live (Seconds)
 */
int ttl = 0;

/*
 * Number of cars (threads) in the system
 */
int num_cars = 0;

/*
 * Indicate when for threads to stop processing and exit
 */
int time_to_exit = FALSE;


/*****************************
 * Function Declarations
 *****************************/
/*
 * Parse command line arguments
 */
int parse_args(int argc, char **argv);

/*
 * Main thread function that picks an arbitrary direction to approach from,
 * and to travel to for each car.
 *
 * Write and comment this function
 *
 * Arguments:
 *   param = The car ID number for printing purposes
 *
 * Returns:
 *   NULL
 */
void *start_car(void *param);



// Get the queue for a given approach direction
approach_queue_t* get_queue(car_direction_t dir);

// Add a car to the queue
void enter_queue(approach_queue_t *q, int car_id);

// Wait until this car is at the front
void wait_for_turn(approach_queue_t *q, int car_id);

// Remove front car and signal next
void leave_queue(approach_queue_t *q);

// Get the semaphore for a quadrant
semaphore_t* get_quadrant_sem(int quadrant);

// Get the quadrants a car needs for its path
int get_path_quadrants(car_direction_t from, car_direction_t to, int path[]);

// Update car state based on turn
void update_state_for_turn(car_t *car);

// Store statistics for a car
void store_stats(int car_id, double trip_time);
