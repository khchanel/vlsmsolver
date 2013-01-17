/*********************************************************
 * ui_cli.c  --- Part of VLSM Solver program
 * Copyright (C) 2011 Nelson Ka Hei Chan <khcha.n.el@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vlsm.h"


static void
usage (char * argv0)
{
  printf("VLSM Solver %s --- by Nelson Chan\n",VLSM_VERSION);
  printf("Usage: %s [ base_network base_netmask [numbers...] ]\n",argv0);
  printf("EX: %s 218.20.30.0 22  477 40 10 2\n",argv0);
  printf("If no parameter given, will run in interactive mode.\n");
}


static int
normal_intf (int argc, char ** argv)
{
  /* initialize */
  int i;
  ipv4str_t ipstr;
  network_t * subnets;
  int vlsm_code=0;
  int num_subnets = argc - 3 ;


  /* init given_net */
  network_t given_net;
  strtoipv4(given_net.addr,argv[1]);
  given_net.mask = (unsigned char) atoi (argv[2]);
  makenetwork(&given_net,given_net.addr,given_net.mask);

  /* init nhosts array */
  unsigned long n_arr[num_subnets];
  for (i=0;i<num_subnets;i++) {
    n_arr[i] = (unsigned long) atol (argv[i+3]);
  }


  /* print desciption */
  ipv4tostr(ipstr,given_net.addr);
  printf("## Given network: %s/%d\n",ipstr,given_net.mask);
  printf("## %d subnets to address\n",num_subnets);
  printf("## Format: net_addr/smask (dmask)|first_host|last_host|broadcast [usable]\n");

  /* allocate memory for subnets */
  subnets = (network_t *) malloc (sizeof(network_t) * num_subnets);
  if (subnets == NULL) {
    printf("#Error: memory error\n");
    return 3;
  }

  /* VLSM */
  vlsm_code = vlsm(subnets,given_net.addr,given_net.mask,n_arr,num_subnets);

  /* output */
  if (vlsm_code >= 0) {
    for (i=0;i<num_subnets;i++) {
      if (n_arr[i] == 0 ) continue; /* due to invalid user input */
      printf("# Subnet %ld :\n",n_arr[i]);
      print_network(&subnets[i]);
    }
  } else if (vlsm_code == -1) {
    printf("#Error: invalid net mask\n");
    return 1;
  } else if (vlsm_code == -2) {
    printf("#Error: too many or no host to address for the given network\n");
    return 2;
  }

  return 0;
}


/* as a wrapper to normal_intf */
static int interactive_intf (const char * argv0)
{
  /* main */
  int i;
  int run=1;
  while (run) {
    /* init */
    char  tmpstr[32];
    char ** new_argv;
    new_argv = (char **)malloc(sizeof(char *));
    new_argv[0] = (char *) malloc (sizeof (char) * strlen(argv0) +1);
    int new_argc=1;
    strcpy(new_argv[0],argv0);
    printf("VLSM Solver %s --- by Nelson Chan\n",VLSM_VERSION);
    printf("Interactive mode\n** see %s --help\n\n",new_argv[0]);

    /* input net addr */
    printf("Enter the base network's address: ");
    scanf("%s",tmpstr);
    new_argc += 1;
    new_argv = (char **) realloc (new_argv, sizeof(char *) * new_argc );
    new_argv[new_argc-1] = (char *) malloc (sizeof(char) * strlen(tmpstr) + 1);
    strcpy (new_argv[new_argc-1] , tmpstr);
    printf ("addr:%s\n",new_argv[new_argc-1]);

    /* input mask */
    printf("Enter the base network's mask: ");
    scanf("%s",tmpstr);
    new_argc += 1;
    new_argv = (char **) realloc (new_argv, sizeof(char *) * new_argc );
    new_argv[new_argc-1] = (char *) malloc (sizeof(char) * strlen(tmpstr) + 1);
    strcpy (new_argv[new_argc-1] , tmpstr);
    printf("mask:%s\n",new_argv[new_argc-1]);

    /* input subnets host num */
    printf("Enter the no. of hosts in each subnets, enter 0 to end:\n");
    scanf("%s",tmpstr);
    while (strcmp(tmpstr,"0") != 0) {
      new_argc += 1;
      new_argv = (char **) realloc (new_argv, sizeof(char *) * new_argc );
      new_argv[new_argc-1] = (char *) malloc (sizeof(char) * strlen(tmpstr)+ 1);
      strcpy (new_argv[new_argc-1] , tmpstr);

      scanf("%s",tmpstr);
    }

    /* verbose */
    printf("@ new_argc = %d\n",new_argc);
    for (i=0;i<new_argc;i++) {
      printf("@ %s\n",new_argv[i]);
    }

    /* call normal_intf */
    printf("\n\n");
    normal_intf(new_argc,new_argv);

    /* free new_argv... */
    for (i=0;i<new_argc;i++) {
      free(new_argv[i]);
    }
    free(new_argv);

    /* finish */
    printf("\nAgain? (yes/NO) ");
    scanf("%s",tmpstr);

    if (strncmp(tmpstr,"yes",3) == 0 ) {
      printf("\n\n");
    } else {
      run = 0;
    }
  }

  return 0;
}


int main (int argc, char ** argv)
{
  /* init */
  int intf=0; /* 0 for normal mode, 1 for interactive */

  /* check args */
  if (argc == 1) {
    intf = 1;
  }  else if ( argc < 3 ) {
    usage(argv[0]);
    return 0 ;
  }

  /* main */
  return (intf == 0)?normal_intf(argc,argv):interactive_intf(argv[0]);
}
