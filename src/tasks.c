/*
* Copyright (c) 2014-2018
* Konrad Kusnierz <iryont@gmail.com>
*
* All information, intellectual and technical concepts contained herein is,
* and remains the property of the above person. Dissemination of this
* information or reproduction of this material is strictly forbidden unless
* prior written permission is obtained.
*/

#include "tasks.h"

// list containing tasks
struct Task* tasks = NULL;
struct Task* current = NULL;

void Tasks_tick_ms(void)
{
	struct Task* node = tasks;
	while (node)
	{
		if (!node->running && ++node->ticks >= node->millis && node->enabled)
		{
			// clear total amount of ticks
			node->ticks = 0;

			// lock the task for execution
			node->running = TRUE;
		}

		node = node->next;
	}
}

void Tasks_poll(void)
{
	struct Task* node = tasks;
	struct Task* last = node;

	while (node)
	{
		if (node->enabled && node->running)
		{
			if (node->listener)
			{
				current = node;
				node->listener();
				current = NULL;
			}

			// scheduled task is one time task
			if (node->schedule == TASK_SCHEDULE)
			{
				if (node->callback)
				{
					node->callback();
				}

				ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
				{
					if (tasks == node)
					{
						tasks = node->next;
					}
					else
					{
						last->next = node->next;
					}
				}

				// next node
				struct Task* next = node->next;
				free(node);
				node = next;
				
				continue;
			}

			// the task has been executed
			node->running = FALSE;
		}

		last = node;
		node = node->next;
	}
}

uint16_t Tasks_count(void)
{
	// initialize variable to 0
	uint16_t n = 0;
	struct Task* node = tasks;

	while (node)
	{
		++n;
		node = node->next;
	}

	return n;
}

struct Task* Tasks_create(uint16_t millis, enum Schedule_t schedule, void (*listener)(void), void (*callback)(void))
{
	struct Task* node = (struct Task*)malloc(sizeof(struct Task));

	// it will be the last entry, so no next node
	node->next = NULL;

	// node values
	node->millis = millis;
	node->schedule = schedule;
	node->listener = listener;
	node->callback = callback;

	// tmp variable as comparator
	node->ticks = 0;
	node->running = FALSE;
	node->enabled = TRUE;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (current && current->callback)
		{
			node->callback = current->callback;
			current->callback = NULL;
		}

		if (tasks == NULL)
		{ // no main node yet
			tasks = node;
		}
		else
		{
			struct Task* parent = tasks;
			while (parent->next)
			{
				parent = parent->next;
			}

			parent->next = node;
		}
	}

	return node;
}

BOOL Tasks_disable(struct Task* pTask)
{
	if (pTask != NULL && pTask->enabled)
	{
		pTask->enabled = FALSE;
		return TRUE;
	}

	return FALSE;
}

BOOL Tasks_enable(struct Task* pTask)
{
	if (pTask != NULL && pTask->enabled == FALSE)
	{
		pTask->millis = 0;
		pTask->enabled = TRUE;
		return TRUE;
	}

	return FALSE;
}

BOOL Tasks_restart(struct Task* pTask)
{
	if (pTask != NULL && pTask->enabled)
	{
		pTask->millis = 0;
		return TRUE;
	}

	return FALSE;
}
