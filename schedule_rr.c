/** 
 * Name: Thomas Newman
 * Student Number: 221224662
 * Purpose: Simulating Round Robin CPU scheduling 
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

    /* setting the task variables to each function argument (name, task id, burst, priority)*/
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
 * This scheduler runs every task in a round robin, running them in chronological order with a time quantum of 10
 */
void schedule() {

    double current_time = 0.0;
    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;

    /* Save the original burst total before any tasks remaining burst changes. */
    double total_original_burst = 0.0;
    for (struct node *node = head; node != NULL; node = node->next) {
        total_original_burst += node->task->burst;
    }

    /** 1 means true: all tasks get their first turn in the first round.
     * Set this to 0 after that round so response time is counted only once.
     */
    int first_round = 1;

    /** All tasks arrive at 0. Response is the first start time, turnaround
     * is the final completion time, and waiting is turnaround minus
     * the original burst. Tasks can execute across multiple turns.
     */

    while (head != NULL) {
        struct node *current = head;
        struct node *previous = NULL;
        while (current != NULL) {
            Task *task = current->task;

            if (first_round) {
                total_response += current_time;
            }

            if (task->burst > QUANTUM) {
                run(task, QUANTUM);
                current_time += QUANTUM;

                task->burst -= QUANTUM;
                previous = current;
                current = current->next;
            } else {
                run(task, task->burst);
                current_time += task->burst;
                
                total_turnaround += current_time;

                struct node *next = current->next;
                if (previous == NULL) {
                    head = next;
                } else {
                    previous->next = next;
                }

                if (current == tail) {
                    tail = previous;
                }

                free(current->task->name);
                free(current->task);
                free(current);

                current = next;
            }

        }
        first_round = 0;
    }

    /* Sum (completion - original burst) equals the total waiting time. */
    total_waiting = total_turnaround - total_original_burst;
    

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