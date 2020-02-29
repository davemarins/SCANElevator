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
#include <ncurses.h>
#include "elevator_19.h"

// screen
#define PADDING 10

// variables
int screen_width, screen_height;
int outbox_width, outbox_height;
int elevator_row;

// windows
WINDOW *left_window = NULL;
WINDOW *outbox_window = NULL;
WINDOW *right_window = NULL;

WINDOW *elevator_window = NULL;
WINDOW *container_window = NULL;

// function prototypes
void init_screen();
void display_left_window();
void display_outbox_window();
void display_right_window();
void display_elevator_window();
void display_container_window();
void display_person();

pthread_mutex_t screen_lock = PTHREAD_MUTEX_INITIALIZER;

bool move_elevator(int start_floor, int end_floor)
{
    if (start_floor == end_floor)
        return false;
    if (start_floor < 0 || end_floor < 0)
        return false;

    int start_floor2 = FLOORS - start_floor - 1;
    int end_floor2 = FLOORS - end_floor - 1;

    int start_row = start_floor2 * 5;
    int end_row = end_floor2 * 5;
    int interval = start_row > end_row ? -1 : 1;

    //
    int row = start_row;
    for (;;)
    {
        elevator_row = row;
        update_screen();
        usleep(MILLI);

        if (row == end_row)
            break;
        row += interval;
    }

    return true;
}

void update_screen()
{
    pthread_mutex_lock(&screen_lock);

    display_left_window();
    display_outbox_window();
    display_right_window();
    display_elevator_window();

    // display persons
    display_person();

    pthread_mutex_unlock(&screen_lock);
}

int main(int argc, char *argv[])
{
    FILE *f;
    if (argc != 2)
    {
        printf("\n----- Launch program: ./ncurse_elevator <log_file_name> -----\n");
        return -1;
    }
    if ((f = fopen(argv[1], "w")) == NULL)
    {
        printf("\n----- fopen failed, now terminating -----\n");
        return -1;
    }
    dup2(fileno(f), STDERR_FILENO);

    srandom((unsigned int)time(NULL));

    init_person_list();
    init_elevator();

    pthread_mutex_init(&screen_lock, NULL);

    // init screen
    init_screen();

    // init variables
    getmaxyx(stdscr, screen_height, screen_width);
    outbox_width = 25;
    outbox_height = 5 * FLOORS;
    elevator_row = (FLOORS - 1) * 5;

    // display windows
    display_container_window();
    update_screen();

    // test: move elevator
    // move_elevator(0, 1);

    //sleep(8);

    // run elevator
    run_elevator();

    sleep(5);

    // restore the terminal
    endwin();

    return 0;
}

void init_screen()
{
    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_RED, COLOR_WHITE);
    curs_set(0);
    noecho();
    cbreak();
}

void display_left_window()
{
    // calculate
    int x0 = PADDING;
    int y0 = (screen_height - outbox_height) / 2;
    int width = (screen_width - outbox_width) / 2 - PADDING;
    int height = outbox_height;

    // create the window
    if (left_window == NULL)
    {
        left_window = newwin(height, width, y0, x0);
        wbkgd(left_window, COLOR_PAIR(2));
    }

    // erase the window
    werase(left_window);

    // refresh the window
    wrefresh(left_window);
}

void display_outbox_window()
{
    // calculate
    int x0 = (screen_width - outbox_width) / 2;
    int y0 = (screen_height - outbox_height) / 2;
    int width = outbox_width;
    int height = outbox_height;

    // create the window
    if (outbox_window == NULL)
    {
        outbox_window = newwin(height, width, y0, x0);
        wbkgd(outbox_window, COLOR_PAIR(2));
    }

    // erase the window
    werase(outbox_window);

    // draw the border
    wmove(outbox_window, 0, 0);
    wvline(outbox_window, 0, outbox_height);
    wmove(outbox_window, 0, outbox_width - 4);
    wvline(outbox_window, 0, outbox_height);

    // draw floor numbers
    for (int row = 0; row < outbox_height; ++row)
        if ((row - 2) % 5 == 0)
            mvwprintw(outbox_window, row, outbox_width - 2, "%d", (outbox_height - row - 1) / 5);

    // refresh the window
    wrefresh(outbox_window);
}

void display_right_window()
{
    // calculate
    int x0 = (screen_width + outbox_width) / 2;
    int y0 = (screen_height - outbox_height) / 2;
    int width = (screen_width - outbox_width) / 2 - PADDING;
    int height = outbox_height;

    // create the window
    if (right_window == NULL)
    {
        right_window = newwin(height, width, y0, x0);
        wbkgd(right_window, COLOR_PAIR(2));
    }

    // erase the window
    werase(right_window);

    // refresh the window
    wrefresh(right_window);
}

void display_elevator_window()
{
    // calculate
    int x0 = (screen_width - outbox_width) / 2 + 1;
    int y0 = (screen_height - outbox_height) / 2 + elevator_row;
    int width = outbox_width - 5;
    int height = 6;

    // create the window
    if (elevator_window == NULL)
    {
        elevator_window = newwin(height, width, y0, x0);
        wbkgd(elevator_window, COLOR_PAIR(2));
    }

    // erase the window
    werase(elevator_window);
    wrefresh(elevator_window);

    // move the window
    mvwin(elevator_window, y0, x0);
    fprintf(stderr, "elevator_window wmove: %d, %d\n", y0, x0);
    wrefresh(elevator_window);

    // draw the border
    wmove(elevator_window, 0, 0);
    whline(elevator_window, ACS_S1, width);
    wmove(elevator_window, 5, 0);
    whline(elevator_window, ACS_S1, width);

    // refresh the window
    wrefresh(elevator_window);
}

void display_container_window()
{
    // calculate
    int x0 = PADDING;
    int y0 = (screen_height - outbox_height) / 2 - 2;
    int width = screen_width - PADDING * 2;
    int height = outbox_height + 5;

    // create the window
    if (container_window == NULL)
    {
        container_window = newwin(height, width, y0, x0);
        wbkgd(container_window, COLOR_PAIR(2));
    }

    // erase the window
    werase(container_window);
    wrefresh(container_window);
}

void display_person()
{
    // empty position variables;
    int i, elevator_position = 0;
    int left_positions[FLOORS], right_positions[FLOORS];

    memset(left_positions, 0, sizeof(left_positions));
    memset(right_positions, 0, sizeof(right_positions));

    int position = 0, x0, y0;
    // unsigned char arrow_up = 0x2191, arrow_down = 0x2193;

    for (i = 0; i < TOTAL_PERSONS; i++)
    {
        if (g_persons[i].state == PERSON_WAITING)
        {
            position = ++left_positions[g_persons[i].start_floor];
            position = position * 12;

            y0 = (FLOORS - g_persons[i].start_floor - 1) * 5 + 2;
            x0 = (screen_width - outbox_width) / 2 - PADDING - position - 1;
            if (g_persons[i].direction == UP)
            {
                if (i < 10)
                {
                    // mvwprintw(left_window, y0, x0, "%d(To %d UP)", i, g_persons[i].dest_floor, arrow_up);
                    mvwprintw(left_window, y0, x0, "0%d(To %d UP)", i, g_persons[i].dest_floor);
                }
                else
                {
                    // mvwprintw(left_window, y0, x0, "%d(To %d UP)", i, g_persons[i].dest_floor, arrow_up);
                    mvwprintw(left_window, y0, x0, "%d(To %d UP)", i, g_persons[i].dest_floor);
                }
            }
            else
            {
                if (i < 10)
                {
                    // mvwprintw(left_window, y0, x0, "%d(To %d DW)", i, g_persons[i].dest_floor, arrow_down);
                    mvwprintw(left_window, y0, x0, "0%d(To %d DW)", i, g_persons[i].dest_floor);
                }
                else
                {
                    // mvwprintw(left_window, y0, x0, "%d(To %d DW)", i, g_persons[i].dest_floor, arrow_down);
                    mvwprintw(left_window, y0, x0, "%d(To %d DW)", i, g_persons[i].dest_floor);
                }
            }
        }
        else if (g_persons[i].state == PERSON_BEING_SERVED)
        {
            position = elevator_position++;
            position = position * 2;
            if (i < 10)
            {
                mvwprintw(elevator_window, 1 + 2 * (position / 10), 1 + 2 * (position % 10), "0%d", i);
            }
            else
            {
                mvwprintw(elevator_window, 1 + 2 * (position / 10), 1 + 2 * (position % 10), "%d", i);
            }
        }
        else if (g_persons[i].state == PERSON_SERVED)
        {
            position = right_positions[g_persons[i].dest_floor]++;
            position = position * 11;

            y0 = (FLOORS - g_persons[i].dest_floor - 1) * 5 + 2;
            x0 = position + 1;

            if (i < 10)
            {
                mvwprintw(right_window, y0, x0, "0%d(From %d)", i, g_persons[i].start_floor);
            }
            else
            {
                mvwprintw(right_window, y0, x0, "%d(From %d)", i, g_persons[i].start_floor);
            }
        }
    }

    wrefresh(left_window);
    wrefresh(elevator_window);
    wrefresh(right_window);
}
