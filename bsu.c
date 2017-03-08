/* External definitions for job-shop model. */

#include "simlib.h"		/* Required for use of simlib.c. */

#define EVENT_ARRIVAL         1	/* Event type for arrival of a job to the system. */
#define EVENT_DEPARTURE       2	/* Event type for departure of a job from a particular station. */
#define EVENT_END_SIMULATION  3	/* Event type for end of the simulation. */
#define STREAM_INTERARRIVAL   1	/* Random-number stream for interarrivals. */
#define STREAM_GROUP_SIZE     2 /* Random-number stream for group size */
#define STREAM_JOB_TYPE       3	/* Random-number stream for job types. */
#define MAX_NUM_STATIONS      4	/* Maximum number of stations. */
#define MAX_NUM_JOB_TYPES     3	/* Maximum number of job types. */
#define MAX_GROUP_SIZE        4

/* Declare non-simlib global variables. */

int num_stations, num_job_types, i, j, num_machines[MAX_NUM_STATIONS + 1], num_tasks[MAX_NUM_JOB_TYPES + 1], route[MAX_NUM_JOB_TYPES + 1][MAX_NUM_STATIONS + 1], 
num_machines_busy[MAX_NUM_STATIONS + 1], job_type[MAX_GROUP_SIZE + 1], task, group_size;
double mean_interarrival, length_simulation, prob_distrib_job_type[26], min_service[MAX_NUM_JOB_TYPES + 1][MAX_NUM_STATIONS + 1], max_service[MAX_NUM_JOB_TYPES + 1][MAX_NUM_STATIONS + 1];
FILE *infile, *outfile;

int get_group_size() {
  double prob_distrib_group_size[] = {0.0, 0.5, 0.8, 0.9, 1};
  return (random_integer(prob_distrib_group_size, STREAM_GROUP_SIZE) % 4) + 1;
}

int get_job_type_stream(int job_type) {
  switch (job_type) {
    case 1: return 4;
    case 2: return 5;
    case 3: return 6;
    default: return 7;
  }
}

void serve(int customer_number) {
  /* Determine the station from the route matrix. */
  int station = route[job_type[customer_number]][task];

  /* Check to see whether all machines in this station are busy. */
  if (num_machines_busy[station] == num_machines[station]) {

    /* All machines in this station are busy, so place the arriving job at
       the end of the appropriate queue. Note that the following data are
       stored in the record for each job:
       1. Time of arrival to this station.
       2. Job type.
       3. Current task number. */
    transfer[1] = sim_time;
    transfer[2] = job_type[customer_number];
    transfer[3] = task;
    transfer[5] = customer_number;
    list_file (LAST, station);
  }
  else {
    /* A machine in this station is idle, so start service on the arriving
       job (which has a delay of zero). */
    sampst (0.0, station);  /* For station. */
    sampst (0.0, num_stations + job_type[customer_number]);  /* For job type. */
    ++num_machines_busy[station];
    timest ((double) num_machines_busy[station], station);

    /* Schedule a service completion.  Note defining attributes beyond the
       first two for the event record before invoking event_schedule. */

    transfer[3] = job_type[customer_number];
    transfer[4] = task;
    transfer[5] = customer_number;
    event_schedule (sim_time + uniform (min_service[job_type[customer_number]][task], max_service[job_type[customer_number]][task], get_job_type_stream(job_type[customer_number])), EVENT_DEPARTURE);
  }
}

void arrive (int new_job) {
  /* Function to serve as both an arrival event of a job to the system, as well as the non-event of a job's arriving to a subsequent station along its route. */
  int station;

  /* If this is a new arrival to the system, generate the time of the next
     arrival and determine the job type and task number of the arriving
     job. */


  if (new_job == 1) {
    group_size = get_group_size();
    event_schedule (sim_time + expon (mean_interarrival, STREAM_INTERARRIVAL), EVENT_ARRIVAL);
    for (i = 1; i <= group_size; i++) {
      job_type[i] = random_integer (prob_distrib_job_type, STREAM_JOB_TYPE);
      task = 1;
    }
  }
  for (i = 1; i <= group_size; i++) {
    serve(i);
  }
}


void depart (void)			/* Event function for departure of a job from a particular station. */
{
  int station, job_type_queue, task_queue, customer_number;

  /* Determine the station from which the job is departing. */
  customer_number = transfer[5];
  job_type[customer_number] = transfer[3];
  task = transfer[4];
  station = route[job_type[customer_number]][task];

  /* Check to see whether the queue for this station is empty. */
  if (list_size[station] == 0)
    {

      /* The queue for this station is empty, so make a machine in this
         station idle. */

      --num_machines_busy[station];
      timest ((double) num_machines_busy[station], station);
    }

  else
    {

      /* The queue is nonempty, so start service on first job in queue. */

      list_remove (FIRST, station);

      /* Tally this delay for this station. */

      sampst (sim_time - transfer[1], station);

      /* Tally this same delay for this job type. */

      job_type_queue = transfer[2];
      task_queue = transfer[3];
      sampst (sim_time - transfer[1], num_stations + job_type_queue);

      /* Schedule end of service for this job at this station.  Note defining
         attributes beyond the first two for the event record before invoking
         event_schedule. */

      transfer[3] = job_type_queue;
      transfer[4] = task_queue;
      event_schedule (sim_time + uniform (min_service[job_type_queue][task_queue], max_service[job_type_queue][task_queue], get_job_type_stream(job_type_queue)), EVENT_DEPARTURE);
    }

  /* If the current departing job has one or more tasks yet to be done, send
     the job to the next station on its route. */

  if (task < num_tasks[job_type[customer_number]]) {
    ++task;
    serve (customer_number);
  }
}


void
report (void)			/* Report generator function. */
{
  int i;
  double overall_avg_job_tot_delay, avg_job_tot_delay, sum_probs;

  /* Compute the average total delay in queue for each job type and the
     overall average job total delay. */

  fprintf (outfile, "\n\n\n\nRoute     Average total delay in queue     Maximum total delay in queue");
  overall_avg_job_tot_delay = 0.0;
  sum_probs = 0.0;
  for (i = 1; i <= num_job_types; ++i)
    {
      avg_job_tot_delay = sampst (0.0, -(num_stations + i)) * num_tasks[i];
      fprintf (outfile, "\n\n%4d%24.3f%33.3f", i, avg_job_tot_delay, transfer[3]);
      overall_avg_job_tot_delay += (prob_distrib_job_type[i] - sum_probs) * avg_job_tot_delay;
      sum_probs = prob_distrib_job_type[i];
    }
  // fprintf (outfile, "\n\nOverall average job total delay =%10.3f\n", overall_avg_job_tot_delay);

  /* Compute the average number in queue, the average utilization, and the
     average delay in queue for each station. */

  fprintf (outfile, "\n\n\n\n Work      Average number    Maximum number   Average delay    Maximum delay");
  fprintf (outfile, "\nstation       in queue          in queue        in queue         in queue");

  double avg_num_customer = 0;
  double max_num_customer = 0;
  
  for (j = 1; j <= num_stations; ++j) {
    filest(j);
    double avg_num_queue = transfer[1];
    double max_num_queue = transfer[2];

    if (max_num_queue < 0) {
      max_num_queue = 0;
    }

    avg_num_customer += avg_num_queue;
    max_num_customer += max_num_queue;

    sampst(0.0, -j);
    double avg_delay_queue = transfer[1];
    double max_delay_queue = transfer[3];

    fprintf (outfile, "\n\n%4d%17.3f%17.3f%18.3f%17.3f", j, avg_num_queue, max_num_queue, avg_delay_queue, max_delay_queue);
  }

  fprintf (outfile, "\n\nAverage number of customers in the entire system = %f\n", avg_num_customer);
  fprintf(outfile, "Maximum number of customers in the entire system = %f\n", max_num_customer);
    
}

int
main ()				/* Main function. */
{
  /* Open input and output files. */
  int case_number;
  printf("Input case number:\n");
  scanf("%d", &case_number);

  switch(case_number) {
    case 0: infile = fopen ("base.in", "r"); outfile = fopen ("report-base-case.out", "w"); break;
    case 1: infile = fopen ("ai.in", "r"); outfile = fopen ("report-case-a-i.out", "w"); break;
    case 2: infile = fopen ("aii.in", "r"); outfile = fopen ("report-case-a-ii.out", "w"); break;
    case 3: infile = fopen ("aiii.in", "r"); outfile = fopen ("report-case-a-iii.out", "w"); break;
    case 4: infile = fopen ("bi.in", "r"); outfile = fopen ("report-case-b-i.out", "w"); break;
    case 5: infile = fopen ("bii.in", "r"); outfile = fopen ("report-case-b-ii.out", "w"); break;
    case 6: infile = fopen ("biii.in", "r"); outfile = fopen ("report-case-b-iii.out", "w"); break;
    case 7: infile = fopen ("c.in","r"); outfile = fopen ("report-case-c.out", "w"); break;
  }

  /* Read input parameters. */

  fscanf (infile, "%d %d %lg %lg", &num_stations, &num_job_types, &mean_interarrival, &length_simulation);
  for (j = 1; j <= num_stations; ++j)
    fscanf (infile, "%d", &num_machines[j]);
  for (i = 1; i <= num_job_types; ++i)
    fscanf (infile, "%d", &num_tasks[i]);
  for (i = 1; i <= num_job_types; ++i)
    {
      for (j = 1; j <= num_tasks[i]; ++j)
	fscanf (infile, "%d", &route[i][j]);
      for (j = 1; j <= num_tasks[i]; ++j)
  fscanf (infile, "%lg", &min_service[i][j]);
      for (j = 1; j <= num_tasks[i]; ++j)
  fscanf (infile, "%lg", &max_service[i][j]);
    }
  for (i = 1; i <= num_job_types; ++i)
    fscanf (infile, "%lg", &prob_distrib_job_type[i]);

  /* Write report heading and input parameters. */

  fprintf (outfile, "BSU Cafeteria Model\n\n");
  fprintf (outfile, "Number of work stations%21d\n\n", num_stations);
  fprintf (outfile, "Number of employees in each station     ");
  for (j = 1; j <= num_stations; ++j)
    fprintf (outfile, "%5d", num_machines[j]);
  fprintf (outfile, "\n\nNumber of routes%25d\n\n", num_job_types);
  fprintf (outfile, "Number of tasks for each route      ");
  for (i = 1; i <= num_job_types; ++i)
    fprintf (outfile, "%5d", num_tasks[i]);
  fprintf (outfile, "\n\nDistribution function of routes  ");
  for (i = 1; i <= num_job_types; ++i)
    fprintf (outfile, "%8.3f", prob_distrib_job_type[i]);
  fprintf (outfile, "\n\nMean interarrival time of jobs%14.2f seconds\n\n", mean_interarrival);
  fprintf (outfile, "Length of the simulation%20.1f seconds\n\n\n", length_simulation);
  fprintf (outfile, "Route     Work stations on route");
  for (i = 1; i <= num_job_types; ++i)
    {
      fprintf (outfile, "\n\n%4d        ", i);
      for (j = 1; j <= num_tasks[i]; ++j)
	fprintf (outfile, "%5d", route[i][j]);
    }
 //  fprintf (outfile, "\n\n\nJob type     ");
 //  fprintf (outfile, "Min service time (in seconds) for successive tasks");
 //  for (i = 1; i <= num_job_types; ++i)
 //    {
 //      fprintf (outfile, "\n\n%4d    ", i);
 //      for (j = 1; j <= num_tasks[i]; ++j)
	// fprintf (outfile, "%9.2f", min_service[i][j]);
 //    }

  /* Initialize all machines in all stations to the idle state. */
  for (j = 1; j <= num_stations; ++j)
    num_machines_busy[j] = 0;

  /* Initialize simlib */

  init_simlib ();

  /* Set maxatr = max(maximum number of attributes per record, 4) */

  maxatr = 5;     /* NEVER SET maxatr TO BE SMALLER THAN 4. */

  /* Schedule the arrival of the first job. */

  event_schedule (expon (mean_interarrival, STREAM_INTERARRIVAL), EVENT_ARRIVAL);

  /* Schedule the end of the simulation.  (This is needed for consistency of
     units.) */

  event_schedule (length_simulation, EVENT_END_SIMULATION);

  /* Run the simulation until it terminates after an end-simulation event
     (type EVENT_END_SIMULATION) occurs. */

  do
    {

      /* Determine the next event. */

      timing ();

      /* Invoke the appropriate event function. */

      switch (next_event_type)
  {
  case EVENT_ARRIVAL:
    arrive (1);
    break;
  case EVENT_DEPARTURE:
    depart ();
    break;
  case EVENT_END_SIMULATION:
    report ();
    break;
  }

      /* If the event just executed was not the end-simulation event (type
         EVENT_END_SIMULATION), continue simulating.  Otherwise, end the
         simulation. */

    }
  while (next_event_type != EVENT_END_SIMULATION);

  fclose (infile);
  fclose (outfile);

  return 0;
}
