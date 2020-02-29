#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include "elevator_19.h"

/****************************************************************************************************/

// #define TEST_DATA

Person g_persons[TOTAL_PERSONS];
Elevator g_elevator;
int g_random_count = 0;

pthread_mutex_t g_person_list_lock = PTHREAD_MUTEX_INITIALIZER;

#ifdef TEST_DATA
Selected g_test_data[TOTAL_PERSONS] = {
	{5, 2},
	{5, 2},
	{3, 1},
	{0, 2},
	{3, 1},
	{5, 2},
	{2, 0},
	{0, 1},
	{5, 4},
	{5, 2}
	/*
	{3, 5},
	{4, 1},
	{2, 0},
	{1, 0},
	{2, 1},
	{1, 4},
	{4, 1},
	{3, 5},
	{1, 3},
	{1, 3}
*/
	/*
	{4, 3},
	{2, 0},
	{2, 0},
	{4, 3},
	{3, 5},
	{5, 3},
	{0, 1},
	{1, 5},
	{1, 2},
	{5, 4}
*/
	/*
	{1, 2},
	{4, 1},
	{5, 0},
	{0, 4},
	{1, 4},
	{1, 4},
	{4, 5},
	{0, 3},
	{2, 1},
	{2, 4}
*/
	/*
	{4, 0},
	{5, 1},
	{0, 2},
	{3, 5},
	{5, 3},
	{3, 0},
	{4, 3},
	{5, 4},
	{5, 2},
	{0, 5}
*/
	/*
	{1, 0},
	{4, 3},
	{1, 5},
	{4, 5},
	{2, 3},
	{4, 0},
	{2, 4},
	{4, 1},
	{1, 4},
	{4, 2}
*/
};
#endif

int serving_person_count()
{
	int i, count = 0;

	for (i = 0; i < TOTAL_PERSONS; i++)
	{
		if (g_persons[i].state == PERSON_BEING_SERVED)
			count++;
	}

	return count;
}

int waiting_person_count()
{
	int i, count = 0;

	for (i = 0; i < TOTAL_PERSONS; i++)
	{
		if (g_persons[i].state == PERSON_WAITING)
			count++;
	}

	return count;
}

int served_person_count()
{
	int i, count = 0;

	for (i = 0; i < TOTAL_PERSONS; i++)
	{
		if (g_persons[i].state == PERSON_SERVED)
			count++;
	}

	return count;
}

void get_time_stamp(char *timestr)
{
	time_t currtime;
	currtime = time(NULL);
	strftime(timestr, 10, "%H:%M:%S", localtime(&currtime));
}

int max_random(int max)
{
	int randNum;

	g_random_count++;

	randNum = rand() + g_random_count;
	return (int)(randNum % max);
}

Selected rand_select_two_numbers(int n)
{
	int i, *r;
	Selected sel;
	r = (int *)malloc(n * sizeof(int));
	for (i = 0; i < n; i++)
		r[i] = i;
	sel.start = max_random(n);
	r[sel.start] = r[--n];
	do
	{
		sel.destination = r[max_random(n)];
	} while (sel.start == sel.destination);
	free(r);

	return sel;
}

void init_person_list()
{
	int i;

	for (i = 0; i < TOTAL_PERSONS; i++)
	{
		g_persons[i].id = i;
		g_persons[i].state = PERSON_NO_EXIST;
		g_persons[i].start_floor = -1;
		g_persons[i].dest_floor = -1;
		g_persons[i].direction = STOP;
	}
}

void init_elevator()
{
	g_elevator.floor = 0;
	g_elevator.direction = STOP;
}

void *person(void *arg)
{
	char time[15];
	Selected sel;

	long int *pid = (long int *)arg;
	int id = (int)*pid;
	free(pid);

	if (id < 0 || id >= TOTAL_PERSONS)
		return 0;

	pthread_mutex_lock(&g_person_list_lock);
	g_persons[id].state = PERSON_WAITING;
#ifndef TEST_DATA
	sel = rand_select_two_numbers(FLOORS);
#else
	sel = g_test_data[id];
#endif
	g_persons[id].start_floor = sel.start;
	g_persons[id].dest_floor = sel.destination;
	g_persons[id].direction = (g_persons[id].dest_floor > g_persons[id].start_floor) ? UP : DOWN;
	pthread_mutex_unlock(&g_person_list_lock);

	get_time_stamp(time);
	fprintf(stderr, "Person %d wants to go from %d to %d  at %s\n", id, g_persons[id].start_floor, g_persons[id].dest_floor, time);

	while (g_persons[id].state != PERSON_SERVED)
	{
		usleep(MILLI);
	}

	return 0;
}

bool can_serve_person(Person *person)
{
	if (person->state != PERSON_WAITING || person->start_floor != g_elevator.floor ||
		serving_person_count() + 1 > CAPACITY)
		return false;

	if (g_elevator.floor != 0 && g_elevator.floor != FLOORS - 1 &&
		g_elevator.direction != STOP && g_elevator.direction != person->direction)
		return false;

	return true;
}

bool can_put_person(Person *person)
{
	if (person->state != PERSON_BEING_SERVED || person->dest_floor != g_elevator.floor)
		return false;

	return true;
}

void get_person_in_elevator(Person *person)
{
	char time[15];

	pthread_mutex_lock(&g_person_list_lock);
	person->state = PERSON_BEING_SERVED;
	pthread_mutex_unlock(&g_person_list_lock);

	get_time_stamp(time);
	fprintf(stderr, "Person %d going from %d to %d enters the elevator at %s\n", person->id, person->start_floor, person->dest_floor, time);
}

void put_person_from_elevator(Person *person)
{
	char time[15];

	pthread_mutex_lock(&g_person_list_lock);
	person->state = PERSON_SERVED;
	pthread_mutex_unlock(&g_person_list_lock);

	get_time_stamp(time);
	fprintf(stderr, "Person %d going from %d to %d leaves the elevator at %s\n", person->id, person->start_floor, person->dest_floor, time);
}

void put_and_get_people()
{
	int i;
	Person *person;
	bool get_person;

	for (i = 0; i < TOTAL_PERSONS; i++)
	{
		person = &g_persons[i];
		if (can_put_person(person))
		{
			put_person_from_elevator(person);
		}
	}

	do
	{
		get_person = false;
		for (i = 0; i < TOTAL_PERSONS; i++)
		{
			person = &g_persons[i];
			if (can_serve_person(person))
			{
				get_person_in_elevator(person);
				get_person = true;
				break;
			}
		}
	} while (get_person);
}

bool is_there_waiting_person_in_current_direction()
{
	int i;
	Person *person;

	for (i = 0; i < TOTAL_PERSONS; i++)
	{
		person = &g_persons[i];
		if (person->state == PERSON_WAITING &&
			((g_elevator.direction == UP && person->start_floor > g_elevator.floor) || (g_elevator.direction == DOWN && person->start_floor < g_elevator.floor)))
		{
			return true;
		}
	}

	return false;
}

void change_elevator_direction()
{
	int i;
	Person *person;

	for (i = 0; i < TOTAL_PERSONS; i++)
	{
		person = &g_persons[i];
		if (person->state == PERSON_BEING_SERVED)
		{
			g_elevator.direction = person->dest_floor > g_elevator.floor ? UP : DOWN;
			return;
		}
	}

	if (is_there_waiting_person_in_current_direction())
		return;

	for (i = 0; i < TOTAL_PERSONS; i++)
	{
		person = &g_persons[i];
		if (person->state == PERSON_WAITING)
		{
			if (person->direction)
			{
				if (person->start_floor > g_elevator.floor)
				{
					g_elevator.direction = UP;
					return;
				}
				else if (person->start_floor < g_elevator.floor)
				{
					g_elevator.direction = DOWN;
					return;
				}
			}
		}
	}

	g_elevator.direction = STOP;
}

void *elevator(void *arg)
{

	int prev_floor;
	char time[15];

	while (served_person_count() < TOTAL_PERSONS)
	{
		put_and_get_people();

		change_elevator_direction();

		prev_floor = g_elevator.floor;

		g_elevator.floor += g_elevator.direction;
		if (g_elevator.floor < 0)
		{
			g_elevator.floor = 0;
			g_elevator.direction = STOP;
		}
		if (g_elevator.floor >= FLOORS)
		{
			g_elevator.floor = FLOORS - 1;
			g_elevator.direction = STOP;
		}

		if (served_person_count() >= TOTAL_PERSONS)
			break;

		if (move_elevator(prev_floor, g_elevator.floor))
		{
			get_time_stamp(time);
			fprintf(stderr, "Elevator moved from %d floor to %d floor at %s\n", prev_floor, g_elevator.floor, time);
		}
		else
		{
			usleep(MILLI);
		}
	}

	update_screen();

	return 0;
}

int run_elevator()
{

	long int id;
	pthread_t elevator_th, *person_th;
	setbuf(stdout, NULL);

	pthread_mutex_init(&g_person_list_lock, NULL);

	// real function of the elevator
	pthread_create(&elevator_th, NULL, elevator, (void *)NULL);

	person_th = (pthread_t *)malloc(TOTAL_PERSONS * sizeof(pthread_t));
	for (id = 0; id < TOTAL_PERSONS; id++)
	{
		// real function of the single person
		long int *pid = (long int *)malloc(sizeof(int));
		*pid = id;
		pthread_create(&person_th[id], NULL, person, (void *)pid);
		sleep(max_random(T));
	}

	// waiting for all threads - elevator's and people's
	for (id = 0; id < TOTAL_PERSONS; id++)
		pthread_join(person_th[id], NULL);
	pthread_join(elevator_th, NULL);

	free(person_th);
	return 0;
}
