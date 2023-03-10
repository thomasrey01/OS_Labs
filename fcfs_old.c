#include "scheduler.h"
#include "queue.h"

#include <stdio.h>

struct Job_info {
	size_t job_id;
	size_t pos, size;
	uint running, done;
	size_t start_time, end_time;
	struct Job_task {
		enum TASK_TYPE {
			IO,
			CPU,
		} type;
		uint cycles;
	} *tasks;
};

struct Queue_info {
	uint cycles;
	size_t job_id;
};

static struct Job_info *create_jobs(struct Schedule_task **jobs_incoming,
				    const size_t n)
{
	struct Job_info *jobs = malloc(n * sizeof(*jobs));
	for (size_t i = 0; i < n; i++) {
		jobs[i] = (struct Job_info){
		    .job_id	= i,
		    .pos	= 0,
		    .size	= jobs_incoming[i]->size,
		    .running	= 0,
		    .done	= 0,
		    .start_time = jobs_incoming[i]->start,
		    .end_time	= 0,
		};
		jobs[i].tasks =
		    malloc(jobs_incoming[i]->size * sizeof(*jobs[i].tasks));
		for (size_t j = 0; j < jobs_incoming[i]->size; j++) {
			if (j % 2)
				jobs[i].tasks[j].type = IO;
			else
				jobs[i].tasks[j].type = CPU;
			jobs[i].tasks[j].cycles = jobs_incoming[i]->tasks[j];
		}
	}

	return jobs;
}

static void free_jobs(struct Job_info *jobs, const size_t n)
{
	for (size_t i = 0; i < n; i++)
		free(jobs[i].tasks);
	free(jobs);
}

static uint is_done(const struct Job_info *jobs, const size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (!jobs[i].done)
			return 0;
	}
	return 1;
}

static void perform_action(struct Job_info *jobs, const size_t cycles,
			   Queue *qp, struct Queue_info **left)
{
	if (!queue_is_empty(qp) && (!*left || !(*left)->cycles))
		*left = *(struct Queue_info **)queue_dequeue(qp);

	if (!*left)
		return;

	if ((*left)->cycles)
		(*left)->cycles--;

	if ((*left)->cycles)
		return;

	jobs[(*left)->job_id].running = 0;
	jobs[(*left)->job_id].pos++;
	if (jobs[(*left)->job_id].pos >= jobs[(*left)->job_id].size) {
		jobs[(*left)->job_id].end_time = cycles;
		jobs[(*left)->job_id].done     = 1;
	}

	free(*left);
	*left = NULL;
}

void scheduler(struct Schedule_task **jobs_incoming, size_t n, uint *ret)
{
	size_t cycles = 0;

	struct Job_info *jobs = create_jobs(jobs_incoming, n);

	Queue *io_queue		   = queue_create(sizeof(struct Queue_info *));
	Queue *cpu_queue	   = queue_create(sizeof(struct Queue_info *));
	struct Queue_info *io_left = NULL, *cpu_left = NULL;

	for (; !is_done(jobs, n); cycles++) {
		for (size_t i = 0; i < n && jobs[i].start_time <= cycles; i++) {
			if (jobs[i].running || jobs[i].done)
				continue;
			struct Queue_info *qi = malloc(sizeof(*qi));
			qi->cycles = jobs[i].tasks[jobs[i].pos].cycles;
			qi->job_id = i;
			if (jobs[i].tasks[jobs[i].pos].type == IO)
				queue_enqueue(io_queue, &qi);
			else
				queue_enqueue(cpu_queue, &qi);

			jobs[i].running = 1;
		}

		perform_action(jobs, cycles, io_queue, &io_left);
		perform_action(jobs, cycles, cpu_queue, &cpu_left);
	}

	for (size_t i = 0; i < n; i++)
		ret[i] = jobs[i].end_time - jobs[i].start_time;

	queue_destroy(io_queue);
	queue_destroy(cpu_queue);
	free_jobs(jobs, n);
}

