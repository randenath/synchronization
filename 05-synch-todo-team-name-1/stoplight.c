/*
 * Authors: Samantha Foley & Rohan Hari
 *
 * CS 441/541 : Synchronization Project
 *
 */
#include "stoplight.h"

int main(int argc, char * argv[]) {
    int ret;
    pthread_t car_threads[100];
    int i;
    
    /*
     * Parse Command Line arguments
     */
    if( 0 != (ret = parse_args(argc, argv)) ) {
        return -1;
    }
    
    /*
     * Initialize:
     * - random number generator
     * - semaphores for quadrants
     * - queues for each direction
     */
    srand(time(NULL));

    semaphore_create(&quadrant_nw, 1);
    semaphore_create(&quadrant_ne, 1);
    semaphore_create(&quadrant_sw, 1);
    semaphore_create(&quadrant_se, 1);

    semaphore_create(&stats_lock, 1);

    for (i = 0; i < 4; i++) {
	    approach_queue_t *q;
	    
	    if (i == 0) {
		    q = &north_queue;
	    } else if (i == 1) {
		    q = &south_queue;
	    } else if (i == 2) {
		    q = &east_queue;
	    } else {
		    q = &west_queue;
	    }
	    
	    q -> front = 0;
	    q -> rear = 0;
	    semaphore_create(&q -> lock, 1);

	    for (int j = 0; j < 100; j++) {
		    semaphore_create(&q -> turn_sem[j], 0);
	    }
    }

    for (i = 0; i < num_cars; i++) {
	car_stats[i].min_time = 1e9;
        car_stats[i].max_time = 0;
        car_stats[i].total_time = 0;
        car_stats[i].num_trips = 0;
    }
    
    /*
     * Create Car Threads
     *
     */

    for (i = 0; i < num_cars; i++) {
	    ret = pthread_create(&car_threads[i], NULL, start_car, (void*)(intptr_t)i);
	    if (ret != 0) {
		    fprintf(stderr, "Error creating thread for car %d\n", i);
		    return -1;
	    }
    }


    /*
     * Wait for the TTL to expire
     */
    // sleep specified amount of time
    // set time_to_exit to TRUE

    sleep(ttl);
    time_to_exit = TRUE;


    /*
     * Reap threads
     */

    for (i = 0; i < num_cars; i++) {
	    pthread_join(car_threads[i], NULL);
    }


    /*
     * Print timing information
     */

    double global_min = 1e9;
    double global_max = 0;
    double global_total = 0;
    int global_trips = 0;

 
    for (int i = 0; i < num_cars; i++) {
            if (car_stats[i].num_trips > 0) {

                    if (car_stats[i].min_time < global_min) {
                            global_min = car_stats[i].min_time;
                    }

                    if (car_stats[i].max_time > global_max) {
                            global_max = car_stats[i].max_time;
                    }

                    global_total += car_stats[i].total_time;
                    global_trips += car_stats[i].num_trips;
            }
    }

    printf("\nMin Time : %.3f msec\n", global_min);
    printf("Avg Time : %.3f msec\n", global_total / global_trips);
    printf("Max Time : %.3f msec\n", global_max);
    printf("Total Time : %.3f msec\n", global_total);
    print_footer();

    /*
     * Cleanup
     *
     */

    semaphore_destroy(&quadrant_nw);
    semaphore_destroy(&quadrant_ne);
    semaphore_destroy(&quadrant_sw);
    semaphore_destroy(&quadrant_se);

    semaphore_destroy(&stats_lock);


    for (i = 0; i < 4; i++) {
	    approach_queue_t *q;
	    if (i == 0) {
		    q = &north_queue;
	    } else if (i == 1) {
		    q = &south_queue;
	    } else if (i == 2) {
		    q = &east_queue;
	    } else {
		    q = &west_queue;
	    }

	    semaphore_destroy(&q -> lock);
	    for (int j = 0; j < 100; j++) {
		    semaphore_destroy(&q -> turn_sem[j]);
	    }
    }

    /*
     * Finalize support library
     */
    support_finalize();

    return 0;
}

int parse_args(int argc, char **argv) {


	if (argc != 3) {
		fprintf(stderr, "Wrong usage!!!\n%s <time to run (seconds)> <number of cars>", argv[0]);
		return(-1);
	}

	ttl = atoi(argv[1]);
	num_cars = atoi(argv[2]);


	//checking to see if the correct number of arguments were presented.
	if (ttl <= 0) {
		fprintf(stderr, "Error: time_to_run_seconds must mbe positive.");
		return -1;
	}

	if (num_cars <= 0 || num_cars > 100) {
		fprintf(stderr, "Error: num_cars must be between 1 and 100.");
		return -1;
	}

	//printing what the arguments entered were.
	printf("Time to live: %d\n", ttl);
	printf("Number of cars: %d\n", num_cars);
	printf("\n");



    /*
     * Initialize support library
     */
    support_init();
    print_header();

    return 0;
}



/*
 *Helper functions that I added
 */

approach_queue_t* get_queue(car_direction_t dir) {
	if (dir == NORTH1) {
   		return &north_queue;

    	} else if (dir == SOUTH1) {
		return &south_queue;
    	
	} else if (dir == EAST) {
        	return &east_queue;
    	
	} else if (dir == WEST) {
        	return &west_queue;
    	
	} else {
        	return NULL;
    	}
}


void enter_queue(approach_queue_t *q, int car_id) {
	semaphore_wait(&q -> lock);
	q -> queue[q -> rear] = car_id;
	q -> rear++;
	semaphore_post(&q -> lock);
}

void wait_for_turn(approach_queue_t *q, int car_id) {
	semaphore_wait(&q -> lock);
	int is_front = (q -> queue[q -> front] == car_id);
	semaphore_post(&q -> lock);
	
	if (!is_front) {
		semaphore_wait(&q -> turn_sem[car_id]);
	}
}

void leave_queue(approach_queue_t *q) {
	semaphore_wait(&q -> lock);
	q -> front++;

	if (q -> front < q -> rear) {
		int next_car = q -> queue[q -> front];
		semaphore_post(&q -> turn_sem[next_car]);
	}
	semaphore_post(&q -> lock);
}

semaphore_t* get_quadrant_sem(int quadrant) {
    	if (quadrant == 0) {
        	return &quadrant_nw;

    	} else if (quadrant == 1) {
        	return &quadrant_ne;

    	} else if (quadrant == 2) {
        	return &quadrant_sw;

    	} else if (quadrant == 3) {
        	return &quadrant_se;

    	} else {
        	return NULL;
    	}
}


int get_path_quadrants (car_direction_t from, car_direction_t to, int path[]) {
	if (from == NORTH1) {
		if (to == EAST) {
			path[0] = 0;
			return 1;
		} else if (to == SOUTH1) {
			path[0] = 0;
			path[1] = 2;
			return 2;
		} else { 
			path[0] = 0;
			path[1] = 2;
			path[2] = 3;
			return 3;
		}
	} else if (from == SOUTH1) {
                if (to == EAST) {
                        path[0] = 3;
                        return 1;
                } else if (to == SOUTH1) {
                        path[0] = 3;
                        path[1] = 1;
                        return 2;
                } else {
                        path[0] = 3;
                        path[1] = 1;
                        path[2] = 0;
                        return 3;
                }
        } else if (from == EAST) {
                if (to == EAST) {
                        path[0] = 1;
                        return 1;
                } else if (to == SOUTH1) {
                        path[0] = 1;
                        path[1] = 0;
                        return 2;
                } else {
                        path[0] = 1;
                        path[1] = 0;
                        path[2] = 2;
                        return 3;
                }
        } else if (from == WEST) {
                if (to == EAST) {
                        path[0] = 2;
                        return 1;
                } else if (to == SOUTH1) {
                        path[0] = 2;
                        path[1] = 3;
                        return 2;
                } else {
                        path[0] = 2;
                        path[1] = 3;
                        path[2] = 1;
                        return 3;
                }
        }
	return 0;
}


void update_state_for_turn(car_t *car) {
	if (car -> appr_dir == NORTH1) {
		if (car -> dest_dir == EAST) {
			car -> state = STATE_GO_RIGHT_I1;
		} else if (car -> dest_dir == SOUTH1) {
			car -> state = STATE_GO_STRAIGHT_I1;
		} else {
			car -> state = STATE_GO_LEFT_I1;
		}
	} else if (car -> appr_dir == SOUTH1) {
                if (car -> dest_dir == WEST) {
                        car -> state = STATE_GO_RIGHT_I1;
                } else if (car -> dest_dir == NORTH1) {
                        car -> state = STATE_GO_STRAIGHT_I1;
                } else {
                        car -> state = STATE_GO_LEFT_I1;
                }
        } else if (car -> appr_dir == EAST) {
                if (car -> dest_dir == SOUTH1) {
                        car -> state = STATE_GO_RIGHT_I1;
                } else if (car -> dest_dir == WEST) {
                        car -> state = STATE_GO_STRAIGHT_I1;
                } else {
                        car -> state = STATE_GO_LEFT_I1;
                }
        } else if (car -> appr_dir == WEST) {
                if (car -> dest_dir == NORTH1) {
                        car -> state = STATE_GO_RIGHT_I1;
                } else if (car -> dest_dir == EAST) {
                        car -> state = STATE_GO_STRAIGHT_I1;
                } else {
                        car -> state = STATE_GO_LEFT_I1;
                }
        }
}


void store_stats(int car_id, double trip_time_seconds) {
	double trip_time_ms = trip_time_seconds * 1000.0;

	semaphore_wait(&stats_lock);

	car_stats[car_id].num_trips++;
	car_stats[car_id].total_time += trip_time_ms;
	if (trip_time_ms < car_stats[car_id].min_time) {
		car_stats[car_id].min_time = trip_time_ms;
	}

	if (trip_time_ms > car_stats[car_id].max_time) {
		car_stats[car_id].max_time = trip_time_ms;
	}

	semaphore_post(&stats_lock);
}












/*
 * Approach intersection
 * param = Car Number (car_id)
 */
void *start_car(void *param) {
    int car_id = (intptr_t)param;
    car_t this_car;
    this_car.car_id = car_id;

    /*
     * Keep cycling through
     */
    while( time_to_exit == FALSE ) {

        /*
         * Sleep for a bounded random amount of time before approaching the
         * intersection
         */
        usleep(random()%TIME_TO_APPROACH);


        /*
         * Setup the car's direction, where it is headed, set its state
         */
	this_car.appr_dir = random() % DIRMAX;
	do {
		this_car.dest_dir = random() % DIRMAX;
	} while (this_car.dest_dir == this_car.appr_dir);

	this_car.location = LOC_I1;
	this_car.state = STATE_WAITING_I1;

        /*
         * Mark start time for car
         */
        gettimeofday(&this_car.start_time, NULL);
        print_state(this_car, NULL);

        /*
         * Entering the queue for the approach direction.
         */
	approach_queue_t *my_queue = get_queue(this_car.appr_dir);
	enter_queue(my_queue, car_id);

	/*
	 *Wait until we are next in line.
	 */
	this_car.state = STATE_APPROACH_I1;
	print_state(this_car, NULL);
	wait_for_turn(my_queue, car_id);

	/*
	 * determine which quadrants we need.
	 */
	int path[3];
	int num_quadrants = get_path_quadrants(this_car.appr_dir, this_car.dest_dir, path);

	/*
	 *Now we get all the quadrants that we need in order.
	 */
	for (int i = 0; i < num_quadrants - 1; i++) {
		for (int j = 0; j < num_quadrants - i - 1; j++) {
			if (path[j] > path[j + 1]) {
				int temp = path[j];
				path[j] = path[j + 1];
				path[j + 1] = temp;
			}
		}
	}

	for (int i = 0; i < num_quadrants; i++) {
		semaphore_wait(get_quadrant_sem(path[i]));
	}

	/*
	 * Now we enter the intersection
	 */
	update_state_for_turn(&this_car);
	print_state(this_car, NULL);

	/*
	 * Sleep for the crossing time
	 */
	usleep(num_quadrants * TIME_TO_CROSS);

	/*
	 * Releaseing the quadrants so others can use
	 */
	for (int i = num_quadrants - 1; i >= 0; i--) {
		semaphore_post(get_quadrant_sem(path[i]));
	}

	/*
	 *Leaving the intersection
	 */
        gettimeofday(&this_car.end_time, NULL);
	this_car.state = STATE_LEAVE_I1;
        print_state(this_car, NULL);

        /*
         * Save statistics about the cars travel
         */
	double trip_time = get_timeval_diff_as_double(this_car.start_time, &this_car.end_time);
	store_stats(car_id, trip_time);

	/*
	 * Signal next car in the queue
	 */
	leave_queue(my_queue);

    }

    /*
     * All done
     */
    pthread_exit((void *) 0);

    return NULL;
}
