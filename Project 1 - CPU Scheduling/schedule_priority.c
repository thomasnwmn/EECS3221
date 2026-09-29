/** 
 * Name: Thomas Newman
 * Student Number: 221224662
 * Purpose: Simulating Highest Priority First CPU scheduling 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedulers.h"
#include "task.h"
#include "list.h"
#include "cpu.h"

/* Setting both of the scheduler state variables outside of the functions so that both can be accessed. */
static struct node *head = NULL;
static struct node *tail = NULL;
static int task_count = 0;

/**  Storing each new task at the end of the list to preserve input order
 *   For example, P0, 5, 25, P1, 9, 15, P2, 8, 12, would result in:
 *   head -> P0 -> P1 -> P2 -> NULL    where P2 is the tail
*/
void add( char *name, int priority, int burst) {
    /* creating space in the memory for a new task, node and name */
    Task *task = malloc(sizeof *task);
    struct node *node = malloc(sizeof *node);
    char *name_copy = malloc(strlen(name) +1);

    /* If any of the allocations fails, release the successful allocations and exit */

    if (task == NULL || node == NULL || name_copy == NULL) {
        free(task);
        free(node);
        free(name_copy);
        fprintf(stderr, "Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(name_copy, name);

    /* setting the task variables to each function argument (name, task id, burst, priority) */
    task->name = name_copy;
    task->tid = task_count;
    task->burst = burst;
    task->priority = priority;

    node->task = task;
    node->next = NULL;

    /* checking if this is the first node, and setting it up accordingly */
    if (head == NULL) {
        /* for the case that this is the first node */
        head = node;
        tail = node;
    } else {
        tail->next = node;
        tail = node;
    }

    task_count++;
}

/**
 * This scheduler runs every task once, in the order of highest priority first
 */
void schedule() {

    double current_time = 0.0;
    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;

    /**
     * All tasks are considered to arrive at time 0, and run once to completion
     * Before execution of the task, current_time is the start time
     * so it equals waiting and response time.
     * Turnaround time is the completion time, so after running,
     * current_time is the turnaround time.
     */
    while (head != NULL) {

        struct node *current = head;
        struct node *previous = NULL;

        struct node *priority = head;
        struct node *priority_previous = NULL;

        /* finding the highest priority task in the list */
        while (current != NULL) {
            if (priority->task->priority < current->task->priority) {
                priority = current;
                priority_previous = previous;
            }

            previous = current;
            current = current->next;
        }

        /* found the highest priority task, now run the task and calculate turnaround time */

        Task *priority_task = priority->task;

        total_waiting += current_time;
        total_response += current_time;

        run(priority_task, priority_task->burst);
        current_time += priority_task->burst;

        total_turnaround += current_time;


        if (priority_previous == NULL) {
            head = priority->next;
        } else {
            priority_previous->next = priority->next;
        }

        if (priority == tail) {
            tail = priority_previous;
        }

        /* free the task from the memory and move on */
        free(priority->task->name);
        free(priority->task);
        free(priority);
    }

    double average_waiting = 0.0;
    double average_turnaround = 0.0;
    double average_response = 0.0;

    /* if we ran at least one task, calculate the average times by taking the total time and dividing it by how many task we ran */
    if (task_count > 0) {
        average_waiting = total_waiting / task_count;
        average_turnaround = total_turnaround / task_count;
        average_response = total_response / task_count;
    }

    printf("\nAverage waiting time = %.2f\n", average_waiting);
    printf("Average turnaround time = %.2f\n", average_turnaround);
    printf("Average response time = %.2f\n", average_response);

    /* final clean up */
    head = NULL;
    tail = NULL;
    task_count = 0;
}