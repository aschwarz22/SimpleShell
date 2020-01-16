#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "parseline.h"


/* Turns input line into tokens, then pipe stages */
int parseline(char *line, struct stage *stg, int concurrent_argnums[CMD_MAX])
{
   int filedes[2];
   int c;
   char *cmdarg;
   char *arg;
   char *stgdup;
   char *argdup;
   input fullString;
   char str2[512];
   strncpy(str2, line, 512);
   initInput(str2, &fullString);
   

   /*initialize stage in memory */
   memset(stg, 0, sizeof(struct stage) * STAGE_MAX);

   /* get the first pipe of input */
   cmdarg = strtok_r(line, "|", &stgdup);

   /*counts commands */
   c = 0;
   while (cmdarg != NULL)
   {
      strcpy(stg[c].cmd, cmdarg);

      /* begins to pipe stages */
      if (c != 0)
      {
         pipe(filedes);
         stg[c-1].outfd = filedes[WR_END];
         stg[c].infd = filedes[RD_END];
      }
 
      /* seperate stage into commands in stage */
      arg = strtok_r(cmdarg, " ", &argdup);

      /* initialize arg count of stage */
      stg[c].argc = 0;

      /* iterate though all commands in current stage */
      while (arg != NULL)
      {
         /* NEW ADDITION- check for concurrency */
         if (!strcmp(arg, "&"))
         {
            /* indicates which command in the stage is to be run concurrently */
            concurrent_argnums[c] = 1;
            arg = strtok_r(NULL, " ", &argdup);
            if (arg == NULL){
               break;
            }
         }
  
         /*check for redirection */
         else if (strcmp(arg, ">") == 0)
         {

            arg = strtok_r(NULL, " ", &argdup);
            if ((stg[c].outfd = open(arg, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR
               | S_IWUSR)) < 0)
            {
               perror(arg);
               return -1;
            }
         }

         /* incremment argument count of stage */
         else 
         {
            strcpy(stg[c].argv[stg[c].argc++], arg);
         }

         /* go to next command in stage */
         arg = strtok_r(NULL, " ", &argdup);
      }
      
      /* go to next stage */
      cmdarg = strtok_r(NULL, "|", &stgdup);
      c++;
   }

   /* if no redirection, default to standard in and standard out */
   if (!stg[0].infd){
      stg[0].infd = dup(STDIN_FILENO);
   }
   if (!stg[c-1].outfd){
      stg[c-1].outfd = dup(STDOUT_FILENO);
   }

   return c;
}


void initInput(char *str, input *in) {
   in->len = 0;
   in->str[in->len++] = strtok(str, " \n");

   while ((in->str[in->len++] = strtok(NULL, " \n")) != NULL) {
      if (in->len == (131) 
         && in->str[in->len - 1] != NULL) {
         break;
      }
   }

   in->len--; 
}
