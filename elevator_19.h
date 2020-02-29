#ifndef HEADER_FILE
#define HEADER_FILE

// Global Constants Definitions
#define FLOORS (8)

#define CAPACITY (7)
#define T (5)
#define TOTAL_PERSONS (20)
#define STRING_LENGTH (32)

#define MILLI (1000000) // 1 second

//struct sem_t;

// support structure for randomly choosing
// start and destination floor
typedef struct
{
	int start, destination;
} Selected;

enum Person_State
{
	PERSON_NO_EXIST = -1,
	PERSON_WAITING = 0,
	PERSON_BEING_SERVED = 1,
	PERSON_SERVED = 2
};

enum Direction
{
	DOWN = -1,
	STOP = 0,
	UP = 1
};

typedef struct
{
	int id;
	enum Person_State state;
	int start_floor;
	int dest_floor;
	enum Direction direction;
} Person;

typedef struct
{
	int floor;
	enum Direction direction;
} Elevator;

// function prototypes
void init_person_list();
void init_elevator();
int run_elevator();
bool move_elevator(int start_floor, int end_floor);
void update_screen();

// variables
extern Person g_persons[TOTAL_PERSONS];
extern Elevator g_elevator;

#endif
