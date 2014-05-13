/* e-comm.h - Main file
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more
 * details (available in COPYING).
 */

#include "e-comm.h"

extern rtp_t rtp;

pthread_t job_sout;                     /* Sound output */
pthread_t job_cin;                      /* Communication input */
pthread_t job_cout;                     /* Communication output */

/* Safely closes up program */
void close_up (int sigtype)
{
  /* Kill all threads */
  pthread_kill (job_sout, SIGKILL);
  pthread_kill (job_cin, SIGKILL);
  pthread_kill (job_cout, SIGKILL);

  rtp_destroy (&rtp);

  fprintf(stderr, "Program aborted\n");

  /* Return signal number to environment */
  exit (sigtype);
}

/* Initialization settings */
void init_program (void) {

  rtp_init (&rtp, RTP_SENDER);

  signal(SIG_PAYLOAD_TIMER, SIG_IGN);//ignore alarm signal
  signal (SIGINT, close_up);
  signal (SIGABRT, close_up);
}

int main (int argc, char *argv[])
{
  char *hostname = NULL;        /* Host to connect */
  int listen = 0;               /* Listen flag */

  int c;                        /* Used by getopt() */

  if (argc == 1) {
    fprintf (stderr, "Syntax: %s [-l] [-h hostname].\n", argv[0]);
    fprintf (stderr, "   -l listen call\n");
    fprintf (stderr, "   -h talk to hostname\n");
    exit(1);
  }

  while ((c = getopt(argc, argv, "h:l")) != -1) {
    switch (c) {
      case 'h':
        hostname = optarg;
        break;
      case 'l':
        listen = 1;
        break;
      case '?':
        if (isprint (optopt))
          fprintf (stderr, "Unknown option '-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
        exit(1);
      default:
        abort();
    }
  }

  /* Initialize */
  init_program();

  if (hostname != NULL) {
    /* Communication output */
    pthread_create (&job_cout, (pthread_attr_t *) NULL, mod_cout, hostname);
  }

  if (listen > 0) {
    /* Sound output */
    pthread_create (&job_sout, (pthread_attr_t *) NULL, mod_sout, NULL);

    /* Communication input */
    pthread_create (&job_cin, (pthread_attr_t *) NULL, mod_cin, NULL);
  }

  pthread_join (job_sout, NULL);
  pthread_join (job_cin, NULL);
  pthread_join (job_cout, NULL);

  return(0);
}
