#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

//Where the information of each flight thread will be stored.
struct flight_datum{
        int id;	//unique id for each flight. If the id is equal to -1, then the plane is no longer being monitored.
        int state; //holds the state of every flight.
        int runwaysFree; //if 1 then this plane is currently not taking up any runway. If 0 it is occupying a runway.
};


void* takeoff(void*);
void* landing(void*);
void collision(int);

//Declared on a global scope so the contents can be edited in one function and remain that way in main();
int FLIGHT_COUNT;
int RUNWAYS_FREE;
struct flight_datum flight_data[100];

int main(int argc, char **argv){

	RUNWAYS_FREE = atoi(argv[1]);
	FLIGHT_COUNT = atoi(argv[2]);

	int flightCount = FLIGHT_COUNT;
	int takeoffCount;
	int landingCount;

	// A random int that can either be 0 or 1 is generated so that it can be passed as an argument for the sleep() 
	// function meaning that whether the sleep() happens or not is randomized. This is because if a 0 is passed in, then
	// the sleep function won't do anything, whereas it works normaly when a 1 is passed in.
	srand(time(NULL));
	int random = rand()%2;
//	printf("\nRANDOM IS EQUAL TO = %d\n", random);	// Used to test random.

	if(flightCount%2==0){
		takeoffCount = flightCount/2;
		landingCount = flightCount/2;	
	}
	else{
		takeoffCount = (flightCount/2)+1;
		landingCount = flightCount/2;
	}
	int rc;
	
	printf("\n-----Starting the AFLIC created by Jeffrey H. Diaz-----\n");

	pthread_t flights[flightCount];

	int i;
	for(i=0;i<flightCount;i++){
		
		flight_data[i].id = i;
		flight_data[i].runwaysFree = 1;
		
		//As long as there are flights requesting takeoff, they will attempt to do so.
		if(flightCount-landingCount>0){
			flight_data[i].state = 0;
			landingCount++;
			rc = pthread_create(&flights[i], NULL, takeoff, (void *) &flight_data[i]);
			sleep(random); //Randomized sleep so that either it is 0 and nothing happens or it is 1 and works.
			if(rc){
				printf("ERROR: return code from pthread_create() is %d.\n", rc);
				exit(-1);
			}
		}
		//Once there are no longer flights requesting to takeoff, we will let flights attempt to land.
		else{
			flight_data[i].state = 10;
			rc = pthread_create(&flights[i], NULL, landing, (void *) &flight_data[i]);
			sleep(random); //Makes sure that any data changed is changed prior to being used again.
			if(rc){
                        	printf("ERROR: return code from pthread_create() is %d.\n", rc);
				exit(-1);
                        }
		}

	}
	pthread_exit(NULL);

	return 0;
}

void* takeoff(void* x){

        int id, state, runwaysFree;
        struct flight_datum *datum;

        datum = (struct flight_datum *) x;

	id = (*datum).id;
	state = (*datum).state;
	runwaysFree = (*datum).runwaysFree;

	// A random int that can either be 0 or 1 is generated so that it can be passed as an argument for the sleep() 
        // function meaning that whether the sleep() happens or not is randomized. This is because if a 0 is passed in, then
        // the sleep function won't do anything, whereas it works normaly when a 1 is passed in.
        srand(time(NULL));
        int random = rand()%2;
//	printf("\nRANDOM IS EQUAL TO = %d\n", random);

	switch(state){
		case 0:
			printf("Flight %d registered.\n", id);
			(*datum).state = 1;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
			
		case 1:
			printf("Flight %d at gate.\n", id);
			(*datum).state = 2;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 2:
			printf("Flight %d cleared for take-off.\n", id);
			(*datum).state = 3;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 3:
			printf("Flight %d approaching runway.\n", id);
			(*datum).state = 4;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 4:
			printf("Flight %d requesting runway.\n", id);
			if(runwaysFree>0){
				printf("Runway request was approved for flight %d.\n", id);
				(*datum).state = 5;
			}
			else{
				printf("Runway request was denied for flight %d . All runways are busy.\n", id);
			}
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 5:
			printf("Flight %d cleared for runway.\n", id);
			(*datum).state = 6;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		// If a flight somehow gets to this point and attempts to land on a runway even though none are available,
		// this indicates that a collision has occured because there are more flights on runways then there are
		// runways. Meaning that at least 1 runway has two flights on it.
		// If a collision occures then the collision function will be called and the flight's data will all be set to		     // -1 indicating that it is no longer being monitored by the AFLIC as a result of the collision.
		case 6:
			printf("Flight %d is at runway.\n", id);
			if((*datum).runwaysFree==1 && RUNWAYS_FREE>0){
				(*datum).runwaysFree--;
				RUNWAYS_FREE--;
			}
			else if(RUNWAYS_FREE==0){
                                collision((*datum).id);
				(*datum).id = -1;
                                (*datum).state = -1;
                                (*datum).runwaysFree = -1;
				break;
                        }
			(*datum).state = 7;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 7:
			printf("Flight %d is taking off.\n", id);
			if((*datum).runwaysFree==1 && RUNWAYS_FREE>0){
                                RUNWAYS_FREE--;
                                (*datum).runwaysFree--;
                        }
			else if(RUNWAYS_FREE==0){
                                collision((*datum).id);
				(*datum).id = -1;
                                (*datum).state = -1;
                                (*datum).runwaysFree = -1;
                                break;
                        }
                        printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
			(*datum).state = 8;
			sleep(random);

		case 8:
			printf("Flight %d is at cruising altitude.\n", id);
			if((*datum).runwaysFree==0){
                                RUNWAYS_FREE++;
                                (*datum).runwaysFree++;
                        }
                        printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
			(*datum).state = 9;
	
		case 9:
			printf("Flight %d is out of range.\n", id);
			(*datum).state = 10;
			(*datum).id = -1;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
		break;

		default:
			printf("Incorrect state.\n");
		break;
	}
}

void* landing(void* x){
	
	int id, state, runwaysFree;
	struct flight_datum *datum;

	datum = (struct flight_datum *) x;

	id = (*datum).id;
	state = (*datum).state;
	runwaysFree = (*datum).runwaysFree;

	// A random int that can either be 0 or 1 is generated so that it can be passed as an argument for the sleep() 
        // function meaning that whether the sleep() happens or not is randomized. This is because if a 0 is passed in, then
        // the sleep function won't do anything, whereas it works normaly when a 1 is passed in.
        srand(time(NULL));
        int random = rand()%2;
//	printf("\nRANDOM IS EQUAL TO = %d\n", random);

	switch(state){
		case 10:
			printf("Flight %d detected.\n", id);
			(*datum).state = 11;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
			sleep(random);

		case 11:
			printf("Flight %d is requesting landing.\n", id);
			if(RUNWAYS_FREE>0){
				printf("Flight %d's landing request was approved.\n", id);
				(*datum).state = 13;
			}
			else{
				printf("Flight %d's landing request was denied. No runways are free.\n", id);
				(*datum).state = 12;
			}
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
		
		case 12:
			if(state==12){
				printf("Flight %d is in holding.\n", id);
				(*datum).state = 11;
				printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
			}

		case 13:
			printf("Flight %d is cleared for landing.\n", id);
			(*datum).state = 14;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 14:
			printf("Flight %d is currently landing.\n", id);
			if((*datum).runwaysFree==1 && RUNWAYS_FREE>0){
				RUNWAYS_FREE--;
				(*datum).runwaysFree--;
			}
			else if(RUNWAYS_FREE==0){
                                collision((*datum).id);
				(*datum).id = -1;
                                (*datum).state = -1;
                                (*datum).runwaysFree = -1;
                                break;
                        }
			(*datum).state = 15;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 15:
			printf("Flight %d is at the runway.\n", id);
			if((*datum).runwaysFree==1 && RUNWAYS_FREE>0){
                                RUNWAYS_FREE--;
                                (*datum).runwaysFree--;
            		}
			else if(RUNWAYS_FREE==0){
                                collision((*datum).id);
				(*datum).id = -1;
                                (*datum).state = -1;
                                (*datum).runwaysFree = -1;
                                break;
                        }
			(*datum).state = 16;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 16:
			printf("Flight %d is moving towards the gate.\n", id);
			if((*datum).runwaysFree==1 && RUNWAYS_FREE>0){
                                RUNWAYS_FREE--;
                                (*datum).runwaysFree--;
                        }
			else if(RUNWAYS_FREE==0){
                                collision((*datum).id);
				(*datum).id = -1;
                                (*datum).state = -1;
                                (*datum).runwaysFree = -1;
                                break;
                        }
			(*datum).state = 17;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 17:
			printf("Flight %d is at the gate.\n", id);
			if((*datum).runwaysFree==1 && RUNWAYS_FREE>0){
                                RUNWAYS_FREE--;
                                (*datum).runwaysFree--;
                        }
			else if(RUNWAYS_FREE==0){
                                collision((*datum).id);
				(*datum).id = -1;
                                (*datum).state = -1;
                                (*datum).runwaysFree = -1;
                                break;
                        }
			(*datum).state = 18;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 18:
			printf("Flight %d is cleared to enter the gate.\n", id);
			if((*datum).runwaysFree==1 && RUNWAYS_FREE>0){
                                RUNWAYS_FREE--;
                                (*datum).runwaysFree--;
                        }
			else if(RUNWAYS_FREE==0){
                                collision((*datum).id);
				(*datum).id = -1;
				(*datum).state = -1;
				(*datum).runwaysFree = -1;
                                break;
                        }
			(*datum).state = 19;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);

		case 19:
			printf("Flight %d has entered the gate and is no longer on the runway.\n", id);
			if((*datum).runwaysFree==0){
                                RUNWAYS_FREE++;
                                (*datum).runwaysFree++;
                        }
			(*datum).state = 0;
			(*datum).id = -1;
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
		break;

		default:
			printf("Incorrect state by flight %d.\n", id);
			printf("RUNWAYS_FREE = %d\n", RUNWAYS_FREE);
		break;
	}

}

void collision(int x){
	printf("\n---A collision involving flight %d has occured---\n\n", x);
}
